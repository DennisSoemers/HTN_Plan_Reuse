#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/TaskNetwork.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_AttackRangedCompoundSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_EvaluateWeaponChoiceSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_GetLoadedGunSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_GetLoadedGunWithNightVisionSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MakeContactCompoundSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MoveToAreaSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MoveToPointCompoundSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_SneakKillCompoundSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_TakeCoverSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_TakeCoverAndWoundPlayerSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_TurnOnLightsCompoundSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_TurnOffLightsCompoundSimpleFPS.h"

UHTNTask_TakeCoverAndWoundPlayerSimpleFPS::UHTNTask_TakeCoverAndWoundPlayerSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Take Cover And Wound Player";

	// minimum heuristic of the two possible paths
	HeuristicCost = FMath::Min(
		UHTNTask_EvaluateWeaponChoiceSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_EvaluateWeaponChoiceSimpleFPS>()->GetHeuristicCost() +
		UHTNTask_GetLoadedGunSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_GetLoadedGunSimpleFPS>()->GetHeuristicCost() +
		UHTNTask_MoveToAreaSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_MoveToAreaSimpleFPS>()->GetHeuristicCost() +
		UHTNTask_TurnOnLightsCompoundSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_TurnOnLightsCompoundSimpleFPS>()->GetHeuristicCost() +
		UHTNTask_MakeContactCompoundSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_MakeContactCompoundSimpleFPS>()->GetHeuristicCost() +
		UHTNTask_MoveToPointCompoundSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_MoveToPointCompoundSimpleFPS>()->GetHeuristicCost() +
		UHTNTask_TakeCoverSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_TakeCoverSimpleFPS>()->GetHeuristicCost() +
		UHTNTask_AttackRangedCompoundSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_AttackRangedCompoundSimpleFPS>()->GetHeuristicCost()
		,
		UHTNTask_EvaluateWeaponChoiceSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_EvaluateWeaponChoiceSimpleFPS>()->GetHeuristicCost() +
		UHTNTask_GetLoadedGunWithNightVisionSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_GetLoadedGunWithNightVisionSimpleFPS>()->GetHeuristicCost() +
		UHTNTask_MoveToAreaSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_MoveToAreaSimpleFPS>()->GetHeuristicCost() +
		UHTNTask_TurnOffLightsCompoundSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_TurnOffLightsCompoundSimpleFPS>()->GetHeuristicCost() +
		UHTNTask_MoveToPointCompoundSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_MoveToPointCompoundSimpleFPS>()->GetHeuristicCost() +
		UHTNTask_TakeCoverSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_TakeCoverSimpleFPS>()->GetHeuristicCost() +
		UHTNTask_SneakKillCompoundSimpleFPS::StaticClass()->GetDefaultObject<UHTNTask_SneakKillCompoundSimpleFPS>()->GetHeuristicCost()
		);
}

