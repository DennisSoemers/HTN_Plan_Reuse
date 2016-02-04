#include "HTN_PluginPrivatePCH.h"
#include "JSHOP2_Experiments/DataCollector.h"
#include "JSHOP2_Experiments/JSHOP2_Experimenter.h"
#include "JSHOP2_Experiments/JSHOP2_BatchExperimenter.h"

AJSHOP2_BatchExperimenter::AJSHOP2_BatchExperimenter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	NumberOfExperiments = 1;
	ExperimentProgress = 0;
}

void AJSHOP2_BatchExperimenter::BeginPlay()
{
	if(!DataCollector)
	{
		DataCollector = NewObject<UDataCollector>();
	}
}

void AJSHOP2_BatchExperimenter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if(DataCollector)
	{
		DataCollector->ExportResults();
	}
}

void AJSHOP2_BatchExperimenter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(Experimenter != nullptr && Experimenter->FinishedExperiment())
	{
		Experimenter = nullptr;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, *FString::Printf(TEXT("Finished %d / %d planning problems."), 
			ExperimentProgress, NumberOfExperiments));
	}

	if(Experimenter == nullptr)
	{
		if(ExperimentProgress < NumberOfExperiments)
		{
			++ExperimentProgress;
			Experimenter = Cast<AJSHOP2_Experimenter>(GetWorld()->SpawnActor(ExperimenterAsset));

			if(!DataCollector)
			{
				DataCollector = NewObject<UDataCollector>();
			}

			Experimenter->SetDataCollector(DataCollector);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 3600.f, FColor::Yellow, TEXT("Finished batch experiment!"));
			SetActorTickEnabled(false);
			DataCollector->ExportResults();
			DataCollector = nullptr;
		}
	}
}