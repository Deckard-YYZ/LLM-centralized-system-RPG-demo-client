// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "UMyBTTask_MoveToSetUnset.generated.h"
/**
 * 
 */
UCLASS()
class UMyBTTask_MoveToSetUnset: public UBTTask_MoveTo
{
	GENERATED_BODY()

public:
	UMyBTTask_MoveToSetUnset();

	// Expose a Blackboard key to Blueprint BT editor
	UPROPERTY(EditAnywhere, Category="Blackboard")
	struct FBlackboardKeySelector BoolKeyToSet;

	// // The value you want to set
	// UPROPERTY(EditAnywhere, Category="Blackboard")
	// bool NewValue = true;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	
	
};
