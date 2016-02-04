#pragma once

#include "HTNPlan.generated.h"

struct FHTNTaskInstance;
class UPrimitiveTask;

/**
 * Describes a Plan of Tasks that need to be executed. Can be a partial plan constructed
 * so far during the planning process that may turn out to be incorrect, or can be a completely
 * finalized plan.
 */
USTRUCT(BlueprintType)
struct HTN_PLUGIN_API FHTNPlan
{
	GENERATED_BODY()

public:
	FHTNPlan();

	void AppendSearchHistory(const TSharedPtr<FHTNTaskInstance>& NewTaskInstance);
	void AppendSearchHistory(const TArray<TSharedPtr<FHTNTaskInstance>>& NewTaskInstances);
	void AppendTaskInstance(const TSharedPtr<FHTNTaskInstance>& NewTaskInstance);
	void AppendTaskInstances(const TArray<TSharedPtr<FHTNTaskInstance>>& NewTaskInstances);
	TSharedPtr<FHTNPlan> Copy();
	int32 GetLastStreakScore(const TSharedPtr<FHTNPlan>& OtherPlan, int32 MinStreakLength) const;
	int32 GetLongestMatchingStreak(const TSharedPtr<FHTNPlan>& OtherPlan, int32 MinStreakLength) const;
	int32 GetMatchingCompounds(const TSharedPtr<FHTNPlan>& OtherPlan, const TSharedPtr<FHTNTaskInstance>& TaskNetwork) const;
	int32 GetMatchingStreak(const TSharedPtr<FHTNPlan>& OtherPlan, int32 MinStreakLength) const;
	int32 GetPlanSize() const;
	int32 GetPotentialSimilarity(const TSharedPtr<FHTNPlan>& OtherPlan, const TSharedPtr<FHTNTaskInstance>& TaskNetwork) const;
	int32 GetRemainingPlanSize() const;
	int32 GetSimilarity(const TSharedPtr<FHTNPlan>& OtherPlan, const TSharedPtr<FHTNTaskInstance>& TaskNetwork) const;
	int32 GetStreakScore(const TSharedPtr<FHTNPlan>& OtherPlan, int32 MinStreakLength) const;
	TSharedPtr<FHTNTaskInstance> GetTaskInstanceToExecute();
	const TArray<TSharedPtr<FHTNTaskInstance>>& GetSearchHistory() const;
	const TArray<TSharedPtr<FHTNTaskInstance>>& GetTasks() const;
	void IncrementExecutionIndex();
	bool IsComplete() const;
	void RemoveExecutedTasks(int32 NumTasksExecuted);
	void SetComplete(bool bNewStatus);

	void Print();

protected:
	/** List of primitive Task Instances that need to be executed */
	TArray<TSharedPtr<FHTNTaskInstance>> TaskInstances;

	/** 
	 * List of all the Task Instances (Compound AND Primitive) that were processed in the search tree 
	 * to reach this plan.
	 */
	TArray<TSharedPtr<FHTNTaskInstance>> SearchHistory;

	/** Index of the next Task to execute. */
	int32 ExecutionIndex;
	/** If true, this Plan is complete (the planning process that generated it has finished) */
	bool bComplete;
};