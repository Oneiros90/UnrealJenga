// Fill out your copyright notice in the Description page of Project Settings.

#include "JengaGameMode.h"
#include "JengaPawn.h"
#include "JengaPlayerController.h"
#include "JengaHUD.h"

#include "Engine/World.h"
#include <EngineGlobals.h>
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"


static const FName JENGA_BLOCK_TAG = "JengaBlock";
static const float BLOCKS_MAX_RANDOM_OFFSET = 0.8f;


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

   // Save the initial blocks configuration
   blocksMemory.Add(getActualTowerConfiguration());

   // Create a new tower
   NewTower();
}

///////////////////////////////////////////////////////////////////////////
// Resets the blocks position to the default one
void AJengaGameMode::NewTower()
{
   applyTowerConfiguration(blocksMemory[0]);

   // Apply a little randomness to blocks' positions (this stops the tower's jelly effect!)
   TArray<AActor*> jengaBlocks;
   UGameplayStatics::GetAllActorsWithTag(GetWorld(), JENGA_BLOCK_TAG, jengaBlocks);
   for (const auto& jengaBlock : jengaBlocks)
   {
      FTransform trx = jengaBlock->GetTransform();
      trx.SetLocation(trx.GetLocation() + FVector(
         FMath::RandRange(-BLOCKS_MAX_RANDOM_OFFSET, BLOCKS_MAX_RANDOM_OFFSET),
         FMath::RandRange(-BLOCKS_MAX_RANDOM_OFFSET, BLOCKS_MAX_RANDOM_OFFSET),
         FMath::RandRange(-BLOCKS_MAX_RANDOM_OFFSET, BLOCKS_MAX_RANDOM_OFFSET)
      ));
      jengaBlock->SetActorTransform(trx);
   }
}

///////////////////////////////////////////////////////////////////////////
// Returns the actual tower configuration
AJengaGameMode::TowerConfiguration AJengaGameMode::getActualTowerConfiguration()
{
   TArray<AActor*> jengaBlocks;
   UGameplayStatics::GetAllActorsWithTag(GetWorld(), JENGA_BLOCK_TAG, jengaBlocks);

   TowerConfiguration towerConfiguration;
   for (const auto& jengaBlock : jengaBlocks)
      towerConfiguration.Add(jengaBlock->GetTransform());

   return towerConfiguration;
}

///////////////////////////////////////////////////////////////////////////
// Applies a given tower configuration
void AJengaGameMode::applyTowerConfiguration(AJengaGameMode::TowerConfiguration towerConf)
{
   TArray<AActor*> jengaBlocks;
   UGameplayStatics::GetAllActorsWithTag(GetWorld(), JENGA_BLOCK_TAG, jengaBlocks);
   for (int i = 0; i < jengaBlocks.Num(); i++)
   {
      // Apply the transform
      jengaBlocks[i]->SetActorTransform(towerConf[i]);

      TArray<UStaticMeshComponent*> staticMeshes;
      jengaBlocks[i]->GetComponents<UStaticMeshComponent>(staticMeshes);
      for (UStaticMeshComponent* staticMesh : staticMeshes)
      {
         // Little trick to stop blocks' momentum
         staticMesh->SetSimulatePhysics(false);
         staticMesh->SetSimulatePhysics(true);
      }
   }

}

