#pragma once

#include "JSHOP2_Experiments/JSHOP2_Experimenter.h"
#include "JSHOP2_ExperimenterBasic.generated.h"

struct FHTNDummyObject;
struct FHTNWorldState_Basic;

UCLASS(HideDropdown)
class HTN_PLUGIN_API AJSHOP2_ExperimenterBasic : public AJSHOP2_Experimenter
{
	GENERATED_BODY()

public:
	virtual UHTNPlanner* GeneratePlanner() override;
	void InitializeWorldState(FHTNWorldState_Basic* WorldState);

protected:
	/** Will store all the objects that we have in our 'basic' problem here */
	TArray<TSharedPtr<FHTNDummyObject>> ObjectPool;
};