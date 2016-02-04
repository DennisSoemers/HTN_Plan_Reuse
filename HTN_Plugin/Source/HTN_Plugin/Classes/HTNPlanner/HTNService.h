#pragma once

#include "HTNService.generated.h"

class UHTNPlannerComponent;

/**
 * HTN Services are designed to perform "background" tasks.
 *
 * Services will continuously be ticked as soon as they are registered, and will only stop ticking
 * they are unregistered again or when an HTNPlannerComponent is stopped.
 *
 * The HTNService fulfills a similar role in HTN Planners as the BTService does in a Behavior Tree.
 */
UCLASS(Abstract, HideDropdown)
class HTN_PLUGIN_API UHTNService : public UObject
{
	GENERATED_BODY()

public:
	UHTNService(const FObjectInitializer& ObjectInitializer);

	void SetOwner(AActor* ActorOwner);
	void Tick(UHTNPlannerComponent& HTNComp, float DeltaSeconds);

protected:
	/** Cached AIController owner of HTNPlannerComponent. */
	UPROPERTY(Transient)
	AAIController* AIOwner;

	/** Cached actor owner of HTNPlannerComponent. */
	UPROPERTY(Transient)
	AActor* ActorOwner;

	/** defines time span between subsequent ticks of the service */
	UPROPERTY(Category = Service, EditAnywhere, meta = (ClampMin = "0.001"))
	float Interval;
	/** adds random range to service's Interval */
	UPROPERTY(Category = Service, EditAnywhere, meta = (ClampMin = "0.0"))
	float RandomDeviation;

	float NextTickRemainingTime;
	float AccumulatedDeltaTime;

	/** Called whenever the specified amount of time has expired. The service's functionality should be implemented here */
	virtual void OnUpdate(UHTNPlannerComponent& HTNComp, float DeltaSeconds);
};