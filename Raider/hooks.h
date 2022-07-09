#pragma once

#include "gui.h"
#include "ufunctionhooks.h"

// #define LOGGING
#define HEALTH 100
#define SHIELD 100

namespace Hooks
{
    bool LocalPlayerSpawnPlayActor(ULocalPlayer* Player, const FString& URL, FString& OutError, UWorld* World) // prevent server's pc from spawning
    {
        if (bTraveled)
            return true;
        else
            return Native::LocalPlayer::SpawnPlayActor(Player, URL, OutError, World);
    }

    uint64 GetNetMode(UWorld* World) // PlayerController::SendClientAdjustment checks if the netmode is not client
    {
        return 2; // ENetMode::NM_ListenServer;
    }

    void TickFlush(UNetDriver* NetDriver, float DeltaSeconds)
    {
        if (!NetDriver)
            return;

		if (bMapFullyLoaded)
        {
            if (NetDriver->IsA(UIpNetDriver::StaticClass()) && NetDriver->ClientConnections.Num() > 0 && NetDriver->ClientConnections[0]->InternalAck == false)
            {
                Replication::ServerReplicateActors(NetDriver);
            }
        }

        Native::NetDriver::TickFlush(NetDriver, DeltaSeconds);
    }

    void WelcomePlayer(UWorld* World, UNetConnection* IncomingConnection)
    {
        Native::World::WelcomePlayer(GetWorld(), IncomingConnection);
    }

    char KickPlayer(__int64 a1, __int64 a2, __int64 a3)
    {
        return 0;
    }

    void World_NotifyControlMessage(UWorld* World, UNetConnection* Connection, uint8 MessageType, void* Bunch)
    {
        Native::World::NotifyControlMessage(GetWorld(), Connection, MessageType, Bunch);
    }

    // eliminates the chance of two players being on the same team randomly.
    // TODO: Sort STW teams from PvP teams.
    auto ApplyTeam()
    {
        if (!hasSetup)
        {
            hasSetup = true;
            for (int i = 2; i < 104; i++)
            {
                teamsmap.insert_or_assign((EFortTeam)i, false);
            }
        }

        for (auto team : teamsmap)
        {
            if (team.second)
                continue;

            teamsmap.insert_or_assign(team.first, true);

            return team.first;
        }
    }

