#include "MyProjectCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h" // For logging functions

AMyProjectCharacter::AMyProjectCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Set the character to fly mode to allow movement in all directions

    // Set the max fly speed to 6000
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
        // Bind the Move and Look functions to the input actions
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyProjectCharacter::Move);
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyProjectCharacter::Look);
    }
}

void AMyProjectCharacter::Move(const FInputActionValue& Value)
{
    // Get the movement vector from the input action value
    FVector2D MovementVector = Value.Get<FVector2D>();

    // Log the raw movement input
    UE_LOG(LogTemp, Log, TEXT("Move Input: X = %f, Y = %f"), MovementVector.X, MovementVector.Y);

    if (Controller != nullptr)
    {
        // Get the control rotation, which includes pitch and yaw
        FRotator ControlRotation = Controller->GetControlRotation();

        // Remove roll component
        ControlRotation.Roll = 0.0f;

        // Calculate direction vectors
        FVector ForwardDirection = FRotationMatrix(ControlRotation).GetScaledAxis(EAxis::X);
        FVector RightDirection = FRotationMatrix(ControlRotation).GetScaledAxis(EAxis::Y);

        // Log the control rotation and direction vectors
        UE_LOG(LogTemp, Log, TEXT("Control Rotation: Pitch = %f, Yaw = %f"), ControlRotation.Pitch, ControlRotation.Yaw);
        UE_LOG(LogTemp, Log, TEXT("Forward Direction: X = %f, Y = %f, Z = %f"), ForwardDirection.X, ForwardDirection.Y, ForwardDirection.Z);
        UE_LOG(LogTemp, Log, TEXT("Right Direction: X = %f, Y = %f, Z = %f"), RightDirection.X, RightDirection.Y, RightDirection.Z);

        // Apply movement input
        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);

        // Log the movement input application
        UE_LOG(LogTemp, Log, TEXT("Applied Movement Input: Forward = %f, Right = %f"), MovementVector.Y, MovementVector.X);
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
    UE_LOG(LogTemp, Log, TEXT("Look Input: X = %f, Y = %f"), LookAxisVector.X, LookAxisVector.Y);

    if (Controller != nullptr)
    {
        // Apply look input to control the character's rotation
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);

        // Log the application of look input
        UE_LOG(LogTemp, Log, TEXT("Applied Look Input: Yaw = %f, Pitch = %f"), LookAxisVector.X, LookAxisVector.Y);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Controller is null in Look()"));
    }
}