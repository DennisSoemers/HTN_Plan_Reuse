#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/HTNPlanner.h"
#include "HTNPlanner/HTNPlannerComponent.h"
#include "HTNPlanner/TaskNetwork.h"
#include "JSHOP2_Experiments/DataCollector.h"
#include "JSHOP2_Experiments/SimpleFPS/HTNWorldState_SimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/SimpleFPSObject.h"
#include "JSHOP2_Experiments/SimpleFPS/JSHOP2_ExperimenterSimpleFPS.h"
#include "JSHOP2_Experiments/SimpleFPS/Tasks/HTNTask_BehaveSimpleFPS.h"

AJSHOP2_ExperimenterSimpleFPS::AJSHOP2_ExperimenterSimpleFPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NumberOfAreas = 10;
	NumberOfPointsOfInterest = 100;
	ProbabilityConnected = 0.6f;
	ProbabilityInjured = 0.7f;
	ProbabilityLighted = 0.1f;
	ProbabilityLoaded = 0.4f;
	ProbabilityNightVision = 0.2f;
	ProbabilityOpen = 0.6f;

	ProbabilityGun = 0.25f;
	ProbabilityKnife = 0.25f;
	ProbabilityMedikit = 0.25f;

	bGenerateRealisticMaps = true;

	bPrintJSHOP2Format = false;

	bInitializationSeedSet = false;

	InitialWorldState = nullptr;
}

UHTNPlanner* AJSHOP2_ExperimenterSimpleFPS::GeneratePlanner()
{
	UHTNPlanner* Planner = NewObject<UHTNPlanner>(this);
	Planner->bIgnoreTaskCosts = bIgnoreCosts;
	Planner->bDepthFirstSearch = bDepthFirstSearch;
	Planner->bExecutePlan = false;
	Planner->bLoop = bLoop;
	Planner->MaxSearchTime = MaxSearchTime;
	Planner->EmptyWorldState = TSharedPtr<FHTNWorldState>(new FHTNWorldState_SimpleFPS());

	// create our Task Network
	TSharedPtr<FHTNTaskInstance> TaskNetwork = HTNComp->InstantiateNetwork(UTaskNetwork::StaticClass());
	// create our behave task
	TSharedPtr<FHTNTaskInstance> BehaveTask = HTNComp->InstantiateTask(UHTNTask_BehaveSimpleFPS::StaticClass());
	(Cast<UTaskNetwork>(TaskNetwork->Task))->AddTaskOrSubNetwork(BehaveTask, TaskNetwork->GetMemory());
	Planner->TaskNetworkInstance = TaskNetwork;

	return Planner;
}

int32 AJSHOP2_ExperimenterSimpleFPS::GetNumAreas() const
{
	return NumberOfAreas;
}

