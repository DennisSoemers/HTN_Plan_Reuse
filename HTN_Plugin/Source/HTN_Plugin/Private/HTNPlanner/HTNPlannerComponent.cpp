#include "HTN_PluginPrivatePCH.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "HTNPlanner/CompoundTask.h"
#include "HTNPlanner/HTNPlan.h"
#include "HTNPlanner/HTNPlanner.h"
#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/HTNService.h"
#include "HTNPlanner/HTNTask.h"
#include "HTNPlanner/HTNWorldState.h"
#include "HTNPlanner/PrimitiveTask.h"
#include "HTNPlanner/TaskNetwork.h"

#include "JSHOP2_Experiments/DataCollector.h"

UHTNPlannerComponent::UHTNPlannerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BestPlan = nullptr;
	CurrentPlannerAsset = nullptr;
	ActiveTask = nullptr;
	PendingExecution = nullptr;
	bIsPaused = false;
	bIsRunning = false;
	bRequestedExecutionUpdate = false;
	bRequestedStop = false;
	bWaitingForAbortingTasks = false;
	bWantsInitializeComponent = true;
	PreviousPlan = nullptr;
	NumStackElements = 0;

	DefaultMinMatchingStreakLength = 10;
	MinMatchingStreakLength = DefaultMinMatchingStreakLength;

	ProbabilityIgnorePlanReuse = 0.25f;
	bHitLeaf = false;
	bIgnoringPlanReuse = false;
	bProbabilisticPlanReuse = true;
}

/** Helper struct for sorting Stack Elements according to planning cost */
struct FStackElementSorter
{
	bool operator()(const FHTNStackElement& A, const FHTNStackElement& B) const
	{
		return ((A.Cost + Cast<UTaskNetwork>(A.TaskNetwork->Task)->GetHeuristicCost(A.WorldState, A.TaskNetwork->GetMemory())) <
				(B.Cost + Cast<UTaskNetwork>(B.TaskNetwork->Task)->GetHeuristicCost(B.WorldState, B.TaskNetwork->GetMemory())));
	}
};

void UHTNPlannerComponent::AddStackElement(const FHTNStackElement& StackElement)
{
	++NumStackElements;

	if(bDepthFirstSearch)
	{
		if(PreviousPlan.IsValid())
		{		
			if(bProbabilisticPlanReuse && bIgnoringPlanReuse)
			{
				// we're ignoring plan reuse right now due to probabilistic plan reuse
				PlanningStack.Push(StackElement);
			}
			else if(CurrentMatchingStreakLength > 0 /*&& (!bAdaptivePlanReuse || NumLeaves > 50)*/)
			{
				// this node is continuing a streak
				StreakStacks[CurrentMatchingStreakLength].Push(StackElement);
				MaxCurrentMatchingStreakLength = FMath::Max(MaxCurrentMatchingStreakLength, CurrentMatchingStreakLength);
			}
			else if(MaxCurrentMatchingStreakLength > 0)
			{
				// this node is no longer continuing a streak, but it did previously have a streak
				StreakEndedQueues[MaxCurrentMatchingStreakLength].Enqueue(StackElement);
				MaxEndedMatchingStreakLength = FMath::Max(MaxEndedMatchingStreakLength, 
														  MaxCurrentMatchingStreakLength);
			}
			else if(MaxPastMatchingStreakLength > 0)
			{
				// this node has had some streak in the past, but has had > 1 nodes afterwards that did not match
				PastStreakStacks[MaxPastMatchingStreakLength].Push(StackElement);
			}
			else if(MaxEndedMatchingStreakLength > 0)
			{
				// this node has had some streak in the past, but has had exactly 1 node afterwards that did not match
				PastStreakStacks[MaxEndedMatchingStreakLength].Push(StackElement);
				MaxPastMatchingStreakLength = MaxEndedMatchingStreakLength;
			}
			else
			{
				// this node has never had a matching streak
				PlanningStack.Push(StackElement);
			}
		}
		else
		{
			PlanningStack.Push(StackElement);
		}
	}
	else
	{
		PlanningStack.HeapPush(StackElement, FStackElementSorter());
	}
}

FHTNStackElement UHTNPlannerComponent::PopStackElement()
{
	--NumStackElements;

	if(bDepthFirstSearch)
	{
		if(PreviousPlan.IsValid())
		{
			if(PlanningStack.Num() == 0)
			{
				bIgnoringPlanReuse = false;	// cant ignore plan reuse if the stack without matching streaks is empty
			}

			if(bProbabilisticPlanReuse && bIgnoringPlanReuse)
			{
				// ignoring plan reuse, so pop from the stack of nodes without matching streaks
				return PlanningStack.Pop(false);
			}
			else if(MaxCurrentMatchingStreakLength > 0)
			{
				// we can pop a node that's currently in a matching streak
				return StreakStacks[MaxCurrentMatchingStreakLength].Pop(false);
			}
			else if(MaxPastMatchingStreakLength > 0)
			{
				// we can pop a node that has had some streak in the past, but has had > 1 nodes afterwards that did not match
				return PastStreakStacks[MaxPastMatchingStreakLength].Pop(false);
			}
			else if(MaxEndedMatchingStreakLength > 0)
			{
				// we can pop a node that has had some streak in the past, but has had exactly 1 node afterwards that did not match
				FHTNStackElement Output;
				StreakEndedQueues[MaxEndedMatchingStreakLength].Dequeue(Output);
				return Output;
			}
			else
			{
				// we can pop a node that has never had a matching streak
				return PlanningStack.Pop(false);
			}
		}
		else
		{
			return PlanningStack.Pop(false);
		}
	}
	else
	{
		FHTNStackElement Output;
		PlanningStack.HeapPop(Output, FStackElementSorter());
		return Output;
	}
}

