//+--------------------------------------------------------+
//| Project    : MafiaBattleground                         |
//| UE Version : 4.27                                      |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "SemiautomaticWeapon.generated.h"

/**
 * 
 */
UCLASS()
class MAFIABATTLEGROUND_API ASemiautomaticWeapon : public AWeapon
{
    GENERATED_BODY()

public:
    //!Constructor
    ASemiautomaticWeapon();

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED VARIABLES                                                     *
    //*******************************************************************************************************************

    UPROPERTY(Replicated)
    bool bHasTriggered;

    FTimerHandle TimerHandle_ResetTrigger;

public:
    //*******************************************************************************************************************
    //                                          PUBLIC FUNCTIONS                                                        *
    //*******************************************************************************************************************

    virtual void StartFire() override;
    virtual void Fire() override;

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED FUNCTIONS                                                     *
    //*******************************************************************************************************************

    void ResetTrigger();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
