// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "JengaPawn.generated.h"

/**
*
*/
UCLASS()
class JENGA_API AJengaPawn : public APawn
{
	GENERATED_BODY()

public:
   // Constructor
	AJengaPawn();

protected:
   // Called when the game starts or when spawned
   virtual void BeginPlay() override;

   // Called to bind functionality to input
   virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

   // Called every frame
   virtual void Tick(float DeltaTime) override;

private:
   // Control navigation
   void GoUp() { upAndDown += 1.0f; }
   void GoDown() { upAndDown -= 1.0f; }
   void EnableNavigation() { navigationEnabled = true; }
   void DisableNavigation() { navigationEnabled = false; }
   void SetCameraYaw(float f) { cameraRot.X = f; }
   void SetCameraPitch(float f) { cameraRot.Y = f; }
   void SetZoom(float f) { zoom = f; }

private:
   USpringArmComponent * springArm;
   UCameraComponent* camera;
   FVector2D cameraRot;
   float upAndDown, zoom;
   bool navigationEnabled;

};
