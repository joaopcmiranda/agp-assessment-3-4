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

void AEnvironment::RegenerateTerrain()
{
	if(!Terrain)
	{
		InitializeTerrain();
	}
	SetTerrainParams();
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		RegenerateEnvironmentInternal();
	});
}

void AEnvironment::RegenerateFixedBeings()
{

	if(!Terrain || !Terrain->IsOk())
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
	USpawnedActorsEditorSubsystem* Manager = GEditor->GetEditorSubsystem<USpawnedActorsEditorSubsystem>();
	Manager->ClearAllSpawnedActors();
	for(int i = Children.Num() - 1; i >= 0; --i)
	{
		if(IsValid(Children[i]) && Children[i]->IsA<AFixedBeing>())
		{
			Children[i]->Destroy();
		}
	}
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

	ClearFixedBeings();
	// Terrain Generation
	Terrain->ClearLandScape(Progress);
	Terrain->Regenerate(Progress);

	// Log Bounding Box
	UE_LOG(LogTemp, Warning, TEXT("Terrain Bounding Box: %s"), *Terrain->GetBoundingBox2D().ToString());

	// Fixed Beings Placement
	PlaceFixedBeings(Progress);
}

// FIXED BEINGS STUFF

void AEnvironment::RegenerateFixedBeingsInternal()
{
	const float NumOfXVertices = Width * Density;
	const float NumOfYVertices = Height * Density;


	const float NumberOfTasks =
	+1 // Fixed Beings Preparation
	+ 1 // Fixed Beings Placement Begin
	+ FixedBeingPlacingPasses * NumOfXVertices * NumOfYVertices // Fixed Beings Placement
	;
	FScopedSlowTask Progress(NumberOfTasks, FText::FromString("Regenerating Environment"));
	Progress.MakeDialog(true, true);

	// Fixed Beings Placement
	PlaceFixedBeings(Progress);
}

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
	TArray<FSpawnedBeing>&         FixedBeings = Manager->SpawnedBeings;

	// empty Beings into Picker according to class
	for(auto [Location, Actor] : FixedBeings)
	{
		if(!IsValid(Actor)) continue;
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

	for(int32 i = Children.Num() - 1; i >= 0; --i)
	{
		if(IsValid(Children[i]) && Children[i]->IsA<AFixedBeing>())
		{
			Children[i]->Destroy();
		}
	}

	Manager->SpawnedBeings.Empty();

	ReplenishPicker(Picker);


	// We have now a full picker to get Beings from

	for(int32 i = 1; i <= FixedBeingPlacingPasses; i++)
	{
		PlaceFixedBeingsPass(Picker, i, Progress);
	}

	// clean picker
	for(auto& Array : Picker)
	{
		for(int32 i = Array.Num() - 1; i >= 0; --i)
		{
			Array[i]->Destroy();
		}
		Array.Empty();
	}
}

