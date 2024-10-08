// Fill out your copyright notice in the Description page of Project Settings.


#include "Shark.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"

AShark::AShark()
{
	MaxSpeed = 2500.0f;
    PerceptionSensor->SetSphereRadius(5000.0f);
}

void AShark::BeginPlay()
{
	Super::BeginPlay();
}

EFishType AShark::GetFishType() const
{
	return EFishType::Shark;
}

void AShark::Steer(float DeltaTime)
{
    FVector Acceleration = FVector::ZeroVector;

    if (LockedPrey == nullptr || !LockedPrey->IsValidLowLevel() || IsValid(LockedPrey))
    {
        float MinDistance = TNumericLimits<float>::Max();

        for (AActor* DetectedActor : FishInRadius)
        {
            ABaseFish* PotentialPrey = Cast<ABaseFish>(DetectedActor);
            if (PotentialPrey != nullptr && PotentialPrey != this && PotentialPrey->GetFishType() == PreyType)
            {
                float Distance = FVector::Dist(this->GetActorLocation(), PotentialPrey->GetActorLocation());
                if (Distance < MinDistance)
                {
                    MinDistance = Distance;
                    LockedPrey = PotentialPrey;
                }
            }
        }
    }
    
    if (LockedPrey)
    {
        FVector DirectionToPrey = (LockedPrey->GetActorLocation() - this->GetActorLocation()).GetSafeNormal();
        Acceleration = DirectionToPrey * 1500.0f;
        Velocity += Acceleration * DeltaTime;
        Velocity = Velocity.GetClampedToSize(MinSpeed, MaxSpeed);
    }
    
    this->SetActorLocation(this->GetActorLocation() + (Velocity * DeltaTime));

    if (LockedPrey)
    {
        FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(this->GetActorLocation(), LockedPrey->GetActorLocation());
        SetActorRotation(FMath::RInterpTo(GetActorRotation(), LookAtRotation, DeltaTime, 5.0f));
    }
    else
    {
        this->SetActorRotation(Velocity.ToOrientationQuat());
    }

    FVector CurrentLocation = this->GetActorLocation();
    if (CurrentLocation.Z > 5000.0f)
    {
        Velocity.Z = FMath::Clamp(Velocity.Z, -MaxSpeed, 0.0f);
    }
    else if (CurrentLocation.Z < -5000.0f)
    {
        Velocity.Z = FMath::Clamp(Velocity.Z, 0.0f, MaxSpeed);
    }
}

