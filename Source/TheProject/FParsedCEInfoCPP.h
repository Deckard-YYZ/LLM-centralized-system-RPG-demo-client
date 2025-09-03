#pragma once

#include "CoreMinimal.h"
#include "FParsedCEInfoCPP.generated.h"  // Required for USTRUCT

USTRUCT(BlueprintType)
struct FParsedCEInfoCPP
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FName  IntermediatorID;
	
	UPROPERTY(BlueprintReadWrite)
	FName RecipientID;

	UPROPERTY(BlueprintReadWrite)
	FString IntermediatorStatus;

	UPROPERTY(BlueprintReadWrite)
	FString RecipientStatus;

	UPROPERTY(BlueprintReadWrite)
	FString IntermediatorDialogue;

	UPROPERTY(BlueprintReadWrite)
	FString RecipientDialogue;

	UPROPERTY(BlueprintReadWrite)
	FString ItoR_AttitudeChangeTo;

	UPROPERTY(BlueprintReadWrite)
	FString ItoR_RelTypeChangeTo;

	UPROPERTY(BlueprintReadWrite)
	FString ItoS_RelTypeChangeTo;

	UPROPERTY(BlueprintReadWrite)
	FString ItoS_AttitudeChangeTo;
};
