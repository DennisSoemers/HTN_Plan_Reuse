#pragma once

#include "JSHOP2_Experimenter.generated.h"

class UDataCollector;
class UHTNPlanner;
class UHTNPlannerComponent;

/**
 * Base class for Actors that can be placed in levels to carry out experiments based on JSHOP2
 * example domains/problems when gameplay in a level begins.
 *
 * Subclasses should implement specific initial Task Network and can also be used to set up initial
 * World State.
 */
UCLASS(Abstract, HideDropdown, HideCategories=("Actor Tick", "Rendering", "Replication", "Input"))
class HTN_PLUGIN_API AJSHOP2_Experimenter : public AActor
{
	GENERATED_BODY()

public:
	AJSHOP2_Experimenter(const FObjectInitializer& ObjectInitializer);

	/** Overridden to start the experiment */
	virtual void BeginPlay() override;
	/** Overridden to clean up experiment */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	bool FinishedExperiment() const;

	/** Must be implemented by subclasses to dynamically generate and return a Planner asset to be used in experiment */
	virtual UHTNPlanner* GeneratePlanner();

	void SetDataCollector(UDataCollector* DataCollector);

	virtual void Tick(float DeltaSeconds) override;

protected:
	/** If true, the Planner will perform a breadth-first search. Otherwise, a more breadth-first / A* style search will be used */
	UPROPERTY(EditAnywhere, Category = "Hierarchical Task Network Planner")
	bool bDepthFirstSearch;
	/** If true, the Planner will ignore costs assigned to Tasks and only search for the first plan */
	UPROPERTY(EditAnywhere, Category = "Hierarchical Task Network Planner")
	bool bIgnoreCosts;
	/** If true, the Planner will re-start from the initial Task Network to create a new plan once the previous plan has been executed. */
	UPROPERTY(EditAnywhere, Category = "Hierarchical Task Network Planner")
	bool bLoop;
	/** The maximum amount of time (in seconds) that the planner can dedicate to planning per Tick */
	UPROPERTY(EditAnywhere, Category = "Hierarchical Task Network Planner", meta = (ClampMin = 0, ClampMax = 1))
	float MaxSearchTime;

	/** The maximum amount of time (in seconds) that the planner is allowed to spend on finding a first solution. */
	UPROPERTY(EditAnywhere, Category = "Experiments")
	float MaxSearchTimeFirstSolution;
	/** The maximum amount of time (in seconds) that the planner is allowed to spend on finding an optimal solution */
	UPROPERTY(EditAnywhere, Category = "Experiments")
	float MaxSearchTimeOptimalSolution;

	UPROPERTY(EditAnywhere, Category = "Experiments")
	uint8 bExperimentPlanReuse : 1;

	uint8 bConstructedInitialPlan : 1;
	uint8 bExecutedPlan : 1;
	uint8 bReplannedWithoutReuse : 1;

	// flags for replanning with reuse (number = min streak length, on/off = probabilistic replanning)
	uint8 bReplannedWithReuse_1_off : 1;
	uint8 bReplannedWithReuse_1_on : 1;
	uint8 bReplannedWithReuse_10_off : 1;
	uint8 bReplannedWithReuse_10_on : 1;
	uint8 bReplannedWithReuse_20_off : 1;
	uint8 bReplannedWithReuse_20_on : 1;
	uint8 bReplannedWithReuse_30_off : 1;
	uint8 bReplannedWithReuse_30_on : 1;

	uint8 bFinishedExperiment : 1;

	/** The Component that will be used for the planning process */
	UPROPERTY()
	UHTNPlannerComponent* HTNComp;

	UPROPERTY(transient)
	UHTNPlanner* Planner;

	/** Pointer to the current world state, used so that execute plans on it and re-plan when after partial execution it fails */
	TSharedPtr<struct FHTNWorldState> CurrentWorldState;

	TSharedPtr<struct FHTNPlan> PartiallyExecutedPlan;
};