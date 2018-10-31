// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "JengaGameMode.generated.h"

class AActor;

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

   // Starts a new game
   void NewGame(int nPlayers);

   void NewPick(AActor* jengaBlock);
   void PickReleased(AActor* jengaBlock);

protected:
   // Called when the game starts or when spawned
   virtual void BeginPlay() override;
   virtual void Tick(float deltaTime) override;

   void NextRound();
   void GameOver(FString msg);
   int CurrentPlayer();
   bool IsOnTop(AActor* jengaBlock);

   void SetInteractive(AActor* jengaBlock, bool b);
   bool IsInteractive(AActor* jengaBlock);

   typedef TArray<FTransform> TowerConfiguration;
   TowerConfiguration GetActualTowerConfiguration();
   void ApplyTowerConfiguration(TowerConfiguration towerConf);

   UFUNCTION() void OnFloorHit(
      UPrimitiveComponent* hitComponent,
      AActor* otherActor,
      UPrimitiveComponent* otherComponent,
      FVector normalImpulse,
      const FHitResult& hit
   );

private:
   TArray<AActor*> jengaBlocks;
   TArray<TowerConfiguration> blocksMemory;

   int turn;
   int nPlayers;
   TSet<AActor*> jengaBlocksOnFloor;

   AActor* pickedJengaBlock;
   bool holdingPickedJengaBlock;
   enum TowerStatus { BALANCED, MOVING, COLLAPSED };
   TowerStatus towerStatus;
};
