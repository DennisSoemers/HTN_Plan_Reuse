#include "HTN_PluginPrivatePCH.h"

#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/HTNService.h"

UHTNService::UHTNService(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NextTickRemainingTime = 0.f;
	AccumulatedDeltaTime = 0.f;

	Interval = 0.5f;
	RandomDeviation = 0.1f;
}

void UHTNService::OnUpdate(UHTNPlannerComponent& HTNComp, float DeltaSeconds)
{
	// no default implementation
}

void UHTNService::SetOwner(AActor* InActorOwner)
{
	ActorOwner = InActorOwner;
	AIOwner = Cast<AAIController>(InActorOwner);
}

void UHTNService::Tick(UHTNPlannerComponent& HTNComp, float DeltaSeconds)
{
	float UseDeltaTime = DeltaSeconds;

	NextTickRemainingTime -= DeltaSeconds;
	AccumulatedDeltaTime += DeltaSeconds;

	if(NextTickRemainingTime > 0.0f)
	{
		return;
	}

	UseDeltaTime = AccumulatedDeltaTime;
	AccumulatedDeltaTime = 0.0f;

	OnUpdate(HTNComp, UseDeltaTime);

	NextTickRemainingTime = FMath::FRandRange(FMath::Max(0.0f, Interval - RandomDeviation), (Interval + RandomDeviation));
}