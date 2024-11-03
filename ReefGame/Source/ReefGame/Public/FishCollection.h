#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "REEFGAME/BaseFish.h"
#include "FishCollection.generated.h"

UCLASS()
class REEFGAME_API AFishCollection : public AGameState
{
	GENERATED_BODY()

public:
	AFishCollection();

	// Array to track collected fish types
	UPROPERTY(ReplicatedUsing = OnRep_CollectedFish)
	TArray<EFishType> CollectedFish;

	// Function to add a fish to the collection
	UFUNCTION()
	void AddFishToCollection(EFishType FishType);

	// Function called when CollectedFish array is updated
	UFUNCTION()
	void OnRep_CollectedFish();

	// Check if a fish has been collected
	UFUNCTION(BlueprintCallable, Category = "Fish Collection")
	bool IsFishCollected(EFishType FishType) const;

	UPROPERTY()
	EFishType CurrentCollectedFish;
	
};
