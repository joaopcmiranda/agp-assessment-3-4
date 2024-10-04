// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseFish.h"
#include "HealthComponent.h"
#include "FishSchoolController.h"
#include "EngineUtils.h"

// Sets default values
ABaseFish::ABaseFish()
{
	PrimaryActorTick.bCanEverTick = true;
	HealthComponent = CreateDefaultSubobject<UHealthComponent>("Health Component");
}

// Called when the game starts or when spawned
void ABaseFish::BeginPlay()
{
	Super::BeginPlay();

	for (TActorIterator<AFishSchoolController> It(GetWorld()); It; ++It)
	{
		FishSchoolController = *It;
		break;
	}
}

void ABaseFish::CheckForNearbyFish()
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (!FishSchoolController) return;

	float CurrentTime = World->GetTimeSeconds();
	
	FishInRadius.Empty();
	
	if (CurrentTime - LastSensedTime < SensingCooldown)
	{
		return;
	}
	
	for (ABaseFish* OtherFish : FishSchoolController->Fish)
	{
		if (OtherFish == this) continue;

		float DistanceToFish = FVector::Dist(GetActorLocation(), OtherFish->GetActorLocation());

		if (DistanceToFish <= SensingRadius)
		{
			UE_LOG(LogTemp, Display, TEXT("Detected nearby fish: %s"), *OtherFish->GetName());

			FishInRadius.Add(OtherFish);
			LastSensedTime = CurrentTime;
		}
	}
}

void ABaseFish::SmoothMovementInDirection(FVector Direction, float DeltaTime)
{
	float Speed = MaxSpeed;

	FVector CurrentLocation = GetActorLocation();
	FVector MovementDelta = Direction * Speed * DeltaTime;
	SetActorLocation(CurrentLocation + MovementDelta);
}

FVector ABaseFish::Cohere()
{
	FVector Steering;

	for (ABaseFish* OtherFish : FishInRadius)
	{
		Steering += OtherFish->GetActorLocation();
	}

	int NumFish = FishInRadius.Num();
	
	if (NumFish > 0)
	{
		Steering /= NumFish;
		Steering -= this->GetActorLocation();
		Steering = Steering.GetSafeNormal()*300.0f; //max speed
		Steering -= this->GetVelocity();
		Steering = Steering.GetClampedToMaxSize(MaxForce);
	}
	return Steering;
}

FVector ABaseFish::Separate()
{
	FVector Steering;

	for (ABaseFish* OtherFish : FishInRadius)
	{
		float Distance = FVector::Dist(this->GetActorLocation(), OtherFish->GetActorLocation());

		FVector Diff = GetActorLocation() - OtherFish->GetActorLocation();
		if (Distance > 0)
		{
			Diff /= (Distance * Distance);
		}
		Steering += Diff;
	}

	int NumFish = FishInRadius.Num();
	
	if (NumFish > 0)
	{
		Steering /= NumFish;
		Steering = Steering.GetSafeNormal()*MaxSpeed;
		Steering -= this->GetVelocity();
		Steering = Steering.GetClampedToMaxSize(MaxForce);
	}
	return Steering;
}

FVector ABaseFish::Align()
{
	FVector Steering;
	
	for (ABaseFish* OtherFish : FishInRadius)
	{
		Steering += OtherFish->GetVelocity();
	}

	int NumFish = FishInRadius.Num();
	
	if (NumFish > 0)
	{
		Steering /= NumFish;
		Steering = Steering.GetSafeNormal()*MaxSpeed;
		Steering -= this->GetVelocity();
		Steering = Steering.GetClampedToMaxSize(MaxForce);
	}

	return Steering;
}

FVector ABaseFish::FlockingSteering()
{
	FVector Alignment = Align();
	FVector Cohesion = Cohere();
	FVector Separation = Separate();

	Alignment *= 1.0f;
	Cohesion *= 1.0f;
	Separation *= 1.5f;

	FVector FlockingSteering = Alignment + Cohesion + Separation;

	return FlockingSteering;
}

void ABaseFish::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckForNearbyFish();

	if ((GetWorld()->GetTimeSeconds() - LastDirectionChangeTime) > DirectionChangeInterval)
	{
		RandomDirection = FMath::VRand();
		RandomDirection *= MaxSpeed;
		LastDirectionChangeTime = GetWorld()->GetTimeSeconds();
	}

	FVector FinalDirection = (RandomDirection + FlockingSteering()).GetSafeNormal();
	SmoothMovementInDirection(FinalDirection, DeltaTime);
}

	

