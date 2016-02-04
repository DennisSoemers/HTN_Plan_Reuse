#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/HTNPlan.h"
#include "HTNPlanner/HTNPlanner.h"
#include "HTNPlanner/HTNPlannerComponent.h"
#include "JSHOP2_Experiments/DataCollector.h"
#include "JSHOP2_Experiments/JSHOP2_Experimenter.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_AttackMeleeCompoundSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_AttackRangedCompoundSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_EvaluateWeaponChoiceSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_ExploreSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MovingToPatrolSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MovingToTakePositionSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_SneakKillCompoundSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_UnexploreSimpleFPS.h"

AJSHOP2_Experimenter::AJSHOP2_Experimenter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	HTNComp = ObjectInitializer.CreateDefaultSubobject<UHTNPlannerComponent>(this, TEXT("HTNComp"));
	bDepthFirstSearch = true;
	bLoop = false;
	MaxSearchTime = 0.05f;

	PrimaryActorTick.bCanEverTick = true;

	MaxSearchTimeFirstSolution = 75.f;
	MaxSearchTimeOptimalSolution = 150.f;

	bExperimentPlanReuse = true;

	bConstructedInitialPlan = false;
	bExecutedPlan = false;
	bReplannedWithoutReuse = false;

	bReplannedWithReuse_1_off = false;
	bReplannedWithReuse_1_on = false;
	bReplannedWithReuse_10_off = false;
	bReplannedWithReuse_10_on = false;
	bReplannedWithReuse_20_off = false;
	bReplannedWithReuse_20_on = false;
	bReplannedWithReuse_30_off = false;
	bReplannedWithReuse_30_on = false;

	bFinishedExperiment = false;
}

void AJSHOP2_Experimenter::BeginPlay()
{
	Super::BeginPlay();

	Planner = GeneratePlanner();
}

void AJSHOP2_Experimenter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	HTNComp->StopPlanner();
}

bool AJSHOP2_Experimenter::FinishedExperiment() const
{
	return bFinishedExperiment;
}

