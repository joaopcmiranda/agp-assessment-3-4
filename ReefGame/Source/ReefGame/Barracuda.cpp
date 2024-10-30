// Fill out your copyright notice in the Description page of Project Settings.


#include "Barracuda.h"

void ABarracuda::BeginPlay()
{
	Super::BeginPlay();
	FishType = EFishType::Barracuda;
	MinSpeed = 2500.0f;
	MaxSpeed = 3500.0f;
}
