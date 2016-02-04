#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/HTNTask.h"

void UHTNTask::Cleanup(uint8* TaskMemory)
{
	// no default implementation
}

TSharedPtr<FHTNTaskInstance> UHTNTask::Copy(const TSharedPtr<FHTNTaskInstance>& OriginalInstance)
{
	TSharedPtr<FHTNTaskInstance> Copy = TSharedPtr<FHTNTaskInstance>(OriginalInstance);
	return Copy;
}

uint16 UHTNTask::GetInstanceMemorySize() const
{
	return 0;
}

FString UHTNTask::GetTaskName(uint8* TaskMemory) const
{
	return TaskName;
}

void UHTNTask::Instantiate(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	// don't need to do anything in base class
}