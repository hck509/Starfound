// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlockActor.h"
#include "StarfoundPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class STARFOUND_API AStarfoundPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	void StartCreating(TSubclassOf<ABlockActor> BlockClass);

	UFUNCTION(BlueprintCallable)
	void CreateBlockOnCurrentPosition();

	virtual void Tick(float DeltaSeconds) override;
	virtual bool InputKey(FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad) override;

private:
	
	UPROPERTY()
	TSubclassOf<ABlockActor> CreatingBlockClass;

	UPROPERTY()
	ABlockActor* CreatingBlockActor;
};
