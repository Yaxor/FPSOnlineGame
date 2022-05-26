//+--------------------------------------------------------+
//| Project    : MafiaBattleground                         |
//| UE Version : 4.27                                      |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+


#include "MBFPSMainHUD.h"

#include "MafiaBattleground/Player/FPSMBPlayerController.h"

//------------------------------------------------------------------------------------------------------------------------------------------
AMBFPSMainHUD::AMBFPSMainHUD()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AMBFPSMainHUD::BeginPlay()
{
    Super::BeginPlay();

    AFPSMBPlayerController* FPSMBPlayerController = Cast<AFPSMBPlayerController>(GetOwningPlayerController());
    if (FPSMBPlayerController)
    {
        if (FPSMBPlayerController->IsLocalController())
        {
            CreateMainHUD();
        }
    }
}
