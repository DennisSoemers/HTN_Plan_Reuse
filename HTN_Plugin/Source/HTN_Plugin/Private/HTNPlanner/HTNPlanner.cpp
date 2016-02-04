#include "HTN_PluginPrivatePCH.h"

#include "HTNPlanner/HTNPlanner.h"
#include "HTNPlanner/HTNWorldState.h"

DEFINE_LOG_CATEGORY(LogHTNPlanner);

UHTNPlanner::UHTNPlanner(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// default values of properties
	bExecutePlan = true;
	bDepthFirstSearch = true;
	bIgnoreTaskCosts = false;
	bLoop = true;
	bUseHeuristicCosts = true;
	MaxSearchTime = 0.05f;

	EmptyWorldState = nullptr;
	TaskNetworkInstance = nullptr;
}