#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/TaskNetwork.h"
#include "JSHOP2_Experiments/HTNDummyObject.h"
#include "JSHOP2_Experiments/Basic/HTNWorldState_Basic.h"
#include "JSHOP2_Experiments/Basic/Tasks/HTNTask_DropBasic.h"
#include "JSHOP2_Experiments/Basic/Tasks/HTNTask_PickupBasic.h"
#include "JSHOP2_Experiments/Basic/Tasks/HTNTask_SwapBasic.h"

TArray<TSharedPtr<FHTNTaskInstance>> UHTNTask_SwapBasic::FindDecompositions(UHTNPlannerComponent& HTNComp, 
																			const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory)
{
	TArray<TSharedPtr<FHTNTaskInstance>> Decompositions;

	FHTNTask_SwapBasicMemory* Memory = (FHTNTask_SwapBasicMemory*)TaskMemory;
	if(const FHTNWorldState_Basic* BasicWorldState = static_cast<FHTNWorldState_Basic*>(WorldState.Get()))
	{
		if(BasicWorldState->HasObject(Memory->Object1) && !BasicWorldState->HasObject(Memory->Object2))
		{
			// the first branch of the method
			TSharedPtr<FHTNTaskInstance> Decomposition = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());

			// create task to drop our first object
			TSharedPtr<FHTNTaskInstance> DropTask = HTNComp.InstantiateTask(UHTNTask_DropBasic::StaticClass());
			FHTNTask_DropBasicMemory* DropTaskMemory = (FHTNTask_DropBasicMemory*)DropTask->GetMemory();
			DropTaskMemory->Object = Memory->Object1;

			// create task to pick up our second object
			TSharedPtr<FHTNTaskInstance> PickupTask = HTNComp.InstantiateTask(UHTNTask_PickupBasic::StaticClass());
			FHTNTask_PickupBasicMemory* PickupTaskMemory = (FHTNTask_PickupBasicMemory*)PickupTask->GetMemory();
			PickupTaskMemory->Object = Memory->Object2;

			// finalize and add Decomposition
			UTaskNetwork* DecompositionNetwork = Cast<UTaskNetwork>(Decomposition->Task);
			DecompositionNetwork->AddTaskOrSubNetwork(DropTask, Decomposition->GetMemory());
			DecompositionNetwork->AddTaskOrSubNetwork(PickupTask, Decomposition->GetMemory());
			Decompositions.Add(Decomposition);
		}
		else if(BasicWorldState->HasObject(Memory->Object2) && !BasicWorldState->HasObject(Memory->Object1))
		{
			// the second branch of the method
			TSharedPtr<FHTNTaskInstance> Decomposition = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());

			// create task to drop our second object
			TSharedPtr<FHTNTaskInstance> DropTask = HTNComp.InstantiateTask(UHTNTask_DropBasic::StaticClass());
			FHTNTask_DropBasicMemory* DropTaskMemory = (FHTNTask_DropBasicMemory*)DropTask->GetMemory();
			DropTaskMemory->Object = Memory->Object2;

			// create task to pick up our first object
			TSharedPtr<FHTNTaskInstance> PickupTask = HTNComp.InstantiateTask(UHTNTask_PickupBasic::StaticClass());
			FHTNTask_PickupBasicMemory* PickupTaskMemory = (FHTNTask_PickupBasicMemory*)PickupTask->GetMemory();
			PickupTaskMemory->Object = Memory->Object1;

			// finalize and add Decomposition
			UTaskNetwork* DecompositionNetwork = Cast<UTaskNetwork>(Decomposition->Task);
			DecompositionNetwork->AddTaskOrSubNetwork(DropTask, Decomposition->GetMemory());
			DecompositionNetwork->AddTaskOrSubNetwork(PickupTask, Decomposition->GetMemory());
			Decompositions.Add(Decomposition);
		}
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_SwapBasic is only compatible with FHTNWorldState_Basic as World State class!"));
	}

	return Decompositions;
}

float UHTNTask_SwapBasic::GetHeuristicCost() const
{
	return UHTNTask_DropBasic::StaticClass()->GetDefaultObject<UHTNTask_DropBasic>()->GetHeuristicCost() +
		UHTNTask_PickupBasic::StaticClass()->GetDefaultObject<UHTNTask_PickupBasic>()->GetHeuristicCost();
}

uint16 UHTNTask_SwapBasic::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_SwapBasicMemory);
}