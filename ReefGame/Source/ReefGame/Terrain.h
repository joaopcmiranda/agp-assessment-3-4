// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "Curves/CurveVector.h"
#include "Terrain.generated.h"

USTRUCT()
struct FTerrainParameters {
	GENERATED_BODY()

	int32 Width;
	int32 Height;

	float Density;

	float SandBankHeight;
	float SandRoughness;
	float PerlinOffset;

	float CliffScale;
	float CliffIntensity;
	float CliffRoughness;
	float CliffRoughnessIntensity;

	float CliffModifierSeed;
	float CliffModifierDensity;
	float CliffModifierIntensity;

	// equals
	bool operator==(FTerrainParameters const& Other) const;
};

UCLASS()
class REEFGAME_API ATerrain : public AActor {
	GENERATED_BODY()


public:
	// Sets default values for this actor's properties
	ATerrain();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void         Regenerate(FScopedSlowTask& Progress);
	void    ClearLandScape(FScopedSlowTask& Progress);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	UProceduralMeshComponent* ProceduralMesh;

	UPROPERTY()
	UCurveVector* CliffCurve;

	float   CalculateDistanceToCentre(float X, float Y) const;
	float   CalculateHeight(float X, float Y) const;
	FVector CalculateDisplacement(float X, float Y) const;

	UPROPERTY()
	TArray<FVector> Vertices;

	UPROPERTY()
	TArray<int32> Triangles;

	UPROPERTY()
	TArray<FVector2D> UVCoords;

	UPROPERTY()
	TArray<FProcMeshTangent> Tangents;

	UPROPERTY()
	TArray<FVector> Normals;

	UPROPERTY()
	UMaterialInterface* Material;

	UPROPERTY()
	FTerrainParameters TerrainParameters;

public:
	void SetTerrainParameters(FTerrainParameters const& NewTerrainParameters) { TerrainParameters = FTerrainParameters(NewTerrainParameters); }
	void SetMaterial(UMaterialInterface* NewMaterial) { Material = NewMaterial; }
	void SetCliffCurve(UCurveVector* NewCliffCurve) { CliffCurve = NewCliffCurve; }
};
