#pragma once

#include "HTNPlanner/PrimitiveTask.h"
#include "HTNTask_AttackMeleeSimpleFPS.generated.h"

struct FSimpleFPSObject;

struct FHTNTask_AttackMeleeSimpleFPSMemory
{
	/** The Knife that should be used */
	FSimpleFPSObject* Knife;
	/** The Player that should be attacked */
	FSimpleFPSObject* Player;
	int32 Area;
};

/**
 * 'Attack-Melee' primitive task for the SimpleFPS domain.
 */
UCLASS()
class HTN_PLUGIN_API UHTNTask_AttackMeleeSimpleFPS : public UPrimitiveTask
{
	GENERATED_BODY()

public:
	UHTNTask_AttackMeleeSimpleFPS(const FObjectInitializer& ObjectInitializer);

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