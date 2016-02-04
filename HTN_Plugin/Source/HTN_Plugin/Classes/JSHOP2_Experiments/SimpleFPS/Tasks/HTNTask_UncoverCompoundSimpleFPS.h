#pragma once

#include "HTNPlanner/CompoundTask.h"
#include "HTNTask_UncoverCompoundSimpleFPS.generated.h"

struct FSimpleFPSObject;

/**
 * 'Uncover' compound task for the SimpleFPS domain.
 */
UCLASS()
class HTN_PLUGIN_API UHTNTask_UncoverCompoundSimpleFPS : public UCompoundTask
{
	GENERATED_BODY()

public:
	UHTNTask_UncoverCompoundSimpleFPS(const FObjectInitializer& ObjectInitializer);

	virtual TArray<TSharedPtr<FHTNTaskInstance>> FindDecompositions(UHTNPlannerComponent& HTNComp,
																	const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) override;
	virtual float GetHeuristicCost() const override;

protected:
	/**
	 * Caching the heuristic cost here. Only the class CDO needs to memorize this,
	 * so it doesn't go into the TaskMemory of individual instances of this class.
	 */
	float HeuristicCost;
};