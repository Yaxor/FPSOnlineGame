//+--------------------------------------------------------+
//| Project    : MafiaBattleground                         |
//| UE Version : 4.27                                      |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+

#include "MafiaBattlegroundCharacter.h"
#include "Net/UnrealNetwork.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
//#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#include "MBGPlayerController.h"
#include "MafiaBattleground/Weapons/Weapon.h"

//------------------------------------------------------------------------------------------------------------------------------------------
AMafiaBattlegroundCharacter::AMafiaBattlegroundCharacter()
{
    // Create a camera boom (pulls in towards the player if there is a collision)
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 72.0f));
    SpringArm->SocketOffset             = FVector(0.0f, 40.0f, 0.0f);
    SpringArm->TargetArmLength          = 250.0f; // The camera follows at this distance behind the character
    SpringArm->bUsePawnControlRotation  = true; // Rotate the arm based on the controller
    SpringArm->bEnableCameraLag         = true;
    SpringArm->bEnableCameraRotationLag = true;
    SpringArm->CameraLagSpeed           = 16.0f;
    SpringArm->CameraRotationLagSpeed   = 22.0f;

    // Create a follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
    FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(36.0f, 92.0f);

    // Set Mesh Location and Rotation
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, 270.0f, 0.0f));

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement             = true; // Character moves in the direction of input...
    GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
    GetCharacterMovement()->RotationRate                          = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
    GetCharacterMovement()->GravityScale                          = 1.25f;
    GetCharacterMovement()->CrouchedHalfHeight                    = 60.0f;
    GetCharacterMovement()->JumpZVelocity                         = 460.0f;
    GetCharacterMovement()->AirControl                            = 0.2f;

    // Don't rotate when the controller rotates. Let that just affect the camera.
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;

    RunMaxWalkSpeed = 1000.0f;
    AimMaxWalkSpeed = 400.0f;
    bIsDead         = false;

    bAlwaysRelevant    = true;
    NetDormancy        = DORM_Never;
    NetUpdateFrequency = 120.0f;
    NetPriority        = 6.0f;
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AMafiaBattlegroundCharacter::ServerSetAiming_Implementation(bool bIsAimingVal)
{
    bIsAiming    = bIsAimingVal;
    MultiSetAiming(bIsAimingVal);
    ServerSetMaxSpeed();
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AMafiaBattlegroundCharacter::ServerSetAiming_Validate(bool bIsAimingVal)
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AMafiaBattlegroundCharacter::MultiSetAiming_Implementation(bool bIsAimingVal)
{
    bUseControllerRotationYaw = bIsAimingVal;
    bIsAimingVal ? GetCharacterMovement()->bOrientRotationToMovement = false : GetCharacterMovement()->bOrientRotationToMovement = true;
    SetAimingCrosshair(bIsAimingVal);
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AMafiaBattlegroundCharacter::MultiSetAiming_Validate(bool bIsAimingVal)
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AMafiaBattlegroundCharacter::ServerSetRun_Implementation(bool bIsRuningVal)
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
bool AMafiaBattlegroundCharacter::ServerSetRun_Validate(bool bIsRuningVal)
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
float AMafiaBattlegroundCharacter::GetControlPitchRotation()
{
    const FRotator ControlRot = GetBaseAimRotation();
    return UKismetMathLibrary::NormalizeAxis(ControlRot.Pitch);
}


//------------------------------------------------------------------------------------------------------------------------------------------
void AMafiaBattlegroundCharacter::BeginPlay()
{
    Super::BeginPlay();

    DefaultFOV          = FollowCamera->FieldOfView;
    DefaultMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

    // Delays Functions
    CheckInitialPlayerRefInController();

    if (IsLocallyControlled())
    {
        ServerSpawnDefaultWeapon();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AMafiaBattlegroundCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ServerSetVelocity();
    ServerSetJump();
    ZoomInterp(DeltaTime);
}


//------------------------------------------------------------------------------------------------------------------------------------------
void AMafiaBattlegroundCharacter::CheckInitialPlayerRefInController()
{
    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget    = this;
    LatentInfo.ExecutionFunction = FName("SetPlayerRefToController_Delay");
    LatentInfo.Linkage           = 0;
    LatentInfo.UUID              = 0;

    UKismetSystemLibrary::Delay(GetWorld(), 0.1f, LatentInfo);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AMafiaBattlegroundCharacter::SetPlayerRefToController_Delay()
{
    //GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("SetPlayerRefToController_Delay"));

    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (PlayerController)
    {
        AMBGPlayerController* MBGPlayerController = Cast<AMBGPlayerController>(GetController());
        if (MBGPlayerController)
        {
            MBGPlayerController->MyPlayerRef = this;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AMafiaBattlegroundCharacter::ServerSpawnDefaultWeapon_Implementation()
{
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.Instigator = this;

    CurrentWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeapon, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    if (CurrentWeapon)
    {
        CurrentWeapon->ServerGiveToPayer(this);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AMafiaBattlegroundCharacter::ServerSpawnDefaultWeapon_Validate()
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AMafiaBattlegroundCharacter::ServerSetMaxSpeed_Implementation()
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
bool AMafiaBattlegroundCharacter::ServerSetMaxSpeed_Validate()
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AMafiaBattlegroundCharacter::MultiSetMaxSpeed_Implementation(float MaxVel)
{
    GetCharacterMovement()->MaxWalkSpeed = MaxVel;
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AMafiaBattlegroundCharacter::MultiSetMaxSpeed_Validate(float MaxVel)
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AMafiaBattlegroundCharacter::ServerSetVelocity_Implementation()
{
    MultiSetVelocity(GetVelocity());
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AMafiaBattlegroundCharacter::ServerSetVelocity_Validate()
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AMafiaBattlegroundCharacter::MultiSetVelocity_Implementation(FVector CurrentVel)
{
    CurrentVelocity = CurrentVel;
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AMafiaBattlegroundCharacter::MultiSetVelocity_Validate(FVector CurrentVel)
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AMafiaBattlegroundCharacter::ServerSetJump_Implementation()
{
    MultiSetJump(bWasJumping);
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AMafiaBattlegroundCharacter::ServerSetJump_Validate()
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AMafiaBattlegroundCharacter::MultiSetJump_Implementation(bool bJumpValue)
{
    bJumped = bJumpValue;
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AMafiaBattlegroundCharacter::MultiSetJump_Validate(bool bJumpValue)
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AMafiaBattlegroundCharacter::ZoomInterp(const float DeltaTime)
{
    if (CurrentWeapon)
    {
        const float TargetFOV = bIsAiming ? (CurrentWeapon->GetWeaponAimFOV()) : DefaultFOV;

        float CurrentFOV = FMath::FInterpTo(FollowCamera->FieldOfView, TargetFOV, DeltaTime, CurrentWeapon->GetWeaponInterpSpeedAim());

        FollowCamera->SetFieldOfView(CurrentFOV);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AMafiaBattlegroundCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMafiaBattlegroundCharacter, CurrentWeapon);
    DOREPLIFETIME(AMafiaBattlegroundCharacter, bIsAiming);
    DOREPLIFETIME(AMafiaBattlegroundCharacter, bIsRuning);
    DOREPLIFETIME(AMafiaBattlegroundCharacter, bIsDead);
}
