//+--------------------------------------------------------+
//| Project    : MafiaBattleground                         |
//| UE Version : 4.27                                      |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+

#include "MBGPlayerController.h"

#include "MafiaBattlegroundCharacter.h"

//-------------------------------------------------------------------------------------------------
AMBGPlayerController::AMBGPlayerController()
{
    // set our turn rates for input
    BaseTurnRate   = 45.f;
    BaseLookUpRate = 45.f;
}

//-------------------------------------------------------------------------------------------------
void AMBGPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (IsLocalController())
    {
        CreateMainHUD();
    }
}

//-------------------------------------------------------------------------------------------------
void AMBGPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    InputComponent->BindAction("Jump"  , IE_Pressed , this, &AMBGPlayerController::Jump);
    InputComponent->BindAction("Jump"  , IE_Released, this, &AMBGPlayerController::StopJumping);
    InputComponent->BindAction("Crouch", IE_Pressed , this, &AMBGPlayerController::BeginCrouch);
    InputComponent->BindAction("Crouch", IE_Released, this, &AMBGPlayerController::EndCrouch);
    InputComponent->BindAction("Run"   , IE_Pressed , this, &AMBGPlayerController::BeginRun);
    InputComponent->BindAction("Run"   , IE_Released, this, &AMBGPlayerController::EndRun);
    InputComponent->BindAction("Aim"   , IE_Pressed , this, &AMBGPlayerController::SetAim<true>);
    InputComponent->BindAction("Aim"   , IE_Released, this, &AMBGPlayerController::SetAim<false>);

    InputComponent->BindAxis("MoveForward", this, &AMBGPlayerController::MoveForward);
    InputComponent->BindAxis("MoveRight"  , this, &AMBGPlayerController::MoveRight);

    InputComponent->BindAxis("Turn"      , this, &AMBGPlayerController::AddControllerYawInput);
    InputComponent->BindAxis("LookUp"    , this, &AMBGPlayerController::AddControllerPitchInput);
    InputComponent->BindAxis("TurnRate"  , this, &AMBGPlayerController::TurnAtRate);
    InputComponent->BindAxis("LookUpRate", this, &AMBGPlayerController::LookUpAtRate);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AMBGPlayerController::SetAim(bool bIsAiming)
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
void AMBGPlayerController::Jump()
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
void AMBGPlayerController::StopJumping()
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
void AMBGPlayerController::BeginRun()
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
void AMBGPlayerController::EndRun()
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
void AMBGPlayerController::BeginCrouch()
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
void AMBGPlayerController::EndCrouch()
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
void AMBGPlayerController::MoveForward(float Value)
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
void AMBGPlayerController::MoveRight(float Value)
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
void AMBGPlayerController::AddControllerYawInput(float Value)
{
    AddYawInput(Value);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AMBGPlayerController::AddControllerPitchInput(float Value)
{
    AddPitchInput(Value);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AMBGPlayerController::TurnAtRate(float Rate)
{
    // calculate delta for this frame from the rate information
    AddYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AMBGPlayerController::LookUpAtRate(float Rate)
{
    // calculate delta for this frame from the rate information
    AddPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}
