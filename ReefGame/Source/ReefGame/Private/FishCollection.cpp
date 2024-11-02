// Fill out your copyright notice in the Description page of Project Settings.


#include "FishCollection.h"
#include "Net/UnrealNetwork.h"

AFishCollection::AFishCollection()
{
	bReplicates = true;
}

void AFishCollection::AddFishToCollection(EFishType FishType)
{
	if (!CollectedFish.Contains(FishType))
	{
		CollectedFish.Add(FishType);

		// Notify clients about the update
		OnRep_CollectedFish();
	}
}

void AFishCollection::OnRep_CollectedFish()
{
	// This function is called on clients when CollectedFish is replicated
	// You can add any client-side logic here if needed
}

bool AFishCollection::IsFishCollected(EFishType FishType) const
{
	return CollectedFish.Contains(FishType);
}

void AFishCollection::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFishCollection, CollectedFish);
}
