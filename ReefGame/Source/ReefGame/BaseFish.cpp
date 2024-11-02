// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseFish.h"
#include "HealthComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "HighlightComponent.h"

// Sets default values
ABaseFish::ABaseFish()
{
	PrimaryActorTick.bCanEverTick = true;
	HealthComponent = CreateDefaultSubobject<UHealthComponent>("Health Component");

	HighlightComponent = CreateDefaultSubobject<UHighlightComponent>(TEXT("Highlight Component"));

	//setupcollision component and set as root
	FishCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Component"));
	RootComponent = FishCollision;
	
	FishCollision->SetCollisionObjectType(ECC_Pawn);
	FishCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FishCollision->SetCollisionResponseToAllChannels(ECR_Overlap);
	
	//setup mesh component & attach to root
	FishMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh Component"));
	FishMesh->SetupAttachment(RootComponent);
	FishMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FishMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

	//setup cohesion sensor component
	PerceptionSensor = CreateDefaultSubobject<USphereComponent>(TEXT("Perception Sensor Component"));
	PerceptionSensor->SetupAttachment(RootComponent);
	PerceptionSensor->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PerceptionSensor->SetCollisionResponseToAllChannels(ECR_Ignore);
	PerceptionSensor->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PerceptionSensor->SetSphereRadius(3000.0f);

	//Set random initial velocity with random direction and speed.
	float InitialSpeed = FMath::RandRange(MinSpeed, MaxSpeed);
	FVector RandomDirection = FMath::VRand();
	Velocity = RandomDirection * InitialSpeed;

	// Set random initial rotation
	FRotator RandomRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
	SetActorRotation(RandomRotation);

	// Add a small initial random acceleration to break symmetry
	FVector InitialAcceleration = FMath::VRand() * FMath::RandRange(0.0f, 50.0f);
	Velocity += InitialAcceleration;

	//UE_LOG(LogTemp, Warning, TEXT("Initial Velocity: %s"), *Velocity.ToString());
}

// Called when the game starts or when spawned
void ABaseFish::BeginPlay()
{
	Super::BeginPlay();
}

FVector ABaseFish::Cohere(TArray<AActor*> School)
{
	FVector Steering = FVector::ZeroVector;
	int32 SchoolCount = 0.0f;
	FVector AveragePosition = FVector::ZeroVector;
	
	for (AActor* OverlapActor : School)
	{
		if (OverlapActor)
		{
			ABaseFish* Schoolmate = Cast<ABaseFish>(OverlapActor);

			if (!OverlapActor || OverlapActor == this || OverlapActor == Prey)
			{
				continue;
			}
			
			AveragePosition += Schoolmate->GetActorLocation();
			SchoolCount++;
		}
	}

	if (SchoolCount > 0)
	{
		AveragePosition /= SchoolCount;
		Steering = AveragePosition - this->GetActorLocation();
		Steering.GetSafeNormal() -= this->Velocity.GetSafeNormal();
		Steering *= CoherenceStrength;
		//UE_LOG(LogTemp, Warning, TEXT("Cohere Steering: %s"), *Steering.ToString());
		return Steering;
	}
	else
	{
		return FVector::ZeroVector;
	}
}

FVector ABaseFish::Separate(TArray<AActor*> AllFish)
{
	FVector Steering = FVector::ZeroVector;
	int32 SchoolCount = 0;
	FVector SeparationDirection = FVector::ZeroVector;
	float ProximityFactor = 0.0f;
	
	for (AActor* OverlapActor : AllFish)
	{
		if (OverlapActor)
		{
			ABaseFish* OtherFish = Cast<ABaseFish>(OverlapActor);

			if (!OtherFish || OtherFish == this || OtherFish == Prey)
			{
				continue;
			}


			SeparationDirection = this->GetActorLocation() - OtherFish->GetActorLocation();
			SeparationDirection = SeparationDirection.GetSafeNormal();
		
			ProximityFactor = 1.0f - (SeparationDirection.Size() / this->PerceptionSensor->GetScaledSphereRadius());

			if (ProximityFactor <0.0f)
			{
				continue;
			}

			Steering += (ProximityFactor * SeparationDirection);
			SchoolCount++;
		}
	}

	if (SchoolCount > 0)
	{
		Steering /= SchoolCount;
		Steering.GetSafeNormal() -= this->Velocity.GetSafeNormal();
		Steering *= SeparationStrength;
		//UE_LOG(LogTemp, Warning, TEXT("Separate Steering: %s"), *Steering.ToString());
		return Steering;
	}
	else
	{
		return FVector::ZeroVector;
	}
}