void AJSHOP2_Experimenter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(Planner != nullptr && !HTNComp->IsRunning())
	{
		// time to start a new search
		if(!bConstructedInitialPlan)
		{
			HTNComp->DataCollector->StartNewProblem();
			HTNComp->DataCollector->StartNewPlanningEpisode();
			HTNComp->DataCollector->SetPlanningMode(EPlanningMode::InitialPlan);

			// first construct an initial plan
			HTNComp->PreviousPlan = nullptr;	// no previous plan to re-use exists yet
			HTNComp->StopPlanner();

			Planner->bDepthFirstSearch = bDepthFirstSearch;
			UE_LOG(LogHTNPlanner, Warning, TEXT("_"));
			UE_LOG(LogHTNPlanner, Warning, TEXT("==== Constructing Initial Plan ===="));
			bConstructedInitialPlan = true;

			HTNComp->StartPlanner(*Planner);

			FHTNWorldState_SimpleFPS* WorldState = (FHTNWorldState_SimpleFPS*)CurrentWorldState.Get();
			WorldState->SetExecutionMode(false);	// we don't want perfect information here
		}
		else if(!bExecutedPlan)
		{
			TSharedPtr<FHTNPlan> Plan = HTNComp->GetBestPlan();

			if(!Plan.IsValid() || !Plan->IsComplete())
			{
				UE_LOG(LogHTNPlanner, Warning, TEXT("Could not find a valid plan!"));
				bFinishedExperiment = true;
				SetActorTickEnabled(false);
			}
			else
			{
				// prepare world state for execution of plan
				FHTNWorldState_SimpleFPS* WorldState = (FHTNWorldState_SimpleFPS*)CurrentWorldState.Get();
				WorldState->SetExecutionMode(true);

				bExecutedPlan = true;
				const TArray<TSharedPtr<FHTNTaskInstance>> Tasks = Plan->GetTasks();
				int32 NumTasksExecuted = 0;

				for(int TaskIdx = 0; TaskIdx < Tasks.Num(); ++TaskIdx)
				{
					const TSharedPtr<FHTNTaskInstance>& Task = Tasks[TaskIdx];
					bool bTaskApplicable = false;

					if(UHTNTask_MovingToPatrolSimpleFPS* PatrolTask = Cast<UHTNTask_MovingToPatrolSimpleFPS>(Task->Task))
					{
						bTaskApplicable = PatrolTask->IsApplicable(CurrentWorldState, Task->GetMemory());

						if(!bTaskApplicable)
						{
							FHTNTask_MovingToPatrolSimpleFPSMemory* Memory = (FHTNTask_MovingToPatrolSimpleFPSMemory*)Task->GetMemory();
							WorldState->SetKnownLock(Memory->Waypoint);
						}
					}
					else if(UHTNTask_MovingToTakePositionSimpleFPS* TakePositionTask
							= Cast<UHTNTask_MovingToTakePositionSimpleFPS>(Task->Task))
					{
						bTaskApplicable = PatrolTask->IsApplicable(CurrentWorldState, Task->GetMemory());

						if(!bTaskApplicable)
						{
							FHTNTask_MovingToTakePositionSimpleFPSMemory* Memory = 
								(FHTNTask_MovingToTakePositionSimpleFPSMemory*)Task->GetMemory();
							WorldState->SetKnownLock(Memory->Waypoint);
						}
					}
					else
					{
						bTaskApplicable = true;
					}

					if(bTaskApplicable)
					{
						if(!Task->Task->IsA(UHTNTask_ExploreSimpleFPS::StaticClass()) &&
						   !Task->Task->IsA(UHTNTask_UnexploreSimpleFPS::StaticClass()))
						{
							// we don't actually want to apply the explore/unexplore bookkeeping tasks here
							UPrimitiveTask* PrimitiveTask = Cast<UPrimitiveTask>(Task->Task);
							PrimitiveTask->ApplyTo(CurrentWorldState, Task->GetMemory());
						}
						
						++NumTasksExecuted;
					}
					else
					{
						break;
					}
				}

				// save our preferred weapon choice
				const TArray<TSharedPtr<FHTNTaskInstance>>& PlanSearchHistory = Plan->GetSearchHistory();
				for(const TSharedPtr<FHTNTaskInstance>& TaskInstance : PlanSearchHistory)
				{
					if(TaskInstance->Task->IsA(UHTNTask_AttackMeleeCompoundSimpleFPS::StaticClass()))
					{
						//UE_LOG(LogHTNPlanner, Warning, TEXT("Preferred Weapon Choice: Melee"));
						WorldState->SetPreferredWeaponChoice((uint8)EHTNWeaponChoiceSimpleFPS::Melee);
						break;
					}
					else if(TaskInstance->Task->IsA(UHTNTask_AttackRangedCompoundSimpleFPS::StaticClass()))
					{
						//UE_LOG(LogHTNPlanner, Warning, TEXT("Preferred Weapon Choice: Ranged"));
						WorldState->SetPreferredWeaponChoice((uint8)EHTNWeaponChoiceSimpleFPS::Ranged);
						break;
					}
					else if(TaskInstance->Task->IsA(UHTNTask_SneakKillCompoundSimpleFPS::StaticClass()))
					{
						//UE_LOG(LogHTNPlanner, Warning, TEXT("Preferred Weapon Choice: Stealth"));
						WorldState->SetPreferredWeaponChoice((uint8)EHTNWeaponChoiceSimpleFPS::Stealth);
						break;
					}
				}

				UE_LOG(LogHTNPlanner, Warning, TEXT("Executed %d tasks!"), NumTasksExecuted);
				HTNComp->DataCollector->SetTasksExecuted(NumTasksExecuted, Tasks.Num());

				if(NumTasksExecuted == Tasks.Num())
				{
					// we've successfully executed the entire plan
					SetActorTickEnabled(false);
					bFinishedExperiment = true;
					UE_LOG(LogHTNPlanner, Warning, TEXT("Finished executing plan!"));
				}
				else
				{
					// time to re-plan, so we'll want to turn off perfect information again
					WorldState->SetExecutionMode(false);

					// remove all the tasks that we've already executed from the plan
					Plan->RemoveExecutedTasks(NumTasksExecuted);

					// cache the plan so we can re-use it
					PartiallyExecutedPlan = Plan;

					// reset the flags so that we will re-plan
					bReplannedWithoutReuse = false;
					bReplannedWithReuse_1_off = true;	// skipping this one
					bReplannedWithReuse_1_on = true;	// skipping this one
					bReplannedWithReuse_10_off = false;
					bReplannedWithReuse_10_on = false;
					bReplannedWithReuse_20_off = false;
					bReplannedWithReuse_20_on = false;
					bReplannedWithReuse_30_off = false;
					bReplannedWithReuse_30_on = false;
				}
			}
		}
		else if(!bReplannedWithoutReuse)
		{
			HTNComp->DataCollector->StartNewPlanningEpisode();
			HTNComp->DataCollector->SetPlanningMode(EPlanningMode::ReplanningWithoutReuse);

			FHTNWorldState_SimpleFPS* WorldState = (FHTNWorldState_SimpleFPS*)CurrentWorldState.Get();
			WorldState->SetExecutionMode(false);	// we don't want perfect information here

			// do a re-planning episode without plan reuse
			HTNComp->PreviousPlan = nullptr;	// no plan reuse
			HTNComp->StopPlanner();

			Planner->bDepthFirstSearch = bDepthFirstSearch;
			UE_LOG(LogHTNPlanner, Warning, TEXT("_"));
			UE_LOG(LogHTNPlanner, Warning, TEXT("==== Re-planning without Plan Reuse ===="));
			bReplannedWithoutReuse = true;

			HTNComp->StartPlanner(*Planner);
		}
		else if(!bReplannedWithReuse_1_off)
		{
			HTNComp->DataCollector->StartNewPlanningEpisode();
			HTNComp->DataCollector->SetPlanningMode(EPlanningMode::ReplanningWithReuse_1_off);

			FHTNWorldState_SimpleFPS* WorldState = (FHTNWorldState_SimpleFPS*)CurrentWorldState.Get();
			WorldState->SetExecutionMode(false);	// we don't want perfect information here

			// do a re-planning episode with plan reuse
			HTNComp->PreviousPlan = PartiallyExecutedPlan;
			HTNComp->StopPlanner();
			HTNComp->DefaultMinMatchingStreakLength = 1;
			HTNComp->bProbabilisticPlanReuse = false;

			Planner->bDepthFirstSearch = bDepthFirstSearch;
			UE_LOG(LogHTNPlanner, Warning, TEXT("_"));
			UE_LOG(LogHTNPlanner, Warning, TEXT("==== Re-planning with Plan Reuse (1, off) ===="));
			bReplannedWithReuse_1_off = true;

			HTNComp->StartPlanner(*Planner);
		}
		else if(!bReplannedWithReuse_1_on)
		{
			HTNComp->DataCollector->StartNewPlanningEpisode();
			HTNComp->DataCollector->SetPlanningMode(EPlanningMode::ReplanningWithReuse_1_on);

			FHTNWorldState_SimpleFPS* WorldState = (FHTNWorldState_SimpleFPS*)CurrentWorldState.Get();
			WorldState->SetExecutionMode(false);	// we don't want perfect information here

			// do a re-planning episode with plan reuse
			HTNComp->PreviousPlan = PartiallyExecutedPlan;
			HTNComp->StopPlanner();
			HTNComp->DefaultMinMatchingStreakLength = 1;
			HTNComp->bProbabilisticPlanReuse = true;

			Planner->bDepthFirstSearch = bDepthFirstSearch;
			UE_LOG(LogHTNPlanner, Warning, TEXT("_"));
			UE_LOG(LogHTNPlanner, Warning, TEXT("==== Re-planning with Plan Reuse (1, on) ===="));
			bReplannedWithReuse_1_on = true;

			HTNComp->StartPlanner(*Planner);
		}
		else if(!bReplannedWithReuse_10_off)
		{
			HTNComp->DataCollector->StartNewPlanningEpisode();
			HTNComp->DataCollector->SetPlanningMode(EPlanningMode::ReplanningWithReuse_10_off);

			FHTNWorldState_SimpleFPS* WorldState = (FHTNWorldState_SimpleFPS*)CurrentWorldState.Get();
			WorldState->SetExecutionMode(false);	// we don't want perfect information here

			// do a re-planning episode with plan reuse
			HTNComp->PreviousPlan = PartiallyExecutedPlan;
			HTNComp->StopPlanner();
			HTNComp->DefaultMinMatchingStreakLength = 10;
			HTNComp->bProbabilisticPlanReuse = false;

			Planner->bDepthFirstSearch = bDepthFirstSearch;
			UE_LOG(LogHTNPlanner, Warning, TEXT("_"));
			UE_LOG(LogHTNPlanner, Warning, TEXT("==== Re-planning with Plan Reuse (10, off) ===="));
			bReplannedWithReuse_10_off = true;

			HTNComp->StartPlanner(*Planner);
		}
		else if(!bReplannedWithReuse_10_on)
		{
			HTNComp->DataCollector->StartNewPlanningEpisode();
			HTNComp->DataCollector->SetPlanningMode(EPlanningMode::ReplanningWithReuse_10_on);

			FHTNWorldState_SimpleFPS* WorldState = (FHTNWorldState_SimpleFPS*)CurrentWorldState.Get();
			WorldState->SetExecutionMode(false);	// we don't want perfect information here

			// do a re-planning episode with plan reuse
			HTNComp->PreviousPlan = PartiallyExecutedPlan;
			HTNComp->StopPlanner();
			HTNComp->DefaultMinMatchingStreakLength = 10;
			HTNComp->bProbabilisticPlanReuse = true;

			Planner->bDepthFirstSearch = bDepthFirstSearch;
			UE_LOG(LogHTNPlanner, Warning, TEXT("_"));
			UE_LOG(LogHTNPlanner, Warning, TEXT("==== Re-planning with Plan Reuse (10, on) ===="));
			bReplannedWithReuse_10_on = true;

			HTNComp->StartPlanner(*Planner);
		}
		else if(!bReplannedWithReuse_20_off)
		{
			HTNComp->DataCollector->StartNewPlanningEpisode();
			HTNComp->DataCollector->SetPlanningMode(EPlanningMode::ReplanningWithReuse_20_off);

			FHTNWorldState_SimpleFPS* WorldState = (FHTNWorldState_SimpleFPS*)CurrentWorldState.Get();
			WorldState->SetExecutionMode(false);	// we don't want perfect information here

			// do a re-planning episode with plan reuse
			HTNComp->PreviousPlan = PartiallyExecutedPlan;
			HTNComp->StopPlanner();
			HTNComp->DefaultMinMatchingStreakLength = 20;
			HTNComp->bProbabilisticPlanReuse = false;

			Planner->bDepthFirstSearch = bDepthFirstSearch;
			UE_LOG(LogHTNPlanner, Warning, TEXT("_"));
			UE_LOG(LogHTNPlanner, Warning, TEXT("==== Re-planning with Plan Reuse (20, off) ===="));
			bReplannedWithReuse_20_off = true;

			HTNComp->StartPlanner(*Planner);
		}
		else if(!bReplannedWithReuse_20_on)
		{
			HTNComp->DataCollector->StartNewPlanningEpisode();
			HTNComp->DataCollector->SetPlanningMode(EPlanningMode::ReplanningWithReuse_20_on);

			FHTNWorldState_SimpleFPS* WorldState = (FHTNWorldState_SimpleFPS*)CurrentWorldState.Get();
			WorldState->SetExecutionMode(false);	// we don't want perfect information here

			// do a re-planning episode with plan reuse
			HTNComp->PreviousPlan = PartiallyExecutedPlan;
			HTNComp->StopPlanner();
			HTNComp->DefaultMinMatchingStreakLength = 20;
			HTNComp->bProbabilisticPlanReuse = true;

			Planner->bDepthFirstSearch = bDepthFirstSearch;
			UE_LOG(LogHTNPlanner, Warning, TEXT("_"));
			UE_LOG(LogHTNPlanner, Warning, TEXT("==== Re-planning with Plan Reuse (20, on) ===="));
			bReplannedWithReuse_20_on = true;

			HTNComp->StartPlanner(*Planner);
		}
		else if(!bReplannedWithReuse_30_off)
		{
			HTNComp->DataCollector->StartNewPlanningEpisode();
			HTNComp->DataCollector->SetPlanningMode(EPlanningMode::ReplanningWithReuse_30_off);

			FHTNWorldState_SimpleFPS* WorldState = (FHTNWorldState_SimpleFPS*)CurrentWorldState.Get();
			WorldState->SetExecutionMode(false);	// we don't want perfect information here

			// do a re-planning episode with plan reuse
			HTNComp->PreviousPlan = PartiallyExecutedPlan;
			HTNComp->StopPlanner();
			HTNComp->DefaultMinMatchingStreakLength = 30;
			HTNComp->bProbabilisticPlanReuse = false;

			Planner->bDepthFirstSearch = bDepthFirstSearch;
			UE_LOG(LogHTNPlanner, Warning, TEXT("_"));
			UE_LOG(LogHTNPlanner, Warning, TEXT("==== Re-planning with Plan Reuse (30, off) ===="));
			bReplannedWithReuse_30_off = true;

			HTNComp->StartPlanner(*Planner);
		}
		else if(!bReplannedWithReuse_30_on)
		{
			HTNComp->DataCollector->StartNewPlanningEpisode();
			HTNComp->DataCollector->SetPlanningMode(EPlanningMode::ReplanningWithReuse_30_on);

			FHTNWorldState_SimpleFPS* WorldState = (FHTNWorldState_SimpleFPS*)CurrentWorldState.Get();
			WorldState->SetExecutionMode(false);	// we don't want perfect information here

			// do a re-planning episode with plan reuse
			HTNComp->PreviousPlan = PartiallyExecutedPlan;
			HTNComp->StopPlanner();
			HTNComp->DefaultMinMatchingStreakLength = 30;
			HTNComp->bProbabilisticPlanReuse = true;

			Planner->bDepthFirstSearch = bDepthFirstSearch;
			UE_LOG(LogHTNPlanner, Warning, TEXT("_"));
			UE_LOG(LogHTNPlanner, Warning, TEXT("==== Re-planning with Plan Reuse (30, on) ===="));
			bReplannedWithReuse_30_on = true;

			HTNComp->StartPlanner(*Planner);

			// we'll want to execute the plan we find here again
			bExecutedPlan = false;
		}
		else
		{
			// we should never exit here, should only exit above when we successfully execute a full plan
			GEngine->AddOnScreenDebugMessage(-1, 500.f, FColor::Yellow, TEXT("THIS SHOULD NEVER HAPPEN"));
			UE_LOG(LogHTNPlanner, Warning, TEXT("THIS SHOULD NEVER HAPPEN"));

			// finished all experiments, so can stop ticking
			SetActorTickEnabled(false);
			bFinishedExperiment = true;
		}
	}
	else if(Planner != nullptr)
	{
#if HTN_LOG_RUNTIME_STATS
		if(HTNComp->GetTimeSearched() >= MaxSearchTimeFirstSolution * 1000.0)
		{
			if(!HTNComp->GetBestPlan().IsValid() || !HTNComp->GetBestPlan()->IsComplete())
			{
				HTNComp->StopPlanner();
				UE_LOG(LogHTNPlanner, Warning, TEXT("Stopping HTN Planner early because no plan found within %.2f seconds!"), 
					   MaxSearchTimeFirstSolution);
				HTNComp->DataCollector->EarlyTermination(MaxSearchTimeFirstSolution * 1000.0);
			}
			else if(HTNComp->GetTimeSearched() >= MaxSearchTimeOptimalSolution * 1000.0)
			{
				HTNComp->StopPlanner();
				UE_LOG(LogHTNPlanner, Warning, TEXT("Stopping HTN Planner early because no optimal plan found within %.2f seconds!"),
					   MaxSearchTimeOptimalSolution);
				UE_LOG(LogHTNPlanner, Warning, TEXT("Best cost so far = %.2f"), HTNComp->GetBestCost());
				HTNComp->DataCollector->EarlyTermination(MaxSearchTimeOptimalSolution * 1000.0);
			}
		}
#endif // HTN_LOG_RUNTIME_STATS
	}
}

UHTNPlanner* AJSHOP2_Experimenter::GeneratePlanner()
{
	return nullptr;
}

void AJSHOP2_Experimenter::SetDataCollector(UDataCollector* DataCollector)
{
	HTNComp->DataCollector = DataCollector;
}