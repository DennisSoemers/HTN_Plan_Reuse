#pragma once

#include "HTNPlannerTypes.h"
#include "HTNTask.h"
#include "PrimitiveTask.generated.h"

class UBrainComponent;
class UHTNPlannerComponent;

struct FAIMessage;
struct FHTNWorldState;

UCLASS(HideDropdown)
class HTN_PLUGIN_API UPrimitiveTask : public UHTNTask
{
	GENERATED_BODY()

public:
	/** Aborts execution of the Task, and should return the status of the execution. */
	virtual EHTNExecutionResult AbortTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory);

	/** Should be implemented to modify the given World State according to the effects of the Task */
	virtual void ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const;

	/**
	 * Starts execution of the Task, and should return the status of the execution.
	 */
	virtual EHTNExecutionResult ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory);

	/** helper function: finish latent executing */
	void FinishLatentTask(UHTNPlannerComponent& HTNComp, EHTNExecutionResult TaskResult, uint8* TaskMemory);
	/** helper function: finishes latent aborting */
	void FinishLatentAbort(UHTNPlannerComponent& HTNComp, uint8* TaskMemory);

	/** 
	 * Can be overridden to return the cost of applying this Task in the given World State.
	 * Default implementation returns a cost of 1.0 regardless of the World State
	 */
	virtual float GetCost(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const;

	/**
	 * Can be overridden to return a heuristic estimate of the cost of executing this action in a future World
	 * State. Must be an admissible heuristic (exact or underestimate of the real cost) to be able to guarantee
	 * finding an optimal plan.
	 *
	 * Default implementation returns a cost of 0.f
	 */
	virtual float GetHeuristicCost() const;

	/** Should be implemented to return true if the Task is applicable in the given World State. */
	virtual bool IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const;

	/** message handler, default implementation will finish latent execution/abortion */
	virtual void OnMessage(UHTNPlannerComponent& HTNComp, FName Message, int32 RequestID, 
						   bool bSuccess, const TSharedPtr<FHTNTaskInstance>& TaskInstance);
	/** Called when the Task's execution is finished */
	virtual void OnTaskFinished(UHTNPlannerComponent& HTNComp, EHTNExecutionResult TaskResult, uint8* TaskMemory);

	/** message observer's hook */
	void ReceivedMessage(UBrainComponent* BrainComp, const FAIMessage& Message, TSharedPtr<FHTNTaskInstance> TaskInstance);

	/** ticks this task */
	virtual void TickTask(UHTNPlannerComponent& HTNComp, float DeltaSeconds, uint8* TaskMemory);

	/** register message observer */
	void WaitForMessage(UHTNPlannerComponent& HTNComp, FName MessageType, uint8* TaskMemory) const;
	void WaitForMessage(UHTNPlannerComponent& HTNComp, FName MessageType, int32 RequestID, uint8* TaskMemory) const;
};