    APlayerController* SpawnPlayActor(UWorld* World, UPlayer* NewPlayer, ENetRole RemoteRole, FURL& URL, void* UniqueId, SDK::FString& Error, uint8 NetPlayerIndex)
    {
        auto PlayerController = (AFortPlayerControllerAthena*)Native::World::SpawnPlayActor(GetWorld(), NewPlayer, RemoteRole, URL, UniqueId, Error, NetPlayerIndex);
        NewPlayer->PlayerController = PlayerController;

        auto PlayerState = (AFortPlayerStateAthena*)PlayerController->PlayerState;

        InitInventory(PlayerController, true);

        auto Pawn = (APlayerPawn_Athena_C*)SpawnActorTrans(APlayerPawn_Athena_C::StaticClass(), GetPlayerStart(PlayerController), PlayerController);

        PlayerController->Pawn = Pawn;
        PlayerController->AcknowledgedPawn = Pawn;
        Pawn->Owner = PlayerController;
        Pawn->OnRep_Owner();
        PlayerController->OnRep_Pawn();
        PlayerController->Possess(Pawn);

        Pawn->SetMaxHealth(HEALTH);
        Pawn->SetMaxShield(SHIELD);
        Pawn->ShieldRegenGameplayEffect = nullptr;
        Pawn->ShieldRegenDelayGameplayEffect = nullptr;
        Pawn->HealthRegenGameplayEffect = nullptr;
        Pawn->HealthRegenDelayGameplayEffect = nullptr;

        PlayerController->bHasClientFinishedLoading = true;
        PlayerController->bHasServerFinishedLoading = true;
        PlayerController->bHasInitiallySpawned = true;
        PlayerController->OnRep_bHasServerFinishedLoading();

        PlayerState->bHasFinishedLoading = true;
        PlayerState->bHasStartedPlaying = true;
        PlayerState->OnRep_bHasStartedPlaying();

        static auto FortRegisteredPlayerInfo = ((UFortGameInstance*)GetWorld()->OwningGameInstance)->RegisteredPlayers[0];

        if (FortRegisteredPlayerInfo)
        {
            auto Hero = FortRegisteredPlayerInfo->AthenaMenuHeroDef;

            if (Hero)
            {
                if (bCosmetics)
                {
                    PlayerState->HeroType = Hero->GetHeroTypeBP();
                    PlayerState->OnRep_HeroType();

                    for (auto i = 0; i < Hero->CharacterParts.Num(); i++)
                    {
                        auto Part = Hero->CharacterParts[i];

                        if (!Part)
                            continue;

                        PlayerState->CharacterParts[i] = Part;
                    }

                    PlayerState->CharacterBodyType = Hero->CharacterParts[1]->BodyTypesPermitted;
                    Pawn->CharacterBodyType = Hero->CharacterParts[1]->BodyTypesPermitted;
                    Pawn->CharacterGender = Hero->CharacterParts[1]->GenderPermitted;
                    PlayerState->OnRep_CharacterBodyType();
                    PlayerState->OnRep_CharacterParts();
                }
                else
                {
                    UFortHeroType* HeroType = Hero->GetHeroTypeBP(); // UObject::FindObject<UFortHeroType>("FortHeroType HID_Outlander_015_F_V1_SR_T04.HID_Outlander_015_F_V1_SR_T04");
                    PlayerState->HeroType = HeroType;
                    PlayerState->OnRep_HeroType();

                    static auto Head = UObject::FindObject<UCustomCharacterPart>("CustomCharacterPart F_Med_Head1.F_Med_Head1");
                    static auto Body = UObject::FindObject<UCustomCharacterPart>("CustomCharacterPart F_Med_Soldier_01.F_Med_Soldier_01");

                    PlayerState->CharacterParts[(uint8_t)EFortCustomPartType::Head] = Head;
                    PlayerState->CharacterParts[(uint8_t)EFortCustomPartType::Body] = Body;
                    PlayerState->OnRep_CharacterParts();
                }
            }
        }

        static std::vector<std::string> PickaxePool = 
        {
            "WID_Harvest_Pickaxe_Prismatic",
            "WID_Harvest_Pickaxe_Anchor_Athena",
            "WID_Harvest_Pickaxe_ArtDeco",
            "WID_Harvest_Pickaxe_Assassin",
            "WID_Harvest_Pickaxe_Assassin",
            "WID_Harvest_Pickaxe_BoltOn_Athena_C_T01",
            "WID_Harvest_Pickaxe_Brite",
            "WID_Harvest_Pickaxe_Carrot",
            "WID_Harvest_Pickaxe_CircuitBreaker",
            "WID_Harvest_Pickaxe_CuChulainn",
            "WID_Harvest_Pickaxe_Deathvalley_Athena_C_T01",
            "WID_Harvest_Pickaxe_Disco_Athena",
            "WID_Harvest_Pickaxe_District_Athena",
            "WID_Harvest_Pickaxe_Dragon",
            "WID_Harvest_Pickaxe_Flamingo_Athena_C_T01",
            "WID_Harvest_Pickaxe_Heart_Athena",
            "WID_Harvest_Pickaxe_HolidayCandyCane_Athena",
            "WID_Harvest_Pickaxe_HolidayGiftWrap_Athena",
            "WID_Harvest_Pickaxe_IcePick_Athena_C_T01",
            "WID_Harvest_Pickaxe_Keg_Athena",
            "WID_Harvest_Pickaxe_Lockjaw_Athena_C_T01",
            "WID_Harvest_Pickaxe_Medieval_Athena",
            "WID_Harvest_Pickaxe_Megalodon_Athena",
            "WID_Harvest_Pickaxe_PajamaParty",
            "WID_Harvest_Pickaxe_Pizza",
            "WID_Harvest_Pickaxe_Plunger",
            "WID_Harvest_Pickaxe_PotOfGold",
            "WID_Harvest_Pickaxe_RockerPunk",
            "WID_Harvest_Pickaxe_Scavenger",
            "WID_Harvest_Pickaxe_Shark_Athena",
            "WID_Harvest_Pickaxe_SickleBat_Athena_C_T01",
            "WID_Harvest_Pickaxe_SkiBoot",
            "WID_Harvest_Pickaxe_Smiley_Athena_C_T01",
            "WID_Harvest_Pickaxe_Space",
            "WID_Harvest_Pickaxe_Spikey_Athena_C_T01",
            "WID_Harvest_Pickaxe_Squeak",
            "WID_Harvest_Pickaxe_Stealth",
            "WID_Harvest_Pickaxe_Tactical",
            "WID_Harvest_Pickaxe_TacticalBlack",
            "WID_Harvest_Pickaxe_TacticalUrban",
            "WID_Harvest_Pickaxe_Teslacoil_Athena",
            "WID_Harvest_Pickaxe_WinterCamo_Athena"
        };

        static auto Default = FindWID("WID_Harvest_Pickaxe_Athena_C_T01");
        static auto RandomPickaxe = FindWID(PickaxePool[rand() % PickaxePool.size()]);
        UFortWeaponRangedItemDefinition* Pickaxe;
        Pickaxe = bCosmetics ? RandomPickaxe : Default;
        std::cout << Pickaxe->GetName() << ": Pickaxe\n";

        switch (loadoutToUse)
        {
            case WeaponLoadout::REGULAR:
            {
                // EXPLANATION: For choice between default pickaxe/cosmetic pickaxes to work, the vector must not be static, so then we make the WEAPONS static and not the vector.

                static auto Shotgun = FindWID("WID_Shotgun_Standard_Athena_UC_Ore_T03");
                static auto SMG = FindWID("WID_Pistol_AutoHeavy_Athena_R_Ore_T03");
                static auto Rifle = FindWID("WID_Assault_AutoHigh_Athena_SR_Ore_T03");
                //static auto Sniper = FindWID("WID_Sniper_BoltAction_Scope_Athena_SR_Ore_T03");
                //static auto Consumable = FindWID("Athena_PurpleStuff");
                PlayerLoadout FortLoadout = 
                {
                    Pickaxe,
                    Game::Shotguns[rand() % Game::Shotguns.size()],
                    Game::SMGs[rand() % Game::SMGs.size()],
                    Game::Rifles[rand() % Game::Rifles.size()],
                    Game::FunConsumables[rand() % Game::FunConsumables.size()],
                    Game::Heals[rand() % Game::Heals.size()]
                };

                EquipLoadout(PlayerController, FortLoadout);
                break;
            }
            case WeaponLoadout::EXPLOSIVES:
            {
                static auto Rocket1 = FindWID("WID_Launcher_Rocket_Athena_R_Ore_T03");
                static auto Rocket2 = FindWID("WID_Launcher_Rocket_Athena_R_Ore_T03");
                static auto GrenadeLauncher = FindWID("WID_Launcher_Grenade_Athena_SR_Ore_T03");
                static auto C4 = FindWID("Athena_C4");
                static auto Consumable = FindWID("Athena_PurpleStuff");
                PlayerLoadout FortLoadout = 
                {
                    Pickaxe,
                    Rocket1,
                    Rocket2,
                    GrenadeLauncher,
                    C4,
                    Consumable
                };
                EquipLoadout(PlayerController, FortLoadout);
                break;
            }
            case WeaponLoadout::SNIPERS:
            {
                static auto BoltAction = FindWID("WID_Sniper_BoltAction_Scope_Athena_SR_Ore_T03");
                static auto Automatic = FindWID("WID_Sniper_Standard_Scope_Athena_SR_Ore_T03");
                static auto Hunting = FindWID("WID_Sniper_NoScope_Athena_R_Ore_T03");
                static auto Crossbow = FindWID("WID_Sniper_Crossbow_Athena_VR_Ore_T03");
                static auto Consumable = FindWID("Athena_PurpleStuff");
                PlayerLoadout FortLoadout = 
                {
                    Pickaxe,
                    BoltAction,
                    Automatic,
                    Hunting,
                    Crossbow,
                    Consumable
                };
                EquipLoadout(PlayerController, FortLoadout);
                break;
            }
        }

        auto CheatManager = CreateCheatManager(PlayerController);
        CheatManager->ToggleInfiniteAmmo();
        CheatManager->ToggleInfiniteDurability();
        if (reinterpret_cast<AAthena_GameState_C*>(GetWorld()->GameState)->GamePhase == EAthenaGamePhase::Warmup)
            CheatManager->God();

        if (PlayerController->Pawn)
        {
            if (PlayerController->Pawn->PlayerState)
            {
                PlayerState->TeamIndex = ApplyTeam(); //EFortTeam(rand() % 101);
                PlayerState->OnRep_PlayerTeam();
            }
        }

        PlayerController->OverriddenBackpackSize = 100;

        if (Pawn && Pawn->AbilitySystemComponent)
            ApplyAbilities(Pawn);

        auto Drone = (ABP_VictoryDrone_C*)SpawnActor(ABP_VictoryDrone_C::StaticClass(), Pawn->K2_GetActorLocation());
        Drone->Owner = nullptr;
        Drone->OnRep_Owner();
        Drone->TriggerPlayerSpawnEffects();

        return PlayerController;
    }

