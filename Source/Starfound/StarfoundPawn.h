#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "StarfoundPawn.generated.h"

UCLASS()
class STARFOUND_API AStarfoundPawn : public APawn
{
	GENERATED_BODY()

public:
	AStarfoundPawn();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	class AStarfoundAIController* GetAIController() const;

	UFUNCTION(BlueprintCallable)
	class UStarfoundMovementComponent* GetStarfoundMovementController() const;

	float GetWorkPercentagePerSeconds() const { return WorkPercentagePerSeconds; }

private:
	UPROPERTY()
	UStarfoundMovementComponent* MovementComponent;

	UPROPERTY(EditDefaultsOnly)
	float WorkPercentagePerSeconds;
};
