// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerGameMode.h"

#include "FishCollection.h"

AMultiplayerGameMode::AMultiplayerGameMode()
{
	// Access GameStateClass within the class scope
	GameStateClass = AFishCollection::StaticClass();
}
