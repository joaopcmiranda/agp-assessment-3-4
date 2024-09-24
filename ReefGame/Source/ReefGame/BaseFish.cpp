// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseFish.h"
#include "HealthComponent.h"
#include "PathfindingSubsystem.h"
#include "Perception/PawnSensingComponent.h"

// Sets default values
ABaseFish::ABaseFish()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>("Health Component");
	PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>("Pawn Sensing Component");
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
	if (PawnSensingComponent)
	{
		PawnSensingComponent->OnSeePawn.AddDynamic(this, &ABaseFish::OnSensedPawn);
	}
	
}

void ABaseFish::MoveAlongPath()
{
	if (CurrentPath.IsEmpty()) return;
	
	FVector MovementDirection = CurrentPath[CurrentPath.Num()-1] - GetActorLocation();
	MovementDirection.Normalize();
	AddMovementInput(MovementDirection);
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

void ABaseFish::OnSensedPawn(APawn* SensedActor)
{
	if (ABaseFish* OtherFish = Cast<ABaseFish>(SensedActor))
	{
		SensedFish = OtherFish;
		UE_LOG(LogTemp, Display, TEXT("Sensed Fish"))
	}
}

void ABaseFish::UpdateSight()
{
	if (!SensedFish) return;
	if (PawnSensingComponent)
	{
		if (!PawnSensingComponent->HasLineOfSightTo(SensedFish))
		{
			SensedFish = nullptr;
			UE_LOG(LogTemp, Display, TEXT("Lost Fish"))
		}
	}
}

// Called every frame
void ABaseFish::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateSight();
	
	/*switch(CurrentState)
	{
	case EFishState::Swim:
		TickSwim();
		if (SensedFish)
		{
			//do something
		}
		break;
	}*/
}

// Called to bind functionality to input
void ABaseFish::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
	

