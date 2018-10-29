// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "JengaHUD.generated.h"

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

protected:
   // Called when the game starts or when spawned
   virtual void BeginPlay() override;

private:
   // Called when the reset button is clicked
   UFUNCTION() void OnResetButtonClicked();

private:
   // Pointers to HUD widget class and instance
   class UClass* hudWidgetClass;
   class UUserWidget* hudWidget;
};
