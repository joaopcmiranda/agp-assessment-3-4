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

	float FixedBeingPlacingPrecision = 10.f;
	int32 FixedBeingPlacingPasses = 100;
	float ClusterRange = 1000.f;
};

USTRUCT()
struct FSpawnedBeing {
	GENERATED_BODY()

	FVector Location;

	UPROPERTY()
	TWeakObjectPtr<AFixedBeing> Being;

	bool operator==(const FSpawnedBeing& Other) const
	{
		return Being.Get() == Other.Being.Get();
	}
};

USTRUCT()
struct FIndividualPicker {
	GENERATED_BODY()

	UPROPERTY()
	TArray<AFixedBeing*> Beings;

	AFixedBeing* operator[](int32 const Index) const
	{
		if(Index < 0 || Index >= Beings.Num())
		{
			return nullptr;
		}
		return Beings[Index];
	}
	int32 Num() const { return Beings.Num(); }
	void  Add(AFixedBeing* const Being);
	void  Empty() { Beings.Empty(); }
	void  RemoveAt(int32 const Index);
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
	TArray<FIndividualPicker> Picker_Internal;

	TArray<FIndividualPicker>& GetPicker();
	void                          ClearPicker();
	void                          DespawnToPicker();

	void PlaceFixedBeingsPass(const int32& Pass, FScopedSlowTask& Progress, AActor* Parent);
	void PlaceFixedBeingInEnvironment(const int32& Pass, float Y, float X, AActor* Parent);

	bool bDirty = true;

	FFixedBeingsParameters Parameters;

	int32 StepSeed = Seed;
	int32 PickerIndexSeed = Seed;
	int32 SelectionSeed = Seed;
	int32 Seed = 0;
	float GetDeterministicStep(const int32& Pass, const int32 ArraySize);
	int32 GetDeterministicPickerIndex();
	float GetDetRand0To1();

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	void         ClearAll();
	void         ClearSpawned();
	void         CheckChildren(const AActor* Parent);
	void         SetFixedBeingsClasses(TArray<TSubclassOf<AFixedBeing>> const& NewClasses);
	void         SetSeed(int32 NewSeed);

	void RedistributeFixedBeings(FFixedBeingsParameters NewParameters, AActor* Parent, TArray<TSubclassOf<AFixedBeing>> const& NewClasses);

};
