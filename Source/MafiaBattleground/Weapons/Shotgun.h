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

    bool bHasTriggered;

public:
    //*******************************************************************************************************************
    //                                          PUBLIC FUNCTIONS                                                        *
    //*******************************************************************************************************************

    virtual void StartFire() override;
    virtual void StopFire() override;
    virtual void Fire() override;
};
