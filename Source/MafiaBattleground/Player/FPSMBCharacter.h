//+--------------------------------------------------------+
//| Project    : MafiaBattleground                         |
//| UE Version : 4.27                                      |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSMBCharacter.generated.h"

UCLASS(config = Game)
class MAFIABATTLEGROUND_API AFPSMBCharacter : public ACharacter
{
    GENERATED_BODY()

    friend class AFPSMBPlayerController;

public:
    //!Constructor
    AFPSMBCharacter();

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED COMPONENTS AND VARIABLES                                      *
    //*******************************************************************************************************************

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* SpringArm;
    /** FPS camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* FPSCamera;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* ArmsMesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* ShadowMesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
    class UFPSMBHealthComponent* HealthComp;

    UPROPERTY(EditDefaultsOnly, Category = Player)
    FName HeadBoneName;
    //UPROPERTY(EditDefaultsOnly, Category = RenderPlayer)
    //FName HipBoneName;

    UPROPERTY(EditDefaultsOnly, Category = Player)
    TSubclassOf<class AWeapon> AK47;
    UPROPERTY(EditDefaultsOnly, Category = Player)
    TSubclassOf<class AWeapon> SARifle;

    UPROPERTY(Replicated)
    TArray<class AWeapon*> Weapons;

    UPROPERTY(Replicated,VisibleAnywhere)
    class AWeapon* CurrentWeapon = nullptr;

    FVector ArmsDefaultLocation;
    UPROPERTY(BlueprintReadOnly)
    FVector CurrentVelocity;
    UPROPERTY(EditDefaultsOnly, Category = Player)
    FVector CrouchSALocation;
    FVector DefaultSALocation;
    UPROPERTY(EditDefaultsOnly, Category = Player)
    FVector FoldWeaponLocation;

    UPROPERTY(Replicated)
    uint8 CurrentWeaponIndex;

    UPROPERTY(EditDefaultsOnly, Category = Player);
    float AimMaxWalkSpeed;
    UPROPERTY(EditDefaultsOnly, Category = Player);
    float CrouchInterpSpeed;
    UPROPERTY(EditDefaultsOnly, Category = Player);
    float DeathImpulse;
    UPROPERTY(EditDefaultsOnly, Category = Player);
    float DeathTime;
    float DefaultFOV;
    UPROPERTY(EditDefaultsOnly, Category = Player);
    float DefaultMaxWalkSpeed;
    float DefaultSpringArmLength;
    UPROPERTY(EditDefaultsOnly, Category = Player)
    float HealLoopTime;
    UPROPERTY(EditDefaultsOnly, Category = Player);
    float RunMaxWalkSpeed;
    UPROPERTY(EditDefaultsOnly, Category = Player)
    float VelocityThresholdX;
    UPROPERTY(EditDefaultsOnly, Category = Player)
    float VelocityThresholdY;

    UPROPERTY(BlueprintReadWrite)
    bool bJumped;
    UPROPERTY(Replicated, BlueprintReadOnly)
    bool bIsAiming;
    UPROPERTY(ReplicatedUsing = OnRep_Died, BlueprintReadOnly, Category = Player)
    bool bIsDead;
    UPROPERTY(Replicated, BlueprintReadOnly)
    bool bIsRuning;
    UPROPERTY(Replicated, BlueprintReadOnly)
    bool bIsReloading;

    FTimerHandle TimerHandle_Heal;

public:
    //*******************************************************************************************************************
    //                                          PUBLIC INLINE FUNCTIONS                                                 *
    //*******************************************************************************************************************

    /* Return true if it is a ListenerServer or false if it is a Client */
    FORCEINLINE bool GetIsServer() { return GetLocalRole() == ROLE_Authority && (GetRemoteRole() == ROLE_SimulatedProxy || GetRemoteRole() == ROLE_AutonomousProxy); };

