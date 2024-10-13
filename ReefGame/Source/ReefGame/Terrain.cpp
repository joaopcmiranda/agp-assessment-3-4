// Fill out your copyright notice in the Description page of Project Settings.


#include "Terrain.h"

#include "Kismet/KismetSystemLibrary.h"
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
	Progress.EnterProgressFrame(1.f, FText::FromString("Clearing Triangles"));
	Triangles.Empty();
	Progress.EnterProgressFrame(1.f, FText::FromString("Clearing Vertices"));
	Vertices.Empty();
	Progress.EnterProgressFrame(1.f, FText::FromString("Clearing UVs"));
	UVCoords.Empty();
	Progress.EnterProgressFrame(1.f, FText::FromString("Clearing Mesh"));
	ProceduralMesh->ClearAllMeshSections();
}

float ATerrain::CalculateDistanceToCentre(const float X, const float Y) const
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

float ATerrain::CalculateHeight(const float X, const float Y) const
{
	const float SandNoise = FMath::PerlinNoise2D(FVector2d(X * TerrainParameters.SandRoughness, Y * TerrainParameters.SandRoughness)) * TerrainParameters
	.SandBankHeight;
	const float ZCurve = CliffCurve->GetVectorValue(CalculateDistanceToCentre(X, Y)).Z;

	const float Modifier = FMath::PerlinNoise2D(
		FVector2d((X + TerrainParameters.CliffModifierSeed) * TerrainParameters.CliffModifierDensity,
		          (Y + TerrainParameters.CliffModifierSeed) * TerrainParameters.CliffModifierDensity)) * TerrainParameters.CliffModifierIntensity;

	const float SandbankHeight = SandNoise * (1 - ZCurve);
	const float CliffHeight = ZCurve * TerrainParameters.CliffIntensity;

	return SandbankHeight + CliffHeight + Modifier;
}

FVector ATerrain::CalculateDisplacement(const float X, const float Y) const
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

void ATerrain::Regenerate(FScopedSlowTask& Progress)
{
	UE_LOG(LogTemp, Warning, TEXT("ATerrain::Regenerate"));
	if(!CliffCurve)
	{
		UE_LOG(LogTemp, Error, TEXT("CliffCurve is not set"));
		return;
	}

	const float NumOfXVertices = TerrainParameters.Width * TerrainParameters.Density;
	const float NumOfYVertices = TerrainParameters.Height * TerrainParameters.Density;

	for(int32 y = 0; y < NumOfYVertices; y++)
	{
		for(int32 x = 0; x < NumOfXVertices; x++)
		{
			Progress.EnterProgressFrame(1.f, FText::FromString(FString::Printf(TEXT("Generating Vertices at %d, %d"), x, y)));
			const FVector2d Position = FVector2D(x / TerrainParameters.Density, y / TerrainParameters.Density);
			FVector         Vec = FVector(Position.X, Position.Y, 0) + CalculateDisplacement(Position.X, Position.Y);

			Vertices.Add(Vec);

			UVCoords.Add(FVector2D(x, y));
		}
	}

	for(int32 y = 0; y < NumOfYVertices - 1; y++)
	{
		for(int32 x = 0; x < NumOfXVertices - 1; x++)
		{
			Progress.EnterProgressFrame(1.f, FText::FromString(FString::Printf(TEXT("Generating Triangles at %d, %d"), x, y)));
			Triangles.Add(x + y * NumOfXVertices);
			Triangles.Add(x + (y + 1) * NumOfXVertices);
			Triangles.Add(x + 1 + y * NumOfXVertices);

			Triangles.Add(x + 1 + y * NumOfXVertices);
			Triangles.Add(x + (y + 1) * NumOfXVertices);
			Triangles.Add(x + 1 + (y + 1) * NumOfXVertices);
		}
	}

	if(ProceduralMesh)
	{
		Progress.EnterProgressFrame(1.f, FText::FromString("Generating Normals"));
		UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UVCoords, Normals, Tangents);

		Progress.EnterProgressFrame(1.f, FText::FromString("Generating Mesh"));
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
