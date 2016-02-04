#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/TaskNetwork.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_GetItemSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_ReloadCompoundSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_ReloadSimpleFPS.h"

UHTNTask_ReloadCompoundSimpleFPS::UHTNTask_ReloadCompoundSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Reload (Compound)";

	// we'll always end up with at least one UHTNTask_ReloadSimpleFPS task, and sometimes nothing more
	HeuristicCost = UHTNTask_ReloadSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_ReloadSimpleFPS>()->GetHeuristicCost();
}

TArray<TSharedPtr<FHTNTaskInstance>> UHTNTask_ReloadCompoundSimpleFPS::FindDecompositions(UHTNPlannerComponent& HTNComp, 
																						  const TSharedPtr<FHTNWorldState> WorldState, 
																						  uint8* TaskMemory)
{
	TArray<TSharedPtr<FHTNTaskInstance>> Decompositions;

	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		// TO DO we're currently assuming only 1 Ammo object corresponding to every weapon, which always is correct
		// in our random problem generator. If this changes, need to change world state representation and also need
		// to look at more ammo objects here (and sort according to distance if none in inventory)
		FHTNTask_ReloadCompoundSimpleFPSMemory* Memory = (FHTNTask_ReloadCompoundSimpleFPSMemory*)TaskMemory;
		FSimpleFPSObject* Ammo = SimpleFPSWorldState->GetAmmo()[SimpleFPSWorldState->GunData[Memory->Gun->Index].Ammo];

		if(SimpleFPSWorldState->NPCInventory.Contains(Ammo))
		{
			// already have the ammo in inventory, so simply use it to reload
			// create decomposition
			TSharedPtr<FHTNTaskInstance> Decomposition = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());

			// create task to reload
			TSharedPtr<FHTNTaskInstance> ReloadTask = HTNComp.InstantiateTask(UHTNTask_ReloadSimpleFPS::StaticClass());
			FHTNTask_ReloadSimpleFPSMemory* ReloadMemory = (FHTNTask_ReloadSimpleFPSMemory*)ReloadTask->GetMemory();
			ReloadMemory->Gun = Memory->Gun;
			ReloadMemory->Ammo = Ammo;

			// finalize and add Decomposition
			UTaskNetwork* DecompositionNetwork = Cast<UTaskNetwork>(Decomposition->Task);
			DecompositionNetwork->AddTaskOrSubNetwork(ReloadTask, Decomposition->GetMemory());
			Decompositions.Add(Decomposition);
		}
		else
		{
			// need to get ammo from the world
			int32 AmmoArea = SimpleFPSWorldState->AmmoData[Ammo->Index].Area;

			if(AmmoArea >= 0)
			{
				// create decomposition
				TSharedPtr<FHTNTaskInstance> Decomposition = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());

				// create task to get the ammo
				TSharedPtr<FHTNTaskInstance> GetItemTask = HTNComp.InstantiateTask(UHTNTask_GetItemSimpleFPS::StaticClass());
				FHTNTask_GetItemSimpleFPSMemory* GetItemMemory = (FHTNTask_GetItemSimpleFPSMemory*)GetItemTask->GetMemory();
				GetItemMemory->Item = Ammo;
				GetItemMemory->Area = AmmoArea;

				// create task to reload
				TSharedPtr<FHTNTaskInstance> ReloadTask = HTNComp.InstantiateTask(UHTNTask_ReloadSimpleFPS::StaticClass());
				FHTNTask_ReloadSimpleFPSMemory* ReloadMemory = (FHTNTask_ReloadSimpleFPSMemory*)ReloadTask->GetMemory();
				ReloadMemory->Gun = Memory->Gun;
				ReloadMemory->Ammo = Ammo;

				// finalize and add Decomposition
				UTaskNetwork* DecompositionNetwork = Cast<UTaskNetwork>(Decomposition->Task);
				DecompositionNetwork->AddTaskOrSubNetwork(GetItemTask, Decomposition->GetMemory());
				DecompositionNetwork->AddTaskOrSubNetwork(ReloadTask, Decomposition->GetMemory());
				Decompositions.Add(Decomposition);
			}
		}
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_ReloadCompoundSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}

	return Decompositions;
}

float UHTNTask_ReloadCompoundSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_ReloadCompoundSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_ReloadCompoundSimpleFPSMemory);
}