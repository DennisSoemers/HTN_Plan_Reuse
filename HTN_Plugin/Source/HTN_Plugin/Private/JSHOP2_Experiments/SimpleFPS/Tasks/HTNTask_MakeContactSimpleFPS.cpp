#include "HTN_PluginPrivatePCH.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MakeContactSimpleFPS.h"

UHTNTask_MakeContactSimpleFPS::UHTNTask_MakeContactSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Make Contact";
	HeuristicCost = 1.f;
}

void UHTNTask_MakeContactSimpleFPS::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		// make the NPC aware
		SimpleFPSWorldState->bNPCAware = true;
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_MakeContactSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}
}

EHTNExecutionResult UHTNTask_MakeContactSimpleFPS::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	FHTNTask_MakeContactSimpleFPSMemory* Memory = (FHTNTask_MakeContactSimpleFPSMemory*)TaskMemory;
	UE_LOG(LogHTNPlanner, Warning, TEXT("[Make Contact: %s]"), *(Memory->Player->ObjectName));
	return EHTNExecutionResult::Succeeded;
}

float UHTNTask_MakeContactSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_MakeContactSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_MakeContactSimpleFPSMemory);
}

bool UHTNTask_MakeContactSimpleFPS::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_MakeContactSimpleFPSMemory* Memory = (FHTNTask_MakeContactSimpleFPSMemory*)TaskMemory;
		return (SimpleFPSWorldState->AreaData[Memory->Area].bLighted						&&
				SimpleFPSWorldState->NPCArea == Memory->Area								&&
				SimpleFPSWorldState->PlayerData[Memory->Player->Index].Area == Memory->Area	&&
				!SimpleFPSWorldState->bNPCAware);
	}
	else
	{
		return false;
	}
}