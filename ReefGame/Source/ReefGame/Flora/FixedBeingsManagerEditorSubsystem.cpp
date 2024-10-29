#include "FixedBeingsManagerEditorSubsystem.h"
#include "FixedBeing.h"
#include "Misc/ScopedSlowTask.h"


// Unreal Overrides

void UFixedBeingsManagerEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UFixedBeingsManagerEditorSubsystem::Deinitialize()
{
	Super::Deinitialize();

	ClearAll();
}

// Setters

/**
 * @brief Sets the seed value for the subsystem and updates related properties.
 *
 * This function sets the Seed, StepSeed, and PickerIndexSeed properties to the provided
 * seed value and marks the subsystem as dirty, indicating that there are unsaved changes.
 *
 * @param NewSeed The new seed value to be used for the subsystem.
 */
void UFixedBeingsManagerEditorSubsystem::SetSeed(int32 NewSeed)
{
	Seed = NewSeed;
	StepSeed = Seed;
	PickerIndexSeed = Seed;
	bDirty = true;
}

/**
 * @brief Updates the list of classes that represent fixed beings.
 *
 * This function sets the FixedBeingsClasses property to the provided list of classes
 * and marks the subsystem as dirty, indicating that there are unsaved changes.
 *
 * @param NewClasses A constant reference to an array of TSubclassOf<AFixedBeing> representing the new classes
 *                   for fixed beings.
 */
void UFixedBeingsManagerEditorSubsystem::SetFixedBeingsClasses(TArray<TSubclassOf<AFixedBeing>> const& NewClasses)
{
	FixedBeingsClasses = NewClasses;
	bDirty = true;
}

// Cleaners
/**
 * @brief Clears all the data managed by the Fixed Beings Manager Editor Subsystem.
 *
 * This function invokes ClearPicker() to clear any selections in the picker tool,
 * and ClearSpawned() to remove all spawned actors from the subsystem.
 */
void UFixedBeingsManagerEditorSubsystem::ClearAll()
{
	ClearPicker();
	ClearSpawned();
}

/**
 * @brief Destroys all spawned beings managed by the subsystem.
 *
 * This function iterates over all actors in the SpawnedBeings array and
 * destroys them if they are valid. After all actors are destroyed, the
 * SpawnedBeings array is cleared to remove all references to these actors.
 */
void UFixedBeingsManagerEditorSubsystem::ClearSpawned()
{
	for(auto [_, Actor] : SpawnedBeings)
	{
		if(Actor && IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
	SpawnedBeings.Empty();
}

/**
 * @brief Clears all the entries in the Picker_Internal array.
 *
 * This function iterates over each sub-array within the Picker_Internal array.
 * For each sub-array, it destroys all valid AFixedBeing objects and then empties the sub-array.
 */
void UFixedBeingsManagerEditorSubsystem::ClearPicker()
{
	for(auto& Array : Picker_Internal)
	{
		for(int32 i = Array.Num() - 1; i >= 0; --i)
		{
			if(Array[i] && IsValid(Array[i]))
			{
				Array[i]->Destroy();
			}
		}
		Array.Empty();
	}
}

// Picker

/**
 * @brief Despawns all currently spawned beings and organizes them into the picker array.
 *
 * This function iterates over all the spawned beings. For each valid actor, detaches the actor from its parent,
 * hides it, disables its collision and tick updates, and then adds it to the appropriate category in the picker.
 * Finally, it clears the list of remaining spawned beings.
 */
void UFixedBeingsManagerEditorSubsystem::DespawnToPicker()
{
	auto& Picker = GetPicker();
	// remove all from the spawned array and organise them in the picker. anything that is not in the picker is destroyed
	// put all in picker
	for(auto [_, Actor] : SpawnedBeings)
	{
		if(Actor && IsValid(Actor))
		{
			for(int8 i = 0; i < FixedBeingsClasses.Num(); i++)
			{
				if(Actor->IsA(FixedBeingsClasses[i]))
				{
					// Detach from parent
					Actor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

					// Hide the actor
					Actor->SetActorHiddenInGame(true);

					// Disable collision to make sure it does not interfere with other objects
					Actor->SetActorEnableCollision(false);

					Actor->SetActorTickEnabled(false);

					Picker[i].Add(Actor);
					break;
				}
			}
		}
	}
	// remove from spawned
	ClearSpawned();
}

/**
 * @brief Retrieves and initializes the picker array for fixed beings.
 *
 * This function returns a reference to the Picker_Internal array. If the Picker_Internal array's size does not match
 * the FixedBeingsClasses array size, or if the bDirty flag is set, it reinitialises the Picker_Internal array. When reinitialised,
 * it spawns instances of AFixedBeing for each class in the FixedBeingsClasses array if they are not already present so that we can use them wehn placing in the world.
 *
 * @return A reference to the Picker array.
 */
TArray<TArray<AFixedBeing*>>& UFixedBeingsManagerEditorSubsystem::GetPicker()
{
	if(Picker_Internal.Num() != FixedBeingsClasses.Num() || bDirty)
	{
		ClearPicker();
		Picker_Internal.Init(TArray<AFixedBeing*>(), FixedBeingsClasses.Num());
	}
	for(int8 i = 0; i < Picker_Internal.Num(); i++)
	{
		if(Picker_Internal[i].Num() == 0)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AFixedBeing* Actor = GetWorld()->SpawnActor<AFixedBeing>(FixedBeingsClasses[i], FVector::ZeroVector, FRotator::ZeroRotator,
			                                                         SpawnParams);
			Picker_Internal[i].Add(Actor);
		}
	}
	return Picker_Internal;
}

// Placement

void UFixedBeingsManagerEditorSubsystem::RedistributeFixedBeings(FFixedBeingsParameters NewParameters, AActor* Parent)
{
	Parameters = NewParameters;

	if(!TerrainManager)
	{
		TerrainManager = GEditor->GetEditorSubsystem<UTerrainManagerEditorSubsystem>();
	}

	if(!TerrainManager || !TerrainManager->IsOk())
	{
		UE_LOG(LogTemp, Error, TEXT("FBManager: Terrain Manager is not set or not properly initialized"));
		return;
	}

	if(FixedBeingsClasses.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("FBManager: No Fixed Beings Classes to be placed"));
		return;
	}

	const float NumberOfTasks =
	+1 // Fixed Beings Preparation
	+ 1 // Fixed Beings Placement Begin
	+ Parameters.FixedBeingPlacingPasses * TerrainManager->NumOfXVertices * TerrainManager->NumOfYVertices // Fixed Beings Placement
	;
	FScopedSlowTask Progress(NumberOfTasks, FText::FromString("Redistributing Fixed Beings"));

	Progress.EnterProgressFrame(1.f, FText::FromString(FString::Printf(TEXT("Preparing Fixed Beings..."))));

	if(Progress.ShouldCancel())
	{
		return;
	}

	DespawnToPicker();

	for(int32 i = 1; i <= Parameters.FixedBeingPlacingPasses; i++)
	{
		PlaceFixedBeingsPass(i, Progress, Parent);
	}

	ClearPicker();
	bDirty = false;
}

