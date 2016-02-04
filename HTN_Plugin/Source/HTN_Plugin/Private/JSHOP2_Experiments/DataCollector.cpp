#include "HTN_PluginPrivatePCH.h"

#include "JSHOP2_Experiments/DataCollector.h"

UDataCollector::UDataCollector(const FObjectInitializer& ObjectInitializer)
	: PlanningProblemResults()
{}

void UDataCollector::EarlyTermination(double Milliseconds)
{
	PlanningProblemResults.Last().PlanningEpisodes.Last().EarlyTerminationTime = Milliseconds;
}

void UDataCollector::ExportResults()
{
	FString ExportDirectory = FPaths::GameDir() + TEXT("../../Experiments/Results/");

	if(!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*ExportDirectory))
	{
		UE_LOG(LogHTNPlanner, Warning, TEXT("Directory for exporting results did not exist yet, creating it: %s"), *ExportDirectory);
		FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*ExportDirectory);

		if(!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*ExportDirectory))
		{
			UE_LOG(LogHTNPlanner, Error, TEXT("Failed to create directory for exporting results: %s"), *ExportDirectory);
			return;
		}
	}

	FString FileName = TEXT("HTN_Experiment_") + FDateTime::Now().ToString() + TEXT(".txt");
	FString Filepath = ExportDirectory + FileName;

	if(FPlatformFileManager::Get().GetPlatformFile().FileExists(*Filepath))
	{
		// file already exists
		UE_LOG(LogHTNPlanner, Error, TEXT("Could not export results, file already exists: %s"), *Filepath);
		return;
	}

	FString Output = TEXT("");
	for(FPlanningProblemResult& Problem : PlanningProblemResults)
	{
		Output += TEXT("BEGIN PROBLEM") + FString(LINE_TERMINATOR);
		Output += FString::Printf(TEXT("World Generation Seed = %d"), Problem.WorldGenerationSeed) + FString(LINE_TERMINATOR);

		for(FPlanningEpisode& Episode : Problem.PlanningEpisodes)
		{
			Output += TEXT("BEGIN PLANNING EPISODE") + FString(LINE_TERMINATOR);

			Output += TEXT("Planning Mode = ");
			if(Episode.PlanningMode == EPlanningMode::InitialPlan)
			{
				Output += TEXT("Initial Plan");
			}
			else if(Episode.PlanningMode == EPlanningMode::ReplanningWithoutReuse)
			{
				Output += TEXT("Replanning Without Reuse");
			}
			else if(Episode.PlanningMode == EPlanningMode::ReplanningWithReuse_1_off)
			{
				Output += TEXT("Replanning With Reuse (1, off)");
			}
			else if(Episode.PlanningMode == EPlanningMode::ReplanningWithReuse_1_on)
			{
				Output += TEXT("Replanning With Reuse (1, on)");
			}
			else if(Episode.PlanningMode == EPlanningMode::ReplanningWithReuse_10_off)
			{
				Output += TEXT("Replanning With Reuse (10, off)");
			}
			else if(Episode.PlanningMode == EPlanningMode::ReplanningWithReuse_10_on)
			{
				Output += TEXT("Replanning With Reuse (10, on)");
			}
			else if(Episode.PlanningMode == EPlanningMode::ReplanningWithReuse_20_off)
			{
				Output += TEXT("Replanning With Reuse (20, off)");
			}
			else if(Episode.PlanningMode == EPlanningMode::ReplanningWithReuse_20_on)
			{
				Output += TEXT("Replanning With Reuse (20, on)");
			}
			else if(Episode.PlanningMode == EPlanningMode::ReplanningWithReuse_30_off)
			{
				Output += TEXT("Replanning With Reuse (30, off)");
			}
			else if(Episode.PlanningMode == EPlanningMode::ReplanningWithReuse_30_on)
			{
				Output += TEXT("Replanning With Reuse (30, on)");
			}
			else
			{
				Output += TEXT("Unknown");
			}
			Output += FString(LINE_TERMINATOR);

			Output += TEXT("Solution Costs = ");
			for(int32 Idx = 0; Idx < Episode.SolutionCosts.Num(); ++Idx)
			{
				Output += FString::Printf(TEXT("%.2f"), Episode.SolutionCosts[Idx]);
				if(Idx < Episode.SolutionCosts.Num() - 1)
				{
					Output += TEXT(",");
				}
			}
			Output += FString(LINE_TERMINATOR);

			Output += TEXT("Solution Plan Sizes = ");
			for(int32 Idx = 0; Idx < Episode.SolutionPlanSizes.Num(); ++Idx)
			{
				Output += FString::Printf(TEXT("%d"), Episode.SolutionPlanSizes[Idx]);
				if(Idx < Episode.SolutionPlanSizes.Num() - 1)
				{
					Output += TEXT(",");
				}
			}
			Output += FString(LINE_TERMINATOR);

			Output += TEXT("Solution Search History Sizes = ");
			for(int32 Idx = 0; Idx < Episode.SolutionSearchHistorySizes.Num(); ++Idx)
			{
				Output += FString::Printf(TEXT("%d"), Episode.SolutionSearchHistorySizes[Idx]);
				if(Idx < Episode.SolutionSearchHistorySizes.Num() - 1)
				{
					Output += TEXT(",");
				}
			}
			Output += FString(LINE_TERMINATOR);

			Output += TEXT("Solution Search Times = ");
			for(int32 Idx = 0; Idx < Episode.SolutionSearchTimes.Num(); ++Idx)
			{
				Output += FString::Printf(TEXT("%.2f"), Episode.SolutionSearchTimes[Idx]);
				if(Idx < Episode.SolutionSearchTimes.Num() - 1)
				{
					Output += TEXT(",");
				}
			}
			Output += FString(LINE_TERMINATOR);

			Output += TEXT("Solution Num Nodes = ");
			for(int32 Idx = 0; Idx < Episode.SolutionNumNodes.Num(); ++Idx)
			{
				Output += FString::Printf(TEXT("%d"), Episode.SolutionNumNodes[Idx]);
				if(Idx < Episode.SolutionNumNodes.Num() - 1)
				{
					Output += TEXT(",");
				}
			}
			Output += FString(LINE_TERMINATOR);

			Output += FString::Printf(TEXT("Early Termination Time = %.2f"), Episode.EarlyTerminationTime) + FString(LINE_TERMINATOR);
			Output += FString::Printf(TEXT("Optimality Proof Time = %.2f"), Episode.OptimalityProofTime) + FString(LINE_TERMINATOR);
			Output += FString::Printf(TEXT("Optimality Num Nodes = %d"), Episode.OptimalityNumNodes) + FString(LINE_TERMINATOR);

			Output += TEXT("END PLANNING EPISODE") + FString(LINE_TERMINATOR);
		}

		Output += TEXT("END PROBLEM") + FString(LINE_TERMINATOR);
	}

	if(FFileHelper::SaveStringToFile(Output, *Filepath))
	{
		UE_LOG(LogHTNPlanner, Warning, TEXT("Succesfully exported results to: %s"), *Filepath);
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("Failed to export results to: %s"), *Filepath);
	}
}

