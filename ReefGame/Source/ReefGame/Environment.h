// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Terrain.h"
#include "GameFramework/Actor.h"
#include "Environment.generated.h"

UCLASS()
class REEFGAME_API AEnvironment : public AActor {
	GENERATED_BODY()

	FTerrainParameters CachedTerrainParameters;

	void RegenerateEnvironmentInternal() const;
	bool SetTerrainParams();

public:
	// Sets default values for this actor's properties
	AEnvironment();
	void OnConstruction(const FTransform& Transform);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Terrain")
	void RegenerateEnvironment();
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Terrain")
	void ForceRegenerateEnvironment();
	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void InitializeTerrain();
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



	UPROPERTY()
	TArray<class AFlora*> Flora;

	UPROPERTY()
	ATerrain* Terrain;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
