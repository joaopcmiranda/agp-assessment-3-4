#include "FixedBeingsManagerEditorSubsystem.h"
#include "FixedBeing.h"
#include "AssetRegistry/AssetRegistryModule.h"
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

void UFixedBeingsManagerEditorSubsystem::ReleaseAll()
{
    SpawnedBeings.Empty();
    ClearPicker();
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
	for(auto [_, WeakBeing] : SpawnedBeings)
	{
		if(auto Being = WeakBeing.Get())
		{
			if(IsValid(Being))
			{
				Being->Destroy();
			}
		}
	}
	SpawnedBeings.Empty();
}

void UFixedBeingsManagerEditorSubsystem::CheckChildren(const AActor* Parent)
{;

	TArray<AActor*> Children;
	Parent->GetAttachedActors(Children);

	// First, clean up invalid references in SpawnedBeings
	SpawnedBeings.RemoveAll([](const FSpawnedBeing& SpawnedBeing) {
		return !SpawnedBeing.Being.IsValid();
	});

	// iterate through the actors, if they are fixed beings check if they are in the SpawnedBeings or Picker_Internal, if not, destroy them
	for(const auto& Child : Children)
	{
		if(AFixedBeing* FixedBeing = Cast<AFixedBeing>(Child))
		{
			bool bFound = false;
			for(auto& [_, WSpawnedBeing] : SpawnedBeings)
			{
				if(WSpawnedBeing.Get() == FixedBeing)
				{
					bFound = true;
					break;
				}
			}

			// If not found in SpawnedBeings, check Picker
			if(!bFound)
			{
				for(auto& Array : Picker_Internal)
				{
					for(int32 i = Array.Num()-1; i >= 0; --i)
					{
						if(Array[i] == FixedBeing)
						{
							// Move from picker to spawned
							SpawnedBeings.Add(FSpawnedBeing{FixedBeing->GetActorLocation(), FixedBeing});
							Array.RemoveAt(i);

							// unHide the actor
							FixedBeing->SetActorHiddenInGame(false);

							// Enable collision
							FixedBeing->SetActorEnableCollision(true);

							FixedBeing->SetActorTickEnabled(true);
							bFound = true;
							break;
						}
					}
					if(bFound) break;
				}
			}

			// If not found in either collection, register it
			if(!bFound)
			{
				SpawnedBeings.Add(FSpawnedBeing{FixedBeing->GetActorLocation(), FixedBeing});
			}
		}
	}
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
	for(auto [_, WActor] : SpawnedBeings)
	{
		if(WActor.IsValid() && IsValid(WActor.Get()))
		{
			auto Actor = WActor.Get();
			for(int8 i = 0; i < FixedBeingsClasses.Num(); i++)
			{
				if(Actor->IsA(FixedBeingsClasses[i]))
				{
					Picker[i].Add(Actor);
					break;
				}
			}
		}
	}
	// remove from spawned
	ClearSpawned();
}

void FIndividualPicker::Add(AFixedBeing* const Being)
{

	// Detach from parent
	Being->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// Hide the actor
	Being->SetActorHiddenInGame(true);

	// Disable collision to make sure it does not interfere with other objects
	Being->SetActorEnableCollision(false);

	Being->SetActorTickEnabled(false);
	Beings.Add(Being);
}

void FIndividualPicker::RemoveAt(int32 const Index)
{ Beings.RemoveAt(Index); }

/**
 * @brief Retrieves and initializes the picker array for fixed beings.
 *
 * This function returns a reference to the Picker_Internal array. If the Picker_Internal array's size does not match
 * the FixedBeingsClasses array size, or if the bDirty flag is set, it reinitialises the Picker_Internal array. When reinitialised,
 * it spawns instances of AFixedBeing for each class in the FixedBeingsClasses array if they are not already present so that we can use them wehn placing in the world.
 *
 * @return A reference to the Picker array.
 */
TArray<FIndividualPicker>& UFixedBeingsManagerEditorSubsystem::GetPicker()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if(!World)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get World context"));
		return Picker_Internal;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.ObjectFlags = RF_Transactional;

	if(Picker_Internal.Num() != FixedBeingsClasses.Num() || bDirty)
	{
		ClearPicker();
		Picker_Internal.Init(FIndividualPicker(), FixedBeingsClasses.Num());
	}
	for(int8 i = 0; i < Picker_Internal.Num(); i++)
	{
		if(Picker_Internal[i].Num() == 0)
		{

			AFixedBeing* Actor = World->SpawnActor<AFixedBeing>(FixedBeingsClasses[i], FVector::ZeroVector, FRotator::ZeroRotator,
			                                                    SpawnParams);
			Picker_Internal[i].Add(Actor);
		}
	}
	bDirty = false;
	return Picker_Internal;
}

// Placement

void UFixedBeingsManagerEditorSubsystem::RedistributeFixedBeings(FFixedBeingsParameters                  NewParameters, AActor* Parent,
                                                                 TArray<TSubclassOf<AFixedBeing>> const& NewClasses)

