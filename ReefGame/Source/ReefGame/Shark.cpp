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

	UE_LOG(LogTemp, Log, TEXT("Shark Constructor: FishType = %s, PreyType = %s"), 
		*UEnum::GetValueAsString(FishType), 
		*UEnum::GetValueAsString(PreyTypeA));
}
