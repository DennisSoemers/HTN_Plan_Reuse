#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/TaskNetwork.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_GetItemSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_GetKnifeSimpleFPS.h"

UHTNTask_GetKnifeSimpleFPS::UHTNTask_GetKnifeSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Get Knife";

	// this task can sometimes be decomposed into an empty network, so heuristic of 0
	HeuristicCost = 0.f;
}

TArray<TSharedPtr<FHTNTaskInstance>> UHTNTask_GetKnifeSimpleFPS::FindDecompositions(UHTNPlannerComponent& HTNComp, 
																					const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory)
{
	TArray<TSharedPtr<FHTNTaskInstance>> Decompositions;

	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		// if we have a knife in inventory, simply return empty network
		for(FSimpleFPSObject* Item : SimpleFPSWorldState->NPCInventory)
		{
			if(Item->ObjectType == ESimpleFPSObjectTypes::Knife)
			{
				Decompositions.Add(HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass()));
				return Decompositions;
			}
		}

		// didn't return yet, so don't have any knives in inventory and need to find one in the world
		const TArray<FSimpleFPSObject*>& WorldStateKnives = SimpleFPSWorldState->GetKnives();
		TArray<FSimpleFPSObject*> Knives;
		Knives.Reserve(WorldStateKnives.Num());
		int32 NPCArea = SimpleFPSWorldState->NPCArea;

		for(FSimpleFPSObject* Knife : WorldStateKnives)
		{
			int32 KnifeArea = SimpleFPSWorldState->KnifeData[Knife->Index].Area;
			if(KnifeArea >= 0)	// valid area, not in some inventory already
			{
				if(SimpleFPSWorldState->IsReachable(NPCArea, KnifeArea))	// reachable from our current position
				{
					Knives.Add(Knife);
				}
			}
		}

		// sort knives in the world according to distance to the NPC
		const TArray<int32>& DistanceRow = SimpleFPSWorldState->GetDistanceRow(NPCArea);
		Knives.Sort([&](const FSimpleFPSObject& A, const FSimpleFPSObject& B)
		{
			const FSimpleFPSKnifeData& KnifeDataA = SimpleFPSWorldState->KnifeData[A.Index];
			const FSimpleFPSKnifeData& KnifeDataB = SimpleFPSWorldState->KnifeData[B.Index];

			return (DistanceRow[KnifeDataA.Area] < DistanceRow[KnifeDataB.Area]);
		});

		// now make a decomposition for every knife in the order obtained by sorting
		for(int32 Idx = 0; Idx < Knives.Num(); ++Idx)
		{
			FSimpleFPSObject* Knife = Knives[Idx];

			// create decomposition
			TSharedPtr<FHTNTaskInstance> Decomposition = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());

			// create task to get the knife
			TSharedPtr<FHTNTaskInstance> GetItemTask = HTNComp.InstantiateTask(UHTNTask_GetItemSimpleFPS::StaticClass());
			FHTNTask_GetItemSimpleFPSMemory* GetItemMemory = (FHTNTask_GetItemSimpleFPSMemory*)GetItemTask->GetMemory();
			GetItemMemory->Item = Knife;
			GetItemMemory->Area = SimpleFPSWorldState->KnifeData[Knife->Index].Area;

			// add task to get the knife
			UTaskNetwork* DecompositionNetwork = Cast<UTaskNetwork>(Decomposition->Task);
			DecompositionNetwork->AddTaskOrSubNetwork(GetItemTask, Decomposition->GetMemory());

			// add the decomposition
			Decompositions.Add(Decomposition);
		}
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_GetKnifeSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}

	return Decompositions;
}

float UHTNTask_GetKnifeSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}