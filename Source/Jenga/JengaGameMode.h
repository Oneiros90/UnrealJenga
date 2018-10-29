// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "JengaGameMode.generated.h"

/**
*
*/
UCLASS()
class JENGA_API AJengaGameMode : public AGameModeBase
{
   GENERATED_BODY()

public:
   // Constructor
   AJengaGameMode();

protected:
   // Called when the game starts or when spawned
   virtual void BeginPlay() override;

};
