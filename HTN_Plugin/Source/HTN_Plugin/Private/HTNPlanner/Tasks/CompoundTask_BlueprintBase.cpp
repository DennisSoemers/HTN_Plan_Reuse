#include "HTN_PluginPrivatePCH.h"

#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/Tasks/CompoundTask_BlueprintBase.h"

TArray<TSharedPtr<FHTNTaskInstance>> UCompoundTask_BlueprintBase::FindDecompositions(UHTNPlannerComponent& HTNComp,
																					 const TSharedPtr<FHTNWorldState> WorldState, 
																					 uint8* TaskMemory)
{
	TArray<TSubclassOf<UTaskNetwork>> Decompositions = GetDecompositions(&HTNComp, FHTNWorldStateWrapper(WorldState));
	TArray<TSharedPtr<FHTNTaskInstance>> DecompositionInstances;
	DecompositionInstances.Reserve(Decompositions.Num());

	for(const TSubclassOf<UTaskNetwork>& Decomposition : Decompositions)
	{
		DecompositionInstances.Add(HTNComp.InstantiateNetwork(Decomposition));
	}

	return DecompositionInstances;
}