#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/TaskNetwork.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MovingSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MovingToPatrolSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MovingToTakePositionSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_UncoverCompoundSimpleFPS.h"

UHTNTask_MovingSimpleFPS::UHTNTask_MovingSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Moving";

	// we'll always have an uncover task and either a patrol or a take position task
	float UncoverHeuristic = UHTNTask_UncoverCompoundSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_UncoverCompoundSimpleFPS>()->GetHeuristicCost();
	HeuristicCost = FMath::Min
		(
			UncoverHeuristic +
			UHTNTask_MovingToPatrolSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_MovingToPatrolSimpleFPS>()->GetHeuristicCost()
			,
			UncoverHeuristic +
			UHTNTask_MovingToTakePositionSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_MovingToTakePositionSimpleFPS>()->GetHeuristicCost()
		);
}

TArray<TSharedPtr<FHTNTaskInstance>> UHTNTask_MovingSimpleFPS::FindDecompositions(UHTNPlannerComponent& HTNComp, 
																				  const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory)
{
	TArray<TSharedPtr<FHTNTaskInstance>> Decompositions;

	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_MovingSimpleFPSMemory* Memory = (FHTNTask_MovingSimpleFPSMemory*)TaskMemory;

		if(!SimpleFPSWorldState->bNPCAware)
		{
			// first branch; NPC not aware, so should patrol
			// create decomposition
			TSharedPtr<FHTNTaskInstance> Decomposition = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());

			// create task to get out of cover
			TSharedPtr<FHTNTaskInstance> UncoverTask = HTNComp.InstantiateTask(UHTNTask_UncoverCompoundSimpleFPS::StaticClass());

			// create task to patrol
			TSharedPtr<FHTNTaskInstance> PatrolTask = HTNComp.InstantiateTask(UHTNTask_MovingToPatrolSimpleFPS::StaticClass());
			FHTNTask_MovingToPatrolSimpleFPSMemory* PatrolMemory = (FHTNTask_MovingToPatrolSimpleFPSMemory*)PatrolTask->GetMemory();
			PatrolMemory->FromArea = Memory->FromArea;
			PatrolMemory->ToArea = Memory->ToArea;
			PatrolMemory->Waypoint = Memory->Waypoint;

			// finalize and add the decomposition
			UTaskNetwork* DecompositionNetwork = Cast<UTaskNetwork>(Decomposition->Task);
			DecompositionNetwork->AddTaskOrSubNetwork(UncoverTask, Decomposition->GetMemory());
			DecompositionNetwork->AddTaskOrSubNetwork(PatrolTask, Decomposition->GetMemory());
			Decompositions.Add(Decomposition);
		}
		else
		{
			// second branch; NPC aware, so should move to take position
			// create decomposition
			TSharedPtr<FHTNTaskInstance> Decomposition = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());

			// create task to get out of cover
			TSharedPtr<FHTNTaskInstance> UncoverTask = HTNComp.InstantiateTask(UHTNTask_UncoverCompoundSimpleFPS::StaticClass());

			// create task to take position
			TSharedPtr<FHTNTaskInstance> TakePositionTask = HTNComp.InstantiateTask(UHTNTask_MovingToTakePositionSimpleFPS::StaticClass());
			FHTNTask_MovingToTakePositionSimpleFPSMemory* TakePositionMemory = 
				(FHTNTask_MovingToTakePositionSimpleFPSMemory*)TakePositionTask->GetMemory();
			TakePositionMemory->FromArea = Memory->FromArea;
			TakePositionMemory->ToArea = Memory->ToArea;
			TakePositionMemory->Waypoint = Memory->Waypoint;

			// finalize and add the decomposition
			UTaskNetwork* DecompositionNetwork = Cast<UTaskNetwork>(Decomposition->Task);
			DecompositionNetwork->AddTaskOrSubNetwork(UncoverTask, Decomposition->GetMemory());
			DecompositionNetwork->AddTaskOrSubNetwork(TakePositionTask, Decomposition->GetMemory());
			Decompositions.Add(Decomposition);
		}
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_MovingSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}

	return Decompositions;
}

float UHTNTask_MovingSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_MovingSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_MovingSimpleFPSMemory);
}