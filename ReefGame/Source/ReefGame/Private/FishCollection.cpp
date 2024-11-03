// Fill out your copyright notice in the Description page of Project Settings.


#include "FishCollection.h"

#include "MyProjectCharacter.h"
#include "Blueprint/UserWidget.h"
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
		CurrentCollectedFish = FishType;
		// Notify clients about the update
		OnRep_CollectedFish();
	}
}

void AFishCollection::OnRep_CollectedFish()
{
	// Find all player controllers and refresh their collection menu if open
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PlayerController = It->Get())
		{
			if (AMyProjectCharacter* Character = Cast<AMyProjectCharacter>(PlayerController->GetPawn()))
			{
				if (Character->CollectionMenuInstance && Character->CollectionMenuInstance->IsInViewport())
				{
					// Call RefreshCollectionUI event in the Blueprint widget
					FName FunctionName = FName("RefreshMenu");
					if (Character->CollectionMenuInstance->FindFunction(FunctionName))
					{
						Character->CollectionMenuInstance->ProcessEvent(Character->CollectionMenuInstance->FindFunction(FunctionName), nullptr);
					}
				}
				Character->ShowCollectionNotification(CurrentCollectedFish);
			}
		}
	}
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
