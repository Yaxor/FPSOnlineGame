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

    UFUNCTION(BlueprintImplementableEvent)
    void CreateInGameMenu();
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
    void ToggleInGameMenu();

private:
    //*******************************************************************************************************************
    //                                          PRIVATE FUNCTIONS                                                       *
    //*******************************************************************************************************************

    virtual void BeginPlay();
};
