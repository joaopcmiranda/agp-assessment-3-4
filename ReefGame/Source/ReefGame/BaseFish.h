// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NiagaraSystem.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseFish.generated.h"

class UHealthComponent;
class UStaticMeshComponent;
class USphereComponent;

UENUM(BlueprintType)
enum class EFishType : uint8
{
	BaseFish UMETA(DisplayName = "Base Fish"),
	FishTypeA UMETA(DisplayName = "Type A"),
	FishTypeB UMETA(DisplayName = "Type B"),
	Shark UMETA(DisplayName = "Shark")
};

UENUM(BlueprintType)
enum class EFishState : uint8
{
	Roam UMETA(DisplayName = "Roam"),
	Evade UMETA(DisplayName = "Evade"),
	Hunt UMETA(DisplayName = "Hunt")
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

	EFishType FishType = EFishType::BaseFish;
	
//Components
	UPROPERTY(VisibleAnywhere)
	USphereComponent* FishCollision;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* FishMesh;

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

	UPROPERTY()
	ABaseFish* Prey = nullptr;

	UPROPERTY()
	EFishType PredatorType = EFishType::Shark;

	UPROPERTY()
	EFishType PreyType = EFishType::FishTypeB;
	
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

	virtual void Hunt(float DeltaTime);

	void CapMovementArea();
	
//Obstacle Avoidance

	bool IsObstacle();
	FVector AvoidObstacle();

	TArray<FVector> TargetForces;

//Perception
	float FOV = FMath::Cos(FMath::DegreesToRadians(120.0f));
	
//Weighting
	float CoherenceStrength = 1.9f;
	float SeparationStrength = 1.6f;
	float AlignmentStrength = 1.5f;

//Death effect
	UPROPERTY(EditDefaultsOnly)
	UNiagaraSystem* DeathEffect;

public:	

	virtual void Tick(float DeltaTime) override;
	
	void AddTargetForce(FVector TargetForce);

	virtual EFishType GetFishType() const;

	UFUNCTION(BlueprintCallable)
	float GetMinSpeed();

	virtual void OnDeath();
};
