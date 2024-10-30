// Fill out your copyright notice in the Description page of Project Settings.


#include "GreatTrevally.h"

void AGreatTrevally::BeginPlay()
{
	Super::BeginPlay();
	FishType = EFishType::GreatTrevally;
	PreyTypeA = EFishType::AngelFish;
	PreyTypeB = EFishType::ClownTriggerFish;

	MinSpeed = 2500.0f;
	MaxSpeed = 3000.0f;
}
