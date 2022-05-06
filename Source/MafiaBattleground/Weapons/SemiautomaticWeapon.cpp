//+--------------------------------------------------------+
//| Project    : MafiaBattleground                         |
//| UE Version : 4.27                                      |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+


#include "SemiautomaticWeapon.h"
#include "Net/UnrealNetwork.h"

//------------------------------------------------------------------------------------------------------------------------------------------
ASemiautomaticWeapon::ASemiautomaticWeapon()
{
    bHasTriggered = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ASemiautomaticWeapon::StartFire()
{
    Super::StartFire();

    Fire();
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ASemiautomaticWeapon::Fire()
{
    if (!bHasTriggered)
    {
        GetWorldTimerManager().SetTimer(TimerHandle_ResetTrigger, this, &ASemiautomaticWeapon::ResetTrigger, Cadence, false);
        Super::Fire();
        bHasTriggered = true;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ASemiautomaticWeapon::ResetTrigger()
{
    bHasTriggered = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ASemiautomaticWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASemiautomaticWeapon, bHasTriggered);
}
