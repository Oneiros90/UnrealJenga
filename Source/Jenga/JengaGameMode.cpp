// Fill out your copyright notice in the Description page of Project Settings.

#include "JengaGameMode.h"
#include "JengaPawn.h"
#include "JengaPlayerController.h"
#include "JengaHUD.h"

#include "Engine/World.h"
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>


///////////////////////////////////////////////////////////////////////////
// Constructor
AJengaGameMode::AJengaGameMode()
{
   DefaultPawnClass = AJengaPawn::StaticClass();
   PlayerControllerClass = AJengaPlayerController::StaticClass();
   HUDClass = AJengaHUD::StaticClass();
}

///////////////////////////////////////////////////////////////////////////
// Called when the game starts or when spawned
void AJengaGameMode::BeginPlay()
{
   GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Welcome to Jenga"));
}


