// Fill out your copyright notice in the Description page of Project Settings.


#include "ClownFish.h"

void AClownFish::BeginPlay()
{
	Super::BeginPlay();
	FishType = EFishType::ClownFish;
	PredatorType = EFishType::Tuna;
}
