#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/TaskNetwork.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_ExploreSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_GetItemSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MakeAccessibleSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MoveToAreaSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MoveToPointCompoundSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MovingSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_UnexploreSimpleFPS.h"

UHTNTask_MoveToAreaSimpleFPS::UHTNTask_MoveToAreaSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Move To Area";

	// this task can sometimes be decomposed into an empty network, so heuristic of 0
	HeuristicCost = 0.f;
}

TArray<TSharedPtr<FHTNTaskInstance>> UHTNTask_MoveToAreaSimpleFPS::FindDecompositions(UHTNPlannerComponent& HTNComp, 
																					  const TSharedPtr<FHTNWorldState> WorldState, 
																					  uint8* TaskMemory)
{
	TArray<TSharedPtr<FHTNTaskInstance>> Decompositions;

	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_MoveToAreaSimpleFPSMemory* Memory = (FHTNTask_MoveToAreaSimpleFPSMemory*)TaskMemory;

		int32 NPCArea = SimpleFPSWorldState->NPCArea;
		if(Memory->Area == NPCArea)
		{
			// we're already there, so return empty network
			Decompositions.Add(HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass()));
		}
		else if(SimpleFPSWorldState->IsReachable(NPCArea, Memory->Area))	// we need the target area to be reachable
		{
			// collect waypoints that connect the goal area to some other area that we didn't explore yet
			const TArray<FSimpleFPSObject*>& WorldStateWaypoints = SimpleFPSWorldState->GetWaypoints();
			TArray<FSimpleFPSObject*> Waypoints;
			Waypoints.Reserve(WorldStateWaypoints.Num());

			for(FSimpleFPSObject* Waypoint : WorldStateWaypoints)
			{
				const FSimpleFPSWaypointData& WaypointData = SimpleFPSWorldState->WaypointData[Waypoint->Index];
				if(WaypointData.Area1 == Memory->Area && !SimpleFPSWorldState->AreaData[WaypointData.Area2].bExplored)
				{
					Waypoints.Add(Waypoint);
				}
				else if(WaypointData.Area2 == Memory->Area && !SimpleFPSWorldState->AreaData[WaypointData.Area1].bExplored)
				{
					Waypoints.Add(Waypoint);
				}
			}

			// sort the waypoints (first all open doors, then closed doors, and within each of those sets
			// place the closer waypoints before waypoints further away).
			const TArray<int32>& DistanceRow = SimpleFPSWorldState->GetDistanceRow(NPCArea);
			Waypoints.Sort([&](const FSimpleFPSObject& A, const FSimpleFPSObject& B)
			{
				const FSimpleFPSWaypointData& WaypointDataA = SimpleFPSWorldState->WaypointData[A.Index];
				const FSimpleFPSWaypointData& WaypointDataB = SimpleFPSWorldState->WaypointData[B.Index];

				if(!SimpleFPSWorldState->IsLocked(&A) && SimpleFPSWorldState->IsLocked(&B))
				{
					return true;
				}
				else if(SimpleFPSWorldState->IsLocked(&A) && !SimpleFPSWorldState->IsLocked(&B))
				{
					return false;
				}

				int32 AreaA = (WaypointDataA.Area1 == Memory->Area) ? WaypointDataA.Area2 : WaypointDataA.Area1;
				int32 AreaB = (WaypointDataB.Area1 == Memory->Area) ? WaypointDataB.Area2 : WaypointDataB.Area1;
				return (DistanceRow[AreaA] < DistanceRow[AreaB]);
			});

			// in the order obtained by sorting, try reaching the target area through the waypoints
			for(int32 Idx = 0; Idx < Waypoints.Num(); ++Idx)
			{
				FSimpleFPSObject* Waypoint = Waypoints[Idx];
				const FSimpleFPSWaypointData& WaypointData = SimpleFPSWorldState->WaypointData[Waypoint->Index];
				int32 FromArea = (WaypointData.Area1 == Memory->Area) ? WaypointData.Area2 : WaypointData.Area1;

				// create decomposition
				TSharedPtr<FHTNTaskInstance> Decomposition = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());

				// create task to mark that we've explored the goal area
				TSharedPtr<FHTNTaskInstance> ExploreTask = HTNComp.InstantiateTask(UHTNTask_ExploreSimpleFPS::StaticClass());
				FHTNTask_ExploreSimpleFPSMemory* ExploreMemory = (FHTNTask_ExploreSimpleFPSMemory*)ExploreTask->GetMemory();
				ExploreMemory->Area = FromArea;

				// create task to move to the FromArea
				TSharedPtr<FHTNTaskInstance> MoveToAreaTask = HTNComp.InstantiateTask(UHTNTask_MoveToAreaSimpleFPS::StaticClass());
				FHTNTask_MoveToAreaSimpleFPSMemory* MoveToAreaMemory = (FHTNTask_MoveToAreaSimpleFPSMemory*)MoveToAreaTask->GetMemory();
				MoveToAreaMemory->Area = FromArea;

				// create task to move to the Waypoint
				TSharedPtr<FHTNTaskInstance> MoveToPointTask = HTNComp.InstantiateTask(UHTNTask_MoveToPointCompoundSimpleFPS::StaticClass());
				FHTNTask_MoveToPointCompoundSimpleFPSMemory* MoveToPointMemory = 
					(FHTNTask_MoveToPointCompoundSimpleFPSMemory*)MoveToPointTask->GetMemory();
				MoveToPointMemory->Area = FromArea;
				MoveToPointMemory->PointOfInterest = Waypoint;

				// create single-step moving task
				TSharedPtr<FHTNTaskInstance> MovingTask = HTNComp.InstantiateTask(UHTNTask_MovingSimpleFPS::StaticClass());
				FHTNTask_MovingSimpleFPSMemory* MovingMemory = (FHTNTask_MovingSimpleFPSMemory*)MovingTask->GetMemory();
				MovingMemory->FromArea = FromArea;
				MovingMemory->ToArea = Memory->Area;
				MovingMemory->Waypoint = Waypoint;

				// create task to remove the mark that we've explored the goal area
				TSharedPtr<FHTNTaskInstance> UnexploreTask = HTNComp.InstantiateTask(UHTNTask_UnexploreSimpleFPS::StaticClass());
				FHTNTask_UnexploreSimpleFPSMemory* UnexploreMemory = (FHTNTask_UnexploreSimpleFPSMemory*)UnexploreTask->GetMemory();
				UnexploreMemory->Area = FromArea;

				if(!SimpleFPSWorldState->IsLocked(Waypoint))
				{
					// waypoint already open, so only need the tasks created above
					UTaskNetwork* DecompositionNetwork = Cast<UTaskNetwork>(Decomposition->Task);
					DecompositionNetwork->AddTaskOrSubNetwork(ExploreTask, Decomposition->GetMemory());
					DecompositionNetwork->AddTaskOrSubNetwork(MoveToAreaTask, Decomposition->GetMemory());
					DecompositionNetwork->AddTaskOrSubNetwork(MoveToPointTask, Decomposition->GetMemory());
					DecompositionNetwork->AddTaskOrSubNetwork(MovingTask, Decomposition->GetMemory());
					DecompositionNetwork->AddTaskOrSubNetwork(UnexploreTask, Decomposition->GetMemory());
				}
				else
				{
					// waypoint is closed, so need to create some extra tasks in between to find keycard
					FSimpleFPSObject* Keycard = SimpleFPSWorldState->GetKeycards()[WaypointData.KeycardIndex];

					// create task to get the keycard
					TSharedPtr<FHTNTaskInstance> GetItemTask = HTNComp.InstantiateTask(UHTNTask_GetItemSimpleFPS::StaticClass());
					FHTNTask_GetItemSimpleFPSMemory* GetItemMemory = (FHTNTask_GetItemSimpleFPSMemory*)GetItemTask->GetMemory();
					GetItemMemory->Item = Keycard;

					// create task to make the waypoint accessible
					TSharedPtr<FHTNTaskInstance> MakeAccessibleTask = HTNComp.InstantiateTask(UHTNTask_MakeAccessibleSimpleFPS::StaticClass());
					FHTNTask_MakeAccessibleSimpleFPSMemory* MakeAccessibleMemory = 
						(FHTNTask_MakeAccessibleSimpleFPSMemory*)MakeAccessibleTask->GetMemory();
					MakeAccessibleMemory->FromArea = FromArea;
					MakeAccessibleMemory->ToArea = Memory->Area;
					MakeAccessibleMemory->Waypoint = Waypoint;
					MakeAccessibleMemory->Keycard = Keycard;

					// add tasks to decomposition
					UTaskNetwork* DecompositionNetwork = Cast<UTaskNetwork>(Decomposition->Task);
					DecompositionNetwork->AddTaskOrSubNetwork(ExploreTask, Decomposition->GetMemory());
					DecompositionNetwork->AddTaskOrSubNetwork(GetItemTask, Decomposition->GetMemory());
					DecompositionNetwork->AddTaskOrSubNetwork(MoveToAreaTask, Decomposition->GetMemory());
					DecompositionNetwork->AddTaskOrSubNetwork(MoveToPointTask, Decomposition->GetMemory());
					DecompositionNetwork->AddTaskOrSubNetwork(MakeAccessibleTask, Decomposition->GetMemory());
					DecompositionNetwork->AddTaskOrSubNetwork(MovingTask, Decomposition->GetMemory());
					DecompositionNetwork->AddTaskOrSubNetwork(UnexploreTask, Decomposition->GetMemory());
				}

				// add the Decomposition
				Decompositions.Add(Decomposition);
			}
		}
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_MoveToAreaSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}

	return Decompositions;
}

float UHTNTask_MoveToAreaSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_MoveToAreaSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_MoveToAreaSimpleFPSMemory);
}