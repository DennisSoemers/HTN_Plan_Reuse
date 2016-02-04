#pragma once

#include "HTNPlannerTypes.h"
#include "HTNPlannerComponent.generated.h"

struct FHTNPlan;
struct FHTNWorldState;
class UCompoundTask;
class UDataCollector;
class UHTNPlanner;
class UHTNService;
class UHTNTask;
class UPrimitiveTask;
class UTaskNetwork;

/**
 * A single element of the stack used by the HTN Planner during the planning process to
 * memorize progress in between ticks and to allow for backtracking.
 */
struct FHTNStackElement
{
	/** The (partial) plan found so far */
	TSharedPtr<FHTNPlan> Plan;
	/** The Network of Tasks that still need to be done */
	TSharedPtr<FHTNTaskInstance> TaskNetwork;
	/** The World State at the current stage of the planning process */
	TSharedPtr<FHTNWorldState> WorldState;

	/** The cost accumulated so far for executing the Plan */
	float Cost;
};

/**
 * Struct to hold a pair consisting of an AI Message Observer and a Task Instance.
 * Unlike the BehaviorTreeComponent (see it's TaskMessageObservers property), we don't
 * have indices to use for hashing, so it's difficult to use a MultiMap.
 * We also don't expect to have more than one Task Instance running at a time, so we shouldn't need
 * the hashing either.
 */
struct FTaskMessageObserver
{
	FAIMessageObserverHandle MessageObserver;
	const TSharedPtr<FHTNTaskInstance> TaskInstance;

	FTaskMessageObserver(FAIMessageObserverHandle MessageObserver, const TSharedPtr<FHTNTaskInstance>& TaskInstance)
		: MessageObserver(MessageObserver), TaskInstance(TaskInstance)
	{}
};

/**
 * A Brain Component that contains a Hierarchical Task Network (HTN) Planner.
 */
UCLASS()
class HTN_PLUGIN_API UHTNPlannerComponent : public UBrainComponent
{
	GENERATED_BODY()

public:
	UHTNPlannerComponent(const FObjectInitializer& ObjectInitializer);

	/** Returns the cost of the best plan found (so far) */
	float GetBestCost() const;
	/** Returns the best plan found (so far) */
	TSharedPtr<FHTNPlan> GetBestPlan() const;
	/** @return status of speficied task */
	EHTNTaskStatus GetTaskStatus(const TSharedPtr<FHTNTaskInstance>& TaskInstance) const;
	/** Instantiates a new Task Network of the given (Blueprint) type */
	TSharedPtr<FHTNTaskInstance> InstantiateNetwork(const TSubclassOf<UTaskNetwork>& NetworkClass);
	/** Instantiates a new HTN Task of the given (Blueprint) type */
	TSharedPtr<FHTNTaskInstance> InstantiateTask(const TSubclassOf<UHTNTask>& TaskClass);
	/** called when the execution of a latent task has been finished or is aborted */
	void OnTaskFinished(TSharedPtr<FHTNTaskInstance> TaskInstance, EHTNExecutionResult TaskResult);
	void OnTaskFinished(UPrimitiveTask* Task, uint8* TaskMemory, EHTNExecutionResult TaskResult);
	/** process execution of the planner (continue searching for a plan if not found, or execution of plan otherwise) */
	void ProcessExecutionRequest();
	/** set up message observer for given task */
	void RegisterMessageObserver(const TSharedPtr<FHTNTaskInstance>& TaskInstance, FName MessageType);
	void RegisterMessageObserver(const TSharedPtr<FHTNTaskInstance>& TaskInstance, FName MessageType, FAIRequestID MessageID);
	void RegisterMessageObserver(const UPrimitiveTask* const Task, uint8* TaskMemory, FName MessageType);
	void RegisterMessageObserver(const UPrimitiveTask* const Task, uint8* TaskMemory, FName MessageType, FAIRequestID MessageID);
	/** Registers a service of the given type */
	void RegisterService(const TSubclassOf<UHTNService>& Service);
	/** restarts execution of the planner from the initial task */
	void RestartPlanner();
	/** schedule execution update in next tick */
	void ScheduleExecutionUpdate();
	/** starts execution of the planner from the initial task */
	void StartPlanner(UHTNPlanner& Asset);
	/** stops execution */
	void StopPlanner(EHTNStopMode StopMode = EHTNStopMode::Safe);
	/** remove message observers registered with task */
	void UnregisterMessageObserversFrom(const TSharedPtr<FHTNTaskInstance>& TaskInstance);
	/** Unregisters a service of the given type */
	void UnregisterService(const TSubclassOf<UHTNService>& Service);

