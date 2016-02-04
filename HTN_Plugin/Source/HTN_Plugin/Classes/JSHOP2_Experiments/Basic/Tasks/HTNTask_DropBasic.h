#pragma once

#include "HTNPlanner/PrimitiveTask.h"
#include "HTNTask_DropBasic.generated.h"

struct FHTNDummyObject;

struct FHTNTask_DropBasicMemory
{
	/** The Object that should be dropped */
	FHTNDummyObject* Object;
};

/**
 * 'Drop' task for the JSHOP2 'basic' example domain.
 */
UCLASS()
class HTN_PLUGIN_API UHTNTask_DropBasic : public UPrimitiveTask
{
	GENERATED_BODY()

public:
	virtual void ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const override;
	virtual EHTNExecutionResult ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory) override;
	virtual float GetHeuristicCost() const override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual bool IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const override;
};