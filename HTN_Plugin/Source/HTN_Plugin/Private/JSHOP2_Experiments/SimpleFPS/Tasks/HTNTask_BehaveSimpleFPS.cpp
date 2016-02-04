#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/TaskNetwork.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_BehaveSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_RestoreFullHealthSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_TakeCoverAndWoundPlayerSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_WoundPlayerSimpleFPS.h"

UHTNTask_BehaveSimpleFPS::UHTNTask_BehaveSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Behave";

	// we'll use the path where we don't have to take cover as admissible heuristic, 
	// since that's always shorter than the path where we do take cover
	HeuristicCost = UHTNTask_RestoreFullHealthSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_RestoreFullHealthSimpleFPS>()->GetHeuristicCost() +
					UHTNTask_WoundPlayerSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_WoundPlayerSimpleFPS>()->GetHeuristicCost();
}

TArray<TSharedPtr<FHTNTaskInstance>> UHTNTask_BehaveSimpleFPS::FindDecompositions(UHTNPlannerComponent& HTNComp, 
																				  const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory)
{
	TArray<TSharedPtr<FHTNTaskInstance>> Decompositions;

	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		const TArray<FSimpleFPSObject*>& Players = SimpleFPSWorldState->GetPlayers();

		// first branch: find cover points in same area as players and use them to take cover and wound player
		for(FSimpleFPSObject* Player : Players)
		{
			int32 Area = SimpleFPSWorldState->PlayerData[Player->Index].Area;

			const TArray<FSimpleFPSObject*>& CoverPoints = SimpleFPSWorldState->GetCoverPoints();
			for(FSimpleFPSObject* CoverPoint : CoverPoints)
			{
				if(SimpleFPSWorldState->CoverPointData[CoverPoint->Index].Area == Area)
				{
					// cover point is in same area as player
					// create decomposition
					TSharedPtr<FHTNTaskInstance> Decomposition = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());

					// create task to restore to full health
					TSharedPtr<FHTNTaskInstance> RestoreFullHealthTask = HTNComp.InstantiateTask(UHTNTask_RestoreFullHealthSimpleFPS::StaticClass());

					// create task to take cover and wound player
					TSharedPtr<FHTNTaskInstance> TakeCoverAndWoundPlayerTask = 
						HTNComp.InstantiateTask(UHTNTask_TakeCoverAndWoundPlayerSimpleFPS::StaticClass());
					FHTNTask_TakeCoverAndWoundPlayerSimpleFPSMemory* TakeCoverAndWoundPlayerTaskMemory =
						(FHTNTask_TakeCoverAndWoundPlayerSimpleFPSMemory*)TakeCoverAndWoundPlayerTask->GetMemory();
					TakeCoverAndWoundPlayerTaskMemory->Area = Area;
					TakeCoverAndWoundPlayerTaskMemory->Player = Player;
					TakeCoverAndWoundPlayerTaskMemory->CoverPoint = CoverPoint;

					// finalize and add Decomposition
					UTaskNetwork* DecompositionNetwork = Cast<UTaskNetwork>(Decomposition->Task);
					DecompositionNetwork->AddTaskOrSubNetwork(RestoreFullHealthTask, Decomposition->GetMemory());
					DecompositionNetwork->AddTaskOrSubNetwork(TakeCoverAndWoundPlayerTask, Decomposition->GetMemory());
					Decompositions.Add(Decomposition);
				}
			}
		}

		if(Decompositions.Num() == 0)
		{
			// didn't find any decompositions yet, so try second branch; players without nearby cover points
			for(FSimpleFPSObject* Player : Players)
			{
				int32 Area = SimpleFPSWorldState->PlayerData[Player->Index].Area;

				// create decomposition
				TSharedPtr<FHTNTaskInstance> Decomposition = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());

				// create task to restore to full health
				TSharedPtr<FHTNTaskInstance> RestoreFullHealthTask = HTNComp.InstantiateTask(UHTNTask_RestoreFullHealthSimpleFPS::StaticClass());

				// create task to wound player
				TSharedPtr<FHTNTaskInstance> WoundPlayerTask = HTNComp.InstantiateTask(UHTNTask_WoundPlayerSimpleFPS::StaticClass());
				FHTNTask_WoundPlayerSimpleFPSMemory* WoundPlayerTaskMemory = (FHTNTask_WoundPlayerSimpleFPSMemory*)WoundPlayerTask->GetMemory();
				WoundPlayerTaskMemory->Area = Area;
				WoundPlayerTaskMemory->Player = Player;

				// finalize and add Decomposition
				UTaskNetwork* DecompositionNetwork = Cast<UTaskNetwork>(Decomposition->Task);
				DecompositionNetwork->AddTaskOrSubNetwork(RestoreFullHealthTask, Decomposition->GetMemory());
				DecompositionNetwork->AddTaskOrSubNetwork(WoundPlayerTask, Decomposition->GetMemory());
				Decompositions.Add(Decomposition);
			}
		}
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_BehaveSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}

	return Decompositions;
}

float UHTNTask_BehaveSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}