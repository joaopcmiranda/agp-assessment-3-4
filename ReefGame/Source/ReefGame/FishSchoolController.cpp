// Fill out your copyright notice in the Description page of Project Settings.

#include "FishSchoolController.h"
#include "BaseFish.h"
#include "EngineUtils.h"

// Sets default values
AFishSchoolController::AFishSchoolController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFishSchoolController::BeginPlay()
{
	Super::BeginPlay();
	PopulateFish();
	
}

void AFishSchoolController::PopulateFish()
{
	UWorld* World = GetWorld();
    
	if (!World) return;

	// Get all actors of the class ABaseFish
	for (TActorIterator<ABaseFish> FishItr(World); FishItr; ++FishItr)
	{
		ABaseFish* FishActor = *FishItr;
		if (FishActor)
		{
			Fish.Add(FishActor);
		}
	}
}

ABaseSchool* AFishSchoolController::CreateSchool(FVector SpawnLocation)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FRotator SpawnRotation = FRotator(0.0f, 0.0f, 0.0f);
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;  // Set the owner to this controller
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		ABaseSchool* SpawnedSchool = World->SpawnActor<ABaseSchool>(ABaseSchool::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);
		Schools.Add(SpawnedSchool);

		if (SpawnedSchool)
		{
			UE_LOG(LogTemp, Log, TEXT("BaseSchool spawned successfully at location: %s"), *SpawnLocation.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to spawn BaseSchool"));
		}
		return SpawnedSchool;
	}
	return nullptr;
}

// Called every frame
void AFishSchoolController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

