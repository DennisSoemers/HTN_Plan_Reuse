#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/Tasks/HTNTask_Wait.h"

UHTNTask_Wait::UHTNTask_Wait(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	TaskName = "Wait";
	WaitTime = 5.0f;
}

void UHTNTask_Wait::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	// no effects
}

EHTNExecutionResult UHTNTask_Wait::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	FHTNTask_WaitMemory* Memory = (FHTNTask_WaitMemory*)TaskMemory;
	Memory->RemainingWaitTime = FMath::FRandRange(FMath::Max(0.0f, WaitTime - RandomDeviation), (WaitTime + RandomDeviation));
	GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, FString::Printf(TEXT("Initial Wait Time = %.2f"), Memory->RemainingWaitTime));

	return EHTNExecutionResult::InProgress;
}

float UHTNTask_Wait::GetCost(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	return 0.f;
}

uint16 UHTNTask_Wait::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_WaitMemory);
}

bool UHTNTask_Wait::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	return true;
}

void UHTNTask_Wait::TickTask(UHTNPlannerComponent& HTNComp, float DeltaSeconds, uint8* TaskMemory)
{
	FHTNTask_WaitMemory* Memory = (FHTNTask_WaitMemory*)TaskMemory;
	Memory->RemainingWaitTime -= DeltaSeconds;

	if(Memory->RemainingWaitTime <= 0.0f)
	{
		// finished waiting
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, TEXT("Wait Task finishing Latent Task from TickTask()!"));
		FinishLatentTask(HTNComp, EHTNExecutionResult::Succeeded, TaskMemory);
	}
}