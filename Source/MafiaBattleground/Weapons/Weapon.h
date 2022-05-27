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

    friend class ASemiautomaticWeapon;
    friend class AAutomaticWeapon;
    friend class AShotgun;

public:
    //!Constructor
    AWeapon();

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED COMPONENTS AND VARIABLES                                      *
    //*******************************************************************************************************************

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* GunMesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* ClientsGunMesh;

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

    UPROPERTY()
    FName AimShotSocket;
    UPROPERTY(EditDefaultsOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    FName ClientsWeaponSocket;
    UPROPERTY()
    FName MuzzleSocketName;
    UPROPERTY()
    FName MuzzleSocketNameOthers;
    UPROPERTY(EditDefaultsOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    FName WeaponSocket;

    FTimerHandle TimerHandle_Fire;

    UPROPERTY(EditDefaultsOnly, Category = Weapon)
    FVector ArmsAimLocation;

    UPROPERTY(Replicated)
    uint8 CurrentAmmo;
    UPROPERTY(EditDefaultsOnly, Category = Weapon)
    uint8 MaxAmmo;
    /* Using for play shot FX */
    UPROPERTY(ReplicatedUsing = OnRep_OthersPlayFireFX)
    uint8 ShotsCounterFireFX;

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
    UPROPERTY(EditDefaultsOnly, Category = Weapon)
    float RecoilForce;

public:
    //*******************************************************************************************************************
    //                                          PUBLIC INLINE FUNCTIONS                                                 *
    //*******************************************************************************************************************

    /* Return true if it is a ListenerServer or false if it is a Client */
    FORCEINLINE bool GetIsServer() { return GetLocalRole() == ROLE_Authority && (GetRemoteRole() == ROLE_SimulatedProxy || GetRemoteRole() == ROLE_AutonomousProxy); };

    FORCEINLINE USkeletalMeshComponent* GetGunMesh()              { return GunMesh; };
    FORCEINLINE USkeletalMeshComponent* GetClientsGunMesh()       { return ClientsGunMesh; };
    FORCEINLINE FName                   GetWeaponSocket()         { return WeaponSocket; };
    FORCEINLINE float                   GetWeaponAimFOV()         { return AimFOV; };
    FORCEINLINE float                   GetWeaponInterpSpeedAim() { return AimInterSpeedAim; };
    FORCEINLINE float                   GetCanReload()            { return CurrentAmmo != MaxAmmo; };

    //*******************************************************************************************************************
    //                                          PUBLIC FUNCTIONS                                                        *
    //*******************************************************************************************************************

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerGiveToPayer(class ACharacter* Player);

    UFUNCTION(NetMulticast, Reliable, WithValidation)
    virtual void MultiAim(bool bAimingVal);

    /* Stop Fire and refill CurrentAmmo */
    void Reload();

    /**/
    virtual void StartFire();

    /* Call ClientStopFire() */
    virtual void StopFire();

    /* Trace the world, from pawn eyes to crosshair location */
    virtual void Fire();

    /* Set life span */
    void OnDeath();

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED FUNCTIONS                                                     *
    //*******************************************************************************************************************

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintImplementableEvent)
    void WeaponRecoil();
    UFUNCTION(BlueprintImplementableEvent)
    void StopWeaponRecoil();

    /* Stop the Fire Timer */
    UFUNCTION(Client, Reliable, WithValidation)
    void ClientStopFire();

    /* Call Fire() */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerFire();

    void WeaponRecoil_Delay();

    UFUNCTION(Client, Reliable, WithValidation)
    void ClientWeaponRecoil();

    virtual void CustomWeaponRecoil();

    /* Call MultiPlayImpactFX() */
    void PlayImpactFX(EPhysicalSurface SurfaceType, FVector ImpactPoint);

    /* Play Impact FX in All Clients */
    UFUNCTION(NetMulticast, Reliable, WithValidation)
    void MultiPlayImpactFX(EPhysicalSurface SurfaceType, FVector ImpactPoint);

    /* Call ClientPlayFireFX() */
    void PlayFireFX();

    /* Play Shot FX in Client & if is not the server call ServerPlayFireFX */
    UFUNCTION(Client, Reliable, WithValidation)
    void ClientPlayFireFX();

    /* Play Fire FX in the ClientsWeaponMesh */
    void OthersPlayersFireFX();

    /* Call OthersPlayersFireFX */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerPlayFireFX();

    /* Call OthersPlayersFireFX in the others clients, SkipOwner */
    UFUNCTION()
    void OnRep_OthersPlayFireFX();

    /* Call Reload(); */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerReload();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
