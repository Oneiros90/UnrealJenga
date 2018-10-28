// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "JengaPlayerController.generated.h"

class UPhysicsHandleComponent;

/**
*
*/
UCLASS()
class JENGA_API AJengaPlayerController : public APlayerController
{
	GENERATED_BODY()

   // Sets default values for this pawn's properties
   AJengaPlayerController();

protected:
   // Called when the game starts or when spawned
   virtual void BeginPlay() override;

   // Called every frame
   virtual void Tick(float DeltaTime) override;
	
private:

   // Drag events
   void DraggingStart(FVector2D screenPos);
   void DraggingUpdate(FVector2D screenPos);
   void DraggingStop();

   UPhysicsHandleComponent* physicsHandle;
   FHitResult lastPick;
	
};
