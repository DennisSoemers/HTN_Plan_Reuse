#pragma once

#include "HTNPlanner/PrimitiveTask.h"
#include "HTNTask_RegisterService.generated.h"

struct FHTNWorldState;
class UHTNPlannerComponent;
class UHTNService;

/**
 * A Primitive Task that instantly executes by registering a Service that should be ticked by the HTN Planner Component.
 */
UCLASS(Blueprintable, HideDropdown)
class HTN_PLUGIN_API UHTNTask_RegisterService : public UPrimitiveTask
{
	GENERATED_BODY()

public:
	UHTNTask_RegisterService(const FObjectInitializer& ObjectInitializer);

	/** Won't have any effect on world state */
	virtual void ApplyTo(TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const override;
	/** Execution will simply register the service */
	virtual EHTNExecutionResult ExecuteTask(UHTNPlannerComponent& HTNComp, uint8* TaskMemory) override;
	/** Will assume that the background behavior of Services will always have a cost of 0.0 */
	virtual float GetCost(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const override;
	/** Will assume that services can always be registered in any World State */
	virtual bool IsApplicable(const TSharedPtr<FHTNWorldState> WorldState, uint8* TaskMemory) const override;

protected:
	/** The type of Service that should be registered */
	UPROPERTY(Category = Task, EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UHTNService> Service;
};