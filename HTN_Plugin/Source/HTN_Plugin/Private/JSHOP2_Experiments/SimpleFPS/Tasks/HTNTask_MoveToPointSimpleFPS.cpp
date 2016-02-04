#include "HTN_PluginPrivatePCH.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MoveToPointSimpleFPS.h"

UHTNTask_MoveToPointSimpleFPS::UHTNTask_MoveToPointSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Move To Point";
	HeuristicCost = 1.f;
}

void UHTNTask_MoveToPointSimpleFPS::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_MoveToPointSimpleFPSMemory* Memory = (FHTNTask_MoveToPointSimpleFPSMemory*)TaskMemory;

		// move the NPC close to the point of interest
		SimpleFPSWorldState->NPCNearbyPOI = Memory->PointOfInterest;
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_MoveToPointSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}
}

EHTNExecutionResult UHTNTask_MoveToPointSimpleFPS::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	FHTNTask_MoveToPointSimpleFPSMemory* Memory = (FHTNTask_MoveToPointSimpleFPSMemory*)TaskMemory;
	UE_LOG(LogHTNPlanner, Warning, TEXT("[Move To Point: %s]"), *(Memory->PointOfInterest->ObjectName));
	return EHTNExecutionResult::Succeeded;
}

float UHTNTask_MoveToPointSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_MoveToPointSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_MoveToPointSimpleFPSMemory);
}

bool UHTNTask_MoveToPointSimpleFPS::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_MoveToPointSimpleFPSMemory* Memory = (FHTNTask_MoveToPointSimpleFPSMemory*)TaskMemory;
		return (SimpleFPSWorldState->NPCArea == Memory->Area							&&
				SimpleFPSWorldState->IsInArea(Memory->PointOfInterest, Memory->Area)	&&
				SimpleFPSWorldState->NPCNearbyPOI == nullptr								);
	}
	else
	{
		return false;
	}
}