void UHTNPlannerComponent::Cleanup()
{
	StopPlanner();
	BestPlan = nullptr;
	CurrentPlannerAsset = nullptr;
	ActiveTask = nullptr;
	PendingExecution = nullptr;
	PlanningStack.Empty();
	PastStreakStacks.Empty();
	StreakEndedQueues.Empty();
	StreakStacks.Empty();
	NumStackElements = 0;
}

TSharedPtr<FHTNPlan> UHTNPlannerComponent::GetBestPlan() const
{
	return BestPlan;
}

void UHTNPlannerComponent::ExecuteTask(TSharedPtr<FHTNTaskInstance> TaskInstance)
{
	//UE_VLOG(GetOwner(), LogHTNPlanner, Log, TEXT("Execute task: %s"), 
	//		*UHTNPlannerTypes::DescribeTaskHelper(TaskInstance->Task, TaskInstance->GetMemory()));
	//GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, 
	//									FString::Printf(TEXT("UHTNPlannerComponent::ExecuteTask(): %s"), 
	//									*UHTNPlannerTypes::DescribeTaskHelper(TaskInstance->Task, TaskInstance->GetMemory())));
	PendingExecution = nullptr;

	if(UPrimitiveTask* PrimitiveTask = Cast<UPrimitiveTask>(TaskInstance->Task))
	{
		ActiveTask = TaskInstance;
		EHTNExecutionResult TaskResult = PrimitiveTask->ExecuteTask(*this, TaskInstance->GetMemory());
		if(ActiveTask == TaskInstance)
		{
			// this means OnTaskFinished() hasn't been called yet
			OnTaskFinished(TaskInstance, TaskResult);
		}
	}
}

float UHTNPlannerComponent::GetBestCost() const
{
	return BestCost;
}

FString UHTNPlannerComponent::GetDebugInfoString() const
{
	return FString::Printf(TEXT("HTN Planner: %s\n"), *GetNameSafe(CurrentPlannerAsset));
}

EHTNTaskStatus UHTNPlannerComponent::GetTaskStatus(const TSharedPtr<FHTNTaskInstance>& TaskInstance) const
{
	if(TaskInstance == ActiveTask)
	{
		return bWaitingForAbortingTasks ? EHTNTaskStatus::Aborting : EHTNTaskStatus::Active;
	}
	else
	{
		return EHTNTaskStatus::Inactive;
	}
}

#if HTN_LOG_RUNTIME_STATS
double UHTNPlannerComponent::GetTimeSearched() const
{
	return CumulativeSearchTimeMs;
}
#endif // HTN_LOG_RUNTIME_STATS

TSharedPtr<FHTNTaskInstance> UHTNPlannerComponent::InstantiateNetwork(const TSubclassOf<UTaskNetwork>& NetworkClass)
{
	if(*NetworkClass)
	{
		UTaskNetwork* NetworkCDO = Cast<UTaskNetwork>(NetworkClass->GetDefaultObject());
		TSharedPtr<FHTNTaskInstance> NewInstance = TSharedPtr<FHTNTaskInstance>(new FHTNTaskInstance
																				(NetworkCDO, NetworkCDO->GetInstanceMemorySize()));
		NewInstance->Task->Instantiate(*this, NewInstance->GetMemory());
		return NewInstance;
	}

	return TSharedPtr<FHTNTaskInstance>(nullptr);
}

TSharedPtr<FHTNTaskInstance> UHTNPlannerComponent::InstantiateTask(const TSubclassOf<UHTNTask>& TaskClass)
{
	if(*TaskClass)
	{
		UHTNTask* TaskCDO = Cast<UHTNTask>(TaskClass->GetDefaultObject());
		TSharedPtr<FHTNTaskInstance> NewInstance = TSharedPtr<FHTNTaskInstance>(new FHTNTaskInstance(TaskCDO, TaskCDO->GetInstanceMemorySize()));
		NewInstance->Task->Instantiate(*this, NewInstance->GetMemory());
		return NewInstance;
	}

	return TSharedPtr<FHTNTaskInstance>(nullptr);
}

bool UHTNPlannerComponent::IsRunning() const
{
	return (bIsPaused == false && PlannerHasBeenStarted() == true);
}

bool UHTNPlannerComponent::IsPaused() const
{
	return bIsPaused;
}

void UHTNPlannerComponent::OnTaskFinished(UPrimitiveTask* Task, uint8* TaskMemory, EHTNExecutionResult TaskResult)
{
	if(ActiveTask->Task == Task && ActiveTask->GetMemory() == TaskMemory)
	{
		OnTaskFinished(ActiveTask, TaskResult);
	}
}

