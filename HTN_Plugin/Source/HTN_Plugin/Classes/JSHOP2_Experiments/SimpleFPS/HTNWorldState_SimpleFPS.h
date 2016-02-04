#pragma once

#include "HTNPlanner/HTNWorldState.h"
#include "HTNWorldState_SimpleFPS.generated.h"

struct FSimpleFPSObject;

/** Data related to Ammo objects */
struct FSimpleFPSAmmoData
{
	/** The index of the area in which the Ammo is */
	int32 Area;

	FSimpleFPSAmmoData(int32 Area) : Area(Area) {}
};

/** Data related to Area objects */
struct FSimpleFPSAreaData
{
	/** True if we've already explored this area, false otherwise */
	uint8 bExplored : 1;
	/** True if the area is currently lit, false otherwise */
	uint8 bLighted : 1;

	FSimpleFPSAreaData(bool bLighted) : bExplored(false), bLighted(bLighted) {}
};

/** Data related to ControlBox objects */
struct FSimpleFPSControlBoxData
{
	/** The index of the area in which the Control Box is */
	int32 Area;

	FSimpleFPSControlBoxData(int32 Area) : Area(Area) {}
};

/** Data related to CoverPoint objects */
struct FSimpleFPSCoverPointData
{
	/** The index of the area in which the Cover Point is */
	int32 Area;

	FSimpleFPSCoverPointData(int32 Area) : Area(Area) {}
};

/** Data related to Gun objects */
struct FSimpleFPSGunData
{
	/** The index of the Ammo required to load this Gun. -1 if no ammo required */
	int32 Ammo;
	/** The index of the area in which the Gun is */
	int32 Area;
	/** True if the gun has night vision, false otherwise */
	uint8 bHasNightVision : 1;
	/** True if the gun is loaded, false otherwise*/
	uint8 bLoaded : 1;

	FSimpleFPSGunData(int32 Ammo, int32 Area, bool bHasNightVision, bool bLoaded) 
		: Ammo(Ammo), Area(Area), bHasNightVision(bHasNightVision), bLoaded(bLoaded) {}
};

/** Data related to Keycard objects */
struct FSimpleFPSKeycardData
{
	/** The index of the area in which the Keycard is */
	int32 Area;

	FSimpleFPSKeycardData(int32 Area) : Area(Area) {}
};

/** Data related to Knife objects */
struct FSimpleFPSKnifeData
{
	/** The index of the area in which the Knife is */
	int32 Area;

	FSimpleFPSKnifeData(int32 Area) : Area(Area) {}
};

/** Data related to Medikit objects */
struct FSimpleFPSMedikitData
{
	/** The index of the area in which the Medikit is */
	int32 Area;

	FSimpleFPSMedikitData(int32 Area) : Area(Area) {}
};

/** Data related to the player object */
struct FSimpleFPSPlayerData
{
	/** The index of the area in which the player is */
	int32 Area;
	/** True if the player is injured */
	bool bInjured;

	FSimpleFPSPlayerData(int32 Area) : Area(Area), bInjured(false) {}
};

/** Data related to Waypoint objects */
struct FSimpleFPSWaypointData	// TO DO the areas and keycard index are actually constant data
{
	/** The index of the first area in which the Waypoint is */
	int32 Area1;
	/** The index of the second area in which the Waypoint is */
	int32 Area2;
	/** The index of the keycard that can open this Waypoint. -1 if no Keycard required. */
	int32 KeycardIndex;
	/** True if the Waypoint is currently open, false otherwise */
	bool bOpen;

	FSimpleFPSWaypointData(int32 Area1, int32 Area2, int32 KeycardIndex, bool bOpen)
		: Area1(Area1), Area2(Area2), KeycardIndex(KeycardIndex), bOpen(bOpen) {}
};

/**
 * Struct that holds all the data of a SimpleFPS world state that remains constant throughout the planning
 * process. All copies of World States generated whilst planning will simply point to a single instance of
 * this class to share this constant data.
 */
USTRUCT()
struct FHTNWorldState_SimpleFPS_ConstData
{
	GENERATED_BODY()

public:
	TArray<TArray<int32>> DistanceMatrix;