FVector ABaseFish::Align(TArray<AActor*> School)
{
	FVector Steering = FVector::ZeroVector;
	int32 SchoolCount = 0;
	
	for (AActor* OverlapActor : School)
	{
		if (OverlapActor)
		{
			ABaseFish* Schoolmate = Cast<ABaseFish>(OverlapActor);

			if (!OverlapActor || OverlapActor == this || OverlapActor == Prey)
			{
				continue;
			}
			
			Steering += Schoolmate->Velocity.GetSafeNormal();
			SchoolCount++;
		}
	}

	if (SchoolCount > 0)
	{
		Steering /= SchoolCount;
		Steering.GetSafeNormal() -= this->Velocity.GetSafeNormal();
		Steering *= AlignmentStrength;
		//UE_LOG(LogTemp, Warning, TEXT("Align Steering: %s"), *Steering.ToString());
		return Steering;
	}
	else
	{
		return FVector::ZeroVector;
	}
}

void ABaseFish::UpdateFishInRadius()
{
	if (PerceptionSensor)
	{
		FishInRadius.Empty();
		PerceptionSensor->GetOverlappingActors(FishInRadius, ABaseFish::StaticClass());
	}
}

void ABaseFish::UpdateFishTypes()
{
	if (IsValid(Prey))
	{
		bool bPreyFoundInRadius = FishInRadius.Contains(Prey);
		if (!bPreyFoundInRadius)
		{
			// Prey is no longer in radius
			Prey = nullptr;
			//UE_LOG(LogTemp, Log, TEXT("Prey is out of range or invalid. Resetting Prey."));
		}
	}
	else
	{
		// Prey is invalid
		Prey = nullptr;
	}

	// Similarly, check if the current Predator is still valid and within the perception radius
	if (IsValid(Predator))
	{
		bool bPredatorFoundInRadius = FishInRadius.Contains(Predator);
		if (!bPredatorFoundInRadius)
		{
			// Predator is no longer in radius
			Predator = nullptr;
			//UE_LOG(LogTemp, Log, TEXT("Predator is out of range or invalid. Resetting Predator."));
		}
	}
	else
	{
		// Predator is invalid
		Predator = nullptr;
	}
	
	FishOfSameType.Empty();
	
	for (AActor* OverlapActor : FishInRadius)
	{
		ABaseFish* Fish = Cast<ABaseFish>(OverlapActor);
		
		if (!IsValid(Fish) || Fish == this)
		{
			continue;
		}
		
		EFishType OtherFishType = Fish->GetFishType();

		if (Predator == nullptr && OtherFishType == PredatorType)
		{
			Predator = Fish;
		}
		else if (Prey == nullptr && (OtherFishType == PreyTypeA || OtherFishType == PreyTypeB || OtherFishType == PreyTypeC))
		{
			Prey = Fish;
			//UE_LOG(LogTemp, Log, TEXT("Prey detected: %s"), *Prey->GetName());
		}
		// Otherwise, add fish of the same type
		else if (OtherFishType == this->GetFishType())
		{
			FishOfSameType.Add(Fish);
		}
	}
}

void ABaseFish::UpdateMeshRotation()
{
	if (FishMesh)
	{
		CurrentRotation = FMath::RInterpTo(CurrentRotation, this->GetActorRotation(), GetWorld()->DeltaTimeSeconds, 7.0f);
		FishMesh->SetWorldRotation(CurrentRotation);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FishMesh is null in UpdateMeshRotation()"));
	}
}

void ABaseFish::Steer(float DeltaTime)
{
	FVector Acceleration = FVector::ZeroVector;

	// Update position and rotation
	this->SetActorLocation(this->GetActorLocation() + (Velocity * DeltaTime));
	this->SetActorRotation(Velocity.ToOrientationQuat());

	// Apply steering forces
	if (FishInRadius.Num() > 0)
	{
		Acceleration += Separate(FishInRadius);
		Acceleration += Align(FishOfSameType);
		Acceleration += Cohere(FishOfSameType);
	} else
	{
		Acceleration += Separate(FishInRadius);
		Acceleration += Align(FishInRadius);
		Acceleration += Cohere(FishInRadius);
	}

	if (IsObstacle())
	{
		Acceleration += AvoidObstacle();
	}

	for (FVector TargetForce : TargetForces)
	{
		Acceleration += TargetForce;
		TargetForces.Remove(TargetForce);
	}

	Velocity += (Acceleration * DeltaTime);
	Velocity = Velocity.GetClampedToSize(MinSpeed, MaxSpeed);

	// Smoothly cap movement in the Z-axis between -5000 and 5000
	CapMovementArea();
}

