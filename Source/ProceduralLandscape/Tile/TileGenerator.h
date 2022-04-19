// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralTile.h"

#include "TileGenerator.generated.h"

UCLASS()
class PROCEDURALLANDSCAPE_API ATileGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATileGenerator();

	UPROPERTY(EditAnywhere)
	TSubclassOf<ACharacter> PlayerClass;

	UPROPERTY(EditAnywhere, meta = (UIMin = 0))
	int DrawDistance = 1;

	UPROPERTY(EditAnywhere, meta = (UIMin = 1))
	bool bReloadInEditor;//Should the mesh be updated in the editor if changes are made to it's properties?

	UPROPERTY(EditAnywhere, meta = (UIMin = 1))
	int TileSize; //Width of the tile

	UPROPERTY(EditAnywhere, meta = (UIMin = 2, UIMax = 100))
	int TileResolution;//Count of vertices on each axis

	UPROPERTY(EditAnywhere, meta = (UIMin = 0))
	int MajorNoiseStrength; //The influence of the PerlinNoise

	UPROPERTY(EditAnywhere)
	FVector2D MajorNoiseOffset; //X and Y offset of the PerlinNoise

	UPROPERTY(EditAnywhere)
	FVector2D MajorNoiseScale;//X and Y scale of the PerlinNoise

	UPROPERTY(EditAnywhere, meta = (UIMin = 0))
	int MinorNoiseStrength; //The influence of the PerlinNoise

	UPROPERTY(EditAnywhere)
	FVector2D MinorNoiseOffset; //X and Y offset of the PerlinNoise

	UPROPERTY(EditAnywhere)
	FVector2D MinorNoiseScale;//X and Y scale of the PerlinNoise

	UPROPERTY(EditAnywhere)
	UMaterialInterface* LandscapeMaterial;

	UPROPERTY(EditAnywhere)
	class ATreeGenerator* TreeGenerator;

	//Generates new tiles for the specified drawing distance
	UFUNCTION(CallInEditor)
	void GenerateTiles();

	//Is called to update the tiles of the landscape
	void UpdateTiles(FTileIndex NewCenterIndex);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;	

private:

	FTileIndex CenterTileIndex;

	TMap<FTileIndex, AProceduralTile*> Tiles;

	//Delets all tiles in Tiles map
	void DeleteAllTiles();

	//Sets up a TileGenerationParams instant
	FTileGenerationParams SetupTileGenerationParams();
};
