// Fill out your copyright notice in the Description page of Project Settings.


#include "TileGenerator.h"

#include "Foliage/FoliageGenerationComponent.h"
#include "Math/RandomStream.h"

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

// Called when the game starts or when spawned
void ATileGenerator::BeginPlay()
{
	Super::BeginPlay();
	CenterTileIndex.X = 0;
	CenterTileIndex.Y = 0;
	GenerateTiles();
}



void ATileGenerator::Tick(float DeltaSeconds)
{
	/*TSet<FTileIndex> TreeKeys;
	TreesToSpawn.GetKeys(TreeKeys);
	if (TreeKeys.Num() > 0) {
		UFoliageGenerationComponent* TreeGenerationComponent = *TreesToSpawn.Find(TreeKeys.Array()[0]);
		if (!TreeGenerationComponent->SpawnSingleBatch()) {
			TreesToSpawn.Remove(TreeKeys.Array()[0]);
		}
	}

	TSet<FTileIndex> GrassKeys;
	GrassToSpawn.GetKeys(GrassKeys);
	if (GrassKeys.Num() > 0) {
		UFoliageGenerationComponent* GrassGenerationComponent = *GrassToSpawn.Find(GrassKeys.Array()[0]);
		if (!GrassGenerationComponent->SpawnSingleBatch()) {
			GrassToSpawn.Remove(GrassKeys.Array()[0]);
		}
	}
	*/
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
			TArray<FGeneratedFoliageInfo> GeneratedFoliageInfos;
			if (bGenerateTrees) {
				CurrentTile->GetTreeGenerationComponent()->SetupFoliageGeneration(TreeSpawnCount, TreeMaxTries, TreeMaxSpawnPerTick, true, TreeData, RandomSeed, true);
				CurrentTile->GetTreeGenerationComponent()->GenerateFoliage(CurrentTileIndex, TileSize, CurrentTile->GetMaxZPosition() + 10, CurrentTile->GetMinZPosition() - 10, GeneratedFoliageInfos, bDrawTreeDebug);
			}

			if (bGenerateBranches) {
				CurrentTile->GetBranchGenerationComponent()->SetupFoliageGeneration(BranchSpawnCount, BranchMaxTries, BranchMaxSpawnPerTick, true, BranchData, RandomSeed, false);
				CurrentTile->GetBranchGenerationComponent()->GenerateFoliage(CurrentTileIndex, TileSize, CurrentTile->GetMaxZPosition() + 10, CurrentTile->GetMinZPosition() - 10, GeneratedFoliageInfos);
			}

			if (bGenerateBushes) {
				CurrentTile->GetBushGenerationComponent()->SetupFoliageGeneration(BushSpawnCount, BushMaxTries, BushMaxSpawnPerTick, true, BushData, RandomSeed, false);
				CurrentTile->GetBushGenerationComponent()->GenerateFoliage(CurrentTileIndex, TileSize, CurrentTile->GetMaxZPosition() + 10, CurrentTile->GetMinZPosition() - 10, GeneratedFoliageInfos);
			}

			if (bGenerateGrass) {
				CurrentTile->GetGrassGenerationComponent()->SetupFoliageGeneration(GrassSpawnCount, GrassMaxTries, GrassMaxSpawnPerTick, true, GrassData, RandomSeed, false);
				CurrentTile->GetGrassGenerationComponent()->GenerateFoliage(CurrentTileIndex, TileSize, CurrentTile->GetMaxZPosition() + 10, CurrentTile->GetMinZPosition() -10,  GeneratedFoliageInfos);
			}
			Tiles.Add(CurrentTileIndex, CurrentTile);
		}
	}
	
}

void ATileGenerator::UpdateTiles(FTileIndex NewCenterIndex)
{
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
			TileToUpdate->GetTreeGenerationComponent()->GenerateFoliage(IndexToGenerate, TileSize, TileToUpdate->GetMaxZPosition(), TileToUpdate->GetMinZPosition(), GeneratedFoliageInfos, bDrawTreeDebug);
		}

		if (bGenerateBranches) {
			TileToUpdate->GetBranchGenerationComponent()->GenerateFoliage(IndexToGenerate, TileSize, TileToUpdate->GetMaxZPosition(), TileToUpdate->GetMinZPosition(), GeneratedFoliageInfos);
		}

		if (bGenerateBushes) {
			TileToUpdate->GetBushGenerationComponent()->GenerateFoliage(IndexToGenerate, TileSize, TileToUpdate->GetMaxZPosition(), TileToUpdate->GetMinZPosition(), GeneratedFoliageInfos);
		}

		if (bGenerateGrass) {
			TileToUpdate->GetGrassGenerationComponent()->GenerateFoliage(IndexToGenerate, TileSize, TileToUpdate->GetMaxZPosition(), TileToUpdate->GetMinZPosition(), GeneratedFoliageInfos);
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

