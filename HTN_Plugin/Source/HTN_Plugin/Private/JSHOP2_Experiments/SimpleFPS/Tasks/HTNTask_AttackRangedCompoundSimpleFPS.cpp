#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/TaskNetwork.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_AttackRangedSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_AttackRangedCompoundSimpleFPS.h"

UHTNTask_AttackRangedCompoundSimpleFPS::UHTNTask_AttackRangedCompoundSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Attack Ranged (Compound)";

	// we'll always end up following some path with a single UHTNTask_AttackRangedSimpleFPS task
	HeuristicCost = UHTNTask_AttackRangedSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_AttackRangedSimpleFPS>()->GetHeuristicCost();
}

TArray<TSharedPtr<FHTNTaskInstance>> UHTNTask_AttackRangedCompoundSimpleFPS::FindDecompositions(UHTNPlannerComponent& HTNComp, 
																								const TSharedPtr<FHTNWorldState> WorldState, 
																								uint8* TaskMemory)
{
	TArray<TSharedPtr<FHTNTaskInstance>> Decompositions;

	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_AttackRangedCompoundSimpleFPSMemory* Memory = (FHTNTask_AttackRangedCompoundSimpleFPSMemory*)TaskMemory;

		// we make one decomposition for every loaded gun that we're holding
		for(FSimpleFPSObject* Item : SimpleFPSWorldState->NPCInventory)
		{
			if(Item->ObjectType == ESimpleFPSObjectTypes::Gun)
			{
				if(SimpleFPSWorldState->GunData[Item->Index].bLoaded)
				{
					// create decomposition
					TSharedPtr<FHTNTaskInstance> Decomposition = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());

					// create primitive task to attack
					TSharedPtr<FHTNTaskInstance> AttackTask = HTNComp.InstantiateTask(UHTNTask_AttackRangedSimpleFPS::StaticClass());
					FHTNTask_AttackRangedSimpleFPSMemory* AttackTaskMemory = (FHTNTask_AttackRangedSimpleFPSMemory*)AttackTask->GetMemory();
					AttackTaskMemory->Area = Memory->Area;
					AttackTaskMemory->Gun = Item;
					AttackTaskMemory->Player = Memory->Player;

					// finalize and add decomposition
					UTaskNetwork* DecompositionNetwork = Cast<UTaskNetwork>(Decomposition->Task);
					DecompositionNetwork->AddTaskOrSubNetwork(AttackTask, Decomposition->GetMemory());
					Decompositions.Add(Decomposition);
				}
			}
		}
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_AttackRangedCompoundSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}

	return Decompositions;
}

float UHTNTask_AttackRangedCompoundSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_AttackRangedCompoundSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_AttackRangedCompoundSimpleFPSMemory);
}