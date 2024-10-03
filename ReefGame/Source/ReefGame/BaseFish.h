// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseFish.generated.h"

class UHealthComponent;
class AFishSchoolController;
class UPathfindingSubsystem;
class ABaseSchool;

UENUM(BlueprintType)
enum class EFishState : uint8
{
	Swim,
	SwimGroup,
	Evade
};

UCLASS()
class REEFGAME_API ABaseFish : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseFish();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	float SensingRadius = 2000.0f;

	void MoveAlongPath();

	void TickSwim();

	void CheckForNearbyFish();

	UPROPERTY()
	AFishSchoolController* FishSchoolController = nullptr;

	//UFUNCTION()
	//void OnSensedPawn(APawn* SensedActor);

	//void UpdateSight();

	UPROPERTY()
	UPathfindingSubsystem* PathfindingSubsystem;

	//UPROPERTY(VisibleAnywhere)
	//UPawnSensingComponent* PawnSensingComponent;

	UPROPERTY()
	ABaseFish* SensedFish = nullptr;

	UPROPERTY(VisibleAnywhere)
	TArray<FVector> CurrentPath;

	UPROPERTY(EditAnywhere)
	EFishState CurrentState = EFishState::Swim;

	UPROPERTY(EditAnywhere)
	float PathfindingError = 150.0f;

	UPROPERTY(VisibleAnywhere)
	UHealthComponent* HealthComponent;

	void AddFishToSchool();

	bool bIsInSchool = false;

	UPROPERTY()
	ABaseSchool* SchoolFishIsIn = nullptr;

	float LastSensedTime = 0.0f;
	float SensingCooldown = 1.0f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void SmoothMovement(FVector TargetLocation, float DeltaTime);
};
