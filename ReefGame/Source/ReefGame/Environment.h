// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Terrain.h"
#include "GameFramework/Actor.h"
#include "Environment.generated.h"

class AFixedBeing;

UCLASS()
class REEFGAME_API AEnvironment : public AActor {
	GENERATED_BODY()

	FTerrainParameters CachedTerrainParameters;

	void RegenerateEnvironmentInternal();
	bool SetTerrainParams();
public:
	// Sets default values for this actor's properties
	AEnvironment();

	#if WITH_EDITOR

	void OnConstruction(const FTransform& Transform);
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Terrain")
	void Regenerate();
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Terrain")
	void ForceRegenerate();
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Terrain")
	void ClearFixedBeings();

	void PlaceFixedBeings(FScopedSlowTask& Progress);
	void ReplenishPicker(TArray<TArray<AFixedBeing*>>& Picker);
	void PlaceFixedBeingsPass(TArray<TArray<AFixedBeing*>>& Picker, const int32& Pass, FScopedSlowTask& Progress);

	#endif

	void InitializeTerrain();

	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void LogTerrainStatus();


	UPROPERTY(EditAnywhere, Category = "Terrain")
	int32 Width = 20000;
	UPROPERTY(EditAnywhere, Category = "Terrain")
	int32 Height = 20000;

	UPROPERTY(EditAnywhere, Category = "Terrain")
	float Density = 0.002f;

	UPROPERTY(EditAnywhere, Category = "Terrain")
	float SandBankHeight = 300.0f;
	UPROPERTY(EditAnywhere, Category = "Terrain")
	float SandRoughness = 0.0003f;
	UPROPERTY(VisibleAnywhere, Category = "Terrain")
	float PerlinOffset = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Terrain")
	float CliffScale = 3000.0f;
	UPROPERTY(EditAnywhere, Category = "Terrain")
	float CliffIntensity = 5000.0f;
	UPROPERTY(EditAnywhere, Category = "Terrain")
	float CliffRoughness = .0006f;
	UPROPERTY(EditAnywhere, Category = "Terrain")
	float CliffRoughnessIntensity = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Terrain")
	float CliffModifierSeed = 0.0f;
	UPROPERTY(EditAnywhere, Category = "Terrain")
	float CliffModifierDensity = 4;
	UPROPERTY(EditAnywhere, Category = "Terrain")
	float CliffModifierIntensity = 100;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UChildActorComponent* TerrainComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	UMaterialInterface* TerrainMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain")
	UCurveVector* CliffCurve;

	UPROPERTY(EditAnywhere, Category="Terrain Fauna/Flora")
	float FixedBeingPlacingPrecision = 0.1f;
	UPROPERTY(EditAnywhere, Category="Terrain Fauna/Flora")
	int32 FixedBeingPlacingPasses = 5;

	UPROPERTY(EditAnywhere, Category="Terrain Fauna/Flora")
	TArray<TSubclassOf<AFixedBeing>> FixedBeingsClasses;

	UPROPERTY()
	ATerrain* Terrain;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
