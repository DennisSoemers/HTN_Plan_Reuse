#include "HTN_PluginPrivatePCH.h"

#include "HTNPlanner/CompoundTask.h"
#include "HTNPlanner/HTNPlan.h"
#include "HTNPlanner/HTNPlannerTypes.h"
#include "HTNPlanner/PrimitiveTask.h"
#include "HTNPlanner/TaskNetwork.h"

FHTNPlan::FHTNPlan()
	: TaskInstances(), ExecutionIndex(0), bComplete(false)
{}

void FHTNPlan::AppendSearchHistory(const TSharedPtr<FHTNTaskInstance>& NewTaskInstance)
{
	SearchHistory.Add(NewTaskInstance);
}

void FHTNPlan::AppendSearchHistory(const TArray<TSharedPtr<FHTNTaskInstance>>& NewTaskInstances)
{
	SearchHistory.Append(NewTaskInstances);
}

void FHTNPlan::AppendTaskInstance(const TSharedPtr<FHTNTaskInstance>& NewTaskInstance)
{
	TaskInstances.Add(NewTaskInstance);
}

void FHTNPlan::AppendTaskInstances(const TArray<TSharedPtr<FHTNTaskInstance>>& NewTaskInstances)
{
	TaskInstances.Append(NewTaskInstances);
}

TSharedPtr<FHTNPlan> FHTNPlan::Copy()
{
	TSharedPtr<FHTNPlan> Copy = MakeShareable<FHTNPlan>(new FHTNPlan());
	Copy->SetComplete(bComplete);
	Copy->AppendTaskInstances(TaskInstances);
	Copy->AppendSearchHistory(SearchHistory);
	return Copy;
}

int32 FHTNPlan::GetLongestMatchingStreak(const TSharedPtr<FHTNPlan>& OtherPlan, int32 MinStreakLength) const
{
	if(!OtherPlan.IsValid())
	{
		return 0;
	}

	const TArray<TSharedPtr<FHTNTaskInstance>>& MyTasks = GetSearchHistory();
	const TArray<TSharedPtr<FHTNTaskInstance>>& OtherTasks = OtherPlan->GetSearchHistory();

	TArray<TArray<int32>> ScoreMatrix;;
	ScoreMatrix.AddDefaulted(MyTasks.Num());
	for(TArray<int32>& Row : ScoreMatrix)
	{
		Row.AddDefaulted(OtherTasks.Num());
	}

	int32 LongestStreakLength = 0;
	for(int32 MyIdx = 0; MyIdx < MyTasks.Num(); ++MyIdx)
	{
		const TSharedPtr<FHTNTaskInstance>& MyTask = MyTasks[MyIdx];

		for(int32 OtherIdx = 0; OtherIdx < OtherTasks.Num(); ++OtherIdx)
		{
			const TSharedPtr<FHTNTaskInstance>& OtherTask = OtherTasks[OtherIdx];

			if(MyTask->Task == OtherTask->Task && MyTask->GetUint8Memory() == OtherTask->GetUint8Memory())
			{
				// found a match
				if(MyIdx == 0 || OtherIdx == 0)
				{
					ScoreMatrix[MyIdx][OtherIdx] = 1;
				}
				else
				{
					ScoreMatrix[MyIdx][OtherIdx] = ScoreMatrix[MyIdx - 1][OtherIdx - 1] + 1;
				}

				if(ScoreMatrix[MyIdx][OtherIdx] > LongestStreakLength)
				{
					LongestStreakLength = ScoreMatrix[MyIdx][OtherIdx];
				}
			}
		}
	}

	//if(LongestStreakLength < MinStreakLength)
	//{
	//	return 0;
	//}

	return LongestStreakLength;
}

