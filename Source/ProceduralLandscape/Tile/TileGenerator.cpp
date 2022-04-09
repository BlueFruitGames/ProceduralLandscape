// Fill out your copyright notice in the Description page of Project Settings.


#include "TileGenerator.h"


// Sets default values
ATileGenerator::ATileGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ATileGenerator::OnConstruction(const FTransform& Transform) {
	if (bReloadInEditor) {
		GenerateTiles();
	}
}

void ATileGenerator::GenerateTiles()
{
	DeleteAllTiles();
	FTileGenerationParams TileGenerationParams;
	TileGenerationParams.TileSize = TileSize;
	TileGenerationParams.TileResolution = TileResolution;
	TileGenerationParams.MajorNoiseOffset = MajorNoiseOffset;
	TileGenerationParams.MajorNoiseScale = MajorNoiseScale;
	TileGenerationParams.MajorNoiseStrength = MajorNoiseStrength;
	TileGenerationParams.MinorNoiseOffset = MinorNoiseOffset;
	TileGenerationParams.MinorNoiseScale = MinorNoiseScale;
	TileGenerationParams.MinorNoiseStrength = MinorNoiseStrength;

	for (int Row = int(CenterTileIndex.X) - DrawDistance; Row <= int(CenterTileIndex.X) + DrawDistance; ++Row) {
		for (int Column = int(CenterTileIndex.Y) - DrawDistance; Column <= int(CenterTileIndex.Y) + DrawDistance; ++Column) {
			FTileIndex CurrentTileIndex(Row, Column);
			TileGenerationParams.TileIndex = CurrentTileIndex;
			FVector TileLocation(CurrentTileIndex.X * TileSize, CurrentTileIndex.Y * TileSize, 0);
			FActorSpawnParameters SpawnParams;
			AProceduralTile* CurrentTile = GetWorld()->SpawnActor<AProceduralTile>(TileLocation, GetActorRotation(), SpawnParams);
			CurrentTile->GenerateTile(TileGenerationParams, LandscapeMaterial);
			FString TileName = FString::Printf(TEXT("TILE %d,%d"), CurrentTileIndex.X, CurrentTileIndex.Y);
			CurrentTile->SetActorLabel(TileName);
			Tiles.Add(CurrentTileIndex, CurrentTile);
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

// Called when the game starts or when spawned
void ATileGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATileGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