    UFUNCTION(BlueprintCallable)
    FORCEINLINE USkeletalMeshComponent* GetArmsMesh()               { return ArmsMesh; };
    FORCEINLINE FVector                 GetArmsAimDefaultLocation() { return ArmsDefaultLocation; };
    FORCEINLINE UCameraComponent*       GetCamera()                 { return FPSCamera; };
    FORCEINLINE AWeapon*                GetCurrentWeapon()          { return CurrentWeapon; };
    FORCEINLINE USpringArmComponent*    GetSpringArm()              { return SpringArm; };
    FORCEINLINE bool                    GetIsAiming()               { return bIsAiming; };
    FORCEINLINE bool                    GetIsDead()                 { return bIsDead; };
    FORCEINLINE bool                    GetIsReloading()            { return bIsReloading; };
    FORCEINLINE float                   GetVelocityThresholdX()     { return VelocityThresholdX; };
    FORCEINLINE float                   GetVelocityThresholdY()     { return VelocityThresholdY; };

    //*******************************************************************************************************************
    //                                          PUBLIC FUNCTIONS                                                        *
    //*******************************************************************************************************************

    UFUNCTION(BlueprintImplementableEvent)
    void SetAimingCrosshair(bool bIsAimVal);

    /* Set Is Aiming */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSetAiming(bool bIsAimingVal);

    /* If is not aiming set bIsRuning to bIsRuningVal, SetMaxSpeed and StopFire.
     * Else set bIsRuning to false and SetMaxSpeedd */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSetRun(bool bIsRuningVal);

    /* Return a Normalized Axis from Base Aim Rotation */
    UFUNCTION(BlueprintCallable, BlueprintPure)
    float GetControlPitchRotation();

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED FUNCTIONS                                                     *
    //*******************************************************************************************************************

    UFUNCTION(BlueprintImplementableEvent)
    void OnDeathHUD();

    /* Set bUseControllerRotationYaw to the Aiming Value and set bIsRuning to false */
    UFUNCTION(Client, Reliable, WithValidation)
    void ClientSetAiming(bool bIsAimingVal);

    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    void LoopHeal();

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    void WeaponReload();

    UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
    void ServerWeaponReload(bool ReloadingVal);

    void UpdateCrouch(bool bIsCrouch, float DeltaTime);

    void CheckInitialPlayerRefInController_Delay();
    /* Set the MafiaBattlegroundCharacter reference into the MBGPlayerController */
    UFUNCTION()
    void CheckInitialParametersDelayed();

    void SetReviveText(bool bVisible);

    /* Spawn Default Weapon */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSpawnDefaultWeapon();

    /*Change the equipped weapon to that of the number passed by parameter*/
    void ChangeWeapon(uint8_t WeaponIndex);

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerChangeWeapon(int WeaponIndex);

    /* Update the MaxWalkSpeed */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSetMaxSpeed();
    /* Update the MaxWalkSpeed */
    UFUNCTION(NetMulticast, Reliable, WithValidation)
    void MultiSetMaxSpeed(float MaxVel);

    /* Set value to CurrentVelocity */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSetVelocity();
    /* Set value to CurrentVelocity */
    UFUNCTION(NetMulticast, Reliable, WithValidation)
    void MultiSetVelocity(FVector CurrentVel);

    /* Update BWasJump */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSetJump();
    /* Update BWasJump */
    UFUNCTION(NetMulticast, Reliable, WithValidation)
    void MultiSetJump(bool bJumpValue);

    /* Interpolate the Camera FOV */
    void ZoomInterp(const float DeltaTime);

    UFUNCTION()
    void OnHealthChanged(UFPSMBHealthComponent* HealthComponent, float Health, float HealthDelta,
                         const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

    UFUNCTION()
    void OnRep_Died();

    UFUNCTION(NetMulticast, Reliable, WithValidation)
    void MultiOnDeathMesh(const FVector& DeathDirection);

    /* Update HUD */
    UFUNCTION(Client, Reliable, WithValidation)
    void ClientOnDeath();

    virtual void Destroyed() override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
