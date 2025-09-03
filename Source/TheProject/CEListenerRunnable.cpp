#include "CEListenerRunnable.h"
#include "Misc/ScopeLock.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Networking.h"

CEListenerRunnable::CEListenerRunnable(const FString& InServerIP, int32 InPort, TFunction<void(const FString&)> InOnMessageReceived)
	: ServerIP(InServerIP)
	, Port(InPort)
	, ClientSocket(nullptr)
	, OnMessageReceived(MoveTemp(InOnMessageReceived))
{
	PendingBuffer.Reserve(4096);
}

CEListenerRunnable::~CEListenerRunnable()
{
	// Caller should have stopped the thread; Stop() is safe to call again.
	Stop();
}

bool CEListenerRunnable::Init()
{
	// Do NOT connect here. Always return true so the thread can start and handle retries in Run().
	return true;
}

uint32 CEListenerRunnable::Run()
{
	while (!bStopThread)
	{
		// Ensure connected (retry every ReconnectDelayMs while not connected)
		if (!ClientSocket)
		{
			if (!TryConnect())
			{
				FPlatformProcess::Sleep(static_cast<float>(ReconnectDelayMs) / 1000.f);
				continue;
			}
		}

		// --- Connected receive loop ---
		while (!bStopThread)
		{
			// Local snapshot of the socket pointer; only Run() will destroy it,
			// so it's safe to use without holding the lock during blocking calls.
			FSocket* LocalSocket = ClientSocket;
			if (!LocalSocket)
			{
				break;
			}

			// Wait up to RecvWaitMs for data (keeps CPU cool and lets Stop() break via Shutdown)
			if (!LocalSocket->Wait(ESocketWaitConditions::WaitForRead, FTimespan::FromMilliseconds(RecvWaitMs)))
			{
				continue; // no data yet; loop again
			}

			// Check pending size
			uint32 PendingDataSize = 0;
			if (!LocalSocket->HasPendingData(PendingDataSize))
			{
				// Could be orderly shutdown; try a zero-length recv to detect disconnect
				int32 DummyBytes = 0;
				uint8 Dummy;
				if (!LocalSocket->Recv(&Dummy, 0, DummyBytes, ESocketReceiveFlags::None))
				{
					UE_LOG(LogTemp, Warning, TEXT("Socket appears closed by peer; reconnecting..."));
					CloseSocket(); // Run-thread only
					break;
				}
				continue;
			}

			PendingDataSize = FMath::Min<uint32>(PendingDataSize, static_cast<uint32>(RecvChunkMax));

			TArray<uint8> Data;
			Data.SetNumUninitialized(PendingDataSize);

			int32 BytesRead = 0;
			if (!LocalSocket->Recv(Data.GetData(), Data.Num(), BytesRead, ESocketReceiveFlags::None))
			{
				UE_LOG(LogTemp, Warning, TEXT("Recv failed; reconnecting..."));
				CloseSocket(); // Run-thread only
				break;
			}

			if (BytesRead <= 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("Socket closed by peer (0 bytes); reconnecting..."));
				CloseSocket(); // Run-thread only
				break;
			}

			// Append only the bytes we actually read
			PendingBuffer.Append(Data.GetData(), BytesRead);

			// Extract as many complete frames as we can (delimited by '\n')
			while (true)
			{
				// Find newline byte (0x0A). Support Windows '\r\n' by trimming '\r' later.
				int32 NewlineIdx = INDEX_NONE;
				for (int32 i = 0; i < PendingBuffer.Num(); ++i)
				{
					if (PendingBuffer[i] == '\n')
					{
						NewlineIdx = i;
						break;
					}
				}

				if (NewlineIdx == INDEX_NONE)
				{
					// No full frame yet; wait for more bytes
					break;
				}

				// ---- Extract one frame (bytes up to but not including '\n') ----
				// Handle optional '\r' before '\n' (CRLF)
				int32 FrameLen = NewlineIdx;
				if (FrameLen > 0 && PendingBuffer[FrameLen - 1] == '\r')
				{
					--FrameLen;
				}

				// Skip optional UTF-8 BOM at the start of the frame
				int32 StartIdx = 0;
				if (FrameLen >= 3 &&
					PendingBuffer[0] == 0xEF &&
					PendingBuffer[1] == 0xBB &&
					PendingBuffer[2] == 0xBF)
				{
					StartIdx = 3;
				}

				// Convert this exact frame from UTF-8 -> TCHAR safely
				const int32 Utf8Len = FrameLen - StartIdx;
				FString Message;
				if (Utf8Len > 0)
				{
					const ANSICHAR* Utf8Start = reinterpret_cast<const ANSICHAR*>(PendingBuffer.GetData() + StartIdx);
					FUTF8ToTCHAR Converter(Utf8Start, Utf8Len);
					Message = FString(Converter.Length(), Converter.Get());
				}
				else
				{
					Message.Empty();
				}

				// Pop the consumed frame + newline (and optional '\r')
				PendingBuffer.RemoveAt(0, NewlineIdx + 1, /*bAllowShrinking*/ EAllowShrinking::No);

				// Deliver the complete string (one line == one message)
				if (!Message.IsEmpty() && OnMessageReceived)
				{
					OnMessageReceived(Message);
				}
			}
		}

		// If we fell out due to disconnect and not stopping, wait a bit before next reconnect attempt
		if (!bStopThread && !ClientSocket)
		{
			FPlatformProcess::Sleep(static_cast<float>(ReconnectDelayMs) / 1000.f);
		}
	}

	return 0;
}

