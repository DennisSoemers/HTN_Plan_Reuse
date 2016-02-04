#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/TaskNetwork.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_GetItemSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MoveToAreaSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MoveToPointCompoundSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_PlaceInInventorySimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_TurnOnLightsCompoundSimpleFPS.h"

UHTNTask_GetItemSimpleFPS::UHTNTask_GetItemSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Get Item";

	// this task can sometimes be decomposed into an empty network, so heuristic of 0
	HeuristicCost = 0.f;
}

TArray<TSharedPtr<FHTNTaskInstance>> UHTNTask_GetItemSimpleFPS::FindDecompositions(UHTNPlannerComponent& HTNComp,
																				   const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory)
{
	TArray<TSharedPtr<FHTNTaskInstance>> Decompositions;

	FHTNTask_GetItemSimpleFPSMemory* Memory = (FHTNTask_GetItemSimpleFPSMemory*)TaskMemory;
	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		if(Memory->Area < 0)
		{
			// area not specified
			if(SimpleFPSWorldState->NPCInventory.Contains(Memory->Item))
			{
				// we already have the item, so return empty network
				Decompositions.Add(HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass()));
				return Decompositions;
			}
			else
			{
				// find the area
				if(Memory->Item->ObjectType == ESimpleFPSObjectTypes::Ammo)
				{
					Memory->Area = SimpleFPSWorldState->AmmoData[Memory->Item->Index].Area;
				}
				else if(Memory->Item->ObjectType == ESimpleFPSObjectTypes::Gun)
				{
					Memory->Area = SimpleFPSWorldState->GunData[Memory->Item->Index].Area;
				}
				else if(Memory->Item->ObjectType == ESimpleFPSObjectTypes::Keycard)
				{
					Memory->Area = SimpleFPSWorldState->KeycardData[Memory->Item->Index].Area;
				}
				else if(Memory->Item->ObjectType == ESimpleFPSObjectTypes::Knife)
				{
					Memory->Area = SimpleFPSWorldState->KnifeData[Memory->Item->Index].Area;
				}
				else if(Memory->Item->ObjectType == ESimpleFPSObjectTypes::Medikit)
				{
					Memory->Area = SimpleFPSWorldState->MedikitData[Memory->Item->Index].Area;
				}
				else
				{
					UE_LOG(LogHTNPlanner, Error, TEXT("Item of non-item type in UHTNTask_GetItemSimpleFPS::FindDecompositions()!"));
					return Decompositions;
				}
			}
		}
		
		// area must be known now
		// create decomposition
		TSharedPtr<FHTNTaskInstance> Decomposition = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());

		// create task to move to area
		TSharedPtr<FHTNTaskInstance> MoveToAreaTask = HTNComp.InstantiateTask(UHTNTask_MoveToAreaSimpleFPS::StaticClass());
		FHTNTask_MoveToAreaSimpleFPSMemory* MoveToAreaMemory = (FHTNTask_MoveToAreaSimpleFPSMemory*)MoveToAreaTask->GetMemory();
		MoveToAreaMemory->Area = Memory->Area;

		// create task to turn on the lights
		TSharedPtr<FHTNTaskInstance> TurnOnLightsTask = HTNComp.InstantiateTask(UHTNTask_TurnOnLightsCompoundSimpleFPS::StaticClass());
		FHTNTask_TurnOnLightsCompoundSimpleFPSMemory* TurnOnLightsMemory = 
			(FHTNTask_TurnOnLightsCompoundSimpleFPSMemory*)TurnOnLightsTask->GetMemory();
		TurnOnLightsMemory->Area = Memory->Area;

		// create task to move to the item
		TSharedPtr<FHTNTaskInstance> MoveToPointTask = HTNComp.InstantiateTask(UHTNTask_MoveToPointCompoundSimpleFPS::StaticClass());
		FHTNTask_MoveToPointCompoundSimpleFPSMemory* MoveToPointMemory = (FHTNTask_MoveToPointCompoundSimpleFPSMemory*)MoveToPointTask->GetMemory();
		MoveToPointMemory->Area = Memory->Area;
		MoveToPointMemory->PointOfInterest = Memory->Item;

		// create task to place item in inventory
		TSharedPtr<FHTNTaskInstance> PlaceInInventoryTask = HTNComp.InstantiateTask(UHTNTask_PlaceInInventorySimpleFPS::StaticClass());
		FHTNTask_PlaceInInventorySimpleFPSMemory* PlaceInInventoryMemory = (FHTNTask_PlaceInInventorySimpleFPSMemory*)PlaceInInventoryTask->GetMemory();
		PlaceInInventoryMemory->Area = Memory->Area;
		PlaceInInventoryMemory->Item = Memory->Item;

		// finalize and add Decomposition
		UTaskNetwork* DecompositionNetwork = Cast<UTaskNetwork>(Decomposition->Task);
		DecompositionNetwork->AddTaskOrSubNetwork(MoveToAreaTask, Decomposition->GetMemory());
		DecompositionNetwork->AddTaskOrSubNetwork(TurnOnLightsTask, Decomposition->GetMemory());
		DecompositionNetwork->AddTaskOrSubNetwork(MoveToPointTask, Decomposition->GetMemory());
		DecompositionNetwork->AddTaskOrSubNetwork(PlaceInInventoryTask, Decomposition->GetMemory());
		Decompositions.Add(Decomposition);
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_GetItemSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}

	return Decompositions;
}

float UHTNTask_GetItemSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_GetItemSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_GetItemSimpleFPSMemory);
}