void UHTNPlannerComponent::OnTaskFinished(TSharedPtr<FHTNTaskInstance> TaskInstance, EHTNExecutionResult TaskResult)
{
	if(TaskInstance->Task == nullptr || CurrentPlannerAsset == nullptr || IsPendingKill())
	{
		return;
	}

	UE_VLOG(GetOwner(), LogHTNPlanner, Log, TEXT("Task %s finished: %s"),
			*UHTNPlannerTypes::DescribeTaskHelper(TaskInstance->Task, TaskInstance->GetMemory()), 
			*UHTNPlannerTypes::DescribeTaskResult(TaskResult));

	bWaitingForAbortingTasks = false;	// TO DO should this be moved down into the if?

	if(TaskResult != EHTNExecutionResult::InProgress)
	{
		// cleanup task observers
		UnregisterMessageObserversFrom(TaskInstance);

		UPrimitiveTask* PrimitiveTask = Cast<UPrimitiveTask>(TaskInstance->Task);
		PrimitiveTask->OnTaskFinished(*this, TaskResult, TaskInstance->GetMemory());

		// update execution if the task we were executing for plan is finished
		if(ActiveTask == TaskInstance)
		{
			ActiveTask = nullptr;
			if(BestPlan.IsValid())
			{
				BestPlan->IncrementExecutionIndex();
				PendingExecution = BestPlan->GetTaskInstanceToExecute();

				if(!bRequestedStop)
				{
					ScheduleExecutionUpdate();
				}
			}
		}
	}
}

void UHTNPlannerComponent::PauseLogic(const FString& Reason)
{
	UE_VLOG(GetOwner(), LogHTNPlanner, Log, TEXT("Execution updates: PAUSED (%s)"), *Reason);
	bIsPaused = true;

	if(BlackboardComp)
	{
		BlackboardComp->PauseUpdates();
	}
}

bool UHTNPlannerComponent::PlannerHasBeenStarted() const
{
	return (bIsRunning && CurrentPlannerAsset != nullptr);
}

