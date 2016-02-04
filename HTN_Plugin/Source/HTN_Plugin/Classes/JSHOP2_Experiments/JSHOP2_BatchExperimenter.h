#pragma once

#include "JSHOP2_BatchExperimenter.generated.h"

UCLASS(Abstract, HideDropdown, HideCategories = ("Actor Tick", "Rendering", "Replication", "Input"))
class HTN_PLUGIN_API AJSHOP2_BatchExperimenter : public AActor
{
	GENERATED_BODY()

public:
	AJSHOP2_BatchExperimenter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	/** Asset that should be used for each experiment */
	UPROPERTY(EditAnywhere, Category = "Batch Experimenter")
	TSubclassOf<class AJSHOP2_Experimenter> ExperimenterAsset;

	/** The number of experiments to do */
	UPROPERTY(EditAnywhere, Category = "Batch Experimenter")
	int32 NumberOfExperiments;

	int32 ExperimentProgress;

	UPROPERTY(transient)
	class AJSHOP2_Experimenter* Experimenter;

	UPROPERTY(transient)
	class UDataCollector* DataCollector;
};