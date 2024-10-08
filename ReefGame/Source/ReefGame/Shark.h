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
	virtual EFishType GetFishType() const override;

protected:

	virtual void Steer(float DeltaTime) override;
	virtual void BeginPlay() override;

	EFishType PreyType = EFishType::BaseFish;

private:
	UPROPERTY()
	ABaseFish* LockedPrey = nullptr;
	
};
