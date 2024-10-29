#include "TerrainManagerEditorSubsystem.h"

#include "KismetProceduralMeshLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/ScopedSlowTask.h"
#include "Curves/CurveVector.h"
#include "ProceduralMeshComponent.h"


// Unreal Overrides
void UTerrainManagerEditorSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UTerrainManagerEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Subscribe to asset registry to monitor changes to the curve
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().OnAssetUpdated().AddUObject(this, &UTerrainManagerEditorSubsystem::OnCurveModified);

}

// Setters

/**
 *  Set the material of the procedural mesh
 *	if the procedural mesh is not set, set the dirty flag so that the material is set when the mesh is created
 *
 * @param NewMaterial  The material to set
 */
void UTerrainManagerEditorSubsystem::SetMaterial(UMaterialInterface* NewMaterial)
{
	Material = NewMaterial;
	if(ProceduralMesh)
	{
		ProceduralMesh->SetMaterial(0, Material);
	}
	else
	{
		bDirty = true;
	}
}

/**
 *  Set the cliff curve and mark the subsystem as dirty so that the terrain is regenerated next time it is requested
 *
 * @param NewCliffCurve  The curve to set
 */
void UTerrainManagerEditorSubsystem::SetCliffCurve(UCurveVector* NewCliffCurve)
{
	CliffCurve = NewCliffCurve;
	bDirty = true;
}

// Getters and Util

/**
 * Gets the position of a vertex in the procedural mesh.
 * If the procedural mesh is not set, returns a zero vector.
 *
 * @param X The X coordinate of the vertex.
 * @param Y The Y coordinate of the vertex.
 * @return The position of the vertex as an FVector. If the procedural mesh is not set, returns FVector::ZeroVector.
 */
FVector UTerrainManagerEditorSubsystem::GetVertexPosition(const int32 X, const int32 Y) const
{
	if(!ProceduralMesh || X < 0 || Y < 0 || X >= NumOfXVertices || Y >= NumOfYVertices)
	{
		return FVector::ZeroVector;
	}
	return Vertices[X + Y * NumOfXVertices];
}


/**
 * Get the normal vector at the specified vertex coordinates.
 * If the procedural mesh is not set, a zero vector is returned.
 *
 * @param X The X coordinate of the vertex.
 * @param Y The Y coordinate of the vertex.
 * @return The normal vector at the specified vertex coordinates.
 */
FVector UTerrainManagerEditorSubsystem::GetNormal(const int32 X, const int32 Y) const
{
	if(!ProceduralMesh || X < 0 || Y < 0 || X >= NumOfXVertices || Y >= NumOfYVertices)
	{
		return FVector::ZeroVector;
	}
	return Normals[X + Y * NumOfXVertices].GetSafeNormal();
}

/**
 *  Calculates the depth percentage of a vertex in the procedural mesh based on its location.
 *  The depth percentage is the normalized distance from the maximum Z value.
 *  1.0 is the maximum Z value, 0.0 is the minimum Z value.
 *
 * @param X  The X-coordinate of the vertex in the mesh.
 * @param Y  The Y-coordinate of the vertex in the mesh.
 * @return A float representing the depth percentage. If the coordinates are out of bounds or the mesh is not set, returns 0.f.
 */
float UTerrainManagerEditorSubsystem::GetDepthPercentage(const int32 X, const int32 Y) const
{
	if(!ProceduralMesh || X < 0 || Y < 0 || X >= NumOfXVertices || Y >= NumOfYVertices)
	{
		return 0.f;
	}

	const float HeightRange = MaxZ - MinZ;

	const float VertexHeight = Vertices[X + Y * NumOfXVertices].Z;

	if(HeightRange > SMALL_NUMBER)
	{
		return 1.f - ((VertexHeight - MinZ) / HeightRange);
	}

	return 0.f;
}

/**
 * Get the 2D bounding box of the terrain managed by the subsystem.
 *
 * @return The 2D bounding box represented by the minimum and maximum x and y coordinates.
 */
FBox2D UTerrainManagerEditorSubsystem::GetBoundingBox2D() const
{
	const FVector2D Min(MinX, MinY);
	const FVector2D Max(MaxX, MaxY);

	return FBox2D(Min, Max);
}

/**
 * Retrieves the bounding box of the terrain.
 *
 * This function calculates the minimum and maximum points based on the internal state
 * of the terrain manager and returns a bounding box encompassing those points.
 *
 * @return The bounding box defined by the minimum and maximum points.
 */
FBox UTerrainManagerEditorSubsystem::GetBoundingBox() const
{
	const FVector Min(MinX, MinY, MinZ);
	const FVector Max(MaxX, MaxY, MaxZ);

	return FBox(Min, Max);
}