void AEnvironment::PlaceFixedBeingInEnvironment(TArray<TArray<AFixedBeing*>>& Picker, const int32& Pass, TArray<FSpawnedBeing>& FixedBeings, int32 y,
                                                int32                         x)
{
	const float   Depth = Terrain->CalculateDepth(x, y);
	const FVector Normal = Terrain->CalculateNormal(x, y).GetSafeNormal();
	const FVector Location = Terrain->GetVertexPosition(x, y);
	const float   Flatness = FMath::Abs(Normal.Z);
	const bool    bUpsideDown = Normal.Z < 0;

	int32 const CurrentPickerIndex = FMath::RandRange(0, Picker.Num() - 1);
	if(Picker[CurrentPickerIndex].Num() == 0)
	{
		ReplenishPicker(Picker);
	}

	AFixedBeing* FixedBeing = Picker[CurrentPickerIndex][0];
	if(!FixedBeing->bIsSpawnable) return;
	if(FMath::FRand() > FixedBeing->Frequency) return;

	// Is upside down
	if(!FixedBeing->bCanBeUpsideDown && bUpsideDown) return;

	// Depth affinity
	float const SkewedDepthAffinity = FMath::Lerp(FixedBeing->ShallowAffinity, FixedBeing->DeepAffinity, Depth);
	if(FMath::RandRange(0.0f, 1.0f) > SkewedDepthAffinity) return;

	// Slope affinity
	float const SkewedFlatnessAffinity = FMath::Lerp(FixedBeing->VerticalAffinity, FixedBeing->FlatAffinity, Flatness);
	if(FMath::FRand() > SkewedFlatnessAffinity) return;


	// Clustering affinity


	float SelfClusterPositiveScore = 1.f - FixedBeing->SelfClusterAfinity;
	float SelfClusterNegativeScore = 1.0f;
	float OthersClusterPositiveScore = 1.f - FixedBeing->OthersClusterAfinity;
	float OthersClusterNegativeScore = 1.0f;

	int SelfNearbyCount = 0;
	int OthersNearbyCount = 0;

	if(FixedBeings.Num())
	{
		for(const auto& [OtherLocation, Other] : FixedBeings)
		{
			const float Distance = FVector::Distance(Location, OtherLocation);
			if(Distance < FMath::Max(FixedBeing->MinimumSpacing, Other->MinimumSpacing))
			{
				return;
			}

			if(Distance < ClusterRange)
			{

				const float Weight = FMath::Clamp(1.0f - (Distance - FixedBeing->MinimumSpacing) / ClusterRange, 0.0f, 1.0f);

				if(Other->GetClass() == FixedBeing->GetClass())
				{
					SelfNearbyCount++;
					SelfClusterNegativeScore -= Weight * (FixedBeing->SelfClusterAversion);
					SelfClusterPositiveScore += Weight * (FixedBeing->SelfClusterAfinity);
				}
				else
				{
					OthersNearbyCount++;
					OthersClusterNegativeScore -= Weight * (FixedBeing->OthersClusterAversion);
					OthersClusterPositiveScore += Weight * (FixedBeing->OthersClusterAfinity);
				}

			}
		}
	}

	if(FMath::FRand() > SelfClusterNegativeScore) return;
	if(FMath::FRand() > SelfClusterPositiveScore) return;
	if(FMath::FRand() > OthersClusterNegativeScore) return;
	if(FMath::FRand() > OthersClusterPositiveScore) return;
	//
	// // item # passed
	// UE_LOG (LogTemp, Warning, TEXT("PASSED: item# %d (%s): SelfClusterPositiveScore: %f, SelfClusterNegativeScore: %f, OthersClusterPositiveScore: %f, OthersClusterNegativeScore: %f, Pass: %d, SelfNearbyCount: %d, OthersNearbyCount: %d"),
	// 	x * y,
	// 	*FixedBeing->GetClass()->GetName(),
	// 	SelfClusterPositiveScore,
	// 	SelfClusterNegativeScore,
	// 	OthersClusterPositiveScore,
	// 	OthersClusterNegativeScore,
	// 	Pass,
	// 	SelfNearbyCount,
	// 	OthersNearbyCount
	// );

	// store audit info
	FixedBeing->ItemNumber = x * y;
	FixedBeing->SkewedDepthAffinity = SkewedDepthAffinity;
	FixedBeing->SkewedFlatnessAffinity = SkewedFlatnessAffinity;
	FixedBeing->SelfClusterPositiveScore = SelfClusterPositiveScore;
	FixedBeing->SelfClusterNegativeScore = SelfClusterNegativeScore;
	FixedBeing->OthersClusterPositiveScore = OthersClusterPositiveScore;
	FixedBeing->OthersClusterNegativeScore = OthersClusterNegativeScore;
	FixedBeing->PlacementPass = Pass;
	FixedBeing->NumberOfBeingsWhenPlaced = FixedBeings.Num();
	FixedBeing->NumberOfBeingsNearby = SelfNearbyCount + OthersNearbyCount;
	FixedBeing->NumberOfOtherBeingsNearby = OthersNearbyCount;
	FixedBeing->NumberOfSameBeingsNearby = SelfNearbyCount;
	FixedBeing->ClusterRadius = ClusterRange;

	// place the being
	FixedBeing->SetActorRotation(FRotator::ZeroRotator);

	FQuat Rotation;

	if(FixedBeing->bAlwaysPointUp)
	{
		// Up but random rotation
		Rotation = FQuat(FRotator(0, FMath::RandRange(0.0f, 360.0f), 0));
	}
	else
	{
		// Rotate to match normal
		const FQuat FromUpToNormal = FQuat::FindBetweenNormals(FVector::UpVector, Normal);

		// Apply a random rotation around the normal axis
		const float RandomRotationAroundNormal = FMath::RandRange(0.0f, 360.0f);
		const FQuat RandomRotation = FQuat(Normal, FMath::DegreesToRadians(RandomRotationAroundNormal));

		// Combine the rotations
		Rotation = RandomRotation * FromUpToNormal;
	}

	// Move FixedBeing to the correct location and rotation
	FixedBeing->SetActorLocationAndRotation(Location, Rotation);

	FixedBeing->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);

	// remove from picker, add to FixedBeings
	Picker[CurrentPickerIndex].RemoveAt(0);
	FixedBeings.Add(FSpawnedBeing{Location, FixedBeing});
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
	TArray<FSpawnedBeing>&         FixedBeings = Manager->SpawnedBeings;

	int32 y = FMath::Floor((1.f / FixedBeingPlacingPrecision) * FMath::RandRange(0.5f, 1.5f) / Pass);

	while(y < NumOfYVertices)
	{
		int32 x = FMath::Floor((1.f / FixedBeingPlacingPrecision) * FMath::RandRange(0.5f, 1.5f) / Pass);

		while(x < NumOfXVertices)
		{

			PlaceFixedBeingInEnvironment(Picker, Pass, FixedBeings, y, x);

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
