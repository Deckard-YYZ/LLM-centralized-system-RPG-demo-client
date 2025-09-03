#pragma once

#include "CoreMinimal.h"
#include "FToSendPreCEInfo.generated.h"

// Holds info about a single NPC
USTRUCT(BlueprintType)
struct FNPCInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Attitude;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RelationshipType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Settings;
};

// Main struct to send
USTRUCT(BlueprintType)
struct FToSendPreCEInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString StarterAndAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName IntermediatorAndSetting;
	// Key = NPCID, Value = FNPCInfo, ToWhom's knowing NPCs
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FNPCInfo> NpcInfoMap;
};