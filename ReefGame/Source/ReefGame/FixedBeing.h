// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FixedBeing.generated.h"

UCLASS()
class REEFGAME_API AFixedBeing : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFixedBeing();

	UPROPERTY(EditAnywhere, Category="Fauna/Flora")
	bool bIsSpawnable = true;
	UPROPERTY(EditAnywhere, Category="Fauna/Flora")
	float Frequency = 0.5f;
	UPROPERTY(EditAnywhere, Category="Fauna/Flora")
	float FlatAffinity = 0.5f;
	UPROPERTY(EditAnywhere, Category="Fauna/Flora")
	float VerticalAffinity = 0.5f;
	UPROPERTY(EditAnywhere, Category="Fauna/Flora")
	float DeepAffinity = 0.5f;
	UPROPERTY(EditAnywhere, Category="Fauna/Flora")
	float ShallowAffinity = 0.5f;
	UPROPERTY(EditAnywhere, Category="Fauna/Flora")
	float OthersClusterAversion = 0.5f;
	UPROPERTY(EditAnywhere, Category="Fauna/Flora")
	float OthersClusterAfinity = 0.5f;
	UPROPERTY(EditAnywhere, Category="Fauna/Flora")
	float SelfClusterAversion = 0.5f;
	UPROPERTY(EditAnywhere, Category="Fauna/Flora")
	float SelfClusterAfinity = 0.5f;
	UPROPERTY(EditAnywhere, Category="Fauna/Flora")
	float MinimumSpacing = 0.5f;
	UPROPERTY(EditAnywhere, Category="Fauna/Flora")
	bool bCanBeUpsideDown = true;
	UPROPERTY(EditAnywhere, Category="Fauna/Flora")
	bool bAlwaysPointUp = false;

	UPROPERTY(VisibleAnywhere, Category="Placement")
	int ItemNumber;
	UPROPERTY(VisibleAnywhere, Category="Placement")
	float SkewedDepthAffinity;
	UPROPERTY (VisibleAnywhere, Category="Placement")
	float SkewedFlatnessAffinity;
	UPROPERTY (VisibleAnywhere, Category="Placement")
	float SelfClusterPositiveScore;
	UPROPERTY (VisibleAnywhere, Category="Placement")
	float SelfClusterNegativeScore;
	UPROPERTY (VisibleAnywhere, Category="Placement")
	float OthersClusterPositiveScore;
	UPROPERTY (VisibleAnywhere, Category="Placement")
	float OthersClusterNegativeScore;
	UPROPERTY(VisibleAnywhere, Category="Placement")
	float PlacementPass;
	UPROPERTY(VisibleAnywhere, Category="Placement")
	int64 NumberOfBeingsWhenPlaced;
	UPROPERTY(VisibleAnywhere, Category="Placement")
	int64 NumberOfBeingsNearby;
	UPROPERTY(VisibleAnywhere, Category="Placement")
	int64 NumberOfOtherBeingsNearby;
	UPROPERTY(VisibleAnywhere, Category="Placement")
	int64 NumberOfSameBeingsNearby;
	UPROPERTY(VisibleAnywhere, Category="Placement")
	float ClusterRadius;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual bool ShouldTickIfViewportsOnly() const override;
	void         OnObjectSelected(UObject* Object);
	bool         bIsSelected = false;
#endif

};
