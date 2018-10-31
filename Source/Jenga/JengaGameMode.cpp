// Fill out your copyright notice in the Description page of Project Settings.

#include "JengaGameMode.h"
#include "JengaPawn.h"
#include "JengaPlayerController.h"
#include "JengaHUD.h"

#include "Engine/World.h"
#include "EngineGlobals.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "Runtime/Engine/Public/TimerManager.h"


static const FName JENGA_BLOCK_TAG = "JengaBlock";
static const FName JENGA_FLOOR_TAG = "JengaFloor";
static const int DEFAULT_NUMBER_OF_PLAYERS = 1;
static const float BLOCKS_MAX_RANDOM_OFFSET = 0.8f;
static const float BLOCKS_BALANCE_SPEED_THRESHOLD = 7.f;
static const float FLOOR_THRESHOLD = 1.f;


inline UStaticMeshComponent* getMesh(AActor* actor)
{
   if (!actor)
      return nullptr;

   TArray<UStaticMeshComponent*> staticMeshes;
   actor->GetComponents<UStaticMeshComponent>(staticMeshes);
   return staticMeshes.Num() > 0 ? staticMeshes[0] : nullptr;
}


///////////////////////////////////////////////////////////////////////////
// Constructor
AJengaGameMode::AJengaGameMode()
{
   DefaultPawnClass = AJengaPawn::StaticClass();
   PlayerControllerClass = AJengaPlayerController::StaticClass();
   HUDClass = AJengaHUD::StaticClass();

   // Enable tick
   PrimaryActorTick.bStartWithTickEnabled = true;
   PrimaryActorTick.bCanEverTick = true;
}

///////////////////////////////////////////////////////////////////////////
// Called when the game starts or when spawned
void AJengaGameMode::BeginPlay()
{
   GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Welcome to Jenga!"));

   // Get the array of blocks
   UGameplayStatics::GetAllActorsWithTag(GetWorld(), JENGA_BLOCK_TAG, this->jengaBlocks);

   // Save the initial blocks configuration
   blocksMemory.Add(getActualTowerConfiguration());

   // Register floor collision event
   TArray<AActor*> floors;
   UGameplayStatics::GetAllActorsWithTag(GetWorld(), JENGA_FLOOR_TAG, floors);
   for (const auto& floor : floors)
      getMesh(floor)->OnComponentHit.AddDynamic(this, &AJengaGameMode::onFloorHit);

   // Start a new game with the default number of players
   NewGame(DEFAULT_NUMBER_OF_PLAYERS);
}

///////////////////////////////////////////////////////////////////////////
// Resets the blocks positions and starts a new game
void AJengaGameMode::NewGame(int nPlayers)
{
   // Init game parameters
   this->pickedJengaBlock = nullptr;
   this->turn = -1;
   this->nPlayers = nPlayers;
   this->holdingPickedJengaBlock = false;
   this->towerStatus = TowerStatus::BALANCED;

   // Load the first (pinpoint accurate) tower configuration
   applyTowerConfiguration(blocksMemory[0]);

   // Save those blocks touching the floor (they should be 3)
   this->jengaBlocksOnFloor.Reset();
   for (const auto& jengaBlock : this->jengaBlocks)
      if (jengaBlock->GetTransform().GetLocation().Z == 0.0f)
         this->jengaBlocksOnFloor.Add(jengaBlock);

   // Apply a little randomness to blocks' positions (this stops the tower's jelly effect!)
   for (const auto& jengaBlock : this->jengaBlocks)
   {
      FTransform trx = jengaBlock->GetTransform();
      trx.SetLocation(trx.GetLocation() + FVector(
         FMath::RandRange(-BLOCKS_MAX_RANDOM_OFFSET, BLOCKS_MAX_RANDOM_OFFSET),
         FMath::RandRange(-BLOCKS_MAX_RANDOM_OFFSET, BLOCKS_MAX_RANDOM_OFFSET),
         FMath::RandRange(-BLOCKS_MAX_RANDOM_OFFSET, BLOCKS_MAX_RANDOM_OFFSET)
      ));
      jengaBlock->SetActorTransform(trx);
   }

   // Make sure no block is highlighted
   for (const auto& jengaBlock : this->jengaBlocks)
      getMesh(jengaBlock)->SetRenderCustomDepth(false);

   // Make sure all blocks are interactive
   for (const auto& jengaBlock : this->jengaBlocks)
      jengaBlock->Tags.Add("Interactive");

   // Game start message
   const FString noOfPlayersString = FString::FromInt(this->nPlayers);
   GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "Starting a game with " + noOfPlayersString + " player(s)!");

   // First player can move!
   NextRound();
}

