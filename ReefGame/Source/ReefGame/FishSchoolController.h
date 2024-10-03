// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseSchool.h"
#include "BaseFish.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FishSchoolController.generated.h"

UCLASS()
class REEFGAME_API AFishSchoolController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFishSchoolController();

	UPROPERTY();
	TArray<ABaseFish*> Fish;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
	TArray<ABaseSchool*> Schools;

	void PopulateFish();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	ABaseSchool* CreateSchool(FVector SpawnLocation);

};
