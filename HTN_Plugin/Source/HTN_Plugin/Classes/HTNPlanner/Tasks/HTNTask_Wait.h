#pragma once

#include "HTNPlanner/PrimitiveTask.h"
#include "HTNTask_Wait.generated.h"

struct FHTNWorldState;
class UHTNPlannerComponent;

struct FHTNTask_WaitMemory
{
	/** time left */
	float RemainingWaitTime;
};

/**
 * Wait task
 * Wait for the specified time when executed.
 *
 * This HTN Task is equivalent to the Behavior Tree's BTTask_Wait
 */
UCLASS(Blueprintable)
class HTN_PLUGIN_API UHTNTask_Wait : public UPrimitiveTask
{
	GENERATED_BODY()

public:
	UHTNTask_Wait(const FObjectInitializer& ObjectInitializer);

	/** No effects on world state */
	virtual void ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const override;

	virtual EHTNExecutionResult ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory) override;

	/** We'll assume waiting has a default cost of 0.0 */
	virtual float GetCost(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const override;
	uint16 GetInstanceMemorySize() const override;
	/** We'll assume waiting is always possible by default */
	virtual bool IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const override;

	virtual void TickTask(UHTNPlannerComponent& HTNComp, float DeltaSeconds, uint8* TaskMemory) override;

protected:
	/** wait time in seconds */
	UPROPERTY(Category = Wait, EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float WaitTime;
	/** allows adding random time to wait time */
	UPROPERTY(Category = Wait, EditAnywhere, meta = (UIMin = 0, ClampMin = 0))
	float RandomDeviation;
};
