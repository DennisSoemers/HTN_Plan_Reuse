#pragma once

#include "HTNDummyObject.generated.h"

/**
 * A dummy object used in a number of the JSHOP2 Experiments to represent objects that have a name.
 * In a ''real'' planning problem, other classes (typically AActors or UObjects) should be used instead.
 */
USTRUCT()
struct HTN_PLUGIN_API FHTNDummyObject
{
	GENERATED_BODY()

public:
	virtual ~FHTNDummyObject() {}

	FString ObjectName;
};