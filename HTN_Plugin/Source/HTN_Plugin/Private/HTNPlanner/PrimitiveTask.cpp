#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/HTNWorldState.h"
#include "HTNPlanner/PrimitiveTask.h"

EHTNExecutionResult UPrimitiveTask::AbortTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	return EHTNExecutionResult::Aborted;
}

void UPrimitiveTask::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	// no default implementation
}

EHTNExecutionResult UPrimitiveTask::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	return EHTNExecutionResult::Succeeded;
}

void UPrimitiveTask::FinishLatentTask(UHTNPlannerComponent& HTNComp, EHTNExecutionResult TaskResult, uint8* TaskMemory) 
{
	HTNComp.OnTaskFinished(this, TaskMemory, TaskResult);
}

void UPrimitiveTask::FinishLatentAbort(UHTNPlannerComponent& HTNComp, uint8* TaskMemory) 
{
	HTNComp.OnTaskFinished(this, TaskMemory, EHTNExecutionResult::Aborted);
}

float UPrimitiveTask::GetCost(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	return 1.f;	// TO DO default cost of 0.0 probably makes more sense in context of UE, but 1.0 as default corresponds to SHOP2 / JSHOP2 / academia
}

float UPrimitiveTask::GetHeuristicCost() const
{
	return 0.f;
}

bool UPrimitiveTask::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	return false;
}

void UPrimitiveTask::OnMessage(UHTNPlannerComponent& HTNComp, FName Message, int32 RequestID, 
							   bool bSuccess, const TSharedPtr<FHTNTaskInstance>& TaskInstance)
{
	const EHTNTaskStatus Status = HTNComp.GetTaskStatus(TaskInstance);
	if(Status == EHTNTaskStatus::Active)
	{
		FinishLatentTask(HTNComp, bSuccess ? EHTNExecutionResult::Succeeded : EHTNExecutionResult::Failed, TaskInstance->GetMemory());
	}
	else if(Status == EHTNTaskStatus::Aborting)
	{
		FinishLatentAbort(HTNComp, TaskInstance->GetMemory());
	}
}

void UPrimitiveTask::OnTaskFinished(UHTNPlannerComponent& HTNComp, EHTNExecutionResult TaskResult, uint8* TaskMemory)
{
	//no default implementation
}

void UPrimitiveTask::ReceivedMessage(UBrainComponent* BrainComp, const FAIMessage& Message, TSharedPtr<FHTNTaskInstance> TaskInstance)
{
	UHTNPlannerComponent* OwnerComp = static_cast<UHTNPlannerComponent*>(BrainComp);
	check(OwnerComp);
	OnMessage(*OwnerComp, Message.MessageName, Message.RequestID, Message.Status == FAIMessage::Success, TaskInstance);
}

void UPrimitiveTask::TickTask(UHTNPlannerComponent& HTNComp, float DeltaSeconds, uint8* TaskMemory)
{
	// empty in base class
}

void UPrimitiveTask::WaitForMessage(UHTNPlannerComponent& HTNComp, FName MessageType, uint8* TaskMemory) const
{
	HTNComp.RegisterMessageObserver(this, TaskMemory, MessageType);
}

void UPrimitiveTask::WaitForMessage(UHTNPlannerComponent& HTNComp, FName MessageType, int32 RequestID, uint8* TaskMemory) const
{
	HTNComp.RegisterMessageObserver(this, TaskMemory, MessageType, RequestID);
}