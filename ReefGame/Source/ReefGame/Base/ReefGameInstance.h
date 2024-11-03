// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ReefGameInstance.generated.h"

class Terrain;
class AFixedBeing;
/**
 *
 */
UCLASS()
class REEFGAME_API UReefGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TArray<AFixedBeing*> FixedBeings;
};
