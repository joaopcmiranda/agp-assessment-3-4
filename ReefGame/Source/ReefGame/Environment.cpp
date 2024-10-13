// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment.h"

#include "Terrain.h"
#include "Misc/ScopedSlowTask.h"

void AEnvironment::RegenerateEnvironment()
{
	UE_LOG(LogTemp, Log, TEXT("Regenerating Environment..."));
	LogTerrainStatus();
	if(!Terrain)
	{
		InitializeTerrain();
	}
	// If the terrain parameters have not changed, do not regenerate the environment
	if(!SetTerrainParams())
	{
		return;
	}
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		RegenerateEnvironmentInternal();
	});
}

void AEnvironment::ForceRegenerateEnvironment()
{
	if(!Terrain)
	{
		InitializeTerrain();
	}
	// If the terrain parameters have not changed, do not regenerate the environment
	SetTerrainParams();
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		RegenerateEnvironmentInternal();
	});
}

void AEnvironment::RegenerateEnvironmentInternal() const
{
	const float NumOfXVertices = Width * Density;
	const float NumOfYVertices = Height * Density;


	const float NumberOfTasks =
	+4 // Clearing the landscape
	+ NumOfXVertices * NumOfYVertices // Vertex/UV generation
	+ (NumOfXVertices - 1) * (NumOfYVertices - 1) // Triangle generation
	+ 1 // Normals generation
	+ 1 // Procedural mesh generation
	+ 1 // Set Material
	;
	FScopedSlowTask Progress(NumberOfTasks, FText::FromString("Regenerating Environment"));
	Progress.MakeDialog(true, true);

	// Terrain Generation
	Terrain->ClearLandScape(Progress);
	Terrain->Regenerate(Progress);
}

bool AEnvironment::SetTerrainParams()
{
	auto const NewParameters = FTerrainParameters(
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

	if(!Terrain)
	{
		UE_LOG(LogTemp, Warning, TEXT("Terrain is not set"));
		return false;
	}

	Terrain->SetTerrainParameters(NewParameters);
	if(NewParameters == CachedTerrainParameters)
	{
		UE_LOG(LogTemp, Warning, TEXT("Terrain parameters have not changed"));
		return false;
	}

	CachedTerrainParameters = NewParameters;
	return true;
}


AEnvironment::AEnvironment()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create a root scene component
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	RootComponent = RootSceneComponent;

	// Create a ChildActorComponent for the terrain
	TerrainComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("TerrainComponent"));
	TerrainComponent->SetupAttachment(RootComponent);

	// Set the default class for the terrain
	TerrainComponent->SetChildActorClass(ATerrain::StaticClass());

	if(ATerrain* TerrainActor = Cast<ATerrain>(TerrainComponent->GetChildActor()))
	{
		Terrain = TerrainActor;
	}

	UE_LOG(LogTemp, Warning, TEXT("AEnvironment::AEnvironment"));
}


void AEnvironment::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	InitializeTerrain();
}

void AEnvironment::PostLoad()
{
	Super::PostLoad();
	InitializeTerrain();
}


void AEnvironment::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	UE_LOG(LogTemp, Log, TEXT("Property changed: %s"), *PropertyName.ToString());

	InitializeTerrain();
}

// Called when the game starts or when spawned
void AEnvironment::BeginPlay()
{
	Super::BeginPlay();
	InitializeTerrain();

}


// Called every frame
void AEnvironment::Tick(float const DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnvironment::InitializeTerrain()
{
	UE_LOG(LogTemp, Log, TEXT("Initializing Terrain..."));
	LogTerrainStatus();

	if(!TerrainComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("TerrainComponent is null in AEnvironment"));
		return;
	}

	if(!TerrainComponent->GetChildActorClass())
	{
		UE_LOG(LogTemp, Error, TEXT("TerrainComponent's ChildActorClass is not set"));
		return;
	}

	if(!TerrainComponent->GetChildActor())
	{
		UE_LOG(LogTemp, Warning, TEXT("TerrainComponent's ChildActor is null, attempting to spawn..."));
		TerrainComponent->CreateChildActor();
	}

	Terrain = Cast<ATerrain>(TerrainComponent->GetChildActor());
	if(Terrain)
	{
		UE_LOG(LogTemp, Log, TEXT("Terrain initialized successfully"));
		Terrain->SetMaterial(TerrainMaterial);
		Terrain->SetCliffCurve(CliffCurve);
		// Set other necessary parameters
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to initialize Terrain in AEnvironment"));
	}

	LogTerrainStatus();
}


void AEnvironment::LogTerrainStatus()
{
	UE_LOG(LogTemp, Log, TEXT("TerrainComponent valid: %s"), TerrainComponent ? TEXT("True") : TEXT("False"));
	if(TerrainComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("TerrainComponent's ChildActor class: %s"),
		       *GetNameSafe(TerrainComponent->GetChildActorClass()));
		UE_LOG(LogTemp, Log, TEXT("TerrainComponent's ChildActor: %s"),
		       *GetNameSafe(TerrainComponent->GetChildActor()));
	}
	UE_LOG(LogTemp, Log, TEXT("Terrain pointer valid: %s"), Terrain ? TEXT("True") : TEXT("False"));
}
