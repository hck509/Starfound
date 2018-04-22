#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemActor.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	None,
	Dirt
};

UCLASS()
class STARFOUND_API AItemActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AItemActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void InitItem(EItemType InItemType);

	UFUNCTION(BlueprintCallable)
	EItemType GetItemType() const { return ItemType; }

private:
	UPROPERTY()
	EItemType ItemType;
};
