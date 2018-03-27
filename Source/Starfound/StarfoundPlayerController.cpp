// Fill out your copyright notice in the Description page of Project Settings.

#include "StarfoundPlayerController.h"
#include "DrawDebugHelpers.h"
#include "StarfoundGameMode.h"

void AStarfoundPlayerController::StartConstruct(TSubclassOf<ABlockActor> BlockClass)
{
	if (!ensure(BlockClass.Get()))
	{
		return;
	}

	if (CreatingBlockActor)
	{
		CreatingBlockActor->Destroy();
		CreatingBlockActor = nullptr;
	}

	CreatingBlockClass = BlockClass;

	CreatingBlockActor = GetWorld()->SpawnActorDeferred<ABlockActor>(BlockClass, FTransform::Identity, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	CreatingBlockActor->SetTemporal(true);
	CreatingBlockActor->FinishSpawning(FTransform::Identity);

	CreatingBlockActor->GetRootComponent()->DestroyPhysicsState();

	ActiveToolType = EToolType::Construct;
}

void AStarfoundPlayerController::ConstructBlock()
{
	if (!ensure(ActiveToolType == EToolType::Construct) || !ensure(CreatingBlockActor) || !ensure(CreatingBlockClass))
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ABlockActor* NewBlockActor = GetWorld()->SpawnActor<ABlockActor>(CreatingBlockClass, CreatingBlockActor->GetActorTransform(), SpawnParameters);
}

void AStarfoundPlayerController::StartDestruct()
{
	ActiveToolType = EToolType::Destruct;

	if (CreatingBlockActor)
	{
		CreatingBlockActor->Destroy();
		CreatingBlockActor = nullptr;
	}
}

void AStarfoundPlayerController::DestructBlock()
{
	if (!ensure(ActiveToolType == EToolType::Destruct))
	{
		return;
	}

	FHitResult HitResult;
	bool bHitFound = GetHitResultUnderCursor(ECC_WorldStatic, true, HitResult);

	if (bHitFound && HitResult.Actor.IsValid() && HitResult.Actor->IsA(ABlockActor::StaticClass()))
	{
		HitResult.Actor->Destroy();
	}
}

void AStarfoundPlayerController::MoveToCursorLocation()
{
	APawn* Pawn = GetPawn();
	if (!Pawn)
	{
		return;
	}

	AStarfoundGameMode* GameMode = Cast<AStarfoundGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		return;
	}

	const FVector PawnLocation = Pawn->GetActorLocation();
	const FVector CursorLocation = GetCursorLocation();

	TArray<FVector2D> PathPoints;
	const bool bPathFound = GameMode->GetNavigation()->FindPath(PawnLocation, CursorLocation, PathPoints);

	DrawDebugPoint(GetWorld(), PawnLocation + FVector(150, 0, 0), 40.0f, FColor::Blue, false, 5.0f);
	DrawDebugPoint(GetWorld(), CursorLocation + FVector(150, 0, 0), 40.0f, FColor::Blue, false, 5.0f);

	if (bPathFound)
	{
		for (const FVector2D& Point : PathPoints)
		{
			DrawDebugPoint(GetWorld(), FVector(150, Point.X, Point.Y), 30.0f, FColor::Red, false, 5.0f);
		}
	}
}

FVector AStarfoundPlayerController::GetCursorLocation()
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

			return MouseWorldPositionAtPlane;
		}
	}

	return FVector::ZeroVector;
}

void AStarfoundPlayerController::BeginPlay()
{
	ActiveToolType = EToolType::None;

	Super::BeginPlay();
}

void AStarfoundPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (ActiveToolType == EToolType::Construct)
	{
		FVector CursorLocation = GetCursorLocation();

		if (!CursorLocation.IsZero())
		{
			CreatingBlockActor->SetActorLocation(CursorLocation);
		}
	}
	else if (ActiveToolType == EToolType::Destruct)
	{
		FHitResult HitResult;
		bool bHitFound = GetHitResultUnderCursor(ECC_WorldStatic, true, HitResult);

		if (bHitFound && HitResult.Actor.IsValid() && HitResult.Actor->IsA(ABlockActor::StaticClass()))
		{
			DrawDebugBox(GetWorld(), HitResult.Actor->GetActorLocation(), FVector(50, 50, 50), FColor::White);
		}
	}
}

bool AStarfoundPlayerController::InputKey(FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad)
{
	Super::InputKey(Key, EventType, AmountDepressed, bGamepad);

	if (Key == EKeys::C)
	{
		ensure(CreatingBlockClass);
		StartConstruct(CreatingBlockClass);
	}
	else if (Key == EKeys::D)
	{
		StartDestruct();
	}
	else if (Key == EKeys::Q)
	{
		ActiveToolType = EToolType::None;

		if (CreatingBlockActor)
		{
			CreatingBlockActor->Destroy();
			CreatingBlockActor = nullptr;
		}
	}

	if (Key == EKeys::LeftMouseButton)
	{
		if (ActiveToolType == EToolType::Construct)
		{
			ConstructBlock();
			return true;
		}
		else if (ActiveToolType == EToolType::Destruct)
		{
			DestructBlock();
			return true;
		}
		else
		{
			MoveToCursorLocation();
			return true;
		}
	}

	return false;
}
