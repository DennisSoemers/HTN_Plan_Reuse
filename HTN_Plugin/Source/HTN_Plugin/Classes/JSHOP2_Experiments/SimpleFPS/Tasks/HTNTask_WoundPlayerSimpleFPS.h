#pragma once

#include "HTNPlanner/CompoundTask.h"
#include "HTNTask_WoundPlayerSimpleFPS.generated.h"

struct FSimpleFPSObject;

struct FHTNTask_WoundPlayerSimpleFPSMemory
{
	FSimpleFPSObject* Player;
	int32 Area;
};

/**
 * 'Wound-Player' compound task for the SimpleFPS domain.
 */
UCLASS()
class HTN_PLUGIN_API UHTNTask_WoundPlayerSimpleFPS : public UCompoundTask
{
	GENERATED_BODY()

public:
	UHTNTask_WoundPlayerSimpleFPS(const FObjectInitializer& ObjectInitializer);

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