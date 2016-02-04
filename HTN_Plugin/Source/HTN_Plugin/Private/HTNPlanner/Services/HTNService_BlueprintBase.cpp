#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/Services/HTNService_BlueprintBase.h"

UHTNService_BlueprintBase::UHTNService_BlueprintBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{}

void UHTNService_BlueprintBase::OnUpdate(UHTNPlannerComponent& HTNComp, float DeltaSeconds)
{
	Super::OnUpdate(HTNComp, DeltaSeconds);

	if(AIOwner != nullptr)
	{
		ReceiveTickAI(AIOwner, AIOwner->GetPawn(), DeltaSeconds);
	}
	else
	{
		ReceiveTick(ActorOwner, DeltaSeconds);
	}
}