//+--------------------------------------------------------+
//| Project    : MafiaBattleground                         |
//| UE Version : 4.27                                      |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+


#include "FPSMBHealthComponent.h"

#include "Net/UnrealNetwork.h"

#include "Kismet/KismetSystemLibrary.h"

#include "MafiaBattleground/Player/FPSMBCharacter.h"
#include "MafiaBattleground/Player/FPSMBPlayerController.h"
#include "MafiaBattleground/HUD/MBFPSMainHUD.h"

//------------------------------------------------------------------------------------------------------------------------------------------
UFPSMBHealthComponent::UFPSMBHealthComponent()
{
    bFriendlyDamage = true;
    bIsDead         = false;
    MaxHealth       = 100.0f;
    TeamNum         = 255;

    SetIsReplicatedByDefault(true);
}

//------------------------------------------------------------------------------------------------------------------------------------------
float UFPSMBHealthComponent::GetHealth() const
{
    return Health;
}

//------------------------------------------------------------------------------------------------------------------------------------------
void UFPSMBHealthComponent::Heal(float HealAmount)
{
    if ((HealAmount <= 0.0f) || (Health <= 0.0f) || (bIsDead) || (Health == MaxHealth))
    {
        return;
    }

    Health = FMath::Clamp(Health + HealAmount, 0.0f, MaxHealth);

    //GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("Get Heal: %s (+%s)"), *FString::SanitizeFloat(Health), *FString::SanitizeFloat(HealAmount)));

    OnHealthChanged.Broadcast(this, Health, (-HealAmount), nullptr, nullptr, nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool UFPSMBHealthComponent::IsFriendly(AActor* ActorA, AActor* ActorB)
{
    if (ActorA == nullptr || ActorB == nullptr)
    {
        // Assume friendly
        return true;
    }

    UFPSMBHealthComponent* HealthCompA = Cast<UFPSMBHealthComponent>(ActorA->GetComponentByClass(UFPSMBHealthComponent::StaticClass()));
    UFPSMBHealthComponent* HealthCompB = Cast<UFPSMBHealthComponent>(ActorB->GetComponentByClass(UFPSMBHealthComponent::StaticClass()));

    if (HealthCompA == nullptr || HealthCompB == nullptr)
    {
        // Assume friendly
        return true;
    }

    // Else Compare TeamNum
    return (HealthCompA->TeamNum == HealthCompB->TeamNum);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void UFPSMBHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    Health = MaxHealth;

    AFPSMBCharacter* MyCharacter = Cast<AFPSMBCharacter>(GetOwner());
    if (MyCharacter)
    {
        if (MyCharacter->GetIsServer())
        {
            // When i receibe any damage call HandleTakeAnyDamage
            MyCharacter->OnTakeAnyDamage.AddDynamic(this, &UFPSMBHealthComponent::HandleTakeAnyDamage);
        }
    }

    GetMyHUD_Delay();
}

//------------------------------------------------------------------------------------------------------------------------------------------
void UFPSMBHealthComponent::GetMyHUD_Delay()
{
    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget    = this;
    LatentInfo.ExecutionFunction = FName("GetMyHUD");
    LatentInfo.Linkage           = 0;
    LatentInfo.UUID              = 0;

    UKismetSystemLibrary::Delay(GetWorld(), 0.02f, LatentInfo); //0.1f
}

//------------------------------------------------------------------------------------------------------------------------------------------
void UFPSMBHealthComponent::GetMyHUD()
{
    if (AFPSMBCharacter* MyCharacter = Cast<AFPSMBCharacter>(GetOwner()))
    {
        if (MyCharacter->IsLocallyControlled())
        {
            if (AFPSMBPlayerController* MyController = Cast<AFPSMBPlayerController>(MyCharacter->GetController()))
            {
                if (AMBFPSMainHUD* MainHUD = Cast<AMBFPSMainHUD>(MyController->GetHUD()))
                {
                    MyMainHUD = MainHUD;
                }
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void UFPSMBHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
    if (Damage <= 0.0f || bIsDead)
    {
        return;
    }

    // Check if FriendlyDamage is true and the damage causer is my teammate or if the damage causer its myself
    if ((!bFriendlyDamage) && (DamageCauser != DamagedActor) && IsFriendly(DamagedActor, DamageCauser) || (DamageCauser == DamagedActor))
    {
        return;
    }

    // Update Health Clamped
    Health  = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);
    bIsDead = (Health <= 0.0f);

    ClientDamageFeedback();

    //GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("Get Damage: %s (-%s)"), *FString::SanitizeFloat(Health), *FString::SanitizeFloat(Damage)));

    OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void UFPSMBHealthComponent::ClientDamageFeedback_Implementation()
{
    if (MyMainHUD)
    {
        MyMainHUD->DamageFeedback();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void UFPSMBHealthComponent::OnRep_Health(float OldHealth)
{
    float DeltaHealth = Health - OldHealth;

    OnHealthChanged.Broadcast(this, Health, DeltaHealth, nullptr, nullptr, nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void UFPSMBHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UFPSMBHealthComponent, Health);
}
