#pragma once

#include "HTNPlanner/PrimitiveTask.h"
#include "HTNTask_UnregisterService.generated.h"

struct FHTNWorldState;
class UHTNPlannerComponent;
class UHTNService;

/**
 * A Primitive Task that instantly executes by unregistering a Service that should no longer be ticked by the HTN Planner Component.
 */
UCLASS(Blueprintable, HideDropdown)
class HTN_PLUGIN_API UHTNTask_UnregisterService : public UPrimitiveTask
{
	GENERATED_BODY()

public:
	UHTNTask_UnregisterService(const FObjectInitializer& ObjectInitializer);

	/** Won't have any effect on world state */
	virtual void ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const override;
	/** Execution will simply unregister the service */
	virtual EHTNExecutionResult ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory) override;
	/** Will assume that stopping the background behavior of Services will always have a cost of 0.0 */
	virtual float GetCost(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const override;
	/** Will assume that services can always be unregistered in any World State */
	virtual bool IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const override;

protected:
	/** The type of Service that should be unregistered */
	UPROPERTY(Category = Task, EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UHTNService> Service;
};