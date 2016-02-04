#include "HTN_PluginPrivatePCH.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/JSHOP2_ExperimenterSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_MoveToAreaSimpleFPS.h"

TSharedPtr<FHTNWorldState> FHTNWorldState_SimpleFPS::Copy() const
{
	FHTNWorldState_SimpleFPS* NewWorldState = new FHTNWorldState_SimpleFPS();
	
	// everything in the data arrays is plain old data structs, so we can safely just copy memory
	NewWorldState->AmmoData.AddUninitialized(AmmoData.Num());
	FMemory::Memcpy(NewWorldState->AmmoData.GetData(), AmmoData.GetData(), AmmoData.Num() * sizeof(FSimpleFPSAmmoData));

	NewWorldState->AreaData.AddUninitialized(AreaData.Num());
	FMemory::Memcpy(NewWorldState->AreaData.GetData(), AreaData.GetData(), AreaData.Num() * sizeof(FSimpleFPSAreaData));

	NewWorldState->ControlBoxData.AddUninitialized(ControlBoxData.Num());
	FMemory::Memcpy(NewWorldState->ControlBoxData.GetData(), ControlBoxData.GetData(), ControlBoxData.Num() * sizeof(FSimpleFPSControlBoxData));

	NewWorldState->CoverPointData.AddUninitialized(CoverPointData.Num());
	FMemory::Memcpy(NewWorldState->CoverPointData.GetData(), CoverPointData.GetData(), CoverPointData.Num() * sizeof(FSimpleFPSCoverPointData));

	NewWorldState->GunData.AddUninitialized(GunData.Num());
	FMemory::Memcpy(NewWorldState->GunData.GetData(), GunData.GetData(), GunData.Num() * sizeof(FSimpleFPSGunData));

	NewWorldState->KeycardData.AddUninitialized(KeycardData.Num());
	FMemory::Memcpy(NewWorldState->KeycardData.GetData(), KeycardData.GetData(), KeycardData.Num() * sizeof(FSimpleFPSKeycardData));

	NewWorldState->KnifeData.AddUninitialized(KnifeData.Num());
	FMemory::Memcpy(NewWorldState->KnifeData.GetData(), KnifeData.GetData(), KnifeData.Num() * sizeof(FSimpleFPSKnifeData));

	NewWorldState->MedikitData.AddUninitialized(MedikitData.Num());
	FMemory::Memcpy(NewWorldState->MedikitData.GetData(), MedikitData.GetData(), MedikitData.Num() * sizeof(FSimpleFPSMedikitData));

	NewWorldState->PlayerData.AddUninitialized(PlayerData.Num());
	FMemory::Memcpy(NewWorldState->PlayerData.GetData(), PlayerData.GetData(), PlayerData.Num() * sizeof(FSimpleFPSPlayerData));

	NewWorldState->WaypointData.AddUninitialized(WaypointData.Num());
	FMemory::Memcpy(NewWorldState->WaypointData.GetData(), WaypointData.GetData(), WaypointData.Num() * sizeof(FSimpleFPSWaypointData));

	// need to copy the NPC-related properties
	NewWorldState->NPCInventory.Append(NPCInventory);
	NewWorldState->NPCNearbyPOI = NPCNearbyPOI;
	NewWorldState->NPCArea = NPCArea;
	NewWorldState->bNPCAware = bNPCAware;
	NewWorldState->bNPCCovered = bNPCCovered;
	NewWorldState->bNPCInjured = bNPCInjured;

	// finally need to make sure we share the correct pointer to constant data
	NewWorldState->SetConstData(ConstData);

	return MakeShareable<FHTNWorldState>(NewWorldState);
}

