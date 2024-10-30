// Fill out your copyright notice in the Description page of Project Settings.


#include "PurpleTang.h"

void APurpleTang::BeginPlay()
{
	Super::BeginPlay();
	FishType = EFishType::PurpleTang;
	PredatorType = EFishType::SailFish;
}
