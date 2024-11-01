// Fill out your copyright notice in the Description page of Project Settings.


#include "Grouper.h"

void AGrouper::BeginPlay()
{
	Super::BeginPlay();
	FishType = EFishType::Grouper;

	MinSpeed = 1000.0f;
	MaxSpeed = 1500.0f;
}
