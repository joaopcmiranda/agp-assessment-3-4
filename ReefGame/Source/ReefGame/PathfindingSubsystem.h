// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PathfindingSubsystem.generated.h"

class ANavigationNode;
/**
 * 
 */
UCLASS()
class REEFGAME_API UPathfindingSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	TArray<FVector> GetRandomPath(const FVector& StartLocation);

	TArray<FVector> GetPath(const FVector& StartLocation, const FVector& TargetLocation);

	TArray<FVector> GetPathAway(const FVector& StartLocation, const FVector& TargetLocation);

	void PlaceProceduralNodes(const TArray<FVector>& LandscapeVertexData, int32 MapWidth, int32 MapHeight, int32 MapDepth);

	TArray<FVector> GetWaypointPositions();

protected:

	TArray<ANavigationNode*> Nodes;

	TArray<ANavigationNode*> ProcedurallyPlacedNodes;

private:

	void PopulateNodes();
	ANavigationNode* GetRandomNode();
	ANavigationNode* FindNearestNode(const FVector& TargetLocation);
	ANavigationNode* FindFurthestNode(const FVector& TargetLocation);
	TArray<FVector> GetPath(ANavigationNode* StartNode, ANavigationNode* EndNode);
	static TArray<FVector> ReconstructPath(const TMap<ANavigationNode*, ANavigationNode*>& CameFromMap, ANavigationNode* EndNode);
	
};
