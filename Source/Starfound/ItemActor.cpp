#include "ItemActor.h"

AItemActor::AItemActor()
{
	PrimaryActorTick.bCanEverTick = true;

	ItemType = EItemType::None;
}

void AItemActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AItemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AItemActor::InitItem(EItemType InItemType)
{
	ItemType = InItemType;
}

