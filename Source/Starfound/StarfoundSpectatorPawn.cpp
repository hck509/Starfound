#include "StarfoundSpectatorPawn.h"

AStarfoundSpectatorPawn::AStarfoundSpectatorPawn()
{
	bAddDefaultMovementBindings = 0;
}

void AStarfoundSpectatorPawn::SetupPlayerInputComponent(UInputComponent* InInputComponent)
{
	Super::SetupPlayerInputComponent(InInputComponent);

	InInputComponent->BindAxis("Camera_Forward", this, &ADefaultPawn::MoveForward);
	InInputComponent->BindAxis("Camera_MoveRight", this, &ADefaultPawn::MoveRight);
	InInputComponent->BindAxis("Camera_MoveUp", this, &ADefaultPawn::MoveUp_World);
}
