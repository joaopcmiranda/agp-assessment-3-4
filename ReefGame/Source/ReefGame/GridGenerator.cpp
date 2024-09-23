// Fill out your copyright notice in the Description page of Project Settings.


#include "GridGenerator.h"
#include "Kismet/KismetSystemLibrary.h"
//#include "KismetProceduralMeshLibrary.h"
#include "PathfindingSubsystem.h"
#include "NavigationSystemTypes.h"
#include "VectorTypes.h"

// Sets default values
AGridGenerator::AGridGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

bool AGridGenerator::ShouldTickIfViewportsOnly() const
{
	return true;
}

// Called when the game starts or when spawned
void AGridGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGridGenerator::CreateGrid()
{
	UWorld* World = GetWorld();
	
	for (int32 Z = 0; Z < Depth; Z++)
	{
		for (int32 Y = 0; Y < Height; Y++)
		{
			for (int32 X = 0; X < Width; X++)
			{
				FVector VertexLocation = FVector(X * VertexSpacing, Y * VertexSpacing, Z * VertexSpacing);
			
				Vertices.Add(VertexLocation);
				UVCoords.Add(FVector3d(X, Y, Z));

				if (World)
				{
					DrawDebugSphere(World, VertexLocation, 50.0f, 8, FColor::Blue, true, -1, 0, 10.0f);
				}
			}
		}
	}
	
	if (UPathfindingSubsystem* PathfindingSubsystem = GetWorld()->GetSubsystem<UPathfindingSubsystem>())
	{
		PathfindingSubsystem->PlaceProceduralNodes(Vertices, Width, Height, Depth);
	}
	
}

// Called every frame
void AGridGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGridGenerator::ClearLandscape()
{
	Vertices.Empty();
	UVCoords.Empty();
	UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
}
