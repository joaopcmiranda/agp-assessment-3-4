#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "Terrain.h"
#include "TerrainManagerEditorSubsystem.generated.h"

struct FProcMeshTangent;
class UProceduralMeshComponent;
class UCurveVector;

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
class UTerrainManagerEditorSubsystem : public UEditorSubsystem {
	GENERATED_BODY()


	UPROPERTY()
	UCurveVector* CliffCurve;

	UPROPERTY()
	TWeakObjectPtr<ATerrain> WTerrainActor;

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


	float   GetNormalisedSkewedDistanceToCentre(const int32 X, const int32 Y) const;
	float   CalculateHeight(const int32 X, const int32 Y) const;
	FVector CalculateDisplacement(const int32 X, const int32 Y) const;
	bool    GenerateVertices(FScopedSlowTask& Progress);
	bool    GenerateTriangles(FScopedSlowTask& Progress);
	bool    GenerateTangentsNormalsAndMesh(FScopedSlowTask& Progress);

public:
	bool bDirty = true;
	int32 NumOfXVertices;
	int32 NumOfYVertices;
	float MaxZ = TNumericLimits<float>::Lowest();
	float MinZ = TNumericLimits<float>::Max();
	float MaxX = TNumericLimits<float>::Lowest();
	float MinX = TNumericLimits<float>::Max();
	float MaxY = TNumericLimits<float>::Lowest();
	float MinY = TNumericLimits<float>::Max();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	void         OnCurveModified(const FAssetData& AssetData);
	virtual void Deinitialize() override;

	void Release();

	void SetMaterial(UMaterialInterface* NewMaterial);

	void      CheckChildren(const AActor* Parent);
	void      SetCliffCurve(UCurveVector* NewCliffCurve);
	FVector   GetVertexPosition(float X, float Y) const;
	FVector   GetNormal(float X, float Y) const;
	float     GetDepthPercentage(float X, float Y) const;
	FBox2D    GetBoundingBox2D() const;
	FBox      GetBoundingBox() const;
	bool      IsOk() const;
	ATerrain* GetTerrain() const;
	ATerrain* GetTerrain(FTerrainParameters const& NewParameters, UMaterialInterface* NewMaterial, UCurveVector* NewCliffCurve);

	ATerrain* GetTerrain(FTerrainParameters const& NewParameters);
};
