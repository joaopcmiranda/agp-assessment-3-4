// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "SpawnedActorsEditorSubsystem.generated.h"

class AFixedBeing;
/**
 *
 */
UCLASS()
class REEFGAME_API USpawnedActorsEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<AFixedBeing*> SpawnedActors;

	virtual void Deinitialize() override;
	void         ClearAllSpawnedActors();
};
