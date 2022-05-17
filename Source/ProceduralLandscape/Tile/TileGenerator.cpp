// Fill out your copyright notice in the Description page of Project Settings.


#include "TileGenerator.h"

#include "Foliage/FoliageGenerationComponent.h"
#include "Math/RandomStream.h"
#include "HAL/RunnableThread.h"


// Sets default values
ATileGenerator::ATileGenerator()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATileGenerator::OnConstruction(const FTransform& Transform) {
	if (bReloadInEditor) {
		GenerateTiles();
	}
}

void ATileGenerator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (CurrentFoliageThread && RunningThread) {
		RunningThread->Kill(true);
		RunningThread->WaitForCompletion();
		delete CurrentFoliageThread;
	}
	
	FFoliageGenerationThread* CurrentThread;
	while (FoliageGenerationThreads.Dequeue(CurrentThread)) {
		delete CurrentThread;
	}
}

// Called when the game starts or when spawned
void ATileGenerator::BeginPlay()
{
	Super::BeginPlay();
	LastTileIndex = FTileIndex(0, 0);
	LastGeneratedFoliageInfos = TArray<FGeneratedFoliageInfo>();
	CenterTileIndex.X = 0;
	CenterTileIndex.Y = 0;
	GenerateTiles();
}



void ATileGenerator::Tick(float DeltaSeconds)
{
	CurrentUpdateTime += DeltaSeconds;

	if (CurrentUpdateTime >= FoliageUpdateCooldown && FoliageComponentsToUpdate.Num() > 0) {
		UFoliageGenerationComponent* CurrentFoliageComponent = FoliageComponentsToUpdate[0];
		FoliageComponentsToUpdate.RemoveAt(0);
		if (!CurrentFoliageComponent->UpdateFoliage()) {
			FoliageComponentsToUpdate.Add(CurrentFoliageComponent);
		}
		CurrentUpdateTime = 0;
	}

	if (!bIsFoliageThreadFinished) return;

	if (RunningThread && CurrentFoliageThread) {
		RunningThread->Kill(false);
		RunningThread->WaitForCompletion();
		UFoliageGenerationComponent* CurrentFoliageGenerationComponent = CurrentFoliageThread->GetFoliageGenerationComponent();
		LastTileIndex = CurrentFoliageThread->GetTileIndex();
		LastGeneratedFoliageInfos =	CurrentFoliageThread->GetFoliageInfos();
		if (FoliageComponentsToUpdate.Find(CurrentFoliageGenerationComponent) < 0) {
			FoliageComponentsToUpdate.Add(CurrentFoliageGenerationComponent);
		}
		delete CurrentFoliageThread;
		RunningThread = nullptr;
	}
	if (FoliageGenerationThreads.Dequeue(CurrentFoliageThread)) {
			if (LastTileIndex == CurrentFoliageThread->GetTileIndex()) {
				CurrentFoliageThread->SetFoliageInfos(LastGeneratedFoliageInfos);
			}
			else {
				CurrentFoliageThread->SetFoliageInfos(TArray<FGeneratedFoliageInfo>());
			}
			bIsFoliageThreadFinished = false;
			RunningThread = FRunnableThread::Create(CurrentFoliageThread, TEXT("FoliageGeneration"));
	}
}


