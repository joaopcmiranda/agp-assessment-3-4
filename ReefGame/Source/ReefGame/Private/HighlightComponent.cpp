#include "HighlightComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"

// In HighlightComponent.cpp

UHighlightComponent::UHighlightComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    bHighlightEnabled = false;
    bPendingHighlightUpdate = false;
}

#if WITH_EDITOR
void UHighlightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bPendingHighlightUpdate)
    {
        if (bHighlightEnabled)
        {
            Highlight();
        }
        else
        {
            RemoveHighlight();
        }

        bPendingHighlightUpdate = false;
    }
}

void UHighlightComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    FName PropertyName = (PropertyChangedEvent.Property != nullptr)
                             ? PropertyChangedEvent.Property->GetFName()
                             : NAME_None;

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UHighlightComponent, OverlayMaterial) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(UHighlightComponent, MeshComponent) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(UHighlightComponent, bHighlightEnabled))
    {
        // Set a flag to update the highlight later
        bPendingHighlightUpdate = true;
    }
}
#endif

void UHighlightComponent::BeginPlay()
{
    Super::BeginPlay();

#if !WITH_EDITOR
    SetComponentTickEnabled(false);
#endif

    if (!MeshComponent)
    {
        // Find the collision component
        UPrimitiveComponent* CollisionComponent = GetOwner()->FindComponentByClass<UPrimitiveComponent>();

        if (CollisionComponent)
        {
            // Find the child skeletal mesh component under the collision component
            TArray<USceneComponent*> ChildComponents;
            CollisionComponent->GetChildrenComponents(true, ChildComponents);

            for (USceneComponent* ChildComponent : ChildComponents)
            {
                USkeletalMeshComponent* SkeletalMeshComp = Cast<USkeletalMeshComponent>(ChildComponent);
                if (SkeletalMeshComp)
                {
                    MeshComponent = SkeletalMeshComp;
                    break;
                }
            }

            if (!MeshComponent)
            {
                UE_LOG(LogTemp, Warning, TEXT("SkeletalMeshComponent not found under the collision component on actor %s."), *GetOwner()->GetName());
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("CollisionComponent not found on actor %s."), *GetOwner()->GetName());
        }
    }
}

void UHighlightComponent::Highlight()
{
    if (!MeshComponent || !OverlayMaterial)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot highlight: MeshComponent or OverlayMaterial is missing."));
        return;
    }

    MeshComponent->SetOverlayMaterial(OverlayMaterial);

    UE_LOG(LogTemp, Log, TEXT("Overlay material applied to %s"), *GetOwner()->GetName());
}

void UHighlightComponent::RemoveHighlight()
{
    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot remove highlight: MeshComponent is missing or OriginalMaterials were not stored."));
        return;
    }

    MeshComponent->SetOverlayMaterial(nullptr);

    UE_LOG(LogTemp, Log, TEXT("Original materials restored on %s"), *GetOwner()->GetName());
}

void UHighlightComponent::ToggleHighlight()
{
    if (MeshComponent)
    {
        if (MeshComponent->GetOverlayMaterial() != nullptr)
        {
            Highlight();
        }
        else
        {
            RemoveHighlight();
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("MeshComponent is null in ToggleHighlight."));
    }
}