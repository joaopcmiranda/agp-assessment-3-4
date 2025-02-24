﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "Terrain.generated.h"

UCLASS()
class REEFGAME_API ATerrain : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATerrain();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	UProceduralMeshComponent* ProceduralMesh;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