void UHTNPlannerComponent::ProcessExecutionRequest()
{
	bRequestedExecutionUpdate = false;
	if(!IsRegistered())
	{
		// it shouldn't be called, component is no longer valid
		return;
	}

	if(bIsPaused)
	{
		UE_VLOG(GetOwner(), LogHTNPlanner, Verbose, TEXT("Ignoring ProcessExecutionRequest call due to HTNPlannerComponent still being paused"));
		return;
	}

	//GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, TEXT("UHTNPlannerComponent::ProcessExecutionRequest()"));

	if(PendingExecution.IsValid())
	{
		ProcessPendingExecution();
		return;
	}

	if(NumStackElements == 0)
	{
		//BestPlan = nullptr;

		if(CurrentPlannerAsset->bLoop)
		{
			// finished execution of plan and we want to loop, so re-start
			RestartPlanner();
		}
		else
		{
			bIsRunning = false;
		}

		return;
	}

#if HTN_LOG_RUNTIME_STATS
	if(StartPlanningTime == FDateTime::MinValue())
	{
		StartPlanningTime = FDateTime::UtcNow();
	}
#endif // HTN_LOG_RUNTIME_STATS

	FDateTime PlanningStart = FDateTime::UtcNow();
	while(NumStackElements > 0)
	{
		// our stack is not empty
		FTimespan TimePlanned = FDateTime::UtcNow() - PlanningStart;
		if(TimePlanned.GetTotalSeconds() >= CurrentPlannerAsset->MaxSearchTime)
		{
			// we've spent our maximum allowed search time for this tick on planning, so need to continue some other time
			ScheduleExecutionUpdate();

#if HTN_LOG_RUNTIME_STATS
			CumulativeSearchTimeMs += TimePlanned.GetTotalMilliseconds();
			++CumulativeFrameCount;
#endif
			return;
		}

#if HTN_LOG_RUNTIME_STATS
		++NumNodesExpanded;
#endif

		if(PreviousPlan.IsValid())
		{
			UE_LOG(LogHTNPlanner, Warning, TEXT("%d nodes in data structure(s)"), NumStackElements);
		}

		if(bProbabilisticPlanReuse && bHitLeaf)
		{
			// we've hit a leaf node, so it's time to re-evaluate whether we're ignoring plan reuse probabilistically
			bHitLeaf = false;

			if(NumStackElements == PlanningStack.Num())
			{
				bIgnoringPlanReuse = false;		// not ignoring plan reuse if everything's still in the non-prioritized stack
			}
			else
			{
				bIgnoringPlanReuse = (FMath::FRand() <= ProbabilityIgnorePlanReuse);
			}
		}

		const FHTNStackElement StackTop = PopStackElement();
		
		if(StackTop.Cost + Cast<UTaskNetwork>(StackTop.TaskNetwork->Task)->
		   GetHeuristicCost(StackTop.WorldState, StackTop.TaskNetwork->GetMemory()) >= BestCost)
		{
			if(!bDepthFirstSearch)
			{
				// everything remaining in the heap will be at least as bad, and maybe worse
				PlanningStack.Empty();
				NumStackElements = 0;
			}

			if(PreviousPlan.IsValid())	// we're doing plan reuse
			{
				// verify that all of our values of maximum streak lengths among unprocessed nodes are still correct
				UpdateMaxStreakLengths();
			}

			continue;	// we won't find any improvements down this path
		}

		UTaskNetwork* TopNetwork = Cast<UTaskNetwork>(StackTop.TaskNetwork->Task);
		if(TopNetwork->IsEmpty(StackTop.TaskNetwork->GetMemory()))
		{
			// we've found a path leading to a legal, complete Plan
#if HTN_LOG_RUNTIME_STATS
			CumulativeSearchTimeMsTillLastImprovement = CumulativeSearchTimeMs + (FDateTime::UtcNow() - PlanningStart).GetTotalMilliseconds();

			if(BestCost == TNumericLimits<float>::Max())	// this means that this is the first time we find a valid plan
			{
				CumulativeSearchTimeMsTillFirstPlan = CumulativeSearchTimeMsTillLastImprovement;
				FirstPlanCost = StackTop.Cost;
			}
#endif
			BestPlan = StackTop.Plan;
			BestPlan->SetComplete(true);
			BestCost = StackTop.Cost;

#if HTN_LOG_RUNTIME_STATS
			DataCollector->FoundSolution(BestCost, StackTop.Plan->GetPlanSize(), StackTop.Plan->GetSearchHistory().Num(),
										 CumulativeSearchTimeMsTillLastImprovement, NumNodesExpanded);
#endif

			if(CurrentPlannerAsset->bIgnoreTaskCosts)
			{
				// the HTN Planner doesn't care about finding optimal plans, only about finding the first one
				PlanningStack.Empty();
				NumStackElements = 0;
			}
			else if(!bDepthFirstSearch)
			{
				// best-first search finds an optimal solution as first solution
				PlanningStack.Empty();
				NumStackElements = 0;
			}

			if(PreviousPlan.IsValid())	// we're doing plan reuse
			{
				if(bProbabilisticPlanReuse)
				{
					bHitLeaf = true;
				}

				// verify that all of our values of maximum streak lengths among unprocessed nodes are still correct
				UpdateMaxStreakLengths();
			}

			continue;
		}

		// find all tasks that share the highest priority amongst the tasks in the task network
		TArray<TSharedPtr<FHTNTaskInstance>> TaskInstances = TopNetwork->FindTasksWithoutPredecessors(StackTop.TaskNetwork->GetMemory());
		for(const TSharedPtr<FHTNTaskInstance>& TaskInstance : TaskInstances)
		{
			UHTNTask* Task = TaskInstance->Task;
			if(UPrimitiveTask* PrimitiveTask = Cast<UPrimitiveTask>(Task))
			{				
				if(PrimitiveTask->IsApplicable(StackTop.WorldState, TaskInstance->GetMemory()))
				{
					// prepare a new element for the stack where we'll have applied this primitive task
					FHTNStackElement NewStackElement;
					NewStackElement.Cost = 
						FMath::Max(0.f, StackTop.Cost) + PrimitiveTask->GetCost(StackTop.WorldState, TaskInstance->GetMemory());

					TSharedPtr<FHTNPlan> Plan = StackTop.Plan->Copy();
					Plan->AppendTaskInstance(TaskInstance);
					Plan->AppendSearchHistory(TaskInstance);
					TSharedPtr<FHTNTaskInstance> TaskNetwork = TopNetwork->Copy(StackTop.TaskNetwork);
					Cast<UTaskNetwork>(TaskNetwork->Task)->Remove(TaskInstance, TaskNetwork->GetMemory());
					TSharedPtr<FHTNWorldState> WorldState = StackTop.WorldState->Copy();
					PrimitiveTask->ApplyTo(WorldState, TaskInstance->GetMemory());

					NewStackElement.Plan = Plan;
					NewStackElement.TaskNetwork = TaskNetwork;
					NewStackElement.WorldState = WorldState;

					if(PreviousPlan.IsValid())
					{
						// we're doing plan reuse
						CurrentMatchingStreakLength = Plan->GetMatchingStreak(PreviousPlan, MinMatchingStreakLength);
					}

					AddStackElement(NewStackElement);	// TO DO maybe should explicitly Move the NewStackElement?
				}
				else if(PreviousPlan.IsValid())
				{
					if(bProbabilisticPlanReuse)
					{
						bHitLeaf = true;
					}
				}
			}
			else if(UCompoundTask* CompoundTask = Cast<UCompoundTask>(Task))
			{
				TArray<TSharedPtr<FHTNTaskInstance>> Decompositions = CompoundTask->FindDecompositions(*this, StackTop.WorldState, 
																									   TaskInstance->GetMemory());

				// regardless of which decomposition we pick, effect on plan will be the same
				TSharedPtr<FHTNPlan> Plan = StackTop.Plan->Copy();
				Plan->AppendSearchHistory(TaskInstance);

				if(PreviousPlan.IsValid())
				{
					// we're doing plan reuse

					if(Decompositions.Num() == 0)	// leaf node
					{
						if(bProbabilisticPlanReuse)
						{
							bHitLeaf = true;
						}
					}

					CurrentMatchingStreakLength = Plan->GetMatchingStreak(PreviousPlan, MinMatchingStreakLength);

					TArray<FHTNStackElement> NewStackElements;
					TArray<FHTNStackElement> NewFifoElements;

					for(int32 Idx = 0; Idx < Decompositions.Num(); ++Idx)
					{
						const TSharedPtr<FHTNTaskInstance>& Decomposition = Decompositions[Idx];

						// prepare a new element where we'll have decomposed this compound task
						FHTNStackElement NewStackElement;
						NewStackElement.WorldState = StackTop.WorldState;
						NewStackElement.Cost = StackTop.Cost;

						TSharedPtr<FHTNTaskInstance> TaskNetwork = TopNetwork->Copy(StackTop.TaskNetwork);
						Cast<UTaskNetwork>(TaskNetwork->Task)->Replace(TaskInstance, Decomposition, TaskNetwork->GetMemory());

						NewStackElement.Plan = Plan->Copy();
						NewStackElement.TaskNetwork = TaskNetwork;

						if(bProbabilisticPlanReuse && bIgnoringPlanReuse)
						{
							// probabilistically ignoring plan reuse, so no need to waste time computing streak lengths
							NewStackElements.Push(NewStackElement);
						}
						else
						{
							if(MaxCurrentMatchingStreakLength > 0 && CurrentMatchingStreakLength == 0)
							{
								// this element belongs in a FIFO queue
								NewFifoElements.Push(NewStackElement);
							}
							else
							{
								// this element belongs in a stack
								NewStackElements.Push(NewStackElement);
							}
						}
					}

					// first we'll add all the elements that belong in FIFO queues to their FIFO queues (in given order)
					for(int32 Idx = 0; Idx < NewFifoElements.Num(); ++Idx)
					{
						AddStackElement(NewFifoElements[Idx]);
					}

					// now we'll add all the elements that belong in some stack to those stacks (reverse order)
					while(NewStackElements.Num() > 0)
					{
						AddStackElement(NewStackElements.Pop());
					}
				}
				else
				{
					// we're not doing plan reuse
					// looping through Decompositions in reverse order so that they'll be popped off stack in correct order again
					for(int32 Idx = Decompositions.Num() - 1; Idx >= 0; --Idx)
					{
						const TSharedPtr<FHTNTaskInstance>& Decomposition = Decompositions[Idx];

						// prepare a new element for the stack where we'll have decomposed this compound task
						FHTNStackElement NewStackElement;
						NewStackElement.WorldState = StackTop.WorldState;
						NewStackElement.Cost = StackTop.Cost;

						TSharedPtr<FHTNTaskInstance> TaskNetwork = TopNetwork->Copy(StackTop.TaskNetwork);
						Cast<UTaskNetwork>(TaskNetwork->Task)->Replace(TaskInstance, Decomposition, TaskNetwork->GetMemory());

						NewStackElement.Plan = Plan->Copy();
						NewStackElement.TaskNetwork = TaskNetwork;

						AddStackElement(NewStackElement);	// TO DO maybe should explicitly Move the NewStackElement?
					}
				}
			}
			else
			{
				UE_LOG(LogHTNPlanner, Error, TEXT("UHTNPlannerComponent::ProcessExecutionRequest() encountered a Task that was neither Primitive nor Compound!"))
			}
		}

		if(PreviousPlan.IsValid())	// we're doing plan reuse
		{
			// verify that all of our values of maximum streak lengths among unprocessed nodes are still correct
			UpdateMaxStreakLengths();
		}
	}

	if(BestPlan.IsValid())
	{
#if HTN_LOG_RUNTIME_STATS
		CumulativeSearchTimeMs += (FDateTime::UtcNow() - PlanningStart).GetTotalMilliseconds();
		++CumulativeFrameCount;

		CumulativeSearchTimespan = FDateTime::UtcNow() - StartPlanningTime;

		// print runtime stats
		//UE_LOG(LogHTNPlanner, Warning, TEXT("Cumulative Search Timespan = %.2f ms"), CumulativeSearchTimespan.GetTotalMilliseconds());
		UE_LOG(LogHTNPlanner, Warning, TEXT("Cumulative Search Time = %.2f ms"), CumulativeSearchTimeMs);
		UE_LOG(LogHTNPlanner, Warning, TEXT("Cumulative Search Time Till Last Improvement = %.2f ms"), CumulativeSearchTimeMsTillLastImprovement);
		UE_LOG(LogHTNPlanner, Warning, TEXT("Cumulative Search Time First Plan = %.2f ms"), CumulativeSearchTimeMsTillFirstPlan);
		//UE_LOG(LogHTNPlanner, Warning, TEXT("Cumulative Frame Count = %d frames"), CumulativeFrameCount);
		UE_LOG(LogHTNPlanner, Warning, TEXT("Num Nodes Expanded = %d"), NumNodesExpanded);
		UE_LOG(LogHTNPlanner, Warning, TEXT("Cost of first plan found = %.2f"), FirstPlanCost);

		if(PreviousPlan.IsValid())
		{
			UE_LOG(LogHTNPlanner, Warning, TEXT("Longest matching streak = %d"), 
				   BestPlan->GetLongestMatchingStreak(PreviousPlan, MinMatchingStreakLength));
		}

		DataCollector->OptimalityProven(CumulativeSearchTimeMs, NumNodesExpanded);
#endif

		// we have a complete plan, so we'll want to execute the next task in the plan
		UE_LOG(LogHTNPlanner, Warning, TEXT("Found Plan with size = %d, cost = %.2f, search history size = %d!"), 
			   BestPlan->GetPlanSize(), BestCost, BestPlan->GetSearchHistory().Num());
		if(CurrentPlannerAsset->bExecutePlan)
		{
			PendingExecution = BestPlan->GetTaskInstanceToExecute();
			ProcessPendingExecution();
		}
		else
		{
			bIsRunning = false;
		}
	}
}

