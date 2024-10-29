// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "ReefGame/Terrain/TerrainManagerEditorSubsystem.h"
#include "FixedBeingsManagerEditorSubsystem.generated.h"
class AFixedBeing;

USTRUCT()
struct FFixedBeingsParameters {
	GENERATED_BODY()

	float FixedBeingPlacingPrecision = .1f;
	int32 FixedBeingPlacingPasses = 100;
	float ClusterRange = 1000.f;
};

USTRUCT()
struct FSpawnedBeing {
	GENERATED_BODY()

	FVector Location;

	UPROPERTY()
	AFixedBeing* Being;

	bool operator==(const FSpawnedBeing& Other) const
	{
		return Being == Other.Being;
	}
};


/**
 *
 */
UCLASS()
class REEFGAME_API UFixedBeingsManagerEditorSubsystem : public UEditorSubsystem {
	GENERATED_BODY()

	UPROPERTY()
	UTerrainManagerEditorSubsystem* TerrainManager;

	UPROPERTY()
	TArray<TSubclassOf<AFixedBeing>> FixedBeingsClasses;

	UPROPERTY()
	TArray<FSpawnedBeing> SpawnedBeings;

	UPROPERTY()
	TArray<TArray<AFixedBeing*>> Picker_Internal;

	TArray<TArray<AFixedBeing*>>& GetPicker();
	void                          ClearPicker();
	void                          DespawnToPicker();

	void PlaceFixedBeingsPass(const int32& Pass, FScopedSlowTask& Progress, AActor* Parent);
	void PlaceFixedBeingInEnvironment(const int32& Pass, int32 Y, int32 X, AActor* Parent);

	bool bDirty = true;

	FFixedBeingsParameters Parameters;

	int32 StepSeed = Seed;
	int32 PickerIndexSeed = Seed;
	int32 SelectionSeed = Seed;
	int32 Seed = 0;
	int32 GetDeterministicStep(const int32& Pass);
	int32 GetDeterministicPickerIndex();
	float GetDetRand0To1();

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	void         ClearAll();
	void         ClearSpawned();
	void         SetFixedBeingsClasses(TArray<TSubclassOf<AFixedBeing>> const& NewClasses);
	void         SetSeed(int32 NewSeed);

	void RedistributeFixedBeings(FFixedBeingsParameters NewParameters, AActor* Parent);

};
