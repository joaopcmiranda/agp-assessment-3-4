#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "PlayerPerceptionSensor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHighlightedFishChanged, class ABaseFish*, NewHighlightedFish);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class REEFGAME_API UPlayerPerceptionSensor : public USphereComponent
{
	GENERATED_BODY()

public:
	UPlayerPerceptionSensor();

	/** Event called when the highlighted fish changes */
	UPROPERTY(BlueprintAssignable, Category = "Perception")
	FOnHighlightedFishChanged OnHighlightedFishChanged;

	// Function to get the currently highlighted fish
	ABaseFish* GetCurrentlyHighlightedFish() const { return CurrentlyHighlightedFish; }

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

	// Overlap event functions
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
						const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
					  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// List of overlapping ABaseFish actors
	UPROPERTY()
	TArray<class ABaseFish*> OverlappingFish;

	// Function to find and highlight the closest fish in front of the player
	void HighlightClosestFish();

	// Pointer to the currently highlighted fish
	UPROPERTY()
	class ABaseFish* CurrentlyHighlightedFish;

	// Field of view angle in degrees
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	float FieldOfViewAngle = 90.0f;
};

