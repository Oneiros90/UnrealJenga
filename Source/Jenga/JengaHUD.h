// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "JengaHUD.generated.h"

class UClass;
class UUserWidget;
class UButton;
class UTextBlock;

/**
 * 
 */
UCLASS()
class JENGA_API AJengaHUD : public AHUD
{
	GENERATED_BODY()

public:
   // Constructor
   AJengaHUD();

   // Adds one player to the game
   UFUNCTION() void AddOnePlayer();
   // Removes one player to the game
   UFUNCTION() void RemoveOnePlayer();
   // Restart the game
   UFUNCTION() void Restart();
   // Undo last action
   UFUNCTION() void Undo();
   // Redo last action
   UFUNCTION() void Redo();

protected:
   // Called when the game starts or when spawned
   virtual void BeginPlay() override;

private:
   void setNoOfPlayers(int n);

   // Private utilities
   UButton* getButton(const FName& s);
   UTextBlock* getText(const FName& s);

private:
   // Pointers to HUD widget class and instance
   UClass* hudWidgetClass;
   UUserWidget* hudWidget;

   int noOfPlayers;
};
