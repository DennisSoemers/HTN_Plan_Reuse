#pragma once

#include "HTNPlanner/PrimitiveTask.h"
#include "HTNTask_EvaluateWeaponChoiceSimpleFPS.generated.h"

enum class EHTNWeaponChoiceSimpleFPS : uint8
{
	NoChoice,
	Ranged,
	Stealth,
	Melee
};

struct FHTNTask_EvaluateWeaponChoiceSimpleFPSMemory
{
	EHTNWeaponChoiceSimpleFPS Choice;
};

/**
 * An extra bookkeeping task we use in SimpleFPS to punish changing to a different choice of
 * attack, resulting in a greater likelihood that old plans will be reusable
 */
UCLASS()
class HTN_PLUGIN_API UHTNTask_EvaluateWeaponChoiceSimpleFPS : public UPrimitiveTask
{
	GENERATED_BODY()

public:
	UHTNTask_EvaluateWeaponChoiceSimpleFPS(const FObjectInitializer& ObjectInitializer);

	virtual void ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const override;
	virtual EHTNExecutionResult ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory) override;
	virtual float GetCost(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual bool IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const override;

	virtual FString GetTaskName(uint8* TaskMemory) const override;

protected:
	/**
	 * Caching the heuristic cost here. Only the class CDO needs to memorize this,
	 * so it doesn't go into the TaskMemory of individual instances of this class.
	 */
	float HeuristicCost;
};