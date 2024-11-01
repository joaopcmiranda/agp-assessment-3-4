// Fill out your copyright notice in the Description page of Project Settings.


#include "LionFish.h"

void ALionFish::BeginPlay()
{
	Super::BeginPlay();
	FishType = EFishType::LionFish;
	MaxSpeed = 1500.0f;
	MinSpeed = 1000.0f;
}