void UHTNPlannerComponent::ProcessPendingExecution()
{
	//GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, TEXT("UHTNPlannerComponent::ProcessPendingExecution()"));

	// can't continue if current task is still aborting
	if(bWaitingForAbortingTasks || !PendingExecution.IsValid())
	{
		//GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, TEXT("Returning in UHTNPlannerComponent::ProcessPendingExecution()"));
		if(!PendingExecution.IsValid())
		{
			bIsRunning = false;
		}

		return;
	}

	ExecuteTask(PendingExecution);
}

void UHTNPlannerComponent::RegisterMessageObserver(const UPrimitiveTask* const Task, uint8* TaskMemory, FName MessageType)
{
	if(ActiveTask->Task == Task && ActiveTask->GetMemory() == TaskMemory)
	{
		RegisterMessageObserver(ActiveTask, MessageType);
	}
}

void UHTNPlannerComponent::RegisterMessageObserver(const UPrimitiveTask* const Task, uint8* TaskMemory, FName MessageType, FAIRequestID RequestID)
{
	if(ActiveTask->Task == Task && ActiveTask->GetMemory() == TaskMemory)
	{
		RegisterMessageObserver(ActiveTask, MessageType, RequestID);
	}
}

void UHTNPlannerComponent::RegisterMessageObserver(const TSharedPtr<FHTNTaskInstance>& TaskInstance, FName MessageType)
{
	if(TaskInstance.IsValid())
	{
		if(UPrimitiveTask* PrimitiveTask = Cast<UPrimitiveTask>(TaskInstance->Task))
		{
			TaskMessageObservers.Add(FTaskMessageObserver(
				FAIMessageObserver::Create(this, MessageType,
				FOnAIMessage::CreateUObject(PrimitiveTask, &UPrimitiveTask::ReceivedMessage, TaskInstance)),
				TaskInstance)
				);

			UE_VLOG(GetOwner(), LogHTNPlanner, Log, TEXT("Message[%s] observer added for %s"),
					*MessageType.ToString(), *UHTNPlannerTypes::DescribeTaskHelper(TaskInstance->Task, TaskInstance->GetMemory()));
		}
	}
}

