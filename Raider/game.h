#pragma once

#include "ue4.h"
#include "gui.h"

namespace Game
{
    void Start()
    {
        GetKismetSystem()->STATIC_ExecuteConsoleCommand(GetWorld(), L"open athena_terrain", GetPlayerController());
        //GetPlayerController()->SwitchLevel(L"Athena_Terrain?game=/Game/Athena/Athena_GameMode.Athena_GameMode_C");
        bTraveled = true;
    }

    void OnReadyToStartMatch()
    {
        GameState = reinterpret_cast<AAthena_GameState_C*>(GetWorld()->GameState);
        auto GameMode = reinterpret_cast<AFortGameModeAthena*>(GetWorld()->AuthorityGameMode);
        auto GameInstance = reinterpret_cast<UFortGameInstance*>(GetWorld()->OwningGameInstance);
        SoloPlaylist = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_DefaultSolo.Playlist_DefaultSolo");
        static auto DuoPlaylist = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_DefaultDuo.Playlist_DefaultDuo");
        auto InProgress = GetKismetString()->STATIC_Conv_StringToName(L"InProgress");

        GameState->bGameModeWillSkipAircraft = true;
        GameState->AircraftStartTime = 9999.9f;
        GameState->WarmupCountdownEndTime = 99999.9f;

        GameState->GamePhase = EAthenaGamePhase::Warmup;
        GameState->OnRep_GamePhase(EAthenaGamePhase::None);

        GameMode->bDisableGCOnServerDuringMatch = true;
        GameMode->bAllowSpectateAfterDeath = true;
        GameMode->bEnableReplicationGraph = true;

        GameMode->MatchState = InProgress;
        GameMode->K2_OnSetMatchState(InProgress);

        auto Playlist = SoloPlaylist;

        if (Playlist)
        {
            Playlist->bNoDBNO = true;
            Playlist->FriendlyFireType = EFriendlyFireType::On;
            Playlist->RespawnLocation = EAthenaRespawnLocation::Air;
            Playlist->RespawnType = EAthenaRespawnType::None;
            Playlist->SafeZoneStartUp = ESafeZoneStartUp::StartsWithAirCraft;

            GameState->CurrentPlaylistData = Playlist;
            GameState->OnRep_CurrentPlaylistData();
        }

        GameMode->FriendlyFireType = EFriendlyFireType::Off;

        if (!Looting::bInitialized)
            Looting::Init();

        GameMode->StartPlay();
        //GameState->DefaultBattleBus = UObject::FindObject<UAthenaBattleBusItemDefinition>("AthenaBattleBusItemDefinition BBID_PurpleBus.BBID_PurpleBus");
        GameState->bReplicatedHasBegunPlay = true;
        GameState->OnRep_ReplicatedHasBegunPlay();

        GameMode->StartMatch();
        GameMode->bAlwaysDBNO = false;

    }

    auto GetDeathReason(FFortPlayerDeathReport DeathReport)
    {
        static std::map<std::string, EDeathCause> DeathReasonMap {
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
                if (nameTag == Map.first)
                    return Map.second;
                else
                    continue;
            }
        }

        return EDeathCause::Unspecified;
    }
}