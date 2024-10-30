// Fill out your copyright notice in the Description page of Project Settings.


#include "AngelFish.h"

void AAngelFish::BeginPlay()
{
    Super::BeginPlay();
    FishType = EFishType::AngelFish;
    PredatorType = EFishType::GreatTrevally;
}
