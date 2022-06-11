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
#include "MafiaBattleground/Components/FPSMBHealthComponent.h"

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
    ArmsMesh->SetRelativeLocation(FVector(12.0f, 6.0f, -40.0f));
    ArmsMesh->SetRelativeRotation(FRotator(-10.0f, -89.0f, 92.0f));
    ArmsMesh->bOnlyOwnerSee = true;
    ArmsMesh->CastShadow    = false;

    // Body Shadow
    ShadowMesh = CreateDefaultSubobject<USkeletalMeshComponent>("ShadowMesh");
    ShadowMesh->SetupAttachment(RootComponent);
    ShadowMesh->SetRelativeLocation(FVector(-150.0f, 0.0f, -90.0f));
    ShadowMesh->SetRelativeRotation(FRotator(0.0, -90.0f, 0.0f));
    ShadowMesh->bOnlyOwnerSee     = true;
    ShadowMesh->bRenderInMainPass = false;

    // HealthComponent
    HealthComp = CreateDefaultSubobject<UFPSMBHealthComponent>("HealthComp");

    // Capsule
    GetCapsuleComponent()->InitCapsuleSize(36.0f, 92.0f);

    // Body Mesh Location and Rotation
    GetMesh()->SetRelativeLocation(FVector (0.0f, 0.0f, -90.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, 270.0f, 0.0f));
    GetMesh()->SetIsReplicated(true);
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

    //HipBoneName         = FName("spine_01");
    HeadBoneName        = FName("Head");
    CrouchSALocation    = FVector(0.0f, 0.0f, 40.0f);
    FoldWeaponLocation  = FVector(0.0f, 0.0f, -600.0f);
    bIsDead             = false;
    AimMaxWalkSpeed     = 350.0f;
    CurrentWeaponIndex  = 0;
    CrouchInterpSpeed   = 10.0f;
    DeathImpulse        = 20000.0f;
    DeathTime           = 10.0f;
    RunMaxWalkSpeed     = 1000.0f;
    VelocityThresholdX  = 600.0f;
    VelocityThresholdY  = 600.0f;


    bAlwaysRelevant    = true;
    NetDormancy        = DORM_Never;
    NetUpdateFrequency = 120.0f;
    NetPriority        = 2.0f;
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
    if (!bIsAiming && !bIsReloading)
    {
        // Change bIsRuning if he is going to walk or if he is running and my speeds are greater than 0 
        const float VelX = FMath::Abs(GetVelocity().X);
        const float VelY = FMath::Abs(GetVelocity().Y);
        if ((VelX > 0) || (VelY > 0) || !bIsRuningVal)
        {
            bIsRuning = bIsRuningVal;
            ServerSetMaxSpeed();
        }

        if (bIsRuning && CurrentWeapon)
        {
            CurrentWeapon->ClientStopFire();
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

    HealthComp->OnHealthChanged.AddDynamic(this, &AFPSMBCharacter::OnHealthChanged);

    // Delays Functions
    CheckInitialPlayerRefInController_Delay();
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

    UKismetSystemLibrary::Delay(GetWorld(), 0.02f, LatentInfo); //0.1f
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

        // Create Weapons and turn on the crosshair
        if (PlayerController->IsLocalController())
        {
            ServerSpawnDefaultWeapon();
            SetAimingCrosshair(false);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::WeaponReload()
{
    if (CurrentWeapon->GetCanReload())
    {
        // Stop fire and StandUp
        UnCrouch();
        CurrentWeapon->StopFire();

        // Play reload weapon sound
        CurrentWeapon->MultiPlayReloadSound();

        // Set bIsReloadin to true and stop runing
        ServerWeaponReload(true);
        ServerSetRun(false);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::ServerWeaponReload_Implementation(bool ReloadingVal)
{
    bIsReloading = ReloadingVal;

    // If the reload animation finish reload the weapon
    ReloadingVal? NULL : CurrentWeapon->Reload();
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AFPSMBCharacter::ServerWeaponReload_Validate(bool ReloadingVal)
{    return true;}

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
    // If it is greater than the number of items or less than 0 or WeaponIndex is equals current or is a invalid index return
    if ((WeaponIndex >= Weapons.Num()) || (WeaponIndex < 0) || ( WeaponIndex == CurrentWeaponIndex) || (!Weapons.IsValidIndex(CurrentWeaponIndex)) || bIsReloading)
    {
        return;
    }

    if (!GetIsServer())
    {
        ServerChangeWeapon(WeaponIndex);
        return;
    }

    // Stop runing
    ServerSetRun(false);

    // Stop aiming and stop fire
    ServerSetAiming(false);

    if (CurrentWeapon)
    {
        CurrentWeapon->ClientStopFire();
    }

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
void AFPSMBCharacter::OnHealthChanged(UFPSMBHealthComponent* HealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
    if ((Health <= 0.0f && !bIsDead))
    {
        // Die!
        bIsDead = true;
        OnRep_Died();
        ClientOnDeath();

        GetMovementComponent()->StopMovementImmediately();
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        if (DamageCauser)
        {
            const FVector& DeathDirection = UKismetMathLibrary::Normal(DamageCauser->GetActorForwardVector());
            MultiOnDeathMesh(DeathDirection);
        }
        else
        {
            MultiOnDeathMesh(-GetActorForwardVector());
        }

        // Despawn all Weapons
        for (AWeapon*& Weapon : Weapons)
        {
            Weapon->OnDeath();
        }

        DetachFromControllerPendingDestroy();
        SetLifeSpan(DeathTime);

        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Died!"));
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::OnRep_Died()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->ClientStopFire();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::MultiOnDeathMesh_Implementation(const FVector& DeathDirection)
{
    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    if (GetMesh())
    {
        GetMesh()->SetSimulatePhysics(true);
        GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
        GetMesh()->AddImpulse(DeathDirection * DeathImpulse, HeadBoneName, true);// false
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AFPSMBCharacter::MultiOnDeathMesh_Validate(const FVector& DeathDirection)
{    return true;}

void AFPSMBCharacter::ClientOnDeath_Implementation()
{
    OnDeathHUD();
}

bool AFPSMBCharacter::ClientOnDeath_Validate()
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::Destroyed()
{
    Super::Destroyed();

    // Destroy all Weapons
    for (AWeapon*& Weapon : Weapons)
    {
        Weapon->Destroy();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AFPSMBCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AFPSMBCharacter, CurrentWeaponIndex);
    DOREPLIFETIME(AFPSMBCharacter, CurrentWeapon);
    DOREPLIFETIME(AFPSMBCharacter, Weapons);
    DOREPLIFETIME(AFPSMBCharacter, bIsAiming);
    DOREPLIFETIME(AFPSMBCharacter, bIsDead);
    DOREPLIFETIME(AFPSMBCharacter, bIsReloading);
    DOREPLIFETIME(AFPSMBCharacter, bIsRuning);
}
