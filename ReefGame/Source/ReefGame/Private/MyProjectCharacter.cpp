#include "MyProjectCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PlayerPerceptionSensor.h"
#include "InputAction.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "FishCollection.h"
#include "FishInfoWidget.h"
#include "ReefGame/BaseFish.h"
#include "UObject/UnrealTypePrivate.h"

AMyProjectCharacter::AMyProjectCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    PerceptionSensor = CreateDefaultSubobject<UPlayerPerceptionSensor>(TEXT("PerceptionSensor"));

    // Attach it to the RootComponent or any other component as needed
    PerceptionSensor->SetupAttachment(RootComponent);

    // Configure the perception sensor
    PerceptionSensor->SetSphereRadius(200.0f); // Adjust the radius as needed
    PerceptionSensor->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PerceptionSensor->SetCollisionObjectType(ECC_WorldDynamic);
    PerceptionSensor->SetCollisionResponseToAllChannels(ECR_Ignore);
    PerceptionSensor->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    PerceptionSensor->SetGenerateOverlapEvents(true);

    InteractPromptWidgetInstance = nullptr;
    bIsInteractPromptVisible = false;
}

void AMyProjectCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Ensure the controller is valid
    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (PlayerController)
    {
        // Get the Enhanced Input Local Player Subsystem
        UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
        if (Subsystem)
        {
            // Add the input mapping context
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }

    GetCharacterMovement()->SetMovementMode(MOVE_Flying);
    GetCharacterMovement()->MaxFlySpeed = 4000.0f;
    GetCharacterMovement()->MaxAcceleration = 10000.0f;
    GetCharacterMovement()->BrakingDecelerationFlying = 10000.0f;

    if (PerceptionSensor)
    {
        PerceptionSensor->OnHighlightedFishChanged.AddDynamic(this, &AMyProjectCharacter::OnHighlightedFishChanged);
    }

    // Create and add the interact prompt widget to the viewport
    if (InteractPromptWidgetClass)
    {
        InteractPromptWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), InteractPromptWidgetClass);
        if (InteractPromptWidgetInstance)
        {
            InteractPromptWidgetInstance->AddToViewport();
            InteractPromptWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
            bIsInteractPromptVisible = false;
        }
    }

    if (HowToOpenCollectionMenuClass)
    {
        HowToOpenCollectionMenuInstance = CreateWidget<UUserWidget>(GetWorld(), HowToOpenCollectionMenuClass);
        if (HowToOpenCollectionMenuInstance)
        {
            HowToOpenCollectionMenuInstance->AddToViewport();
        }
    }
}

void AMyProjectCharacter::Server_AddFishToCollection_Implementation(EFishType FishType)
{
    AFishCollection* FishCollectionGameState = GetWorld()->GetGameState<AFishCollection>();
    if (FishCollectionGameState)
    {
        FishCollectionGameState->AddFishToCollection(FishType);
    }
}

bool AMyProjectCharacter::Server_AddFishToCollection_Validate(EFishType FishType)
{
    return true; // Add any validation logic if needed
}

void AMyProjectCharacter::ShowFishInfoWidget(EFishType FishType)
{
    if (FishInfoWidgetClass)
    {
        // Create the widget if it doesn't exist
        if (!FishInfoWidgetInstance)
        {
            FishInfoWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), FishInfoWidgetClass);
            if (FishInfoWidgetInstance)
            {
                FishInfoWidgetInstance->AddToViewport();
            }
        }

        if (FishInfoWidgetInstance)
        {
            // Cast to your specific widget class if needed
            UFishInfoWidget* FishInfoWidget = Cast<UFishInfoWidget>(FishInfoWidgetInstance);
            if (FishInfoWidget)
            {
                FishInfoWidget->SetFishType(FishType);
            }

            // Show the widget
            FishInfoWidgetInstance->SetVisibility(ESlateVisibility::Visible);
            bIsFishInfoWidgetDisplayed = true;
        }
    }
}

