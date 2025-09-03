#include "UExternalCEHandleComp.h"

#include "CEListenerRunnable.h"
#include "FParsedCEInfoCPP.h"
#include "FToSendPreCEInfo.h"

#include "JsonObjectConverter.h"
#include "Json.h"
#include "MyGameModeBaseCpp.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Async/Async.h"
void UExternalCEHandleComp::BeginPlay()
{
	Super::BeginPlay();

	FString ServerIP = TEXT("127.0.0.1");
	int32 ServerPort = 7777;

	ListenerRunnable = new CEListenerRunnable(ServerIP, ServerPort, [this](const FString& Message)
	{
		// We're on a different thread, so hop to game thread
		AsyncTask(ENamedThreads::GameThread, [this, Message]()
		{
			HandleMessage(Message);
		});
	});

	Thread = FRunnableThread::Create(ListenerRunnable, TEXT("NPCEventThread"));
}

void UExternalCEHandleComp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ListenerRunnable)
	{
		ListenerRunnable->Stop();         // Signal the runnable to exit loop
		if (Thread)
		{
			Thread->WaitForCompletion();  // Wait for thread to fully exit (safer than Kill)
			delete Thread;
			Thread = nullptr;
		}
		delete ListenerRunnable;
		ListenerRunnable = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void UExternalCEHandleComp::SendMessageToServer(const FToSendPreCEInfo& Info)
{
	FString MessageJson;

	// Convert struct to JSON string
	if (FJsonObjectConverter::UStructToJsonObjectString(Info, MessageJson))
	{
		if (ListenerRunnable)
		{
			bool bSent = ListenerRunnable->Send(MessageJson);

			if (bSent)
			{
				UE_LOG(LogTemp, Log, TEXT("Sent message to server: %s"), *MessageJson);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to send message to server."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ListenerRunnable is not valid!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to serialize FToSendPreCEInfo to JSON!"));
	}
}

void UExternalCEHandleComp::HandleMessage(const FString& Message)
{
	TArray<TSharedPtr<FJsonValue>> JsonArray;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);

    if (!FJsonSerializer::Deserialize(Reader, JsonArray) || JsonArray.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("HandleMessage: failed to deserialize or empty array"));
        return;
    }
	
	// --- Handle the initial "change" block (legacy + new schema) ---
	// We'll process a contiguous prefix of the array where each element either:
	// - has exactly 5 fields (matches new Att/Rel schema)
	int32 CutoffIdx = INDEX_NONE;
	{
		// Identify the contiguous prefix that qualifies as "change" entries
		for (int32 i = 0; i < JsonArray.Num(); ++i)
		{
			const TSharedPtr<FJsonValue>& Val = JsonArray[i];
			if (!Val.IsValid() || Val->Type != EJson::Object) { break; }

			const TSharedPtr<FJsonObject>& Obj = Val->AsObject();
			if (!Obj.IsValid()) { break; }

			// const bool bHasRationale = Obj->HasField(TEXT("Rationale"));
			// const bool bHasRelChange = Obj->HasField(TEXT("RelationshipTypeChange"));
			const bool bHas5Fields   = (Obj->Values.Num() == 5);

			// If any of the 'change' markers are present, extend the cutoff
			if (bHas5Fields)
			{
				CutoffIdx = i;
				continue;
			}

			// As soon as we hit a non-matching element, stop the prefix
			break;
		}

		// Early out if no qualifying entries at the head
		if (CutoffIdx == INDEX_NONE)
		{
			// Nothing to do for the "change" block
		}
		else
		{
			UWorld* World = GetWorld();
			AMyGameModeBaseCpp* GM = nullptr;
			if (World)
			{
				GM = Cast<AMyGameModeBaseCpp>(World->GetAuthGameMode());
			}

			// If we don't have a GM, we still parse safely but skip the calls
			for (int32 i = 0; i <= CutoffIdx; ++i)
			{
				const TSharedPtr<FJsonObject>& Obj = JsonArray[i]->AsObject();

				// --- Legacy one-off "starter" element support (back-compat) ---
				// Legacy fields: "starterID", "IntermediatorID" (capital I), "AttitudeChange", "RelationshipTypeChange"
				{
					FString StarterID;
					bool bHasStarter = Obj->TryGetStringField(TEXT("starterID"), StarterID);

					if (bHasStarter)
					{
						// Legacy capitalized key
						FString IntermediatorID_legacy;
						Obj->TryGetStringField(TEXT("IntermediatorID"), IntermediatorID_legacy);

						FString AttChange;
						FString RelChange;
						Obj->TryGetStringField(TEXT("AttitudeChange"), AttChange);
						Obj->TryGetStringField(TEXT("RelationshipTypeChange"), RelChange);

						if ((!AttChange.IsEmpty() || !RelChange.IsEmpty()) && GM)
						{
							GM->HandleStarterChange(StarterID, IntermediatorID_legacy, AttChange, RelChange);
						}

						// If it's the legacy shape, skip further interpretation
						continue;
					}
				}

				// --- New schema "Att/Rel change" entries ---
				// Fields: "intermediatorID", "AttAndRelRCPT", "AttitudeChange", "RelationshipTypeChange", "Rationale"
				{
					FString IntermediatorID;
					FString AttAndRelRcpt;
					FString AttChange;
					FString RelChange;

					Obj->TryGetStringField(TEXT("intermediatorID"), IntermediatorID);       // lower-case 'i'
					Obj->TryGetStringField(TEXT("AttAndRelRCPT"), AttAndRelRcpt);
					Obj->TryGetStringField(TEXT("AttitudeChange"), AttChange);
					Obj->TryGetStringField(TEXT("RelationshipTypeChange"), RelChange);

					// Only act if recipient field is present and at least one change is non-empty
					if (!AttAndRelRcpt.IsEmpty() && (!AttChange.IsEmpty() || !RelChange.IsEmpty()))
					{
						if (GM)
						{
							// Reuse HandleStarterChange: pass recipient as "StarterID"-like, and the initiator
							GM->HandleActorAttAndRelChange(IntermediatorID, AttAndRelRcpt, AttChange, RelChange);
						}
					}
				}
			}
		}

		// Anything after CutoffIdx (if any) will be your dialogue items like:
		// {
		//   "intermediatorID": "Alex",
		//   "RecipientID": "...",
		//   "intermediatorStatus": "...",
		//   "RecipientStatus": "...",
		//   "intermediatorDialogue": "...",
		//   "RecipientDialogue": "..."
		// }
		// Process those as you already do in your dialogue handling loop.
	}

	
    // --- 2) Process the rest of the entries exactly as before ---
    for (int32 Index = CutoffIdx+1; Index < JsonArray.Num(); ++Index)
    {
        const TSharedPtr<FJsonObject>& Obj = JsonArray[Index]->AsObject();
        FParsedCEInfoCPP Instruction;
        if (FJsonObjectConverter::JsonObjectToUStruct(
                Obj.ToSharedRef(),
                FParsedCEInfoCPP::StaticStruct(),
                &Instruction, /*Flags=*/0, /*MaxDepth=*/0))
        {
            ForwardInstructionToGameMode(Instruction);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("HandleMessage: failed to convert JSON to FParsedCEInfoCPP at index %d"), Index);
        }
    }
}

void UExternalCEHandleComp::ForwardInstructionToGameMode(const FParsedCEInfoCPP& Instruction)
{

	AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
	if (GM)
	{
		// Call a BlueprintImplementableEvent or custom dispatcher
		AMyGameModeBaseCpp* GameModeBP = Cast<AMyGameModeBaseCpp>(GM);
		if (GameModeBP)
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,                    // Key (-1 means create a new message)
					5.f,                   // Duration in seconds
					FColor::Yellow,        // Text color
					TEXT("Got it") // Message
				);
			}
			UE_LOG(LogTemp, Warning, TEXT("Forwarding Instruction: Sender=%s, Receiver=%s"),
			*Instruction.IntermediatorID.ToString(),
			*Instruction.RecipientID.ToString());
			
			GameModeBP->TryDispatchEventFromInstruction(Instruction); // You implement this in BP
		}
	}
}