{

	// check if classes are the same
	bool bFound = false;
	for(auto& NewClass : NewClasses)
	{
		for(auto& OldClass : FixedBeingsClasses)
		{
			if(NewClass == OldClass)
			{
				bFound = true;
				break;
			}
		}
	}
	for(auto& OldClass : FixedBeingsClasses)
	{
		for(auto& NewClass : NewClasses)
		{
			if(OldClass == NewClass)
			{
				bFound = true;
				break;
			}
		}
	}
	if(!bFound)
	{
		FixedBeingsClasses = NewClasses;
		bDirty = true;
	}

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
	Progress.MakeDialog(true, true);

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

	float y = GetDeterministicStep(Pass, NumOfYVertices);

	while(y < NumOfYVertices)
	{
		float x = GetDeterministicStep(Pass, NumOfXVertices);

		while(x < NumOfXVertices)
		{

			PlaceFixedBeingInEnvironment(Pass, y, x, Parent);

			const float XAmount = GetDeterministicStep(Pass, NumOfXVertices);
			x += XAmount;

			if(Progress.ShouldCancel())
			{
				ClearAll();
				return;
			}
		}
		const float YAmount = GetDeterministicStep(Pass, NumOfYVertices);
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

void UFixedBeingsManagerEditorSubsystem::PlaceFixedBeingInEnvironment(const int32& Pass, float const Y, float const X, AActor* Parent)
{
	const float   Depth = TerrainManager->GetDepthPercentage(X, Y);
	const FVector Normal = TerrainManager->GetNormal(X, Y);
	const FVector Location = TerrainManager->GetVertexPosition(X, Y);
	const float   Flatness = FMath::Abs(Normal.Z);
	const bool    bUpsideDown = Normal.Z < 0;

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
	auto const  DepthRandom = GetDetRand0To1();
	if(DepthRandom > SkewedDepthAffinity) return;

	// Slope affinity
	const float SkewedFlatnessAffinity = FMath::Lerp(FixedBeing->VerticalAffinity, FixedBeing->FlatAffinity, Flatness);
	auto const  SlopeRandom = GetDetRand0To1();
	if(SlopeRandom > SkewedFlatnessAffinity) return;

	// Clustering affinity

	float SelfClusterPositiveScore = 1.f - FixedBeing->SelfClusterAfinity;
	float SelfClusterNegativeScore = 1.0f;
	float OthersClusterPositiveScore = 1.f - FixedBeing->OthersClusterAfinity;
	float OthersClusterNegativeScore = 1.0f;

	int SelfNearbyCount = 0;
	int OthersNearbyCount = 0;

	if(SpawnedBeings.Num())
	{
		for(auto& [OtherLocation, WOther] : SpawnedBeings)
		{
			const auto Other = WOther.Get();
			if(!Other) continue;
			const float Distance = FVector::Distance(Location, OtherLocation);
			if(Distance < FMath::Max(FixedBeing->MinimumSpacing, Other->MinimumSpacing))
			{
				return;
			}

			if(Distance < Parameters.ClusterRange)
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


	// unHide the actor
	FixedBeing->SetActorHiddenInGame(false);

	// Enable collision
	FixedBeing->SetActorEnableCollision(true);

	FixedBeing->SetActorTickEnabled(true);

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
	const float GoldenRatioConjugate = (FMath::Sqrt(5.f) - 1) / 2;

	SelectionSeed = (SelectionSeed + 1) % 100000000;
	return FMath::Fmod(SelectionSeed * SelectionSeed * GoldenRatioConjugate, 1.0f);
}

/**
 * @brief Computes a deterministic step to move the  placer.
 *
 * @param Pass The current pass number, affecting the step size calculation.
 * @param ArraySize
 *
 * @return The computed step size as an integer.
 */
float UFixedBeingsManagerEditorSubsystem::GetDeterministicStep(const int32& Pass, const int32 ArraySize)
{

	// Use golden ratio to create a mostly even distribution
	const double GoldenRatioConjugate = (FMath::Sqrt(5.0) - 1.0) / 2.0;

	StepSeed = (StepSeed + 1) % 100000000;
	const double RandomFrac = 0.1  + (FMath::Fmod(StepSeed * GoldenRatioConjugate, 1.0 ));

	// Precision will pull it down for smaller increments, Pass will also pull it down
	return RandomFrac * (ArraySize / (Pass * Parameters.FixedBeingPlacingPrecision));
}

/**
 * @brief Generates a deterministic index for selecting a fixed being class.
 *
 * @return An integer representing the index of the fixed being class to be selected,
 *         or 0 if there are no classes available.
 */
int32 UFixedBeingsManagerEditorSubsystem::GetDeterministicPickerIndex()
{
	const int32 NumClasses = FixedBeingsClasses.Num();
	if(NumClasses <= 0)
	{
		return 0;
	}

	// Use golden ratio to create a mostly even distribution
	const double GoldenRatioConjugate = (FMath::Sqrt(5.0) - 1.0) / 2.0;

	PickerIndexSeed = (PickerIndexSeed + 1) % 100000;

	// Use doubles consistently and ensure positive result
	double result = PickerIndexSeed * PickerIndexSeed * GoldenRatioConjugate;
	result = FMath::Fmod(result, 1.0);  // Use 1.0 not 1.0f
	if(result < 0) result = FMath::Abs(result);  // Ensure positive

	return FMath::Floor(result * NumClasses);
}
