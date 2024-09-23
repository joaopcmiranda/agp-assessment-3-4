// Fill out your copyright notice in the Description page of Project Settings.


#include "PathfindingSubsystem.h"
#include "EngineUtils.h"
#include "NavigationNode.h"

void UPathfindingSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	PopulateNodes();
}

TArray<FVector> UPathfindingSubsystem::GetRandomPath(const FVector& StartLocation)
{
	return GetPath(FindNearestNode(StartLocation), GetRandomNode());
}

TArray<FVector> UPathfindingSubsystem::GetPath(const FVector& StartLocation, const FVector& TargetLocation)
{
	return GetPath(FindNearestNode(StartLocation), FindNearestNode(TargetLocation));
}

TArray<FVector> UPathfindingSubsystem::GetPathAway(const FVector& StartLocation, const FVector& TargetLocation)
{
	return GetPath(FindNearestNode(StartLocation), FindFurthestNode(TargetLocation));
}

void UPathfindingSubsystem::PlaceProceduralNodes(const TArray<FVector>& LandscapeVertexData, int32 MapWidth,
	int32 MapHeight, int32 MapDepth)
{

	for (int Z =0; Z < MapDepth; Z++)
	{
		for (int Y = 0; Y < MapHeight; Y++)
		{
			for (int X = 0; X < MapWidth; X++)
			{
				if (ANavigationNode* Node = GetWorld()->SpawnActor<ANavigationNode>())
				{
					int32 Index = Z * MapWidth * MapHeight + Y * MapWidth + X;
					Node->SetActorLocation(LandscapeVertexData[Index]);
					ProcedurallyPlacedNodes.Add(Node);
				}
			}
		}
	}
	
	for (int Z =0; Z < MapDepth; Z++)
	{
		for (int Y = 0; Y < MapHeight; Y++)
		{
			for (int X = 0; X < MapWidth; X++)
			{
				int32 Index = Z * MapWidth * MapHeight + Y * MapWidth + X;
				if (ANavigationNode* CurrentNode = ProcedurallyPlacedNodes[Index])
				{
					// X axis neighbors
					if (X != MapWidth - 1) // Right
						CurrentNode->ConnectedNodes.Add(ProcedurallyPlacedNodes[Index + 1]);
					if (X != 0) // Left
						CurrentNode->ConnectedNodes.Add(ProcedurallyPlacedNodes[Index - 1]);

					// Y axis neighbors
					if (Y != MapHeight - 1) // Up
						CurrentNode->ConnectedNodes.Add(ProcedurallyPlacedNodes[Index + MapWidth]);
					if (Y != 0) // Down
						CurrentNode->ConnectedNodes.Add(ProcedurallyPlacedNodes[Index - MapWidth]);

					// Z axis neighbors
					if (Z != MapDepth - 1) // Forward (Up in Z-axis)
						CurrentNode->ConnectedNodes.Add(ProcedurallyPlacedNodes[Index + MapWidth * MapHeight]);
					if (Z != 0) // Backward (Down in Z-axis)
						CurrentNode->ConnectedNodes.Add(ProcedurallyPlacedNodes[Index - MapWidth * MapHeight]);

					// Diagonal connections
					
					// 2D Diagonal Connections (X, Y)
                    if (X != MapWidth - 1 && Y != MapHeight - 1) // Up-Right
                        CurrentNode->ConnectedNodes.Add(ProcedurallyPlacedNodes[Index + MapWidth + 1]);
                    if (X != 0 && Y != MapHeight - 1) // Up-Left
                        CurrentNode->ConnectedNodes.Add(ProcedurallyPlacedNodes[Index + MapWidth - 1]);
                    if (X != MapWidth - 1 && Y != 0) // Down-Right
                        CurrentNode->ConnectedNodes.Add(ProcedurallyPlacedNodes[Index - MapWidth + 1]);
                    if (X != 0 && Y != 0) // Down-Left
                        CurrentNode->ConnectedNodes.Add(ProcedurallyPlacedNodes[Index - MapWidth - 1]);
					
				}
			}
		}
	}
}

TArray<FVector> UPathfindingSubsystem::GetWaypointPositions()
{
	TArray<FVector> NodeCoordinates;

	for (int i = 0; i < Nodes.Num(); i++)
	{
		NodeCoordinates.Add(Nodes[i]->GetVectorLocation());
	}

	return NodeCoordinates;
}

