#include "HTN_PluginPrivatePCH.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MovingToPatrolSimpleFPS.h"

UHTNTask_MovingToPatrolSimpleFPS::UHTNTask_MovingToPatrolSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Moving To Patrol";
	HeuristicCost = 1.f;
}

void UHTNTask_MovingToPatrolSimpleFPS::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_MovingToPatrolSimpleFPSMemory* Memory = (FHTNTask_MovingToPatrolSimpleFPSMemory*)TaskMemory;

		// change the NPC's area
		SimpleFPSWorldState->NPCArea = Memory->ToArea;

		// move the NPC away from waypoint
		SimpleFPSWorldState->NPCNearbyPOI = nullptr;
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_MovingToPatrolSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}
}

EHTNExecutionResult UHTNTask_MovingToPatrolSimpleFPS::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	FHTNTask_MovingToPatrolSimpleFPSMemory* Memory = (FHTNTask_MovingToPatrolSimpleFPSMemory*)TaskMemory;
	UE_LOG(LogHTNPlanner, Warning, TEXT("[Moving To Patrol from Area %d to Area %d]"), Memory->FromArea, Memory->ToArea);
	return EHTNExecutionResult::Succeeded;
}

float UHTNTask_MovingToPatrolSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_MovingToPatrolSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_MovingToPatrolSimpleFPSMemory);
}

bool UHTNTask_MovingToPatrolSimpleFPS::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_MovingToPatrolSimpleFPSMemory* Memory = (FHTNTask_MovingToPatrolSimpleFPSMemory*)TaskMemory;
		const FSimpleFPSWaypointData& WaypointData = SimpleFPSWorldState->WaypointData[Memory->Waypoint->Index];

		return (((WaypointData.Area1 == Memory->FromArea && WaypointData.Area2 == Memory->ToArea)	||
				(WaypointData.Area1 == Memory->ToArea && WaypointData.Area2 == Memory->FromArea))		&&
				SimpleFPSWorldState->NPCArea == Memory->FromArea										&&
				!SimpleFPSWorldState->IsLocked(Memory->Waypoint)										&&
				!SimpleFPSWorldState->bNPCCovered														&&
				!SimpleFPSWorldState->bNPCAware															&&
				SimpleFPSWorldState->NPCNearbyPOI == Memory->Waypoint);
	}
	else
	{
		return false;
	}
}