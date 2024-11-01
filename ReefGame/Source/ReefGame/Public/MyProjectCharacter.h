#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h" // For FInputActionValue
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

	// Input Actions and Mapping Context
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* LookAction;

public:
	virtual void Tick(float DeltaTime) override;
};