	TArray<FSimpleFPSObject*> Ammo;
	TArray<FSimpleFPSObject*> Areas;
	TArray<FSimpleFPSObject*> ControlBoxes;
	TArray<FSimpleFPSObject*> CoverPoints;
	TArray<FSimpleFPSObject*> Guns;
	TArray<FSimpleFPSObject*> Keycards;
	TArray<FSimpleFPSObject*> Knives;
	TArray<FSimpleFPSObject*> Medikits;
	TArray<FSimpleFPSObject*> Players;
	TArray<FSimpleFPSObject*> Waypoints;

	/** Array containing all objects from the arrays above together, for memory management */
	TArray<TSharedPtr<FSimpleFPSObject>> ObjectPool;

	/** Array of waypoints of which the agent knows that they are locked */
	TArray<FSimpleFPSObject*> KnownLockedWaypoints;

	/** Weapon choice that is preferred by the agent (ranged, stealth or melee) */
	uint8 PreferredWeaponChoice;

	/** If true, we're in execution mode, meaning that we need to check for locked doors even if they're not known yet */
	bool bExecutionMode;
};

/**
 * Describes a World State in the 'SimpleFPS' domain.
 * See http://stavros.lostre.org/files/Vassos11SimpleFPS.pdf for a description of the domain.
 *
 * The original SimpleFPS domain is defined in PDDL. A similar domain has been described in JSHOP2
 * by Alexandre Menif (see http://www.lamsade.dauphine.fr/~cazenave/papers/MenifCGW2014.pdf for experiments),
 * and this version has been translated into this plugin's UE4-based framework here.
 */
USTRUCT(BlueprintType)
struct HTN_PLUGIN_API FHTNWorldState_SimpleFPS : public FHTNWorldState
{
	GENERATED_BODY()

public:
	virtual ~FHTNWorldState_SimpleFPS() {}

	virtual TSharedPtr<FHTNWorldState> Copy() const override;
	virtual void Initialize(AActor* Owner, UBlackboardComponent* BlackboardComponent) override;

	/** Copies all data from the given other world state */
	void CopyFrom(const FHTNWorldState_SimpleFPS* Other);

	// ------- These CreateXXX() methods should only be called during initialization, NOT during planning!
	/** Creates a new Ammo object */
	FSimpleFPSObject* CreateAmmo(int32 Area, FString Name);
	/** Creates a new Area object */
	FSimpleFPSObject* CreateArea(bool bLighted, FString Name);
	/** Creates a new Control Box object */
	FSimpleFPSObject* CreateControlBox(int32 Area, FString Name);
	/** Creates a new Cover Point object */
	FSimpleFPSObject* CreateCoverPoint(int32 Area, FString Name);
	/** Creates a new Gun object */
	FSimpleFPSObject* CreateGun(int32 Area, bool bLoaded, int32 AmmoIndex, bool bNightVision, FString Name);
	/** Creates a new Keycard object */
	FSimpleFPSObject* CreateKeycard(int32 Area, FString Name);
	/** Creates a new Knife object */
	FSimpleFPSObject* CreateKnife(int32 Area, FString Name);
	/** Creates a new Medikit object */
	FSimpleFPSObject* CreateMedikit(int32 Area, FString Name);
	/** Creates a new Player object */
	FSimpleFPSObject* CreatePlayer(int32 Area, FString Name);
	/** Creates a new Waypoint object */
	FSimpleFPSObject* CreateWaypoint(int32 Area1, int32 Area2, bool bOpen, int32 Keycard, FString Name);

	const TArray<FSimpleFPSObject*>& GetAmmo() const;
	const TArray<FSimpleFPSObject*>& GetAreas() const;
	const TArray<FSimpleFPSObject*>& GetControlBoxes() const;
	const TArray<FSimpleFPSObject*>& GetCoverPoints() const;
	const TArray<FSimpleFPSObject*>& GetGuns() const;
	const TArray<FSimpleFPSObject*>& GetKeycards() const;
	const TArray<FSimpleFPSObject*>& GetKnives() const;
	const TArray<FSimpleFPSObject*>& GetMedikits() const;
	const TArray<FSimpleFPSObject*>& GetPlayers() const;
	const TArray<FSimpleFPSObject*>& GetWaypoints() const;

