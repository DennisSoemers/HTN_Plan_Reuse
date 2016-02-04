#include "HTN_PluginPrivatePCH.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_TakeCoverSimpleFPS.h"

UHTNTask_TakeCoverSimpleFPS::UHTNTask_TakeCoverSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Take Cover";
	HeuristicCost = 1.f;
}

void UHTNTask_TakeCoverSimpleFPS::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		// NPC will be covered
		SimpleFPSWorldState->bNPCCovered = true;
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_TakeCoverSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}
}

EHTNExecutionResult UHTNTask_TakeCoverSimpleFPS::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	FHTNTask_TakeCoverSimpleFPSMemory* Memory = (FHTNTask_TakeCoverSimpleFPSMemory*)TaskMemory;
	UE_LOG(LogHTNPlanner, Warning, TEXT("[Take Cover at Point: %s]"), *(Memory->CoverPoint->ObjectName));
	return EHTNExecutionResult::Succeeded;
}

float UHTNTask_TakeCoverSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_TakeCoverSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_TakeCoverSimpleFPSMemory);
}

bool UHTNTask_TakeCoverSimpleFPS::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_TakeCoverSimpleFPSMemory* Memory = (FHTNTask_TakeCoverSimpleFPSMemory*)TaskMemory;
		return (SimpleFPSWorldState->NPCArea == Memory->Area										&&
				Memory->CoverPoint->ObjectType == ESimpleFPSObjectTypes::CoverPoint					&&
				SimpleFPSWorldState->CoverPointData[Memory->CoverPoint->Index].Area == Memory->Area	&&
				SimpleFPSWorldState->NPCNearbyPOI == Memory->CoverPoint								&&
				!SimpleFPSWorldState->bNPCCovered														);
	}
	else
	{
		return false;
	}
}