// Fill out your copyright notice in the Description page of Project Settings.


#include "ParrotFish.h"

void AParrotFish::BeginPlay()
{
	Super::BeginPlay();
	FishType = EFishType::ParrotFish;
	PredatorType = EFishType::SailFish;
}
