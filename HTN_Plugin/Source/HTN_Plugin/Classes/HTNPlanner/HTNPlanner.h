#pragma once

#include "HTNPlanner/HTNPlannerTypes.h"
#include "HTNPlanner/HTNWorldState.h"
#include "HTNPlanner.generated.h"

class UBlackboardData;
class UTaskNetwork;

UCLASS()
class HTN_PLUGIN_API UHTNPlanner : public UDataAsset
{
	GENERATED_BODY()

public:
	UHTNPlanner(const FObjectInitializer& ObjectInitializer);

	/** blackboard asset for this planner */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	UBlackboardData* BlackboardAsset;
	/** If true, the Planner will perform a breadth-first search. Otherwise, a more breadth-first / A* style search will be used */
	UPROPERTY(EditAnywhere, Category = "Hierarchical Task Network Planner")
	bool bDepthFirstSearch;
	/** If true, the Planner will also execute the plan after finding it. */
	UPROPERTY(EditAnywhere, Category = "Hierarchical Task Network Planner")
	bool bExecutePlan;
	/** If true, the Planner will ignore the costs of Tasks and simply produce the first plan it is able to find instead. */
	UPROPERTY(EditAnywhere, Category = "Hierarchical Task Network Planner")
	bool bIgnoreTaskCosts;
	/** If true, the Planner will re-start from the initial Task Network to create a new plan once the previous plan has been executed. */
	UPROPERTY(EditAnywhere, Category = "Hierarchical Task Network Planner")
	bool bLoop;
	/** If true, the Planner will use heuristic costs to prune the search early */
	UPROPERTY(EditAnywhere, Category = "Hierarchical Task Network Planner")
	bool bUseHeuristicCosts;
	/** The maximum amount of time (in seconds) that the planner can dedicate to planning per Tick */
	UPROPERTY(EditAnywhere, Category = "Hierarchical Task Network Planner", meta=(ClampMin=0, ClampMax=1))
	float MaxSearchTime;
	/** The Task Network for which a plan needs to be found when the planner is (re)started */
	UPROPERTY(EditAnywhere, Category = "Hierarchical Task Network Planner")
	TSubclassOf<UTaskNetwork> TaskNetwork;

	/**
	 * Can be set to some initial World State struct so that the planner knows
	 * what struct type should be used for the World State (cannot use a TSubclassOf
	 * property because they're not compatible with non-UObjects).
	 *
	 * If not set, will use EmptyWorldState set in HTNPlannerComponent instead of in the Planner asset.
	 *
	 * Should not already be initialized, only needs to be of the correct type!
	 */
	TSharedPtr<FHTNWorldState> EmptyWorldState;

	/** 
	 * Will be used if TaskNetwork does not hold a valid blueprint. 
	 * Can be used to dynamically create planning problems at runtime.
	 */
	TSharedPtr<FHTNTaskInstance> TaskNetworkInstance;
};