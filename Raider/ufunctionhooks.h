#pragma once

#include <functional>

#include "game.h"
#include "ue4.h"

// #define LOGGING
//#define CHEATS
#define MAXPLAYERS 100

//Define the hook with ufunction full name
//Return true in the lambda to prevent the original function call

namespace UFunctionHooks
{
    inline std::vector<UFunction*> toHook;
    inline std::vector<std::function<bool(UObject*, void*)>> toCall;
    inline ABuildingSMActor* PreviousBuildingActor;

    #define DEFINE_PEHOOK(ufunctionName, func)                           \
        toHook.push_back(UObject::FindObject<UFunction>(ufunctionName)); \
        toCall.push_back([](UObject * Object, void* Parameters) -> bool func);

    auto Initialize()
    {
        DEFINE_PEHOOK("Function GameplayAbilities.AbilitySystemComponent.ServerTryActivateAbility", {
            auto AbilitySystemComponent = (UAbilitySystemComponent*)Object;
            auto Params = (UAbilitySystemComponent_ServerTryActivateAbility_Params*)Parameters;

            TryActivateAbility(AbilitySystemComponent, Params->AbilityToActivate, Params->InputPressed, &Params->PredictionKey, nullptr);

            return false;
        })

        DEFINE_PEHOOK("Function GameplayAbilities.AbilitySystemComponent.ServerTryActivateAbilityWithEventData", {
            auto AbilitySystemComponent = (UAbilitySystemComponent*)Object;
            auto Params = (UAbilitySystemComponent_ServerTryActivateAbilityWithEventData_Params*)Parameters;

            TryActivateAbility(AbilitySystemComponent, Params->AbilityToActivate, Params->InputPressed, &Params->PredictionKey, &Params->TriggerEventData);

            return false;
        })

        DEFINE_PEHOOK("Function GameplayAbilities.AbilitySystemComponent.ServerAbilityRPCBatch", {
            auto AbilitySystemComponent = (UAbilitySystemComponent*)Object;
            auto Params = (UAbilitySystemComponent_ServerAbilityRPCBatch_Params*)Parameters;

            TryActivateAbility(AbilitySystemComponent, Params->BatchInfo.AbilitySpecHandle, Params->BatchInfo.InputPressed, &Params->BatchInfo.PredictionKey, nullptr);
        
            return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerPawn.ServerHandlePickup", { 
            Inventory::OnPickup((AFortPlayerControllerAthena*)((APawn*)Object)->Controller, Parameters);
            return false;
        })
        /*
        DEFINE_PEHOOK("Function FortniteGame.FortPlayerController.ServerCheat", {
            if (Object->IsA(AFortPlayerControllerAthena::StaticClass()))
            {
                auto PC = (AFortPlayerControllerAthena*)Object;
                auto Params = (AFortPlayerController_ServerCheat_Params*)Parameters;

                if (Params && PC && !PC->IsInAircraft())
                {
                    auto Pawn = (APlayerPawn_Athena_C*)PC->Pawn;
                    auto Message = Params->Msg.ToString() + ' ';

                    std::vector<std::string> Arguments;

                    while (Message.find(" ") != -1)
                    {
                        Arguments.push_back(Message.substr(0, Message.find(' ')));
                        Message.erase(0, Message.find(' ') + 1);
                    }

                    auto NumArgs = Arguments.size() - 1;

                    if (NumArgs >= 0)
                    {
                        auto& Command = Arguments[0];
                        std::transform(Command.begin(), Command.end(), Command.begin(), ::tolower);

                        if (Command == "revive" && Pawn->bIsDBNO)
                        {
                            Pawn->bIsDBNO = false;
                            Pawn->OnRep_IsDBNO();

                            // PC->ClientOnPawnRevived(InstigatorPC);
                            Pawn->SetHealth(100);
                        }

                        else if (Command == "testindicator") // It doesn't replicate to teammates for some reason.
                        {
							auto PlayerState = (AFortPlayerStateAthena*)PC->PlayerState;

                            if (PlayerState)
                            {
                                PlayerState->OnRep_MapIndicatorPos();
								ClientMessage(PC, L"Updated Minimap Indicator!");
                            }
                        }
						
                        else if (Command == "giveweapon" && NumArgs >= 1)
                        {
                            auto& weaponName = Arguments[1];
                            int slot = 1;
                            int count = 1;

                            try
                            {
                                if (NumArgs >= 2)
                                    slot = std::stoi(Arguments[2]);

                                if (NumArgs >= 3)
                                    count = std::stoi(Arguments[3]);
                            }
                            catch (...)
                            {
                            }

                            auto WID = UObject::FindObject<UFortWeaponRangedItemDefinition>("FortWeaponRangedItemDefinition " + weaponName + '.' + weaponName);

                            if (WID && WID->IsA(UFortWeaponRangedItemDefinition::StaticClass()))
                            {
                                AddItemWithUpdate(PC, WID, slot, EFortQuickBars::Primary, count);
                                ClientMessage(PC, std::wstring(L"Successfully gave " + count + std::wstring(L" ") + toWStr(weaponName) + L" to slot " + std::to_wstring(slot)).c_str());
                            }
                            else
                                ClientMessage(PC, L"Requested item is not a weapon!\n");
                        }

                        else
                            ClientMessage(PC, L"Unable to handle command!");
                    }
                }
            }

            return false;
        })
        */
        DEFINE_PEHOOK("Function Engine.CheatManager.CheatScript", {
            return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerController.ServerCreateBuildingActor", {
            auto PC = (AFortPlayerControllerAthena*)Object;
            auto Params = (AFortPlayerController_ServerCreateBuildingActor_Params*)Parameters;
            auto CurrentBuildClass = Params->BuildingClassData.BuildingClass;
            auto Pawn = (AFortPlayerPawnAthena*)PC->Pawn;

            if (!Pawn)
                return true;

            if (!bBuildingAllowed)
                return false;

            if (!Pawn->PreviousBuild)
            {
                std::cout << "LogRaider: Player's previous build is invalid! Creating new instance.\n";
                if (PC && Params && CurrentBuildClass)
                {
                    {
                        auto BuildingActor = (ABuildingSMActor*)SpawnActor(CurrentBuildClass, Params->BuildLoc, Params->BuildRot, PC, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
                        Build(PC, BuildingActor, Params);
                        Pawn->PreviousBuild = BuildingActor;
                    }
                }
                return false;
            }

            if (CurrentBuildClass->GetFullName() != Pawn->PreviousBuild->Class->GetFullName())
            {
                if (PC && Params && CurrentBuildClass)
                {
                    auto BuildingActor = (ABuildingSMActor*)SpawnActor(CurrentBuildClass, Params->BuildLoc, Params->BuildRot, PC, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
                    Build(PC, BuildingActor, Params);
                    Pawn->PreviousBuild = BuildingActor;
                }
            }
            else
            {
                auto PreviousBuildLoc = Pawn->PreviousBuild->K2_GetActorLocation();
                if (Params->BuildLoc.X != PreviousBuildLoc.X || Params->BuildLoc.Y != PreviousBuildLoc.Y || Params->BuildLoc.Z != PreviousBuildLoc.Z)
                {
                    if (PC && Params && CurrentBuildClass)
                    {
                        auto BuildingActor = (ABuildingSMActor*)SpawnActor(CurrentBuildClass, Params->BuildLoc, Params->BuildRot, PC, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
                        Build(PC, BuildingActor, Params);
                        Pawn->PreviousBuild = BuildingActor;
                    }
                }
            }

            return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerController.ServerBeginEditingBuildingActor", {
            auto Params = (AFortPlayerController_ServerBeginEditingBuildingActor_Params*)Parameters;
            auto Controller = (AFortPlayerControllerAthena*)Object;
            auto Pawn = (APlayerPawn_Athena_C*)Controller->Pawn;
            bool bFound = false;
            auto EditToolEntry = FindItemInInventory<UFortEditToolItemDefinition>(Controller, bFound);

            if (Controller && Pawn && Params->BuildingActorToEdit && bFound)
            {
                auto EditTool = (AFortWeap_EditingTool*)EquipWeaponDefinition(Pawn, (UFortWeaponItemDefinition*)EditToolEntry.ItemDefinition, EditToolEntry.ItemGuid);

                if (EditTool)
                {
                    EditTool->EditActor = Params->BuildingActorToEdit;
                    EditTool->OnRep_EditActor();
                    Params->BuildingActorToEdit->EditingPlayer = (AFortPlayerStateZone*)Pawn->PlayerState;
                    Params->BuildingActorToEdit->OnRep_EditingPlayer();
                }
            }

            return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortDecoTool.ServerSpawnDeco", {
            SpawnDeco((AFortDecoTool*)Object, Parameters);
            return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerController.ServerEditBuildingActor",
        {
            auto Params = (AFortPlayerController_ServerEditBuildingActor_Params*)Parameters;
            auto PC = (AFortPlayerControllerAthena*)Object;

            if (!PC->Pawn)
                return true;

            if (PC && Params)
            {
                auto BuildingActor = Params->BuildingActorToEdit;
                auto NewBuildingClass = Params->NewBuildingClass;
                auto RotationIterations = Params->RotationIterations;

                if (BuildingActor && NewBuildingClass)
                {
                    auto location = BuildingActor->K2_GetActorLocation();
                    auto rotation = BuildingActor->K2_GetActorRotation();

                    int yaw = (int(rotation.Yaw) + 360) % 360; // Gets the rotation ranging from 0 to 360 degrees

                    if (BuildingActor->BuildingType != EFortBuildingType::Wall) // Centers building pieces if necessary
                    {
                        switch (yaw)
                        {
                        case 89:
                        case 90:
                        case 91: // Sometimes the rotation may differ by 1
                            switch (RotationIterations)
                            {
                            case 1:
                                location.X += -256;
                                location.Y += 256;
                                break;
                            case 2:
                                location.X += -512;
                                location.Y += 0;
                                break;
                            case 3:
                                location.X += -256;
                                location.Y += -256;
                                break;
                            }
                            yaw = 90;
                            break;
                        case 179:
                        case 180:
                        case 181:
                            switch (RotationIterations)
                            {
                            case 1:
                                location.X += -256;
                                location.Y += -256;
                                break;
                            case 2:
                                location.X += 0;
                                location.Y += -512;
                                break;
                            case 3:
                                location.X += 256;
                                location.Y += -256;
                                break;
                            }
                            yaw = 180;
                            break;
                        case 269:
                        case 270:
                        case 271:
                            switch (RotationIterations)
                            {
                            case 1:
                                location.X += 256;
                                location.Y += -256;
                                break;
                            case 2:
                                location.X += 512;
                                location.Y += 0;
                                break;
                            case 3:
                                location.X += 256;
                                location.Y += 256;
                                break;
                            }
                            yaw = 270;
                            break;
                        default: // 0, 360. etc.
                            switch (RotationIterations)
                            {
                            case 1:
                                location.X += 256;
                                location.Y += 256;
                                break;
                            case 2:
                                location.X += 0;
                                location.Y += 512;
                                break;
                            case 3:
                                location.X += -256;
                                location.Y += 256;
                                break;
                            }
                            yaw = 0;
                        }
                    }

                    rotation.Yaw = yaw + 90 * RotationIterations;

                    auto HealthPercent = BuildingActor->GetHealthPercent();

                    //  BuildingActor->K2_DestroyActor();
                    BuildingActor->SilentDie();

                    if (auto NewBuildingActor = (ABuildingSMActor*)SpawnActor(NewBuildingClass, location, rotation, PC))
                    {
                        if (!BuildingActor->bIsInitiallyBuilding)
                            NewBuildingActor->ForceBuildingHealth(NewBuildingActor->GetMaxHealth() * HealthPercent);
                        NewBuildingActor->SetMirrored(Params->bMirrored);
                        NewBuildingActor->InitializeKismetSpawnedBuildingActor(NewBuildingActor, PC);
                        NewBuildingActor->bPlayerPlaced = true;
                        auto PlayerState = (AFortPlayerStateAthena*)PC->PlayerState;
                        NewBuildingActor->Team = PlayerState->TeamIndex;
                        PlayerBuilds.push_back(NewBuildingActor);
                    }
                }
            }

            return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerController.ServerEndEditingBuildingActor", {
            auto Params = (AFortPlayerController_ServerEndEditingBuildingActor_Params*)Parameters;
            auto PC = (AFortPlayerControllerAthena*)Object;

            if (!PC->Pawn)
                return true;

            if (!PC->IsInAircraft() && Params->BuildingActorToStopEditing)
            {
                Params->BuildingActorToStopEditing->EditingPlayer = nullptr;
                Params->BuildingActorToStopEditing->OnRep_EditingPlayer();

                auto EditTool = (AFortWeap_EditingTool*)((APlayerPawn_Athena_C*)PC->Pawn)->CurrentWeapon;

                if (EditTool)
                {
                    EditTool->bEditConfirmed = true;
                    EditTool->EditActor = nullptr;
                    EditTool->OnRep_EditActor();
                }
            }

            return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerController.ServerRepairBuildingActor", {
            auto Params = (AFortPlayerController_ServerRepairBuildingActor_Params*)Parameters;
            auto Controller = (AFortPlayerControllerAthena*)Object;
            auto Pawn = (APlayerPawn_Athena_C*)Controller->Pawn;

            if (Controller && Pawn && Params->BuildingActorToRepair)
            {
                Params->BuildingActorToRepair->RepairBuilding(Controller, 10); // TODO: Figure out how to get the repair amount
            }

            return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerControllerAthena.ServerAttemptAircraftJump", {
            auto Params = (AFortPlayerControllerAthena_ServerAttemptAircraftJump_Params*)Parameters;
            auto PC = (AFortPlayerControllerAthena*)Object;
            auto GameState = (AAthena_GameState_C*)GetWorld()->AuthorityGameMode->GameState;

            if (PC && Params && !PC->Pawn && PC->IsInAircraft())
            {
                auto Aircraft = (AFortAthenaAircraft*)GameState->Aircrafts[0];

                if (Aircraft)
                {
                    auto ExitLocation = Aircraft->K2_GetActorLocation();

                    // ExitLocation.Z -= 500;

                    InitPawn(PC, ExitLocation, FQuat(), false);
                    ((AAthena_GameState_C*)GetWorld()->AuthorityGameMode->GameState)->Aircrafts[0]->PlayEffectsForPlayerJumped();
                    PlayersJumpedFromBus++;
                    auto State = (AFortPlayerStateAthena*)PC->PlayerState;

                    //Seeker = (AFortPlayerPawnAthena*)State->GetCurrentPawn();

                    if (bHideAndSeek)
                        HideAndSeek().AircraftJump(PC, State, Seeker);
                    else
                    {
                        PC->ActivateSlot(EFortQuickBars::Primary, 0, 0, true); // Select the pickaxe

                        bool bFound = false;
                        auto PickaxeEntry = FindItemInInventory<UFortWeaponMeleeItemDefinition>(PC, bFound);

                        if (bFound)
                            EquipInventoryItem(PC, PickaxeEntry.ItemGuid);
                    }

                    auto PlayerState = (AFortPlayerStateAthena*)PC->PlayerState;
                    PlayerState->OnRep_CharacterParts();
                    // PC->Pawn->K2_TeleportTo(ExitLocation, Params->ClientRotation);
                    if (PlayersJumpedFromBus >= GameState->PlayerArray.Num() && bSafeZoneBased)
                    {
                        ((AFortGameModeAthena*)GetWorld()->AuthorityGameMode)->OnAircraftExitedDropZone(GameState->GetAircraft(0));
                        GameState->SafeZonesStartTime -= 59;
                        //GetKismetSystem()->STATIC_ExecuteConsoleCommand(GetWorld(), L"startaircraft", nullptr);
                        //GameState->GamePhase = EAthenaGamePhase::SafeZones;
                        //GameState->OnRep_GamePhase(EAthenaGamePhase::Aircraft);
                    }
                }
            }

            return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerController.ServerAttemptInventoryDrop", {
            auto PC = (AFortPlayerControllerAthena*)Object;

            if (PC && !PC->IsInAircraft())
                Inventory::OnDrop(PC, Parameters);

            return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerController.ServerExecuteInventoryItem", {
            EquipInventoryItem((AFortPlayerControllerAthena*)Object, *(FGuid*)Parameters);

            return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerController.ServerReturnToMainMenu", {
            ((AFortPlayerController*)Object)->ClientTravel(L"Frontend", ETravelType::TRAVEL_Absolute, false, FGuid());

            return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerController.ServerLoadingScreenDropped", {
            auto Pawn = (APlayerPawn_Athena_C*)((AFortPlayerController*)Object)->Pawn;

            if (IsBanned(std::wstring(Pawn->PlayerState->SavedNetworkAddress.c_str())))
            {
                std::cout << "Player is banned!\n";
                KickController((APlayerController*)Pawn->Controller, L"You are banned."); // TODO: Add ban reason to here too.
                // Should we change their name to "Banned" or something?
                return true;
            }

            auto PC = (AFortPlayerControllerAthena*)Object;
            PC->ActivateSlot(EFortQuickBars::Primary, 0, 0, true); // Select the pickaxe

            bool bFound = false;
            auto PickaxeEntry = FindItemInInventory<UFortWeaponMeleeItemDefinition>(PC, bFound);

            if (bFound)
                EquipInventoryItem(PC, PickaxeEntry.ItemGuid);

            return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerPawn.ServerChoosePart", 
        {
            return true;
        })

        DEFINE_PEHOOK("Function Engine.GameMode.ReadyToStartMatch", {
            if (!bListening)
            {
                if (!bReadyToStart)
                {
                    return true;
                }
                Game::OnReadyToStartMatch();

                HostBeacon = SpawnActor<AFortOnlineBeaconHost>();
                HostBeacon->ListenPort = 7776;
                auto bInitBeacon = Native::OnlineBeaconHost::InitHost(HostBeacon);
                CheckNullFatal(bInitBeacon, "Failed to initialize the Beacon!");

                HostBeacon->NetDriverName = FName(282); // REGISTER_NAME(282,GameNetDriver)
                HostBeacon->NetDriver->NetDriverName = FName(282); // REGISTER_NAME(282,GameNetDriver)
                HostBeacon->NetDriver->World = GetWorld();
                FString Error;
                auto InURL = FURL();
                InURL.Port = 7777;

                Native::NetDriver::InitListen(HostBeacon->NetDriver, GetWorld(), InURL, true, Error);

                Native::ReplicationDriver::ServerReplicateActors = decltype(Native::ReplicationDriver::ServerReplicateActors)(HostBeacon->NetDriver->ReplicationDriver->Vtable[0x53]);

                auto ClassRepNodePolicies = GetClassRepNodePolicies(HostBeacon->NetDriver->ReplicationDriver);

                for (auto&& Pair : ClassRepNodePolicies)
                {
                    auto key = Pair.Key().ResolveObjectPtr();
                    auto& value = Pair.Value();

                    LOG_INFO("ClassRepNodePolicies: {} - {}", key->GetName(), ClassRepNodeMappingToString(value));

                    if (key == AFortInventory::StaticClass())
                    {
                        value = EClassRepNodeMapping::RelevantAllConnections;
                        LOG_INFO("Found ClassRepNodePolicy for AFortInventory! {}", (int)value);
                    }

                    if (key == AFortQuickBars::StaticClass())
                    {
                        value = EClassRepNodeMapping::RelevantAllConnections;
                        LOG_INFO("Found ClassRepNodePolicy for AFortQuickBars! {}", (int)value);
                    }
                }

                GetWorld()->NetDriver = HostBeacon->NetDriver;
                GetWorld()->LevelCollections[0].NetDriver = HostBeacon->NetDriver;
                GetWorld()->LevelCollections[1].NetDriver = HostBeacon->NetDriver;

                GetWorld()->AuthorityGameMode->GameSession->MaxPlayers = MAXPLAYERS;

                Native::OnlineBeacon::PauseBeaconRequests(HostBeacon, false);
                bListening = true;
                LOG_INFO("Listening for connections on port {}!", HostBeacon->ListenPort);
            }

            return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortGameModeAthena.OnAircraftExitedDropZone", {

			if (GetWorld() && GetWorld()->NetDriver && GetWorld()->NetDriver->ClientConnections.Data)
            {
                auto Connections = HostBeacon->NetDriver->ClientConnections;

                for (int i = 0; i < Connections.Num(); i++)
                {
                    auto Controller = (AFortPlayerControllerAthena*)Connections[i]->PlayerController;

                    if (!Controller || !Controller->IsA(AFortPlayerControllerAthena::StaticClass()) || Controller->PlayerState->bIsSpectator)
                        continue;

                    if (Controller && Controller->IsInAircraft())
                        Controller->ServerAttemptAircraftJump(FRotator());
                }            
            }

            return false;
        })

        DEFINE_PEHOOK("Function FortniteGame.FortPlayerController.ServerCheatAll", {
            auto PlayerController = (AFortPlayerControllerAthena*)Object;

            if (PlayerController)
                KickController((AFortPlayerControllerAthena*)Object, L"Please do not do that!");
            return true;
        })

        LOG_INFO("[UFunctionHooks::Initialize] Hooked {} UFunction(s)", toHook.size());
    }
}
