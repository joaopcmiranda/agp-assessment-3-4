#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FishInfoWidget.generated.h"

UCLASS()
class REEFGAME_API UFishInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Function to set the fish type and update the UI
	UFUNCTION(BlueprintCallable, Category = "Fish Info")
	void SetFishType(EFishType FishType);

protected:
	// Fish type being displayed
	UPROPERTY(BlueprintReadOnly, Category = "Fish Info")
	EFishType CurrentFishType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish Info")
	UDataTable* FishInfoDataTable;

	// Function to update the UI elements
	UFUNCTION(BlueprintImplementableEvent, Category = "Fish Info")
	void UpdateFishInfoUI();
};