FTileGenerationParams ATileGenerator::SetupTileGenerationParams()
{
	FRandomStream RandomStream(RandomSeed);
	TileGenerationParams.TileSize = TileSize;
	TileGenerationParams.TileResolution = TileResolution;

	TileGenerationParams.MajorNoiseStrength = RandomStream.FRandRange(MajorNoiseStrength - MajorNoiseStrengthDeviation, MajorNoiseStrength + MajorNoiseStrengthDeviation);
	float MajorNoiseOffsetX = RandomStream.FRandRange(MajorNoiseOffset.X - MajorNoiseOffsetDeviation, MajorNoiseOffset.X + MajorNoiseOffsetDeviation);
	float MajorNoiseOffsetY = RandomStream.FRandRange(MajorNoiseOffset.Y - MajorNoiseOffsetDeviation, MajorNoiseOffset.Y + MajorNoiseOffsetDeviation);
	TileGenerationParams.MinorNoiseOffset = FVector2D(MajorNoiseOffsetX, MajorNoiseOffsetY);
	float MajorNoiseScaleX = RandomStream.FRandRange(MajorNoiseScale.X - MajorNoiseScaleDeviation, MajorNoiseScale.X + MajorNoiseScaleDeviation);
	float MajorNoiseScaleY = RandomStream.FRandRange(MajorNoiseScale.Y - MajorNoiseScaleDeviation, MajorNoiseScale.Y + MajorNoiseScaleDeviation);
	TileGenerationParams.MajorNoiseScale = FVector2D(MajorNoiseScaleX, MajorNoiseScaleY);

	TileGenerationParams.MinorNoiseStrength = RandomStream.FRandRange(MinorNoiseStrength - MinorNoiseStrengthDeviation, MinorNoiseStrength + MinorNoiseStrengthDeviation);
	float MinorNoiseOffsetX = RandomStream.FRandRange(MinorNoiseOffset.X - MinorNoiseOffsetDeviation, MinorNoiseOffset.X + MinorNoiseOffsetDeviation);
	float MinorNoiseOffsetY = RandomStream.FRandRange(MinorNoiseOffset.Y - MinorNoiseOffsetDeviation, MinorNoiseOffset.Y + MinorNoiseOffsetDeviation);
	TileGenerationParams.MinorNoiseOffset = FVector2D(MinorNoiseOffsetX, MinorNoiseOffsetY);
	float MinorNoiseScaleX = RandomStream.FRandRange(MinorNoiseScale.X - MinorNoiseScaleDeviation, MinorNoiseScale.X + MinorNoiseScaleDeviation);
	float MinorNoiseScaleY = RandomStream.FRandRange(MinorNoiseScale.Y - MinorNoiseScaleDeviation, MinorNoiseScale.Y + MinorNoiseScaleDeviation);
	TileGenerationParams.MinorNoiseScale = FVector2D(MinorNoiseScaleX, MinorNoiseScaleY);

	return TileGenerationParams;
}


void ATileGenerator::GenerateTiles()
{
	DeleteAllTiles();
	SetupTileGenerationParams();

	for (int Row = CenterTileIndex.X - DrawDistance; Row <= CenterTileIndex.X + DrawDistance; ++Row) {
		for (int Column = CenterTileIndex.Y - DrawDistance; Column <= CenterTileIndex.Y + DrawDistance; ++Column) {
			FTileIndex CurrentTileIndex(Row, Column);
			TileGenerationParams.TileIndex = CurrentTileIndex;
			FVector TileLocation(CurrentTileIndex.X * TileSize, CurrentTileIndex.Y * TileSize, 0);
			FActorSpawnParameters SpawnParams;
			AProceduralTile* CurrentTile = GetWorld()->SpawnActor<AProceduralTile>(TileLocation, GetActorRotation(), SpawnParams);
			CurrentTile->Setup(this, PlayerClass, LandscapeMaterial);
			CurrentTile->GenerateTile(TileGenerationParams);
			FString TileName = FString::Printf(TEXT("TILE %d,%d"), CurrentTileIndex.X, CurrentTileIndex.Y);
			CurrentTile->SetActorLabel(TileName);

			TArray <FGeneratedFoliageInfo> GeneratedFoliage;
			
			if (bGenerateTrees) {
				CurrentTile->GetTreeGenerationComponent()->SetupFoliageGeneration(TreeSpawnCount, TreeMaxTries, TreeMaxSpawnPerTick, TreeData, RandomSeed, true);
				CurrentTile->GetTreeGenerationComponent()->GenerateFoliage(true, CurrentTileIndex, TileSize, CurrentTile->GetMaxZPosition(), CurrentTile->GetMinZPosition(), GeneratedFoliage);
				
			}

			if (bGenerateBranches) {
				CurrentTile->GetBranchGenerationComponent()->SetupFoliageGeneration(BranchSpawnCount, BranchMaxTries, BranchMaxSpawnPerTick, BranchData, RandomSeed, false);
				CurrentTile->GetBranchGenerationComponent()->GenerateFoliage(true, CurrentTileIndex, TileSize, CurrentTile->GetMaxZPosition(), CurrentTile->GetMinZPosition(), GeneratedFoliage);
			}

			if (bGenerateBushes) {
				CurrentTile->GetBushGenerationComponent()->SetupFoliageGeneration(BushSpawnCount, BushMaxTries, BushMaxSpawnPerTick, BushData, RandomSeed, false);
				CurrentTile->GetBushGenerationComponent()->GenerateFoliage(true, CurrentTileIndex, TileSize, CurrentTile->GetMaxZPosition(), CurrentTile->GetMinZPosition(), GeneratedFoliage);
			}

			if (bGenerateGrass) {
				CurrentTile->GetGrassGenerationComponent()->SetupFoliageGeneration(GrassSpawnCount, GrassMaxTries, GrassMaxSpawnPerTick, GrassData, RandomSeed, false);
				CurrentTile->GetGrassGenerationComponent()->GenerateFoliage(true, CurrentTileIndex, TileSize, CurrentTile->GetMaxZPosition(), CurrentTile->GetMinZPosition(), GeneratedFoliage);
			}
			Tiles.Add(CurrentTileIndex, CurrentTile);
		}
	}
	
}

