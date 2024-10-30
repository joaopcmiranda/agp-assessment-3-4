// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseFish.h"
#include "Tuna.generated.h"

/**
 * 
 */
UCLASS()
class REEFGAME_API ATuna : public ABaseFish
{
	GENERATED_BODY()

protected:

	virtual void BeginPlay() override;
	
};
