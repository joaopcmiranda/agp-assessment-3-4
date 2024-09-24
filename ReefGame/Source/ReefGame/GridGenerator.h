// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridGenerator.generated.h"

UCLASS()
class REEFGAME_API AGridGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGridGenerator();
	
	virtual bool ShouldTickIfViewportsOnly() const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void CreateGrid();

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	UPROPERTY()
	TArray<FVector> Vertices;
    
	UPROPERTY()
	TArray<FVector3d> UVCoords;

	UPROPERTY(EditAnywhere)
	int32 Width = 10;

	UPROPERTY(EditAnywhere)
	int32 Height = 10;

	UPROPERTY(EditAnywhere)
	int32 Depth = 5;

	UPROPERTY(EditAnywhere)
	float VertexSpacing = 1000.0f;

	void OnConstruction(const FTransform& Transform);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ClearLandscape();
	
};