void UHTNPlannerComponent::RegisterMessageObserver(const TSharedPtr<FHTNTaskInstance>& TaskInstance, FName MessageType, FAIRequestID RequestID)
{
	if(TaskInstance.IsValid())
	{
		if(UPrimitiveTask* PrimitiveTask = Cast<UPrimitiveTask>(TaskInstance->Task))
		{
			TaskMessageObservers.Add(FTaskMessageObserver(
				FAIMessageObserver::Create(this, MessageType, RequestID,
				FOnAIMessage::CreateUObject(PrimitiveTask, &UPrimitiveTask::ReceivedMessage, TaskInstance)),
				TaskInstance)
				);

			UE_VLOG(GetOwner(), LogHTNPlanner, Log, TEXT("Message[%s:%d] observer added for %s"),
					*MessageType.ToString(), RequestID, *UHTNPlannerTypes::DescribeTaskHelper(TaskInstance->Task, TaskInstance->GetMemory()));
		}
	}
}

void UHTNPlannerComponent::RegisterService(const TSubclassOf<UHTNService>& Service)
{
	if(*Service)
	{
		UHTNService* ServiceObject = NewObject<UHTNService>(this, *Service);
		ServiceObject->SetOwner(GetOwner());
		RegisteredServices.Add(ServiceObject);
	}
}

void UHTNPlannerComponent::RestartLogic()
{
	UE_VLOG(GetOwner(), LogHTNPlanner, Log, TEXT("UHTNPlannerComponent::RestartLogic"));
	RestartPlanner();
}

void UHTNPlannerComponent::RestartPlanner()
{
	UE_VLOG(GetOwner(), LogHTNPlanner, Log, TEXT("UHTNPlannerComponent::RestartPlanner"));

	if(!bIsRunning)
	{
		bIsRunning = (CurrentPlannerAsset != nullptr);
	}
	else if(CurrentPlannerAsset)
	{
		StopPlanner();
		StartPlanner(*CurrentPlannerAsset);
	}
}