void UDataCollector::FoundSolution(float Cost, int32 PlanSize, int32 SearchHistorySize, double SearchTime, uint64 NumNodes)
{
	FPlanningEpisode& PlanningEpisode = PlanningProblemResults.Last().PlanningEpisodes.Last();
	PlanningEpisode.SolutionCosts.Add(Cost);
	PlanningEpisode.SolutionPlanSizes.Add(PlanSize);
	PlanningEpisode.SolutionSearchHistorySizes.Add(SearchHistorySize);
	PlanningEpisode.SolutionSearchTimes.Add(SearchTime);
	PlanningEpisode.SolutionNumNodes.Add(NumNodes);
}

void UDataCollector::OptimalityProven(double SearchTime, uint64 NumNodes)
{
	FPlanningEpisode& PlanningEpisode = PlanningProblemResults.Last().PlanningEpisodes.Last();
	PlanningEpisode.OptimalityProofTime = SearchTime;
	PlanningEpisode.OptimalityNumNodes = NumNodes;
}

void UDataCollector::SetPlanningMode(EPlanningMode Mode)
{
	PlanningProblemResults.Last().PlanningEpisodes.Last().PlanningMode = Mode;
}

void UDataCollector::SetTasksExecuted(int32 NumTasks, int32 PlanSize)
{
	FPlanningProblemResult& Problem = PlanningProblemResults.Last();
	Problem.TaskExecutionCounters.Add(NumTasks);
	Problem.PlanSizes.Add(PlanSize);
}

void UDataCollector::SetWorldStateSeed(int32 Seed)
{
	PlanningProblemResults.Last().WorldGenerationSeed = Seed;
}

void UDataCollector::StartNewPlanningEpisode()
{
	PlanningProblemResults.Last().PlanningEpisodes.Add(FPlanningEpisode());
}

void UDataCollector::StartNewProblem()
{
	PlanningProblemResults.Add(FPlanningProblemResult());
}