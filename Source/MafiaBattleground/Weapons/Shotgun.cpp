//+--------------------------------------------------------+
//| Project    : MafiaBattleground                         |
//| UE Version : 4.27                                      |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+


#include "Shotgun.h"
#include "Net/UnrealNetwork.h"

#include "Camera/CameraComponent.h"

#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

#include "MafiaBattleground/Player/FPSMBCharacter.h"
#include "MafiaBattleground/MafiaBattleground.h"

//------------------------------------------------------------------------------------------------------------------------------------------
static int32 DebugWeaponDrawingShotG = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawingShotG(TEXT("Mafia.DebugWeaponsShotG"), DebugWeaponDrawingShotG,
                                                    TEXT("Draw Debug Lines for Weapons"), ECVF_Cheat);

//------------------------------------------------------------------------------------------------------------------------------------------
AShotgun::AShotgun()
{
    bHasTriggered  = false;
    BulletSpread   = 8.0f;
    AimSpreadBoost = 4.0f;
    Pellets        = 8;
    FireRate       = 9.0f;
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AShotgun::StartFire()
{
    Super::StartFire();

    Fire();
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AShotgun::Fire()
{
    if (!bHasTriggered)
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

        GetWorldTimerManager().SetTimer(TimerHandle_ResetTrigger, this, &AShotgun::ResetTrigger, Cadence, false);

        AActor* MyOwner = GetOwner();
        AFPSMBCharacter* MyFPSPlayer = CastChecked<AFPSMBCharacter>(GetOwner());

        if (MyOwner && MyFPSPlayer && GetIsServer()) // (GetLocalRole() == ROLE_Authority))
        {
            CurrentAmmo--;
            ShotsCounterFireFX++;

            const FVector& StartLocation = GunMesh->GetSocketLocation(AimShotSocket);
            const FRotator& AimRotation  = MyFPSPlayer->GetBaseAimRotation();

            FVector ShotDirection = AimRotation.Vector();

            for (uint8 i = 0; i < Pellets; i++)
            {
                // Bullet Spread if my player is not aiming
                if (!MyFPSPlayer->GetIsAiming())
                {
                    float HalfRad = FMath::DegreesToRadians(BulletSpread);
                    ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);
                }
                else
                {
                    float HalfRad = FMath::DegreesToRadians(BulletSpread / AimSpreadBoost);
                    ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);
                }

                FVector EndLocation = StartLocation + (ShotDirection * ShotDistance);

                FCollisionQueryParams QueryParams;
                QueryParams.AddIgnoredActor(MyOwner);
                QueryParams.AddIgnoredActor(this);
                QueryParams.bTraceComplex = true;
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
                        ActualDamage *= HeadshotMultiplier;
                    }

                    UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit,
                                                       MyOwner->GetInstigatorController(), MyOwner, DamageType);

                    TraceEndPoint = Hit.ImpactPoint;
                }

                if (DebugWeaponDrawingShotG > 0)
                {
                    DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 1.0f, 0, 1.0f);
                }

                PlayFireFX();

                ClientWeaponRecoil();

                if (Hit.GetActor())
                {
                    PlayImpactFX(SurfaceType, TraceEndPoint);
                }

                ClientUpdateAmmo();
            }
        }

        bHasTriggered = true;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AShotgun::ResetTrigger()
{
    bHasTriggered = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AShotgun::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AShotgun, bHasTriggered);
}
