// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnedActorsEditorSubsystem.h"
#include "FixedBeing.h"

void USpawnedActorsEditorSubsystem::Deinitialize()
{
	Super::Deinitialize();

	ClearAllSpawnedActors();
}


void USpawnedActorsEditorSubsystem::ClearAllSpawnedActors()
{
	for (AFixedBeing* Actor : SpawnedActors)
	{
		if (Actor && IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
	SpawnedActors.Empty();
}
