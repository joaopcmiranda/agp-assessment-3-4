// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Terrain/TerrainManagerEditorSubsystem.h"
#include "Environment.generated.h"

class ATerrain;
class UProceduralMeshComponent;
struct FSpawnedBeing;
class AFixedBeing;

UCLASS()
class REEFGAME_API AEnvironment : public AActor {
	GENERATED_BODY()

	void                RegenerateEnvironmentInternal();
	void                RegenerateFixedBeingsInternal();
	FTerrainParameters GetTerrainParams() const;
public:
	// Sets default values for this actor's properties
	AEnvironment();

	#if WITH_EDITOR

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Environment")
	void RegenerateTerrain();
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Environment")
	void RegenerateFixedBeings();
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Environment")
	void ClearFixedBeings();

	virtual void OnConstruction(const FTransform& Transform) override;
	#endif



	UPROPERTY(EditAnywhere, Category = "Environment")
	int32 Width = 20000;
	UPROPERTY(EditAnywhere, Category = "Environment")
	int32 Height = 20000;

	UPROPERTY(EditAnywhere, Category = "Environment")
	float Density = 0.002f;

	UPROPERTY(EditAnywhere, Category = "Environment")
	float SandBankHeight = 300.0f;
	UPROPERTY(EditAnywhere, Category = "Environment")
	float SandRoughness = 0.0003f;
	UPROPERTY(VisibleAnywhere, Category = "Environment")
	float PerlinOffset = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Environment")
	float CliffScale = 3000.0f;
	UPROPERTY(EditAnywhere, Category = "Environment")
	float CliffIntensity = 5000.0f;
	UPROPERTY(EditAnywhere, Category = "Environment")
	float CliffRoughness = .0006f;
	UPROPERTY(EditAnywhere, Category = "Environment")
	float CliffRoughnessIntensity = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Environment")
	float CliffModifierSeed = 0.0f;
	UPROPERTY(EditAnywhere, Category = "Environment")
	float CliffModifierDensity = 4;
	UPROPERTY(EditAnywhere, Category = "Environment")
	float CliffModifierIntensity = 100;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	ATerrain* TerrainActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	UMaterialInterface* TerrainMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Environment")
	UCurveVector* CliffCurve;

	UPROPERTY(EditAnywhere, Category="Environment")
	float FixedBeingPlacingPrecision = 10.f;
	UPROPERTY(EditAnywhere, Category="Environment")
	int32 FixedBeingPlacingPasses = 100;

	UPROPERTY(EditAnywhere, Category="Environment")
	float ClusterRange = 1000.f;

	UPROPERTY(EditAnywhere, Category="Environment")
	TArray<TSubclassOf<AFixedBeing>> FixedBeingsClasses;

};
