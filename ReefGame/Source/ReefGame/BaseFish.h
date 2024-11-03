// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NiagaraSystem.h"
#include "HighlightComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "BaseFish.generated.h"

class UHealthComponent;
class UStaticMeshComponent;
class USphereComponent;

UENUM(BlueprintType)
enum class EFishType : uint8
{
	BaseFish UMETA(DisplayName = "Base Fish"),
	NullType UMETA(DisplayName = "NullType"),
	AngelFish UMETA(DisplayName = "AngelFish"),
	MoorishIdol UMETA(DisplayName = "MoorishIdol"),
	BlueTang UMETA(DisplayName = "BlueTang"),
	PurpleTang UMETA(DisplayName = "PurpleTang"),
	ClownFish UMETA(DisplayName = "ClownFish"),
	LionFish UMETA(DisplayName = "LionFish"),
	ParrotFish UMETA(DisplayName = "ParrotFish"),
	PufferFish UMETA(DisplayName = "PufferFish"),
	ClownTriggerFish UMETA(DisplayName = "ClownTriggerFish"),
	Barracuda UMETA(DisplayName = "Barracuda"),
	Grouper UMETA(DisplayName = "Grouper"),
	GreatTrevally UMETA(DisplayName = "GreatTrevally"),
	SailFish UMETA(DisplayName = "SailFish"),
	Tuna UMETA(DisplayName = "Tuna")
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
	UPROPERTY(Replicated)
	EFishState CurrentState = EFishState::Roam;

	UPROPERTY(Replicated)
	EFishType FishType = EFishType::BaseFish;

//Components
	UPROPERTY(VisibleAnywhere)
	USphereComponent* FishCollision;

	UPROPERTY(BlueprintReadOnly)
	USkeletalMeshComponent* FishMesh;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* PerceptionSensor;

	UPROPERTY(VisibleAnywhere)
	UHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere)
	UHighlightComponent* HighlightComponent;


//Fish in radius
	UPROPERTY(Replicated)
	TArray<AActor*> FishInRadius;

	UPROPERTY(Replicated)
	TArray<AActor*> FishOfSameType;

	UPROPERTY(Replicated)
	ABaseFish* Predator = nullptr;

	UPROPERTY(Replicated)
	ABaseFish* Prey = nullptr;

	UPROPERTY(Replicated)
	EFishType PredatorType = EFishType::NullType;

	UPROPERTY(Replicated)
	EFishType PreyTypeA = EFishType::NullType;
	UPROPERTY(Replicated)
    EFishType PreyTypeB = EFishType::NullType;
    UPROPERTY(Replicated)
    EFishType PreyTypeC = EFishType::NullType;

	void UpdateFishInRadius();
	void UpdateFishTypes();

//Movement

	UPROPERTY(Replicated)
	FVector Velocity;
	UPROPERTY(Replicated)
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

	UPROPERTY(Replicated)
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

	// Server RPCs
	UFUNCTION(BlueprintCallable)
	void ServerUpdateState();
	UFUNCTION(Server, Reliable)
	void ServerOnDeath();
	UFUNCTION(Server, Reliable)
	void ServerAddTargetForce(const FVector& TargetForce);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpawnDeathEffect();

public:

	virtual void Tick(float DeltaTime) override;

	void AddTargetForce(FVector TargetForce);

	virtual EFishType GetFishType() const;

	UFUNCTION(BlueprintCallable)
	float GetMinSpeed();

	virtual void OnDeath();

	//Highlight

	UFUNCTION(BlueprintImplementableEvent)
	void Highlight(bool bHighlight);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(ABaseFish, CurrentState);
		DOREPLIFETIME(ABaseFish, FishType);
		DOREPLIFETIME(ABaseFish, Predator);
		DOREPLIFETIME(ABaseFish, Prey);
		DOREPLIFETIME(ABaseFish, PredatorType);
		DOREPLIFETIME(ABaseFish, PreyTypeA);
		DOREPLIFETIME(ABaseFish, PreyTypeB);
		DOREPLIFETIME(ABaseFish, PreyTypeC);
		DOREPLIFETIME(ABaseFish, Velocity);
		DOREPLIFETIME(ABaseFish, CurrentRotation);
		DOREPLIFETIME(ABaseFish, FishInRadius);
		DOREPLIFETIME(ABaseFish, FishOfSameType);
	};

};
