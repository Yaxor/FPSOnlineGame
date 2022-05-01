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

    bool bHasTriggered;

public:
    //*******************************************************************************************************************
    //                                          PUBLIC FUNCTIONS                                                        *
    //*******************************************************************************************************************

    virtual void StartFire() override;
    virtual void StopFire() override;
    virtual void Fire() override;

};
