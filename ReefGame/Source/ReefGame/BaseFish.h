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

	void TickSwim();

	void CheckForNearbyFish();

	UPROPERTY(EditAnywhere)
	EFishState CurrentState = EFishState::Swim;

	UPROPERTY(VisibleAnywhere)
	UHealthComponent* HealthComponent;

	float LastSensedTime = 0.0f;
	float SensingCooldown = 1.0f;

	UPROPERTY()
	TArray<ABaseFish*> FishInRadius;

	UPROPERTY()
	AFishSchoolController* FishSchoolController;

	FVector Cohere();
	FVector Separate();
	FVector Align();

	FVector FlockingSteering();

	UPROPERTY()
	float MaxForce = 50.0f;
	UPROPERTY()
	float MaxSpeed = 300.0f;
	
	FVector Acceleration = FVector::ZeroVector;

	FVector RandomDirection = FVector::ZeroVector;
	float LastDirectionChangeTime = 0.0f;
	UPROPERTY(EditAnywhere)
	float DirectionChangeInterval = 2.0f;

public:	

	virtual void Tick(float DeltaTime) override;
	
	void SmoothMovementInDirection(FVector Direction, float DeltaTime);
	
};
