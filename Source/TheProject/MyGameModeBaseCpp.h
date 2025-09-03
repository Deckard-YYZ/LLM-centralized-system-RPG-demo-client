// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyGameModeBaseCpp.generated.h"

/**
 * 
 */
UCLASS()
class THEPROJECT_API AMyGameModeBaseCpp : public AGameModeBase
{
	GENERATED_BODY()
	public:
	UFUNCTION(BlueprintImplementableEvent, Category="SearchMe")
	void TryDispatchEventFromInstruction(FParsedCEInfoCPP Instruction);

	UFUNCTION(BlueprintImplementableEvent)
	void HandleStarterChange(const FString& StarterID, const FString& IntermediatorID, const FString& NewAttitude, const FString& NewRelationship);

	UFUNCTION(BlueprintImplementableEvent)
	void HandleActorAttAndRelChange(const FString& IntermediatorID, const FString& AttAndRelRCPT, const FString& NewAttitude, const FString& NewRelationship);
};
