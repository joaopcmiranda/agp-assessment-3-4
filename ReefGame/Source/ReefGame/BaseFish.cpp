// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseFish.h"
#include "HealthComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ABaseFish::ABaseFish()
{
	PrimaryActorTick.bCanEverTick = true;
	HealthComponent = CreateDefaultSubobject<UHealthComponent>("Health Component");

	//setupcollision component and set as root
	FishCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Component"));
	RootComponent = FishCollision;
	
	FishCollision->SetCollisionObjectType(ECC_Pawn);
	FishCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FishCollision->SetCollisionResponseToAllChannels(ECR_Overlap);
	
	//setup mesh component & attach to root
	FishMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	FishMesh->SetupAttachment(RootComponent);
	FishMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FishMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

	//setup cohesion sensor component
	PerceptionSensor = CreateDefaultSubobject<USphereComponent>(TEXT("Perception Sensor Component"));
	PerceptionSensor->SetupAttachment(RootComponent);
	PerceptionSensor->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PerceptionSensor->SetCollisionResponseToAllChannels(ECR_Ignore);
	PerceptionSensor->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PerceptionSensor->SetSphereRadius(2000.0f);

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

	UE_LOG(LogTemp, Warning, TEXT("Initial Velocity: %s"), *Velocity.ToString());
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
		UE_LOG(LogTemp, Warning, TEXT("Cohere Steering: %s"), *Steering.ToString());
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

			if (!OtherFish)
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
		UE_LOG(LogTemp, Warning, TEXT("Separate Steering: %s"), *Steering.ToString());
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
			Steering += Schoolmate->Velocity.GetSafeNormal();
			SchoolCount++;
		}
	}

	if (SchoolCount > 0)
	{
		Steering /= SchoolCount;
		Steering.GetSafeNormal() -= this->Velocity.GetSafeNormal();
		Steering *= AlignmentStrength;
		UE_LOG(LogTemp, Warning, TEXT("Align Steering: %s"), *Steering.ToString());
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
	Predator = nullptr;
	FishOfSameType.Empty();
	
	for (AActor* OverlapActor : FishInRadius)
	{
		ABaseFish* Fish = Cast<ABaseFish>(OverlapActor);
		
		if (!IsValid(Fish) || Fish == this)
		{
			continue;
		}
		
		//Check for and update predator
		if (Fish->GetFishType() == EFishType::Shark)
		{
			Predator = Fish;
		}
		// Otherwise, add fish of the same type
		else if (Fish->GetFishType() == this->GetFishType())
		{
			FishOfSameType.Add(Fish);
		}
	}
}

void ABaseFish::UpdateMeshRotation()
{
	CurrentRotation = FMath::RInterpTo(CurrentRotation, this->GetActorRotation(), GetWorld()->DeltaTimeSeconds, 7.0f);
	this->FishMesh->SetWorldRotation(CurrentRotation);
}

void ABaseFish::Steer(float DeltaTime)
{
	FVector Acceleration = FVector::ZeroVector;

	// Update position and rotation
	this->SetActorLocation(this->GetActorLocation() + (Velocity * DeltaTime));
	this->SetActorRotation(Velocity.ToOrientationQuat());

	// Apply steering forces
	Acceleration += Separate(FishInRadius);
	Acceleration += Align(FishOfSameType);
	Acceleration += Cohere(FishOfSameType);

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

void ABaseFish::CapMovementArea()
{
	FVector CurrentLocation = this->GetActorLocation();

	//x
	if (CurrentLocation.X > 5000.0f)
	{
		Velocity.X = FMath::Clamp(Velocity.X, -MaxSpeed, 0.0f);
	}
	else if (CurrentLocation.X < -5000.0f)
	{
		Velocity.X = FMath::Clamp(Velocity.X, 0.0f, MaxSpeed);
	}

	//y
	if (CurrentLocation.Y > 5000.0f)
	{
		Velocity.Y = FMath::Clamp(Velocity.Y, -MaxSpeed, 0.0f);
	}
	else if (CurrentLocation.Y < -5000.0f)
	{
		Velocity.Y = FMath::Clamp(Velocity.Y, 0.0f, MaxSpeed);
	}

	//z
	if (CurrentLocation.Z > 5000.0f)
	{
		Velocity.Z = FMath::Clamp(Velocity.Z, -MaxSpeed, 0.0f);
	}
	else if (CurrentLocation.Z < -5000.0f)
	{
		Velocity.Z = FMath::Clamp(Velocity.Z, 0.0f, MaxSpeed);
	}
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
					UE_LOG(LogTemp, Warning, TEXT("Fish is inside the obstacle: %s"), *Hit.GetActor()->GetName());
					return false;
				}
			}
			UE_LOG(LogTemp, Warning, TEXT("Obstacle detected: %s"), *Hit.GetActor()->GetName());
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

	UE_LOG(LogTemp, Warning, TEXT("Avoiding obstacle with velocity: %s"), *Velocity.ToString());

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
	}
	else if (CurrentState == EFishState::Evade)
	{
		AvoidPredator(DeltaTime);
		if (Predator == nullptr)
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
	return EFishType::BaseFish;
}

	

