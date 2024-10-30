// Fill out your copyright notice in the Description page of Project Settings.


#include "PufferFish.h"

void APufferFish::BeginPlay()
{
	Super::BeginPlay();
	FishType = EFishType::PufferFish;
	MinSpeed = 800.0f;
	MaxSpeed = 1000.0f;
	SeparationStrength = 2.0f;
}
