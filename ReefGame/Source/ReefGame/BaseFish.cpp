// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseFish.h"
#include "HealthComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"

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
		ABaseFish* Schoolmate = Cast<ABaseFish>(OverlapActor);
		if (Schoolmate != nullptr && Schoolmate != this)
		{
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

FVector ABaseFish::Separate(TArray<AActor*> School)
{
	FVector Steering = FVector::ZeroVector;
	int32 SchoolCount = 0;
	FVector SeparationDirection = FVector::ZeroVector;
	float ProximityFactor = 0.0f;
	
	for (AActor* OverlapActor : School)
	{
		ABaseFish* Schoolmate = Cast<ABaseFish>(OverlapActor);
		if (Schoolmate != nullptr && Schoolmate != this)
		{
			//getting direction away from the nearest fish
			SeparationDirection = this->GetActorLocation() - Schoolmate->GetActorLocation();
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
	UE_LOG(LogTemp, Warning, TEXT("SchoolCount: %d"), SchoolCount);

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
		ABaseFish* Schoolmate = Cast<ABaseFish>(OverlapActor);
		if (Schoolmate != nullptr && Schoolmate != this)
		{
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

void ABaseFish::UpdateMeshRotation()
{
	CurrentRotation = FMath::RInterpTo(CurrentRotation, this->GetActorRotation(), GetWorld()->DeltaTimeSeconds, 7.0f);
	this->FishMesh->SetWorldRotation(CurrentRotation);
}

void ABaseFish::Steer(float DeltaTime)
{
	FVector Acceleration = FVector::ZeroVector;
	
	//Update position and rotation
	this->SetActorLocation(this->GetActorLocation() + (Velocity * DeltaTime));
	this->SetActorRotation(Velocity.ToOrientationQuat());
	
	// Apply steering forces
	TArray<AActor*> Schoolmates;
	PerceptionSensor->GetOverlappingActors(Schoolmates, TSubclassOf<ABaseFish>());
	Acceleration += Separate(Schoolmates);
	Acceleration += Align(Schoolmates);
	Acceleration += Cohere(Schoolmates);

	/*if (IsObstacle())
	{
		Acceleration += AvoidObstacle();
	}*/

	for (FVector TargetForce : TargetForces)
	{
		Acceleration += TargetForce;
		TargetForces.Remove(TargetForce);
	}

	// Draw debug lines to visualize the steering forces
	//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + (Separate(Schoolmates) * 100), FColor::Red, false, -1.0f, 0, 1.5f);
	//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + (Align(Schoolmates) * 100), FColor::Green, false, -1.0f, 0, 1.5f);
	//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + (Cohere(Schoolmates) * 100), FColor::Blue, false, -1.0f, 0, 1.5f);

	Velocity += (Acceleration * DeltaTime);
	Velocity = Velocity.GetClampedToSize(MinSpeed, MaxSpeed);

	//UE_LOG(LogTemp, Warning, TEXT("Acceleration: %s"), *Acceleration.ToString());
	//UE_LOG(LogTemp, Warning, TEXT("Velocity: %s"), *Velocity.ToString());
	
}

bool ABaseFish::IsObstacle()
{
	// Mixed obstacle detection using multiple line traces in different directions.
	FHitResult Hit;
	FVector StartLocation = GetActorLocation();
	float TraceDistance = 500.0f; // Trace distance of 500 units
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
			// Check if the fish is already inside the detected object
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

	Steer(DeltaTime);

	UpdateMeshRotation();
}

void ABaseFish::AddTargetForce(FVector TargetForce)
{
	TargetForces.Add(TargetForce);
}

	

