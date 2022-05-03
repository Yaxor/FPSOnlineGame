//+--------------------------------------------------------+
//| Project    : MafiaBattleground                         |
//| UE Version : 4.27                                      |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPSMBPlayerController.generated.h"

UCLASS()
class MAFIABATTLEGROUND_API AFPSMBPlayerController : public APlayerController
{
    GENERATED_BODY()

    friend class AFPSMBCharacter;

public:
    //!Constructor
    AFPSMBPlayerController();

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED VARIABLES                                                     *
    //*******************************************************************************************************************

    UPROPERTY(BlueprintReadOnly, Category = Player)
    class AFPSMBCharacter* MyPlayerRef = nullptr;

    /** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera)
    float BaseTurnRate;

    /** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera)
    float BaseLookUpRate;

public:
    //*******************************************************************************************************************
    //                                          PUBLIC FUNCTIONS                                                        *
    //*******************************************************************************************************************

    UFUNCTION(BlueprintImplementableEvent)
    void CreateMainHUD();

private:
    //*******************************************************************************************************************
    //                                          PRIVATE FUNCTIONS                                                       *
    //*******************************************************************************************************************

    virtual void BeginPlay();

    virtual void SetupInputComponent() override;

    void ChangeWeapon(const uint8_t Index);

    template<uint8_t Index>
    void ChangeWeapon()
    {
        ChangeWeapon(Index);
    }

    void SetAim(bool bIsAiming);

    template<bool bIsAiming>
    void SetAim()
    {
        SetAim(bIsAiming);
    }

    void Jump();

    void StopJumping();

    void BeginRun();
    void EndRun();

    void BeginCrouch();
    void EndCrouch();

    void StartFireWeapon();
    void StopFireWeapon();
    void ReloadWeapon();

    /** Called for forwards/backward input */
    void MoveForward(float Value);

    /** Called for side to side input */
    void MoveRight(float Value);

    void AddControllerYawInput(float Value);

    void AddControllerPitchInput(float Value);

    /**
     * Called via input to turn at a given rate.
     * @param Rate This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
     */
    void TurnAtRate(float Rate);

    /**
     * Called via input to turn look up/down at a given rate.
     * @param Rate This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
     */
    void LookUpAtRate(float Rate);

};
