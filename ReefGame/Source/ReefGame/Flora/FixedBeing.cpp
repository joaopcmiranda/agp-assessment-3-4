// Fill out your copyright notice in the Description page of Project Settings.


#include "FixedBeing.h"

#include "Selection.h"

// Sets default values
AFixedBeing::AFixedBeing()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    #if WITH_EDITOR
	// tick in editor
	PrimaryActorTick.bStartWithTickEnabled = true;


	USelection::SelectObjectEvent.AddUObject(this, &AFixedBeing::OnObjectSelected);
	#endif
}

// Called when the game starts or when spawned
void AFixedBeing::BeginPlay()
{
	Super::BeginPlay();

}

bool AFixedBeing::ShouldTickIfViewportsOnly() const
{
	return bIsSelected;
}

// Called every frame
void AFixedBeing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(bIsSelected)
	{
		// draw debug sprere
		DrawDebugSphere(GetWorld(), GetActorLocation(), ClusterRadius, 100, FColor::FromHex( "AAAAAAAA" ), false, -1, 0, 1);
	}

}
void AFixedBeing::OnObjectSelected(UObject* Object)
{
	if(Object == this)
	{
		bIsSelected = true;
	}
	else if(bIsSelected && !IsSelected())
	{
		bIsSelected = false;
	}
}
