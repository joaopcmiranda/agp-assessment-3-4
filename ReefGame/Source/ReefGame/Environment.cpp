// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment.h"

#include "Editor.h"
#include "FixedBeing.h"
#include "ReefGameInstance.h"
#include "Terrain.h"
#include "Async/Async.h"
#include "Misc/ScopedSlowTask.h"


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

// LIFECYCLE

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

void AEnvironment::BeginPlay()
{
	Super::BeginPlay();
	InitializeTerrain();

}

void AEnvironment::Tick(float const DeltaTime)
{
	Super::Tick(DeltaTime);

}

// TERRAIN STUFF

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


#if WITH_EDITOR
#include "SpawnedActorsEditorSubsystem.h"

// REGENERATION STUFF

void AEnvironment::Regenerate()
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

void AEnvironment::ForceRegenerate()
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

void AEnvironment::ClearFixedBeings()
{
	USpawnedActorsEditorSubsystem* Manager = GEditor->GetEditorSubsystem<USpawnedActorsEditorSubsystem>();
	Manager->ClearAllSpawnedActors();
}

void AEnvironment::RegenerateEnvironmentInternal()
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
	+ 1 // Fixed Beings Preparation
	+ 1 // Fixed Beings Placement Begin
	+ FixedBeingPlacingPasses * NumOfXVertices * NumOfYVertices // Fixed Beings Placement
	;
	FScopedSlowTask Progress(NumberOfTasks, FText::FromString("Regenerating Environment"));
	Progress.MakeDialog(true, true);

	// Terrain Generation
	Terrain->ClearLandScape(Progress);
	Terrain->Regenerate(Progress);

	// Fixed Beings Placement
	PlaceFixedBeings(Progress);
}

// FIXED BEINGS STUFF

void AEnvironment::ReplenishPicker(TArray<TArray<AFixedBeing*>>& Picker)
{
	// if any of the Picker arrays are empty, add a FixedBeing of the respective class to them
	for(int32 i = 0; i < Picker.Num(); i++)
	{
		if(Picker[i].Num() == 0)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AFixedBeing* Actor = GetWorld()->SpawnActor<AFixedBeing>(FixedBeingsClasses[i], FVector::ZeroVector, FRotator::ZeroRotator,
			                                                         SpawnParams);
			Picker[i].Add(Actor);
		}
	}
}

void AEnvironment::PlaceFixedBeings(FScopedSlowTask& Progress)
{
	if(!Terrain)
	{
		UE_LOG(LogTemp, Error, TEXT("Terrain is not set"));
		return;
	}

	if(FixedBeingsClasses.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No beings to place"));
		return;
	}

	Progress.EnterProgressFrame(1.f, FText::FromString(FString::Printf(TEXT("Preparing Fixed Beings..."))));

	if(Progress.ShouldCancel())
	{
		return;
	}

	TArray<TArray<AFixedBeing*>> Picker;
	Picker.Init(TArray<AFixedBeing*>(), FixedBeingsClasses.Num());

	USpawnedActorsEditorSubsystem* Manager = GEditor->GetEditorSubsystem<USpawnedActorsEditorSubsystem>();
	TArray<AFixedBeing*>*          FixedBeings = &(Manager->SpawnedActors);

	// empty Beings into Picker according to class
	for(auto Actor : *FixedBeings)
	{
		for(int32 i = 0; i < FixedBeingsClasses.Num(); i++)
		{
			if(Actor->IsA(FixedBeingsClasses[i]))
			{
				Actor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
				Picker[i].Add(Actor);
				break;
			}
		}
	}

	FixedBeings->Empty();

	ReplenishPicker(Picker);


	// We have now a full picker to get Beings from

	for(int32 i = 1; i <= FixedBeingPlacingPasses; i++)
	{
		PlaceFixedBeingsPass(Picker, i, Progress);
	}

	// clean picker
	for(auto& Array : Picker)
	{
		for(const auto Actor : Array)
		{
			Actor->Destroy();
		}
		Array.Empty();
	}
}

