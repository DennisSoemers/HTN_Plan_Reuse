#pragma once

#include "HTNPlanner/CompoundTask.h"
#include "HTNPlanner/HTNWorldState.h"
#include "CompoundTask_BlueprintBase.generated.h"

/**
 * Wrapper struct passed into the BlueprintImplementable GetDecompositions() function so that 
 * that wrapper struct can be passed into a C++-based casting function from blueprint.
 */
USTRUCT(BlueprintType)
struct HTN_PLUGIN_API FHTNWorldStateWrapper
{
	GENERATED_BODY()

public:
	FHTNWorldStateWrapper() : WorldState(nullptr) {}
	FHTNWorldStateWrapper(TSharedPtr<FHTNWorldState> WorldState) : WorldState(WorldState) {}

	TSharedPtr<FHTNWorldState> WorldState;
};

/**
 * Base class for Compound HTN Tasks that should be implemented in Blueprint.
 */
UCLASS(Blueprintable)
class HTN_PLUGIN_API UCompoundTask_BlueprintBase : public UCompoundTask
{
	GENERATED_BODY()

public:
	virtual TArray<TSharedPtr<FHTNTaskInstance>> FindDecompositions(UHTNPlannerComponent& HTNComp,
																	const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) override;

	/**
	 * Should be implemented to return an Array of Decompositions, where every Decomposition
	 * holds one of the Networks that the Compound Task can be decomposed into given the World State.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Compound Task")
	TArray<TSubclassOf<UTaskNetwork>> GetDecompositions(UHTNPlannerComponent* HTNComp, const FHTNWorldStateWrapper& WorldStateWrapper);
};