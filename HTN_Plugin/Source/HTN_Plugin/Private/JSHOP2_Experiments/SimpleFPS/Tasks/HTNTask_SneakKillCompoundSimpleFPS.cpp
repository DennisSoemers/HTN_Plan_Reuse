#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/TaskNetwork.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_SneakKillCompoundSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_SneakKillSimpleFPS.h"

UHTNTask_SneakKillCompoundSimpleFPS::UHTNTask_SneakKillCompoundSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Sneak Kill (Compound)";

	// we'll always end up following some path with a single UHTNTask_SneakKillSimpleFPS task
	HeuristicCost = UHTNTask_SneakKillSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_SneakKillSimpleFPS>()->GetHeuristicCost();
}

TArray<TSharedPtr<FHTNTaskInstance>> UHTNTask_SneakKillCompoundSimpleFPS::FindDecompositions(UHTNPlannerComponent& HTNComp, 
																							 const TSharedPtr<FHTNWorldState> WorldState, 
																							 uint8* TaskMemory)
{
	TArray<TSharedPtr<FHTNTaskInstance>> Decompositions;

	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_SneakKillCompoundSimpleFPSMemory* Memory = (FHTNTask_SneakKillCompoundSimpleFPSMemory*)TaskMemory;

		// for every loaded gun with night vision in our inventory, create a decomposition where we use that gun for the kill
		for(FSimpleFPSObject* Item : SimpleFPSWorldState->NPCInventory)
		{
			if(Item->ObjectType == ESimpleFPSObjectTypes::Gun)
			{
				const FSimpleFPSGunData& GunData = SimpleFPSWorldState->GunData[Item->Index];

				if(GunData.bLoaded && GunData.bHasNightVision)
				{
					// create decomposition
					TSharedPtr<FHTNTaskInstance> Decomposition = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());

					// create primitive sneak kill task
					TSharedPtr<FHTNTaskInstance> SneakKillTask = HTNComp.InstantiateTask(UHTNTask_SneakKillSimpleFPS::StaticClass());
					FHTNTask_SneakKillSimpleFPSMemory* SneakKillMemory = (FHTNTask_SneakKillSimpleFPSMemory*)SneakKillTask->GetMemory();
					SneakKillMemory->Area = Memory->Area;
					SneakKillMemory->Gun = Item;
					SneakKillMemory->Player = Memory->Player;

					// finalize and add the Decomposition
					UTaskNetwork* DecompositionNetwork = Cast<UTaskNetwork>(Decomposition->Task);
					DecompositionNetwork->AddTaskOrSubNetwork(SneakKillTask, Decomposition->GetMemory());
					Decompositions.Add(Decomposition);
				}
			}
		}
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_SneakKillCompoundSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}

	return Decompositions;
}

float UHTNTask_SneakKillCompoundSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_SneakKillCompoundSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_SneakKillCompoundSimpleFPSMemory);
}