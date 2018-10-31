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
static const FName JENGA_INTERACTIVITY_TAG = "Interactive";

static const int DEFAULT_NUMBER_OF_PLAYERS = 1;

static const FVector BLOCK_SIZES = FVector(75.f, 25.f, 15.f);
static const float BLOCKS_MAX_RANDOM_OFFSET = 0.8f;
static const float BLOCKS_BALANCE_SPEED_THRESHOLD = 7.f;


///////////////////////////////////////////////////////////////////////////
// Utility that finds the StaticMeshComponent of an actor
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
   this->defaultConfiguration = GetActualTowerConfiguration();

   // Register floor collision event
   TArray<AActor*> floors;
   UGameplayStatics::GetAllActorsWithTag(GetWorld(), JENGA_FLOOR_TAG, floors);
   for (const auto& floor : floors)
      getMesh(floor)->OnComponentHit.AddDynamic(this, &AJengaGameMode::OnFloorHit);

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
   this->moves = 0;
   this->nPlayers = nPlayers;
   this->holdingPickedJengaBlock = false;
   this->towerStatus = TowerStatus::BALANCED;

   // Load the first (pinpoint accurate) tower configuration and clear the old ones
   ApplyTowerConfiguration(this->defaultConfiguration);
   this->oldConfigurations.Reset();

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
   this->gameConfiguration = GetActualTowerConfiguration();

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
      SetInteractive(jengaBlock, false);

   // Enable highlight and interactivity only on the picked one!
   SetInteractive(this->pickedJengaBlock, true);
   getMesh(this->pickedJengaBlock)->SetRenderCustomDepth(true);

}

///////////////////////////////////////////////////////////////////////////
// Called every frame
void AJengaGameMode::Tick(float deltaTime)
{
   if (this->pickedJengaBlock && this->towerStatus != TowerStatus::COLLAPSED)
   {
      // Estabilish the balance status of the tower
      this->towerStatus = TowerStatus::BALANCED;
      for (const auto& jengaBlock : this->jengaBlocks)
      {
         if (jengaBlock->GetVelocity().Size() > BLOCKS_BALANCE_SPEED_THRESHOLD)
         {
            this->towerStatus = TowerStatus::MOVING;
            break;
         }
      }

      // Ok, the tower is balanced and the player has released the block...
      if (this->towerStatus == TowerStatus::BALANCED && !this->holdingPickedJengaBlock)
      {
         // Estabilish if the move is good or not!
         if (IsOnTop(this->pickedJengaBlock))
            NextRound();
      }
   }
}

///////////////////////////////////////////////////////////////////////////
// Called when the player has released the picked block
void AJengaGameMode::PickReleased(AActor* block)
{
   this->holdingPickedJengaBlock = false;
}

///////////////////////////////////////////////////////////////////////////
// Goes back to the previous round
void AJengaGameMode::Undo()
{
   if (this->turn >= 1)
   {
      this->turn-=2;
      NextRound();
   }
}

///////////////////////////////////////////////////////////////////////////
// Restores a canceled round
void AJengaGameMode::Redo()
{
   if (this->turn + 1 <= this->moves)
      NextRound();
}

///////////////////////////////////////////////////////////////////////////
// Increases the turn counter number and initializes the next round
void AJengaGameMode::NextRound()
{
   this->turn++;
   this->moves = FMath::Max(this->turn, this->moves);

   if (this->nPlayers > 1)
   {
      // Show message
      FString turnStr = "Turn " + FString::FromInt(this->turn + 1);
      FString playerStr = "Player " + FString::FromInt(CurrentPlayer() + 1);
      GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, turnStr + ": " + playerStr + " moves!");
   }

   // Do we already had this turn? (because of undos/redos)
   if (this->turn == -1)
      ApplyTowerConfiguration(this->gameConfiguration);
   else if (this->turn < this->oldConfigurations.Num())
      ApplyTowerConfiguration(this->oldConfigurations[this->turn]);
   else
      this->oldConfigurations.Add(GetActualTowerConfiguration());

   // Make sure all blocks are interactive (except the top ones!)
   for (const auto& jengaBlock : this->jengaBlocks)
      SetInteractive(jengaBlock, !IsOnTop(jengaBlock));

   // Deactivate the previously picked block (if any)
   if (this->pickedJengaBlock)
   {
      getMesh(this->pickedJengaBlock)->SetRenderCustomDepth(false);
      this->pickedJengaBlock = nullptr;
   }
}

///////////////////////////////////////////////////////////////////////////
// Game over event
void AJengaGameMode::GameOver(FString msg)
{
   GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, msg);

   // Deactivate the picked block
   SetInteractive(this->pickedJengaBlock, false);
   getMesh(this->pickedJengaBlock)->SetRenderCustomDepth(false);
}

///////////////////////////////////////////////////////////////////////////
// Returns the index of the current player (0-based)
int AJengaGameMode::CurrentPlayer()
{
   return (this->turn % this->nPlayers);
}

///////////////////////////////////////////////////////////////////////////
// Is this block on top of the tower?
bool AJengaGameMode::IsOnTop(AActor* block)
{
   // Find the height of the tower
   float highestZ = -1.0f;
   for (const auto& jengaBlock : this->jengaBlocks)
      highestZ = FMath::Max(highestZ, jengaBlock->GetTransform().GetLocation().Z);

   // Is the block on top?
   float blockZ = block->GetTransform().GetLocation().Z;
   return (highestZ - blockZ) < (BLOCK_SIZES[2] / 2.f);
}

///////////////////////////////////////////////////////////////////////////
// Sets a block interactivity
void AJengaGameMode::SetInteractive(AActor* jengaBlock, bool b)
{
   if (b)
   {
      if (!IsInteractive(jengaBlock))
         jengaBlock->Tags.Add(JENGA_INTERACTIVITY_TAG);
   }
   else
      jengaBlock->Tags.Remove(JENGA_INTERACTIVITY_TAG);
}

///////////////////////////////////////////////////////////////////////////
// Is this block interactive?
bool AJengaGameMode::IsInteractive(AActor* jengaBlock)
{
   return jengaBlock->Tags.Contains(JENGA_INTERACTIVITY_TAG);
}

///////////////////////////////////////////////////////////////////////////
// Returns the actual tower configuration
AJengaGameMode::TowerConfiguration AJengaGameMode::GetActualTowerConfiguration()
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
void AJengaGameMode::ApplyTowerConfiguration(AJengaGameMode::TowerConfiguration towerConf)
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
// Collision event on the floor
void AJengaGameMode::OnFloorHit(
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
      GameOver("Tower collapsed!");
   }
}
