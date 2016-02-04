#include "HTN_PluginPrivatePCH.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_UnexploreSimpleFPS.h"

UHTNTask_UnexploreSimpleFPS::UHTNTask_UnexploreSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Unexplore";
	HeuristicCost = 0.f;
}

void UHTNTask_UnexploreSimpleFPS::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_UnexploreSimpleFPSMemory* Memory = (FHTNTask_UnexploreSimpleFPSMemory*)TaskMemory;
		SimpleFPSWorldState->AreaData[Memory->Area].bExplored = false;
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_UnexploreSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}
}

EHTNExecutionResult UHTNTask_UnexploreSimpleFPS::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	FHTNTask_UnexploreSimpleFPSMemory* Memory = (FHTNTask_UnexploreSimpleFPSMemory*)TaskMemory;
	UE_LOG(LogHTNPlanner, Warning, TEXT("[!!Unexplore: Area %d]"), Memory->Area);
	return EHTNExecutionResult::Succeeded;
}

float UHTNTask_UnexploreSimpleFPS::GetCost(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	return 0.f;
}

uint16 UHTNTask_UnexploreSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_UnexploreSimpleFPSMemory);
}

bool UHTNTask_UnexploreSimpleFPS::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	return true;
}