void ATileGenerator::UpdateTiles(FTileIndex NewCenterIndex)
{
	//FoliageGenerationThreads.Empty();
	CenterTileIndex = NewCenterIndex;
	TArray<FTileIndex> UpdateableTileIndices;
	TArray<FTileIndex> IndicesToGenerate;
	Tiles.GetKeys(UpdateableTileIndices);

	for (int Row = CenterTileIndex.X - DrawDistance; Row <= CenterTileIndex.X + DrawDistance; ++Row) {
		for (int Column = CenterTileIndex.Y - DrawDistance; Column <= CenterTileIndex.Y + DrawDistance; ++Column) {
			FTileIndex CurrentTileIndex(Row, Column);
			if (Tiles.Find(CurrentTileIndex)) {
				UpdateableTileIndices.Remove(CurrentTileIndex);
			}
			else {
				IndicesToGenerate.Add(CurrentTileIndex);
			}
		}
	}
	for (FTileIndex& IndexToGenerate : IndicesToGenerate) {
		FTileIndex IndexToUpdate = UpdateableTileIndices[0];
		UpdateableTileIndices.RemoveAt(0);
		TileGenerationParams.TileIndex = IndexToGenerate;
		AProceduralTile* TileToUpdate = *Tiles.Find(IndexToUpdate);
		FVector TileLocation(IndexToGenerate.X * TileSize, IndexToGenerate.Y * TileSize, 0);
		TileToUpdate->SetActorLocation(TileLocation);
		TileToUpdate->GenerateTile(TileGenerationParams, true);
		Tiles.Add(IndexToGenerate, TileToUpdate);
		Tiles.Remove(IndexToUpdate);

		

		TArray<FGeneratedFoliageInfo> GeneratedFoliageInfos;
		if (bGenerateTrees) {
			FFoliageGenerationThread* NewThread = new FFoliageGenerationThread(TileToUpdate->GetTreeGenerationComponent(), this, IndexToGenerate, TileSize, TileToUpdate->GetMaxZPosition(), TileToUpdate->GetMinZPosition());
			FoliageGenerationThreads.Enqueue(NewThread);
		}

		if (bGenerateBranches) {
			FFoliageGenerationThread* NewThread = new FFoliageGenerationThread(TileToUpdate->GetBranchGenerationComponent(), this, IndexToGenerate, TileSize, TileToUpdate->GetMaxZPosition(), TileToUpdate->GetMinZPosition());
			FoliageGenerationThreads.Enqueue(NewThread);
		}

		if (bGenerateBushes) {
			FFoliageGenerationThread* NewThread = new FFoliageGenerationThread(TileToUpdate->GetBushGenerationComponent(), this, IndexToGenerate, TileSize, TileToUpdate->GetMaxZPosition(), TileToUpdate->GetMinZPosition());
			FoliageGenerationThreads.Enqueue(NewThread);
		}

		if (bGenerateGrass) {
			FFoliageGenerationThread* NewThread = new FFoliageGenerationThread(TileToUpdate->GetGrassGenerationComponent(), this, IndexToGenerate, TileSize, TileToUpdate->GetMaxZPosition(), TileToUpdate->GetMinZPosition());
			FoliageGenerationThreads.Enqueue(NewThread);
		}
	}
}
void ATileGenerator::DeleteAllTiles() {
	TArray<AProceduralTile*> Values;
	Tiles.GenerateValueArray(Values);
	for (AProceduralTile* Tile : Values) {
		Tile->Destroy();
	}
	Tiles.Empty();
}

