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

   // Resets the blocks position
   void NewTower();

protected:
   // Called when the game starts or when spawned
   virtual void BeginPlay() override;

   typedef TArray<FTransform> TowerConfiguration;

   TowerConfiguration getActualTowerConfiguration();
   void applyTowerConfiguration(TowerConfiguration towerConf);

private:
   TArray<TowerConfiguration> blocksMemory;

};
