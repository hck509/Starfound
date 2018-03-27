// Fill out your copyright notice in the Description page of Project Settings.

#include "StarfoundPlayerController.h"
#include "DrawDebugHelpers.h"
#include "StarfoundGameMode.h"
#include "StarfoundAIController.h"

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
	bool bHitFound = GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_WorldStatic), true, HitResult);

	if (bHitFound && HitResult.Actor.IsValid() && HitResult.Actor->IsA(ABlockActor::StaticClass()))
	{
		HitResult.Actor->Destroy();
	}
}

void AStarfoundPlayerController::MoveToCursorLocation()
{
	if (!SelectedPawn)
	{
		return;
	}

	SelectedPawn->GetAIController()->MoveToLocation(GetCursorLocation());
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

bool AStarfoundPlayerController::TrySelectPawnOnCursorLocation()
{
	FHitResult HitResult;
	bool bHitFound = GetHitResultUnderCursor(ECC_Pawn, true, HitResult);

	if (bHitFound && HitResult.Actor.IsValid() && HitResult.Actor->IsA(AStarfoundPawn::StaticClass()))
	{
		SelectedPawn = Cast<AStarfoundPawn>(HitResult.Actor.Get());
		return true;
	}

	return false;
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
			DrawDebugBox(GetWorld(), HitResult.Actor->GetActorLocation(), FVector(50, 50, 50), FColor::Green);
		}
	}
	else
	{
		if (SelectedPawn)
		{
			DrawDebugBox(GetWorld(), SelectedPawn->GetActorLocation(), FVector(50, 50, 50), FColor::Green);
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
			if (SelectedPawn)
			{
				MoveToCursorLocation();
				return true;
			}
			else
			{
				const bool bPicked = TrySelectPawnOnCursorLocation();
				if (bPicked)
				{
					return true;
				}
			}
		}
	}

	return false;
}
