#include "HTN_PluginPrivatePCH.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_UseMedikitSimpleFPS.h"

UHTNTask_UseMedikitSimpleFPS::UHTNTask_UseMedikitSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Use Medikit";
	HeuristicCost = 1.f;
}

void UHTNTask_UseMedikitSimpleFPS::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_UseMedikitSimpleFPSMemory* Memory = (FHTNTask_UseMedikitSimpleFPSMemory*)TaskMemory;

		// remove the NPC's injured
		SimpleFPSWorldState->bNPCInjured = false;

		// remove the Medikit from inventory
		SimpleFPSWorldState->NPCInventory.RemoveSingleSwap(Memory->Medikit);
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_UseMedikitSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}
}

EHTNExecutionResult UHTNTask_UseMedikitSimpleFPS::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	FHTNTask_UseMedikitSimpleFPSMemory* Memory = (FHTNTask_UseMedikitSimpleFPSMemory*)TaskMemory;
	UE_LOG(LogHTNPlanner, Warning, TEXT("[Use Medikit: %s]"), *(Memory->Medikit->ObjectName));
	return EHTNExecutionResult::Succeeded;
}

float UHTNTask_UseMedikitSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_UseMedikitSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_UseMedikitSimpleFPSMemory);
}

bool UHTNTask_UseMedikitSimpleFPS::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_UseMedikitSimpleFPSMemory* Memory = (FHTNTask_UseMedikitSimpleFPSMemory*)TaskMemory;
		return (SimpleFPSWorldState->NPCInventory.Contains(Memory->Medikit)		&&
				Memory->Medikit->ObjectType == ESimpleFPSObjectTypes::Medikit	&&
				SimpleFPSWorldState->bNPCInjured);
	}
	else
	{
		return false;
	}
}