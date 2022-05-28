//+--------------------------------------------------------+
//| Project    : MafiaBattleground                         |
//| UE Version : 4.27                                      |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+

#include "MafiaBattlegroundGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

//-------------------------------------------------------------------------------------------------
AMafiaBattlegroundGameMode::AMafiaBattlegroundGameMode()
{
}

//-------------------------------------------------------------------------------------------------
void AMafiaBattlegroundGameMode::BeginPlay()
{
    Super::BeginPlay();

    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
}

//-------------------------------------------------------------------------------------------------
void AMafiaBattlegroundGameMode::RespawnPlayer(AController* PController)
{
    if (PlayerStarts.Num() > 0)
    {
        // Get a Random PlayerStart
        const uint8 PSRange    = PlayerStarts.Num() - 1;
        const uint8 PSRanIndex = FMath::RandRange(0, PSRange);
        AActor* PlayerStart    = PlayerStarts[PSRanIndex];

        // Respawn Player at Random PlayerStart
        RestartPlayerAtPlayerStart(PController, PlayerStart);
    }
}
