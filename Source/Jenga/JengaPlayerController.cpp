// Fill out your copyright notice in the Description page of Project Settings.

#include "JengaPlayerController.h"
#include "Components/PrimitiveComponent.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
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

   // Ray-tracing to find a picked actor
   this->lastPick.Reset();
   FVector rayStart = worldPos;
   FVector rayEnd = rayStart + worldDir * RAY_LENGTH;
   if (GetWorld()->LineTraceSingleByChannel(this->lastPick, rayStart, rayEnd, ECollisionChannel::ECC_Visibility))
   {
      // Creating the handle
      physicsHandle = NewObject<UPhysicsHandleComponent>(this, TEXT("PhysicsHandle"));
      physicsHandle->GrabComponent(this->lastPick.GetComponent(), NAME_None, this->lastPick.ImpactPoint, false);
      physicsHandle->SetTargetLocation(this->lastPick.ImpactPoint);
      physicsHandle->RegisterComponent();

      // Locking component rotations
      lockRotations(*this->lastPick.GetComponent(), true);

#if defined(UE_EDITOR) || defined(UE_BUILD_DEBUG)
      // Debug message on screen
      GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Picking ") + this->lastPick.GetComponent()->GetOwner()->GetName());
#endif
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

#if defined(UE_EDITOR) || defined(UE_BUILD_DEBUG)
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
      lockRotations(*this->lastPick.GetComponent(), false);
      this->lastPick.Reset();
      physicsHandle->ReleaseComponent();
   }
}


