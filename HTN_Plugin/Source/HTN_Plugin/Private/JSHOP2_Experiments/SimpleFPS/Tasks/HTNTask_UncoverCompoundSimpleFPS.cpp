#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/TaskNetwork.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_UncoverSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_UncoverCompoundSimpleFPS.h"

UHTNTask_UncoverCompoundSimpleFPS::UHTNTask_UncoverCompoundSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Uncover (Compound)";

	// we'll sometimes return an empty network, so heuristic cost of 0
	HeuristicCost = 0.f;
}

TArray<TSharedPtr<FHTNTaskInstance>> UHTNTask_UncoverCompoundSimpleFPS::FindDecompositions(UHTNPlannerComponent& HTNComp,
																						   const TSharedPtr<FHTNWorldState> WorldState, 
																						   uint8* TaskMemory)
{
	TArray<TSharedPtr<FHTNTaskInstance>> Decompositions;

	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		if(!SimpleFPSWorldState->bNPCCovered)
		{
			// we're already uncovered, so return empty network
			Decompositions.Add(HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass()));
		}
		else if(SimpleFPSWorldState->NPCNearbyPOI != nullptr)
		{
			// create decomposition
			TSharedPtr<FHTNTaskInstance> Decomposition = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());

			// create primitive task to get out of cover
			TSharedPtr<FHTNTaskInstance> UncoverTask = HTNComp.InstantiateTask(UHTNTask_UncoverSimpleFPS::StaticClass());
			FHTNTask_UncoverSimpleFPSMemory* UncoverMemory = (FHTNTask_UncoverSimpleFPSMemory*)UncoverTask->GetMemory();
			UncoverMemory->CoverPoint = SimpleFPSWorldState->NPCNearbyPOI;

			// finalize and add decomposition
			UTaskNetwork* DecompositionNetwork = Cast<UTaskNetwork>(Decomposition->Task);
			DecompositionNetwork->AddTaskOrSubNetwork(UncoverTask, Decomposition->GetMemory());
			Decompositions.Add(Decomposition);
		}
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_UncoverCompoundSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}

	return Decompositions;
}

float UHTNTask_UncoverCompoundSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}