void FHTNWorldState_SimpleFPS::CopyFrom(const FHTNWorldState_SimpleFPS* Other)
{
	// everything in the data arrays is plain old data structs, so we can safely just copy memory
	AmmoData.AddUninitialized(Other->AmmoData.Num());
	FMemory::Memcpy(AmmoData.GetData(), Other->AmmoData.GetData(), Other->AmmoData.Num() * sizeof(FSimpleFPSAmmoData));

	AreaData.AddUninitialized(Other->AreaData.Num());
	FMemory::Memcpy(AreaData.GetData(), Other->AreaData.GetData(), Other->AreaData.Num() * sizeof(FSimpleFPSAreaData));

	ControlBoxData.AddUninitialized(Other->ControlBoxData.Num());
	FMemory::Memcpy(ControlBoxData.GetData(), Other->ControlBoxData.GetData(), Other->ControlBoxData.Num() * sizeof(FSimpleFPSControlBoxData));

	CoverPointData.AddUninitialized(Other->CoverPointData.Num());
	FMemory::Memcpy(CoverPointData.GetData(), Other->CoverPointData.GetData(), Other->CoverPointData.Num() * sizeof(FSimpleFPSCoverPointData));

	GunData.AddUninitialized(Other->GunData.Num());
	FMemory::Memcpy(GunData.GetData(), Other->GunData.GetData(), Other->GunData.Num() * sizeof(FSimpleFPSGunData));

	KeycardData.AddUninitialized(Other->KeycardData.Num());
	FMemory::Memcpy(KeycardData.GetData(), Other->KeycardData.GetData(), Other->KeycardData.Num() * sizeof(FSimpleFPSKeycardData));

	KnifeData.AddUninitialized(Other->KnifeData.Num());
	FMemory::Memcpy(KnifeData.GetData(), Other->KnifeData.GetData(), Other->KnifeData.Num() * sizeof(FSimpleFPSKnifeData));

	MedikitData.AddUninitialized(Other->MedikitData.Num());
	FMemory::Memcpy(MedikitData.GetData(), Other->MedikitData.GetData(), Other->MedikitData.Num() * sizeof(FSimpleFPSMedikitData));

	PlayerData.AddUninitialized(Other->PlayerData.Num());
	FMemory::Memcpy(PlayerData.GetData(), Other->PlayerData.GetData(), Other->PlayerData.Num() * sizeof(FSimpleFPSPlayerData));

	WaypointData.AddUninitialized(Other->WaypointData.Num());
	FMemory::Memcpy(WaypointData.GetData(), Other->WaypointData.GetData(), Other->WaypointData.Num() * sizeof(FSimpleFPSWaypointData));

	// need to copy the NPC-related properties
	NPCInventory.Append(Other->NPCInventory);
	NPCNearbyPOI = Other->NPCNearbyPOI;
	NPCArea = Other->NPCArea;
	bNPCAware = Other->bNPCAware;
	bNPCCovered = Other->bNPCCovered;
	bNPCInjured = Other->bNPCInjured;

	// finally need to make sure we share the correct pointer to constant data
	SetConstData(Other->ConstData);
}

void FHTNWorldState_SimpleFPS::Initialize(AActor* Owner, UBlackboardComponent* BlackboardComponent)
{
	if(AJSHOP2_ExperimenterSimpleFPS* Experimenter = Cast<AJSHOP2_ExperimenterSimpleFPS>(Owner))
	{
		Experimenter->InitializeWorldState(this);
	}
}

FSimpleFPSObject* FHTNWorldState_SimpleFPS::CreateAmmo(int32 Area, FString Name)
{
	// create the object
	FSimpleFPSObject* Ammo = new FSimpleFPSObject();

	// set up properties
	Ammo->ObjectName = Name;
	Ammo->Index = ConstData->Ammo.Num();
	Ammo->ObjectType = ESimpleFPSObjectTypes::Ammo;

	// add the object
	ConstData->Ammo.Add(Ammo);
	ConstData->ObjectPool.Add(MakeShareable<FSimpleFPSObject>(Ammo));

	// create and add the non-constant data
	AmmoData.Add(FSimpleFPSAmmoData(Area));

	// return the object
	return Ammo;
}

