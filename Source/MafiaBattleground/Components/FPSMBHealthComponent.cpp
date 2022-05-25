//+--------------------------------------------------------+
//| Project    : MafiaBattleground                         |
//| UE Version : 4.27                                      |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+


#include "FPSMBHealthComponent.h"

//------------------------------------------------------------------------------------------------------------------------------------------
UFPSMBHealthComponent::UFPSMBHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
void UFPSMBHealthComponent::BeginPlay()
{
    Super::BeginPlay();

}

//------------------------------------------------------------------------------------------------------------------------------------------
void UFPSMBHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}