/**
 * Checks if the Terrain Manager Editor Subsystem is properly initialized.
 *
 * This method verifies that the essential components of the terrain manager
 * (Vertices, Triangles, ProceduralMesh, Material, and CliffCurve) are properly
 * set up and contain valid data.
 *
 * @return true if the subsystem is initialized and ready; false otherwise.
 */
bool UTerrainManagerEditorSubsystem::IsOk() const
{
	return Vertices.Num() > 0 && Triangles.Num() > 0 && ProceduralMesh && Material && CliffCurve && !bDirty;
}

// Terrain Generation

/**
 *  Get the terrain mesh. If not generated yet null.
 *
 * @return The terrain mesh
 */
UProceduralMeshComponent* UTerrainManagerEditorSubsystem::GetTerrain() const
{
	if(!ProceduralMesh || bDirty || !CliffCurve)
	{
		return nullptr;
	}
	return ProceduralMesh;
}

/**
 * Retrieve or create a procedural terrain mesh using specified parameters, material, and cliff curve.
 *
 * @param NewParameters The parameters defining the terrain.
 * @param NewMaterial The material to apply to the terrain.
 * @param NewCliffCurve The curve vector used for cliff generation.
 * @return A pointer to the generated or updated UProceduralMeshComponent.
 */
UProceduralMeshComponent* UTerrainManagerEditorSubsystem::GetTerrain(FTerrainParameters const& NewParameters, UMaterialInterface* NewMaterial, UCurveVector* NewCliffCurve)
{
	bDirty = true;
	SetMaterial(NewMaterial);
	SetCliffCurve(NewCliffCurve);
	return GetTerrain(NewParameters);
}

/**
 *  Get the terrain mesh with the given parameters.
 *  If the parameters are the same as the last time the terrain was generated, return the cached mesh.
 *  Else, regenerate the terrain mesh with the new parameters.
 *  If marked as dirty, regenerate the terrain mesh regardless of the parameters.
 *
 * @param NewParameters  The parameters to set
 * @return The terrain mesh
 */
UProceduralMeshComponent* UTerrainManagerEditorSubsystem::GetTerrain(FTerrainParameters const& NewParameters)
{
	if(!ProceduralMesh)
	{
		ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Procedural Mesh"));
		bDirty = true;
	}

	if(!bDirty && NewParameters == TerrainParameters)
	{
		return ProceduralMesh;
	}

	if(!CliffCurve)
	{
		UE_LOG(LogTemp, Error, TEXT("CliffCurve is not set"));
		return nullptr;
	}

	NumOfXVertices = FMath::CeilToInt32(TerrainParameters.Width * TerrainParameters.Density);
	NumOfYVertices = FMath::CeilToInt32(TerrainParameters.Height * TerrainParameters.Density);

	const float NumberOfTasks =
	+4 // Clearing the landscape
	+ NumOfXVertices * NumOfYVertices // Vertex/UV generation
	+ (NumOfXVertices - 1) * (NumOfYVertices - 1) // Triangle generation
	+ NumOfXVertices * NumOfYVertices // Normals generation
	+ 1; // Procedural mesh generation
	FScopedSlowTask Progress(NumberOfTasks, FText::FromString("Regenerating Environment"));
	Progress.MakeDialog(true, true);

	GenerateVertices(Progress);
	GenerateTriangles(Progress);
	GenerateTangentsNormalsAndMesh(Progress);

	bDirty = false;
	return ProceduralMesh;

}


/**
 * Calculate the normalized skewed distance from a specified point to the center of the terrain.
 * The distance is adjusted based on the angle to the center and a curve defining the terrain features.
 *
 * @param X  The X coordinate of the specified point.
 * @param Y  The Y coordinate of the specified point.
 * @return The normalized skewed distance to the center.
 */
float UTerrainManagerEditorSubsystem::GetNormalisedSkewedDistanceToCentre(const int32 X, const int32 Y) const
{
	const float CentreX = TerrainParameters.Width / 2;
	const float CentreY = TerrainParameters.Height / 2;

	const float MaxDistToCentre = FVector2D::Distance(FVector2D(0, 0), FVector2D(CentreX, CentreY));

	const float DistToCentre = FVector2D::Distance(FVector2D(X, Y), FVector2D(CentreX, CentreY));

	const float DeltaX = X - CentreX;
	const float DeltaY = Y - CentreY;

	// Calculate the angle in radians between -PI and PI
	const float Angle = FMath::Atan2(DeltaY, DeltaX);

	// Normalize the angle to be between 0 and 1
	const float NormalizedAngle = (Angle + PI) / (2 * PI); // Convert from (-PI, PI) to (0, 1)

	// Get the X value of the curve at the normalized angle
	const float XCurve = CliffCurve->GetVectorValue(NormalizedAngle).X;

	// Return the normalized distance to the centre, skewed by the X value of the curve
	return (DistToCentre / MaxDistToCentre) * XCurve;
}


