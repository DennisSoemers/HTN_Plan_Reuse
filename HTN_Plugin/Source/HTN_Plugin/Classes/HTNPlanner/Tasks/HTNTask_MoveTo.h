#pragma once

#include "HTNPlanner/PrimitiveTask.h"
#include "HTNTask_MoveTo.generated.h"

class UHTNPlannerComponent;
class UNavigationQueryFilter;

struct FHTNTask_MoveToMemory
{
	/** Move request ID */
	FAIRequestID MoveRequestID;

	/** If set, we're currently waiting for a path to be generated */
	uint8 bWaitingForPath : 1;

	FHTNTask_MoveToMemory() : bWaitingForPath(false) {}
};

/**
 * Move To primitive Task.
 * Moves the AI pawn toward the specified Actor or Location using the navigation system.
 *
 * This HTN Primitive Task is equivalent to the Behavior Tree's UBTTask_MoveTo node.
 */
UCLASS(config=Game, Blueprintable)
class HTN_PLUGIN_API UHTNTask_MoveTo : public UPrimitiveTask
{
	GENERATED_BODY()

public:
	UHTNTask_MoveTo(const FObjectInitializer& ObjectInitializer);

	// overrides for in-game execution of task
	virtual EHTNExecutionResult AbortTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory) override;
	virtual EHTNExecutionResult ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory) override;
	virtual void OnMessage(UHTNPlannerComponent& HTNComp, FName Message, int32 RequestID, 
						   bool bSuccess, const TSharedPtr<FHTNTaskInstance>& TaskInstance) override;
	virtual void TickTask(UHTNPlannerComponent& HTNComp, float DeltaSeconds, uint8* TaskMemory) override;

	// overrides for planning
	virtual void ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const override;
	uint16 GetInstanceMemorySize() const override;
	virtual bool IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const override;

protected:
	/** "None" will result in default filter being used */
	UPROPERTY(Category = Task, EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UNavigationQueryFilter> FilterClass;

	/** The Name of the Blackboard Key that holds the target to move to if it is held in a Blackboard */
	UPROPERTY(Category = Task, EditAnywhere, BlueprintReadWrite)
	FName TargetBlackboardKey;
	/** The Actor to move to. This value will only be used if TargetBlackboardKey is not set. */	// TO DO should maybe make this a weak ptr?
	UPROPERTY(Category = Task, EditAnywhere, BlueprintReadWrite)
	AActor* TargetActor;
	/** The Location to move to. This value will only be used if TargetBlackboardKey and TargetLocation are not set. */
	UPROPERTY(Category = Task, EditAnywhere, BlueprintReadWrite)
	FVector TargetLocation;

	/** The maximum distance between the AI pawn and the target for the Task to be considered as successfully finished */
	UPROPERTY(config, Category = Task, EditAnywhere, meta = (ClampMin = "0.0"))
	float AcceptableRadius;

	/** If set, the pawn will be allowed to strafe */
	UPROPERTY(Category = Task, EditAnywhere, BlueprintReadWrite)
	uint8 bAllowStrafe : 1;
	/** if set, use incomplete path when goal can't be reached */
	UPROPERTY(Category = Task, EditAnywhere, AdvancedDisplay, BlueprintReadWrite)
	uint8 bAllowPartialPath : 1;
	/** if set to true agent's radius will be added to AcceptableRadius for purposes of checking
	*	if path's end point has been reached. Will result in AI stopping on contact with destination location*/
	UPROPERTY(Category = Task, EditAnywhere, BlueprintReadWrite)
	uint8 bStopOnOverlap : 1;

	EHTNExecutionResult PerformMoveTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory);
};