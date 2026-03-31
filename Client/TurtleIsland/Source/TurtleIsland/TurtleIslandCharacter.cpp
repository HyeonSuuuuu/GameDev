// Copyright Epic Games, Inc. All Rights Reserved.

#include "TurtleIslandCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "TurtleIsland.h"
#include "Network/NetSubsystem.h"
#include "Protocol.h"

ATurtleIslandCharacter::ATurtleIslandCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ATurtleIslandCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATurtleIslandCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ATurtleIslandCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATurtleIslandCharacter::Look);
	}
	else
	{
		UE_LOG(LogTurtleIsland, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ATurtleIslandCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	LastMoveInputTime += DeltaTime;

	if (LastMoveInputTime > PacketSendInterval)
	{
		LastMoveInputTime -= PacketSendInterval;
		if (InputFlag > 0)
		{
			UNetSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetSubsystem>();
			if (NetSubsystem)
			{
				CS_Move MovePacket{};
				MovePacket.size = CS_MOVE_PACKET_SIZE;
				MovePacket.id = (uint16)PACKET_ID::CS_MOVE;
				MovePacket.inputFlag = InputFlag;

				TArray<uint8> SendBuffer;
				SendBuffer.SetNumUninitialized(CS_MOVE_PACKET_SIZE);
				FMemory::Memcpy(SendBuffer.GetData(), &MovePacket, CS_MOVE_PACKET_SIZE);

				NetSubsystem->EnqueSendPacket(MoveTemp(SendBuffer));
				UE_LOG(LogTemp, Log, TEXT("이동패킷 전송 완료"));
				InputFlag = 0;
			}
		}
	}
}

void ATurtleIslandCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void ATurtleIslandCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ATurtleIslandCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);

		// 서버로 보낼 InputFlag
		if (Forward > 0.1f) InputFlag |= 1; // W
		if (Right < -0.1f) InputFlag |= 2; // A
		if (Forward < -0.1f)   InputFlag |= 4; // S
		if (Right > 0.1f)    InputFlag |= 8; // D
	}
}

void ATurtleIslandCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ATurtleIslandCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void ATurtleIslandCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}
