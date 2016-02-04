#pragma once

#include "HTNPlanner/PrimitiveTask.h"
#include "HTNTask_MoveToPointSimpleFPS.generated.h"

struct FSimpleFPSObject;

struct FHTNTask_MoveToPointSimpleFPSMemory
{
	/** The Point of Interest to move to */
	FSimpleFPSObject* PointOfInterest;
	int32 Area;
};

/**
 * 'Move-to-Point' primitive task for the SimpleFPS domain.
 */
UCLASS()
class HTN_PLUGIN_API UHTNTask_MoveToPointSimpleFPS : public UPrimitiveTask
{
	GENERATED_BODY()

public:
	UHTNTask_MoveToPointSimpleFPS(const FObjectInitializer& ObjectInitializer);

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