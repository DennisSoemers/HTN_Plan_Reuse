#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/TaskNetwork.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_GetItemSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_RestoreFullHealthSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_UseMedikitSimpleFPS.h"

UHTNTask_RestoreFullHealthSimpleFPS::UHTNTask_RestoreFullHealthSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Restore Full Health";

	// we'll sometimes decompose into an empty network, so heuristic cost of 0
	HeuristicCost = 0.f;
}

TArray<TSharedPtr<FHTNTaskInstance>> UHTNTask_RestoreFullHealthSimpleFPS::FindDecompositions(UHTNPlannerComponent& HTNComp, 
																							 const TSharedPtr<FHTNWorldState> WorldState, 
																							 uint8* TaskMemory)
{
	TArray<TSharedPtr<FHTNTaskInstance>> Decompositions;

	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		if(!SimpleFPSWorldState->bNPCInjured)
		{
			// first branch; NPC isn't injured, so simply return an empty task network
			Decompositions.Add(HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass()));
		}
		else
		{
			// second branch; see if NPC is holding any medikits, and use the first medikit found
			bool bMedikitFound = false;
			for(FSimpleFPSObject* Object : SimpleFPSWorldState->NPCInventory)
			{
				if(Object->ObjectType == ESimpleFPSObjectTypes::Medikit)
				{
					// found a medikit
					bMedikitFound = true;

					// create decomposition
					TSharedPtr<FHTNTaskInstance> Decomposition = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());

					// create task to use medikit
					TSharedPtr<FHTNTaskInstance> UseMedikitTask = HTNComp.InstantiateTask(UHTNTask_UseMedikitSimpleFPS::StaticClass());
					FHTNTask_UseMedikitSimpleFPSMemory* UseMedikitMemory = (FHTNTask_UseMedikitSimpleFPSMemory*)UseMedikitTask->GetMemory();
					UseMedikitMemory->Medikit = Object;

					// finalize and add Decomposition
					UTaskNetwork* DecompositionNetwork = Cast<UTaskNetwork>(Decomposition->Task);
					DecompositionNetwork->AddTaskOrSubNetwork(UseMedikitTask, Decomposition->GetMemory());
					Decompositions.Add(Decomposition);

					// doesn't matter which medikit we use, so only stick to this first result and break loop
					break;
				}
			}

			if(!bMedikitFound)
			{
				// third branch; search for a medikit in some area and go pick it up and use it
				// first collect all medikits that have a valid area
				const TArray<FSimpleFPSObject*>& WorldStateMedikits = SimpleFPSWorldState->GetMedikits();
				TArray<FSimpleFPSObject*> Medikits;
				Medikits.Reserve(WorldStateMedikits.Num());
				int32 NPCArea = SimpleFPSWorldState->NPCArea;

				for(FSimpleFPSObject* Medikit : WorldStateMedikits)
				{
					int32 MedikitArea = SimpleFPSWorldState->MedikitData[Medikit->Index].Area;
					if(MedikitArea >= 0)	// valid area
					{
						if(SimpleFPSWorldState->IsReachable(NPCArea, MedikitArea))	// can reach medikit from current area
						{
							Medikits.Add(Medikit);
						}
					}
				}

				// sort medikits according to distance from NPC
				const TArray<int32>& DistanceRow = SimpleFPSWorldState->GetDistanceRow(NPCArea);
				Medikits.Sort([&](const FSimpleFPSObject& A, const FSimpleFPSObject& B) 
					{ 
						return (DistanceRow[SimpleFPSWorldState->MedikitData[A.Index].Area] < 
								DistanceRow[SimpleFPSWorldState->MedikitData[B.Index].Area]);
					});

				// in the order obtained by sorting, try getting the medikits and then using them
				for(int32 Idx = 0; Idx < Medikits.Num(); ++Idx)
				{
					FSimpleFPSObject* Medikit = Medikits[Idx];

					// create decomposition
					TSharedPtr<FHTNTaskInstance> Decomposition = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());

					// create task to get medikit
					TSharedPtr<FHTNTaskInstance> GetItemTask = HTNComp.InstantiateTask(UHTNTask_GetItemSimpleFPS::StaticClass());
					FHTNTask_GetItemSimpleFPSMemory* GetItemMemory = (FHTNTask_GetItemSimpleFPSMemory*)GetItemTask->GetMemory();
					GetItemMemory->Area = SimpleFPSWorldState->MedikitData[Medikit->Index].Area;
					GetItemMemory->Item = Medikit;

					// create task to use medikit
					TSharedPtr<FHTNTaskInstance> UseMedikitTask = HTNComp.InstantiateTask(UHTNTask_UseMedikitSimpleFPS::StaticClass());
					FHTNTask_UseMedikitSimpleFPSMemory* UseMedikitMemory = (FHTNTask_UseMedikitSimpleFPSMemory*)UseMedikitTask->GetMemory();
					UseMedikitMemory->Medikit = Medikit;

					// finalize and add Decomposition
					UTaskNetwork* DecompositionNetwork = Cast<UTaskNetwork>(Decomposition->Task);
					DecompositionNetwork->AddTaskOrSubNetwork(GetItemTask, Decomposition->GetMemory());
					DecompositionNetwork->AddTaskOrSubNetwork(UseMedikitTask, Decomposition->GetMemory());
					Decompositions.Add(Decomposition);
				}
			}
		}
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_RestoreFullHealthSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}

	return Decompositions;
}

float UHTNTask_RestoreFullHealthSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}