///////////////////////////////////////////////////////////////////////////
// Called to lock current's player block to this one
void AJengaGameMode::NewPick(AActor* block)
{
#if defined(UE_EDITOR) || defined(UE_BUILD_DEBUG)
   GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Picking ") + block->GetName());
#endif

   // Save the picked block
   this->pickedJengaBlock = block;
   this->holdingPickedJengaBlock = true;
   this->jengaBlocksOnFloor.Remove(this->pickedJengaBlock);

   // Block interactivity on all blocks
   for (const auto& jengaBlock : this->jengaBlocks)
      jengaBlock->Tags.Remove("Interactive");

   // Enable highlight and interactivity only on the picked one!
   this->pickedJengaBlock->Tags.Add("Interactive");
   getMesh(this->pickedJengaBlock)->SetRenderCustomDepth(true);

}

///////////////////////////////////////////////////////////////////////////
// Called every frame
void AJengaGameMode::Tick(float deltaTime)
{
   if (this->pickedJengaBlock && this->towerStatus != TowerStatus::COLLAPSED)
   {
      this->towerStatus = TowerStatus::BALANCED;
      for (const auto& jengaBlock : this->jengaBlocks)
      {
         if (jengaBlock->GetVelocity().Size() > BLOCKS_BALANCE_SPEED_THRESHOLD)
         {
            this->towerStatus = TowerStatus::MOVING;
            break;
         }
      }

      if (this->towerStatus == TowerStatus::BALANCED && !this->holdingPickedJengaBlock)
      {
         // Deactivate the picked block
         getMesh(this->pickedJengaBlock)->SetRenderCustomDepth(false);
         this->pickedJengaBlock = nullptr;

         // Enable interactivity on all blocks
         for (const auto& jengaBlock : this->jengaBlocks)
            jengaBlock->Tags.Add("Interactive");

         NextRound();
      }
   }
}

///////////////////////////////////////////////////////////////////////////
// 
void AJengaGameMode::PickReleased(AActor* block)
{
   this->holdingPickedJengaBlock = false;
}

///////////////////////////////////////////////////////////////////////////
// Increases the turn counter number and initializes the next round
void AJengaGameMode::NextRound()
{
   this->turn++;
   FString player = "Player " + FString::FromInt(CurrentPlayer() + 1);
   GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, player + "'s turn!");
}

///////////////////////////////////////////////////////////////////////////
// Returns the index of the current player (0-based)
int AJengaGameMode::CurrentPlayer()
{
   return (this->turn % this->nPlayers);
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

      // Little trick to stop blocks' momentum
      UStaticMeshComponent* staticMesh = getMesh(jengaBlocks[i]);
      staticMesh->SetSimulatePhysics(false);
      staticMesh->SetSimulatePhysics(true);
   }
}


///////////////////////////////////////////////////////////////////////////
// 
void AJengaGameMode::onFloorHit(
   UPrimitiveComponent* hitComponent,
   AActor* otherActor,
   UPrimitiveComponent* otherComponent,
   FVector normalImpulse,
   const FHitResult& hit)
{
   // Ignore blocks already on floor
   if (jengaBlocksOnFloor.Contains(otherActor) || (otherActor == pickedJengaBlock && holdingPickedJengaBlock))
      return;
   else if (this->towerStatus != TowerStatus::COLLAPSED)
   {
      this->towerStatus = TowerStatus::COLLAPSED;
      GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Tower collapsed!");

      this->pickedJengaBlock->Tags.Remove("Interactive");
      getMesh(this->pickedJengaBlock)->SetRenderCustomDepth(false);
   }
}
