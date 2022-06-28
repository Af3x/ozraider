#pragma once

#include "gui.h"
#include "ufunctionhooks.h"

// #define LOGGING

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

    auto newIndex()
    {
        static int Index = GetMath()->STATIC_RandomIntegerInRange(0, 42);
        return Index;
    }

    auto ChooseRandomPickaxeIndex()
    {
        static int Index = Index = rand() % 42;
        if (Index == 42)
            Index -= 2;

        return Index;
    }
    
        auto ChooseRandomAssaultRifle()
        {
            int Index2 = Index2 = rand() % 17;
            if (Index2 == 17)
                Index2 -= 2;

<<<<<<< HEAD
            return Index2;
        }
        auto ChooseRandomShotgun()
        {
            int Index3 = Index3 = rand() % 9;
            if (Index3 == 9)
                Index3 -= 2;

            return Index3;
        }
        auto ChooseRandomSniper()
        {
            int Index4 = Index4 = rand() % 23;
            if (Index4 == 23)
                Index4 -= 2;

            return Index4;
        }
        auto ChooseRandomFourthSlot()
        {
            int Index5 = Index5 = rand() % 5;
            if (Index5 == 5)
                Index5 -= 2;

            return Index5;
        }
        auto ChooseRandomFifthSlot()
        {
            int Index6 = Index6 = rand() % 6;
            if (Index6 == 6)
                Index6 -= 2;

            return Index6;
        }
    
=======
    auto ChooseRandomAssaultRifle()
    {
        int Index2 = Index2 = rand() % 13;
        if (Index2 == 13)
            Index2 -= 2;

        return Index2;
    }
    auto ChooseRandomShotgun()
    {
        int Index3 = Index3 = rand() % 11;
        if (Index3 == 11)
            Index3 -= 2;

        return Index3;
    }
    auto ChooseRandomSniper()
    {
        int Index4 = Index4 = rand() % 21;
        if (Index4 == 21)
            Index4 -= 2;

        return Index4;
    }
    auto ChooseRandomFourthSlot()
    {
        int Index5 = Index5 = rand() % 7;
        if (Index5 == 7)
            Index5 -= 2;

        return Index5;
    }
    auto ChooseRandomFifthSlot()
    {
        int Index6 = Index6 = rand() % 6;
        if (Index6 == 6)
            Index6 -= 2;

        return Index6;
    }
