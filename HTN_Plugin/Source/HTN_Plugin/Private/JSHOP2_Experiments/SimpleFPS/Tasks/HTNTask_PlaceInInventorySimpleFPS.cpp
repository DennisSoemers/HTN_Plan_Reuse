#include "HTN_PluginPrivatePCH.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_PlaceInInventorySimpleFPS.h"

UHTNTask_PlaceInInventorySimpleFPS::UHTNTask_PlaceInInventorySimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Place In Inventory";
	HeuristicCost = 1.f;
}

void UHTNTask_PlaceInInventorySimpleFPS::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_PlaceInInventorySimpleFPSMemory* Memory = (FHTNTask_PlaceInInventorySimpleFPSMemory*)TaskMemory;

		// remove item from world
		SimpleFPSWorldState->NPCNearbyPOI = nullptr;
		SimpleFPSWorldState->SetArea(Memory->Item, -1);

		// add item to inventory
		SimpleFPSWorldState->NPCInventory.Add(Memory->Item);
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_PlaceInInventorySimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}
}

EHTNExecutionResult UHTNTask_PlaceInInventorySimpleFPS::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	FHTNTask_PlaceInInventorySimpleFPSMemory* Memory = (FHTNTask_PlaceInInventorySimpleFPSMemory*)TaskMemory;
	UE_LOG(LogHTNPlanner, Warning, TEXT("[Place in Inventory: %s]"), *(Memory->Item->ObjectName));
	return EHTNExecutionResult::Succeeded;
}

float UHTNTask_PlaceInInventorySimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_PlaceInInventorySimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_PlaceInInventorySimpleFPSMemory);
}

bool UHTNTask_PlaceInInventorySimpleFPS::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_PlaceInInventorySimpleFPSMemory* Memory = (FHTNTask_PlaceInInventorySimpleFPSMemory*)TaskMemory;
		return (Memory->Area == SimpleFPSWorldState->NPCArea				&&
				Memory->Area == SimpleFPSWorldState->GetArea(Memory->Item)	&&
				Memory->Item == SimpleFPSWorldState->NPCNearbyPOI			&&
				!SimpleFPSWorldState->bNPCCovered							&&
				SimpleFPSWorldState->AreaData[Memory->Area].bLighted);
	}
	else
	{
		return false;
	}
}