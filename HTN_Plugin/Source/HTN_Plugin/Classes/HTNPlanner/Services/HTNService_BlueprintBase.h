#pragma once
#include "HTNPlanner/HTNService.h"
#include "HTNService_BlueprintBase.generated.h"

/**
 * Base class for blueprint based services in HTN. Do NOT use it for creating native c++ classes!
 *
 * This class fulfills the same role in HTN Planners as BTService_BlueprintBase does in Behavior Trees.
 */
UCLASS(Abstract, Blueprintable)
class HTN_PLUGIN_API UHTNService_BlueprintBase : public UHTNService
{
	GENERATED_BODY()

public:
	UHTNService_BlueprintBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnUpdate(UHTNPlannerComponent& HTNComp, float DeltaSeconds) override;

	/** tick function
	*	@Note that if both generic and AI event versions are implemented only the more
	*	suitable one will be called, meaning the AI version if called for AI, generic one otherwise */
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveTick(AActor* OwnerActor, float DeltaSeconds);

	/** Alternative AI version of ReceiveTick function.
	*	@see ReceiveTick for more details
	*	@Note that if both generic and AI event versions are implemented only the more
	*	suitable one will be called, meaning the AI version if called for AI, generic one otherwise */
	UFUNCTION(BlueprintImplementableEvent, Category = AI)
	void ReceiveTickAI(AAIController* OwnerController, APawn* ControlledPawn, float DeltaSeconds);
};
