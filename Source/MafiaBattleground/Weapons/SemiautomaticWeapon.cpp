//+--------------------------------------------------------+
//| Project    : MafiaBattleground                         |
//| UE Version : 4.27                                      |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+


#include "SemiautomaticWeapon.h"

//------------------------------------------------------------------------------------------------------------------------------------------
ASemiautomaticWeapon::ASemiautomaticWeapon()
{
    bHasTriggered = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ASemiautomaticWeapon::StartFire()
{
    Fire();
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ASemiautomaticWeapon::StopFire()
{
    Super::StopFire();

    bHasTriggered = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ASemiautomaticWeapon::Fire()
{
    if (!bHasTriggered)
    {
        GetWorldTimerManager().SetTimer(TimerHandle_Fire, this, &ASemiautomaticWeapon::StopFire, Cadence, false);
        Super::Fire();
        bHasTriggered = true;
    }
}
