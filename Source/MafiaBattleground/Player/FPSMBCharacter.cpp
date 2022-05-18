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
    SpringArm->TargetArmLength          = 50.0f;
    SpringArm->bUsePawnControlRotation  = true;
    SpringArm->bEnableCameraLag         = false;
    SpringArm->bEnableCameraRotationLag = false;

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
    ArmsMesh->CastShadow    = false;

    // Foots mesh
    FootsMesh = CreateDefaultSubobject<USkeletalMeshComponent>("FootsMesh");
    FootsMesh->SetupAttachment(RootComponent);
    FootsMesh->SetRelativeLocation(FVector(-70.0f, 0.0f, -90.0f));
    FootsMesh->SetRelativeRotation(FRotator(0.0, -90.0f, 0.0f));
    FootsMesh->bOnlyOwnerSee     = true;
    FootsMesh->CastShadow        = false;

    // Body Shadow
    ShadowMesh = CreateDefaultSubobject<USkeletalMeshComponent>("ShadowMesh");
    ShadowMesh->SetupAttachment(RootComponent);
    ShadowMesh->SetRelativeLocation(FVector(-70.0f, 0.0f, -90.0f));
    ShadowMesh->SetRelativeRotation(FRotator(0.0, -90.0f, 0.0f));
    ShadowMesh->bOnlyOwnerSee     = true;
    ShadowMesh->bRenderInMainPass = false;

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

    HipBoneName         = FName("spine_01");
    CrouchSALocation    = FVector(0.0f, 0.0f, 40.0f);
    FoldWeaponLocation  = FVector(0.0f, 0.0f, -600.0f);
    CurrentWeaponIndex  = 0;
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
    ClientSetAiming(bIsAimingVal);

    if (CurrentWeapon)
    {
        CurrentWeapon->MultiAim(bIsAimingVal);
    }

    bIsAimingVal ? bIsRuning = false : NULL;
    ServerSetMaxSpeed();
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AFPSMBCharacter::ServerSetAiming_Validate(bool bIsAimingVal)
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::ClientSetAiming_Implementation(bool bIsAimingVal)
{
    SetAimingCrosshair(bIsAimingVal);
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AFPSMBCharacter::ClientSetAiming_Validate(bool bIsAimingVal)
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::ServerSetRun_Implementation(bool bIsRuningVal)
{
    // Run
    if (!bIsAiming)
    {
        bIsRuning = bIsRuningVal;
        ServerSetMaxSpeed();

        if (CurrentWeapon)
        {
            CurrentWeapon->StopFire();
        }

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

    ArmsDefaultLocation    = ArmsMesh->GetRelativeLocation();
    DefaultSALocation      = SpringArm->GetRelativeLocation();
    DefaultSpringArmLength = SpringArm->TargetArmLength;
    DefaultFOV             = FPSCamera->FieldOfView;
    DefaultMaxWalkSpeed    = GetCharacterMovement()->MaxWalkSpeed;

    FootsMesh->HideBoneByName(HipBoneName, EPhysBodyOp::PBO_None);

    // Delays Functions
    CheckInitialPlayerRefInController_Delay();

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
void AFPSMBCharacter::CheckInitialPlayerRefInController_Delay()
{
    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget    = this;
    LatentInfo.ExecutionFunction = FName("SetPlayerRefToController");
    LatentInfo.Linkage           = 0;
    LatentInfo.UUID              = 0;

    UKismetSystemLibrary::Delay(GetWorld(), 0.1f, LatentInfo);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::SetPlayerRefToController()
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
void AFPSMBCharacter::WeaponReload()
{
    if (CurrentWeapon->GetCanReload())
    {
        // Stop runing and aiming
        ServerSetRun(false);
        ServerSetAiming(false);

        CurrentWeapon->Reload();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::UpdateCrouch(bool bIsCrouch, float DeltaTime)
{
    const FVector TargetLocation = bIsCrouch ? CrouchSALocation : DefaultSALocation;
    const FVector NextLocation   = FMath::VInterpTo(SpringArm->GetRelativeLocation(), TargetLocation, DeltaTime, CrouchInterpSpeed);

    SpringArm->SetRelativeLocation(NextLocation);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::ServerSpawnDefaultWeapon_Implementation()
{
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.Instigator = this;

    CurrentWeapon = GetWorld()->SpawnActor<AWeapon>(AK47, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    Weapons.Add(CurrentWeapon);
    Weapons.Add(GetWorld()->SpawnActor<AWeapon>(SARifle, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams));
    if (CurrentWeapon)
    {
        CurrentWeapon->ServerGiveToPayer(this);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AFPSMBCharacter::ServerSpawnDefaultWeapon_Validate()
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::ChangeWeapon(uint8_t WeaponIndex)
{
    if (!GetIsServer())
    {
        ServerChangeWeapon(WeaponIndex);
        return;
    }

    if (WeaponIndex == CurrentWeaponIndex)
    {
        return;
    }

    // Stop runing
    ServerSetRun(false);

    // Stop aiming and stop fire
    ServerSetAiming(false);
    CurrentWeapon->StopFire();

    // Update Current Weapon
    CurrentWeaponIndex = WeaponIndex;
    CurrentWeapon      = Weapons[CurrentWeaponIndex];

    // Relocalize the Weapons
    for (AWeapon*& Weapon : Weapons)
    {
        if (Weapon == Weapons[CurrentWeaponIndex])
        {
            Weapon->AttachToActor(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
            Weapon->ServerGiveToPayer(this);
            continue;
        }

        //GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("CHANGE WEAPON"));
        Weapon->GetGunMesh()->SetWorldLocation(FoldWeaponLocation);
        Weapon->GetGunMesh()->SetWorldRotation(FRotator::ZeroRotator);
        Weapon->GetClientsGunMesh()->SetWorldLocation(FoldWeaponLocation);
        Weapon->GetClientsGunMesh()->SetWorldRotation(FRotator::ZeroRotator);
        Weapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::ServerChangeWeapon_Implementation(int WeaponIndex)
{
    ChangeWeapon(WeaponIndex);
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AFPSMBCharacter::ServerChangeWeapon_Validate(int WeaponIndex)
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
