// Fill out your copyright notice in the Description page of Project Settings.

#include "JengaPlayerController.h"
#include "JengaGameMode.h"

#include "Components/PrimitiveComponent.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/PhysicsEngine/PhysicsHandleComponent.h"
#include "DrawDebugHelpers.h"


///////////////////////////////////////////////////////////////////////////
// Utility used to lock physical rotations of a component
inline void lockRotations(UPrimitiveComponent& component, bool lock)
{
   component.BodyInstance.bLockXRotation = lock;
   component.BodyInstance.bLockYRotation = lock;
   component.BodyInstance.bLockZRotation = lock;
   component.BodyInstance.CreateDOFLock();
}

const float RAY_LENGTH = 2000;

///////////////////////////////////////////////////////////////////////////
// Constructor
AJengaPlayerController::AJengaPlayerController()
{
   physicsHandle = nullptr;
   bShowMouseCursor = true;
}

///////////////////////////////////////////////////////////////////////////
// Called when the game starts or when spawned
void AJengaPlayerController::BeginPlay()
{
   Super::BeginPlay();
}

///////////////////////////////////////////////////////////////////////////
// Called every frame
void AJengaPlayerController::Tick(float deltaTime)
{
   Super::Tick(deltaTime);

   // Is mouse left button down?
   if (IsInputKeyDown(EKeys::LeftMouseButton))
   {
      FVector2D mouseScreenPos;
      GetMousePosition(mouseScreenPos.X, mouseScreenPos.Y);

      // First frame with mouse pressed? Start dragging
      if (this->lastPick.GetComponent() == nullptr)
         DraggingStart(mouseScreenPos);

      // Else, update dragging
      else
         DraggingUpdate(mouseScreenPos);
   }

   // Button release
   else
      DraggingStop();
}

///////////////////////////////////////////////////////////////////////////
// Tries to pick an actor from given screen space coordinates and attaches a physic handle to it
void AJengaPlayerController::DraggingStart(FVector2D screenPos)
{
   // Retrieving mouse position in world
   FVector worldPos, worldDir;
   DeprojectScreenPositionToWorld(screenPos.X, screenPos.Y, worldPos, worldDir);

   // Ray-tracing to find a pickable actor
   this->lastPick.Reset();
   FVector rayStart = worldPos;
   FVector rayEnd = rayStart + worldDir * RAY_LENGTH;
   if (GetWorld()->LineTraceSingleByChannel(this->lastPick, rayStart, rayEnd, ECollisionChannel::ECC_Visibility))
   {
      AActor* pickedActor = this->lastPick.GetComponent()->GetOwner();
      if (pickedActor->Tags.Contains("Interactive"))
      {
         // Creating the handle
         physicsHandle = NewObject<UPhysicsHandleComponent>(this, TEXT("PhysicsHandle"));
         physicsHandle->GrabComponent(this->lastPick.GetComponent(), NAME_None, this->lastPick.ImpactPoint, false);
         physicsHandle->SetTargetLocation(this->lastPick.ImpactPoint);
         physicsHandle->RegisterComponent();

         // Locking component rotations
         lockRotations(*this->lastPick.GetComponent(), true);

         // Updating the GameMode
         AJengaGameMode* gameMode = (AJengaGameMode*)UGameplayStatics::GetGameMode(GetWorld());
         gameMode->NewPick(pickedActor);
      }
      else
      {
         // Picking a not-interactive actor
         this->lastPick.Reset();
      }
   }
}

///////////////////////////////////////////////////////////////////////////
// Updates physic handle using given screen space coordinates
void AJengaPlayerController::DraggingUpdate(FVector2D screenPos)
{
   FVector worldPos, worldDir;
   DeprojectScreenPositionToWorld(screenPos.X, screenPos.Y, worldPos, worldDir);
   FVector target = worldPos + worldDir * this->lastPick.Distance;
   physicsHandle->SetTargetLocation(target);

#if defined(UE_BUILD_DEBUG)
   // Draw debug arrow
   DrawDebugDirectionalArrow(GetWorld(), this->lastPick.ImpactPoint, target, 10, FColor::Red, false, -1.0f, 0, 1.0f);
#endif
}

///////////////////////////////////////////////////////////////////////////
// Releases the physic handle
void AJengaPlayerController::DraggingStop()
{
   if (this->lastPick.GetComponent())
   {
      AActor* pickedActor = this->lastPick.GetComponent()->GetOwner();

      // Updating the GameMode
      AJengaGameMode* gameMode = (AJengaGameMode*)UGameplayStatics::GetGameMode(GetWorld());
      gameMode->PickReleased(pickedActor);

      lockRotations(*this->lastPick.GetComponent(), false);
      this->lastPick.Reset();
      physicsHandle->ReleaseComponent();
   }
}


