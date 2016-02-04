#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/HTNPlanner.h"
#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/TaskNetwork.h"
#include "JSHOP2_Experiments/HTNDummyObject.h"
#include "JSHOP2_Experiments/Basic/HTNWorldState_Basic.h"
#include "JSHOP2_Experiments/Basic/JSHOP2_ExperimenterBasic.h"
#include "JSHOP2_Experiments/Basic/Tasks/HTNTask_SwapBasic.h"

UHTNPlanner* AJSHOP2_ExperimenterBasic::GeneratePlanner()
{
	UHTNPlanner* Planner = NewObject<UHTNPlanner>(this);
	Planner->bIgnoreTaskCosts = false;
	Planner->bDepthFirstSearch = bDepthFirstSearch;
	Planner->bLoop = bLoop;
	Planner->MaxSearchTime = MaxSearchTime;
	Planner->EmptyWorldState = TSharedPtr<FHTNWorldState>(new FHTNWorldState_Basic());

	// create our objects
	ObjectPool.Reset();

	FHTNDummyObject* Kiwi = new FHTNDummyObject();
	Kiwi->ObjectName = "Kiwi";
	ObjectPool.Add(MakeShareable<FHTNDummyObject>(Kiwi));

	FHTNDummyObject* Banjo = new FHTNDummyObject();
	Banjo->ObjectName = "Banjo";
	ObjectPool.Add(MakeShareable<FHTNDummyObject>(Banjo));

	// create our Task Network
	TSharedPtr<FHTNTaskInstance> TaskNetwork = HTNComp->InstantiateNetwork(UTaskNetwork::StaticClass());
	// create our swap task
	TSharedPtr<FHTNTaskInstance> SwapTask = HTNComp->InstantiateTask(UHTNTask_SwapBasic::StaticClass());
	FHTNTask_SwapBasicMemory* SwapTaskMemory = (FHTNTask_SwapBasicMemory*)SwapTask->GetMemory();
	SwapTaskMemory->Object1 = Banjo;
	SwapTaskMemory->Object2 = Kiwi;
	Cast<UTaskNetwork>(TaskNetwork->Task)->AddTaskOrSubNetwork(SwapTask, TaskNetwork->GetMemory());
	Planner->TaskNetworkInstance = TaskNetwork;

	return Planner;
}

void AJSHOP2_ExperimenterBasic::InitializeWorldState(FHTNWorldState_Basic* WorldState)
{
	// initialize our world state, where we have the kiwi but not the banjo
	WorldState->AddObject(ObjectPool[0].Get());
}