/**
 * Calculate the height of the terrain at a given (X, Y) coordinate.
 *
 * @param X The X coordinate.
 * @param Y The Y coordinate.
 * @return The height of the terrain at the specified (X, Y) coordinate.
 */
float UTerrainManagerEditorSubsystem::CalculateHeight(const int32 X, const int32 Y) const
{
	// This is the noise for the sand variations in elevation
	const float SandNoise = FMath::PerlinNoise2D(FVector2d(X * TerrainParameters.SandRoughness, Y * TerrainParameters.SandRoughness)) * TerrainParameters
	.SandBankHeight;

	// The Z value of the curve at the normalized distance to the centre, this is the height of the cliff
	const float ZCurve = CliffCurve->GetVectorValue(GetNormalisedSkewedDistanceToCentre(X, Y)).Z;

	// The noise for the cliff modifier
	const float Modifier = FMath::PerlinNoise2D(
		FVector2d((X + TerrainParameters.CliffModifierSeed) * TerrainParameters.CliffModifierDensity,
				  (Y + TerrainParameters.CliffModifierSeed) * TerrainParameters.CliffModifierDensity)) * TerrainParameters.CliffModifierIntensity;

	const float SandbankHeight = SandNoise * (1 - ZCurve);
	const float CliffHeight = ZCurve * TerrainParameters.CliffIntensity;

	auto const Height = SandbankHeight + CliffHeight + Modifier;

	return Height;
}

/**
 * Given a position, calculate the displacement of the terrain at that position
 * The displacement is a 3d vector where the X and Y are the XY displacement and the Z is the height
 *
 */
FVector UTerrainManagerEditorSubsystem::CalculateDisplacement(const int32 X, const int32 Y) const
{
	// Get the normalised (0,1) distance to the centre of the mesh. This also includes the distortion from the X in the terrain curve
	const float Distance = GetNormalisedSkewedDistanceToCentre(X, Y);

	// Noise for the roughness of the cliff edge
	const float CliffNoise = FMath::PerlinNoise2D(FVector2d(X * TerrainParameters.CliffRoughness, Y * TerrainParameters.CliffRoughness)) *
	TerrainParameters.CliffRoughnessIntensity * (Distance + .3);

	// How much the terrain is displaced horizontally
	const float CliffEncroachmentAmount = CliffCurve->GetVectorValue(Distance).Y * TerrainParameters.CliffIntensity;

	// Direction to centre of mesh
	const FVector DirectionOfEncroachment = FVector(TerrainParameters.Width / 2, TerrainParameters.Height / 2, 0) - FVector(X, Y, 0);

	// The displacement is the encroachment amount plus the cliff noise in the direction of the centre
	const FVector Encroachment = DirectionOfEncroachment.GetSafeNormal() * CliffEncroachmentAmount + CliffNoise;

	// height is Z, encroachment is XY
	return FVector(Encroachment.X, Encroachment.Y, CalculateHeight(X, Y));
}

/**
 * Generate Vertices
 */
void UTerrainManagerEditorSubsystem::GenerateVertices(FScopedSlowTask& Progress)
{
	Progress.EnterProgressFrame(1.f, FText::FromString("Clearing Vertices/UVs..."));
	FPlatformProcess::Sleep(.01f);
	// Clear the arrays
	Vertices.Empty();
	UVCoords.Empty();
	// Set the min and max values to the lowest and highest possible values
	MaxZ = TNumericLimits<float>::Lowest();
	MinZ = TNumericLimits<float>::Max();
	MaxX = TNumericLimits<float>::Lowest();
	MinX = TNumericLimits<float>::Max();
	MaxY = TNumericLimits<float>::Lowest();
	MinY = TNumericLimits<float>::Max();


	for(int32 y = 0; y < NumOfYVertices; y++)
	{
		Progress.EnterProgressFrame(NumOfXVertices, FText::FromString(FString::Printf(TEXT("Generating Vertices %d/%d..."), y, NumOfYVertices)));

		for(int32 x = 0; x < NumOfXVertices; x++)
		{
			if(Progress.ShouldCancel())
			{
				return;
			}
			const FVector2d Position = FVector2D(x / TerrainParameters.Density, y / TerrainParameters.Density);
			FVector         Vec = FVector(Position.X, Position.Y, 0) + CalculateDisplacement(Position.X, Position.Y);

			if(Vec.Z < MinZ)
			{
				MinZ = Vec.Z;
			}
			if(Vec.Z > MaxZ)
			{
				MaxZ = Vec.Z;
			}
			if(Vec.X < MinX)
			{
				MinX = Vec.X;
			}
			if(Vec.X > MaxX)
			{
				MaxX = Vec.X;
			}
			if(Vec.Y < MinY)
			{
				MinY = Vec.Y;
			}
			if(Vec.Y > MaxY)
			{
				MaxY = Vec.Y;
			}

			Vertices.Add(Vec);

			UVCoords.Add(FVector2D(x, y));
			FPlatformProcess::Sleep(.5f / NumOfYVertices);
		}
	}
}
/**
 * Generate Triangles
 *
 */
