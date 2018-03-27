// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlockActor.h"
#include "StarfoundPawn.h"
#include "StarfoundPlayerController.generated.h"

UENUM()
enum class EToolType
{
	None,
	Construct,
	Destruct
};


/**
 * 
 */
UCLASS()
class STARFOUND_API AStarfoundPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual bool InputKey(FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad) override;
	
	UFUNCTION(BlueprintCallable)
	void StartConstruct(TSubclassOf<ABlockActor> BlockClass);

	UFUNCTION(BlueprintCallable)
	void ConstructBlock();

	UFUNCTION(BlueprintCallable)
	void StartDestruct();

	UFUNCTION(BlueprintCallable)
	void DestructBlock();

private:

	void MoveToCursorLocation();
	FVector GetCursorLocation();

	bool TrySelectPawnOnCursorLocation();
	
	UPROPERTY()
	EToolType ActiveToolType;

	UPROPERTY()
	TSubclassOf<ABlockActor> CreatingBlockClass;

	UPROPERTY()
	ABlockActor* CreatingBlockActor;

	UPROPERTY()
	AStarfoundPawn* SelectedPawn;
};
