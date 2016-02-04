#include "HTN_PluginPrivatePCH.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_AttackMeleeSimpleFPS.h"

UHTNTask_AttackMeleeSimpleFPS::UHTNTask_AttackMeleeSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Attack Melee";
	HeuristicCost = 1.f;
}

void UHTNTask_AttackMeleeSimpleFPS::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		// player will be injured
		SimpleFPSWorldState->PlayerData[((FHTNTask_AttackMeleeSimpleFPSMemory*)TaskMemory)->Player->Index].bInjured = true;
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_AttackMeleeSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}
}

EHTNExecutionResult UHTNTask_AttackMeleeSimpleFPS::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	UE_LOG(LogHTNPlanner, Warning, TEXT("[Attack Melee: %s using knife: %s]"), 
		   *(((FHTNTask_AttackMeleeSimpleFPSMemory*)TaskMemory)->Player->ObjectName), 
		   *(((FHTNTask_AttackMeleeSimpleFPSMemory*)TaskMemory)->Knife->ObjectName));
	return EHTNExecutionResult::Succeeded;
}

float UHTNTask_AttackMeleeSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_AttackMeleeSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_AttackMeleeSimpleFPSMemory);
}

bool UHTNTask_AttackMeleeSimpleFPS::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_AttackMeleeSimpleFPSMemory* Memory = (FHTNTask_AttackMeleeSimpleFPSMemory*)TaskMemory;
		return (SimpleFPSWorldState->AreaData[Memory->Area].bLighted							&&
				SimpleFPSWorldState->bNPCAware													&&
				SimpleFPSWorldState->NPCArea == Memory->Area									&&
				SimpleFPSWorldState->PlayerData[Memory->Player->Index].Area == Memory->Area		&&
				Memory->Player->ObjectType == ESimpleFPSObjectTypes::Player						&&
				Memory->Knife->ObjectType == ESimpleFPSObjectTypes::Knife						&&
				SimpleFPSWorldState->NPCInventory.Contains(Memory->Knife)						&&
				SimpleFPSWorldState->NPCNearbyPOI == Memory->Player								&&
				!SimpleFPSWorldState->bNPCCovered);
	}
	else
	{
		return false;
	}
}