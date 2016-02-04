#include "HTN_PluginPrivatePCH.h"

#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/Tasks/HTNTask_RegisterService.h"

UHTNTask_RegisterService::UHTNTask_RegisterService(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Register Service";
}

void UHTNTask_RegisterService::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	// no effect
}

EHTNExecutionResult UHTNTask_RegisterService::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	HTNComp.RegisterService(Service);
	return EHTNExecutionResult::Succeeded;
}

float UHTNTask_RegisterService::GetCost(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	return 0.f;
}

bool UHTNTask_RegisterService::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	return true;
}