void ABaseFish::Hunt(float DeltaTime)
{
	FVector Acceleration = FVector::ZeroVector;

	if (Prey)
	{
		// Calculate direction towards the prey
		FVector DirectionToPrey = (Prey->GetActorLocation() - this->GetActorLocation()).GetSafeNormal();
		Acceleration += DirectionToPrey * 5000.0f;

		// Use the Separate() function to get the separation force
		FVector SeparationForce = Separate(FishInRadius);
		Acceleration += SeparationForce;

		// Update velocity
		Velocity += Acceleration * DeltaTime;
		Velocity = Velocity.GetClampedToSize(MinSpeed, MaxSpeed);

		// Check if close enough to the prey to "catch" it
		float DistanceToPrey = FVector::Dist(this->GetActorLocation(), Prey->GetActorLocation());
		float CatchDistance = 300.0f;

		if (DistanceToPrey <= CatchDistance)
		{
			// Destroy the prey
			Prey->OnDeath();
			Prey = nullptr;
			UE_LOG(LogTemp, Warning, TEXT("Prey Killed"));
		}
	}

	this->SetActorLocation(this->GetActorLocation() + (Velocity * DeltaTime));

	// Rotate towards the direction of movement
	if (Prey)
	{
		FRotator HuntRotation = UKismetMathLibrary::FindLookAtRotation(this->GetActorLocation(), this->GetActorLocation() + Velocity);
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), HuntRotation, DeltaTime, 5.0f));
	}
	CapMovementArea();
}

void ABaseFish::CapMovementArea()
{
	FVector CurrentLocation = this->GetActorLocation();
	
	float BoundaryLimit = 5000.0f;
	float BoundaryPushStrength = 5000.0f;
	FVector CorrectionAcceleration = FVector::ZeroVector;

	// x-axis
	if (CurrentLocation.X > BoundaryLimit)
	{
		CorrectionAcceleration.X = -BoundaryPushStrength;
	}
	else if (CurrentLocation.X < -BoundaryLimit)
	{
		CorrectionAcceleration.X = BoundaryPushStrength;
	}

	// y-axis
	if (CurrentLocation.Y > BoundaryLimit)
	{
		CorrectionAcceleration.Y = -BoundaryPushStrength;
	}
	else if (CurrentLocation.Y < -BoundaryLimit)
	{
		CorrectionAcceleration.Y = BoundaryPushStrength;
	}

	// z-axis
	if (CurrentLocation.Z > BoundaryLimit)
	{
		CorrectionAcceleration.Z = -BoundaryPushStrength;
	}
	else if (CurrentLocation.Z < -BoundaryLimit)
	{
		CorrectionAcceleration.Z = BoundaryPushStrength;
	}
	
	Velocity += CorrectionAcceleration * GetWorld()->DeltaTimeSeconds;
	Velocity = Velocity.GetClampedToSize(MinSpeed, MaxSpeed);
}


void ABaseFish::AvoidPredator(float DeltaTime)
{
	FVector Steering = FVector::ZeroVector;
	FVector Acceleration = FVector::ZeroVector;

	if (Predator)
	{
		FVector DirectionAwayFromPredator = (this->GetActorLocation() - Predator->GetActorLocation()).GetSafeNormal();
		Acceleration += DirectionAwayFromPredator * 1500.0f;
		
		FVector SeparationForce = FVector::ZeroVector;
		int32 NeighborCount = 0;

		for (AActor* Neighbor : FishInRadius)
		{
			ABaseFish* NeighborFish = Cast<ABaseFish>(Neighbor);
			if (NeighborFish && NeighborFish != this)
			{
				FVector DirectionAwayFromNeighbor = (this->GetActorLocation() - NeighborFish->GetActorLocation()).GetSafeNormal();
				float Distance = FVector::Dist(this->GetActorLocation(), NeighborFish->GetActorLocation());
				SeparationForce += DirectionAwayFromNeighbor / Distance;
				NeighborCount++;
			}
		}

		if (NeighborCount > 0)
		{
			SeparationForce /= NeighborCount;
			SeparationForce *= 1000.0f;
			Acceleration += SeparationForce;
		}

		Velocity += Acceleration * DeltaTime;
		Velocity = Velocity.GetClampedToSize(MinSpeed, MaxSpeed);
	}

	// Update the fish's location
	this->SetActorLocation(this->GetActorLocation() + (Velocity * DeltaTime));

	if (Predator)
	{
		FRotator AvoidRotation = UKismetMathLibrary::FindLookAtRotation(this->GetActorLocation(), this->GetActorLocation() + Velocity);
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), AvoidRotation, DeltaTime, 5.0f));
	}

	CapMovementArea();
}


