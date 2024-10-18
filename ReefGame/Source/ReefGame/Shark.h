// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseFish.h"
#include "Shark.generated.h"

/**
 * 
 */
UCLASS()
class REEFGAME_API AShark : public ABaseFish
{
	GENERATED_BODY()

public:
	AShark();

protected:
	
	virtual void BeginPlay() override;

	virtual void Steer(float DeltaTime) override;
	
};
