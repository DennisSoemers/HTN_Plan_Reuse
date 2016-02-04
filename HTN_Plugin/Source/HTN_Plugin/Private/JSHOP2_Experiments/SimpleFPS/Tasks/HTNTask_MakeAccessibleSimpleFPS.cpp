#include "HTN_PluginPrivatePCH.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MakeAccessibleSimpleFPS.h"

UHTNTask_MakeAccessibleSimpleFPS::UHTNTask_MakeAccessibleSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Make Accessible";
	HeuristicCost = 1.f;
}

void UHTNTask_MakeAccessibleSimpleFPS::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_MakeAccessibleSimpleFPSMemory* Memory = (FHTNTask_MakeAccessibleSimpleFPSMemory*)TaskMemory;

		// open the waypoint
		SimpleFPSWorldState->WaypointData[Memory->Waypoint->Index].bOpen = true;

		// remove the keycard from inventory
		SimpleFPSWorldState->NPCInventory.RemoveSingleSwap(Memory->Keycard);
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_MakeAccessibleSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}
}

EHTNExecutionResult UHTNTask_MakeAccessibleSimpleFPS::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	FHTNTask_MakeAccessibleSimpleFPSMemory* Memory = (FHTNTask_MakeAccessibleSimpleFPSMemory*)TaskMemory;
	UE_LOG(LogHTNPlanner, Warning, TEXT("[Make Accessible: %s]"), *(Memory->Waypoint->ObjectName));
	return EHTNExecutionResult::Succeeded;
}

float UHTNTask_MakeAccessibleSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_MakeAccessibleSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_MakeAccessibleSimpleFPSMemory);
}

bool UHTNTask_MakeAccessibleSimpleFPS::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_MakeAccessibleSimpleFPSMemory* Memory = (FHTNTask_MakeAccessibleSimpleFPSMemory*)TaskMemory;
		const FSimpleFPSWaypointData& WaypointData = SimpleFPSWorldState->WaypointData[Memory->Waypoint->Index];
		int32 NPCArea = SimpleFPSWorldState->NPCArea;

		return (SimpleFPSWorldState->NPCInventory.Contains(Memory->Keycard) &&
				WaypointData.KeycardIndex == Memory->Keycard->Index							&&
				NPCArea == Memory->FromArea &&
				((WaypointData.Area1 == Memory->FromArea && WaypointData.Area2 == Memory->ToArea) ||
				(WaypointData.Area1 == Memory->ToArea && WaypointData.Area2 == Memory->FromArea)) &&
				!WaypointData.bOpen													&&
				SimpleFPSWorldState->NPCNearbyPOI == Memory->Waypoint);
	}
	else
	{
		return false;
	}
}