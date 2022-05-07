// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Net/UnrealNetwork.h"

#include "Camera/CameraComponent.h"

#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"

#include "MafiaBattleground/Player/FPSMBCharacter.h"
#include "MafiaBattleground/MafiaBattleground.h"

//------------------------------------------------------------------------------------------------------------------------------------------
AShotgun::AShotgun()
{
    bHasTriggered  = false;
    BulletSpread   = 8.0f;
    AimSpreadBoost = 4.0f;
    Pellets        = 8;
    FireRate       = 9.0f;
    RecoilForce    = 4.0f;
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

            const FVector& StartLocation = MyFPSPlayer->GetCamera()->GetComponentLocation();
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
                        GEngine->AddOnScreenDebugMessage(i, 2.0f, FColor::Red, TEXT("Headshot"));
                        ActualDamage *= HeadshotMultiplier;
                    }

                    UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit,
                                                       MyOwner->GetInstigatorController(), MyOwner, DamageType);

                    TraceEndPoint = Hit.ImpactPoint;
                }

                PlayFireFX();

                if (MyFPSPlayer->GetIsAiming())
                {
                    WeaponRecoil_Delay();
                }

                if (Hit.GetActor())
                {
                    PlayImpactFX(SurfaceType, TraceEndPoint);
                }
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
void AShotgun::CustomWeaponRecoil()
{
    APawn* MyOwner = Cast<APawn>(GetOwner());
    if (MyOwner)
    {
        APlayerController* PlayerController = Cast<APlayerController>(MyOwner->GetController());
        if (PlayerController)
        {
            PlayerController->AddPitchInput(RecoilForce);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AShotgun::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AShotgun, bHasTriggered);
}
