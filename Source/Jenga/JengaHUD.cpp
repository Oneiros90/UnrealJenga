// Fill out your copyright notice in the Description page of Project Settings.

#include "JengaHUD.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"

#include "Runtime/UMG/Public/Components/Button.h"


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
   if (hudWidgetClass)
   {
      hudWidget = CreateWidget<UUserWidget>(this->GetOwningPlayerController(), this->hudWidgetClass);
      hudWidget->AddToViewport();

      UButton* resetButton = (UButton*) hudWidget->GetWidgetFromName(TEXT("ResetButton"));
      resetButton->OnClicked.AddDynamic(this, &AJengaHUD::OnResetButtonClicked);
   }
   else
      GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Cannot find HUD!"));
}

///////////////////////////////////////////////////////////////////////////
// Called when the game starts or when spawned
void AJengaHUD::OnResetButtonClicked()
{
   GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("TODO: Reset"));
}