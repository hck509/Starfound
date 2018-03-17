// Fill out your copyright notice in the Description page of Project Settings.

#include "StarfoundPlayerController.h"


void AStarfoundPlayerController::StartCreating(TSubclassOf<ABlockActor> BlockClass)
{
	CreatingBlockClass = BlockClass;

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CreatingBlockActor = GetWorld()->SpawnActor<ABlockActor>(BlockClass, FTransform::Identity, SpawnParameters);
	CreatingBlockActor->GetRootComponent()->DestroyPhysicsState();
}

void AStarfoundPlayerController::CreateBlockOnCurrentPosition()
{
	if (!ensure(CreatingBlockActor) || !ensure(CreatingBlockClass))
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ABlockActor* NewBlockActor = GetWorld()->SpawnActor<ABlockActor>(CreatingBlockClass, CreatingBlockActor->GetActorTransform(), SpawnParameters);
}

void AStarfoundPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CreatingBlockActor)
	{
		float MousePositionX, MousePositionY;
		const bool bValidMousePosition = GetMousePosition(MousePositionX, MousePositionY);

		if (bValidMousePosition)
		{
			FVector MouseWorldPosition, MouseWorldDirection;
			const bool bValidProjection = DeprojectScreenPositionToWorld(MousePositionX, MousePositionY, MouseWorldPosition, MouseWorldDirection);

			if (bValidProjection)
			{
				const float DistanceToPlane = MouseWorldPosition.X / (MouseWorldDirection | FVector(-1, 0, 0));
				FVector MouseWorldPositionAtPlane = MouseWorldPosition + (MouseWorldDirection * DistanceToPlane);

				// Snap to grid
				MouseWorldPositionAtPlane.Y = FMath::GridSnap(MouseWorldPositionAtPlane.Y, 100);
				MouseWorldPositionAtPlane.Z = FMath::GridSnap(MouseWorldPositionAtPlane.Z, 100);

				CreatingBlockActor->SetActorLocation(MouseWorldPositionAtPlane);
			}
		}
	}
}

bool AStarfoundPlayerController::InputKey(FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad)
{
	Super::InputKey(Key, EventType, AmountDepressed, bGamepad);

	if (Key == EKeys::LeftMouseButton && CreatingBlockActor)
	{
		CreateBlockOnCurrentPosition();

		return true;
	}

	return false;
}