void UTerrainManagerEditorSubsystem::GenerateTriangles(FScopedSlowTask& Progress)
{
	// Clear the triangles array
	Progress.EnterProgressFrame(1.f, FText::FromString("Clearing Triangles..."));
	Triangles.Empty();


	for(int32 y = 0; y < NumOfYVertices - 1; y++)
	{
		Progress.EnterProgressFrame(NumOfXVertices - 1, FText::FromString(FString::Printf(TEXT("Generating Triangles..."))));
		for(int32 x = 0; x < NumOfXVertices - 1; x++)
		{
			if(Progress.ShouldCancel())
			{
				return;
			}
			Triangles.Add(x + y * NumOfXVertices);
			Triangles.Add(x + (y + 1) * NumOfXVertices);
			Triangles.Add(x + 1 + y * NumOfXVertices);

			Triangles.Add(x + 1 + y * NumOfXVertices);
			Triangles.Add(x + (y + 1) * NumOfXVertices);
			Triangles.Add(x + 1 + (y + 1) * NumOfXVertices);
		}
		FPlatformProcess::Sleep(.5f / NumOfYVertices);
	}
}


/**
 * Generate Tangents, Normals and create the procedural mesh
 */
void UTerrainManagerEditorSubsystem::GenerateTangentsNormalsAndMesh(FScopedSlowTask& Progress)
{
	// Clear the mesh
	Progress.EnterProgressFrame(2.f, FText::FromString("Clearing Mesh..."));
	FPlatformProcess::Sleep(.01f);
	if(Progress.ShouldCancel())
	{
		return;
	}
	ProceduralMesh->ClearAllMeshSections();
	Tangents.Empty();
	Normals.Empty();

	Progress.EnterProgressFrame(1.f, FText::FromString("Generating Tangents and Normals..."));
	FPlatformProcess::Sleep(.01f);
	if(Progress.ShouldCancel())
	{
		return;
	}
	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UVCoords, Normals, Tangents);

	Progress.EnterProgressFrame(1.f, FText::FromString("Generating Mesh..."));
	FPlatformProcess::Sleep(.01f);
	if(Progress.ShouldCancel())
	{
		return;
	}
	ProceduralMesh->CreateMeshSection(
		0,
		Vertices,
		Triangles,
		Normals,
		UVCoords,
		TArray<FColor>(),
		Tangents,
		true
	);
	ProceduralMesh->SetMaterial(0, Material);
}

// Curve

/**
 *  Called when any asset is modified, checks if the asset is the curve we have and sets the dirty flag
 *
 * @param AssetData  The asset data of the modified asset
 */
void UTerrainManagerEditorSubsystem::OnCurveModified(const FAssetData& AssetData)
{
	// if the asset is the curve we have
	if(AssetData.GetAsset()->GetFName() == CliffCurve->GetFName())
	{
		// set the dirty flag
		bDirty = true;
	}
}

// FTerrainParameters

bool FTerrainParameters::operator==(FTerrainParameters const& Other) const
{
	return Width == Other.Width
	&& Height == Other.Height
	&& Density == Other.Density
	&& SandBankHeight == Other.SandBankHeight
	&& SandRoughness == Other.SandRoughness
	&& PerlinOffset == Other.PerlinOffset
	&& CliffScale == Other.CliffScale
	&& CliffIntensity == Other.CliffIntensity
	&& CliffRoughness == Other.CliffRoughness
	&& CliffRoughnessIntensity == Other.CliffRoughnessIntensity
	&& CliffModifierSeed == Other.CliffModifierSeed
	&& CliffModifierDensity == Other.CliffModifierDensity
	&& CliffModifierIntensity == Other.CliffModifierIntensity;
}
