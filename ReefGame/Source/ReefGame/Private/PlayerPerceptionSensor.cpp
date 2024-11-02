#include "PlayerPerceptionSensor.h"
#include "ReefGame/BaseFish.h"
#include "GameFramework/Actor.h"

UPlayerPerceptionSensor::UPlayerPerceptionSensor()
{
    PrimaryComponentTick.bCanEverTick = true;

    // Enable overlap events
    SetGenerateOverlapEvents(true);

    // Bind overlap events
    OnComponentBeginOverlap.AddDynamic(this, &UPlayerPerceptionSensor::OnOverlapBegin);
    OnComponentEndOverlap.AddDynamic(this, &UPlayerPerceptionSensor::OnOverlapEnd);

    CurrentlyHighlightedFish = nullptr;
}

void UPlayerPerceptionSensor::BeginPlay()
{
    Super::BeginPlay();
}

void UPlayerPerceptionSensor::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    HighlightClosestFish();
}

void UPlayerPerceptionSensor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                             bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != GetOwner())
    {
        ABaseFish* Fish = Cast<ABaseFish>(OtherActor);
        if (Fish)
        {
            OverlappingFish.AddUnique(Fish);
        }
    }
}

void UPlayerPerceptionSensor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor != GetOwner())
    {
        ABaseFish* Fish = Cast<ABaseFish>(OtherActor);
        if (Fish)
        {
            OverlappingFish.Remove(Fish);

            // Unhighlight the fish if it was the currently highlighted one
            if (Fish == CurrentlyHighlightedFish)
            {
                Fish->Highlight(false);
                CurrentlyHighlightedFish = nullptr;

                // Notify that there is no highlighted fish
                OnHighlightedFishChanged.Broadcast(nullptr);
            }
        }
    }
}

void UPlayerPerceptionSensor::HighlightClosestFish()
{
    ABaseFish* ClosestFish = nullptr;
    float MinDistance = FLT_MAX;

    FVector SensorLocation = GetComponentLocation();
    FVector PlayerForwardVector = GetOwner()->GetActorForwardVector();

    // Convert FOV angle to radians for calculations
    float HalfFOVRadians = FMath::DegreesToRadians(FieldOfViewAngle / 2.0f);
    float CosHalfFOV = FMath::Cos(HalfFOVRadians);

    for (ABaseFish* Fish : OverlappingFish)
    {
        if (Fish && IsValid(Fish))
        {
            FVector DirectionToFish = (Fish->GetActorLocation() - SensorLocation).GetSafeNormal();

            // Check if the fish is within the field of view angle
            float DotProduct = FVector::DotProduct(PlayerForwardVector, DirectionToFish);

            if (DotProduct >= CosHalfFOV)
            {
                float Distance = FVector::Dist(SensorLocation, Fish->GetActorLocation());
                if (Distance < MinDistance)
                {
                    MinDistance = Distance;
                    ClosestFish = Fish;
                }
            }
        }
    }

    // If the closest fish has changed
    if (ClosestFish != CurrentlyHighlightedFish)
    {
        // Unhighlight the previous fish
        if (CurrentlyHighlightedFish)
        {
            CurrentlyHighlightedFish->Highlight(false);
        }

        // Highlight the new fish
        if (ClosestFish)
        {
            ClosestFish->Highlight(true);
        }

        CurrentlyHighlightedFish = ClosestFish;

        // Broadcast the event
        OnHighlightedFishChanged.Broadcast(CurrentlyHighlightedFish);
    }
}