void AEnvironment::PlaceFixedBeingsPass(TArray<TArray<AFixedBeing*>>& Picker, const int32& Pass, FScopedSlowTask& Progress)
{

	Progress.EnterProgressFrame(1.f, FText::FromString(FString::Printf(TEXT("Placing Fixed Beings Pass %d..."), Pass)));

	if(Progress.ShouldCancel())
	{
		return;
	}
	const int32 NumOfXVertices = Width * Density;
	const int32 NumOfYVertices = Height * Density;

	USpawnedActorsEditorSubsystem* Manager = GEditor->GetEditorSubsystem<USpawnedActorsEditorSubsystem>();
	TArray<AFixedBeing*>*          FixedBeings = &(Manager->SpawnedActors);

	int32 y = 0;

	while(y < NumOfYVertices)
	{
		int32 x = 0;

		while(x < NumOfXVertices)
		{
			const float   Depth = Terrain->CalculateDepth(x, y);
			const FVector Normal = Terrain->CalculateNormal(x, y).GetSafeNormal();
			const float   Flatness = FMath::Abs(Normal.Z);
			const bool    bUpsideDown = Normal.Z < 0;

			// check all beings randomly until one matches or all fail
			for(int32 _i = 0; _i < Picker.Num(); _i++)
			{
				int32 const CurrentPickerIndex = FMath::RandRange(0, Picker.Num() - 1);
				if(Picker[CurrentPickerIndex].Num() == 0)
				{
					ReplenishPicker(Picker);
				}

				AFixedBeing* FixedBeing = Picker[CurrentPickerIndex][0];
				if(!FixedBeing->bIsSpawnable) continue;

				// Is upside down
				if(!FixedBeing->bCanBeUpsideDown && bUpsideDown) continue;

				// Depth affinity
				float const SkewedDepthAffinity = FMath::Lerp(FixedBeing->ShallowAffinity, FixedBeing->DeepAffinity, Depth);
				if(FMath::RandRange(0.0f, 1.0f) > SkewedDepthAffinity) continue;

				// Slope affinity
				float const SkewedFlatnessAffinity = FMath::Lerp(FixedBeing->VerticalAffinity, FixedBeing->FlatAffinity, Flatness);
				if(FMath::FRand() > SkewedFlatnessAffinity) continue;

				// place the being
				FVector const Location = Terrain->GetVertexPosition(x, y);

				FixedBeing->SetActorRotation(FRotator::ZeroRotator);

				FVector UpVector = FVector::UpVector;
				FQuat   Rotation;

				if(FixedBeing->bAlwaysPointUp)
				{
					// Up but random rotation
					Rotation = FQuat(FRotator(0, FMath::RandRange(0.0f, 360.0f), 0));
				}
				else
				{
					// Rotate to match normal
					FQuat FromUpToNormal = FQuat::FindBetweenNormals(FVector::UpVector, Normal);

					// Apply a random rotation around the normal axis
					float RandomRotationAroundNormal = FMath::RandRange(0.0f, 360.0f);
					FQuat RandomRotation = FQuat(Normal, FMath::DegreesToRadians(RandomRotationAroundNormal));

					// Combine the rotations
					Rotation = RandomRotation * FromUpToNormal;
				}

				// Move FixedBeing to the correct location and rotation
				FixedBeing->SetActorLocationAndRotation(Location, Rotation);

				FixedBeing->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);

				// remove from picker, add to FixedBeings
				Picker[CurrentPickerIndex].RemoveAt(0);
				FixedBeings->Add(FixedBeing);

				break;
			}

			const int32 XAmount = FMath::Floor((1.f / FixedBeingPlacingPrecision) * FMath::RandRange(0.5f, 1.5f) / Pass);
			x += XAmount;

			if(Progress.ShouldCancel())
			{
				ClearFixedBeings();
				return;
			}
		}
		const int32 YAmount = FMath::Floor((1.f / FixedBeingPlacingPrecision) * FMath::RandRange(0.5f, 1.5f) / Pass);
		y += YAmount;

		Progress.EnterProgressFrame(YAmount * NumOfXVertices, FText::FromString(FString::Printf(TEXT("Placing Fixed Beings Pass %d..."), Pass)));
		FPlatformProcess::Sleep(.5f / NumOfYVertices);

		if(Progress.ShouldCancel())
		{
			ClearFixedBeings();
			return;
		}
	}
}

#endif
