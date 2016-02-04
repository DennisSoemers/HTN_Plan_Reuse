#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/HTNPlannerTypes.h"
#include "HTNPlanner/HTNTask.h"

FHTNTaskInstance::~FHTNTaskInstance()
{
	if(Task && !Task->IsPendingKill())
	{
		Task->Cleanup(GetMemory());
	}
}

FString UHTNPlannerTypes::DescribeTaskHelper(const UHTNTask* Task, uint8* TaskMemory)
{
	return Task ? FString::Printf(TEXT("%s"), *Task->GetTaskName(TaskMemory)) : FString();
}

FString UHTNPlannerTypes::DescribeTaskResult(EHTNExecutionResult TaskResult)
{
	static FString ResultDesc[] = { TEXT("Succeeded"), TEXT("Failed"), TEXT("Aborted"), TEXT("InProgress") };
	uint32 TaskResultUint = static_cast<uint32>(TaskResult);
	return (TaskResultUint < ARRAY_COUNT(ResultDesc)) ? ResultDesc[TaskResultUint] : FString();
}