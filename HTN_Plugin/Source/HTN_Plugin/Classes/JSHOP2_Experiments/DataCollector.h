#pragma once

#include "DataCollector.generated.h"

enum class EPlanningMode : uint8
{
	Unknown,
	InitialPlan,
	ReplanningWithoutReuse,

	ReplanningWithReuse_1_off,
	ReplanningWithReuse_1_on,
	ReplanningWithReuse_10_off,
	ReplanningWithReuse_10_on,
	ReplanningWithReuse_20_off,
	ReplanningWithReuse_20_on,
	ReplanningWithReuse_30_off,
	ReplanningWithReuse_30_on,
};

USTRUCT()
struct FPlanningEpisode
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<float> SolutionCosts;
	UPROPERTY()
	TArray<int32> SolutionPlanSizes;
	UPROPERTY()
	TArray<int32> SolutionSearchHistorySizes;
	UPROPERTY()
	TArray<double> SolutionSearchTimes;
	UPROPERTY()
	TArray<uint64> SolutionNumNodes;

	double EarlyTerminationTime;
	double OptimalityProofTime;
	uint64 OptimalityNumNodes;

	EPlanningMode PlanningMode;

	FPlanningEpisode() : SolutionCosts(), SolutionPlanSizes(), SolutionSearchTimes(),
		SolutionSearchHistorySizes(), SolutionNumNodes(), EarlyTerminationTime(-1.0), 
		OptimalityProofTime(-1.0), OptimalityNumNodes(-1), PlanningMode(EPlanningMode::Unknown)
	{}
};

USTRUCT()
struct FPlanningProblemResult
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FPlanningEpisode> PlanningEpisodes;

	UPROPERTY()
	TArray<int32> TaskExecutionCounters;

	UPROPERTY()
	TArray<int32> PlanSizes;

	int32 WorldGenerationSeed;

	FPlanningProblemResult() : PlanningEpisodes(), TaskExecutionCounters(), PlanSizes(), WorldGenerationSeed(-1) {}
};

/**
 * Class to collect data/results of experiments and write it to files
 */
UCLASS()
class HTN_PLUGIN_API UDataCollector : public UObject
{
	GENERATED_BODY()

public:
	UDataCollector(const FObjectInitializer& ObjectInitializer);

	void EarlyTermination(double Milliseconds);
	void ExportResults();
	void FoundSolution(float Cost, int32 PlanSize, int32 SearchHistorySize, double SearchTime, uint64 NumNodes);
	void OptimalityProven(double SearchTime, uint64 NumNodes);
	void SetPlanningMode(EPlanningMode Mode);
	void SetTasksExecuted(int32 NumTasks, int32 PlanSize);
	void SetWorldStateSeed(int32 Seed);
	void StartNewPlanningEpisode();
	void StartNewProblem();

protected:
	UPROPERTY()
	TArray<FPlanningProblemResult> PlanningProblemResults;
};