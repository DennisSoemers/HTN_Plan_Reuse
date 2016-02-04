#pragma once

#include "HTNPlannerTypes.h"
#include "HTNWorldState.generated.h"

class UBlackboardComponent;
class UPrimitiveTask;

/**
 * This struct should be used to define the state of the world during the planning process of 
 * a Hierarchical Task Network (HTN) Planner. It fulfills a similar role to the HTN Planner as
 * the Blackboard does to a Behavior Tree, but is different in that the Planner should be able
 * to modify it during the planning process and should be able to efficiently make copies of
 * the World State.
 */
USTRUCT(BlueprintType)
struct HTN_PLUGIN_API FHTNWorldState
{
	GENERATED_BODY()

public:
	virtual ~FHTNWorldState() {}

	/**
	 * Called when a Task does not know how to apply itself to the World State, and therefore
	 * needs the World State to perform the application of the Task.
	 */
	virtual void ApplyTask(const UPrimitiveTask* Task, uint8* TaskMemory);

	/**
	 * Called when a Task does not know how to check whether it is applicable to the World State
	 * itself, and therefore needs the World State to perform this check.
	 *
	 * Should be implemented to return true if and only if the given Task is applicable in the current World State
	 */
	virtual bool CanApply(const UPrimitiveTask* Task, uint8* TaskMemory) const;

	/** Should be implemented to create and return a copy of this World State */
	virtual TSharedPtr<FHTNWorldState> Copy() const;

	/** 
	 * Can be implemented to return an estimate of the cost of executing the entire given collection
	 * of tasks. Will be added to the heuristic costs of the individual tasks, so should only return extra
	 * cost that can be derived from the current world state.
	 *
	 * By default simply returns 0.f
	 */
	virtual float GetHeuristicCost(const TArray<TSharedPtr<struct FHTNTaskInstance>>& TaskInstances) const;

	/** Can be implemented to return a specific name for the world state. By default returns "HTN World State" */
	virtual FName GetWorldStateName() const;

	/** Allows for initialization of the World State according to the given Owner and BlackboardComponent */
	virtual void Initialize(AActor* Owner, UBlackboardComponent* BlackboardComponent);
};