#pragma once

#include "CoreMinimal.h"
#include "FishCollection.h" // Include the correct header
#include "GameFramework/GameMode.h"
#include "GameFramework/GameModeBase.h"
#include "ReefGameGameModeBase.generated.h"

UCLASS()
class REEFGAME_API AReefGameGameModeBase : public AGameMode
{
	GENERATED_BODY()

public:
	// Declare the constructor
	AReefGameGameModeBase();
};
