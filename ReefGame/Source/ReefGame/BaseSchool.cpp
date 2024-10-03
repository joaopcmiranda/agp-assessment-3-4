// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseFish.h"
#include "BaseSchool.h"
#include "PathfindingSubsystem.h"
#include "EngineUtils.h"

// Sets default values
ABaseSchool::ABaseSchool()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABaseSchool::BeginPlay()
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
	
}

void ABaseSchool::MoveAlongPath()
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

void ABaseSchool::TickSwim()
{
	if (CurrentPath.IsEmpty())
	{
		CurrentPath = PathfindingSubsystem->GetRandomPath(GetActorLocation());
	}
	MoveAlongPath();
}

void ABaseSchool::RemoveFishFromSchool(ABaseFish* Fish)
{
	if (Fish)
	{
		Fish->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		
		FishInSchool.Remove(Fish);
		
		UE_LOG(LogTemp, Log, TEXT("Fish removed and detached from BaseSchool"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Fish passed to RemoveFishFromSchool"));
	}
}

void ABaseSchool::AddFishToSchool(ABaseFish* Fish)
{
	if (Fish)
	{
		Fish->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
		
		FishInSchool.Add(Fish);
		
		UE_LOG(LogTemp, Log, TEXT("Fish added and attached to BaseSchool"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Fish passed to AddFishToSchool"));
	}
}

// Called every frame
void ABaseSchool::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABaseSchool::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABaseSchool::SmoothMovement(FVector TargetLocation, float DeltaTime)
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

