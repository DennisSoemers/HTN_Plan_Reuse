#include "HTN_PluginPrivatePCH.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_SneakKillSimpleFPS.h"

UHTNTask_SneakKillSimpleFPS::UHTNTask_SneakKillSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Sneak Kill";
	HeuristicCost = 1.f;
}

void UHTNTask_SneakKillSimpleFPS::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_SneakKillSimpleFPSMemory* Memory = (FHTNTask_SneakKillSimpleFPSMemory*)TaskMemory;

		// npc will be aware
		SimpleFPSWorldState->bNPCAware = true;

		// gun will no longer be loaded
		SimpleFPSWorldState->GunData[Memory->Gun->Index].bLoaded = false;

		// player will be injured
		SimpleFPSWorldState->PlayerData[Memory->Player->Index].bInjured = true;
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_SneakKillSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}
}

EHTNExecutionResult UHTNTask_SneakKillSimpleFPS::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	FHTNTask_SneakKillSimpleFPSMemory* Memory = (FHTNTask_SneakKillSimpleFPSMemory*)TaskMemory;
	UE_LOG(LogHTNPlanner, Warning, TEXT("[Sneak Kill: %s using gun: %s]"), *(Memory->Player->ObjectName), *(Memory->Gun->ObjectName));
	return EHTNExecutionResult::Succeeded;
}

float UHTNTask_SneakKillSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_SneakKillSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_SneakKillSimpleFPSMemory);
}

bool UHTNTask_SneakKillSimpleFPS::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_SneakKillSimpleFPSMemory* Memory = (FHTNTask_SneakKillSimpleFPSMemory*)TaskMemory;
		return (!SimpleFPSWorldState->AreaData[Memory->Area].bLighted							&&
				SimpleFPSWorldState->NPCArea == Memory->Area									&&
				SimpleFPSWorldState->PlayerData[Memory->Player->Index].Area == Memory->Area		&&
				Memory->Player->ObjectType == ESimpleFPSObjectTypes::Player						&&
				SimpleFPSWorldState->GunData[Memory->Gun->Index].bLoaded						&&
				SimpleFPSWorldState->GunData[Memory->Gun->Index].bHasNightVision				&&
				SimpleFPSWorldState->NPCInventory.Contains(Memory->Gun));
	}
	else
	{
		return false;
	}
}