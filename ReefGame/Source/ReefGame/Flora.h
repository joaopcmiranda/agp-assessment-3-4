// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Flora.generated.h"

UCLASS()
class REEFGAME_API AFlora : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFlora();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool bIsSpawnable = true;
	float FlatAffinity = 0.5f;
	float SlopeAffinity = 0.5f;
	float VerticalAffinity = 0.5f;
	float DeepAffinity = 0.5f;
	float ShallowAffinity = 0.5f;
	float SingleAffinity = 0.5f;
	float GeneralClusterAffinity = 0.5f;
	float SelfClusterAffinity = 0.5f;
	float ClusterSpacing = 0.5f;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
