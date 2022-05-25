//+--------------------------------------------------------+
//| Project    : MafiaBattleground                         |
//| UE Version : 4.27                                      |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FPSMBHealthComponent.generated.h"

// On Health Change Event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, UFPSMBHealthComponent*, HealthComp, float, Health, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAFIABATTLEGROUND_API UFPSMBHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    //!Constructor
    UFPSMBHealthComponent();

    //*******************************************************************************************************************
    //                                          PUBLIC VARIABLES                                                        *
    //*******************************************************************************************************************

    /* Delegate to make damage to my owner */
    UPROPERTY(BlueprintAssignable, Category = Events)
    FOnHealthChangedSignature OnHealthChanged;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = HealthComponent)
    uint8 TeamNum;

protected:
    //*******************************************************************************************************************
    //                                          VARIABLES                                                               *
    //*******************************************************************************************************************

    /* Can do damage to all players */
    UPROPERTY(EditDefaultsOnly)
    bool bFriendlyDamage;
    bool bIsDead;

    UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = HealthComponent)
    float Health;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = HealthComponent)
    float MaxHealth;

public:
    //*******************************************************************************************************************
    //                                          PUBLIC FUNCTIONS                                                        *
    //*******************************************************************************************************************

    float GetHealth() const;

    /* Healt my owner */
    UFUNCTION(BlueprintCallable, Category = HealthComponent)
    void Heal(float HealAmount);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = HealthComponent)
    static bool IsFriendly(AActor* ActorA, AActor* ActorB);

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED FUNCTIONS                                                     *
    //*******************************************************************************************************************

    virtual void BeginPlay() override;

    UFUNCTION()
    void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
                             class AController* InstigatedBy, AActor* DamageCauser);

    UFUNCTION()
    void OnRep_Health(float OldHealth);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
