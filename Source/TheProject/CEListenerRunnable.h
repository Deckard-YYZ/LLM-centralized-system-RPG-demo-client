#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Networking.h"


class CEListenerRunnable : public FRunnable
{
public:
	// Pass in server IP as FString (e.g. TEXT("127.0.0.1")), port, and message callback.
	CEListenerRunnable(const FString& InServerIP, int32 InPort, TFunction<void(const FString&)> InOnMessageReceived);
	virtual ~CEListenerRunnable();

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

	/** Send a message to the server (thread-safe if called properly) */
	bool Send(const FString& Message);

private:
	// Helpers
	bool TryConnect();
	void CloseSocket();
	
	FString ServerIP;
	int32 Port;
	FSocket* ClientSocket;
	TFunction<void(const FString&)> OnMessageReceived;
	FThreadSafeBool bStopThread;
	TArray<uint8> PendingBuffer; // byte buffer across Recv calls

	// Basic config
	int32 ReconnectDelayMs = 1000; // retry every ~1s when not connected
	int32 RecvWaitMs       = 100;  // wait up to 100ms for readability
	int32 RecvChunkMax     = 64 * 1024; // cap recv chunk

	// Thread-safety for socket send/close from outside Run()
	FCriticalSection SocketCS;
};
