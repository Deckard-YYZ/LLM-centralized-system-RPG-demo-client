#include "UMyBTTask_MoveToSetUnset.h"
UMyBTTask_MoveToSetUnset::UMyBTTask_MoveToSetUnset()
{
	NodeName = "MoveTo Set Unset";
}

EBTNodeResult::Type UMyBTTask_MoveToSetUnset::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	if (BBComp)
	{
		BBComp->SetValueAsBool(BoolKeyToSet.SelectedKeyName, true);
	}
	// Call the parent class to actually perform the MoveTo logic
	return Super::ExecuteTask(OwnerComp, NodeMemory);
}

// Called when the task (MoveTo) finishes (success or fail)
void UMyBTTask_MoveToSetUnset::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	if (BBComp)
	{
		BBComp->SetValueAsBool(BoolKeyToSet.SelectedKeyName, false);
	}
	// Call parent to finish up any logic
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}