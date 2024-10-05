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
	
//Components
	UPROPERTY(VisibleAnywhere)
	USphereComponent* FishCollision;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* FishMesh;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* PerceptionSensor;

	UPROPERTY(VisibleAnywhere)
	UHealthComponent* HealthComponent;

//Current state	
	UPROPERTY(EditAnywhere)
	EFishState CurrentState = EFishState::Swim;

//School controller	
	UPROPERTY()
	AFishSchoolController* FishSchoolController;

//Movement

	FVector Velocity = FVector(1.0f, 0.0f, 0.0f) * MinSpeed;
	FRotator CurrentRotation;
	
	float MaxSpeed = 1000.0f;
	float MinSpeed = 500.0f;

//MovementFunctions	

	void UpdateMeshRotation();

	FVector Cohere(TArray<AActor*> School);
	FVector Separate(TArray<AActor*> School);
	FVector Align(TArray<AActor*> School);

	void Steer(float DeltaTime);
	
//Obstacle Avoidance

	bool IsObstacle();
	FVector AvoidObstacle();

	TArray<FVector> TargetForces;

//Perception
	float FOV = FMath::Cos(FMath::DegreesToRadians(60.0f));
	float FlockingDelay = 2.0f;


//Weighting
	float CoherenceStrength = 1.0f;
	float SeparationStrength = 1.5f;
	float AlignmentStrength = 1.0f;


public:	

	virtual void Tick(float DeltaTime) override;
	
	void AddTargetForce(FVector TargetForce);
	
};