void AJSHOP2_ExperimenterSimpleFPS::InitializeWorldState(FHTNWorldState_SimpleFPS* WorldState)
{
	if(!bInitializationSeedSet)
	{
		//InitializationSeed = 23468;

		//InitializationSeed = 8072;

		//InitializationSeed = 11780;

		//InitializationSeed = 8417;

		//InitializationSeed = 26149;

		//InitializationSeed = 23666;

		//InitializationSeed = 25594;

		//InitializationSeed = 10937;

		//InitializationSeed = 26149;

		//InitializationSeed = 27207;

		InitializationSeed = FMath::Rand();
		bInitializationSeedSet = true;
		HTNComp->DataCollector->SetWorldStateSeed(InitializationSeed);
	}

	FRandomStream RandStream(InitializationSeed);

	if(!InitialWorldState.IsValid())
	{
		UE_LOG(LogHTNPlanner, Warning, TEXT("Generating problem with seed = %d"), InitializationSeed);

		if(bGenerateRealisticMaps)
		{
			// for realistic maps, we require (5 + 3x) rooms, where x is a nonnegative integer
			if(NumberOfAreas < 5)
			{
				NumberOfAreas = 5;
			}
			else
			{
				int32 Remainder = NumberOfAreas - 5;
				NumberOfAreas += ((3 - (Remainder % 3)) % 3);
			}
		}

		// initialize our world state
		InitialWorldState = MakeShareable<FHTNWorldState_SimpleFPS>(new FHTNWorldState_SimpleFPS());
		InitialWorldState->SetConstData(MakeShareable<FHTNWorldState_SimpleFPS_ConstData>(new FHTNWorldState_SimpleFPS_ConstData()));
		InitialWorldState->SetPreferredWeaponChoice(0);

		// initialize NPC-related properties
		InitialWorldState->NPCNearbyPOI = nullptr;
		InitialWorldState->NPCArea = bGenerateRealisticMaps ? 0 : RandStream.RandRange(0, NumberOfAreas - 1);
		InitialWorldState->bNPCAware = false;
		InitialWorldState->bNPCCovered = false;
		InitialWorldState->bNPCInjured = (RandStream.FRandRange(0.f, 1.f) <= ProbabilityInjured);

		// initialize player
		int32 PlayerArea = bGenerateRealisticMaps ? NumberOfAreas - 1 : RandStream.RandRange(0, NumberOfAreas - 1);
		InitialWorldState->CreatePlayer(PlayerArea, "p");

		// initialize areas, control boxes, waypoints and keycards
		const int32 LARGE_DISTANCE = 10000;
		TArray<TArray<int32>> DistanceMatrix;
		DistanceMatrix.Reserve(NumberOfAreas);
		for(int32 Row = 0; Row < NumberOfAreas; ++Row)
		{
			DistanceMatrix.Add(TArray<int32>());
			TArray<int32>& DistanceRow = DistanceMatrix[Row];
			DistanceRow.Reserve(NumberOfAreas);

			for(int32 Col = 0; Col < NumberOfAreas; ++Col)
			{
				DistanceRow.Add(LARGE_DISTANCE);
			}
		}

		if(bGenerateRealisticMaps)
		{
			int32 NumRoomsCreated = 0;

			// start with first room
			InitialWorldState->CreateArea((RandStream.FRandRange(0.f, 1.f) <= ProbabilityLighted), 
										  FString::Printf(TEXT("area%d"), 0));
			InitialWorldState->CreateControlBox(0, FString::Printf(TEXT("control-box%d"), 0));
			DistanceMatrix[0][0] = 0;
			++NumRoomsCreated;

			// create 4 rooms around the first
			for(int32 Idx = 1; Idx < 5; ++Idx)
			{
				InitialWorldState->CreateArea((RandStream.FRandRange(0.f, 1.f) <= ProbabilityLighted),
											  FString::Printf(TEXT("area%d"), Idx));
				InitialWorldState->CreateControlBox(Idx, FString::Printf(TEXT("control-box%d"), Idx));
				DistanceMatrix[Idx][Idx] = 0;

				// connect new room to the first room
				DistanceMatrix[0][Idx] = 1;
				DistanceMatrix[Idx][0] = 1;
				if(RandStream.FRandRange(0.f, 1.f) <= ProbabilityOpen)
				{
					// door starts open
					InitialWorldState->CreateWaypoint(0, Idx, true, -1, FString::Printf(TEXT("door%d-%d"), 0, Idx));
				}
				else
				{
					// door starts closed, so also need a keycard (spawn keycard in one of the previously generated rooms)
					FSimpleFPSObject* Keycard = InitialWorldState->CreateKeycard(RandStream.RandRange(0, NumRoomsCreated - 1),
																				 FString::Printf(TEXT("keycard%d-%d"), 0, Idx));
					InitialWorldState->CreateWaypoint(0, Idx, false, Keycard->Index, FString::Printf(TEXT("door%d-%d"), 0, Idx));
				}

				++NumRoomsCreated;
			}

			// the outer 4 rooms will be the first rooms in our fringe
			TArray<int32> Fringe;
			Fringe.Add(1);
			Fringe.Add(2);
			Fringe.Add(3);
			Fringe.Add(4);

			// generate new rooms in groups of 3 by always picking one room out of the fringe and connecting it to 3 new rooms
			while(NumRoomsCreated < NumberOfAreas)
			{
				int32 FringeIdx = RandStream.RandRange(0, Fringe.Num() - 1);
				int32 ExistingArea = Fringe[FringeIdx];
				Fringe.RemoveAtSwap(FringeIdx);

				for(int32 Idx = 0; Idx < 3; ++Idx)
				{
					int32 AreaIdx = NumRoomsCreated + Idx;
					InitialWorldState->CreateArea((RandStream.FRandRange(0.f, 1.f) <= ProbabilityLighted),
												  FString::Printf(TEXT("area%d"), AreaIdx));
					InitialWorldState->CreateControlBox(AreaIdx, FString::Printf(TEXT("control-box%d"), AreaIdx));
					DistanceMatrix[AreaIdx][AreaIdx] = 0;

					// connect new room to the existing room
					DistanceMatrix[ExistingArea][AreaIdx] = 1;
					DistanceMatrix[AreaIdx][ExistingArea] = 1;
					if(RandStream.FRandRange(0.f, 1.f) <= ProbabilityOpen)
					{
						// door starts open
						InitialWorldState->CreateWaypoint(ExistingArea, AreaIdx, true, -1, 
														  FString::Printf(TEXT("door%d-%d"), ExistingArea, AreaIdx));
					}
					else
					{
						// door starts closed, so also need a keycard (spawn keycard in one of the previously generated rooms)
						FSimpleFPSObject* Keycard = InitialWorldState->CreateKeycard(RandStream.RandRange(0, NumRoomsCreated - 1),
																					 FString::Printf(TEXT("keycard%d-%d"), 
																					 ExistingArea, AreaIdx));
						InitialWorldState->CreateWaypoint(ExistingArea, AreaIdx, false, Keycard->Index, 
														  FString::Printf(TEXT("door%d-%d"), ExistingArea, AreaIdx));
					}

					// add newly generated room to the fringe
					Fringe.Add(AreaIdx);
				}

				NumRoomsCreated += 3;
			}

			// finally, also create some random connections elsewhere so we have a few cycles in the graph
			for(int32 Idx1 = 0; Idx1 < NumberOfAreas; ++Idx1)
			{
				for(int32 Idx2 = 0; Idx2 < Idx1; ++Idx2)
				{
					if(DistanceMatrix[Idx1][Idx2] <= 1)
					{
						continue;	// these 2 areas are already connected
					}

					if(RandStream.FRandRange(0.f, 1.f) <= ProbabilityConnected)
					{
						// create an extra connection here
						DistanceMatrix[Idx1][Idx2] = 1;
						DistanceMatrix[Idx2][Idx1] = 1;

						if(RandStream.FRandRange(0.f, 1.f) <= ProbabilityOpen)
						{
							// door starts open
							InitialWorldState->CreateWaypoint(Idx1, Idx2, true, -1, FString::Printf(TEXT("door%d-%d"), Idx1, Idx2));
						}
						else
						{
							// door starts closed, so also need a keycard
							FSimpleFPSObject* Keycard = InitialWorldState->CreateKeycard(RandStream.RandRange(0, NumberOfAreas - 1),
																						 FString::Printf(TEXT("keycard%d-%d"),
																						 Idx1, Idx2));
							InitialWorldState->CreateWaypoint(Idx1, Idx2, false, Keycard->Index,
															  FString::Printf(TEXT("door%d-%d"), Idx1, Idx2));
						}
					}
				}
			}
		}
		else
		{
			// generate map completely randomly
			for(int32 Idx1 = 0; Idx1 < NumberOfAreas; ++Idx1)
			{
				InitialWorldState->CreateArea((RandStream.FRandRange(0.f, 1.f) <= ProbabilityLighted), 
											  FString::Printf(TEXT("area%d"), Idx1));
				InitialWorldState->CreateControlBox(Idx1, FString::Printf(TEXT("control-box%d"), Idx1));
				DistanceMatrix[Idx1][Idx1] = 0;

				for(int32 Idx2 = 0; Idx2 < Idx1; ++Idx2)
				{
					if(RandStream.FRandRange(0.f, 1.f) <= ProbabilityConnected)
					{
						// connect this pair of areas by creating a Waypoint
						DistanceMatrix[Idx1][Idx2] = 1;
						DistanceMatrix[Idx2][Idx1] = 1;

						if(RandStream.FRandRange(0.f, 1.f) <= ProbabilityOpen)
						{
							// door starts open
							InitialWorldState->CreateWaypoint(Idx1, Idx2, true, -1, FString::Printf(TEXT("door%d-%d"), Idx1, Idx2));
						}
						else
						{
							// door starts closed, so also need a keycard
							FSimpleFPSObject* Keycard = InitialWorldState->CreateKeycard(RandStream.RandRange(0, NumberOfAreas - 1),
																						 FString::Printf(TEXT("keycard%d-%d"), 
																						 Idx1, Idx2));
							InitialWorldState->CreateWaypoint(Idx1, Idx2, false, Keycard->Index, 
															  FString::Printf(TEXT("door%d-%d"), Idx1, Idx2));
						}
					}
				}
			}
		}

		// finalize computation of distance matrix
		for(int32 Idx1 = 0; Idx1 < NumberOfAreas; ++Idx1)
		{
			for(int32 Idx2 = 0; Idx2 < NumberOfAreas; ++Idx2)
			{
				for(int32 Idx3 = 0; Idx3 < NumberOfAreas; ++Idx3)
				{
					DistanceMatrix[Idx2][Idx3] = FMath::Min(DistanceMatrix[Idx2][Idx3],
															DistanceMatrix[Idx2][Idx1] + DistanceMatrix[Idx1][Idx3]);
				}
			}
		}
		for(int32 Idx1 = 0; Idx1 < NumberOfAreas; ++Idx1)
		{
			for(int32 Idx2 = 0; Idx2 < NumberOfAreas; ++Idx2)
			{
				DistanceMatrix[Idx1][Idx2] = DistanceMatrix[Idx2][Idx1];
			}
		}

		// initialize guns (+ ammo), knives, medikits and cover points
		int32 LastAmmoIndex = 0;
		int32 LastGunIndex = 0;
		int32 LastKnifeIndex = 0;
		int32 LastMedikitIndex = 0;
		int32 LastCoverPointIndex = 0;

		for(int32 Idx = 0; Idx < NumberOfPointsOfInterest; ++Idx)
		{
			float RandomNumber = RandStream.FRandRange(0.f, 1.f);

			if(RandomNumber <= ProbabilityGun)
			{
				// we're adding a gun
				bool bLoaded = false;
				int32 AmmoIndex = -1;

				if(RandStream.FRandRange(0.f, 1.f) <= ProbabilityLoaded)
				{
					// gun is already loaded
					bLoaded = true;
				}
				else
				{
					// also need matching ammo
					FSimpleFPSObject* Ammo = InitialWorldState->CreateAmmo(RandStream.RandRange(0, NumberOfAreas - 1),
																		   FString::Printf(TEXT("ammogun%d"), LastAmmoIndex++));
					AmmoIndex = Ammo->Index;
				}

				InitialWorldState->CreateGun(RandStream.RandRange(0, NumberOfAreas - 1), bLoaded, AmmoIndex,
											 (RandStream.FRandRange(0.f, 1.f) <= ProbabilityNightVision), FString::Printf(TEXT("gun%d"), LastGunIndex++));
			}
			else if(RandomNumber <= ProbabilityGun + ProbabilityKnife)
			{
				// we're adding a knife
				InitialWorldState->CreateKnife(RandStream.RandRange(0, NumberOfAreas - 1), FString::Printf(TEXT("knife%d"), LastKnifeIndex++));
			}
			else if(RandomNumber <= ProbabilityGun + ProbabilityKnife + ProbabilityMedikit)
			{
				// we're adding a medikit
				InitialWorldState->CreateMedikit(RandStream.RandRange(0, NumberOfAreas - 1), FString::Printf(TEXT("firstaid%d"), LastMedikitIndex++));
			}
			else
			{
				// we're adding a cover point
				InitialWorldState->CreateCoverPoint(RandStream.RandRange(0, NumberOfAreas - 1), FString::Printf(TEXT("coverpoint%d"), LastCoverPointIndex++));
			}
		}

		InitialWorldState->SetDistanceMatrix(DistanceMatrix);
		InitialWorldState->SetExecutionMode(true);	// by default set execution mode to true, meaning we'll assume perfect information

		// create a string describing the problem we've generated in JSHOP2 formalism
		FString Problem = "(defproblem problem simplefps\n";
		Problem += "	(\n";
		Problem += "	(npc-unaware)\n";
		Problem += FString::Printf(TEXT("	(npc-at area%d)\n"), InitialWorldState->NPCArea);
		Problem += "	(npc-not-close-to-point)\n";
		Problem += "	(npc-uncovered)\n";
		Problem += (InitialWorldState->bNPCInjured) ? "	(npc-injured)\n" : "	(npc-full-health)\n";
		Problem += "	(player p)\n";
		Problem += FString::Printf(TEXT("	(point-of-interest p area%d)\n"), PlayerArea);
		for(int32 Area = 0; Area < NumberOfAreas; ++Area)
		{
			if(InitialWorldState->AreaData[Area].bLighted)
			{
				Problem += FString::Printf(TEXT("	(lighted area%d)\n"), Area);
			}
			else
			{
				Problem += FString::Printf(TEXT("	(dark area%d)\n"), Area);
			}

			Problem += FString::Printf(TEXT("	(point-of-interest control-box%d area%d)\n"), Area, Area);
			Problem += FString::Printf(TEXT("	(control-box control-box%d)\n"), Area);
		}
		for(const FSimpleFPSObject* Waypoint : InitialWorldState->GetWaypoints())
		{
			const FSimpleFPSWaypointData& WaypointData = InitialWorldState->WaypointData[Waypoint->Index];
			int32 Area1 = WaypointData.Area1;
			int32 Area2 = WaypointData.Area2;
			Problem += FString::Printf(TEXT("	(waypoint door%d-%d)\n"), Area1, Area2);
			Problem += FString::Printf(TEXT("	(point-of-interest door%d-%d area%d)\n"), Area1, Area2, Area1);
			Problem += FString::Printf(TEXT("	(point-of-interest door%d-%d area%d)\n"), Area1, Area2, Area2);
			Problem += FString::Printf(TEXT("	(connected area%d area%d door%d-%d)\n"), Area1, Area2, Area1, Area2);
			Problem += FString::Printf(TEXT("	(connected area%d area%d door%d-%d)\n"), Area2, Area1, Area1, Area2);
			if(WaypointData.bOpen)
			{
				Problem += FString::Printf(TEXT("	(open door%d-%d)\n"), Area1, Area2);
			}
			else
			{
				Problem += FString::Printf(TEXT("	(closed door%d-%d)\n"), Area1, Area2);
				const FSimpleFPSKeycardData& KeycardData = InitialWorldState->KeycardData[WaypointData.KeycardIndex];
				Problem += FString::Printf(TEXT("	(point-of-interest keycard%d-%d area%d)\n"), Area1, Area2, KeycardData.Area);
				Problem += FString::Printf(TEXT("	(item keycard%d-%d)\n"), Area1, Area2);
				Problem += FString::Printf(TEXT("	(keycard keycard%d-%d door%d-%d)\n"), Area1, Area2, Area1, Area2);
			}
		}
		for(const FSimpleFPSObject* Gun : InitialWorldState->GetGuns())
		{
			const FSimpleFPSGunData& GunData = InitialWorldState->GunData[Gun->Index];
			Problem += FString::Printf(TEXT("	(point-of-interest %s area%d)\n"), *Gun->ObjectName, GunData.Area);
			Problem += FString::Printf(TEXT("	(item %s)\n"), *Gun->ObjectName);
			Problem += FString::Printf(TEXT("	(gun %s)\n"), *Gun->ObjectName);
			if(GunData.bHasNightVision)
			{
				Problem += FString::Printf(TEXT("	(has-nightvision %s)\n"), *Gun->ObjectName);
			}
			if(GunData.bLoaded)
			{
				Problem += FString::Printf(TEXT("	(loaded %s)\n"), *Gun->ObjectName);
			}
			else
			{
				Problem += FString::Printf(TEXT("	(unloaded %s)\n"), *Gun->ObjectName);
				FSimpleFPSObject* Ammo = InitialWorldState->GetAmmo()[GunData.Ammo];
				const FSimpleFPSAmmoData& AmmoData = InitialWorldState->AmmoData[GunData.Ammo];
				Problem += FString::Printf(TEXT("	(point-of-interest %s area%d)\n"), *Ammo->ObjectName, AmmoData.Area);
				Problem += FString::Printf(TEXT("	(item %s)\n"), *Ammo->ObjectName);
				Problem += FString::Printf(TEXT("	(ammo %s %s)\n"), *Ammo->ObjectName, *Gun->ObjectName);
			}
		}
		for(const FSimpleFPSObject* Knife : InitialWorldState->GetKnives())
		{
			const FSimpleFPSKnifeData& KnifeData = InitialWorldState->KnifeData[Knife->Index];
			Problem += FString::Printf(TEXT("	(point-of-interest %s area%d)\n"), *Knife->ObjectName, KnifeData.Area);
			Problem += FString::Printf(TEXT("	(item %s)\n"), *Knife->ObjectName);
			Problem += FString::Printf(TEXT("	(knife %s)\n"), *Knife->ObjectName);
		}
		for(const FSimpleFPSObject* Medikit : InitialWorldState->GetMedikits())
		{
			const FSimpleFPSMedikitData& MedikitData = InitialWorldState->MedikitData[Medikit->Index];
			Problem += FString::Printf(TEXT("	(point-of-interest %s area%d)\n"), *Medikit->ObjectName, MedikitData.Area);
			Problem += FString::Printf(TEXT("	(item %s)\n"), *Medikit->ObjectName);
			Problem += FString::Printf(TEXT("	(medikit %s)\n"), *Medikit->ObjectName);
		}
		for(const FSimpleFPSObject* CoverPoint : InitialWorldState->GetCoverPoints())
		{
			const FSimpleFPSCoverPointData& CoverPointData = InitialWorldState->CoverPointData[CoverPoint->Index];
			Problem += FString::Printf(TEXT("	(point-of-interest %s area%d)\n"), *CoverPoint->ObjectName, CoverPointData.Area);
			Problem += FString::Printf(TEXT("	(cover-point %s)\n"), *CoverPoint->ObjectName);
		}
		for(int32 Row = 0; Row < NumberOfAreas; ++Row)
		{
			for(int32 Col = 0; Col < NumberOfAreas; ++Col)
			{
				if(DistanceMatrix[Row][Col] < LARGE_DISTANCE)
				{
					Problem += FString::Printf(TEXT("	(distance area%d area%d %d)\n"), Row, Col, DistanceMatrix[Row][Col]);
				}
				else
				{
					Problem += FString::Printf(TEXT("	(unreachable area%d area%d)\n"), Row, Col);
				}
			}
		}
		Problem += "	)\n";
		Problem += "	((behave)))";

		if(bPrintJSHOP2Format)
		{
			UE_LOG(LogTemp, Warning, TEXT("Generated SimpleFPS problem in JSHOP2 format (seed = %d):"), InitializationSeed);
			UE_LOG(LogTemp, Warning, TEXT("%s"), *Problem);
		}

		// cache our world state's const data
		WorldStateConstData = InitialWorldState->GetConstData();
	}

	if(CurrentWorldState.IsValid())
	{
		WorldState->CopyFrom((const FHTNWorldState_SimpleFPS*)CurrentWorldState.Get());
	}
	else
	{
		CurrentWorldState = InitialWorldState->Copy();
		WorldState->CopyFrom(InitialWorldState.Get());
	}
}