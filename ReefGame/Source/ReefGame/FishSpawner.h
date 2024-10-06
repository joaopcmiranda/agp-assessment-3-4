// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseFish.h"
#include "GameFramework/Actor.h"
#include "FishSpawner.generated.h"

class ABaseFish;

UCLASS()
class REEFGAME_API AFishSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFishSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere)
	int32 NumFishToSpawn;
	UPROPERTY(EditAnywhere)
	float SpawnRadius;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseFish> FishType = ABaseFish::StaticClass();

	void SpawnFish(int32 NumFish);

	//Timer settings
	float SpawnInterval;
	void SpawnFishTimer();
	FTimerHandle SpawnTimerHandle;

};
