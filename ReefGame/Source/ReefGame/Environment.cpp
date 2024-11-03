#pragma optimize("", off)

#include "Environment.h"
#include "Editor.h"
#include "ProceduralMeshComponent.h"
#include "Async/Async.h"
#include "Flora/FixedBeingsManagerEditorSubsystem.h"
#include "Terrain/Terrain.h"

class UFixedBeingsManagerEditorSubsystem;

AEnvironment::AEnvironment()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create a root scene component
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	RootComponent = RootSceneComponent;


	UE_LOG(LogTemp, Warning, TEXT("AEnvironment::AEnvironment"));
}

// LIFECYCLE

void AEnvironment::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if(auto const FBManager = GEditor->GetEditorSubsystem<UFixedBeingsManagerEditorSubsystem>())
	{
		FBManager->CheckChildren(this);
	}
	if(auto const TManager = GEditor->GetEditorSubsystem<UTerrainManagerEditorSubsystem>())
	{
		TManager->SetMaterial(TerrainMaterial);
		TManager->SetCliffCurve(CliffCurve);
		TManager->CheckChildren(this);
	}
}


// TERRAIN STUFF

FTerrainParameters AEnvironment::GetTerrainParams() const
{
	return FTerrainParameters(
		{
			Width,
			Height,
			Density,
			SandBankHeight,
			SandRoughness,
			PerlinOffset,
			CliffScale,
			CliffIntensity,
			CliffRoughness,
			CliffRoughnessIntensity,
			CliffModifierSeed,
			CliffModifierDensity,
			CliffModifierIntensity
		}
	);
}

// REGENERATION STUFF

#if WITH_EDITOR

void AEnvironment::RegenerateTerrain()
{
	AsyncTask(ENamedThreads::GameThread, [this]
	{
		RegenerateEnvironmentInternal();
	});
}


void AEnvironment::RegenerateEnvironmentInternal()
{
	// Terrain Generation
	auto const TerrainManager = GEditor->GetEditorSubsystem<UTerrainManagerEditorSubsystem>();
	if(!TerrainManager)
	{
		UE_LOG(LogTemp, Error, TEXT("TerrainManager is not set"));
		return;
	}
	if(!TerrainMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("TerrainMaterial is not set"));
		return;
	}
	if(!CliffCurve)
	{
		UE_LOG(LogTemp, Error, TEXT("CliffCurve is not set"));
		return;
	}

	TerrainActor = TerrainManager->GetTerrain(GetTerrainParams(), TerrainMaterial, CliffCurve);
	if(!TerrainActor)
	{
		UE_LOG(LogTemp, Error, TEXT("TerrainActor Failed to generate"));
		return;
	}
	TerrainActor->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);


	// Log Bounding Box
	UE_LOG(LogTemp, Warning, TEXT("Terrain Bounding Box: %s"), *TerrainManager->GetBoundingBox2D().ToString());

	RegenerateFixedBeings();
}

// FIXED BEINGS STUFF

void AEnvironment::RegenerateFixedBeings()
{

	auto const TerrainManager = GEditor->GetEditorSubsystem<UTerrainManagerEditorSubsystem>();
	if(!TerrainManager)
	{
		UE_LOG(LogTemp, Error, TEXT("TerrainManager is not set"));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("RegenerateFixedBeings"));
	if(!TerrainActor || !TerrainManager->IsOk())
	{
		RegenerateTerrain();
		return;
	}

	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		RegenerateFixedBeingsInternal();
	});
}

void AEnvironment::ClearFixedBeings()
{
	auto const FBManager = GEditor->GetEditorSubsystem<UFixedBeingsManagerEditorSubsystem>();
	if(!FBManager)
	{
		UE_LOG(LogTemp, Error, TEXT("FBManager is not set"));
		return;
	}
	FBManager->ClearSpawned();
}


void AEnvironment::RegenerateFixedBeingsInternal()
{

	auto const FBManager = GEditor->GetEditorSubsystem<UFixedBeingsManagerEditorSubsystem>();
	if(!FBManager)
	{
		UE_LOG(LogTemp, Error, TEXT("FBManager is not set"));
		return;
	}
	if(!TerrainActor)
	{
		UE_LOG(LogTemp, Error, TEXT("TerrainActor is not set"));
		return;
	}

	FFixedBeingsParameters const Parameters{FixedBeingPlacingPrecision, FixedBeingPlacingPasses, ClusterRange};

	FBManager->RedistributeFixedBeings(Parameters, this, FixedBeingsClasses);
}


#endif
