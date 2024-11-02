#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "HighlightComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class REEFGAME_API UHighlightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHighlightComponent();

	// Toggle highlight visibility
	UFUNCTION(BlueprintCallable, Category = "Highlight")
	void Highlight();

	UFUNCTION(BlueprintCallable, Category = "Highlight")
	void RemoveHighlight();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
#endif


protected:
	virtual void BeginPlay() override;

private:
	// Overlay material to apply as a highlight effect
	UPROPERTY(EditAnywhere, Category = "Highlight")
	UMaterialInterface* OverlayMaterial;

	// Mesh component to apply the overlay on
	UPROPERTY(EditAnywhere, Category = "Highlight")
	USkeletalMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, Category = "Highlight")
	bool bHighlightEnabled;

	bool bPendingHighlightUpdate;

	// Toggle highlight based on bToggleHighlight property
	void ToggleHighlight();
};
