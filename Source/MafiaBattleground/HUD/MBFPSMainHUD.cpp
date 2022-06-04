//+--------------------------------------------------------+
//| Project    : MafiaBattleground                         |
//| UE Version : 4.27                                      |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+


#include "MBFPSMainHUD.h"

#include "Kismet/KismetSystemLibrary.h"

#include "MafiaBattleground/Player/FPSMBPlayerController.h"

//------------------------------------------------------------------------------------------------------------------------------------------
AMBFPSMainHUD::AMBFPSMainHUD()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AMBFPSMainHUD::BeginPlay()
{
    Super::BeginPlay();

    CreateMainHUD();
    CreateInGameMenu();

    /*if (AFPSMBPlayerController* FPSMBPlayerController = Cast<AFPSMBPlayerController>(GetOwningPlayerController()))
    {
        if (FPSMBPlayerController->IsLocalController())
        {
            CreateMainHUD();
            CreateInGameMenu();
        }
    }*/
}
