//+--------------------------------------------------------+
//| Project    : MafiaBattleground                         |
//| UE Version : 4.27                                      |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UCLASS()
class MAFIABATTLEGROUND_API AWeapon : public AActor
{
    GENERATED_BODY()

public:
    //!Constructor
    AWeapon();

    UPROPERTY()
    class AFPSMBCharacter* MyPlayer = nullptr;

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED COMPONENTS AND VARIABLES                                      *
    //*******************************************************************************************************************

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* GunMesh;

    UPROPERTY(EditDefaultsOnly, Category = Weapon)
    TSubclassOf<class UCameraShakeBase> FireCamShake;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
    TSubclassOf<UDamageType> DamageType;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
    UParticleSystem* DefaultImpactVFX;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
    UParticleSystem* FleshImpactVFX;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
    UParticleSystem* MuzzleVFX;

    UPROPERTY(EditDefaultsOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    FName WeaponSocket;
    UPROPERTY()
    FName MuzzleSocketName;

    FTimerHandle TimerHandle_Cadence;

    UPROPERTY(Replicated)
    uint8 CurrentAmmo;
    UPROPERTY(EditDefaultsOnly, Category = Weapon)
    uint8 MaxAmmo;

    UPROPERTY(EditDefaultsOnly, Category = Weapon)
    float AimFOV;
    UPROPERTY(EditDefaultsOnly, Category = Weapon)
    float AimInterSpeedAim;
    UPROPERTY(EditDefaultsOnly, Category = Weapon)
    float BaseDamage;
    UPROPERTY(EditDefaultsOnly, Category = Weapon, meta = (ClampMin = 0.0f))
    float BulletSpread;
    float Cadence;
    UPROPERTY(EditDefaultsOnly, Category = Weapon)
    float FireRate;
    float LastFireTime;
    UPROPERTY(EditDefaultsOnly, Category = Weapon)
    float ShotDistance;
    UPROPERTY(EditDefaultsOnly, Category = Weapon)
    float HeadshotMultiplier;

public:
    //*******************************************************************************************************************
    //                                          PUBLIC INLINE FUNCTIONS                                                 *
    //*******************************************************************************************************************

    /* Return true if it is a ListenerServer or false if it is a Client */
    FORCEINLINE bool GetIsServer() { return GetLocalRole() == ROLE_Authority && (GetRemoteRole() == ROLE_SimulatedProxy || GetRemoteRole() == ROLE_AutonomousProxy); };

    FORCEINLINE USkeletalMeshComponent* GetGunMesh() { return GunMesh; };
    FORCEINLINE float GetWeaponAimFOV()              { return AimFOV; };
    FORCEINLINE float GetWeaponInterpSpeedAim()      { return AimInterSpeedAim; };

    //*******************************************************************************************************************
    //                                          PUBLIC FUNCTIONS                                                        *
    //*******************************************************************************************************************

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerGiveToPayer(class ACharacter* Player);

    void Reload();

    virtual void StartFire();

    void StopFire();

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED FUNCTIONS                                                     *
    //*******************************************************************************************************************

    virtual void BeginPlay() override;

    /* Trace the world, from pawn eyes to creosshair location */
    virtual void Fire();

    void PlayImpactFX(EPhysicalSurface SurfaceType, FVector ImpactPoint);

    void PlayFireFX();

    UFUNCTION(NetMulticast, Reliable, WithValidation)
    void MultiPlayImpactFX(EPhysicalSurface SurfaceType, FVector ImpactPoint);
    UFUNCTION(NetMulticast, Reliable, WithValidation)
    void MultiPlayFireFX();

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerFire();
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerReload();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
