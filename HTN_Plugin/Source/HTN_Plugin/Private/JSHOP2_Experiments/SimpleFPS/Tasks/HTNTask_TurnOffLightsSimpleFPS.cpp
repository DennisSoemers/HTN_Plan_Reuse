#include "HTN_PluginPrivatePCH.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_TurnOffLightsSimpleFPS.h"

UHTNTask_TurnOffLightsSimpleFPS::UHTNTask_TurnOffLightsSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Turn Off Lights";
	HeuristicCost = 1.f;
}

void UHTNTask_TurnOffLightsSimpleFPS::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_TurnOffLightsSimpleFPSMemory* Memory = (FHTNTask_TurnOffLightsSimpleFPSMemory*)TaskMemory;

		// npc will no longer be close to control box
		SimpleFPSWorldState->NPCNearbyPOI = nullptr;

		// area will no longer be lit
		SimpleFPSWorldState->AreaData[Memory->Area].bLighted = false;
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_TurnOffLightsSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}
}

EHTNExecutionResult UHTNTask_TurnOffLightsSimpleFPS::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	FHTNTask_TurnOffLightsSimpleFPSMemory* Memory = (FHTNTask_TurnOffLightsSimpleFPSMemory*)TaskMemory;
	UE_LOG(LogHTNPlanner, Warning, TEXT("[Turn off Lights using Control Box: %s]"), *(Memory->ControlBox->ObjectName));
	return EHTNExecutionResult::Succeeded;
}

float UHTNTask_TurnOffLightsSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_TurnOffLightsSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_TurnOffLightsSimpleFPSMemory);
}

bool UHTNTask_TurnOffLightsSimpleFPS::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_TurnOffLightsSimpleFPSMemory* Memory = (FHTNTask_TurnOffLightsSimpleFPSMemory*)TaskMemory;
		return (!SimpleFPSWorldState->bNPCCovered						&&
				SimpleFPSWorldState->NPCArea == Memory->Area			&&
				SimpleFPSWorldState->AreaData[Memory->Area].bLighted	&&
				SimpleFPSWorldState->NPCNearbyPOI == Memory->ControlBox	&&
				Memory->ControlBox->ObjectType == ESimpleFPSObjectTypes::ControlBox);
	}
	else
	{
		return false;
	}
}