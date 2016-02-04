#include "HTN_PluginPrivatePCH.h"

#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/Tasks/HTNTask_UnregisterService.h"

UHTNTask_UnregisterService::UHTNTask_UnregisterService(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Unregister Service";
}

void UHTNTask_UnregisterService::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	// no effects
}

EHTNExecutionResult UHTNTask_UnregisterService::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	HTNComp.UnregisterService(Service);
	return EHTNExecutionResult::Succeeded;
}

float UHTNTask_UnregisterService::GetCost(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	return 0.f;
}

bool UHTNTask_UnregisterService::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	return true;
}