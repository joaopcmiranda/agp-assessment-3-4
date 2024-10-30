// Fill out your copyright notice in the Description page of Project Settings.


#include "MoorishIdol.h"

void AMoorishIdol::BeginPlay()
{
	Super::BeginPlay();
	FishType = EFishType::MoorishIdol;
	PredatorType = EFishType::Tuna;
}
