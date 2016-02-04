#include "HTN_PluginPrivatePCH.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_ReloadSimpleFPS.h"

UHTNTask_ReloadSimpleFPS::UHTNTask_ReloadSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Reload";
	HeuristicCost = 1.f;
}

void UHTNTask_ReloadSimpleFPS::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_ReloadSimpleFPSMemory* Memory = (FHTNTask_ReloadSimpleFPSMemory*)TaskMemory;

		// remove ammo from inventory
		SimpleFPSWorldState->NPCInventory.RemoveSingleSwap(Memory->Ammo);

		// load the gun
		SimpleFPSWorldState->GunData[Memory->Gun->Index].bLoaded = true;
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_ReloadSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}
}

EHTNExecutionResult UHTNTask_ReloadSimpleFPS::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	FHTNTask_ReloadSimpleFPSMemory* Memory = (FHTNTask_ReloadSimpleFPSMemory*)TaskMemory;
	UE_LOG(LogHTNPlanner, Warning, TEXT("[Reload Gun: %s]"), *(Memory->Gun->ObjectName));
	return EHTNExecutionResult::Succeeded;
}

float UHTNTask_ReloadSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_ReloadSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_ReloadSimpleFPSMemory);
}

bool UHTNTask_ReloadSimpleFPS::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_ReloadSimpleFPSMemory* Memory = (FHTNTask_ReloadSimpleFPSMemory*)TaskMemory;
		return (SimpleFPSWorldState->NPCInventory.Contains(Memory->Gun) &&
				SimpleFPSWorldState->NPCInventory.Contains(Memory->Ammo) &&
				!SimpleFPSWorldState->GunData[Memory->Gun->Index].bLoaded	&&
				SimpleFPSWorldState->GunData[Memory->Gun->Index].Ammo == Memory->Ammo->Index);
	}
	else
	{
		return false;
	}
}