    void Beacon_NotifyControlMessage(AOnlineBeaconHost* Beacon, UNetConnection* Connection, uint8 MessageType, int64* Bunch)
    {
        if (((AAthena_GameState_C*)GetWorld()->GameState)->GamePhase != EAthenaGamePhase::Warmup && !bPlayground)
        {
            KickController(Connection->PlayerController, L"Game has already started.");
            printf("LogRaider: Player tried to join, but cannot because the game has already started.\n");
            return;
        }

        printf("Recieved control message %i\n", MessageType);

        switch (MessageType)
        {
        case 4: // NMT_Netspeed
            Connection->CurrentNetSpeed = 30000;
            return;
        case 5: // NMT_Login
        {
            Bunch[7] += (16 * 1024 * 1024);

            auto OnlinePlatformName = FString(L"");

            Native::NetConnection::ReceiveFString(Bunch, Connection->ClientResponse);
            Native::NetConnection::ReceiveFString(Bunch, Connection->RequestURL);
            Native::NetConnection::ReceiveUniqueIdRepl(Bunch, Connection->PlayerID);
            Native::NetConnection::ReceiveFString(Bunch, OnlinePlatformName);

            Bunch[7] -= (16 * 1024 * 1024);

            Native::World::WelcomePlayer(GetWorld(), Connection);
			
            return;
        }
        case 15: // NMT_PCSwap
            // return;
            break;
        }

        Native::World::NotifyControlMessage(GetWorld(), Connection, MessageType, Bunch);
    }