FSimpleFPSObject* FHTNWorldState_SimpleFPS::CreateArea(bool bLighted, FString Name)
{
	// create the object
	FSimpleFPSObject* Area = new FSimpleFPSObject();

	// set up properties
	Area->ObjectName = Name;
	Area->Index = ConstData->Areas.Num();
	Area->ObjectType = ESimpleFPSObjectTypes::Area;

	// add the object
	ConstData->Areas.Add(Area);
	ConstData->ObjectPool.Add(MakeShareable<FSimpleFPSObject>(Area));

	// create and add the non-constant data
	AreaData.Add(FSimpleFPSAreaData(bLighted));

	// return the object
	return Area;
}

FSimpleFPSObject* FHTNWorldState_SimpleFPS::CreateControlBox(int32 Area, FString Name)
{
	// create the object
	FSimpleFPSObject* ControlBox = new FSimpleFPSObject();

	// set up properties
	ControlBox->ObjectName = Name;
	ControlBox->Index = ConstData->ControlBoxes.Num();
	ControlBox->ObjectType = ESimpleFPSObjectTypes::ControlBox;

	// add the object
	ConstData->ControlBoxes.Add(ControlBox);
	ConstData->ObjectPool.Add(MakeShareable<FSimpleFPSObject>(ControlBox));

	// create and add the non-constant data
	ControlBoxData.Add(FSimpleFPSControlBoxData(Area));

	// return the object
	return ControlBox;
}

FSimpleFPSObject* FHTNWorldState_SimpleFPS::CreateCoverPoint(int32 Area, FString Name)
{
	// create the object
	FSimpleFPSObject* CoverPoint = new FSimpleFPSObject();

	// set up properties
	CoverPoint->ObjectName = Name;
	CoverPoint->Index = ConstData->CoverPoints.Num();
	CoverPoint->ObjectType = ESimpleFPSObjectTypes::CoverPoint;

	// add the object
	ConstData->CoverPoints.Add(CoverPoint);
	ConstData->ObjectPool.Add(MakeShareable<FSimpleFPSObject>(CoverPoint));

	// create and add the non-constant data
	CoverPointData.Add(FSimpleFPSCoverPointData(Area));

	// return the object
	return CoverPoint;
}

FSimpleFPSObject* FHTNWorldState_SimpleFPS::CreateGun(int32 Area, bool bLoaded, int32 AmmoIndex, bool bNightVision, FString Name)
{
	// create the object
	FSimpleFPSObject* Gun = new FSimpleFPSObject();

	// set up properties
	Gun->ObjectName = Name;
	Gun->Index = ConstData->Guns.Num();
	Gun->ObjectType = ESimpleFPSObjectTypes::Gun;

	// add the object
	ConstData->Guns.Add(Gun);
	ConstData->ObjectPool.Add(MakeShareable<FSimpleFPSObject>(Gun));

	// create and add the non-constant data
	GunData.Add(FSimpleFPSGunData(AmmoIndex, Area, bNightVision, bLoaded));

	// return the object
	return Gun;
}

FSimpleFPSObject* FHTNWorldState_SimpleFPS::CreateKeycard(int32 Area, FString Name)
{
	// create the object
	FSimpleFPSObject* Keycard = new FSimpleFPSObject();

	// set up properties
	Keycard->ObjectName = Name;
	Keycard->Index = ConstData->Keycards.Num();
	Keycard->ObjectType = ESimpleFPSObjectTypes::Keycard;

	// add the object
	ConstData->Keycards.Add(Keycard);
	ConstData->ObjectPool.Add(MakeShareable<FSimpleFPSObject>(Keycard));

	// create and add the non-constant data
	KeycardData.Add(FSimpleFPSKeycardData(Area));

	// return the object
	return Keycard;
}

