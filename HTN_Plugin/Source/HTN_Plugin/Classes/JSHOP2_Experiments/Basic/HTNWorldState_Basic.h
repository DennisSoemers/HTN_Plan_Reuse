#pragma once

#include "HTNPlanner/HTNWorldState.h"
#include "HTNWorldState_Basic.generated.h"

struct FHTNDummyObject;

/**
 * Describes a World State in the 'basic' example domain included in the JSHOP2 download package.
 * This is a very simple domain in which the planner can ''have'' certain objects, and can drop
 * and pick up objects.
 */
USTRUCT(BlueprintType)
struct HTN_PLUGIN_API FHTNWorldState_Basic : public FHTNWorldState
{
	GENERATED_BODY()

public:
	virtual ~FHTNWorldState_Basic() {}

	virtual TSharedPtr<FHTNWorldState> Copy() const override;
	virtual void Initialize(AActor* Owner, UBlackboardComponent* BlackboardComponent) override;

	void AddObject(const FHTNDummyObject* Object);
	void AddObjects(const TArray<const FHTNDummyObject*>& Objects);
	bool HasObject(const FHTNDummyObject* Object) const;
	void RemoveObject(const FHTNDummyObject* Object);

protected:
	/** Array of the objects that we ''have'' */
	TArray<const FHTNDummyObject*> Objects;
};