EAILogicResuming::Type UHTNPlannerComponent::ResumeLogic(const FString& Reason)
{
	const EAILogicResuming::Type SuperResumeResult = Super::ResumeLogic(Reason);
	if(bIsPaused)
	{
		bIsPaused = false;

		if(SuperResumeResult == EAILogicResuming::Continue)
		{
			if(BlackboardComp)
			{
				BlackboardComp->ResumeUpdates();
			}

			ScheduleExecutionUpdate();

			return EAILogicResuming::Continue;
		}
	}

	return SuperResumeResult;
}

void UHTNPlannerComponent::ScheduleExecutionUpdate()
{
	bRequestedExecutionUpdate = true;
}

void UHTNPlannerComponent::StartPlanner(UHTNPlanner& Asset)
{
	if(CurrentPlannerAsset == &Asset && PlannerHasBeenStarted())
	{
		UE_VLOG(GetOwner(), LogHTNPlanner, Log, TEXT("Skipping HTN Planner start request - it's already running"));
		return;
	}
	else if(CurrentPlannerAsset)
	{
		UE_VLOG(GetOwner(), LogHTNPlanner, Log, TEXT("Abandoning HTN Planner %s to start new one (%s)"),
				*GetNameSafe(CurrentPlannerAsset), *Asset.GetName());
	}

	StopPlanner();
	bDepthFirstSearch = Asset.bDepthFirstSearch;

	// reset and initialize the stack
	PlanningStack.Reset(10);	// probably safe to start with space for 10 elements, TODO could make this a very advanced parameter?
	BestPlan = MakeShareable<FHTNPlan>(new FHTNPlan());
	BestCost = TNumericLimits<float>::Max();

	if(PreviousPlan.IsValid())
	{
		// we have a valid previous plan to re-use
		// should initialize the number of buckets to the max similarity value + 1 (because 0 index), 
		// which is size of previous plan's search history
		int32 PreviousPlanSearchHistorySize = PreviousPlan->GetSearchHistory().Num();

		PastStreakStacks.Reset(PreviousPlanSearchHistorySize + 1);
		PastStreakStacks.AddDefaulted(PreviousPlanSearchHistorySize + 1);
		StreakEndedQueues.Reset(PreviousPlanSearchHistorySize + 1);
		StreakEndedQueues.AddDefaulted(PreviousPlanSearchHistorySize + 1);
		StreakStacks.Reset(PreviousPlanSearchHistorySize + 1);
		StreakStacks.AddDefaulted(PreviousPlanSearchHistorySize + 1);

		CurrentMatchingStreakLength = 0;
		MaxCurrentMatchingStreakLength = 0;
		MaxEndedMatchingStreakLength = 0;
		MaxPastMatchingStreakLength = 0;

		NumLeaves = 0;
		TotalLeafStreakLengths = 0;

		bHitLeaf = false;
		bIgnoringPlanReuse = false;

		MinMatchingStreakLength = DefaultMinMatchingStreakLength;
		//MinMatchingStreakLength = (int32)(PreviousPlanSearchHistorySize * 0.15);
	}

	TSharedPtr<FHTNWorldState> InitialWorldState;
	if(Asset.EmptyWorldState.IsValid())
	{
		InitialWorldState = Asset.EmptyWorldState->Copy();
	}
	else if(EmptyWorldState.IsValid())
	{
		InitialWorldState = EmptyWorldState->Copy();
	}
	else
	{
		UE_LOG(LogHTNPlanner, Warning, TEXT("No valid initial world state has been set in either the HTNPlanner asset or the HTNPlannerComponent!"));
		StopPlanner();
		return;
	}
	
	InitialWorldState->Initialize(GetOwner(), BlackboardComp);

	FHTNStackElement FirstStackElement;
	FirstStackElement.Plan = BestPlan;
	FirstStackElement.TaskNetwork = (*(Asset.TaskNetwork)) ? InstantiateNetwork(Asset.TaskNetwork) : Asset.TaskNetworkInstance;
	FirstStackElement.WorldState = InitialWorldState;
	FirstStackElement.Cost = 0.f;

	AddStackElement(FirstStackElement);	// TO DO maybe should explicitly Move the FirstStackElement?

	// prepare to start the planning process
	CurrentPlannerAsset = &Asset;
	bIsRunning = true;
	ScheduleExecutionUpdate();

#if HTN_LOG_RUNTIME_STATS
	StartPlanningTime = FDateTime::MinValue();
	CumulativeSearchTimespan = FTimespan::Zero();
	CumulativeSearchTimeMs = 0.0;
	CumulativeSearchTimeMsTillLastImprovement = 0.0;
	CumulativeSearchTimeMsTillFirstPlan = 0.0;
	CumulativeFrameCount = 0;
	NumNodesExpanded = 0;
	FirstPlanCost = TNumericLimits<float>::Max();
#endif // HTN_LOG_RUNTIME_STATS
}

void UHTNPlannerComponent::StopLogic(const FString& Reason)
{
	UE_VLOG(GetOwner(), LogHTNPlanner, Log, TEXT("Stopping HTN Planner, reason: \'%s\'"), *Reason);
	StopPlanner();
}