bool ABaseFish::IsObstacle()
{
	// Mixed obstacle detection using multiple line traces in different directions.
	FHitResult Hit;
	FVector StartLocation = GetActorLocation();
	float TraceDistance = 1000.0f;
	FCollisionQueryParams TraceParams(FName(TEXT("ObstacleTrace")), true, this);

	TArray<FVector> Directions = {
		GetActorForwardVector(),
		(GetActorForwardVector() + GetActorRightVector() * 0.5f).GetSafeNormal(),
		(GetActorForwardVector() - GetActorRightVector() * 0.5f).GetSafeNormal()
	};

	for (const FVector& Direction : Directions)
	{
		FVector EndLocation = StartLocation + (Direction * TraceDistance);
		if (GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECC_Visibility, TraceParams))
		{
			if (Hit.GetActor() == nullptr)
			{
				UE_LOG(LogTemp, Error, TEXT("Hit.GetActor() is nullptr in IsObstacle()."));
				continue;
			}
			
			TArray<AActor*> OverlappingActors;
			GetOverlappingActors(OverlappingActors);
			for (AActor* OverlappingActor : OverlappingActors)
			{
				if (Hit.GetActor() == OverlappingActor)
				{
					//UE_LOG(LogTemp, Warning, TEXT("Fish is inside the obstacle: %s"), *Hit.GetActor()->GetName());
					return false;
				}
			}
			//UE_LOG(LogTemp, Warning, TEXT("Obstacle detected: %s"), *Hit.GetActor()->GetName());
			return true;
		}
	}

	return false;
}

FVector ABaseFish::AvoidObstacle()
{
	FVector AvoidanceDirection = FVector::CrossProduct(GetActorForwardVector(), FVector::UpVector).GetSafeNormal();
	float AvoidanceStrength = 300.0f;
	
	FVector Steering = AvoidanceDirection - GetVelocity().GetSafeNormal();
	Steering *= AvoidanceStrength;
	
	Velocity = (Velocity + Steering).GetClampedToSize(MinSpeed, MaxSpeed);

	//UE_LOG(LogTemp, Warning, TEXT("Avoiding obstacle with velocity: %s"), *Velocity.ToString());

	return Velocity;
}

void ABaseFish::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateFishInRadius();
	UpdateFishTypes();

	if (CurrentState == EFishState::Roam)
	{
		Steer(DeltaTime);
		if (Predator != nullptr)
		{
			CurrentState = EFishState::Evade;
		}
		else if (Prey != nullptr)
		{
			CurrentState = EFishState::Hunt;
			//UE_LOG(LogTemp, Log, TEXT("Entering Hunt Mode"));
		}
	}
	else if (CurrentState == EFishState::Evade)
	{
		AvoidPredator(DeltaTime);
		if (Predator == nullptr)
		{
			CurrentState = EFishState::Roam;
		}
	}
	else if (CurrentState == EFishState::Hunt)
	{
		Hunt(DeltaTime);
		if (Prey == nullptr)
		{
			CurrentState = EFishState::Roam;
		}
	}

	UpdateMeshRotation();
}

void ABaseFish::AddTargetForce(FVector TargetForce)
{
	TargetForces.Add(TargetForce);
}

EFishType ABaseFish::GetFishType() const
{
	return FishType;
}

float ABaseFish::GetMinSpeed()
{
	return MinSpeed;
}

void ABaseFish::OnDeath()
{
	if (DeathEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			DeathEffect,
			GetActorLocation(),
			GetActorRotation()
		);
	}

	Destroy();
}

	

