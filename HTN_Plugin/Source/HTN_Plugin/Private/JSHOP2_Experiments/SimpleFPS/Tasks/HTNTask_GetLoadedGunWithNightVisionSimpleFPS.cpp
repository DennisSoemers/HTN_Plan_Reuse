#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/TaskNetwork.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_GetItemSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_GetLoadedGunWithNightVisionSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_ReloadCompoundSimpleFPS.h"

UHTNTask_GetLoadedGunWithNightVisionSimpleFPS::UHTNTask_GetLoadedGunWithNightVisionSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Get Loaded Gun With Night Vision";

	// this task can sometimes be decomposed into an empty network, so heuristic of 0
	HeuristicCost = 0.f;
}

TArray<TSharedPtr<FHTNTaskInstance>> UHTNTask_GetLoadedGunWithNightVisionSimpleFPS::FindDecompositions(UHTNPlannerComponent& HTNComp,
																									   const TSharedPtr<FHTNWorldState> WorldState,
																									   uint8* TaskMemory)
{
	TArray<TSharedPtr<FHTNTaskInstance>> Decompositions;

	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		// first scan our inventory for guns. If we find a loaded one, we use that and dont look further.
		// Otherwise, we consider all of the unloaded guns for reloading
		TArray<FSimpleFPSObject*> UnloadedGuns;
		UnloadedGuns.Reserve(SimpleFPSWorldState->NPCInventory.Num());
		for(FSimpleFPSObject* Item : SimpleFPSWorldState->NPCInventory)
		{
			if(Item->ObjectType == ESimpleFPSObjectTypes::Gun)
			{
				if(SimpleFPSWorldState->GunData[Item->Index].bHasNightVision)
				{
					if(SimpleFPSWorldState->GunData[Item->Index].bLoaded)
					{
						// found a loaded gun in inventory, so create empty task network for that and ignore all unloaded guns
						Decompositions.Add(HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass()));
						UnloadedGuns.Empty();
						break;
					}
					else
					{
						UnloadedGuns.Add(Item);
					}
				}
			}
		}

		for(FSimpleFPSObject* UnloadedGun : UnloadedGuns)
		{
			// create decomposition
			TSharedPtr<FHTNTaskInstance> Decomposition = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());

			// create task to reload gun
			TSharedPtr<FHTNTaskInstance> ReloadGunTask = HTNComp.InstantiateTask(UHTNTask_ReloadCompoundSimpleFPS::StaticClass());
			FHTNTask_ReloadCompoundSimpleFPSMemory* ReloadGunMemory = (FHTNTask_ReloadCompoundSimpleFPSMemory*)ReloadGunTask->GetMemory();
			ReloadGunMemory->Gun = UnloadedGun;

			// finalize and add Decomposition
			UTaskNetwork* DecompositionNetwork = Cast<UTaskNetwork>(Decomposition->Task);
			DecompositionNetwork->AddTaskOrSubNetwork(ReloadGunTask, Decomposition->GetMemory());
			Decompositions.Add(Decomposition);
		}

		// next we'll also consider finding other guns in the world.
		// we want the list of already loaded guns first, then the list of unloaded guns.
		// both of those lists will internally be sorted according to distance to the NPC
		const TArray<FSimpleFPSObject*>& WorldStateGuns = SimpleFPSWorldState->GetGuns();
		TArray<FSimpleFPSObject*> Guns;
		Guns.Reserve(WorldStateGuns.Num());
		int32 NPCArea = SimpleFPSWorldState->NPCArea;

		for(FSimpleFPSObject* Gun : WorldStateGuns)
		{
			int32 GunArea = SimpleFPSWorldState->GunData[Gun->Index].Area;
			if(GunArea >= 0)	// valid area, not in some inventory already
			{
				if(SimpleFPSWorldState->IsReachable(NPCArea, GunArea))	// can reach gun from our current area
				{
					Guns.Add(Gun);
				}
			}
		}

		const TArray<int32>& DistanceRow = SimpleFPSWorldState->GetDistanceRow(NPCArea);
		Guns.Sort([&](const FSimpleFPSObject& A, const FSimpleFPSObject& B)
		{
			const FSimpleFPSGunData& GunDataA = SimpleFPSWorldState->GunData[A.Index];
			const FSimpleFPSGunData& GunDataB = SimpleFPSWorldState->GunData[B.Index];

			if(GunDataA.bLoaded && !GunDataB.bLoaded)
			{
				return true;
			}
			else if(!GunDataA.bLoaded && GunDataB.bLoaded)
			{
				return false;
			}

			return (DistanceRow[GunDataA.Area] < DistanceRow[GunDataB.Area]);
		});

		// now make a decomposition for every gun in the order obtained by sorting
		for(int32 Idx = 0; Idx < Guns.Num(); ++Idx)
		{
			FSimpleFPSObject* Gun = Guns[Idx];

			// create decomposition
			TSharedPtr<FHTNTaskInstance> Decomposition = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());

			// create task to get the gun
			TSharedPtr<FHTNTaskInstance> GetItemTask = HTNComp.InstantiateTask(UHTNTask_GetItemSimpleFPS::StaticClass());
			FHTNTask_GetItemSimpleFPSMemory* GetItemMemory = (FHTNTask_GetItemSimpleFPSMemory*)GetItemTask->GetMemory();
			GetItemMemory->Item = Gun;
			GetItemMemory->Area = SimpleFPSWorldState->GunData[Gun->Index].Area;

			// add task to get the gun
			UTaskNetwork* DecompositionNetwork = Cast<UTaskNetwork>(Decomposition->Task);
			DecompositionNetwork->AddTaskOrSubNetwork(GetItemTask, Decomposition->GetMemory());

			if(!(SimpleFPSWorldState->GunData[Gun->Index].bLoaded))
			{
				// gun not loaded, so also create task to reload it
				TSharedPtr<FHTNTaskInstance> ReloadGunTask = HTNComp.InstantiateTask(UHTNTask_ReloadCompoundSimpleFPS::StaticClass());
				FHTNTask_ReloadCompoundSimpleFPSMemory* ReloadGunMemory = (FHTNTask_ReloadCompoundSimpleFPSMemory*)ReloadGunTask->GetMemory();
				ReloadGunMemory->Gun = Gun;

				// add task to reload the gun
				DecompositionNetwork->AddTaskOrSubNetwork(ReloadGunTask, Decomposition->GetMemory());
			}

			// add the decomposition
			Decompositions.Add(Decomposition);
		}
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_GetLoadedGunWithNightVisionSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}

	return Decompositions;
}

float UHTNTask_GetLoadedGunWithNightVisionSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}