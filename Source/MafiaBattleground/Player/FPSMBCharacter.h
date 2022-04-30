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

public:
    //!Constructor
    AFPSMBCharacter();

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED COMPONENTS AND VARIABLES                                      *
    //*******************************************************************************************************************

    /** FPS camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* FPSCamera;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* SpringArm;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* ArmsMesh;

    UPROPERTY(EditDefaultsOnly, Category = Player)
    TSubclassOf<class AWeapon> DefaultWeapon;

    UPROPERTY(Replicated,VisibleAnywhere)
    class AWeapon* CurrentWeapon = nullptr;

    FVector ArmsAimLocation;
    FVector ArmsDefaultLocation;
    UPROPERTY(BlueprintReadOnly)
    FVector CurrentVelocity;
    FVector DefaultSALocation;
    UPROPERTY(EditDefaultsOnly, Category = Player)
    FVector CrouchSALocation;

    float DefaultFOV;
    UPROPERTY(EditDefaultsOnly, Category = Player);
    float DefaultMaxWalkSpeed;
    UPROPERTY(EditDefaultsOnly, Category = Player);
    float AimMaxWalkSpeed;
    UPROPERTY(EditDefaultsOnly, Category = Player);
    float RunMaxWalkSpeed;
    UPROPERTY(EditDefaultsOnly, Category = Player);
    float CrouchInterpSpeed;

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

    FORCEINLINE USkeletalMeshComponent* GetArmsMesh() { return ArmsMesh; };
    FORCEINLINE bool GetIsDead() { return bIsDead; };
    FORCEINLINE bool GetIsAiming() { return bIsAiming; }
    FORCEINLINE UCameraComponent* GetCamera() { return FPSCamera; }
    FORCEINLINE AWeapon* GetCurrentWeapon() { return CurrentWeapon; }

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

    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    void UpdateCrouch(bool bIsCrouch, float DeltaTime);

    void CheckInitialPlayerRefInController();
    /* Set the MafiaBattlegroundCharacter reference into the MBGPlayerController */
    UFUNCTION()
    void SetPlayerRefToController_Delay();

    /* Spawn Default Weapon */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSpawnDefaultWeapon();

    /* Update the MaxWalkSpeed */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSetMaxSpeed();
    UFUNCTION(NetMulticast, Reliable, WithValidation)
    void MultiSetMaxSpeed(float MaxVel);

    /* Set value to CurrentVelocity */
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