int32 FHTNPlan::GetMatchingStreak(const TSharedPtr<FHTNPlan>& OtherPlan, int32 MinStreakLength) const
{
	if(!OtherPlan.IsValid())
	{
		return 0;
	}

	const TArray<TSharedPtr<FHTNTaskInstance>>& MyTasks = GetSearchHistory();
	const TArray<TSharedPtr<FHTNTaskInstance>>& OtherTasks = OtherPlan->GetSearchHistory();

	// find all the locations in OtherTasks that match the last task of MyTasks
	TArray<int32> MatchingEndIndices;
	const TSharedPtr<FHTNTaskInstance>& MyLastTask = MyTasks[MyTasks.Num() - 1];
	for(int32 OtherIdx = 0; OtherIdx < OtherTasks.Num(); ++OtherIdx)
	{
		const TSharedPtr<FHTNTaskInstance>& OtherTask = OtherTasks[OtherIdx];

		if(MyLastTask->Task == OtherTask->Task && MyLastTask->GetUint8Memory() == OtherTask->GetUint8Memory())
		{
			// found a match
			MatchingEndIndices.Add(OtherIdx);
		}
	}

	// find which of the collected indices results in the longest matching streak, that's the streak we'll assume to be following
	int32 LongestStreakLength = 0;

	for(int32 MatchingEndIndex : MatchingEndIndices)
	{
		int32 StreakLength = 1;
		int32 MyIdx = MyTasks.Num() - 2;
		int32 OtherIdx = MatchingEndIndex - 1;

		while(MyIdx >= 0 && OtherIdx >= 0)
		{
			const TSharedPtr<FHTNTaskInstance>& MyTask = MyTasks[MyIdx];
			const TSharedPtr<FHTNTaskInstance>& OtherTask = OtherTasks[OtherIdx];

			if(MyTask->Task == OtherTask->Task && MyTask->GetUint8Memory() == OtherTask->GetUint8Memory())
			{
				// found a match
				++StreakLength;
				--MyIdx;
				--OtherIdx;
			}
			else
			{
				break;
			}
		}

		LongestStreakLength = FMath::Max(LongestStreakLength, StreakLength);
	}

	if(LongestStreakLength < MinStreakLength)
	{
		return 0;
	}

	return LongestStreakLength;
}

int32 FHTNPlan::GetPlanSize() const
{
	return TaskInstances.Num();
}

int32 FHTNPlan::GetRemainingPlanSize() const
{
	return TaskInstances.Num() - ExecutionIndex;
}

TSharedPtr<FHTNTaskInstance> FHTNPlan::GetTaskInstanceToExecute()
{
	if(TaskInstances.IsValidIndex(ExecutionIndex))
	{
		return TaskInstances[ExecutionIndex];
	}
	else
	{
		return nullptr;
	}
}

const TArray<TSharedPtr<FHTNTaskInstance>>& FHTNPlan::GetSearchHistory() const
{
	return SearchHistory;
}

const TArray<TSharedPtr<FHTNTaskInstance>>& FHTNPlan::GetTasks() const
{
	return TaskInstances;
}

void FHTNPlan::IncrementExecutionIndex()
{
	++ExecutionIndex;
}

bool FHTNPlan::IsComplete() const
{
	return bComplete;
}

void FHTNPlan::RemoveExecutedTasks(int32 NumTasksExecuted)
{
	int32 PrimitiveTasksRemoved = 0;
	int32 LastRemovalIndex = -1;

	// first ignore compound tasks and remove the given number of primitive tasks from our search history
	for(int32 Idx = 0; Idx < SearchHistory.Num(); /**/)
	{
		const TSharedPtr<FHTNTaskInstance>& TaskInstance = SearchHistory[Idx];

		if(TaskInstance->Task->IsA(UPrimitiveTask::StaticClass()))
		{
			SearchHistory.RemoveAt(Idx);
			++PrimitiveTasksRemoved;
			LastRemovalIndex = Idx;

			if(PrimitiveTasksRemoved == NumTasksExecuted)
			{
				break;
			}
		}
		else
		{
			++Idx;
		}
	}

	// remove all compound tasks except for the last one in front of the last primitive task we removed
	SearchHistory.RemoveAt(0, LastRemovalIndex);
}

void FHTNPlan::SetComplete(bool bNewStatus)
{
	bComplete = bNewStatus;
}

void FHTNPlan::Print()
{
	UE_LOG(LogHTNPlanner, Warning, TEXT("["));
	for(const TSharedPtr<FHTNTaskInstance>& TaskInstance : SearchHistory)
	{
		UE_LOG(LogHTNPlanner, Warning, TEXT("%s"), *(TaskInstance->Task->GetTaskName(TaskInstance->GetMemory())));
	}
	UE_LOG(LogHTNPlanner, Warning, TEXT("]"));
}