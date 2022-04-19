// Fill out your copyright notice in the Description page of Project Settings.


#include "TileGenerator.h"

#include "../TreeGeneration/TreeGenerator.h"

// Sets default values
ATileGenerator::ATileGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ATileGenerator::OnConstruction(const FTransform& Transform) {
	if (bReloadInEditor) {
		CenterTileIndex.X = 0;
		CenterTileIndex.Y = 0;
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

FTileGenerationParams ATileGenerator::SetupTileGenerationParams()
{
	FTileGenerationParams TileGenerationParams;
	TileGenerationParams.TileSize = TileSize;
	TileGenerationParams.TileResolution = TileResolution;
	TileGenerationParams.MajorNoiseOffset = MajorNoiseOffset;
	TileGenerationParams.MajorNoiseScale = MajorNoiseScale;
	TileGenerationParams.MajorNoiseStrength = MajorNoiseStrength;
	TileGenerationParams.MinorNoiseOffset = MinorNoiseOffset;
	TileGenerationParams.MinorNoiseScale = MinorNoiseScale;
	TileGenerationParams.MinorNoiseStrength = MinorNoiseStrength;
	return TileGenerationParams;
}


void ATileGenerator::GenerateTiles()
{
	DeleteAllTiles();
	FTileGenerationParams TileGenerationParams = SetupTileGenerationParams();

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
			Tiles.Add(CurrentTileIndex, CurrentTile);
			if (TreeGenerator) TreeGenerator->GenerateTrees(CurrentTileIndex, TileSize);
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

	FTileGenerationParams TileGenerationParams = SetupTileGenerationParams();
	for (FTileIndex IndexToGenerate : IndicesToGenerate) {
		FTileIndex IndexToUpdate = UpdateableTileIndices[0];
		UpdateableTileIndices.RemoveAt(0);
		TileGenerationParams.TileIndex = IndexToGenerate;
		AProceduralTile* TileToUpdate = *Tiles.Find(IndexToUpdate);
		FVector TileLocation(IndexToGenerate.X * TileSize, IndexToGenerate.Y * TileSize, 0);
		TileToUpdate->SetActorLocation(TileLocation);
		TileToUpdate->GenerateTile(TileGenerationParams, true);
		Tiles.Add(IndexToGenerate, TileToUpdate);
		Tiles.Remove(IndexToUpdate);
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

