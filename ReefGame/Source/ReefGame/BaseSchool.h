// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BaseSchool.generated.h"

class UPathfindingSubsystem;
class AFishSchoolController;
class ABaseFish;

UCLASS()
class REEFGAME_API ABaseSchool : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ABaseSchool();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveAlongPath();

	void TickSwim();

	UPROPERTY()
	UPathfindingSubsystem* PathfindingSubsystem;

	UPROPERTY(VisibleAnywhere)
	TArray<FVector> CurrentPath;

	UPROPERTY(EditAnywhere)
	float PathfindingError = 150.0f;

	UPROPERTY(EditAnywhere)
	TArray<ABaseFish*> FishInSchool;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void SmoothMovement(FVector TargetLocation, float DeltaTime);

	void RemoveFishFromSchool(ABaseFish* Fish);

	void AddFishToSchool(ABaseFish* Fish);
};
