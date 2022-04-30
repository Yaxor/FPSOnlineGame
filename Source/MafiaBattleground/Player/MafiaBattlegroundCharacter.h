//+--------------------------------------------------------+
//| Project    : MafiaBattleground                         |
//| UE Version : 4.27                                      |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MafiaBattlegroundCharacter.generated.h"

UCLASS(config=Game)
class AMafiaBattlegroundCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    //!Constructor
    AMafiaBattlegroundCharacter();

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED COMPONENTS AND VARIABLES                                      *
    //*******************************************************************************************************************

    /** Camera boom positioning the camera behind the character */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* SpringArm;

    /** Follow camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* FollowCamera;

    UPROPERTY(EditDefaultsOnly, Category = Player)
    TSubclassOf<class AWeapon> DefaultWeapon;

    UPROPERTY(Replicated)
    class AWeapon* CurrentWeapon = nullptr;

    UPROPERTY(BlueprintReadOnly)
    FVector CurrentVelocity;

    float DefaultFOV;
    UPROPERTY(EditDefaultsOnly, Category = Player);
    float DefaultMaxWalkSpeed;
    UPROPERTY(EditDefaultsOnly, Category = Player);
    float AimMaxWalkSpeed;
    UPROPERTY(EditDefaultsOnly, Category = Player);
    float RunMaxWalkSpeed;

    UPROPERTY(BlueprintReadWrite)
    bool bJumped;
    UPROPERTY(Replicated)
    bool bIsDead;
    UPROPERTY(Replicated, BlueprintReadOnly)
    bool bIsRuning;
    UPROPERTY(Replicated, BlueprintReadOnly)
    bool bIsAiming;

public:
    //*******************************************************************************************************************
    //                                          PUBLIC INLINE FUNCTIONS                                                 *
    //*******************************************************************************************************************

    /* Return true if it is a ListenerServer or false if it is a Client */
    FORCEINLINE bool GetIsServer() { return GetLocalRole() == ROLE_Authority && (GetRemoteRole() == ROLE_SimulatedProxy || GetRemoteRole() == ROLE_AutonomousProxy); };

    FORCEINLINE bool GetIsDead() { return bIsDead; };

    //*******************************************************************************************************************
    //                                          PUBLIC FUNCTIONS                                                        *
    //*******************************************************************************************************************

    UFUNCTION(BlueprintImplementableEvent)
    void SetAimingCrosshair(bool bIsAimVal);

    /* Set Is Aiming */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSetAiming(bool bIsAimingVal);
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSetRun(bool bIsRuningVal);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    float GetControlPitchRotation();

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED FUNCTIONS                                                     *
    //*******************************************************************************************************************

    /* Set bUseControllerRotationYaw to the Aiming Value */
    UFUNCTION(NetMulticast, Reliable, WithValidation)
    void MultiSetAiming(bool bIsAimingVal);

    virtual void BeginPlay();

    virtual void Tick(float DeltaTime) override;

    void CheckInitialPlayerRefInController();
    /* Set the MafiaBattlegroundCharacter reference into the MBGPlayerController */
    UFUNCTION()
    void SetPlayerRefToController_Delay();

    /* Spawn Default Weapon */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSpawnDefaultWeapon();

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSetMaxSpeed();
    UFUNCTION(NetMulticast, Reliable, WithValidation)
    void MultiSetMaxSpeed(float MaxVel);

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSetVelocity();
    UFUNCTION(NetMulticast, Reliable, WithValidation)
    void MultiSetVelocity(FVector CurrentVel);

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSetJump();
    UFUNCTION(NetMulticast, Reliable, WithValidation)
    void MultiSetJump(bool bJumpValue);

    /* Interpolate the Camera FOV */
    void ZoomInterp(const float DeltaTime);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

