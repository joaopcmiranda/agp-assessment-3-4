// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseFish.h"
#include "BlueTang.generated.h"

/**
 * 
 */
UCLASS()
class REEFGAME_API ABlueTang : public ABaseFish
{
	GENERATED_BODY()

protected:

	virtual void BeginPlay() override;	
	
};