TArray<TSharedPtr<FHTNTaskInstance>> UHTNTask_TakeCoverAndWoundPlayerSimpleFPS::FindDecompositions(UHTNPlannerComponent& HTNComp, 
																								   const TSharedPtr<FHTNWorldState> WorldState,
																								   uint8* TaskMemory)
{
	TArray<TSharedPtr<FHTNTaskInstance>> Decompositions;

	if(const FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		FHTNTask_TakeCoverAndWoundPlayerSimpleFPSMemory* Memory = (FHTNTask_TakeCoverAndWoundPlayerSimpleFPSMemory*)TaskMemory;

		// we'll always have 2 decompositions; working towards a ranged attack and working towards a sneak kill
		TSharedPtr<FHTNTaskInstance> DecompositionRanged = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());
		TSharedPtr<FHTNTaskInstance> DecompositionSneak = HTNComp.InstantiateNetwork(UTaskNetwork::StaticClass());

		// start with the ranged attack
		// create task to evaluate our weapon choice
		TSharedPtr<FHTNTaskInstance> EvaluateWeaponChoiceTask = 
			HTNComp.InstantiateTask(UHTNTask_EvaluateWeaponChoiceSimpleFPS::StaticClass());
		FHTNTask_EvaluateWeaponChoiceSimpleFPSMemory* EvaluateWeaponChoiceMemory =
			(FHTNTask_EvaluateWeaponChoiceSimpleFPSMemory*)EvaluateWeaponChoiceTask->GetMemory();
		EvaluateWeaponChoiceMemory->Choice = EHTNWeaponChoiceSimpleFPS::Ranged;

		// create task to get a loaded gun
		TSharedPtr<FHTNTaskInstance> GetLoadedGunTask = HTNComp.InstantiateTask(UHTNTask_GetLoadedGunSimpleFPS::StaticClass());

		// create task to move to the area
		TSharedPtr<FHTNTaskInstance> MoveToAreaTask = HTNComp.InstantiateTask(UHTNTask_MoveToAreaSimpleFPS::StaticClass());
		FHTNTask_MoveToAreaSimpleFPSMemory* MoveToAreaMemory = (FHTNTask_MoveToAreaSimpleFPSMemory*)MoveToAreaTask->GetMemory();
		MoveToAreaMemory->Area = Memory->Area;

		// create task to turn on the lights
		TSharedPtr<FHTNTaskInstance> TurnOnLightsTask = HTNComp.InstantiateTask(UHTNTask_TurnOnLightsCompoundSimpleFPS::StaticClass());
		FHTNTask_TurnOnLightsCompoundSimpleFPSMemory* TurnOnLightsMemory = 
			(FHTNTask_TurnOnLightsCompoundSimpleFPSMemory*)TurnOnLightsTask->GetMemory();
		TurnOnLightsMemory->Area = Memory->Area;

		// create task to make contact with the player
		TSharedPtr<FHTNTaskInstance> MakeContactTask = HTNComp.InstantiateTask(UHTNTask_MakeContactCompoundSimpleFPS::StaticClass());
		FHTNTask_MakeContactCompoundSimpleFPSMemory* MakeContactMemory = (FHTNTask_MakeContactCompoundSimpleFPSMemory*)MakeContactTask->GetMemory();
		MakeContactMemory->Player = Memory->Player;

		// create task to move to cover point
		TSharedPtr<FHTNTaskInstance> MoveToPointTask = HTNComp.InstantiateTask(UHTNTask_MoveToPointCompoundSimpleFPS::StaticClass());
		FHTNTask_MoveToPointCompoundSimpleFPSMemory* MoveToPointMemory = (FHTNTask_MoveToPointCompoundSimpleFPSMemory*)MoveToPointTask->GetMemory();
		MoveToPointMemory->Area = Memory->Area;
		MoveToPointMemory->PointOfInterest = Memory->CoverPoint;

		// create task to take cover
		TSharedPtr<FHTNTaskInstance> TakeCoverTask = HTNComp.InstantiateTask(UHTNTask_TakeCoverSimpleFPS::StaticClass());
		FHTNTask_TakeCoverSimpleFPSMemory* TakeCoverMemory = (FHTNTask_TakeCoverSimpleFPSMemory*)TakeCoverTask->GetMemory();
		TakeCoverMemory->Area = Memory->Area;
		TakeCoverMemory->CoverPoint = Memory->CoverPoint;

		// create task for ranged attack
		TSharedPtr<FHTNTaskInstance> AttackRangedTask = HTNComp.InstantiateTask(UHTNTask_AttackRangedCompoundSimpleFPS::StaticClass());
		FHTNTask_AttackRangedCompoundSimpleFPSMemory* AttackRangedMemory = 
			(FHTNTask_AttackRangedCompoundSimpleFPSMemory*)AttackRangedTask->GetMemory();
		AttackRangedMemory->Area = Memory->Area;
		AttackRangedMemory->Player = Memory->Player;

		// finalize and add the Decomposition
		UTaskNetwork* DecompositionRangedNetwork = Cast<UTaskNetwork>(DecompositionRanged->Task);
		DecompositionRangedNetwork->AddTaskOrSubNetwork(EvaluateWeaponChoiceTask, DecompositionRanged->GetMemory());
		DecompositionRangedNetwork->AddTaskOrSubNetwork(GetLoadedGunTask, DecompositionRanged->GetMemory());
		DecompositionRangedNetwork->AddTaskOrSubNetwork(MoveToAreaTask, DecompositionRanged->GetMemory());
		DecompositionRangedNetwork->AddTaskOrSubNetwork(TurnOnLightsTask, DecompositionRanged->GetMemory());
		DecompositionRangedNetwork->AddTaskOrSubNetwork(MakeContactTask, DecompositionRanged->GetMemory());
		DecompositionRangedNetwork->AddTaskOrSubNetwork(MoveToPointTask, DecompositionRanged->GetMemory());
		DecompositionRangedNetwork->AddTaskOrSubNetwork(TakeCoverTask, DecompositionRanged->GetMemory());
		DecompositionRangedNetwork->AddTaskOrSubNetwork(AttackRangedTask, DecompositionRanged->GetMemory());
		Decompositions.Add(DecompositionRanged);

		// now the sneak kill option
		// create task to evaluate our weapon choice
		EvaluateWeaponChoiceTask = HTNComp.InstantiateTask(UHTNTask_EvaluateWeaponChoiceSimpleFPS::StaticClass());
		EvaluateWeaponChoiceMemory = (FHTNTask_EvaluateWeaponChoiceSimpleFPSMemory*)EvaluateWeaponChoiceTask->GetMemory();
		EvaluateWeaponChoiceMemory->Choice = EHTNWeaponChoiceSimpleFPS::Stealth;

		// create task to get a loaded gun with nightvision
		TSharedPtr<FHTNTaskInstance> GetLoadedGunWithNightvisionTask = 
			HTNComp.InstantiateTask(UHTNTask_GetLoadedGunWithNightVisionSimpleFPS::StaticClass());

		// create task to move to the area
		MoveToAreaTask = HTNComp.InstantiateTask(UHTNTask_MoveToAreaSimpleFPS::StaticClass());
		MoveToAreaMemory = (FHTNTask_MoveToAreaSimpleFPSMemory*)MoveToAreaTask->GetMemory();
		MoveToAreaMemory->Area = Memory->Area;

		// create task to turn off the lights
		TSharedPtr<FHTNTaskInstance> TurnOffLightsTask = HTNComp.InstantiateTask(UHTNTask_TurnOffLightsCompoundSimpleFPS::StaticClass());
		FHTNTask_TurnOffLightsCompoundSimpleFPSMemory* TurnOffLightsMemory = 
			(FHTNTask_TurnOffLightsCompoundSimpleFPSMemory*)TurnOffLightsTask->GetMemory();
		TurnOffLightsMemory->Area = Memory->Area;

		// create task to move to cover point
		MoveToPointTask = HTNComp.InstantiateTask(UHTNTask_MoveToPointCompoundSimpleFPS::StaticClass());
		MoveToPointMemory = (FHTNTask_MoveToPointCompoundSimpleFPSMemory*)MoveToPointTask->GetMemory();
		MoveToPointMemory->Area = Memory->Area;
		MoveToPointMemory->PointOfInterest = Memory->CoverPoint;

		// create task to take cover
		TakeCoverTask = HTNComp.InstantiateTask(UHTNTask_TakeCoverSimpleFPS::StaticClass());
		TakeCoverMemory = (FHTNTask_TakeCoverSimpleFPSMemory*)TakeCoverTask->GetMemory();
		TakeCoverMemory->Area = Memory->Area;
		TakeCoverMemory->CoverPoint = Memory->CoverPoint;

		// create task for sneak kill
		TSharedPtr<FHTNTaskInstance> SneakKillTask = HTNComp.InstantiateTask(UHTNTask_SneakKillCompoundSimpleFPS::StaticClass());
		FHTNTask_SneakKillCompoundSimpleFPSMemory* SneakKillMemory = 
			(FHTNTask_SneakKillCompoundSimpleFPSMemory*)SneakKillTask->GetMemory();
		SneakKillMemory->Area = Memory->Area;
		SneakKillMemory->Player = Memory->Player;

		// finalize and add the Decomposition
		UTaskNetwork* DecompositionSneakNetwork = Cast<UTaskNetwork>(DecompositionSneak->Task);
		DecompositionSneakNetwork->AddTaskOrSubNetwork(EvaluateWeaponChoiceTask, DecompositionSneak->GetMemory());
		DecompositionSneakNetwork->AddTaskOrSubNetwork(GetLoadedGunWithNightvisionTask, DecompositionSneak->GetMemory());
		DecompositionSneakNetwork->AddTaskOrSubNetwork(MoveToAreaTask, DecompositionSneak->GetMemory());
		DecompositionSneakNetwork->AddTaskOrSubNetwork(TurnOffLightsTask, DecompositionSneak->GetMemory());
		DecompositionSneakNetwork->AddTaskOrSubNetwork(MoveToPointTask, DecompositionSneak->GetMemory());
		DecompositionSneakNetwork->AddTaskOrSubNetwork(TakeCoverTask, DecompositionSneak->GetMemory());
		DecompositionSneakNetwork->AddTaskOrSubNetwork(SneakKillTask, DecompositionSneak->GetMemory());
		Decompositions.Add(DecompositionSneak);
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_TakeCoverAndWoundPlayerSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}

	return Decompositions;
}

float UHTNTask_TakeCoverAndWoundPlayerSimpleFPS::GetHeuristicCost() const
{
	return HeuristicCost;
}

uint16 UHTNTask_TakeCoverAndWoundPlayerSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_TakeCoverAndWoundPlayerSimpleFPSMemory);
}