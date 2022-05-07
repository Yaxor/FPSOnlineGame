// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class MAFIABATTLEGROUND_API AShotgun : public AWeapon
{
    GENERATED_BODY()

public:
    //!Constructor
    AShotgun();

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED VARIABLES                                                     *
    //*******************************************************************************************************************

    UPROPERTY(EditDefaultsOnly, Category = Weapon)
    uint8 Pellets;

    UPROPERTY(EditDefaultsOnly, Category = Weapon)
    float AimSpreadBoost;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
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

    virtual void CustomWeaponRecoil() override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