void UFixedBeingsManagerEditorSubsystem::PlaceFixedBeingsPass(const int32& Pass, FScopedSlowTask& Progress, AActor* Parent)
{

	Progress.EnterProgressFrame(1.f, FText::FromString(FString::Printf(TEXT("Placing Fixed Beings Pass %d..."), Pass)));

	if(Progress.ShouldCancel())
	{
		return;
	}

	const int32 NumOfXVertices = TerrainManager->NumOfXVertices;
	const int32 NumOfYVertices = TerrainManager->NumOfYVertices;

	int32 y = GetDeterministicStep(Pass);

	while(y < NumOfYVertices)
	{
		int32 x = GetDeterministicStep(Pass);

		while(x < NumOfXVertices)
		{

			PlaceFixedBeingInEnvironment(Pass, y, x, Parent);

			const int32 XAmount = GetDeterministicStep(Pass);
			x += XAmount;

			if(Progress.ShouldCancel())
			{
				ClearAll();
				return;
			}
		}
		const int32 YAmount = GetDeterministicStep(Pass);
		y += YAmount;

		Progress.EnterProgressFrame(YAmount * NumOfXVertices, FText::FromString(FString::Printf(TEXT("Placing Fixed Beings Pass %d..."), Pass)));
		FPlatformProcess::Sleep(.5f / NumOfYVertices);

		if(Progress.ShouldCancel())
		{
			ClearAll();
			return;
		}
	}
}

