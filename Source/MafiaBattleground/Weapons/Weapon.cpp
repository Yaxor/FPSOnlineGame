//+--------------------------------------------------------+
//| Project    : MafiaBattleground                         |
//| UE Version : 4.27                                      |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+


#include "Weapon.h"
#include "Net/UnrealNetwork.h"

#include "Camera/CameraComponent.h"

#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

#include "PhysicalMaterials/PhysicalMaterial.h"

#include "Particles/ParticleSystem.h"

#include "GameFramework/Character.h"
#include "MafiaBattleground/Player/FPSMBCharacter.h"
#include "MafiaBattleground/MafiaBattleground.h"

//------------------------------------------------------------------------------------------------------------------------------------------
static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(TEXT("Mafia.DebugWeapons"), DebugWeaponDrawing,
                                               TEXT("Draw Debug Lines for Weapons"), ECVF_Cheat);

//------------------------------------------------------------------------------------------------------------------------------------------
AWeapon::AWeapon()
{
    GunMesh = CreateDefaultSubobject<USkeletalMeshComponent>("GunMesh");
    GunMesh->bOnlyOwnerSee = true;

    // Make other gunmesh for the other clients

    WeaponSocket       = FName("WeaponSocket");
    MuzzleSocketName   = FName("MuzzleSocket");
    AimFOV             = 65.0f;
    AimInterSpeedAim   = 22.0f;
    BaseDamage         = 20.0f;
    BulletSpread       = 2.0f;
    HeadshotMultiplier = 2.5f;
    MaxAmmo            = 30;
    FireRate           = 600.0f;
    ShotDistance       = 10000.0f;

    SetReplicates(true);
    SetReplicatingMovement(true);
    bAlwaysRelevant = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AWeapon::ServerGiveToPayer_Implementation(class ACharacter* Player)
{
    AActor* MyPlayerActor        = CastChecked<AActor>(Player);
    AFPSMBCharacter* MyFPSPlayer = Cast<AFPSMBCharacter>(Player);
    if (MyPlayerActor)
    {
        SetOwner(MyPlayerActor);

        if (MyFPSPlayer)
        {
            GunMesh->AttachToComponent(MyFPSPlayer->GetArmsMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
            return;
        }

        GunMesh->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AWeapon::ServerGiveToPayer_Validate(class ACharacter* Player)
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AWeapon::Reload()
{
    // Call in animation reload

    // If you are a Client, send a request to Server
    if (!GetIsServer()) // GetLocalRole() < ROLE_Authority)
    {
        ServerReload();
        return;
    }

    StopFire();
    CurrentAmmo = MaxAmmo;
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AWeapon::StartFire()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AWeapon::StopFire()
{
    ClientStopFire();
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AWeapon::BeginPlay()
{
    Super::BeginPlay();

    Cadence     = (60 / FireRate);
    CurrentAmmo = MaxAmmo;
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AWeapon::ClientStopFire_Implementation()
{
    GetWorldTimerManager().ClearTimer(TimerHandle_Fire);
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AWeapon::ClientStopFire_Validate()
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AWeapon::Fire()
{
    if (CurrentAmmo <= 0)
    {
        return;
    }

    if (!GetIsServer()) //GetLocalRole() < ROLE_Authority)
    {
        ServerFire();
        return;
    }

    AActor* MyOwner = GetOwner();
    AFPSMBCharacter* MyFPSPlayer = CastChecked<AFPSMBCharacter>(GetOwner());

    if (MyOwner && MyFPSPlayer && GetIsServer()) // (GetLocalRole() == ROLE_Authority))
    {
        CurrentAmmo--;

        const FVector& StartLocation = MyFPSPlayer->GetCamera()->GetComponentLocation();
        const FRotator& AimRotation  = MyFPSPlayer->GetBaseAimRotation();

        FVector ShotDirection = AimRotation.Vector();

        // Bullet Spread if my player is not aiming
        if (!MyFPSPlayer->GetIsAiming())
        {
            float HalfRad = FMath::DegreesToRadians(BulletSpread);
            ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);
        }

        FVector EndLocation = StartLocation + (ShotDirection * ShotDistance);

        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(MyOwner);
        QueryParams.AddIgnoredActor(this);
        QueryParams.bTraceComplex           = true;
        QueryParams.bReturnPhysicalMaterial = true;

        // Particle Target parameter
        FVector TraceEndPoint = EndLocation;

        EPhysicalSurface SurfaceType = SurfaceType_Default;

        FHitResult Hit;
        if (GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, COLLISION_WEAPON, QueryParams))
        {
            AActor* HitActor = Hit.GetActor();

            SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

            float ActualDamage = BaseDamage;
            if (SurfaceType == SURFACE_FLESHVULNERABLE)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Headshot"));
                ActualDamage *= HeadshotMultiplier;
            }

            UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit,
                                               MyOwner->GetInstigatorController(), MyOwner, DamageType);

            TraceEndPoint = Hit.ImpactPoint;
        }

        if (DebugWeaponDrawing > 0)
        {
            DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 1.0f, 0, 1.0f);
        }

        PlayFireFX();

        if (Hit.GetActor())
        {
            PlayImpactFX(SurfaceType, TraceEndPoint);
        }

        LastFireTime = GetWorld()->TimeSeconds;

        // Por cada tiro rotar un poco el arma hacia arriba o sumarle un poco de altura al EndLocation a la direccion
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AWeapon::ServerFire_Implementation()
{
    Fire();
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AWeapon::ServerFire_Validate()
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AWeapon::ServerReload_Implementation()
{
    Reload();
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AWeapon::ServerReload_Validate()
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AWeapon::PlayImpactFX(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
    MultiPlayImpactFX(SurfaceType, ImpactPoint);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AWeapon::PlayFireFX()
{
    ClientPlayFireFX();
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AWeapon::MultiPlayImpactFX_Implementation(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
    // Impact FX
    UParticleSystem* SelectedVFX = nullptr;
    switch (SurfaceType)
    {
        case SURFACE_FLESHDEFAULT:
        case SURFACE_FLESHVULNERABLE: SelectedVFX = FleshImpactVFX; break;
        default: SelectedVFX = DefaultImpactVFX; break;
    }

    if (SelectedVFX)
    {
        FVector MuzzleLocation = GunMesh->GetSocketLocation(MuzzleSocketName);

        FVector ShotDiretion = ImpactPoint - MuzzleLocation;
        ShotDiretion.Normalize();

        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedVFX, ImpactPoint, ShotDiretion.Rotation(), true, EPSCPoolMethod::AutoRelease, true);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AWeapon::MultiPlayImpactFX_Validate(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AWeapon::ClientPlayFireFX_Implementation()
{
    // Shot FX
    if (MuzzleVFX)
    {
        UGameplayStatics::SpawnEmitterAttached(MuzzleVFX, GunMesh, MuzzleSocketName, FVector::ZeroVector, FRotator::ZeroRotator,
                                               EAttachLocation::SnapToTarget, true, EPSCPoolMethod::AutoRelease, true);
        //const FVector MuzzleLocation  = GunMesh->GetSocketLocation(MuzzleSocketName);
        //const FRotator MuzzleRotation = GunMesh->GetSocketRotation(MuzzleSocketName);
        //UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleVFX, MuzzleLocation, MuzzleRotation);
    }

    // Camera Shake
    APawn* MyOwner = Cast<APawn>(GetOwner());
    if (MyOwner)
    {
        APlayerController* PlayerController = Cast<APlayerController>(MyOwner->GetController());
        if (PlayerController)
        {
            PlayerController->ClientStartCameraShake(FireCamShake);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool AWeapon::ClientPlayFireFX_Validate()
{    return true;}

//------------------------------------------------------------------------------------------------------------------------------------------
void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AWeapon, CurrentAmmo);
}
