// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "FixedBeingsManagerEditorSubsystem.generated.h"

class AFixedBeing;

USTRUCT()
struct FSpawnedBeing {
	GENERATED_BODY()

	FVector            Location;
	UPROPERTY()
	AFixedBeing* Actor;

	bool operator==(const FSpawnedBeing& Other) const {
		return Actor == Other.Actor;
	}
};

class AFixedBeing;
/**
 *
 */
UCLASS()
class REEFGAME_API UFixedBeingsManagerEditorSubsystem : public UEditorSubsystem {
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FSpawnedBeing> SpawnedBeings;

	virtual void Deinitialize() override;
	void         ClearAll();
};