>>>>>>> 1d89125bbead42acaf9efc31038269eb3478d320

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

        constexpr static auto Health = 100;
        const static auto Shield = 100;
        Pawn->SetMaxHealth(Health);
        Pawn->SetMaxShield(Shield);
        Pawn->HealthRegenGameplayEffect = nullptr;
        Pawn->HealthRegenDelayGameplayEffect = nullptr;

        PlayerController->bHasClientFinishedLoading = true; // should we do this on ServerSetClientHasFinishedLoading 
        PlayerController->bHasServerFinishedLoading = true;
        PlayerController->bHasInitiallySpawned = true;
        PlayerController->OnRep_bHasServerFinishedLoading();

        PlayerState->bHasFinishedLoading = true;
        PlayerState->bHasStartedPlaying = true;
        PlayerState->OnRep_bHasStartedPlaying();

        static auto FortRegisteredPlayerInfo = ((UFortGameInstance*)GetWorld()->OwningGameInstance)->RegisteredPlayers[0]; // UObject::FindObject<UFortRegisteredPlayerInfo>("FortRegisteredPlayerInfo Transient.FortEngine_0_1.FortGameInstance_0_1.FortRegisteredPlayerInfo_0_1");

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
        
        std::string PickaxePool[42] = {
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
        std::string AssaultRiflePool[17] = {
            "WID_Assault_Auto_Athena_C_Ore_T03",
            "WID_Assault_Auto_Athena_UC_Ore_T03",
            "WID_Assault_Auto_Athena_R_Ore_T03",
            "WID_Assault_AutoHigh_Athena_VR_Ore_T03",
            "WID_Assault_AutoHigh_Athena_SR_Ore_T03",
            "WID_Assault_SemiAuto_Athena_C_Ore_T02",
            "WID_Assault_SemiAuto_Athena_R_Ore_T03",
            "WID_Assault_SemiAuto_Athena_UC_Ore_T03",
            "WID_Pistol_AutoHeavy_Athena_C_Ore_T02",
            "WID_Pistol_AutoHeavy_Athena_R_Ore_T03",
            "WID_Pistol_AutoHeavy_Athena_UC_Ore_T03",
            "WID_Pistol_AutoHeavySuppressed_Athena_C_Ore_T02",
            "WID_Pistol_AutoHeavySuppressed_Athena_R_Ore_T03",
            "WID_Pistol_AutoHeavySuppressed_Athena_UC_Ore_T03",
            "WID_Pistol_Scavenger_Athena_R_Ore_T03",
            "WID_Pistol_Scavenger_Athena_UC_Ore_T03",
            "WID_Pistol_Scavenger_Athena_VR_Ore_T03"
            
        };
        std::string ShotgunPool[9] = {
            "WID_Shotgun_BreakBarrel_Athena_SR_Ore_T03",
            "WID_Shotgun_BreakBarrel_Athena_VR_Ore_T03",
            "WID_Shotgun_SemiAuto_Athena_R_Ore_T03",
            "WID_Shotgun_SemiAuto_Athena_UC_Ore_T03",
            "WID_Shotgun_SemiAuto_Athena_VR_Ore_T03",
            "WID_Shotgun_SlugFire_Athena_SR",
            "WID_Shotgun_SlugFire_Athena_VR",
            "WID_Shotgun_Standard_Athena_C_Ore_T03",
            "WID_Shotgun_Standard_Athena_UC_Ore_T03"
        };
        std::string SniperPool[23] = {
            "WID_Sniper_BoltAction_Scope_Athena_R_Ore_T03",
            "WID_Sniper_BoltAction_Scope_Athena_SR_Ore_T03",
            "WID_Sniper_BoltAction_Scope_Athena_VR_Ore_T03",
            "WID_Sniper_Crossbow_Athena_R_Ore_T03",
            "WID_Sniper_Crossbow_Athena_VR_Ore_T03",
            "WID_Sniper_NoScope_Athena_R_Ore_T03",
            "WID_Sniper_NoScope_Athena_UC_Ore_T03",
            "WID_Sniper_Standard_Scope_Athena_SR_Ore_T03",
            "WID_Sniper_Standard_Scope_Athena_VR_Ore_T03",
            "WID_Pistol_SixShooter_Athena_C_Ore_T02",
            "WID_Pistol_SixShooter_Athena_R_Ore_T03",
            "WID_Pistol_SixShooter_Athena_UC_Ore_T03",
            "WID_RC_Rocket_Athena_SR_T03",
            "WID_RC_Rocket_Athena_VR_T03",
            "WID_Assault_LMG_Athena_SR_Ore_T03",
            "WID_Assault_LMG_Athena_VR_Ore_T03",
            "WID_Assault_LMGSAW_Athena_R_Ore_T03",
            "WID_Assault_LMGSAW_Athena_VR_Ore_T03",
            "WID_Assault_AutoM4A1_Athena_VR_Ore_T04",
            "WID_Assault_Surgical_Athena_R_Ore_T03",
            "WID_Assault_Surgical_Athena_VR_Ore_T03",
            "WID_Pistol_HandCannon_Athena_SR_Ore_T03",
            "WID_Pistol_HandCannon_Athena_VR_Ore_T03"
        };
        std::string FourthSlotPool[5] = {
            "Athena_Bush",
            "Athena_DanceGrenade",
            "Athena_GasGrenade",
            "Athena_KnockGrenade",
            "Athena_StickyGrenade"

        };
        std::string FifthSlotPool[6] = {
            "Athena_PurpleStuff",
            "Athena_Medkit",
            "Athena_Shields",
            "Athena_ShieldSmall",
            "Athena_Bandage",
            "Athena_SuperMedkit"
        };

        int Index = ChooseRandomPickaxeIndex();
        std::cout << Index << ": Pickaxe Index\n";

        int Index2 = ChooseRandomAssaultRifle();
        std::cout << Index2 << ": Assault Rifle Index\n";

        int Index3 = ChooseRandomShotgun();
        std::cout << Index3 << ": Shotgun Index\n";

        int Index4 = ChooseRandomSniper();
        std::cout << Index4 << ": Sniper Index\n";

        int Index5 = ChooseRandomFourthSlot();
        std::cout << Index5 << ": Fourth Slot Index\n";

        int Index6 = ChooseRandomFifthSlot();
        std::cout << Index6 << ": Fifth Slot Index\n";
        

        

        UFortWeaponRangedItemDefinition* Pickaxe = bCosmetics ? FindWID(PickaxePool[Index + 1]) : FindWID("WID_Harvest_Pickaxe_Athena_C_T01");
<<<<<<< HEAD
        
=======
>>>>>>> 1d89125bbead42acaf9efc31038269eb3478d320
        UFortWeaponRangedItemDefinition* RAssaultRifle = FindWID(AssaultRiflePool[Index2 + 1]);
        UFortWeaponRangedItemDefinition* RShotgun = FindWID(ShotgunPool[Index3 + 1]);
        UFortWeaponRangedItemDefinition* RSniper = FindWID(SniperPool[Index4 + 1]);
        UFortWeaponRangedItemDefinition* RFourSlot = FindWID(FourthSlotPool[Index5 + 1]);
        UFortWeaponRangedItemDefinition* RFiveSlot = FindWID(FifthSlotPool[Index6 + 1]);
<<<<<<< HEAD
        
=======
>>>>>>> 1d89125bbead42acaf9efc31038269eb3478d320

        switch (loadoutToUse)
        {
            case WeaponLoadout::REGULAR:
            {
                static std::vector<UFortWeaponRangedItemDefinition*> FortLoadout = 
                {
                    Pickaxe,
                    FindWID("WID_Shotgun_Standard_Athena_UC_Ore_T03"),
                    FindWID("WID_Pistol_AutoHeavy_Athena_R_Ore_T03"),
                    //FindWID("WID_Pistol_Scavenger_Athena_R_Ore_T03"),
                    FindWID("WID_Assault_AutoHigh_Athena_SR_Ore_T03"),
                    FindWID("Athena_KnockGrenade"),
                    FindWID("Athena_PurpleStuff")
                };
                EquipLoadout(PlayerController, FortLoadout);
                break;
            }
            case WeaponLoadout::EXPLOSIVES:
            {
                static std::vector<UFortWeaponRangedItemDefinition*> FortLoadout = 
                {
                    Pickaxe,
                    FindWID("WID_Launcher_Rocket_Athena_R_Ore_T03"),
                    FindWID("WID_Launcher_Rocket_Athena_R_Ore_T03"),
                    FindWID("WID_Launcher_Grenade_Athena_SR_Ore_T03"),
                    FindWID("Athena_C4"),
                    FindWID("Athena_PurpleStuff")
                };
                EquipLoadout(PlayerController, FortLoadout);
                break;
            }
            case WeaponLoadout::SNIPERS:
            {
                static std::vector<UFortWeaponRangedItemDefinition*> FortLoadout = 
                {
                    Pickaxe,
                    FindWID("WID_Sniper_BoltAction_Scope_Athena_SR_Ore_T03"),
                    FindWID("WID_Sniper_Standard_Scope_Athena_SR_Ore_T03"),
                    FindWID("WID_Sniper_NoScope_Athena_R_Ore_T03"),
                    FindWID("WID_Sniper_Crossbow_Athena_VR_Ore_T03"),
                    FindWID("Athena_PurpleStuff")
                };
                EquipLoadout(PlayerController, FortLoadout);
                break;
            }
            case WeaponLoadout::RANDOM:
            {
                std::vector<UFortWeaponRangedItemDefinition*> FortLoadout = {
                    Pickaxe,
                    RAssaultRifle,
                    RShotgun,
                    RSniper,
                    RFourSlot,
                    RFiveSlot,
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
                PlayerState->TeamIndex = EFortTeam(rand() % 101);
                PlayerState->OnRep_PlayerTeam();
            }
        }

        PlayerController->OverriddenBackpackSize = 100;

        if (Pawn && Pawn->AbilitySystemComponent)
        {
            ApplyAbilities(Pawn);
        }

        auto Drone = (ABP_VictoryDrone_C*)SpawnActor(ABP_VictoryDrone_C::StaticClass(), Pawn->K2_GetActorLocation());
        Drone->TriggerPlayerSpawnEffects();

        return PlayerController;
    }

    void Beacon_NotifyControlMessage(AOnlineBeaconHost* Beacon, UNetConnection* Connection, uint8 MessageType, int64* Bunch)
    {

        if (reinterpret_cast<AAthena_GameState_C*>(GetWorld()->GameState)->GamePhase != EAthenaGamePhase::Warmup)
        {
            KickController(Connection->PlayerController, L"Game has already started.");
            //Connection->PlayerController
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

    UNetConnection* IAmTheOneWhoSpectates;
    AFortPlayerStateAthena* ToSpectatePlayerState;

    auto UpdateSpecvars(UNetConnection* TWTS, AFortPlayerStateAthena* TWSP)
    {
        IAmTheOneWhoSpectates = TWTS;
        ToSpectatePlayerState = TWSP;
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
            }
        }

        if (Function->GetFullName() == "Function BP_VictoryDrone.BP_VictoryDrone_C.OnSpawnOutAnimEnded")
        {
            if (!bPlayground)
            {
                auto drone = (ABP_VictoryDrone_C*)Object;
                if (Object->IsA(ABP_VictoryDrone_C::StaticClass()))
                    drone->K2_DestroyActor();

                if (IAmTheOneWhoSpectates && ToSpectatePlayerState)
                    Spectate(IAmTheOneWhoSpectates, ToSpectatePlayerState);
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
        
        if (Function->GetFullName() == "Function FortniteGame.FortPlayerControllerZone.ClientOnPawnDied")
        {
            auto Params = static_cast<AFortPlayerControllerZone_ClientOnPawnDied_Params*>(Parameters);
            auto DeadPC = (AFortPlayerControllerAthena*)Object;
            auto DeadPlayerState = (AFortPlayerStateAthena*)DeadPC->PlayerState;
            bool IsKiller;

            if (DeadPC && Params)
            {
                //TArray<AActor*> Pawns;
                //static auto GameplayStatics = (UGameplayStatics*)UGameplayStatics::StaticClass()->CreateDefaultObject();
                //GameplayStatics->STATIC_GetAllActorsOfClass(GetWorld(), APlayerPawn_Athena_C::StaticClass(), &Pawns);
                //auto GameState = (AAthena_GameState_C*)GetWorld()->AuthorityGameMode->GameState;
                IsKiller = Params->DeathReport.KillerPlayerState ? true : false;
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

                if (!bPlayground)
                    GameState->PlayerArray.RemoveSingle(DeadPC->NetPlayerIndex);

                FDeathInfo deathInfo;
                deathInfo.bDBNO = false;
                deathInfo.DeathLocation = DeadPawnLocation;
                deathInfo.Distance = Params->DeathReport.KillerPawn ? Params->DeathReport.KillerPawn->GetDistanceTo(DeadPC->Pawn) : 0;
                deathInfo.DeathCause = KillerPlayerState ? EDeathCause::Sniper : EDeathCause::FallDamage; // TODO: Determine what the actual death cause was.
                deathInfo.FinisherOrDowner = KillerPlayerState ? KillerPlayerState : DeadPlayerState;
                DeadPlayerState->DeathInfo = deathInfo;
                DeadPlayerState->OnRep_DeathInfo();

                if (IsKiller)
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
                    deathInfo.DeathCause = EDeathCause::FallDamage;
                    deathInfo.FinisherOrDowner = DeadPlayerState;
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
                auto DeadPawn = (AFortPlayerPawnAthena*)DeadPC->Pawn;
                if (DeadPawn->IsSkydiving() && IsKiller && bPlayground)
                {
                    static_cast<AFortPlayerControllerAthena*>(KillerPawn->Controller)->ClientOnPawnDied(Params->DeathReport);
                }

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
