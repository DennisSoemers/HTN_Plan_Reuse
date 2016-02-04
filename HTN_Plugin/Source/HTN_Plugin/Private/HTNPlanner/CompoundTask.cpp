#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/CompoundTask.h"
#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/HTNWorldState.h"
#include "HTNPlanner/TaskNetwork.h"

TArray<TSharedPtr<FHTNTaskInstance>> UCompoundTask::FindDecompositions(UHTNPlannerComponent& HTNComp, 
																	   const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory)
{
	// default implementation: no decompositions possible
	return TArray<TSharedPtr<FHTNTaskInstance>>();
}

float UCompoundTask::GetHeuristicCost() const
{
	return 0.f;
}