void AMyProjectCharacter::ShowCollectionNotification(EFishType FishType)
{
    if (!CollectionNotificationInstance)
    {
        // Create the notification widget if it doesn’t already exist
        CollectionNotificationInstance = CreateWidget<UUserWidget>(GetWorld(), CollectionNotificationClass);
    }

    if (CollectionNotificationInstance)
    {
        // Add to viewport if not already there
        if (!CollectionNotificationInstance->IsInViewport())
        {
            CollectionNotificationInstance->AddToViewport();
        }

        // Find and call the blueprint function `SetNotificationText`
        FName FunctionName = FName("SetNotificationText");
        if (CollectionNotificationInstance->FindFunction(FunctionName))
        {
            struct FNotificationParams
            {
                EFishType FishType;
            };
            FNotificationParams Params;
            Params.FishType = FishType;
            
            CollectionNotificationInstance->ProcessEvent(CollectionNotificationInstance->FindFunction(FunctionName), &Params);
        }

        // Set a timer to hide the notification after 5 seconds
        GetWorld()->GetTimerManager().SetTimer(NotificationTimerHandle, this, &AMyProjectCharacter::HideCollectionNotification, 2.5f, false);
    }
}

void AMyProjectCharacter::HideCollectionNotification()
{
    if (CollectionNotificationInstance)
    {
        CollectionNotificationInstance->RemoveFromParent();
    }
}

void AMyProjectCharacter::HideFishInfoWidget()
{
    if (FishInfoWidgetInstance)
    {
        FishInfoWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
        bIsFishInfoWidgetDisplayed = false;
    }
}

void AMyProjectCharacter::OnHighlightedFishChanged(ABaseFish* NewHighlightedFish)
{
    if (NewHighlightedFish)
    {
        ShowInteractPrompt();
    }
    else
    {
        HideInteractPrompt();
    }
}

void AMyProjectCharacter::ShowInteractPrompt()
{
    if (InteractPromptWidgetInstance && !bIsInteractPromptVisible)
    {
        InteractPromptWidgetInstance->SetVisibility(ESlateVisibility::Visible);
        bIsInteractPromptVisible = true;
    }
    UE_LOG(LogTemp, Warning, TEXT("Show E"));
}

void AMyProjectCharacter::HideInteractPrompt()
{
    if (InteractPromptWidgetInstance && bIsInteractPromptVisible)
    {
        InteractPromptWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
        bIsInteractPromptVisible = false;
    }
}

void AMyProjectCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AMyProjectCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Ensure we're using the Enhanced Input Component
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyProjectCharacter::Move);
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyProjectCharacter::Look);
        EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AMyProjectCharacter::Interact);
        EnhancedInputComponent->BindAction(OpenCollectionMenuAction, ETriggerEvent::Started, this, &AMyProjectCharacter::ToggleCollectionMenu);
    }
}

void AMyProjectCharacter::Move(const FInputActionValue& Value)
{
    // Get the movement vector from the input action value
    FVector2D MovementVector = Value.Get<FVector2D>();

    // Log the raw movement input
    //UE_LOG(LogTemp, Log, TEXT("Move Input: X = %f, Y = %f"), MovementVector.X, MovementVector.Y);

    if (Controller != nullptr)
    {
        // Get the control rotation, which includes pitch and yaw
        FRotator ControlRotation = Controller->GetControlRotation();

        // Remove roll component
        ControlRotation.Roll = 0.0f;

        // Calculate direction vectors
        FVector ForwardDirection = FRotationMatrix(ControlRotation).GetScaledAxis(EAxis::X);
        FVector RightDirection = FRotationMatrix(ControlRotation).GetScaledAxis(EAxis::Y);

        // Log the control rotation and direction vectors UE_LO//G(LogTemp, Log, TEXT("Control Rotation: Pitch = %f, Yaw = %f"), ControlRotation.Pitch, ControlRotation.Yaw);
        //UE_LOG(LogTemp, Log, TEXT("Forward Direction: X = %f, Y = %f, Z = %f"), ForwardDirection.X, ForwardDirection.Y, ForwardDirection.Z);
        //UE_LOG(LogTemp, Log, TEXT("Right Direction: X = %f, Y = %f, Z = %f"), RightDirection.X, RightDirection.Y, RightDirection.Z);

        // Apply movement input
        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);

        // Log the movement input application
        //UE_LOG(LogTemp, Log, TEXT("Applied Movement Input: Forward = %f, Right = %f"), MovementVector.Y, MovementVector.X);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Controller is null in Move()"));
    }
}

