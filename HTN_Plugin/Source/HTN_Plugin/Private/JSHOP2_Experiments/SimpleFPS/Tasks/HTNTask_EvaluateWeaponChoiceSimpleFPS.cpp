#include "HTN_PluginPrivatePCH.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_EvaluateWeaponChoiceSimpleFPS.h"

UHTNTask_EvaluateWeaponChoiceSimpleFPS::UHTNTask_EvaluateWeaponChoiceSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskName = "Evaluate Weapon Choice";
	HeuristicCost = 0.f;
}

void UHTNTask_EvaluateWeaponChoiceSimpleFPS::ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	// no effect on world state
}

EHTNExecutionResult UHTNTask_EvaluateWeaponChoiceSimpleFPS::ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	UE_LOG(LogHTNPlanner, Warning, TEXT("[!!Evaluate Weapon Choice]"));
	return EHTNExecutionResult::Succeeded;
}

float UHTNTask_EvaluateWeaponChoiceSimpleFPS::GetCost(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	if(FHTNWorldState_SimpleFPS* SimpleFPSWorldState = static_cast<FHTNWorldState_SimpleFPS*>(WorldState.Get()))
	{
		if(SimpleFPSWorldState->GetConstData()->PreferredWeaponChoice == 0)
		{
			//UE_LOG(LogHTNPlanner, Warning, TEXT("No preferred Weapon Choice!"));
			return 0.f;
		}
		else if(SimpleFPSWorldState->GetConstData()->PreferredWeaponChoice ==
				(uint8)((FHTNTask_EvaluateWeaponChoiceSimpleFPSMemory*)TaskMemory)->Choice)
		{
			//UE_LOG(LogHTNPlanner, Warning, TEXT("Correct Weapon Choice!"));
			return 0.f;
		}
		  
		//UE_LOG(LogHTNPlanner, Warning, TEXT("Punishing Weapon Choice!"));
		return 0.f;
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UHTNTask_EvaluateWeaponChoiceSimpleFPS is only compatible with FHTNWorldState_SimpleFPS as World State class!"));
	}

	return 0.f;
}

uint16 UHTNTask_EvaluateWeaponChoiceSimpleFPS::GetInstanceMemorySize() const
{
	return sizeof(FHTNTask_EvaluateWeaponChoiceSimpleFPSMemory);
}

FString UHTNTask_EvaluateWeaponChoiceSimpleFPS::GetTaskName(uint8* TaskMemory) const
{
	return TaskName;
}

bool UHTNTask_EvaluateWeaponChoiceSimpleFPS::IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const
{
	return true;
}