#pragma once

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

	bool bDirty = true;

	UPROPERTY()
	UCurveVector* CliffCurve;

	UPROPERTY()
	UProceduralMeshComponent* ProceduralMesh;

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

	int32 NumOfXVertices;
	int32 NumOfYVertices;

	float    GetNormalisedSkewedDistanceToCentre(const int32 X, const int32 Y) const;
	float    CalculateHeight(const int32 X, const int32 Y) const;
	FVector CalculateDisplacement(const int32 X, const int32 Y) const;
	void     GenerateVertices(FScopedSlowTask& Progress);
	void     GenerateTriangles(FScopedSlowTask& Progress);
	void     GenerateTangentsNormalsAndMesh(FScopedSlowTask& Progress);

public:
	float MaxZ = TNumericLimits<float>::Lowest();
	float MinZ = TNumericLimits<float>::Max();
	float MaxX = TNumericLimits<float>::Lowest();
	float MinX = TNumericLimits<float>::Max();
	float MaxY = TNumericLimits<float>::Lowest();
	float MinY = TNumericLimits<float>::Max();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	void         OnCurveModified(const FAssetData& AssetData);
	virtual void Deinitialize() override;

	void SetMaterial(UMaterialInterface* NewMaterial);

	void                      SetCliffCurve(UCurveVector* NewCliffCurve);
	FVector                   GetVertexPosition(int32 X, int32 Y) const;
	FVector                   GetNormal(int32 X, int32 Y) const;
	float                     GetDepthPercentage(int32 X, int32 Y) const;
	FBox2D                    GetBoundingBox2D() const;
	FBox                      GetBoundingBox() const;
	bool                      IsOk() const;
	UProceduralMeshComponent* GetTerrain() const;
	UProceduralMeshComponent* GetTerrain(FTerrainParameters const& NewParameters, UMaterialInterface* NewMaterial, UCurveVector* NewCliffCurve);

	UProceduralMeshComponent* GetTerrain(FTerrainParameters const& NewParameters);
};
