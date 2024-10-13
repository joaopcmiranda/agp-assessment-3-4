// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FixedBeing.generated.h"

UCLASS()
class REEFGAME_API AFixedBeing : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFixedBeing();

	UPROPERTY(EditAnywhere, Category="Fauna/Flora")
	bool bIsSpawnable = true;
	UPROPERTY(EditAnywhere, Category="Fauna/Flora")
	float FlatAffinity = 0.5f;
	UPROPERTY(EditAnywhere, Category="Fauna/Flora")
	float VerticalAffinity = 0.5f;
	UPROPERTY(EditAnywhere, Category="Fauna/Flora")
	float DeepAffinity = 0.5f;
	UPROPERTY(EditAnywhere, Category="Fauna/Flora")
	float ShallowAffinity = 0.5f;
	UPROPERTY(EditAnywhere, Category="Fauna/Flora")
	float SingleAffinity = 0.5f;
	UPROPERTY(EditAnywhere, Category="Fauna/Flora")
	float GeneralClusterAffinity = 0.5f;
	UPROPERTY(EditAnywhere, Category="Fauna/Flora")
	float SelfClusterAffinity = 0.5f;
	UPROPERTY(EditAnywhere, Category="Fauna/Flora")
	float ClusterSpacing = 0.5f;
	UPROPERTY(EditAnywhere, Category="Fauna/Flora")
	bool bCanBeUpsideDown = true;
	UPROPERTY(EditAnywhere, Category="Fauna/Flora")
	bool bAlwaysPointUp = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
