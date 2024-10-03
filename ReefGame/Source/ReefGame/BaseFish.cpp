// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseFish.h"
#include "HealthComponent.h"
#include "PathfindingSubsystem.h"
#include "Perception/PawnSensingComponent.h"
#include "FishSchoolController.h"
#include "EngineUtils.h"

// Sets default values
ABaseFish::ABaseFish()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>("Health Component");
	//PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>("Pawn Sensing Component");
}

// Called when the game starts or when spawned
void ABaseFish::BeginPlay()
{
	Super::BeginPlay();

	PathfindingSubsystem = GetWorld()->GetSubsystem<UPathfindingSubsystem>();
	if (PathfindingSubsystem)
	{
		CurrentPath = PathfindingSubsystem->GetRandomPath(GetActorLocation());
	} else
	{
		UE_LOG(LogTemp, Error, TEXT("Unable to find the PathfindingSubsystem"))
	}

	for (TActorIterator<AFishSchoolController> It(GetWorld()); It; ++It)
	{
		FishSchoolController = *It;
		break;
	}
	
}

void ABaseFish::MoveAlongPath()
{
	if (CurrentPath.IsEmpty()) return;
	
	FVector MovementDirection = CurrentPath[CurrentPath.Num()-1] - GetActorLocation();
	MovementDirection.Normalize();
	AddMovementInput(MovementDirection);

	SmoothMovement(CurrentPath[CurrentPath.Num() - 1], GetWorld()->GetDeltaSeconds());
	
	if (FVector::Distance(GetActorLocation(), CurrentPath[CurrentPath.Num() - 1]) < PathfindingError)
	{
		CurrentPath.Pop();
	}
}

void ABaseFish::TickSwim()
{
	if (CurrentPath.IsEmpty())
	{
		CurrentPath = PathfindingSubsystem->GetRandomPath(GetActorLocation());
	}
	MoveAlongPath();
}

void ABaseFish::CheckForNearbyFish()
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (!FishSchoolController) return;

	float CurrentTime = World->GetTimeSeconds();
    
	// Check if the cooldown period has passed
	if (CurrentTime - LastSensedTime < SensingCooldown)
	{
		return;
	}

	// Access Fish array from the FishSchoolController
	for (ABaseFish* OtherFish : FishSchoolController->Fish)
	{
		if (OtherFish == this) continue;

		float DistanceToFish = FVector::Dist(GetActorLocation(), OtherFish->GetActorLocation());

		if (DistanceToFish <= SensingRadius)
		{
			UE_LOG(LogTemp, Display, TEXT("Detected nearby fish: %s"), *OtherFish->GetName());

			SensedFish = OtherFish;
			CurrentState = EFishState::SwimGroup;

			// Update the time this fish last sensed another fish
			LastSensedTime = CurrentTime;

			// You could break here if you only want to detect one fish, or keep looking for more
			break;
		}
	}
}

void ABaseFish::AddFishToSchool()
{
	if (SensedFish->bIsInSchool)
	{
		ABaseSchool* School = SensedFish->SchoolFishIsIn;
		School->AddFishToSchool(this);
		
	}
	else //no school exists
	{
		ABaseSchool* School = FishSchoolController->CreateSchool(SensedFish->GetActorLocation());
		School->AddFishToSchool(this);
	}
}

// Called every frame
void ABaseFish::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckForNearbyFish();
	
	switch(CurrentState)
	{
	case EFishState::Swim:
		TickSwim();
		if (SensedFish)
		{
			CurrentPath.Empty();
			CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), SensedFish->GetActorLocation());
			MoveAlongPath();

			float DistanceToSensedFish = FVector::Distance(GetActorLocation(), SensedFish->GetActorLocation());
			float AttachmentDistanceThreshold = 100.0f;

			// If close enough, attach to the sensed fish and change state to SwimGroup
			if (DistanceToSensedFish < AttachmentDistanceThreshold)
			{
				AddFishToSchool();
				CurrentState = EFishState::SwimGroup;
			}
		}
		break;
	case EFishState::SwimGroup:
		
		break;
			
	}
}

// Called to bind functionality to input
void ABaseFish::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

// Smooth movement in 3D
void ABaseFish::SmoothMovement(FVector TargetLocation, float DeltaTime)
{
	float MaxSpeed = 300.0f;   // Max movement speed
	float MinSpeed = 50.0f;    // Minimum speed, ensuring continuous movement
	float DistanceThreshold = 200.0f;  // Distance within which fish will start decelerating
	
	FVector CurrentLocation = GetActorLocation();
	FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();  // Movement direction
	
	float DistanceToTarget = FVector::Distance(CurrentLocation, TargetLocation);
	float Speed = MaxSpeed;
	
	if (DistanceToTarget < DistanceThreshold)
	{
		Speed = FMath::Lerp(MinSpeed, MaxSpeed, DistanceToTarget / DistanceThreshold);
	}
	
	FVector MovementDelta = Direction * Speed * DeltaTime;
	SetActorLocation(CurrentLocation + MovementDelta);
}
	

