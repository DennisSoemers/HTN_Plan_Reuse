#pragma once

#include "HTNPlanner/CompoundTask.h"
#include "HTNTask_SwapBasic.generated.h"

struct FHTNDummyObject;

struct FHTNTask_SwapBasicMemory
{
	/** The first of the Objects that should be swapped */
	FHTNDummyObject* Object1;
	/** The second of the Objects that should be swapped */
	FHTNDummyObject* Object2;
};

/**
 * 'Swap' compound task for the JSHOP2 'basic' example domain.
 */
UCLASS()
class HTN_PLUGIN_API UHTNTask_SwapBasic : public UCompoundTask
{
	GENERATED_BODY()

public:
	virtual TArray<TSharedPtr<FHTNTaskInstance>> FindDecompositions(UHTNPlannerComponent& HTNComp,
																	const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) override;
	virtual float GetHeuristicCost() const override;
	virtual uint16 GetInstanceMemorySize() const override;
};