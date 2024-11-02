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


	// Perception Sensor Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception")
	class UPlayerPerceptionSensor* PerceptionSensor;

	// Interact Prompt Widget Class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> InteractPromptWidgetClass;

private:
	// Instance of the Interact Prompt Widget
	UPROPERTY()
	UUserWidget* InteractPromptWidgetInstance;

	// Flag to track visibility
	bool bIsInteractPromptVisible;

	// Function to handle highlighted fish changes
	UFUNCTION()
	void OnHighlightedFishChanged(class ABaseFish* NewHighlightedFish);

	// Functions to show/hide the interact prompt
	void ShowInteractPrompt();
	void HideInteractPrompt();

public:
	virtual void Tick(float DeltaTime) override;
};