    uint8 Beacon_NotifyAcceptingConnection(AOnlineBeacon* Beacon)
    {
        return Native::World::NotifyAcceptingConnection(GetWorld());
    }

    void* SeamlessTravelHandlerForWorld(UEngine* Engine, UWorld* World)
    {
        return Native::Engine::SeamlessTravelHandlerForWorld(Engine, GetWorld());
    }

    void* NetDebug(UObject* _this)
    {
        return nullptr;
    }

    void PostRender(UGameViewportClient* _this, UCanvas* Canvas)
    {
        ZeroGUI::SetupCanvas(Canvas);
        GUI::Tick();

        return Native::GameViewportClient::PostRender(_this, Canvas);
    }

    __int64 CollectGarbage(__int64 a1)
    {
        return 0;
    };

    void InitNetworkHooks()
    {
        DETOUR_START
        DetourAttachE(Native::World::WelcomePlayer, WelcomePlayer);
        DetourAttachE(Native::Actor::GetNetMode, GetNetMode);
        DetourAttachE(Native::World::NotifyControlMessage, World_NotifyControlMessage);
        DetourAttachE(Native::World::SpawnPlayActor, SpawnPlayActor);
        DetourAttachE(Native::OnlineBeaconHost::NotifyControlMessage, Beacon_NotifyControlMessage);
        DetourAttachE(Native::OnlineSession::KickPlayer, KickPlayer);
        DetourAttachE(Native::GameViewportClient::PostRender, PostRender);
        DetourAttachE(Native::GC::CollectGarbage, CollectGarbage);
        DETOUR_END
    }

    void DetachNetworkHooks()
    {
        DETOUR_START
        DetourDetachE(Native::World::WelcomePlayer, WelcomePlayer);
        DetourDetachE(Native::Actor::GetNetMode, GetNetMode);
        DetourDetachE(Native::World::NotifyControlMessage, World_NotifyControlMessage);
        DetourDetachE(Native::World::SpawnPlayActor, SpawnPlayActor);
        DetourDetachE(Native::OnlineBeaconHost::NotifyControlMessage, Beacon_NotifyControlMessage);
        DetourDetachE(Native::OnlineSession::KickPlayer, KickPlayer);
        DetourDetachE(Native::GameViewportClient::PostRender, PostRender);
        DetourDetachE(Native::GC::CollectGarbage, CollectGarbage);
        DETOUR_END
    }

    UNetConnection* SpectatorConnection;
    AFortPlayerStateAthena* ToSpectatePlayerState;

    auto UpdateSpecvars(UNetConnection* NetConnection, AFortPlayerStateAthena* PlayerState)
    {
        SpectatorConnection = NetConnection;
        ToSpectatePlayerState = PlayerState;
    }

    auto GetDeathReason(FFortPlayerDeathReport DeathReport)
    {
        static std::map<std::string, EDeathCause> DeathReasonMap
        {
            { "weapon.ranged.shotgun", EDeathCause::Shotgun },
            { "weapon.ranged.assault", EDeathCause::Rifle },
            { "Gameplay.Damage.Environment.Falling", EDeathCause::FallDamage },
            { "weapon.ranged.sniper", EDeathCause::Sniper },
            { "Weapon.Ranged.SMG", EDeathCause::SMG },
            { "weapon.ranged.heavy.rocket_launcher", EDeathCause::RocketLauncher },
            { "weapon.ranged.heavy.grenade_launcher", EDeathCause::GrenadeLauncher },
            { "Weapon.ranged.heavy.grenade", EDeathCause::Grenade },
            { "Weapon.Ranged.Heavy.Minigun", EDeathCause::Minigun },
            { "Weapon.Ranged.Crossbow", EDeathCause::Bow },
            { "trap.floor", EDeathCause::Trap },
            { "weapon.ranged.pistol", EDeathCause::Pistol },
            { "Gameplay.Damage.OutsideSafeZone", EDeathCause::OutsideSafeZone },
            { "Weapon.Melee.Impact.Pickaxe", EDeathCause::Melee }
        };

        for (int i = 0; i < DeathReport.Tags.GameplayTags.Num(); i++)
        {
            auto nameTag = DeathReport.Tags.GameplayTags[i].TagName.ToString();

            for (auto Map : DeathReasonMap)
            {
                if (nameTag == Map.first) return Map.second;
                else continue;
            }
        }

        return EDeathCause::Unspecified;
    }

