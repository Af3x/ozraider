#pragma once

#include "gui.h"
#include "ue4.h"
#include "game.h"
#include "framework.h"

class Playground
{
public:
    void InitializePlayground(UFortPlaylistAthena* CurrentPlaylist, AAthena_GameState_C* GameState)
    {
        auto GameMode = reinterpret_cast<AFortGameModeAthena*>(GetWorld()->AuthorityGameMode);
        auto Aircraft = GameState->GetAircraft(0);
        if (Aircraft)
        {
            GameState->SafeZonePhase = 5;
            //GameState->SafeZonesStartTime = 2;
            //GameMode->SafeZonePhase = 5;
            
        }
        
        

        auto Playlist = CurrentPlaylist;

        if (Playlist)
        {
            Playlist->bNoDBNO = true;
            Playlist->FriendlyFireType = EFriendlyFireType::On;
            Playlist->RespawnType = EAthenaRespawnType::InfiniteRespawn;
            Playlist->RespawnLocation = EAthenaRespawnLocation::Air;

            GameState->CurrentPlaylistData = Playlist;
            GameState->OnRep_CurrentPlaylistData();
        }
    }

    void DroneAnimEnded(AFortPlayerControllerAthena* PC, FVector NewLocation)
    {
        NewLocation.Z += 4000;

        if (PC->Pawn)
            PC->Pawn->K2_DestroyActor();

        InitPawn(PC, NewLocation, FQuat(), false);

        auto CM = (UFortCheatManager*)PC->CheatManager;
        CM->AllowRespawn();
        PC->RespawnPlayerAfterDeath();
        CM->RespawnPlayer();
        CM->RespawnPlayerServer();
    }

    void OnDeath(AFortPlayerPawnAthena* KillerPawn)
    {
        if (KillerPawn && !KillerPawn->IsDead())
        {
            int HealthToGive = 100;
            if (KillerPawn->GetHealth() > 100)
            {
                HealthToGive = KillerPawn->GetMaxHealth() - KillerPawn->GetHealth();
            }

            KillerPawn->SetHealth(KillerPawn->GetHealth() + HealthToGive);
        }
    }
};