#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "MyProjectCharacter.generated.h"

UCLASS()
class REEFGAME_API AMyProjectCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMyProjectCharacter();

	void ShowCollectionNotification(EFishType FishType);

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Movement and Look input functions
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact();

	// Input Actions and Mapping Context
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* OpenCollectionMenuAction;

	void ToggleCollectionMenu();
	void InitializeCollectionMenu();

	// Perception Sensor Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception")
	class UPlayerPerceptionSensor* PerceptionSensor;

	// Interact Prompt Widget Class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> InteractPromptWidgetClass;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_AddFishToCollection(EFishType FishType);

	// Function to show fish info UI
	void ShowFishInfoWidget(EFishType FishType);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> FishInfoWidgetClass;

	// Add a pointer to the widget instance
	UUserWidget* FishInfoWidgetInstance;

	UPROPERTY()
	UUserWidget* CollectionNotificationInstance;

	UPROPERTY()
	UUserWidget* HowToOpenCollectionMenuInstance;
	
	void HideCollectionNotification();

	FTimerHandle NotificationTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> CollectionNotificationClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> HowToOpenCollectionMenuClass;

private:
	// Instance of the Interact Prompt Widget
	UPROPERTY()
	UUserWidget* InteractPromptWidgetInstance;

	// Flag to track visibility
	bool bIsInteractPromptVisible;

	bool bIsFishInfoWidgetDisplayed;

	void HideFishInfoWidget();

	// Function to handle highlighted fish changes
	UFUNCTION()
	void OnHighlightedFishChanged(class ABaseFish* NewHighlightedFish);

	// Functions to show/hide the interact prompt
	void ShowInteractPrompt();
	void HideInteractPrompt();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> CollectionMenuClass;

public:
	virtual void Tick(float DeltaTime) override;

	UUserWidget* CollectionMenuInstance;
};
