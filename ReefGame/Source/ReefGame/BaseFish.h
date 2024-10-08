// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseFish.generated.h"

class UHealthComponent;
class AFishSchoolController;
class UStaticMeshComponent;
class USphereComponent;

UENUM(BlueprintType)
enum class EFishType : uint8
{
	BaseFish UMETA(DisplayName = "Base Fish"),
	FishTypeA UMETA(DisplayName = "Type A"),
	FIshTypeB UMETA(DisplayName = "Type B"),
	Shark UMETA(DisplayName = "Shark")
};

UENUM(BlueprintType)
enum class EFishState : uint8
{
	Roam,
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

//State
	EFishState CurrentState = EFishState::Roam;
	
//Components
	UPROPERTY(VisibleAnywhere)
	USphereComponent* FishCollision;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* FishMesh;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* PerceptionSensor;

	UPROPERTY(VisibleAnywhere)
	UHealthComponent* HealthComponent;

//Fish in radius
	UPROPERTY()
	TArray<AActor*> FishInRadius;

	UPROPERTY()
	TArray<AActor*> FishOfSameType;

	UPROPERTY()
	ABaseFish* Predator = nullptr;
	
	void UpdateFishInRadius();
	void UpdateFishTypes();

//Movement

	FVector Velocity;
	FRotator CurrentRotation;
	
	float MaxSpeed = 3000.0f;
	float MinSpeed = 2000.0f;

//MovementFunctions	

	void UpdateMeshRotation();

	FVector Cohere(TArray<AActor*> School);
	FVector Separate(TArray<AActor*> School);
	FVector Align(TArray<AActor*> School);

	virtual void AvoidPredator(float DeltaTime);

	virtual void Steer(float DeltaTime);

	void CapMovementArea();
	
//Obstacle Avoidance

	bool IsObstacle();
	FVector AvoidObstacle();

	TArray<FVector> TargetForces;

//Perception
	float FOV = FMath::Cos(FMath::DegreesToRadians(120.0f));
	float FlockingDelay = 0.0f;


//Weighting
	float CoherenceStrength = 1.5f;
	float SeparationStrength = 1.6f;
	float AlignmentStrength = 1.5f;


public:	

	virtual void Tick(float DeltaTime) override;
	
	void AddTargetForce(FVector TargetForce);

	virtual EFishType GetFishType() const;
	
};
