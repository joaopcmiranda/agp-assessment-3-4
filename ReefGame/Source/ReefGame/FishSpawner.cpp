// Fill out your copyright notice in the Description page of Project Settings.

#include "FishSpawner.h"
#include "BaseFish.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"

// Sets default values
AFishSpawner::AFishSpawner()
{
	// Set this actor to call Tick() every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	NumFishToSpawn = 10;
	SpawnRadius = 5000.0f;
	SpawnInterval = 10.0f; // Spawn fish every 10 seconds
}

// Called when the game starts or when spawned
void AFishSpawner::BeginPlay()
{
	Super::BeginPlay();
	SpawnFish(NumFishToSpawn);

	// Set timer to spawn fish periodically
	GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &AFishSpawner::SpawnFishTimer, SpawnInterval, true);
}

void AFishSpawner::SpawnFish(int32 NumFish)
{
	if (FishType == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("FishType is not set. Please assign a fish class to spawn."));
		return;
	}

	for (int32 i = 0; i < NumFish; i++)
	{
		FVector RandomOffset = UKismetMathLibrary::RandomPointInBoundingBox(GetActorLocation(), FVector(SpawnRadius, SpawnRadius, SpawnRadius / 2));
		FRotator SpawnRotation = FRotator::ZeroRotator;
		FActorSpawnParameters FishSpawnParams;
		FishSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ABaseFish* SpawnedFish = GetWorld()->SpawnActor<ABaseFish>(FishType, RandomOffset, SpawnRotation, FishSpawnParams);
		if (SpawnedFish)
		{
			UE_LOG(LogTemp, Log, TEXT("Spawned fish at location: %s"), *SpawnedFish->GetActorLocation().ToString());
		}
	}
}

void AFishSpawner::SpawnFishTimer()
{
	SpawnFish(1); // Spawn one fish every interval
}