void UFixedBeingsManagerEditorSubsystem::PlaceFixedBeingInEnvironment(const int32& Pass, int32 const Y, int32 const X, AActor* Parent)
{
	const float Depth = TerrainManager->GetDepthPercentage(X, Y);
	const FVector Normal = TerrainManager->GetNormal(X, Y);
	const FVector Location = TerrainManager->GetVertexPosition(X, Y);
	const float Flatness = FMath::Abs(Normal.Z);
	const bool bUpsideDown = Normal.Z < 0;

	auto& Picker = GetPicker();

	const int32 CurrentPickerIndex = GetDeterministicPickerIndex();
	if(Picker[CurrentPickerIndex].Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("FBManager: Picker is empty"));
		return;
	}

	AFixedBeing* FixedBeing = Picker[CurrentPickerIndex][0];
	if(!FixedBeing->bIsSpawnable) return;
	if(GetDetRand0To1() > FixedBeing->Frequency) return;

	// Is upside down
	if(!FixedBeing->bCanBeUpsideDown && bUpsideDown) return;

	// Depth affinity
	const float SkewedDepthAffinity = FMath::Lerp(FixedBeing->ShallowAffinity, FixedBeing->DeepAffinity, Depth);
	if(GetDetRand0To1() > SkewedDepthAffinity) return;

	// Slope affinity
	const float SkewedFlatnessAffinity = FMath::Lerp(FixedBeing->VerticalAffinity, FixedBeing->FlatAffinity, Flatness);
	if(GetDetRand0To1() > SkewedFlatnessAffinity) return;

	// Clustering affinity

	float SelfClusterPositiveScore = 1.f - FixedBeing->SelfClusterAfinity;
	float SelfClusterNegativeScore = 1.0f;
	float OthersClusterPositiveScore = 1.f - FixedBeing->OthersClusterAfinity;
	float OthersClusterNegativeScore = 1.0f;

	int SelfNearbyCount = 0;
	int OthersNearbyCount = 0;

	if(SpawnedBeings.Num())
	{
		for(auto& [OtherLocation, Other] : SpawnedBeings)
		{
			const float Distance = FVector::Distance(Location, OtherLocation);
			if(Distance < FMath::Max(FixedBeing->MinimumSpacing, Other->MinimumSpacing))
			{
				return;
			}

			if (Distance < Parameters.ClusterRange)
			{

				const float Weight = FMath::Clamp(1.0f - (Distance - FixedBeing->MinimumSpacing) / Parameters.ClusterRange, 0.0f, 1.0f);

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

	if(GetDetRand0To1() > SelfClusterNegativeScore) return;
	if(GetDetRand0To1() > SelfClusterPositiveScore) return;
	if(GetDetRand0To1() > OthersClusterNegativeScore) return;
	if(GetDetRand0To1() > OthersClusterPositiveScore) return;


	// store audit info
	FixedBeing->ItemNumber = X * Y;
	FixedBeing->SkewedDepthAffinity = SkewedDepthAffinity;
	FixedBeing->SkewedFlatnessAffinity = SkewedFlatnessAffinity;
	FixedBeing->SelfClusterPositiveScore = SelfClusterPositiveScore;
	FixedBeing->SelfClusterNegativeScore = SelfClusterNegativeScore;
	FixedBeing->OthersClusterPositiveScore = OthersClusterPositiveScore;
	FixedBeing->OthersClusterNegativeScore = OthersClusterNegativeScore;
	FixedBeing->PlacementPass = Pass;
	FixedBeing->NumberOfBeingsWhenPlaced = SpawnedBeings.Num();
	FixedBeing->NumberOfBeingsNearby = SelfNearbyCount + OthersNearbyCount;
	FixedBeing->NumberOfOtherBeingsNearby = OthersNearbyCount;
	FixedBeing->NumberOfSameBeingsNearby = SelfNearbyCount;
	FixedBeing->ClusterRadius = Parameters.ClusterRange;

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

	FixedBeing->AttachToActor(Parent, FAttachmentTransformRules::KeepRelativeTransform);

	// remove from picker, add to FixedBeings
	Picker[CurrentPickerIndex].RemoveAt(0);
	SpawnedBeings.Add(FSpawnedBeing{Location, FixedBeing});
}

// Deterministic Random

/**
 * @brief Generates a deterministic random floating-point number between 0 and 1.
 *
 * @return A pseudo-random float between 0 and 1.
 */
float UFixedBeingsManagerEditorSubsystem::GetDetRand0To1()
{
	// Use golden ratio to create a mostly even distribution
	constexpr float GoldenRatioConjugate = 0.618033988749895f;

	SelectionSeed++;
	return FMath::Fmod(SelectionSeed * GoldenRatioConjugate, 1.0f);
}

/**
 * @brief Computes a deterministic step to move the  placer.
 *
 * @param Pass The current pass number, affecting the step size calculation.
 *
 * @return The computed step size as an integer.
 */
int32 UFixedBeingsManagerEditorSubsystem::GetDeterministicStep(const int32& Pass)
{
	// Use golden ratio to create a mostly even distribution
	constexpr float GoldenRatioConjugate = 0.618033988749895f;

	StepSeed++;
	const float RandomFrac = 0.5f + (FMath::Fmod(StepSeed * GoldenRatioConjugate, 1.0f));

	// Precision will pull it down for smaller increments, Pass will also pull it down
	return FMath::Floor( RandomFrac / (Pass * Parameters.FixedBeingPlacingPrecision));
}

/**
 * @brief Generates a deterministic index for selecting a fixed being class.
 *
 * @return An integer representing the index of the fixed being class to be selected,
 *         or 0 if there are no classes available.
 */
int32 UFixedBeingsManagerEditorSubsystem::GetDeterministicPickerIndex()
{
	PickerIndexSeed++;

	const int32 NumClasses = FixedBeingsClasses.Num();
	if(NumClasses <= 0)
	{
		return 0;
	}

	// Use Primes for a simple distribution
	return (PickerIndexSeed * 13 + PickerIndexSeed * 7) % NumClasses;
}
