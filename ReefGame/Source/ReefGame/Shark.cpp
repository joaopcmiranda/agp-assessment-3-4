// Fill out your copyright notice in the Description page of Project Settings.


#include "Shark.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"

AShark::AShark()
{
	PerceptionSensor->SetSphereRadius(10000.0f);
}

void AShark::BeginPlay()
{
	Super::BeginPlay();
	
	MaxSpeed = 4000.0f;
	MinSpeed = 3500.0f;
	PreyType = EFishType::BaseFish;
	FishType = EFishType::Shark;
	PredatorType = EFishType::FishTypeB;

	UE_LOG(LogTemp, Log, TEXT("Shark Constructor: FishType = %s, PreyType = %s"), 
		*UEnum::GetValueAsString(FishType), 
		*UEnum::GetValueAsString(PreyType));
}

void AShark::Steer(float DeltaTime)
{
	FVector Acceleration = FVector::ZeroVector;

	// Update position and rotation
	this->SetActorLocation(this->GetActorLocation() + (Velocity * DeltaTime));
	this->SetActorRotation(Velocity.ToOrientationQuat());

	// Apply steering forces
	Acceleration += Separate(FishInRadius);
	Acceleration += Align(FishInRadius);
	Acceleration += Cohere(FishInRadius);

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
