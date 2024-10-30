// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueTang.h"

void ABlueTang::BeginPlay()
{
	Super::BeginPlay();
	FishType = EFishType::BlueTang;
	PredatorType = EFishType::SailFish;
}
