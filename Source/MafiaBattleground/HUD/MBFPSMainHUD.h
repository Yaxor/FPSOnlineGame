//+--------------------------------------------------------+
//| Project    : MafiaBattleground                         |
//| UE Version : 4.27                                      |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MBFPSMainHUD.generated.h"

UCLASS()
class MAFIABATTLEGROUND_API AMBFPSMainHUD : public AHUD
{
    GENERATED_BODY()

public:
    //!Constructor
    AMBFPSMainHUD();

    //*******************************************************************************************************************
    //                                          PUBLIC FUNCTIONS                                                        *
    //*******************************************************************************************************************

    UFUNCTION(BlueprintImplementableEvent)
    void CreateMainHUD();

private:
    //*******************************************************************************************************************
    //                                          PRIVATE FUNCTIONS                                                       *
    //*******************************************************************************************************************

    virtual void BeginPlay();
};
