#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/TaskNetwork.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_AttackMeleeSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_AttackMeleeCompoundSimpleFPS.h"

UHTNTask_AttackMeleeCompoundSimpleFPS::UHTNTask_AttackMeleeCompoundSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Attack Melee (Compound)";

	// we'll always end up following some path with a single UHTNTask_AttackMeleeSimpleFPS task
	HeuristicCost = UHTNTask_AttackMeleeSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_AttackMeleeSimpleFPS>()->GetHeuristicCost();
}

TArray<TSharedPtr<FHTNTaskInstance>> UHTNTask_AttackMeleeCompoundSimpleFPS::FindDecompositions(UHTNPlannerComponent& HTNComp, 
																							   const TSharedPtr<FHTNWorldState> WorldState, 
																							   uint8* TaskMemory)
{
	TArray<TSharedPtr<FHTNTaskInstance>> Decompositions;

	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_AttackMeleeCompoundSimpleFPSMemory* Memory = (FHTNTask_AttackMeleeCompoundSimpleFPSMemory*)TaskMemory;

		// we make one decomposition for every knife that we're holding
		for(FSimpleFPSObject* Item : SimpleFPSWorldState->NPCInventory)
		{
			if(Item->ObjectType == ESimpleFPSObjectTypes::Knife)
			{
				// create decomposition
				TSharedPtr<FHTNTaskInstance> Decomposition = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());

				// create primitive task to attack
				TSharedPtr<FHTNTaskInstance> AttackTask = HTNComp.InstantiateTask(UHTNTask_AttackMeleeSimpleFPS::StaticClass());
				FHTNTask_AttackMeleeSimpleFPSMemory* AttackTaskMemory = (FHTNTask_AttackMeleeSimpleFPSMemory*)AttackTask->GetMemory();
				AttackTaskMemory->Area = Memory->Area;
				AttackTaskMemory->Knife = Item;
				AttackTaskMemory->Player = Memory->Player;

				// finalize and add decomposition
				Cast<UTaskNetwork>(Decomposition->Task)->AddTaskOrSubNetwork(AttackTask, Decomposition->GetMemory());
				Decompositions.Add(Decomposition);
			}
		}
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_AttackMeleeCompoundSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}

	return Decompositions;
}

float UHTNTask_AttackMeleeCompoundSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_AttackMeleeCompoundSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_AttackMeleeCompoundSimpleFPSMemory);
}