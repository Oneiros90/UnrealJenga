// Fill out your copyright notice in the Description page of Project Settings.

#include "JengaHUD.h"
#include "JengaGameMode.h"

#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/Components/Button.h"
#include "Runtime/UMG/Public/Components/TextBlock.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

const int MIN_NO_OF_PLAYERS = 1;
const int MAX_NO_OF_PLAYERS = 100;

///////////////////////////////////////////////////////////////////////////
// Constructor
AJengaHUD::AJengaHUD()
{
   // This is done to integrate a blueprint HUD widget inside C++.
   // Yeah i know, it's not nice to hardcode the widget's reference...
   static ConstructorHelpers::FClassFinder<UUserWidget> widgetFinder(TEXT("/Game/Jenga/HUD/JengaHUD_BP"));
   hudWidgetClass = widgetFinder.Succeeded() ? widgetFinder.Class : nullptr;
}

///////////////////////////////////////////////////////////////////////////
// Called when the game starts or when spawned
void AJengaHUD::BeginPlay()
{
   Super::BeginPlay();

   // Make sure that the HUD widget has been found
   if (!hudWidgetClass)
   {
      GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "ERROR: Cannot find HUD!");
      return;
   }

   hudWidget = CreateWidget<UUserWidget>(this->GetOwningPlayerController(), this->hudWidgetClass);
   hudWidget->AddToViewport();

   getButton("PlayersNoMinusButton")->OnClicked.AddDynamic(this, &AJengaHUD::RemoveOnePlayer);
   getButton("PlayersNoPlusButton")->OnClicked.AddDynamic(this, &AJengaHUD::AddOnePlayer);
   getButton("RestartButton")->OnClicked.AddDynamic(this, &AJengaHUD::Restart);
   getButton("UndoButton")->OnClicked.AddDynamic(this, &AJengaHUD::Undo);
   getButton("RedoButton")->OnClicked.AddDynamic(this, &AJengaHUD::Redo);

   this->setNoOfPlayers(MIN_NO_OF_PLAYERS);
   this->Restart();
}

///////////////////////////////////////////////////////////////////////////
// Adds one player to the game
void AJengaHUD::AddOnePlayer()
{
   setNoOfPlayers(this->noOfPlayers + 1);
}

///////////////////////////////////////////////////////////////////////////
// Removes one player to the game
void AJengaHUD::RemoveOnePlayer()
{
   setNoOfPlayers(this->noOfPlayers - 1);
}

///////////////////////////////////////////////////////////////////////////
// Restarts the game
void AJengaHUD::Restart()
{
   const FString noOfPlayersString = FString::FromInt(this->noOfPlayers);
   GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "Starting a game with " + noOfPlayersString + " player(s)!");

   AJengaGameMode* gameMode = (AJengaGameMode*)UGameplayStatics::GetGameMode(GetWorld());
   gameMode->NewTower();
}

///////////////////////////////////////////////////////////////////////////
// Undo last action
void AJengaHUD::Undo()
{
   GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, "TODO!");
}

///////////////////////////////////////////////////////////////////////////
// Redo last action
void AJengaHUD::Redo()
{
   GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, "TODO!");
}

///////////////////////////////////////////////////////////////////////////
// Updates the buttons and the labels related to the number of players
void AJengaHUD::setNoOfPlayers(int n)
{
   this->noOfPlayers = FMath::Clamp(n, MIN_NO_OF_PLAYERS, MAX_NO_OF_PLAYERS);
   const FString noOfPlayersString = FString::FromInt(this->noOfPlayers);
   //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, "New number of players: " + noOfPlayersString);

   getButton("PlayersNoMinusButton")->SetIsEnabled(this->noOfPlayers > MIN_NO_OF_PLAYERS);
   getButton("PlayersNoPlusButton")->SetIsEnabled(this->noOfPlayers < MAX_NO_OF_PLAYERS);
   getText("PlayersNo")->SetText(FText::FromString(noOfPlayersString));
}

///////////////////////////////////////////////////////////////////////////
// Finds and return a button
UButton* AJengaHUD::getButton(const FName& s)
{
   return (UButton*)hudWidget->GetWidgetFromName(s);
}

///////////////////////////////////////////////////////////////////////////
// Finds and return a text
UTextBlock* AJengaHUD::getText(const FName& s)
{
   return (UTextBlock*)hudWidget->GetWidgetFromName(s);
}
