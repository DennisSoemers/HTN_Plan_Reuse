#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/TaskNetwork.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MoveToPointSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MoveToPointCompoundSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MoveToPointFromPointSimpleFPS.h"

UHTNTask_MoveToPointCompoundSimpleFPS::UHTNTask_MoveToPointCompoundSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Move To Point (Compound)";

	// we'll always have a single MoveToPoint task or a single MoveToPointFromPoint task
	HeuristicCost = FMath::Min(UHTNTask_MoveToPointSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_MoveToPointSimpleFPS>()->GetHeuristicCost(),
							   UHTNTask_MoveToPointFromPointSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_MoveToPointFromPointSimpleFPS>()->GetHeuristicCost());
}

TArray<TSharedPtr<FHTNTaskInstance>> UHTNTask_MoveToPointCompoundSimpleFPS::FindDecompositions(UHTNPlannerComponent& HTNComp, 
																							   const TSharedPtr<FHTNWorldState> WorldState, 
																							   uint8* TaskMemory)
{
	TArray<TSharedPtr<FHTNTaskInstance>> Decompositions;

	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_MoveToPointCompoundSimpleFPSMemory* Memory = (FHTNTask_MoveToPointCompoundSimpleFPSMemory*)TaskMemory;

		if(Memory->Area == SimpleFPSWorldState->NPCArea)
		{
			// both branches need us to be in the correct area
			if(SimpleFPSWorldState->NPCNearbyPOI != nullptr)
			{
				// we're near a point of interest, so need to move from it
				// create decomposition
				TSharedPtr<FHTNTaskInstance> Decomposition = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());

				// create task to move from point
				TSharedPtr<FHTNTaskInstance> MoveFromPointTask = HTNComp.InstantiateTask(UHTNTask_MoveToPointFromPointSimpleFPS::StaticClass());
				FHTNTask_MoveToPointFromPointSimpleFPSMemory* MoveFromPointMemory = 
					(FHTNTask_MoveToPointFromPointSimpleFPSMemory*)MoveFromPointTask->GetMemory();
				MoveFromPointMemory->Area = Memory->Area;
				MoveFromPointMemory->PointOfInterest = Memory->PointOfInterest;
				MoveFromPointMemory->Previous = SimpleFPSWorldState->NPCNearbyPOI;

				// finalize and add Decomposition
				UTaskNetwork* DecompositionNetwork = Cast<UTaskNetwork>(Decomposition->Task);
				DecompositionNetwork->AddTaskOrSubNetwork(MoveFromPointTask, Decomposition->GetMemory());
				Decompositions.Add(Decomposition);
			}
			else
			{
				// we're in the area but not near any POI, so simply move to it
				TSharedPtr<FHTNTaskInstance> Decomposition = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());

				// create task to move to point
				TSharedPtr<FHTNTaskInstance> MoveToPointTask = HTNComp.InstantiateTask(UHTNTask_MoveToPointSimpleFPS::StaticClass());
				FHTNTask_MoveToPointSimpleFPSMemory* MoveToPointMemory = (FHTNTask_MoveToPointSimpleFPSMemory*)MoveToPointTask->GetMemory();
				MoveToPointMemory->Area = Memory->Area;
				MoveToPointMemory->PointOfInterest = Memory->PointOfInterest;

				// finalize and add Decomposition
				UTaskNetwork* DecompositionNetwork = Cast<UTaskNetwork>(Decomposition->Task);
				DecompositionNetwork->AddTaskOrSubNetwork(MoveToPointTask, Decomposition->GetMemory());
				Decompositions.Add(Decomposition);
			}
		}
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_MoveToPointCompoundSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}

	return Decompositions;
}

float UHTNTask_MoveToPointCompoundSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_MoveToPointCompoundSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_MoveToPointCompoundSimpleFPSMemory);
}