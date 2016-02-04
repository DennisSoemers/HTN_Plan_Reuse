#pragma once

#include "HTNPlanner/CompoundTask.h"
#include "HTNTask_ReloadCompoundSimpleFPS.generated.h"

struct FSimpleFPSObject;

struct FHTNTask_ReloadCompoundSimpleFPSMemory
{
	FSimpleFPSObject* Gun;
};

/**
 * 'Reload' compound task for the SimpleFPS domain.
 */
UCLASS()
class HTN_PLUGIN_API UHTNTask_ReloadCompoundSimpleFPS : public UCompoundTask
{
	GENERATED_BODY()

public:
	UHTNTask_ReloadCompoundSimpleFPS(const FObjectInitializer& ObjectInitializer);

	virtual TArray<TSharedPtr<FHTNTaskInstance>> FindDecompositions(UHTNPlannerComponent& HTNComp,
																	const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) override;
	virtual float GetHeuristicCost() const override;
	virtual uint16 GetInstanceMemorySize() const override;

protected:
	/**
	 * Caching the heuristic cost here. Only the class CDO needs to memorize this,
	 * so it doesn't go into the TaskMemory of individual instances of this class.
	 */
	float HeuristicCost;
};