	// Begin UActorComponent overrides
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void UninitializeComponent() override;
	// End UActorComponent overrides

	// Begin UBrainComponent overrides
	virtual void Cleanup() override;
	virtual FString GetDebugInfoString() const override;
	virtual bool IsRunning() const override;
	virtual bool IsPaused() const override;
	virtual void RestartLogic() override;

	/**
	 * Can be set to some initial World State struct so that the planner knows
	 * what struct type should be used for the World State (cannot use a TSubclassOf
	 * property because they're not compatible with non-UObjects).
	 *
	 * Will be used for initial world state if the EmptyWorldState property is not
	 * set in the HTNPlanner asset.
	 *
	 * Should not already be initialized, only needs to be of the correct type!
	 */
	TSharedPtr<FHTNWorldState> EmptyWorldState;

	/**
	 * Can be set to a previously-found plan.
	 * If not nullptr, will re-use the plan to guide the search and hopefully find a plan more quickly
	 * or find a plan that is more similar to the previous plan.
	 */
	TSharedPtr<FHTNPlan> PreviousPlan;

#if HTN_LOG_RUNTIME_STATS
	double GetTimeSearched() const;
#endif // HTN_LOG_RUNTIME_STATS

	/** Will store data for experiments in here */
	UPROPERTY(transient)
	UDataCollector* DataCollector;

	int32 DefaultMinMatchingStreakLength;

	/** If true, plan reuse is done probabilistically, where sometimes we explore paths that are not similar to previous plan */
	uint8 bProbabilisticPlanReuse : 1;

protected:
	virtual void PauseLogic(const FString& Reason) override;
	virtual EAILogicResuming::Type ResumeLogic(const FString& Reason) override;
	virtual void StopLogic(const FString& Reason) override;
	// End UBrainComponent overrides

	/**
	 * Adds the given element to the Planning Stack.
	 * 
	 * Should probably stick to only 1 type of search before releasing, but for now can be used
	 * to switch between depth-first search or a Dijkstra's/A* -like breadth-first search
	 */
	void AddStackElement(const FHTNStackElement& StackElement);
	/**
	 * Pops an element from the Planning Stack.
	 *
	 * Should probably stick to only 1 type of search before releasing, but for now can be used
	 * to switch between depth-first search or a Dijkstra's/A* -like breadth-first search
	 */
	FHTNStackElement PopStackElement();

	/** execute new task */
	void ExecuteTask(TSharedPtr<FHTNTaskInstance> TaskInstance);

	/** indicates instance has been initialized to work with specific HTN Planner asset */
	bool PlannerHasBeenStarted() const;
	/** apply pending execution of task */
	void ProcessPendingExecution();

	/** verifies that all of our values of maximum streak lengths among unprocessed nodes are still correct or updates them otherwise */
	void UpdateMaxStreakLengths();

	/** HTN Services that have been registered as being interested in regular Tick events */
	UPROPERTY(transient)
	TArray<UHTNService*> RegisteredServices;
	/** AI message observers for running primitive tasks */
	TArray<FTaskMessageObserver> TaskMessageObservers;

	/** Stack used during the planning process to keep track of progress */
	TArray<FHTNStackElement> PlanningStack;

	/** Array of stacks that contain nodes that have had some matching streak when reusing an old plan earlier on */
	TArray<TArray<FHTNStackElement>> PastStreakStacks;
	/** Array of queues that contain nodes that were on a matching streak when reusing an old plan but just ended their streaks */
	TArray<TQueue<FHTNStackElement>> StreakEndedQueues;
	/** Array of stacks that contain nodes that are on a matching streak when reusing an old plan */
	TArray<TArray<FHTNStackElement>> StreakStacks;

