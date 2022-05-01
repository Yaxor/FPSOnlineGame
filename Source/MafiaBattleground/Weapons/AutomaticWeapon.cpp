//+--------------------------------------------------------+
//| Project    : MafiaBattleground                         |
//| UE Version : 4.27                                      |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+


#include "AutomaticWeapon.h"

//------------------------------------------------------------------------------------------------------------------------------------------
AAutomaticWeapon::AAutomaticWeapon()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
void AAutomaticWeapon::StartFire()
{
    float FirstDelay = FMath::Max(LastFireTime + Cadence - GetWorld()->TimeSeconds, 0.0f);

    GetWorldTimerManager().SetTimer(TimerHandle_Fire, this, &AWeapon::Fire, Cadence, true, FirstDelay);
}
