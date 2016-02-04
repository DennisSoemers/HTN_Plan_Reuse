#include "HTN_PluginPrivatePCH.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_TurnOnLightsSimpleFPS.h"

UHTNTask_TurnOnLightsSimpleFPS::UHTNTask_TurnOnLightsSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Turn On Lights";
	HeuristicCost = 1.f;
}

void UHTNTask_TurnOnLightsSimpleFPS::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_TurnOnLightsSimpleFPSMemory* Memory = (FHTNTask_TurnOnLightsSimpleFPSMemory*)TaskMemory;

		// area will be lit
		SimpleFPSWorldState->AreaData[Memory->Area].bLighted = true;
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_TurnOnLightsSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}
}

EHTNExecutionResult UHTNTask_TurnOnLightsSimpleFPS::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	FHTNTask_TurnOnLightsSimpleFPSMemory* Memory = (FHTNTask_TurnOnLightsSimpleFPSMemory*)TaskMemory;
	UE_LOG(LogHTNPlanner, Warning, TEXT("[Turn on Lights using Control Box: %s]"), *(Memory->ControlBox->ObjectName));
	return EHTNExecutionResult::Succeeded;
}

float UHTNTask_TurnOnLightsSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_TurnOnLightsSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_TurnOnLightsSimpleFPSMemory);
}

bool UHTNTask_TurnOnLightsSimpleFPS::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_TurnOnLightsSimpleFPSMemory* Memory = (FHTNTask_TurnOnLightsSimpleFPSMemory*)TaskMemory;
		return (!SimpleFPSWorldState->bNPCCovered				&&
				SimpleFPSWorldState->NPCArea == Memory->Area			&&
				!SimpleFPSWorldState->AreaData[Memory->Area].bLighted	&&
				SimpleFPSWorldState->NPCNearbyPOI == Memory->ControlBox	&&
				Memory->ControlBox->ObjectType == ESimpleFPSObjectTypes::ControlBox);
	}
	else
	{
		return false;
	}
}