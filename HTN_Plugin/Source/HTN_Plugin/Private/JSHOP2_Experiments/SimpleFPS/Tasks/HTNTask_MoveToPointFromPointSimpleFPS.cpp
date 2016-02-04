#include "HTN_PluginPrivatePCH.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MoveToPointFromPointSimpleFPS.h"

UHTNTask_MoveToPointFromPointSimpleFPS::UHTNTask_MoveToPointFromPointSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Move To Point From Point";
	HeuristicCost = 1.f;
}

void UHTNTask_MoveToPointFromPointSimpleFPS::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_MoveToPointFromPointSimpleFPSMemory* Memory = (FHTNTask_MoveToPointFromPointSimpleFPSMemory*)TaskMemory;

		// move the NPC close to the point of interest
		SimpleFPSWorldState->NPCNearbyPOI = Memory->PointOfInterest;
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_MoveToPointFromPointSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}
}

EHTNExecutionResult UHTNTask_MoveToPointFromPointSimpleFPS::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	FHTNTask_MoveToPointFromPointSimpleFPSMemory* Memory = (FHTNTask_MoveToPointFromPointSimpleFPSMemory*)TaskMemory;
	UE_LOG(LogHTNPlanner, Warning, TEXT("[Move To Point: %s (from Point: %s)]"), 
		   *(Memory->PointOfInterest->ObjectName), *(Memory->Previous->ObjectName));
	return EHTNExecutionResult::Succeeded;
}

float UHTNTask_MoveToPointFromPointSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_MoveToPointFromPointSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_MoveToPointFromPointSimpleFPSMemory);
}

bool UHTNTask_MoveToPointFromPointSimpleFPS::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_MoveToPointFromPointSimpleFPSMemory* Memory = (FHTNTask_MoveToPointFromPointSimpleFPSMemory*)TaskMemory;
		return (SimpleFPSWorldState->NPCArea == Memory->Area					&&
				SimpleFPSWorldState->IsInArea(Memory->PointOfInterest, Memory->Area) &&
				SimpleFPSWorldState->IsInArea(Memory->Previous, Memory->Area) &&
				SimpleFPSWorldState->NPCNearbyPOI == Memory->Previous);
	}
	else
	{
		return false;
	}
}