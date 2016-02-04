#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/TaskNetwork.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MoveToAreaSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MoveToPointCompoundSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_TurnOnLightsCompoundSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_TurnOnLightsSimpleFPS.h"

UHTNTask_TurnOnLightsCompoundSimpleFPS::UHTNTask_TurnOnLightsCompoundSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Turn On Lights (Compound)";

	// we'll sometimes return an empty network, so heuristic cost of 0
	HeuristicCost = 0.f;
}

TArray<TSharedPtr<FHTNTaskInstance>> UHTNTask_TurnOnLightsCompoundSimpleFPS::FindDecompositions(UHTNPlannerComponent& HTNComp, 
																								const TSharedPtr<FHTNWorldState> WorldState, 
																								uint8* TaskMemory)
{
	TArray<TSharedPtr<FHTNTaskInstance>> Decompositions;

	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_TurnOnLightsCompoundSimpleFPSMemory* Memory = (FHTNTask_TurnOnLightsCompoundSimpleFPSMemory*)TaskMemory;

		if(SimpleFPSWorldState->AreaData[Memory->Area].bLighted)
		{
			// area is already lighted, so we can return an empty network
			Decompositions.Add(HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass()));
		}
		else
		{
			FSimpleFPSObject* ControlBox = SimpleFPSWorldState->GetControlBoxes()[Memory->Area];

			if(SimpleFPSWorldState->ControlBoxData[ControlBox->Index].Area == Memory->Area)
			{
				// sanity check; this should always be true
				// create decomposition
				TSharedPtr<FHTNTaskInstance> Decomposition = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());

				// create task to move to area
				TSharedPtr<FHTNTaskInstance> MoveToAreaTask = HTNComp.InstantiateTask(UHTNTask_MoveToAreaSimpleFPS::StaticClass());
				FHTNTask_MoveToAreaSimpleFPSMemory* MoveToAreaMemory = (FHTNTask_MoveToAreaSimpleFPSMemory*)MoveToAreaTask->GetMemory();
				MoveToAreaMemory->Area = Memory->Area;

				// create task to move to the control box
				TSharedPtr<FHTNTaskInstance> MoveToPointTask = HTNComp.InstantiateTask(UHTNTask_MoveToPointCompoundSimpleFPS::StaticClass());
				FHTNTask_MoveToPointCompoundSimpleFPSMemory* MoveToPointMemory = 
					(FHTNTask_MoveToPointCompoundSimpleFPSMemory*)MoveToPointTask->GetMemory();
				MoveToPointMemory->Area = Memory->Area;
				MoveToPointMemory->PointOfInterest = ControlBox;

				// create primitive task to turn on lights
				TSharedPtr<FHTNTaskInstance> TurnOnLightsTask = HTNComp.InstantiateTask(UHTNTask_TurnOnLightsSimpleFPS::StaticClass());
				FHTNTask_TurnOnLightsSimpleFPSMemory* TurnOnLightsMemory = (FHTNTask_TurnOnLightsSimpleFPSMemory*)TurnOnLightsTask->GetMemory();
				TurnOnLightsMemory->Area = Memory->Area;
				TurnOnLightsMemory->ControlBox = ControlBox;

				// finalize and add Decomposition
				UTaskNetwork* DecompositionNetwork = Cast<UTaskNetwork>(Decomposition->Task);
				DecompositionNetwork->AddTaskOrSubNetwork(MoveToAreaTask, Decomposition->GetMemory());
				DecompositionNetwork->AddTaskOrSubNetwork(MoveToPointTask, Decomposition->GetMemory());
				DecompositionNetwork->AddTaskOrSubNetwork(TurnOnLightsTask, Decomposition->GetMemory());
				Decompositions.Add(Decomposition);
			}
		}
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_TurnOnLightsCompoundSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}

	return Decompositions;
}

float UHTNTask_TurnOnLightsCompoundSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_TurnOnLightsCompoundSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_TurnOnLightsCompoundSimpleFPSMemory);
}