void UPathfindingSubsystem::PopulateNodes()
{
	Nodes.Empty();

	for (TActorIterator<ANavigationNode> It(GetWorld()); It; ++It)
	{
		Nodes.Add(*It);
		UE_LOG(LogTemp, Warning, TEXT("NODE: %s"), *(*It)->GetActorLocation().ToString())
	}
}

ANavigationNode* UPathfindingSubsystem::GetRandomNode()
{
	// Failure condition
	if (Nodes.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("The nodes array is empty."))
		return nullptr;
	}
	const int32 RandIndex = FMath::RandRange(0, Nodes.Num()-1);
	return Nodes[RandIndex];
}

ANavigationNode* UPathfindingSubsystem::FindNearestNode(const FVector& TargetLocation)
{
	// Failure condition.
	if (Nodes.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("The nodes array is empty."))
		return nullptr;
	}
	
	ANavigationNode* ClosestNode = nullptr;
	float MinDistance = UE_MAX_FLT;
	for (ANavigationNode* Node : Nodes)
	{
		const float Distance = FVector::Distance(TargetLocation, Node->GetActorLocation());
		if (Distance < MinDistance)
		{
			MinDistance = Distance;
			ClosestNode = Node;
		}
	}

	return ClosestNode;
}

ANavigationNode* UPathfindingSubsystem::FindFurthestNode(const FVector& TargetLocation)
{
	// Failure condition.
	if (Nodes.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("The nodes array is empty."))
		return nullptr;
	}
	
	ANavigationNode* FurthestNode = nullptr;
	float MaxDistance = -1.0f;
	for (ANavigationNode* Node : Nodes)
	{
		const float Distance = FVector::Distance(TargetLocation, Node->GetActorLocation());
		if (Distance > MaxDistance)
		{
			MaxDistance = Distance;
			FurthestNode = Node;
		}
	}

	return FurthestNode;
}

TArray<FVector> UPathfindingSubsystem::GetPath(ANavigationNode* StartNode, ANavigationNode* EndNode)
{
	if (!StartNode || !EndNode)
	{
		UE_LOG(LogTemp, Error, TEXT("Either the start or end node are nullptrs."))
		return TArray<FVector>();
	}
	
	TArray<ANavigationNode*> OpenSet;
	OpenSet.Add(StartNode);
	
	TMap<ANavigationNode*, float> GScores, HScores;
	TMap<ANavigationNode*, ANavigationNode*> CameFrom;
	
	GScores.Add(StartNode, 0);
	HScores.Add(StartNode, FVector::Distance(StartNode->GetActorLocation(), EndNode->GetActorLocation()));
	CameFrom.Add(StartNode, nullptr);

	while (!OpenSet.IsEmpty())
	{
		ANavigationNode* CurrentNode = OpenSet[0];
		for (int32 i = 1; i < OpenSet.Num(); i++)
		{
			if (GScores[OpenSet[i]] + HScores[OpenSet[i]] < GScores[CurrentNode] + HScores[CurrentNode])
			{
				CurrentNode = OpenSet[i];
			}
		}
		
		OpenSet.Remove(CurrentNode);

		if (CurrentNode == EndNode)
		{
			return ReconstructPath(CameFrom, EndNode);
		}

		for (ANavigationNode* ConnectedNode : CurrentNode->ConnectedNodes)
		{
			if (!ConnectedNode) continue;
			const float TentativeGScore = GScores[CurrentNode] + FVector::Distance(CurrentNode->GetActorLocation(), ConnectedNode->GetActorLocation());
			
			if (!GScores.Contains(ConnectedNode))
			{
				GScores.Add(ConnectedNode, UE_MAX_FLT);
				HScores.Add(ConnectedNode, FVector::Distance(ConnectedNode->GetActorLocation(), EndNode->GetActorLocation()));
				CameFrom.Add(ConnectedNode, nullptr);
			}
			
			if (TentativeGScore < GScores[ConnectedNode])
			{
				CameFrom[ConnectedNode] = CurrentNode;
				GScores[ConnectedNode] = TentativeGScore;
				if (!OpenSet.Contains(ConnectedNode))
				{
					OpenSet.Add(ConnectedNode);
				}
			}
		}
	}
	
	return TArray<FVector>();
	
}

TArray<FVector> UPathfindingSubsystem::ReconstructPath(const TMap<ANavigationNode*, ANavigationNode*>& CameFromMap, ANavigationNode* EndNode)
{
	TArray<FVector> NodeLocations;

	const ANavigationNode* NextNode = EndNode;
	while(NextNode)
	{
		NodeLocations.Push(NextNode->GetActorLocation());
		NextNode = CameFromMap[NextNode];
	}

	return NodeLocations;
}