bool CEListenerRunnable::Send(const FString& Message)
{
	FScopeLock Lock(&SocketCS);

	// Snapshot pointer while holding the lock, but DO NOT destroy here.
	FSocket* LocalSocket = ClientSocket;
	if (!LocalSocket)
	{
		UE_LOG(LogTemp, Warning, TEXT("Send failed: not connected."));
		return false;
	}

	FTCHARToUTF8 Converter(*Message);
	const uint8* Data = reinterpret_cast<const uint8*>(Converter.Get());
	int32 Size = Converter.Length();

	int32 BytesSent = 0;
	const bool bSuccess = LocalSocket->Send(Data, Size, BytesSent);
	if (!bSuccess || BytesSent != Size)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to send data to server (sent %d / %d)."), BytesSent, Size);
		return false;
	}
	return true;
}

void CEListenerRunnable::Stop()
{
	// Signal the loop to exit
	bStopThread = true;

	// DO NOT destroy the socket here to avoid racing with Run().
	// Just shutdown to unblock Wait/Recv.
	FScopeLock Lock(&SocketCS);
	if (ClientSocket)
	{
		ClientSocket->Shutdown(ESocketShutdownMode::ReadWrite);
	}
}

void CEListenerRunnable::Exit()
{
	// Called on the runnable thread after Run() returns.
	// Safe place to destroy the socket.
	CloseSocket();
}

bool CEListenerRunnable::TryConnect()
{
	ISocketSubsystem* Subsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("Socket subsystem not available"));
		return false;
	}

	bool bIsValid = false;
	TSharedRef<FInternetAddr> Addr = Subsystem->CreateInternetAddr();
	Addr->SetIp(*ServerIP, bIsValid);
	Addr->SetPort(Port);

	if (!bIsValid)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid server IP address: %s"), *ServerIP);
		return false;
	}

	// Create socket as client
	FSocket* NewSocket = Subsystem->CreateSocket(NAME_Stream, TEXT("NPCEventClient"), false);
	if (!NewSocket)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create client socket"));
		return false;
	}

	// Optional tuning
	int32 ActualSize = RecvChunkMax;
	NewSocket->SetReceiveBufferSize(RecvChunkMax, ActualSize);
	NewSocket->SetReuseAddr(true);
	NewSocket->SetNonBlocking(false);
	NewSocket->SetLinger(false, 0);

	// Try connect (blocking)
	if (!NewSocket->Connect(*Addr))
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not connect to server at %s:%d"), *ServerIP, Port);
		Subsystem->DestroySocket(NewSocket);
		return false;
	}

	// Install as current socket
	{
		FScopeLock Lock(&SocketCS);
		ClientSocket = NewSocket;
	}

	UE_LOG(LogTemp, Log, TEXT("Connected to server at %s:%d"), *ServerIP, Port);
	return true;
}

void CEListenerRunnable::CloseSocket()
{
	// Only called on the Run thread (Exit or on error inside Run)
	FScopeLock Lock(&SocketCS);
	if (ClientSocket)
	{
		ISocketSubsystem* Subsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		ClientSocket->Close();
		if (Subsystem)
		{
			Subsystem->DestroySocket(ClientSocket);
		}
		ClientSocket = nullptr;
		PendingBuffer.Reset();
	}
}