// Fill out your copyright notice in the Description page of Project Settings.


#include "Tuna.h"

void ATuna::BeginPlay()
{
	Super::BeginPlay();
	FishType = EFishType::Tuna;
	PreyTypeA = EFishType::ClownFish;
	PreyTypeB = EFishType::MoorishIdol;
	PreyTypeC = EFishType::Barracuda;

	MinSpeed = 2500.0f;
	MaxSpeed = 3000.0f;
}
