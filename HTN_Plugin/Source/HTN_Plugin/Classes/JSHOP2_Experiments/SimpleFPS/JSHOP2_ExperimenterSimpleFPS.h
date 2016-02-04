#pragma once

#include "JSHOP2_Experiments/JSHOP2_Experimenter.h"
#include "JSHOP2_ExperimenterSimpleFPS.generated.h"

struct FHTNWorldState_SimpleFPS;

UCLASS(HideDropdown)
class HTN_PLUGIN_API AJSHOP2_ExperimenterSimpleFPS : public AJSHOP2_Experimenter
{
	GENERATED_BODY()

public:
	AJSHOP2_ExperimenterSimpleFPS(const FObjectInitializer& ObjectInitializer);

	virtual UHTNPlanner* GeneratePlanner() override;
	int32 GetNumAreas() const;
	void InitializeWorldState(FHTNWorldState_SimpleFPS* WorldState);

protected:
	/** Will cache the Initial World State's Const Data here so that objects required for printing the plans remain alive */
	TSharedPtr<const struct FHTNWorldState_SimpleFPS_ConstData> WorldStateConstData;

	/** The number of Areas in the SimpleFPS domain */
	UPROPERTY(EditAnywhere, Category = "Simple FPS", meta=(ClampMin=0))
	int32 NumberOfAreas;
	/** The number of extra Points of Interest (guns, knives, medikits and cover points) in the SimpleFPS domain */
	UPROPERTY(EditAnywhere, Category = "Simple FPS", meta = (ClampMin = 0))
	int32 NumberOfPointsOfInterest;
	/** The probability for every possible pair of unique areas that the two areas are connected through a Waypoint */
	UPROPERTY(EditAnywhere, Category = "Simple FPS", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ProbabilityConnected;
	/** The probability that the NPC starts a problem as injured */
	UPROPERTY(EditAnywhere, Category = "Simple FPS", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ProbabilityInjured;
	/** The probability that an area starts a problem in the ''lighted'' state */
	UPROPERTY(EditAnywhere, Category = "Simple FPS", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ProbabilityLighted;
	/** The probability that a gun starts a problem as ''loaded'' */
	UPROPERTY(EditAnywhere, Category = "Simple FPS", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ProbabilityLoaded;
	/** The probability that a gun has night vision */
	UPROPERTY(EditAnywhere, Category = "Simple FPS", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ProbabilityNightVision;
	/** The probability that a waypoint starts a problem as ''open'' */
	UPROPERTY(EditAnywhere, Category = "Simple FPS", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ProbabilityOpen;

	/** The probability that a Point of Interest is a gun */
	UPROPERTY(EditAnywhere, Category = "Simple FPS (Points of Interest)", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ProbabilityGun;
	/** The probability that a Point of Interest is a knife */
	UPROPERTY(EditAnywhere, Category = "Simple FPS (Points of Interest)", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ProbabilityKnife;
	/** The probability that a Point of Interest is a medikit */
	UPROPERTY(EditAnywhere, Category = "Simple FPS (Points of Interest)", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ProbabilityMedikit;

	/** If true, generated maps will not be completely random but more realistic */
	UPROPERTY(EditAnywhere, Category = "Simple FPS (Map Generation)")
	bool bGenerateRealisticMaps;

	/** If true, will print generated problems in JSHOP2 format to output log */
	UPROPERTY(EditAnywhere, Category = "JSHOP2")
	bool bPrintJSHOP2Format;

	/** Seed used for RNG in the initialization of world state */
	int32 InitializationSeed;
	/** True if the InitializationSeed has been set yet, false otherwise */
	bool bInitializationSeedSet;

	/** Will cache an initial world state here that we can use to initialize multiple equal world states for experiments */
	TSharedPtr<FHTNWorldState_SimpleFPS> InitialWorldState;
};