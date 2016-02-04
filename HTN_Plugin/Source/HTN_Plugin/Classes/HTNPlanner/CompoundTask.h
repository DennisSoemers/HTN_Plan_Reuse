#pragma once

#include "HTNTask.h"
#include "CompoundTask.generated.h"

struct FHTNWorldState;
class UHTNPlannerComponent;
class UTaskNetwork;

UCLASS(HideDropdown)
class HTN_PLUGIN_API UCompoundTask : public UHTNTask
{
	GENERATED_BODY()

public:
	/**
	 * Should be implemented to return an Array of Task Network Instances, where every Task Network
	 * is one of the Networks that the Compound Task can be decomposed into given the World State.
	 */
	virtual TArray<TSharedPtr<FHTNTaskInstance>> FindDecompositions(UHTNPlannerComponent& HTNComp, 
																	const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory);

	/**
	 * Can be overridden to return a heuristic estimate of the cost of executing this action in a future World
	 * State. Must be an admissible heuristic (exact or underestimate of the real cost) to be able to guarantee
	 * finding an optimal plan.
	 *
	 * Default implementation returns a cost of 0.f
	 */
	virtual float GetHeuristicCost() const;
};