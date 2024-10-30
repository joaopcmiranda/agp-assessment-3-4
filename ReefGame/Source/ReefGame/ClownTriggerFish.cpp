// Fill out your copyright notice in the Description page of Project Settings.


#include "ClownTriggerFish.h"

void AClownTriggerFish::BeginPlay()
{
	Super::BeginPlay();
	FishType = EFishType::ClownTriggerFish;
	PredatorType = EFishType::GreatTrevally;
}