    void ProcessEventHook(UObject* Object, UFunction* Function, void* Parameters)
    {
        if (!bPlayButton)
        {
            static auto PlayButtonFn = UObject::FindObject<UFunction>("BndEvt__BP_PlayButton_K2Node_ComponentBoundEvent_1_CommonButtonClicked__DelegateSignature");

            // if (FunctionName.find("BP_PlayButton") != -1)
            if (Function == PlayButtonFn)
            {
                bPlayButton = true;
                Game::Start();
                printf("[Game::Start] Done\n");

                InitNetworkHooks();
                printf("[InitNetworkHooks] Done\n");
                TimesInGame++;
            }
        }

        if (Function->GetName().find("Tick") != std::string::npos && bPlayground && PlayerBuilds.size() >= 600)
        {
            for (auto build : PlayerBuilds)
            {
                if (build != nullptr)
                    build->K2_DestroyActor();
            }
            PlayerBuilds.clear();
        }

        if (Function->GetName().find("Tick") != std::string::npos && bRestart)
        {
            bRestart = false;
            bTraveled = false;
            bPlayButton = false;
            bListening = false;
            bReadyToStart = false;
            bSpawnedFloorLoot = false;
            bMapFullyLoaded = false;
            bStartedBus = false;
            HostBeacon = nullptr;
            bCosmetics = true;
            bLoadoutRegular = true;
            bLoadoutExplosives = false;
            bLoadoutSnipers = false;
            bBusOnLocations = false;
            bBuildingAllowed = true;
            bRespawn = false;
            bHideAndSeek = false;
            bPlayground = false;
            bStormPaused = false;
            bSafeZoneBased = false;
            PlayersJumpedFromBus = 0;
            ((AFortGameModeAthena*)GetWorld()->AuthorityGameMode)->bSafeZonePaused = false;
            SpectatorConnection = nullptr;
            ToSpectatePlayerState = nullptr;
            PlayerBuilds.clear();
            for (auto team : teamsmap)
            {
                if (team.second)
                    teamsmap.insert_or_assign(team.first, false);
                else
                    break;
            }
            DetachNetworkHooks();
            GetKismetSystem()->STATIC_ExecuteConsoleCommand(GetWorld(), L"open frontend", GetPlayerController());
        }

        if (Function->GetFullName() == "Function FortniteGame.FortPlayerController.ServerPlayEmoteItem")
        {
            return;
            auto Params = (AFortPlayerController_ServerPlayEmoteItem_Params*)Parameters;
            auto PC = (AFortPlayerControllerAthena*)Object;
            auto Pawn = (AFortPlayerPawnAthena*)PC->Pawn;

            auto FortAnimInstance = static_cast<UFortAnimInstance*>(Pawn->Mesh->GetAnimInstance());
            auto Montage = Params->EmoteAsset->GetAnimationHardReference(Pawn->CharacterBodyType, Pawn->CharacterGender);
            if (FortAnimInstance)
            {
                Pawn->OnMontageStarted(Montage);
            }
            /*
            if (!FortAnimInstance->bIsJumping && !FortAnimInstance->bIsFalling && !PC->bIsPlayerActivelyMoving)
            {
                if (Pawn->bIsCrouched)
                    Pawn->UnCrouch(true);

                auto EmoteAsset = static_cast<AFortPlayerController_ServerPlayEmoteItem_Params*>(Params)->EmoteAsset;
                auto Montage = EmoteAsset->GetAnimationHardReference(EFortCustomBodyType::All, EFortCustomGender::Both);
                
                auto AnimInstance = (UFortPlayerAnimInstance*)Pawn->Mesh->GetAnimInstance();
                PC->PlayEmoteItem(EmoteAsset);
                AnimInstance->Montage_Play(Montage, 1, EMontagePlayReturnType::Duration, 0, true);
                Pawn->OnMontageStarted(Montage);
                Pawn->RepAnimMontageInfo.AnimMontage = Montage;
                Pawn->RepAnimMontageInfo.PlayRate = 1;
                Pawn->OnRep_ReplicatedAnimMontage();
                Pawn->ForceNetUpdate();
                
            }
            */
        }


        if (Function->GetFullName() == "Function BP_VictoryDrone.BP_VictoryDrone_C.OnSpawnOutAnimEnded")
        {
            if (!bPlayground)
            {
                auto drone = (ABP_VictoryDrone_C*)Object;

                if (drone->Owner != nullptr && drone->PlayerState != nullptr)
                {
                    if (((AFortPlayerControllerAthena*)drone->Owner)->Pawn)
                        ((AFortPlayerControllerAthena*)drone->Owner)->Pawn->K2_DestroyActor();

                    if (((AFortPlayerControllerAthena*)drone->Owner)->NetConnection && ((AFortPlayerControllerAthena*)drone->PlayerState->Owner)->Pawn)
                        Spectate(((AFortPlayerControllerAthena*)drone->Owner)->NetConnection, (AFortPlayerStateAthena*)drone->PlayerState);
                }
                
                drone->K2_DestroyActor();
            }
            else
            {
                auto drone = (ABP_VictoryDrone_C*)Object;
                if (Object->IsA(ABP_VictoryDrone_C::StaticClass()))
                {
                    if (drone->Owner)
                    {
                        auto DronePC = (AFortPlayerControllerAthena*)drone->Owner;
                        if (DronePC->NetConnection)
                            Playground().DroneAnimEnded(DronePC, drone->K2_GetActorLocation());
                    }

                    drone->K2_DestroyActor();
                }
            }
        }

        if (Function->GetFullName() == "Function FortniteGame.FortPlayerController.ServerAttemptInteract" && bLooting)
        {
            auto Params = (AFortPlayerController_ServerAttemptInteract_Params*)Parameters;
            //std::cout << "\n\n" << Params->ReceivingActor->GetName() << "\n" << Params->ReceivingActor->StaticClass()->GetFullName() << "\n\n"; \
            \
            code lowkey needs cleanup + improvements \
            todo: ammo count logic 
            if (Params->ReceivingActor->GetFullName().find("Tiered_Chest_6_Parent_C") != std::string::npos)
            {
                auto Container = (ABuildingContainer*)Params->ReceivingActor;
                Container->bAlreadySearched = true;
                Container->bStartAlreadySearched_Athena = true;
                Container->OnRep_bAlreadySearched();
                auto Loot = Game::GetWeaponLoot();
                SummonPickup(nullptr, Loot, 1, Container->K2_GetActorLocation());
                SummonPickup(nullptr, Loot->GetAmmoWorldItemDefinition_BP(), 1, Container->K2_GetActorLocation());
                SummonPickup(nullptr, Game::GetRandomMaterial(), 30, Container->K2_GetActorLocation());
            }
            if (Params->ReceivingActor->GetFullName().find("Tiered_Short_Ammo_3_Parent_C") != std::string::npos)
            {
                auto Container = (ABuildingContainer*)Params->ReceivingActor;
                Container->bAlreadySearched = true;
                Container->bStartAlreadySearched_Athena = true;
                Container->OnRep_bAlreadySearched();
                auto Loot = Game::GetWeaponLoot()->GetAmmoWorldItemDefinition_BP();
                SummonPickupFromChest(Loot, 30, Container->K2_GetActorLocation());
            }
            if (Params->ReceivingActor->GetFullName().find("AthenaSupplyDrop") != std::string::npos)
            {
                auto Container = (ABuildingContainer*)Params->ReceivingActor;
                Container->bAlreadySearched = true;
                Container->bStartAlreadySearched_Athena = true;
                Container->OnRep_bAlreadySearched();
                auto Loot = Game::GetSupplyDropLoot();
                SummonPickup(nullptr, Loot, 1, Container->K2_GetActorLocation());
                SummonPickup(nullptr, Loot->GetAmmoWorldItemDefinition_BP(), 1, Container->K2_GetActorLocation());
                SummonPickup(nullptr, Game::GetRandomMaterial(), 100, Container->K2_GetActorLocation());
            }
            // this one is just for funsies take it out if u want
            if (Params->ReceivingActor->GetFullName().find("Prop_QuestInteractable_GniceGnome") != std::string::npos)
            {
                KickController((APlayerController*)Object, L"dont fuck with the gnome");
            }
        }

        if (Function->GetFullName() == "Function SafeZoneIndicator.SafeZoneIndicator_C.OnSafeZoneStateChange" && bSafeZoneBased)
        {
            auto Indicator = (ASafeZoneIndicator_C*)Object;
            auto SafeZonePhase = ((AFortGameModeAthena*)GetWorld()->AuthorityGameMode)->SafeZonePhase;
            Indicator->NextCenter = (FVector_NetQuantize100)BusLocation;
            //std::cout << "SafeZonePhase: " << SafeZonePhase << "\n";
            switch (SafeZonePhase)
            {
            case 0:

                Indicator->Radius = 15000;
                Indicator->NextRadius = 450;
                break;
            case 1:
                Indicator->NextRadius = 450;
                break;
            default:
                Indicator->NextRadius = 50;
                break;
            }
            /*
            switch (SafeZonePhase)
            {
            case 0:
                
                Indicator->Radius = 15000;
                Indicator->NextRadius = 11000;
                break;
            case 1:
                Indicator->NextRadius = 7000;
                break;
            case 2:
                Indicator->NextRadius = 4000;
                break;
            case 3:
                Indicator->NextRadius = 1000;
                break;
            case 4:
                Indicator->NextRadius = 500;
                break;
            default:
                GameState->GamePhase = EAthenaGamePhase::EndGame;
                GameState->OnRep_GamePhase(EAthenaGamePhase::SafeZones);
                Indicator->NextRadius = 50;
                break;
            }
            */
        }

        // Loot on death: i dont have time to fix this, it works but it only drops ur consumables
        /*
        if (Function->GetFullName() == "Function FortniteGame.FortPlayerControllerZone.ClientOnPawnDied" && bStartedBus && !bPlayground && !bHideAndSeek)
        {
            auto Params = static_cast<AFortPlayerControllerZone_ClientOnPawnDied_Params*>(Parameters);
            auto DeadPC = (AFortPlayerControllerAthena*)Object;

            for (int i = 0; i < DeadPC->WorldInventory->Inventory.ItemInstances.Num(); i++)
            {
                auto Definition = DeadPC->WorldInventory->Inventory.ItemInstances[i]->GetItemDefinitionBP();
                if (!Definition->GetName().starts_with("WID_") && Definition->Class->GetName() == "FortWeaponRangedItemDefinition")
                {
                    auto Pickup = SummonPickup((AFortPlayerPawn*)DeadPC->Pawn, Definition, 1, DeadPC->Pawn->K2_GetActorLocation());
                    Pickup->PrimaryPickupItemEntry.LoadedAmmo = DeadPC->WorldInventory->Inventory.ItemInstances[i]->ItemEntry.LoadedAmmo;
                    Pickup->OnRep_PrimaryPickupItemEntry();
                }
            }

            if (DeadPC->QuickBars)
                DeadPC->QuickBars->K2_DestroyActor();
        }
        */

        if (Function->GetFullName() == "Function FortniteGame.FortPlayerControllerZone.ClientOnPawnDied" && bStartedBus)
        {
            auto Params = static_cast<AFortPlayerControllerZone_ClientOnPawnDied_Params*>(Parameters);
            auto DeadPC = (AFortPlayerControllerAthena*)Object;
            auto DeadPlayerState = (AFortPlayerStateAthena*)DeadPC->PlayerState;

            if (DeadPC && DeadPC->NetConnection && Params)
            {
                bool IsKiller = Params->DeathReport.KillerPlayerState ? true : false;
                auto KillerPawn = Params->DeathReport.KillerPawn;
                auto KillerPlayerState = (AFortPlayerStateAthena*)Params->DeathReport.KillerPlayerState;

                if (!bPlayground)
                {
                    GameState->PlayersLeft--;
                    GameState->TotalPlayers--;
                    GameState->OnRep_PlayersLeft();
                }
                FVector DeadPawnLocation = DeadPC->Pawn->K2_GetActorLocation();
                auto Drone = SpawnActor(ABP_VictoryDrone_C::StaticClass(), DeadPawnLocation);
                Drone->Owner = DeadPC;
                Drone->OnRep_Owner();
                ((ABP_VictoryDrone_C*)Drone)->PlayerState = IsKiller ? Params->DeathReport.KillerPlayerState : nullptr;
                ((ABP_VictoryDrone_C*)Drone)->OnRep_PlayerState();

                /*
                if (!bPlayground)
                {
                    std::cout << DeadPC->NetPlayerIndex << "Player Index 1\n";
                    for (int i = 0; i < GameState->PlayerArray.Num(); i++)
                    {
                        if (GameState->PlayerArray[i] == DeadPlayerState)
                        {
                            std::cout << i << "Player Index 2\n";
                            GameState->PlayerArray.RemoveAt(i);
                        }
                    }
                    //GameState->PlayerArray.RemoveSingle(DeadPC->NetIndex);
                }
                */

                FDeathInfo deathInfo;
                deathInfo.bDBNO = false;
                deathInfo.DeathLocation = DeadPawnLocation;
                deathInfo.Distance = Params->DeathReport.KillerPawn ? Params->DeathReport.KillerPawn->GetDistanceTo(DeadPC->Pawn) : 0;
                deathInfo.DeathCause = GetDeathReason(Params->DeathReport);
                deathInfo.FinisherOrDowner = KillerPlayerState ? KillerPlayerState : DeadPlayerState;
                DeadPlayerState->DeathInfo = deathInfo;
                DeadPlayerState->OnRep_DeathInfo();

                if (IsKiller && KillerPlayerState != DeadPlayerState)
                {
                    KillerPlayerState->ClientReportKill(DeadPlayerState);
                    KillerPlayerState->KillScore++;
                    KillerPlayerState->TeamKillScore++;
                    KillerPlayerState->OnRep_Kills();
                    UpdateSpecvars(DeadPC->NetConnection, KillerPlayerState);
                    //Spectate(DeadPC->NetConnection, KillerPlayerState);
                }
                else
                {
                    DeadPlayerState->ClientReportKill(DeadPlayerState);
                    if (GameState->PlayersLeft > 0 && !bPlayground)
                    {
                        TArray<AFortPlayerPawn*> Pawns;
                        GetFortKismet()->STATIC_GetAllFortPlayerPawns(GetWorld(), &Pawns);
                        auto RandomPawn = (AFortPlayerPawnAthena*)Pawns[rand() % Pawns.Num()];
                        if (RandomPawn)
                            UpdateSpecvars(DeadPC->NetConnection, (AFortPlayerStateAthena*)RandomPawn->Controller->PlayerState);
                        Pawns.FreeArray();
                        //Spectate(DeadPC->NetConnection, (AFortPlayerStateAthena*)RandomPawn->Controller->PlayerState);
                    }
                }

                // sometimes checking if the killer is the dead player is important
                // if you dont and u die in storm and you were gliding it would loop this (no joke) 2,147,483,647 times, thus causing a stack overflow
                if (bPlayground && ((AFortPlayerPawnAthena*)DeadPC->Pawn)->IsSkydiving() && IsKiller && KillerPlayerState != DeadPC->PlayerState)
                    ((AFortPlayerControllerAthena*)KillerPawn->Controller)->ClientOnPawnDied(Params->DeathReport);

                if (DeadPC->Pawn)
                    DeadPC->Pawn->K2_DestroyActor();

                if (bPlayground && IsKiller)
                    Playground().OnDeath((AFortPlayerPawnAthena*)KillerPawn);

                if (GameState->PlayersLeft == 1)
                {
                    if (!bPlayground)
                    {
                        TArray<AActor*> Pawns;
                        static auto GameplayStatics = (UGameplayStatics*)UGameplayStatics::StaticClass()->CreateDefaultObject();
                        GameplayStatics->STATIC_GetAllActorsOfClass(GetWorld(), APlayerPawn_Athena_C::StaticClass(), &Pawns);
                        auto WinnerPawn = (AFortPlayerPawnAthena*)Pawns[0];
                        Pawns.FreeArray();
                        auto WinnerPC = (AFortPlayerControllerAthena*)WinnerPawn->Controller;
                        auto WinnerState = (AFortPlayerStateAthena*)WinnerPC->PlayerState;
                        if (!WinnerPC->IsClientNotifiedOfWin())
                        {
                            auto GameMode = (AFortGameModeAthena*)GameState->AuthorityGameMode;
                            GameState->WinningPlayerName = WinnerState->PlayerName;
                            GameState->OnRep_WinningPlayerName();
                            GameMode->ReadyToEndMatch();
                            GameMode->EndMatch();

                            reinterpret_cast<UFortCheatManager*>(WinnerPawn->Controller)->AthenaForceVictory(true);
                            WinnerPC->ClientNotifyWon();
                            WinnerPC->ClientNotifyTeamWon();
                            WinnerPC->PlayWinEffects();
                            WinnerPC->bClientNotifiedOfWin = true;
                            WinnerPC->bClientNotifiedOfTeamWin = true;
                            WinnerState->bHasWonAGame = true;
                            WinnerPC->ClientGameEnded(WinnerPawn, true);
                        }
                    }
                    //auto CM = CreateCheatManager((APlayerController*)WinnerPawn->Controller);
                    //CM->AthenaForceVictory(true);
                }
            }
        }

        if (bTraveled)
        {
#ifdef LOGGING
            auto FunctionName = Function->GetName();
            if (Function->FunctionFlags & 0x00200000 || (Function->FunctionFlags & 0x01000000 && FunctionName.find("Ack") == -1 && FunctionName.find("AdjustPos") == -1))
            {
                if (FunctionName.find("ServerUpdateCamera") == -1 && FunctionName.find("ServerMove") == -1)
                {
                    std::cout << "RPC Called: " << FunctionName << '\n';
                }
            }
#endif

            for (int i = 0; i < UFunctionHooks::toHook.size(); i++)
            {
                if (Function == UFunctionHooks::toHook[i])
                {
                    if (UFunctionHooks::toCall[i](Object, Parameters))
                    {
                        return;
                    }
                    break;
                }
            }
        }

        return ProcessEvent(Object, Function, Parameters);
    }
}
