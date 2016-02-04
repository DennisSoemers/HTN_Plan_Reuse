#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/TaskNetwork.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MakeContactCompoundSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MakeContactSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MoveToAreaSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_TurnOnLightsCompoundSimpleFPS.h"

UHTNTask_MakeContactCompoundSimpleFPS::UHTNTask_MakeContactCompoundSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Make Contact (Compound)";

	// this task can sometimes be decomposed into an empty network, so heuristic of 0
	HeuristicCost = 0.f;
}

TArray<TSharedPtr<FHTNTaskInstance>> UHTNTask_MakeContactCompoundSimpleFPS::FindDecompositions(UHTNPlannerComponent& HTNComp, 
																							   const TSharedPtr<FHTNWorldState> WorldState, 
																							   uint8* TaskMemory)
{
	TArray<TSharedPtr<FHTNTaskInstance>> Decompositions;

	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		if(SimpleFPSWorldState->bNPCAware)
		{
			// NPC is already aware, so we can return an empty network
			Decompositions.Add(HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass()));
		}
		else
		{
			FHTNTask_MakeContactCompoundSimpleFPSMemory* Memory = (FHTNTask_MakeContactCompoundSimpleFPSMemory*)TaskMemory;

			// create decomposition
			TSharedPtr<FHTNTaskInstance> Decomposition = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());

			// create task to move to area
			TSharedPtr<FHTNTaskInstance> MoveToAreaTask = HTNComp.InstantiateTask(UHTNTask_MoveToAreaSimpleFPS::StaticClass());
			FHTNTask_MoveToAreaSimpleFPSMemory* MoveToAreaMemory = (FHTNTask_MoveToAreaSimpleFPSMemory*)MoveToAreaTask->GetMemory();
			MoveToAreaMemory->Area = SimpleFPSWorldState->PlayerData[Memory->Player->Index].Area;

			// create task to turn on lights
			TSharedPtr<FHTNTaskInstance> TurnOnLightsTask = HTNComp.InstantiateTask(UHTNTask_TurnOnLightsCompoundSimpleFPS::StaticClass());
			FHTNTask_TurnOnLightsCompoundSimpleFPSMemory* TurnOnLightsMemory = 
				(FHTNTask_TurnOnLightsCompoundSimpleFPSMemory*)TurnOnLightsTask->GetMemory();
			TurnOnLightsMemory->Area = SimpleFPSWorldState->PlayerData[Memory->Player->Index].Area;

			// create primitive task to make contact
			TSharedPtr<FHTNTaskInstance> MakeContactTask = HTNComp.InstantiateTask(UHTNTask_MakeContactSimpleFPS::StaticClass());
			FHTNTask_MakeContactSimpleFPSMemory* MakeContactMemory = (FHTNTask_MakeContactSimpleFPSMemory*)MakeContactTask->GetMemory();
			MakeContactMemory->Area = SimpleFPSWorldState->PlayerData[Memory->Player->Index].Area;
			MakeContactMemory->Player = Memory->Player;

			// finalize and add Decomposition
			UTaskNetwork* DecompositionNetwork = Cast<UTaskNetwork>(Decomposition->Task);
			DecompositionNetwork->AddTaskOrSubNetwork(MoveToAreaTask, Decomposition->GetMemory());
			DecompositionNetwork->AddTaskOrSubNetwork(TurnOnLightsTask, Decomposition->GetMemory());
			DecompositionNetwork->AddTaskOrSubNetwork(MakeContactTask, Decomposition->GetMemory());
			Decompositions.Add(Decomposition);
		}
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_MakeContactCompoundSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}

	return Decompositions;
}

float UHTNTask_MakeContactCompoundSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_MakeContactCompoundSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_MakeContactCompoundSimpleFPSMemory);
}