FSimpleFPSObject* FHTNWorldState_SimpleFPS::CreateKnife(int32 Area, FString Name)
{
	// create the object
	FSimpleFPSObject* Knife = new FSimpleFPSObject();

	// set up properties
	Knife->ObjectName = Name;
	Knife->Index = ConstData->Knives.Num();
	Knife->ObjectType = ESimpleFPSObjectTypes::Knife;

	// add the object
	ConstData->Knives.Add(Knife);
	ConstData->ObjectPool.Add(MakeShareable<FSimpleFPSObject>(Knife));

	// create and add the non-constant data
	KnifeData.Add(FSimpleFPSKnifeData(Area));

	// return the object
	return Knife;
}

FSimpleFPSObject* FHTNWorldState_SimpleFPS::CreateMedikit(int32 Area, FString Name)
{
	// create the object
	FSimpleFPSObject* Medikit = new FSimpleFPSObject();

	// set up properties
	Medikit->ObjectName = Name;
	Medikit->Index = ConstData->Medikits.Num();
	Medikit->ObjectType = ESimpleFPSObjectTypes::Medikit;

	// add the object
	ConstData->Medikits.Add(Medikit);
	ConstData->ObjectPool.Add(MakeShareable<FSimpleFPSObject>(Medikit));

	// create and add the non-constant data
	MedikitData.Add(FSimpleFPSMedikitData(Area));

	// return the object
	return Medikit;
}

FSimpleFPSObject* FHTNWorldState_SimpleFPS::CreatePlayer(int32 Area, FString Name)
{
	// create the object
	FSimpleFPSObject* Player = new FSimpleFPSObject();

	// set up properties
	Player->ObjectName = Name;
	Player->Index = ConstData->Players.Num();
	Player->ObjectType = ESimpleFPSObjectTypes::Player;

	// add the object
	ConstData->Players.Add(Player);
	ConstData->ObjectPool.Add(MakeShareable<FSimpleFPSObject>(Player));

	// create and add the non-constant data
	PlayerData.Add(FSimpleFPSPlayerData(Area));

	// return the object
	return Player;
}

FSimpleFPSObject* FHTNWorldState_SimpleFPS::CreateWaypoint(int32 Area1, int32 Area2, bool bOpen, int32 Keycard, FString Name)
{
	// create the object
	FSimpleFPSObject* Waypoint = new FSimpleFPSObject();

	// set up properties
	Waypoint->ObjectName = Name;
	Waypoint->Index = ConstData->Waypoints.Num();
	Waypoint->ObjectType = ESimpleFPSObjectTypes::Waypoint;

	// add the object
	ConstData->Waypoints.Add(Waypoint);
	ConstData->ObjectPool.Add(MakeShareable<FSimpleFPSObject>(Waypoint));

	// create and add the non-constant data
	WaypointData.Add(FSimpleFPSWaypointData(Area1, Area2, Keycard, bOpen));

	// return the object
	return Waypoint;
}

float FHTNWorldState_SimpleFPS::GetHeuristicCost(const TArray<TSharedPtr<FHTNTaskInstance>>& TaskInstances) const
{
	int32 MaxMoveDistance = 0;

	for(const TSharedPtr<FHTNTaskInstance>& TaskInstance : TaskInstances)
	{
		UHTNTask* Task = TaskInstance->Task;
		if(UHTNTask_MoveToAreaSimpleFPS* MoveToAreaTask = Cast<UHTNTask_MoveToAreaSimpleFPS>(Task))
		{
			FHTNTask_MoveToAreaSimpleFPSMemory* MoveToAreaMemory = (FHTNTask_MoveToAreaSimpleFPSMemory*)TaskInstance->GetMemory();
			int32 Distance = GetDistance(NPCArea, MoveToAreaMemory->Area);

			if(Distance > MaxMoveDistance)
			{
				MaxMoveDistance = Distance;
			}
		}
	}

	// for every point of distance, we'll have at least a move-to-point task to move to the waypoint
	// connecting 2 areas, and then a moving task to use the waypoint to move between the 2 areas.
	// each of those tasks always has a cost of 1.f
	//
	// the move-to-area task itself has a heuristic cost of 0.f to cover cases where the NPC already is
	// in the correct area, so we won't be counting any heuristics double here
	return 2.f * MaxMoveDistance;
}

