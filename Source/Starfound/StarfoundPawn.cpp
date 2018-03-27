// Fill out your copyright notice in the Description page of Project Settings.

#include "StarfoundPawn.h"
#include "StarfoundAIController.h"

// Sets default values
AStarfoundPawn::AStarfoundPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = AStarfoundAIController::StaticClass();
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
