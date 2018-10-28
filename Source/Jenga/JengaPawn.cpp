// Fill out your copyright notice in the Description page of Project Settings.

#include "JengaPawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/PrimitiveComponent.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

#include <string>

///////////////////////////////////////////////////////////////////////////
// Sets default values
AJengaPawn::AJengaPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

   this->springArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
   this->springArm->AttachTo(RootComponent);
   this->springArm->TargetArmLength = 250.0f;
   this->springArm->bDoCollisionTest = false;
   this->springArm->SetWorldRotation(FRotator(-40.f, 0.0f, 0.0f));

   this->camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
   this->camera->AttachTo(this->springArm, USpringArmComponent::SocketName);

   AutoPossessPlayer = EAutoReceiveInput::Player0;

   upAndDown = 0.0f;
   zoom = 0.0f;
   navigationEnabled = false;
}

///////////////////////////////////////////////////////////////////////////
// Called when the game starts or when spawned
void AJengaPawn::BeginPlay()
{
	Super::BeginPlay();
}

///////////////////////////////////////////////////////////////////////////
// Called to bind functionality to input
void AJengaPawn::SetupPlayerInputComponent(UInputComponent* inputComponent)
{
   Super::SetupPlayerInputComponent(inputComponent);

   inputComponent->BindAction("EnableNavigation", IE_Pressed, this, &AJengaPawn::EnableNavigation);
   inputComponent->BindAction("EnableNavigation", IE_Released, this, &AJengaPawn::DisableNavigation);
   inputComponent->BindAction("GoUp", IE_Pressed, this, &AJengaPawn::GoUp);
   inputComponent->BindAction("GoUp", IE_Released, this, &AJengaPawn::GoDown);
   inputComponent->BindAction("GoDown", IE_Pressed, this, &AJengaPawn::GoDown);
   inputComponent->BindAction("GoDown", IE_Released, this, &AJengaPawn::GoUp);
   inputComponent->BindAxis("CameraYaw", this, &AJengaPawn::SetCameraYaw);
   inputComponent->BindAxis("CameraPitch", this, &AJengaPawn::SetCameraPitch);
   inputComponent->BindAxis("Zoom", this, &AJengaPawn::SetZoom);

}

///////////////////////////////////////////////////////////////////////////
// Called every frame
void AJengaPawn::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

   FVector pawnLocation = GetActorLocation();
   pawnLocation.Z = FMath::Clamp(pawnLocation.Z + this->upAndDown, 0.0f, 255.0f);
   SetActorLocation(pawnLocation);

   if (navigationEnabled)
   {
      FRotator springArmRot = this->springArm->GetComponentRotation();
      springArmRot.Yaw += this->cameraRot.X;
      springArmRot.Pitch = FMath::Clamp(springArmRot.Pitch + this->cameraRot.Y, -60.0f, 0.0f);
      this->springArm->SetWorldRotation(springArmRot);
   }

   float springArmLength = this->springArm->TargetArmLength;
   springArmLength = FMath::Clamp(springArmLength - this->zoom, 150.0f, 500.0f);
   this->springArm->TargetArmLength = springArmLength;
}
