#include "HTN_PluginPrivatePCH.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_AttackRangedSimpleFPS.h"

UHTNTask_AttackRangedSimpleFPS::UHTNTask_AttackRangedSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Attack Ranged";
	HeuristicCost = 1.f;
}

void UHTNTask_AttackRangedSimpleFPS::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_AttackRangedSimpleFPSMemory* Memory = (FHTNTask_AttackRangedSimpleFPSMemory*)TaskMemory;

		// gun will no longer be loaded
		SimpleFPSWorldState->GunData[Memory->Gun->Index].bLoaded = false;

		// player will be injured
		SimpleFPSWorldState->PlayerData[Memory->Player->Index].bInjured = true;
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_AttackRangedSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}
}

EHTNExecutionResult UHTNTask_AttackRangedSimpleFPS::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	FHTNTask_AttackRangedSimpleFPSMemory* Memory = (FHTNTask_AttackRangedSimpleFPSMemory*)TaskMemory;
	UE_LOG(LogHTNPlanner, Warning, TEXT("[Attack Ranged: %s using gun: %s]"), *(Memory->Player->ObjectName), *(Memory->Gun->ObjectName));
	return EHTNExecutionResult::Succeeded;
}

float UHTNTask_AttackRangedSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_AttackRangedSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_AttackRangedSimpleFPSMemory);
}

bool UHTNTask_AttackRangedSimpleFPS::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_AttackRangedSimpleFPSMemory* Memory = (FHTNTask_AttackRangedSimpleFPSMemory*)TaskMemory;
		return (SimpleFPSWorldState->AreaData[Memory->Area].bLighted							&&
				SimpleFPSWorldState->bNPCAware													&&
				SimpleFPSWorldState->NPCArea == Memory->Area									&&
				SimpleFPSWorldState->PlayerData[Memory->Player->Index].Area == Memory->Area		&&
				Memory->Player->ObjectType == ESimpleFPSObjectTypes::Player						&&
				Memory->Gun->ObjectType == ESimpleFPSObjectTypes::Gun							&&
				SimpleFPSWorldState->GunData[Memory->Gun->Index].bLoaded						&&
				SimpleFPSWorldState->NPCInventory.Contains(Memory->Gun));
	}
	else
	{
		return false;
	}
}