	/** The best plan found (so far) */
	TSharedPtr<FHTNPlan> BestPlan;

	/** The HTN Planner asset that is currently running */
	UPROPERTY(transient)
	UHTNPlanner* CurrentPlannerAsset;

	/** The Task that is currently active (execution of this task has already been started, and it has an In Progress status) */
	TSharedPtr<FHTNTaskInstance> ActiveTask;
	/** The next Task that needs to be executed according to the plan */
	TSharedPtr<FHTNTaskInstance> PendingExecution;

	/** The length of the matching streak that is currently being processed */
	int32 CurrentMatchingStreakLength;
	/** The length of the longest currently matching streak among unprocessed nodes */
	int32 MaxCurrentMatchingStreakLength;
	/** The length of the longest matching streak among unprocessed nodes that has just been ended (1 non-matching node) */
	int32 MaxEndedMatchingStreakLength;
	/** The length of the longest matching streak among unprocessed nodes that has been ended in the past (> 1 non-matching nodes) */
	int32 MaxPastMatchingStreakLength;

	/** The minimum length that a matching streak must have to be considered a matching streak */
	int32 MinMatchingStreakLength;

	/** The best (lowest) cost found (so far) */
	float BestCost;

	/** The number of stack elements that still need processing */
	int32 NumStackElements;

	int32 NumLeaves;
	int32 TotalLeafStreakLengths;

	float ProbabilityIgnorePlanReuse;
	/** Flag used for probabilistic plan reuse */
	uint8 bHitLeaf : 1;
	/** Flag used for probabilistic plan reuse */
	uint8 bIgnoringPlanReuse : 1;

	/** If true, plan reuse uses an adaptive minimum streak length instead of a constant */
	uint8 bAdaptivePlanReuse : 1;
	/** If true, we do a depth-first search. Otherwise, breadth-first/A* style search */
	uint8 bDepthFirstSearch : 1;
	/** if set, execution requests will be postponed */
	uint8 bIsPaused : 1;
	/** if set, planner execution is allowed */
	uint8 bIsRunning : 1;
	/** set when execution update is scheduled for next tick */
	uint8 bRequestedExecutionUpdate : 1;
	/** set when planner stop was called */
	uint8 bRequestedStop : 1;
	/** set when execution is waiting for tasks to abort */
	uint8 bWaitingForAbortingTasks : 1;

#if HTN_LOG_RUNTIME_STATS
	/** The point in time where our Planner first started searching a solution for the latest problem */
	FDateTime StartPlanningTime;

	/** 
	 * The timespan between StartPlanningTime and the end of a planning process 
	 * (either finding first plan if ignoring costs, or finding best plan otherwise)
	 *
	 * This includes all the time spent by Unreal Engine for everything other than planning
	 * (graphics, etc.) in between the start and the end of the planning process
	 */
	FTimespan CumulativeSearchTimespan;

	/**
	 * The amount of milliseconds that are actually spent search for a plan
	 *
	 * This includes only time spent in the planning functions
	 */
	double CumulativeSearchTimeMs;

	/**
	 * The amount of milliseconds that were spent searching for a plan until the best plan
	 * was found (although not necessarily known yet that that was the best plan).
	 *
	 * This includes only time spent in the planning functions
	 */
	double CumulativeSearchTimeMsTillLastImprovement;

	/**
	 * The amount of milliseconds that were spent searching for a plan until the first plan
	 * was found
	 *
	 * This includes only time spent in the planning functions
	 */
	double CumulativeSearchTimeMsTillFirstPlan;

	/** The number of nodes that we have expanded */
	uint64 NumNodesExpanded;

	/**
	 * The cost of the first plan that was found during the planning process
	 */
	float FirstPlanCost;

	/**
	 * The number of frames (tick events) that the planning process required to finish
	 */
	int32 CumulativeFrameCount;
#endif // HTN_LOG_RUNTIME_STATS
};