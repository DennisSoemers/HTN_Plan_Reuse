#include "HTN_PluginPrivatePCH.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "HTNPlanner/HTNWorldState.h"
#include "HTNPlanner/PrimitiveTask.h"

void FHTNWorldState::ApplyTask(const UPrimitiveTask* Task, uint8* TaskMemory)
{
	UE_LOG(LogHTNPlanner, Warning, TEXT("FHTNWorldState::ApplyTask() cannot handle the given Task!"))
}

bool FHTNWorldState::CanApply(const UPrimitiveTask* Task, uint8* TaskMemory) const
{
	UE_LOG(LogHTNPlanner, Warning, TEXT("FHTNWorldState::CanApply() cannot handle the given Task!"))
	return false;
}

TSharedPtr<FHTNWorldState> FHTNWorldState::Copy() const
{
	UE_LOG(LogHTNPlanner, Warning, TEXT("FHTNWorldState::Copy() should not be called on the base class!"))
	return nullptr;
}

float FHTNWorldState::GetHeuristicCost(const TArray<TSharedPtr<FHTNTaskInstance>>& TaskInstances) const
{
	return 0.f;
}

FName FHTNWorldState::GetWorldStateName() const
{
	return "HTN World State";
}

void FHTNWorldState::Initialize(AActor* Owner, UBlackboardComponent* BlackboardComponent)
{
	// no default initialization
}