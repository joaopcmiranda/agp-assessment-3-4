// Fill out your copyright notice in the Description page of Project Settings.


#include "Terrain.h"

// Sets default values
ATerrain::ATerrain()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Procedural Mesh"));
	SetRootComponent(ProceduralMesh);
}

// Called when the game starts or when spawned
void ATerrain::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("ATerrain::BeginPlay"));
}



// Called every frame
void ATerrain::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

}
