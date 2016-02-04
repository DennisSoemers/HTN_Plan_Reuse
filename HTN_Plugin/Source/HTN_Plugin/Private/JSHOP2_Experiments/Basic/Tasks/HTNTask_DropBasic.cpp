#include "HTN_PluginPrivatePCH.h"
#include "JSHOP2_Experiments/HTNDummyObject.h"
#include "JSHOP2_Experiments/Basic/HTNWorldState_Basic.h"
#include "JSHOP2_Experiments/Basic/Tasks/HTNTask_DropBasic.h"

void UHTNTask_DropBasic::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(FHTNWorldState_Basic* BasicWorldState = static_cast<FHTNWorldState_Basic*>(WorldState.Get()))
	{
		BasicWorldState->RemoveObject(((FHTNTask_DropBasicMemory*)TaskMemory)->Object);
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_DropBasic is only compatible with FHTNWorldState_Basic as World State class!"));
	}
}

EHTNExecutionResult UHTNTask_DropBasic::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	UE_LOG(LogHTNPlanner, Warning, TEXT("[Drop: %s]"), *(((FHTNTask_DropBasicMemory*)TaskMemory)->Object->ObjectName));
	return EHTNExecutionResult::Succeeded;
}

float UHTNTask_DropBasic::GetHeuristicCost() const
{
	return 1.f;
}

uint16 UHTNTask_DropBasic::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_DropBasicMemory);
}

bool UHTNTask_DropBasic::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(const FHTNWorldState_Basic* BasicWorldState = static_cast<FHTNWorldState_Basic*>(WorldState.Get()))
	{
		return BasicWorldState->HasObject(((FHTNTask_DropBasicMemory*)TaskMemory)->Object);
	}
	else
	{
		return false;
	}
}