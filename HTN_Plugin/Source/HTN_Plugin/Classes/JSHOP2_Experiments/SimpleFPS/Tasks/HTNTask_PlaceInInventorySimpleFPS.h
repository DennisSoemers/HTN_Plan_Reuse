#pragma once

#include "HTNPlanner/PrimitiveTask.h"
#include "HTNTask_PlaceInInventorySimpleFPS.generated.h"

struct FSimpleFPSObject;

struct FHTNTask_PlaceInInventorySimpleFPSMemory
{
	/** The Item that should be placed in inventory */
	FSimpleFPSObject* Item;
	int32 Area;
};

/**
 * 'Place-in-Inventory' primitive task for the SimpleFPS domain.
 */
UCLASS()
class HTN_PLUGIN_API UHTNTask_PlaceInInventorySimpleFPS : public UPrimitiveTask
{
	GENERATED_BODY()

public:
	UHTNTask_PlaceInInventorySimpleFPS(const FObjectInitializer& ObjectInitializer);

	virtual void ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const override;
	virtual EHTNExecutionResult ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory) override;
	virtual float GetHeuristicCost() const override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual bool IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const override;

protected:
	/**
	 * Caching the heuristic cost here. Only the class CDO needs to memorize this,
	 * so it doesn't go into the TaskMemory of individual instances of this class.
	 */
	float HeuristicCost;
};