	/** Returns the area of the given object if it's one that can have an area, or -1 otherwise */
	int32 GetArea(FSimpleFPSObject* Object) const;
	TSharedPtr<const FHTNWorldState_SimpleFPS_ConstData> GetConstData() const;
	int32 GetDistance(int32 Area1, int32 Area2) const;
	const TArray<int32>& GetDistanceRow(int32 Area) const;

	/**
	 * Looks for the Move To Area compound task with the highest distance between current NPC location
	 * and destination, and returns a heuristic cost based on that (assumes that any other Move To Area
	 * compound tasks in the network will fit on the same path and don't contribute to the cost, for the 
	 * sake of having an admissible heuristic).
	 */
	virtual float GetHeuristicCost(const TArray<TSharedPtr<struct FHTNTaskInstance>>& TaskInstances) const override;

	/** 
	 * Tests if the given object is in the given area.
	 * Waypoints will be considered as being in both of their connected areas simultaneously
	 */
	bool IsInArea(FSimpleFPSObject* Object, int32 Area) const;
	/**
	 * If in execution mode, returns true if the waypoint is locked.
	 * Otherwise, only returns true if the waypoint is locked and the agent also knows that it is.
	 */
	bool IsLocked(const FSimpleFPSObject* Waypoint) const;
	/** 
	 * Returns true if the two given areas can be reached from each other 
	 * (does not take into account if required keycards are reachable) 
	 */
	bool IsReachable(int32 Area1, int32 Area2) const;

	/** Sets the area of the given object if it is one that can have an area. */
	void SetArea(FSimpleFPSObject* Object, int32 NewArea);
	/** Sets the pointer to the Const Data*/
	void SetConstData(TSharedPtr<FHTNWorldState_SimpleFPS_ConstData> Data);
	/** Caches the given distance matrix */
	void SetDistanceMatrix(const TArray<TArray<int32>>& DistanceMatrix);
	/** Set whether or not we're in execution mode */
	void SetExecutionMode(bool bMode);
	/** Store in memory that the agent has discovered that the given waypoint is locked */
	void SetKnownLock(FSimpleFPSObject* Waypoint);
	/** Store in memory which weapon choice is preferred by the agent */
	void SetPreferredWeaponChoice(uint8 WeaponChoice);

	/** --- Data concerning our non-NPC objects --- */
	/** Non-constant data concerning Ammo */
	TArray<FSimpleFPSAmmoData> AmmoData;
	/** Non-constant data concerning Areas */
	TArray<FSimpleFPSAreaData> AreaData;
	/** Non-constant data concerning Control Boxes */
	TArray<FSimpleFPSControlBoxData> ControlBoxData;
	/** Non-constant data concerning Cover Points */
	TArray<FSimpleFPSCoverPointData> CoverPointData;
	/** Non-constant data concerning Guns */
	TArray<FSimpleFPSGunData> GunData;
	/** Non-constant data concerning Keycards */
	TArray<FSimpleFPSKeycardData> KeycardData;
	/** Non-constant data concerning Knives */
	TArray<FSimpleFPSKnifeData> KnifeData;
	/** Non-constant data concerning Medikits */
	TArray<FSimpleFPSMedikitData> MedikitData;
	/** Non-constant data concerning Players */
	TArray<FSimpleFPSPlayerData> PlayerData;
	/** Non-constant data concerning Waypoints */
	TArray<FSimpleFPSWaypointData> WaypointData;

	/** --- NPC-related properties --- */
	/** Our NPC's inventory (list of objects he is holding) */
	TArray<FSimpleFPSObject*> NPCInventory;
	/** Point of Interest in the area that is near the NPC. nullptr if the NPC is not close to any POI in the area */
	FSimpleFPSObject* NPCNearbyPOI;
	int32 NPCArea;
	uint8 bNPCAware : 1;
	uint8 bNPCCovered : 1;
	uint8 bNPCInjured : 1;

protected:
	/** Constant Data (constant during planning process, not necessarily during initialization!) */
	TSharedPtr<FHTNWorldState_SimpleFPS_ConstData> ConstData;
};