void UHTNPlannerComponent::StopPlanner(EHTNStopMode StopMode)
{
	if(!bRequestedStop)
	{
		bRequestedStop = true;

		if(ActiveTask.IsValid())
		{
			// remove all observers before requesting abort
			UnregisterMessageObserversFrom(ActiveTask);
			UE_VLOG(GetOwner(), LogHTNPlanner, Log, TEXT("Abort task: %s"), 
					*UHTNPlannerTypes::DescribeTaskHelper(ActiveTask->Task, ActiveTask->GetMemory()));

			if(UPrimitiveTask* PrimitiveTask = Cast<UPrimitiveTask>(ActiveTask->Task))
			{
				EHTNExecutionResult TaskResult = PrimitiveTask->AbortTask(*this, ActiveTask->GetMemory());

				if(TaskResult == EHTNExecutionResult::InProgress)
				{
					bWaitingForAbortingTasks = true;
				}

				if(ActiveTask.IsValid())
				{
					OnTaskFinished(ActiveTask, TaskResult);
				}
			}
		}
	}

	if(bWaitingForAbortingTasks)
	{
		if(StopMode == EHTNStopMode::Safe)
		{
			UE_VLOG(GetOwner(), LogHTNPlanner, Log, TEXT("StopPlanner is waiting for aborting tasks to finish..."));
			UE_LOG(LogHTNPlanner, Warning, TEXT("StopPlanner is waiting for aborting tasks to finish..."));
			return;
		}

		UE_VLOG(GetOwner(), LogHTNPlanner, Warning, TEXT("StopPlanner was forced while waiting for tasks to finish aborting!"));
	}

	ActiveTask = nullptr;
	PendingExecution = nullptr;
	PlanningStack.Empty();
	PastStreakStacks.Empty();
	StreakEndedQueues.Empty();
	StreakStacks.Empty();
	RegisteredServices.Empty();
	TaskMessageObservers.Empty();
	NumStackElements = 0;
	
	// make sure to allow new execution requests
	bRequestedExecutionUpdate = false;
	bRequestedStop = false;
	bIsRunning = false;
}

void UHTNPlannerComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	check(this != nullptr && this->IsPendingKill() == false);

	if(bRequestedExecutionUpdate)
	{
		ProcessExecutionRequest();
	}

	if(CurrentPlannerAsset == nullptr || !bIsRunning)
	{
		return;
	}

	// tick active services
	for(UHTNService* Service : RegisteredServices)
	{
		Service->Tick(*this, DeltaTime);
	}

	// tick active task
	if(ActiveTask.IsValid())
	{
		if(!bIsPaused && !bWaitingForAbortingTasks)
		{
			if(UPrimitiveTask* PrimitiveTask = Cast<UPrimitiveTask>(ActiveTask->Task))
			{
				PrimitiveTask->TickTask(*this, DeltaTime, ActiveTask->GetMemory());
			}
		}
	}
}

void UHTNPlannerComponent::UninitializeComponent()
{
	CurrentPlannerAsset = nullptr;
	Super::UninitializeComponent();
}

void UHTNPlannerComponent::UnregisterMessageObserversFrom(const TSharedPtr<FHTNTaskInstance>& TaskInstance)
{
	if(TaskInstance.IsValid())
	{
		int32 NumRemoved = 0;

		for(int32 Idx = 0; Idx < TaskMessageObservers.Num(); /**/)
		{
			if(TaskMessageObservers[Idx].TaskInstance == TaskInstance)
			{
				// found a message observer registered for our task
				TaskMessageObservers.RemoveAtSwap(Idx, 1);
				++NumRemoved;

				// don't need to increment index since we removed an element
			}
			else
			{
				++Idx;
			}
		}

		UE_VLOG(GetOwner(), LogHTNPlanner, Log, TEXT("Message observers removed for task[%s] (num:%d)"),
				*UHTNPlannerTypes::DescribeTaskHelper(TaskInstance->Task, TaskInstance->GetMemory()), NumRemoved);
	}
}

void UHTNPlannerComponent::UnregisterService(const TSubclassOf<UHTNService>& Service)
{
	if(*Service)
	{
		for(int32 Idx = 0; Idx < RegisteredServices.Num(); ++Idx)
		{
			if(RegisteredServices[Idx]->IsA(*Service))
			{
				RegisteredServices.RemoveAtSwap(Idx, 1);
				return;
			}
		}
	}
}

void UHTNPlannerComponent::UpdateMaxStreakLengths()
{
	while(MaxCurrentMatchingStreakLength > 0)
	{
		if(StreakStacks[MaxCurrentMatchingStreakLength].Num() == 0)
		{
			--MaxCurrentMatchingStreakLength;
		}
		else
		{
			break;
		}
	}

	while(MaxPastMatchingStreakLength > 0)
	{
		if(PastStreakStacks[MaxPastMatchingStreakLength].Num() == 0)
		{
			--MaxPastMatchingStreakLength;
		}
		else
		{
			break;
		}
	}

	while(MaxEndedMatchingStreakLength > 0)
	{
		if(StreakEndedQueues[MaxEndedMatchingStreakLength].IsEmpty())
		{
			--MaxEndedMatchingStreakLength;
		}
		else
		{
			break;
		}
	}
}