void AMyProjectCharacter::Look(const FInputActionValue& Value)
{
    // Get the look axis vector from the input action value
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    // Log the raw look input
    //UE_LOG(LogTemp, Log, TEXT("Look Input: X = %f, Y = %f"), LookAxisVector.X, LookAxisVector.Y);

    if (Controller != nullptr)
    {
        // Apply look input to control the character's rotation
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);

        // Log the application of look input
        //UE_LOG(LogTemp, Log, TEXT("Applied Look Input: Yaw = %f, Pitch = %f"), LookAxisVector.X, LookAxisVector.Y);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Controller is null in Look()"));
    }
}

void AMyProjectCharacter::Interact()
{
    if (bIsFishInfoWidgetDisplayed)
    {
        // Close the fish info widget
        HideFishInfoWidget();
    }
    else
    {
        // Proceed with interacting with the highlighted fish
        if (PerceptionSensor)
        {
            ABaseFish* HighlightedFish = PerceptionSensor->GetCurrentlyHighlightedFish();
            if (HighlightedFish)
            {
                EFishType HighlightedFishType = HighlightedFish->GetFishType();
                FString FishTypeName = UEnum::GetDisplayValueAsText(HighlightedFishType).ToString();
                UE_LOG(LogTemp, Log, TEXT("Interacting with Fish Type: %s"), *FishTypeName);

                // Add fish to collection
                AFishCollection* FishCollectionGameState = GetWorld()->GetGameState<AFishCollection>();
                if (FishCollectionGameState)
                {
                    if (!FishCollectionGameState->IsFishCollected(HighlightedFishType))
                    {
                        // Request the server to add the fish to the collection
                        Server_AddFishToCollection(HighlightedFishType);
                    }
                }

                // Show fish info UI
                ShowFishInfoWidget(HighlightedFishType);
            }
        }
    }
}

void AMyProjectCharacter::ToggleCollectionMenu()
{
    if (CollectionMenuInstance && CollectionMenuInstance->IsVisible())
    {
        // Hide the widget
        CollectionMenuInstance->SetVisibility(ESlateVisibility::Hidden);

        // Restore input mode to game only
        APlayerController* PC = Cast<APlayerController>(GetController());
        if (PC)
        {
            PC->SetInputMode(FInputModeGameOnly());
            PC->bShowMouseCursor = false;
        }
    }
    else
    {
        if (!CollectionMenuInstance && CollectionMenuClass)
        {
            CollectionMenuInstance = CreateWidget<UUserWidget>(GetWorld(), CollectionMenuClass);
            if (CollectionMenuInstance)
            {
                CollectionMenuInstance->AddToViewport();

                // Initialize the collection menu
                InitializeCollectionMenu();
            }
        }
        else if (CollectionMenuInstance)
        {
            // Show the widget
            CollectionMenuInstance->SetVisibility(ESlateVisibility::Visible);

            // Re-initialize the collection menu if needed
            InitializeCollectionMenu();
        }

        // Set input mode to Game and UI
        APlayerController* PC = Cast<APlayerController>(GetController());
        if (PC)
        {
            FInputModeGameAndUI InputMode;
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PC->SetInputMode(InputMode);
            PC->bShowMouseCursor = true;
        }
    }
}

void AMyProjectCharacter::InitializeCollectionMenu()
{
    AFishCollection* FishCollection = GetWorld()->GetGameState<AFishCollection>();
    if (FishCollection)
    {
        // Load the data table
        UDataTable* DataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/UI/DT_FishInfo.DT_FishInfo"));
        if (!DataTable)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to load DataTable"));
            return;
        }

        TSet<EFishType> CollectedFishSet(FishCollection->CollectedFish);

        // Call InitializeCollection event in Blueprint
        FName FunctionName = FName("SetCollectedFish");
        if (CollectionMenuInstance->FindFunction(FunctionName))
        {
            struct FInitializeCollectionParams
            {
                TSet<EFishType> CollectedFishSet;
            };
            FInitializeCollectionParams Params;
            Params.CollectedFishSet = CollectedFishSet;

            CollectionMenuInstance->ProcessEvent(CollectionMenuInstance->FindFunction(FunctionName), &Params);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("FishCollection is null"));
    }
}