const TArray<FSimpleFPSObject*>& FHTNWorldState_SimpleFPS::GetAmmo() const
{
	return ConstData->Ammo;
}

const TArray<FSimpleFPSObject*>& FHTNWorldState_SimpleFPS::GetAreas() const
{
	return ConstData->Areas;
}

const TArray<FSimpleFPSObject*>& FHTNWorldState_SimpleFPS::GetControlBoxes() const
{
	return ConstData->ControlBoxes;
}

const TArray<FSimpleFPSObject*>& FHTNWorldState_SimpleFPS::GetCoverPoints() const
{
	return ConstData->CoverPoints;
}

const TArray<FSimpleFPSObject*>& FHTNWorldState_SimpleFPS::GetGuns() const
{
	return ConstData->Guns;
}

const TArray<FSimpleFPSObject*>& FHTNWorldState_SimpleFPS::GetKeycards() const
{
	return ConstData->Keycards;
}

const TArray<FSimpleFPSObject*>& FHTNWorldState_SimpleFPS::GetKnives() const
{
	return ConstData->Knives;
}

const TArray<FSimpleFPSObject*>& FHTNWorldState_SimpleFPS::GetMedikits() const
{
	return ConstData->Medikits;
}

const TArray<FSimpleFPSObject*>& FHTNWorldState_SimpleFPS::GetPlayers() const
{
	return ConstData->Players;
}

const TArray<FSimpleFPSObject*>& FHTNWorldState_SimpleFPS::GetWaypoints() const
{
	return ConstData->Waypoints;
}

int32 FHTNWorldState_SimpleFPS::GetArea(FSimpleFPSObject* Object) const
{
	if(Object->ObjectType == ESimpleFPSObjectTypes::Ammo)
	{
		return AmmoData[Object->Index].Area;
	}
	else if(Object->ObjectType == ESimpleFPSObjectTypes::ControlBox)
	{
		return ControlBoxData[Object->Index].Area;
	}
	else if(Object->ObjectType == ESimpleFPSObjectTypes::CoverPoint)
	{
		return CoverPointData[Object->Index].Area;
	}
	else if(Object->ObjectType == ESimpleFPSObjectTypes::Gun)
	{
		return GunData[Object->Index].Area;
	}
	else if(Object->ObjectType == ESimpleFPSObjectTypes::Keycard)
	{
		return KeycardData[Object->Index].Area;
	}
	else if(Object->ObjectType == ESimpleFPSObjectTypes::Knife)
	{
		return KnifeData[Object->Index].Area;
	}
	else if(Object->ObjectType == ESimpleFPSObjectTypes::Medikit)
	{
		return MedikitData[Object->Index].Area;
	}
	else if(Object->ObjectType == ESimpleFPSObjectTypes::Player)
	{
		return PlayerData[Object->Index].Area;
	}
	else
	{
		return -1;
	}
}

TSharedPtr<const FHTNWorldState_SimpleFPS_ConstData> FHTNWorldState_SimpleFPS::GetConstData() const
{
	return ConstData;
}

int32 FHTNWorldState_SimpleFPS::GetDistance(int32 Area1, int32 Area2) const
{
	return ConstData->DistanceMatrix[Area1][Area2];
}

const TArray<int32>& FHTNWorldState_SimpleFPS::GetDistanceRow(int32 Area) const
{
	return ConstData->DistanceMatrix[Area];
}

