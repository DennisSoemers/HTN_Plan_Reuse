#include "HTN_PluginPrivatePCH.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/HTNWorldState.h"
#include "HTNPlanner/Tasks/HTNTask_MoveTo.h"

UHTNTask_MoveTo::UHTNTask_MoveTo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Move To";

#if HTN_COMPILE_UT_COMPATIBLE
	AcceptableRadius = 50.f;
	bAllowStrafe = false;
	bAllowPartialPath = true;
	bStopOnOverlap = true;
#else
	AcceptableRadius = GET_AI_CONFIG_VAR(AcceptanceRadius);
	bStopOnOverlap = GET_AI_CONFIG_VAR(bFinishMoveOnGoalOverlap);
	bAllowStrafe = GET_AI_CONFIG_VAR(bAllowStrafing);
	bAllowPartialPath = GET_AI_CONFIG_VAR(bAcceptPartialPaths);
#endif // HTN_COMPILE_UT_COMPATIBLE

	// initialize all ways to specify the target location to none/null/invalid
	TargetBlackboardKey = NAME_None;
	TargetActor = nullptr;
	TargetLocation = FAISystem::InvalidLocation;
}

EHTNExecutionResult UHTNTask_MoveTo::AbortTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	FHTNTask_MoveToMemory* Memory = (FHTNTask_MoveToMemory*)TaskMemory;

	if(!Memory->bWaitingForPath)
	{
		AAIController* MyController = HTNComp.GetAIOwner();

		if(MyController && MyController->GetPathFollowingComponent())
		{
			MyController->GetPathFollowingComponent()->AbortMove(TEXT("HTN Planner abort"), Memory->MoveRequestID);
		}
	}

	return Super::AbortTask(HTNComp, TaskMemory);
}

void UHTNTask_MoveTo::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	// TO DO default implementation for HTNWorldState_Blackboard
	WorldState->ApplyTask(this, TaskMemory);
}

EHTNExecutionResult UHTNTask_MoveTo::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	FHTNTask_MoveToMemory* Memory = (FHTNTask_MoveToMemory*)TaskMemory;
	EHTNExecutionResult Result = EHTNExecutionResult::InProgress;

	AAIController* MyController = HTNComp.GetAIOwner();

	Memory->bWaitingForPath = MyController->ShouldPostponePathUpdates();
	if(!Memory->bWaitingForPath)
	{
		Result = PerformMoveTask(HTNComp, TaskMemory);
	}
	else
	{
		UE_VLOG(MyController, LogHTNPlanner, Log, TEXT("Pathfinding requests are freezed, waiting..."));
	}

	return Result;
}

uint16 UHTNTask_MoveTo::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_MoveToMemory);
}

bool UHTNTask_MoveTo::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	// TO DO default implementation for HTNWorldState_Blackboard
	return WorldState->CanApply(this, TaskMemory);
}

void UHTNTask_MoveTo::OnMessage(UHTNPlannerComponent& HTNComp, FName Message, int32 SenderID, 
								bool bSuccess, const TSharedPtr<FHTNTaskInstance>& TaskInstance)
{
	// AIMessage_RepathFailed means task has failed
	bSuccess &= (Message != UBrainComponent::AIMessage_RepathFailed);
	Super::OnMessage(HTNComp, Message, SenderID, bSuccess, TaskInstance);
}

EHTNExecutionResult UHTNTask_MoveTo::PerformMoveTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	const UBlackboardComponent* MyBlackboard = HTNComp.GetBlackboardComponent();
	AAIController* MyController = HTNComp.GetAIOwner();

	EHTNExecutionResult Result = EHTNExecutionResult::Failed;

	if(MyController && MyBlackboard)
	{
		EPathFollowingRequestResult::Type RequestResult = EPathFollowingRequestResult::Failed;

		FAIMoveRequest MoveReq;
		MoveReq.SetNavigationFilter(FilterClass);
		MoveReq.SetAllowPartialPath(bAllowPartialPath);
		MoveReq.SetAcceptanceRadius(AcceptableRadius);
		MoveReq.SetCanStrafe(bAllowStrafe);
		MoveReq.SetStopOnOverlap(bStopOnOverlap);

		if(TargetBlackboardKey != NAME_None)
		{
			FBlackboard::FKey KeyID = MyBlackboard->GetKeyID(TargetBlackboardKey);
			TSubclassOf<UBlackboardKeyType> KeyType = MyBlackboard->GetKeyType(KeyID);

			if(KeyType->IsChildOf(UBlackboardKeyType_Vector::StaticClass()))
			{
				const FVector Loc = MyBlackboard->GetValue<UBlackboardKeyType_Vector>(KeyID);
				MoveReq.SetGoalLocation(Loc);

				RequestResult = MyController->MoveTo(MoveReq);
			}
			else if(KeyType->IsChildOf(UBlackboardKeyType_Object::StaticClass()))
			{
				UObject* Object = MyBlackboard->GetValue<UBlackboardKeyType_Object>(KeyID);
				if(AActor* Actor = Cast<AActor>(Object))
				{
					MoveReq.SetGoalActor(Actor);

					RequestResult = MyController->MoveTo(MoveReq);
				}
				else
				{
					UE_VLOG(MyController, LogHTNPlanner, Warning, TEXT("UHTNTask_MoveTo::PerformMoveTask() tried to go to actor while BB %s entry was empty"), *TargetBlackboardKey.ToString());
				}
			}
			else
			{
				UE_VLOG(MyController, LogHTNPlanner, Warning, TEXT("UHTNTask_MoveTo::PerformMoveTask() TargetBlackboardKey Type is neither Vector nor UObject!"));
			}
		}
		else if(TargetActor)
		{
			MoveReq.SetGoalActor(TargetActor);

			RequestResult = MyController->MoveTo(MoveReq);
		}
		else if(TargetLocation != FAISystem::InvalidLocation)
		{
			MoveReq.SetGoalLocation(TargetLocation);

			RequestResult = MyController->MoveTo(MoveReq);
		}

		if(RequestResult == EPathFollowingRequestResult::RequestSuccessful)
		{
			const FAIRequestID RequestID = MyController->GetCurrentMoveRequestID();

			((FHTNTask_MoveToMemory*)TaskMemory)->MoveRequestID = RequestID;
			WaitForMessage(HTNComp, UBrainComponent::AIMessage_MoveFinished, RequestID, TaskMemory);
			WaitForMessage(HTNComp, UBrainComponent::AIMessage_RepathFailed, TaskMemory);

			Result = EHTNExecutionResult::InProgress;
		}
		else if(RequestResult == EPathFollowingRequestResult::AlreadyAtGoal)
		{
			Result = EHTNExecutionResult::Succeeded;
		}
	}

	return Result;
}

void UHTNTask_MoveTo::TickTask(UHTNPlannerComponent& HTNComp, float DeltaSeconds, uint8* TaskMemory)
{
	FHTNTask_MoveToMemory* Memory = (FHTNTask_MoveToMemory*)TaskMemory;
	if(Memory->bWaitingForPath && !HTNComp.IsPaused())
	{
		AAIController* MyController = HTNComp.GetAIOwner();
		if(MyController && !MyController->ShouldPostponePathUpdates())
		{
			UE_VLOG(MyController, LogHTNPlanner, Log, TEXT("Pathfinding requests are unlocked!"));
			Memory->bWaitingForPath = false;

			const EHTNExecutionResult Result = PerformMoveTask(HTNComp, TaskMemory);
			if(Result != EHTNExecutionResult::InProgress)
			{
				FinishLatentTask(HTNComp, Result, TaskMemory);
			}
		}
	}
}