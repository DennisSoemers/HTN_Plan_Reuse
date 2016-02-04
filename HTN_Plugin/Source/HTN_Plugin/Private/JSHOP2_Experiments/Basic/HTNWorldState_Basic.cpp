#include "HTN_PluginPrivatePCH.h"
#include "JSHOP2_Experiments/HTNDummyObject.h"
#include "JSHOP2_Experiments/Basic/HTNWorldState_Basic.h"
#include "JSHOP2_Experiments/Basic/JSHOP2_ExperimenterBasic.h"

TSharedPtr<FHTNWorldState> FHTNWorldState_Basic::Copy() const
{
	TSharedPtr<FHTNWorldState_Basic> NewWorldState = TSharedPtr<FHTNWorldState_Basic>(new FHTNWorldState_Basic());
	NewWorldState->AddObjects(Objects);
	return NewWorldState;
}

void FHTNWorldState_Basic::AddObject(const FHTNDummyObject* Object)
{
	Objects.Add(Object);
}

void FHTNWorldState_Basic::AddObjects(const TArray<const FHTNDummyObject*>& NewObjects)
{
	Objects.Append(NewObjects);
}

bool FHTNWorldState_Basic::HasObject(const FHTNDummyObject* Object) const
{
	if(Object != nullptr)
	{
		return Objects.Contains(Object);
	}

	return false;
}

void FHTNWorldState_Basic::Initialize(AActor* Owner, UBlackboardComponent* BlackboardComponent)
{
	if(AJSHOP2_ExperimenterBasic* Experimenter = Cast<AJSHOP2_ExperimenterBasic>(Owner))
	{
		Experimenter->InitializeWorldState(this);
	}
}

void FHTNWorldState_Basic::RemoveObject(const FHTNDummyObject* Object)
{
	// this implementation makes the assumption that we'll never have more than one copy of a single object.
	// it also could theoretically be optimized by, for instance, using the object ID as index or hash for cases
	// where we expect to have very large numbers of objects.
	for(int32 Idx = 0; Idx < Objects.Num(); ++Idx)
	{
		if(Objects[Idx] == Object)
		{
			Objects.RemoveAtSwap(Idx);
			return;
		}
	}
}