bool FHTNWorldState_SimpleFPS::IsInArea(FSimpleFPSObject* Object, int32 Area) const
{
	if(Object->ObjectType == ESimpleFPSObjectTypes::Waypoint)
	{
		return (WaypointData[Object->Index].Area1 == Area ||
				WaypointData[Object->Index].Area2 == Area	);
	}
	else
	{
		return (Area == GetArea(Object));
	}
}

bool FHTNWorldState_SimpleFPS::IsLocked(const FSimpleFPSObject* Waypoint) const
{
	if(ConstData->bExecutionMode)
	{
		return !WaypointData[Waypoint->Index].bOpen;
	}
	else
	{
		return (!WaypointData[Waypoint->Index].bOpen && ConstData->KnownLockedWaypoints.Contains(Waypoint));
	}
}

bool FHTNWorldState_SimpleFPS::IsReachable(int32 Area1, int32 Area2) const
{
	return ConstData->DistanceMatrix[Area1][Area2] < 10000;	// TO DO should probably use same variable here as in problem generator, no magic number
}

void FHTNWorldState_SimpleFPS::SetArea(FSimpleFPSObject* Object, int32 NewArea)
{
	if(Object->ObjectType == ESimpleFPSObjectTypes::Ammo)
	{
		AmmoData[Object->Index].Area = NewArea;
	}
	else if(Object->ObjectType == ESimpleFPSObjectTypes::ControlBox)
	{
		ControlBoxData[Object->Index].Area = NewArea;
	}
	else if(Object->ObjectType == ESimpleFPSObjectTypes::CoverPoint)
	{
		CoverPointData[Object->Index].Area = NewArea;
	}
	else if(Object->ObjectType == ESimpleFPSObjectTypes::Gun)
	{
		GunData[Object->Index].Area = NewArea;
	}
	else if(Object->ObjectType == ESimpleFPSObjectTypes::Keycard)
	{
		KeycardData[Object->Index].Area = NewArea;
	}
	else if(Object->ObjectType == ESimpleFPSObjectTypes::Knife)
	{
		KnifeData[Object->Index].Area = NewArea;
	}
	else if(Object->ObjectType == ESimpleFPSObjectTypes::Medikit)
	{
		MedikitData[Object->Index].Area = NewArea;
	}
	else if(Object->ObjectType == ESimpleFPSObjectTypes::Player)
	{
		PlayerData[Object->Index].Area = NewArea;
	}
}

void FHTNWorldState_SimpleFPS::SetConstData(TSharedPtr<FHTNWorldState_SimpleFPS_ConstData> Data)
{
	ConstData = Data;
}

void FHTNWorldState_SimpleFPS::SetDistanceMatrix(const TArray<TArray<int32>>& DistanceMatrix)
{
	ConstData->DistanceMatrix = TArray<TArray<int32>>();
	ConstData->DistanceMatrix.Reserve(DistanceMatrix.Num());
	for(int32 Row = 0; Row < DistanceMatrix.Num(); ++Row)
	{
		ConstData->DistanceMatrix.Add(TArray<int32>());
		TArray<int32>& DistanceRow = ConstData->DistanceMatrix[Row];
		DistanceRow.Reserve(DistanceMatrix.Num());

		for(int32 Col = 0; Col < DistanceMatrix.Num(); ++Col)
		{
			DistanceRow.Add(DistanceMatrix[Row][Col]);
		}
	}
}

void FHTNWorldState_SimpleFPS::SetExecutionMode(bool bMode)
{
	ConstData->bExecutionMode = bMode;
}

void FHTNWorldState_SimpleFPS::SetKnownLock(FSimpleFPSObject* Waypoint)
{
	ConstData->KnownLockedWaypoints.Add(Waypoint);
}

void FHTNWorldState_SimpleFPS::SetPreferredWeaponChoice(uint8 WeaponChoice)
{
	ConstData->PreferredWeaponChoice = WeaponChoice;
}