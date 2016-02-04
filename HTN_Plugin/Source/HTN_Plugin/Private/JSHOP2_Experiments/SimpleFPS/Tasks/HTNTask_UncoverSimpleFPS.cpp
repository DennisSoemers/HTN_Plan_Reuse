#include "HTN_PluginPrivatePCH.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_UncoverSimpleFPS.h"

UHTNTask_UncoverSimpleFPS::UHTNTask_UncoverSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Uncover";
	HeuristicCost = 1.f;
}

void UHTNTask_UncoverSimpleFPS::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		// NPC will no longer be covered
		SimpleFPSWorldState->bNPCCovered = false;

		// NPC will no longer be near the cover point
		SimpleFPSWorldState->NPCNearbyPOI = nullptr;
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_UncoverSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}
}

EHTNExecutionResult UHTNTask_UncoverSimpleFPS::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	FHTNTask_UncoverSimpleFPSMemory* Memory = (FHTNTask_UncoverSimpleFPSMemory*)TaskMemory;
	UE_LOG(LogHTNPlanner, Warning, TEXT("[Uncover from: %s]"), *(Memory->CoverPoint->ObjectName));
	return EHTNExecutionResult::Succeeded;
}

float UHTNTask_UncoverSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_UncoverSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_UncoverSimpleFPSMemory);
}

bool UHTNTask_UncoverSimpleFPS::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_UncoverSimpleFPSMemory* Memory = (FHTNTask_UncoverSimpleFPSMemory*)TaskMemory;
		return (SimpleFPSWorldState->bNPCCovered	&&
				SimpleFPSWorldState->NPCNearbyPOI == Memory->CoverPoint);
	}
	else
	{
		return false;
	}
}