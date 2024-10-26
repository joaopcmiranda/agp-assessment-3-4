// Fill out your copyright notice in the Description page of Project Settings.


#include "Terrain.h"

#include "KismetProceduralMeshLibrary.h"
#include "Misc/ScopedSlowTask.h"

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

// Sets default values
ATerrain::ATerrain()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Procedural Mesh"));
	SetRootComponent(ProceduralMesh);
}

// Called when the game starts or when spawned
void ATerrain::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("ATerrain::BeginPlay"));
}

void ATerrain::ClearLandScape(FScopedSlowTask& Progress)
{
	UE_LOG(LogTemp, Warning, TEXT("ATerrain::ClearLandScape"));

	Progress.EnterProgressFrame(4.f, FText::FromString(FString::Printf(TEXT("Clearing Landscape..."))));
	Triangles.Empty();
	Vertices.Empty();
	UVCoords.Empty();
	ProceduralMesh->ClearAllMeshSections();
}

float ATerrain::CalculateDistanceToCentre(const int32 X, const int32 Y) const
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

	const float XCurve = CliffCurve->GetVectorValue(NormalizedAngle).X;


	return (DistToCentre / MaxDistToCentre) * XCurve;
}

float ATerrain::CalculateHeight(const int32 X, const int32 Y) const
{
	const float SandNoise = FMath::PerlinNoise2D(FVector2d(X * TerrainParameters.SandRoughness, Y * TerrainParameters.SandRoughness)) * TerrainParameters
	.SandBankHeight;
	const float ZCurve = CliffCurve->GetVectorValue(CalculateDistanceToCentre(X, Y)).Z;

	const float Modifier = FMath::PerlinNoise2D(
		FVector2d((X + TerrainParameters.CliffModifierSeed) * TerrainParameters.CliffModifierDensity,
		          (Y + TerrainParameters.CliffModifierSeed) * TerrainParameters.CliffModifierDensity)) * TerrainParameters.CliffModifierIntensity;

	const float SandbankHeight = SandNoise * (1 - ZCurve);
	const float CliffHeight = ZCurve * TerrainParameters.CliffIntensity;

	auto const Height = SandbankHeight + CliffHeight + Modifier;

	return Height;
}

FVector ATerrain::CalculateDisplacement(const int32 X, const int32 Y) const
{
	const float Distance = CalculateDistanceToCentre(X, Y);

	const float CliffNoise = FMath::PerlinNoise2D(FVector2d(X * TerrainParameters.CliffRoughness, Y * TerrainParameters.CliffRoughness)) *
	TerrainParameters.CliffRoughnessIntensity * (Distance + .3);

	const float CliffEncroachmentAmount = CliffCurve->GetVectorValue(Distance).Y * TerrainParameters.CliffIntensity;

	// Direction to centre
	const FVector DirectionOfEncroachment = FVector(TerrainParameters.Width / 2, TerrainParameters.Height / 2, 0) - FVector(X, Y, 0);

	const FVector Encroachment = DirectionOfEncroachment.GetSafeNormal() * CliffEncroachmentAmount + CliffNoise;

	// height is Z, encroachment is XY
	return FVector(Encroachment.X, Encroachment.Y, CalculateHeight(X, Y));

}

FVector ATerrain::CalculateNormal(int32 const X, int32 const Y) const
{
	return Normals[X + Y * (TerrainParameters.Height * TerrainParameters.Density)];
}

float ATerrain::CalculateDepth(int32 const X, int32 const Y) const
{
	const float HeightRange = MaxZ - MinZ;

	const float VertexHeight = Vertices[X + Y * (TerrainParameters.Height * TerrainParameters.Density)].Z;

	if(HeightRange > SMALL_NUMBER)
	{
		return 1.f - ((VertexHeight - MinZ) / HeightRange);
	}

	return 0.f;
}

FVector ATerrain::GetVertexPosition(const int32 X, const int32 Y) const
{
	return Vertices[X + Y * (TerrainParameters.Height * TerrainParameters.Density)];
}

FBox2D ATerrain::GetBoundingBox2D() const
{
	const FVector2D Min(MinX, MinY);
	const FVector2D Max(MaxX, MaxY);

	return FBox2D(Min, Max);
}

bool ATerrain::IsOk() const
{
	return Vertices.Num() > 0 && Triangles.Num() > 0 && ProceduralMesh && Material && CliffCurve;
}

void ATerrain::Regenerate(FScopedSlowTask& Progress)
{
	const float NumOfXVertices = TerrainParameters.Width * TerrainParameters.Density;
	const float NumOfYVertices = TerrainParameters.Height * TerrainParameters.Density;


	UE_LOG(LogTemp, Warning, TEXT("ATerrain::Regenerate"));
	if(!CliffCurve)
	{
		UE_LOG(LogTemp, Error, TEXT("CliffCurve is not set"));
		return;
	}

	for(int32 y = 0; y < NumOfYVertices; y++)
	{
		Progress.EnterProgressFrame(NumOfXVertices, FText::FromString(FString::Printf(TEXT("Generating Vertices..."))));
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
		}
		FPlatformProcess::Sleep(.5f / NumOfYVertices);
	}

	// max and min
	UE_LOG(LogTemp, Warning, TEXT("MaxZ: %f, MinZ: %f"), MaxZ, MinZ);
	UE_LOG(LogTemp, Warning, TEXT("MaxX: %f, MinX: %f"), MaxX, MinX);
	UE_LOG(LogTemp, Warning, TEXT("MaxY: %f, MinY: %f"), MaxY, MinY);

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

	if(ProceduralMesh)
	{
		Progress.EnterProgressFrame(1.f, FText::FromString("Generating Normals"));
		if(Progress.ShouldCancel())
		{
			return;
		}
		FPlatformProcess::Sleep(0.01f);
		UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UVCoords, Normals, Tangents);

		Progress.EnterProgressFrame(1.f, FText::FromString("Generating Mesh"));
		if(Progress.ShouldCancel())
		{
			return;
		}
		FPlatformProcess::Sleep(0.01f);
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

		Progress.EnterProgressFrame(1.f, FText::FromString("Setting Material"));
		if(Progress.ShouldCancel())
		{
			return;
		}
		FPlatformProcess::Sleep(0.01f);
		ProceduralMesh->SetMaterial(0, Material);

	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ProceduralMesh is not set"));
	}
}


// Called every frame
void ATerrain::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

}
