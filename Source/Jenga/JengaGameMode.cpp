// Fill out your copyright notice in the Description page of Project Settings.

#include "JengaGameMode.h"
#include "JengaPawn.h"

#include "Engine/World.h"
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>



AJengaGameMode::AJengaGameMode()
{
   DefaultPawnClass = AJengaPawn::StaticClass();
}


void AJengaGameMode::BeginPlay()
{
   GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("BeginPlay!"));
   UWorld* world = GetWorld();
   if (world)
   {
      GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("World!"));
      APlayerController* playerController = world->GetFirstPlayerController();
      if (playerController)
      {
         GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("GetFirstPlayerController!"));
         playerController->bShowMouseCursor = true;
      }
   }
}


