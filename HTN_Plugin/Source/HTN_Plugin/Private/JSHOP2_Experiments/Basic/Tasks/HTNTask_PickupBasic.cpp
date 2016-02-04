#include "HTN_PluginPrivatePCH.h"
#include "JSHOP2_Experiments/HTNDummyObject.h"
#include "JSHOP2_Experiments/Basic/HTNWorldState_Basic.h"
#include "JSHOP2_Experiments/Basic/Tasks/HTNTask_PickupBasic.h"

void UHTNTask_PickupBasic::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(FHTNWorldState_Basic* BasicWorldState = static_cast<FHTNWorldState_Basic*>(WorldState.Get()))
	{
		BasicWorldState->AddObject(((FHTNTask_PickupBasicMemory*)TaskMemory)->Object);
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_PickupBasic is only compatible with FHTNWorldState_Basic as World State class!"));
	}
}

EHTNExecutionResult UHTNTask_PickupBasic::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	UE_LOG(LogHTNPlanner, Warning, TEXT("[Pick Up: %s]"), *(((FHTNTask_PickupBasicMemory*)TaskMemory)->Object->ObjectName));
	return EHTNExecutionResult::Succeeded;
}

float UHTNTask_PickupBasic::GetHeuristicCost() const
{
	return 1.f;
}

uint16 UHTNTask_PickupBasic::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_PickupBasicMemory);
}

bool UHTNTask_PickupBasic::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	return (((FHTNTask_PickupBasicMemory*)TaskMemory)->Object != nullptr);
}