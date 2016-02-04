#pragma once

#include "JSHOP2_Experiments/HTNDummyObject.h"
#include "SimpleFPSObject.generated.h"

/**
 * The different types of Objects we have in the SimpleFPS domain
 */
enum class ESimpleFPSObjectTypes : uint8
{
	Ammo,
	Area,
	ControlBox,
	CoverPoint,
	Gun,
	Keycard,
	Knife,
	Medikit,
	Player,
	Waypoint,
};

/**
 * An Object in the Simple FPS domain. Inherits the ObjectName property from 
 * FHTNDummyObject, but also has a type and an index.
 */
USTRUCT()
struct HTN_PLUGIN_API FSimpleFPSObject : public FHTNDummyObject
{
	GENERATED_BODY()

public:
	virtual ~FSimpleFPSObject() {}

	int32 Index;
	ESimpleFPSObjectTypes ObjectType;
};