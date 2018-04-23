#include "StarfoundPawn.h"
#include "StarfoundAIController.h"
#include "StarfoundMovementComponent.h"
#include "StarfoundGameMode.h"
#include "DrawDebugHelpers.h"

// Sets default values
AStarfoundPawn::AStarfoundPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = AStarfoundAIController::StaticClass();

	MovementComponent = CreateDefaultSubobject<UStarfoundMovementComponent>("StarfoundMovement");

	WorkPercentagePerSeconds = 50.0f;
}

// Called when the game starts or when spawned
void AStarfoundPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AStarfoundPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DrawDebugString(GetWorld(), FVector::ZeroVector, FString::Printf(TEXT("Inven:%d"), Inventory.Items.Num()), this, FColor::White, 0, true);
}

// Called to bind functionality to input
void AStarfoundPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

class AStarfoundAIController* AStarfoundPawn::GetAIController() const
{
	return Cast<AStarfoundAIController>(GetController());
}

class UStarfoundMovementComponent* AStarfoundPawn::GetStarfoundMovementController() const
{
	return MovementComponent;
}

bool AStarfoundPawn::IsItemActorInRangeToPickup(const AItemActor* ItemActor) const
{
	UBlockActorScene* BlockScene = GetBlockActorScene(GetWorld());

	if (!BlockScene)
	{
		return false;
	}

	const FIntPoint PawnLocation = BlockScene->WorldSpaceToOriginSpaceGrid(GetActorLocation());
	const FIntPoint ItemLocation = BlockScene->WorldSpaceToOriginSpaceGrid(ItemActor->GetActorLocation());

	const FIntPoint LocationDiff = PawnLocation - ItemLocation;

	if (FMath::Abs(LocationDiff.X) <= 1 && FMath::Abs(LocationDiff.Y) <= 3)
	{
		return true;
	}

	return false;
}

bool AStarfoundPawn::PickupItemActor(AItemActor* ItemActor)
{
	if (!ensure(ItemActor))
	{
		return false;
	}

	if (!ensure(IsItemActorInRangeToPickup(ItemActor)))
	{
		return false;
	}

	const int32 NumItemsToAdd = 1;

	int32* Value = Inventory.Items.Find(ItemActor->GetItemType());
	if (Value)
	{
		*Value += NumItemsToAdd;
	}
	else
	{
		Inventory.Items.Add(ItemActor->GetItemType(), NumItemsToAdd);
	}

	ItemActor->Destroy();

	return true;
}
