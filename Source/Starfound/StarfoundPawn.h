#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ItemActor.h"
#include "StarfoundPawn.generated.h"

USTRUCT(BlueprintType)
struct FStarfoundInventory
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TMap<EItemType, int32> Items;
};

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

	UFUNCTION(BlueprintCallable)
	FStarfoundInventory& GetInventory() { return Inventory; }

	UFUNCTION(BlueprintCallable)
	const FStarfoundInventory& GetInventoryConst() const { return Inventory; }

	UFUNCTION(BlueprintCallable)
	bool IsItemActorInRangeToPickup(const AItemActor* ItemActor) const;

	UFUNCTION(BlueprintCallable)
	bool PickupItemActor(AItemActor* ItemActor);

private:
	UPROPERTY()
	UStarfoundMovementComponent* MovementComponent;

	UPROPERTY(EditDefaultsOnly)
	float WorkPercentagePerSeconds;

	UPROPERTY()
	FStarfoundInventory Inventory;
};
