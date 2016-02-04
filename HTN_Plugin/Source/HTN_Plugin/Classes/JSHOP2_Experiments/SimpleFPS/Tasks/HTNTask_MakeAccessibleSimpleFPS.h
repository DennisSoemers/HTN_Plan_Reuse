#pragma once

#include "HTNPlanner/PrimitiveTask.h"
#include "HTNTask_MakeAccessibleSimpleFPS.generated.h"

struct FSimpleFPSObject;

struct FHTNTask_MakeAccessibleSimpleFPSMemory
{
	/** The Keycard that should be used to open the waypoint */
	FSimpleFPSObject* Keycard;
	/** The Waypoint that should be opened */
	FSimpleFPSObject* Waypoint;
	int32 FromArea;
	int32 ToArea;
};

/**
 * 'Make-Accessible' primitive task for the SimpleFPS domain.
 */
UCLASS()
class HTN_PLUGIN_API UHTNTask_MakeAccessibleSimpleFPS : public UPrimitiveTask
{
	GENERATED_BODY()

public:
	UHTNTask_MakeAccessibleSimpleFPS(const FObjectInitializer& ObjectInitializer);

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