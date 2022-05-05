//+--------------------------------------------------------+
//| Project    : MafiaBattleground                         |
//| UE Version : 4.27                                      |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+


#include "FPSMBPlayerController.h"

#include "FPSMBCharacter.h"
#include "MafiaBattleground/Weapons/Weapon.h"

//------------------------------------------------------------------------------------------------------------------------------------------
AFPSMBPlayerController::AFPSMBPlayerController()
{
    BaseTurnRate   = 45.0f;
    BaseLookUpRate = 45.0f;
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (IsLocalController())
    {
        CreateMainHUD();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    InputComponent->BindAction("Jump"  , IE_Pressed , this, &AFPSMBPlayerController::Jump);
    InputComponent->BindAction("Jump"  , IE_Released, this, &AFPSMBPlayerController::StopJumping);
    InputComponent->BindAction("Crouch", IE_Pressed , this, &AFPSMBPlayerController::BeginCrouch);
    InputComponent->BindAction("Crouch", IE_Released, this, &AFPSMBPlayerController::EndCrouch);
    InputComponent->BindAction("Run"   , IE_Pressed , this, &AFPSMBPlayerController::BeginRun);
    InputComponent->BindAction("Run"   , IE_Released, this, &AFPSMBPlayerController::EndRun);
    InputComponent->BindAction("Aim"   , IE_Pressed , this, &AFPSMBPlayerController::SetAim<true>);
    InputComponent->BindAction("Aim"   , IE_Released, this, &AFPSMBPlayerController::SetAim<false>);
    InputComponent->BindAction("Fire"  , IE_Pressed , this, &AFPSMBPlayerController::StartFireWeapon);
    InputComponent->BindAction("Fire"  , IE_Released, this, &AFPSMBPlayerController::StopFireWeapon);
    InputComponent->BindAction("Reload", IE_Pressed , this, &AFPSMBPlayerController::ReloadWeapon);
    InputComponent->BindAction("ItemOne", IE_Pressed, this, &AFPSMBPlayerController::ChangeWeapon<0>);
    InputComponent->BindAction("ItemTwo", IE_Pressed, this, &AFPSMBPlayerController::ChangeWeapon<1>);


    InputComponent->BindAxis("MoveForward", this, &AFPSMBPlayerController::MoveForward);
    InputComponent->BindAxis("MoveRight"  , this, &AFPSMBPlayerController::MoveRight);

    InputComponent->BindAxis("Turn"      , this, &AFPSMBPlayerController::AddControllerYawInput);
    InputComponent->BindAxis("LookUp"    , this, &AFPSMBPlayerController::AddControllerPitchInput);
    InputComponent->BindAxis("TurnRate"  , this, &AFPSMBPlayerController::TurnAtRate);
    InputComponent->BindAxis("LookUpRate", this, &AFPSMBPlayerController::LookUpAtRate);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBPlayerController::ChangeWeapon(const uint8_t Index)
{
    if (MyPlayerRef)
    {
        if (!MyPlayerRef->GetIsDead())
        {
            MyPlayerRef->ChangeWeapon(Index);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBPlayerController::SetAim(bool bIsAiming)
{
    if (MyPlayerRef)
    {
        if (!MyPlayerRef->GetIsDead())
        {
            MyPlayerRef->ServerSetAiming(bIsAiming);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBPlayerController::Jump()
{
    if (MyPlayerRef)
    {
        if (!MyPlayerRef->GetIsDead())
        {
            MyPlayerRef->Jump();
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBPlayerController::StopJumping()
{
    if (MyPlayerRef)
    {
        if (!MyPlayerRef->GetIsDead())
        {
            MyPlayerRef->StopJumping();
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBPlayerController::BeginRun()
{
    if (MyPlayerRef)
    {
        if (!MyPlayerRef->GetIsDead())
        {
            MyPlayerRef->ServerSetRun(true);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBPlayerController::EndRun()
{
    if (MyPlayerRef)
    {
        if (!MyPlayerRef->GetIsDead())
        {
            MyPlayerRef->ServerSetRun(false);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBPlayerController::BeginCrouch()
{
    if (MyPlayerRef)
    {
        if (!MyPlayerRef->GetIsDead())
        {
            MyPlayerRef->Crouch();
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBPlayerController::EndCrouch()
{
    if (MyPlayerRef)
    {
        if (!MyPlayerRef->GetIsDead())
        {
            MyPlayerRef->UnCrouch();
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBPlayerController::StartFireWeapon()
{
    if (MyPlayerRef)
    {
        if ((!MyPlayerRef->GetIsDead()) && (MyPlayerRef->GetCurrentWeapon()) && (!MyPlayerRef->bIsRuning))
        {
            MyPlayerRef->GetCurrentWeapon()->StartFire();
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBPlayerController::StopFireWeapon()
{
    if (MyPlayerRef)
    {
        if (MyPlayerRef->GetCurrentWeapon())
        {
            MyPlayerRef->GetCurrentWeapon()->StopFire();
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBPlayerController::ReloadWeapon()
{
    if (MyPlayerRef)
    {
        if (!MyPlayerRef->GetIsDead() && MyPlayerRef->GetCurrentWeapon())
        {
            //MyPlayerRef->GetCurrentWeapon()->Reload();
            MyPlayerRef->WeaponReload();
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBPlayerController::MoveForward(float Value)
{
    if ((MyPlayerRef) && (Value != 0.0f))
    {
        if (!MyPlayerRef->GetIsDead())
        {
            // find out which way is forward
            const FRotator Rotation = GetControlRotation();
            const FRotator YawRotation(0, Rotation.Yaw, 0);

            // get forward vector
            const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
            MyPlayerRef->AddMovementInput(Direction, Value);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBPlayerController::MoveRight(float Value)
{
    if ((MyPlayerRef) && (Value != 0.0f))
    {
        if (!MyPlayerRef->GetIsDead())
        {
            // find out which way is right
            const FRotator Rotation = GetControlRotation();
            const FRotator YawRotation(0, Rotation.Yaw, 0);

            // get right vector 
            const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
            // add movement in that direction
            MyPlayerRef->AddMovementInput(Direction, Value);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBPlayerController::AddControllerYawInput(float Value)
{
    AddYawInput(Value);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBPlayerController::AddControllerPitchInput(float Value)
{
    AddPitchInput(Value);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBPlayerController::TurnAtRate(float Rate)
{
    // calculate delta for this frame from the rate information
    AddYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBPlayerController::LookUpAtRate(float Rate)
{
    // calculate delta for this frame from the rate information
    AddPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}
