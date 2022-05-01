//+--------------------------------------------------------+
//| Project    : MafiaBattleground                         |
//| UE Version : 4.27                                      |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+


#include "FPSMBCharacter.h"
#include "Net/UnrealNetwork.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#include "FPSMBPlayerController.h"
#include "MafiaBattleground/Weapons/Weapon.h"

//------------------------------------------------------------------------------------------------------------------------------------------
AFPSMBCharacter::AFPSMBCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // SpringArm
    SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));
    SpringArm->TargetArmLength         = 50.0f;
    SpringArm->bUsePawnControlRotation = true;
    SpringArm->bEnableCameraLag        = false;

    // Camera
    FPSCamera = CreateDefaultSubobject<UCameraComponent>("FPSCamera");
    FPSCamera->SetupAttachment(SpringArm);
    FPSCamera->bUsePawnControlRotation = false;

    // ArmMesh
    ArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>("ArmsMesh");
    ArmsMesh->SetupAttachment(SpringArm);
    ArmsMesh->SetRelativeLocation(FVector(30.0f, 6.0f, -40.0f));
    ArmsMesh->SetRelativeRotation(FRotator(-10.0f, -89.0f, 92.0f));
    ArmsMesh->bOnlyOwnerSee = true;

    // Capsule
    GetCapsuleComponent()->InitCapsuleSize(36.0f, 92.0f);

    // Body Mesh Location and Rotation
    GetMesh()->SetRelativeLocation(FVector (0.0f, 0.0f, -90.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, 270.0f, 0.0f));
    GetMesh()->bOwnerNoSee = true;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement             = false;
    GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
    GetCharacterMovement()->GravityScale                          = 1.25f;
    GetCharacterMovement()->CrouchedHalfHeight                    = 58.0f;
    GetCharacterMovement()->MaxWalkSpeedCrouched                  = 150.0f;
    GetCharacterMovement()->JumpZVelocity                         = 460.0f;
    GetCharacterMovement()->AirControl                            = 0.2f;

    // Variables
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = true;
    bUseControllerRotationRoll  = false;

    CrouchSALocation    = FVector(0.0f, 0.0f, 40.0f);
    ArmsAimLocation     = FVector(30.0f, -6.0f, -30.0f);
    ArmsDefaultLocation = FVector(30.0f, 6.0f, -40.0f);
    CrouchInterpSpeed   = 10.0f;
    RunMaxWalkSpeed     = 1000.0f;
    AimMaxWalkSpeed     = 350.0f;
    bIsDead             = false;


    bAlwaysRelevant    = true;
    NetDormancy        = DORM_Never;
    NetUpdateFrequency = 120.0f;
    NetPriority        = 6.0f;
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::ServerSetAiming_Implementation(bool bIsAimingVal)
{
    bIsAiming = bIsAimingVal;
    MultiSetAiming(bIsAimingVal);
    ServerSetMaxSpeed();
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AFPSMBCharacter::ServerSetAiming_Validate(bool bIsAimingVal)
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::MultiSetAiming_Implementation(bool bIsAimingVal)
{
    if (CurrentWeapon)
    {
        if (bIsAimingVal)
        {
            FPSCamera->AttachToComponent(CurrentWeapon->GetGunMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("CameraAim"));
            ArmsMesh->SetRelativeLocation(ArmsAimLocation);
            FPSCamera->bUsePawnControlRotation = true;
        }
        else
        {
            FPSCamera->AttachToComponent(SpringArm, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
            ArmsMesh->SetRelativeLocation(ArmsDefaultLocation);
            FPSCamera->bUsePawnControlRotation = false;
        }
    }

    SetAimingCrosshair(bIsAimingVal);
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AFPSMBCharacter::MultiSetAiming_Validate(bool bIsAimingVal)
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::ServerSetRun_Implementation(bool bIsRuningVal)
{
    if (!bIsAiming)
    {
        bIsRuning = bIsRuningVal;
        ServerSetMaxSpeed();
        return;
    }

    bIsRuning = false;
    ServerSetMaxSpeed();
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AFPSMBCharacter::ServerSetRun_Validate(bool bIsRuningVal)
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
float AFPSMBCharacter::GetControlPitchRotation()
{
    const FRotator ControlRot = GetBaseAimRotation();
    return UKismetMathLibrary::NormalizeAxis(ControlRot.Pitch);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::BeginPlay()
{
    Super::BeginPlay();

    DefaultSALocation   = SpringArm->GetRelativeLocation();
    DefaultFOV          = FPSCamera->FieldOfView;
    DefaultMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

    // Delays Functions
    CheckInitialPlayerRefInController();

    if (IsLocallyControlled())
    {
        ServerSpawnDefaultWeapon();
        SetAimingCrosshair(false); // Set visible the crosshair
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ServerSetVelocity();
    ServerSetJump();
    ZoomInterp(DeltaTime);
    UpdateCrouch(GetCharacterMovement()->IsCrouching(), DeltaTime);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::UpdateCrouch(bool bIsCrouch, float DeltaTime)
{
    const FVector TargetLocation = bIsCrouch ? CrouchSALocation : DefaultSALocation;
    const FVector NextLocation   = FMath::VInterpTo(SpringArm->GetRelativeLocation(), TargetLocation, DeltaTime, CrouchInterpSpeed);

    SpringArm->SetRelativeLocation(NextLocation);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::CheckInitialPlayerRefInController()
{
    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget    = this;
    LatentInfo.ExecutionFunction = FName("SetPlayerRefToController_Delay");
    LatentInfo.Linkage           = 0;
    LatentInfo.UUID              = 0;

    UKismetSystemLibrary::Delay(GetWorld(), 0.1f, LatentInfo);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::SetPlayerRefToController_Delay()
{
    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (PlayerController)
    {
        AFPSMBPlayerController* MBGPlayerController = Cast<AFPSMBPlayerController>(GetController());
        if (MBGPlayerController)
        {
            MBGPlayerController->MyPlayerRef = this;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::ServerSpawnDefaultWeapon_Implementation()
{
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.Instigator = this;

    CurrentWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeapon, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    if (CurrentWeapon)
    {
        CurrentWeapon->ServerGiveToPayer(this);
        CurrentWeapon->MyPlayer = this;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AFPSMBCharacter::ServerSpawnDefaultWeapon_Validate()
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::ServerSetMaxSpeed_Implementation()
{
    if (bIsDead)
    {
        MultiSetMaxSpeed(0.0f);
        return;
    }

    if (bIsAiming && !bIsCrouched)
    {
        MultiSetMaxSpeed(AimMaxWalkSpeed);
        return;
    }

    if (bIsRuning && !bIsCrouched)
    {
        MultiSetMaxSpeed(RunMaxWalkSpeed);
        return;
    }

    MultiSetMaxSpeed(DefaultMaxWalkSpeed);
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AFPSMBCharacter::ServerSetMaxSpeed_Validate()
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::MultiSetMaxSpeed_Implementation(float MaxVel)
{
    GetCharacterMovement()->MaxWalkSpeed = MaxVel;
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AFPSMBCharacter::MultiSetMaxSpeed_Validate(float MaxVel)
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::ServerSetVelocity_Implementation()
{
    MultiSetVelocity(GetVelocity());;
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AFPSMBCharacter::ServerSetVelocity_Validate()
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::MultiSetVelocity_Implementation(FVector CurrentVel)
{
    CurrentVelocity = CurrentVel;
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AFPSMBCharacter::MultiSetVelocity_Validate(FVector CurrentVel)
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::ServerSetJump_Implementation()
{
    MultiSetJump(bWasJumping);
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AFPSMBCharacter::ServerSetJump_Validate()
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::MultiSetJump_Implementation(bool bJumpValue)
{
    bJumped = bJumpValue;
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AFPSMBCharacter::MultiSetJump_Validate(bool bJumpValue)
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::ZoomInterp(const float DeltaTime)
{
    if (CurrentWeapon)
    {
        const float TargetFOV = bIsAiming ? (CurrentWeapon->GetWeaponAimFOV()) : DefaultFOV;

        float CurrentFOV = FMath::FInterpTo(FPSCamera->FieldOfView, TargetFOV, DeltaTime, CurrentWeapon->GetWeaponInterpSpeedAim());

        FPSCamera->SetFieldOfView(CurrentFOV);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AFPSMBCharacter, CurrentWeapon);
    DOREPLIFETIME(AFPSMBCharacter, bIsAiming);
    DOREPLIFETIME(AFPSMBCharacter, bIsRuning);
    DOREPLIFETIME(AFPSMBCharacter, bIsDead);
}
