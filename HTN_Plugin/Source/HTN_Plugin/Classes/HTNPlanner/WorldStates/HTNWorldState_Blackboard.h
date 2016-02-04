#pragma once

#include "HTNPlanner/HTNWorldState.h"
#include "HTNWorldState_Blackboard.generated.h"

/**
 * This type of World State will use a copy of the agent's Blackboard to represent the World State.
 * This means that exactly those fields that are specified in the Blackboard asset can be read and
 * changed during the planning process to interpret and reason about the World State.
 */
USTRUCT(BlueprintType)
struct HTN_PLUGIN_API FHTNWorldState_Blackboard : public FHTNWorldState
{
	GENERATED_BODY()

public:
	virtual ~FHTNWorldState_Blackboard() {}
	// TO DO implement
};