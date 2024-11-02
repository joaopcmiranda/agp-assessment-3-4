#include "ReefGameGameModeBase.h"
#include "FishCollection.h" // Include the correct GameState class

AReefGameGameModeBase::AReefGameGameModeBase()
{
	// Access GameStateClass within the class scope
	GameStateClass = AFishCollection::StaticClass();
}
