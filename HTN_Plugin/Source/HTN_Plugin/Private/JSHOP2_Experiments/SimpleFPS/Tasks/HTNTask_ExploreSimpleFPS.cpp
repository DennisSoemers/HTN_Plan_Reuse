#include "HTN_PluginPrivatePCH.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_ExploreSimpleFPS.h"

UHTNTask_ExploreSimpleFPS::UHTNTask_ExploreSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Explore";
	HeuristicCost = 0.f;
}

void UHTNTask_ExploreSimpleFPS::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_ExploreSimpleFPSMemory* Memory = (FHTNTask_ExploreSimpleFPSMemory*)TaskMemory;
		SimpleFPSWorldState->AreaData[Memory->Area].bExplored = true;
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_ExploreSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}
}

EHTNExecutionResult UHTNTask_ExploreSimpleFPS::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	FHTNTask_ExploreSimpleFPSMemory* Memory = (FHTNTask_ExploreSimpleFPSMemory*)TaskMemory;
	UE_LOG(LogHTNPlanner, Warning, TEXT("[!!Explore: Area %d]"), Memory->Area);
	return EHTNExecutionResult::Succeeded;
}

float UHTNTask_ExploreSimpleFPS::GetCost(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	return 0.f;
}

uint16 UHTNTask_ExploreSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_ExploreSimpleFPSMemory);
}

FString UHTNTask_ExploreSimpleFPS::GetTaskName(uint8* TaskMemory) const
{
	if(TaskMemory)
	{
		FHTNTask_ExploreSimpleFPSMemory* Memory = (FHTNTask_ExploreSimpleFPSMemory*)TaskMemory;
		return FString::Printf(TEXT("Explore Area %d"), Memory->Area);
	}
	else
	{
		return TaskName;
	}
}

bool UHTNTask_ExploreSimpleFPS::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	return true;
}