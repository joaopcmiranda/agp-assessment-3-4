// Fill out your copyright notice in the Description page of Project Settings.


#include "SailFish.h"

void ASailFish::BeginPlay()
{
	Super::BeginPlay();
	FishType = EFishType::SailFish;
	PreyTypeA = EFishType::ParrotFish;
	PreyTypeB = EFishType::PurpleTang;
	PreyTypeC = EFishType::BlueTang;

	MinSpeed = 3000.0f;
	MaxSpeed = 3500.0f;
}
