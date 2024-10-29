#include "Environment.h"
#include "Editor.h"
#include "ProceduralMeshComponent.h"
#include "Async/Async.h"
#include "Flora/FixedBeingsManagerEditorSubsystem.h"

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
	if(!TerrainMeshComponent)
	{
		RegenerateTerrain();
	}
}

void AEnvironment::PostLoad()
{
	Super::PostLoad();
	if(!TerrainMeshComponent)
	{
		RegenerateTerrain();
	}
}

void AEnvironment::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName const PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	UE_LOG(LogTemp, Log, TEXT("Property changed: %s"), *PropertyName.ToString());

	// if the FixedBeingClasses changed, let the FixedBeingsManagerEditorSubsystem know
	if(PropertyName == GET_MEMBER_NAME_CHECKED(AEnvironment, FixedBeingsClasses))
	{
		auto const Manager = GEditor->GetEditorSubsystem<UFixedBeingsManagerEditorSubsystem>();
		if(Manager)
		{
			Manager->SetFixedBeingsClasses(FixedBeingsClasses);
			UE_LOG(LogTemp, Log, TEXT("FixedBeingsClasses changed"));
		}
	}

	if(!TerrainMeshComponent)
	{
		RegenerateTerrain();
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

	TerrainMeshComponent = TerrainManager->GetTerrain(GetTerrainParams(), TerrainMaterial, CliffCurve);
	TerrainMeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

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
	if(!TerrainMeshComponent || !TerrainManager->IsOk())
	{
		RegenerateTerrain();
		return;
	}

	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		RegenerateFixedBeingsInternal();
	});
}


void AEnvironment::RegenerateFixedBeingsInternal()
{

	auto const FBManager = GEditor->GetEditorSubsystem<UFixedBeingsManagerEditorSubsystem>();
	if(!FBManager)
	{
		UE_LOG(LogTemp, Error, TEXT("FBManager is not set"));
		return;
	}
	if(!TerrainMeshComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("TerrainMeshComponent is not set"));
		return;
	}

	FBManager->RedistributeFixedBeings(
		{
			.FixedBeingPlacingPrecision = .1f,
			.FixedBeingPlacingPasses = 100,
			.ClusterRange = 1000.f
		}, this);
}


#endif
