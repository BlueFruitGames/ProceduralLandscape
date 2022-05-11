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

	UPROPERTY(EditAnywhere, Category = "General")
	TSubclassOf<ACharacter> PlayerClass; //The class which is used for the player character

	UPROPERTY(EditAnywhere, Category="General")
	int RandomSeed = 1;//The global RandomSeed for every landscape operation which involes random number generation

	UPROPERTY(EditAnywhere, Category = "General", meta = (UIMin = 0))
	int DrawDistance = 1;//How many layers of additional Tiles are generated

	UPROPERTY(EditAnywhere, Category = "General", meta = (UIMin = 1))
	bool bReloadInEditor;//Should the mesh be updated in the editor if changes are made to it's properties?

	UPROPERTY(EditAnywhere, Category = "General")
	bool bGenerateTrees = false;//If trees should also be generated

	UPROPERTY(EditAnywhere, Category = "General")
	bool bGenerateGrass = false;//If grass should also be generated

	UPROPERTY(EditAnywhere, Category = "General", meta = (UIMin = 1))
	int TileSize; //Width of the tile

	UPROPERTY(EditAnywhere, Category = "General", meta = (UIMin = 2, UIMax = 100))
	int TileResolution;//Count of vertices on each axis

	UPROPERTY(EditAnywhere, Category = "MajorNoise", meta = (UIMin = 0))
	int MajorNoiseStrength; //The influence of the PerlinNoise

	UPROPERTY(EditAnywhere, Category = "MajorNoise")
	FVector2D MajorNoiseOffset; //X and Y offset of the PerlinNoise

	UPROPERTY(EditAnywhere, Category = "MajorNoise")
	FVector2D MajorNoiseScale;//X and Y scale of the PerlinNoise

	UPROPERTY(EditAnywhere, Category = "MajorNoise|Randomness")
	float MajorNoiseStrengthDeviation = 0.0f; //Adjusts bounds of random number generation for the strength value

	UPROPERTY(EditAnywhere, Category = "MajorNoise|Randomness")
	float MajorNoiseOffsetDeviation = 0.0f; //Adjusts bounds of random number generation for the offset value

	UPROPERTY(EditAnywhere, Category = "MajorNoise|Randomness")
	float MajorNoiseScaleDeviation = 0.0f; //Adjusts bounds of random number generation for the scale value

	UPROPERTY(EditAnywhere, Category = "MinorNoise", meta = (UIMin = 0))
	int MinorNoiseStrength; //The influence of the PerlinNoise

	UPROPERTY(EditAnywhere, Category = "MinorNoise")
	FVector2D MinorNoiseOffset; //X and Y offset of the PerlinNoise

	UPROPERTY(EditAnywhere, Category = "MinorNoise")
	FVector2D MinorNoiseScale;//X and Y scale of the PerlinNoise

	UPROPERTY(EditAnywhere, Category = "MinorNoise|Randomness")
	float MinorNoiseStrengthDeviation = 0.0f; //Adjusts bounds of random number generation for the strength value

	UPROPERTY(EditAnywhere, Category = "MinorNoise|Randomness")
	float MinorNoiseOffsetDeviation = 0.0f; //Adjusts bounds of random number generation for the offset value

	UPROPERTY(EditAnywhere, Category = "MinorNoise|Randomness")
	float MinorNoiseScaleDeviation = 0.0f; //Adjusts bounds of random number generation for the scale value

	UPROPERTY(EditAnywhere, Category = "General")
	UMaterialInterface* LandscapeMaterial; //Material that will be applied to every tile

	UPROPERTY(EditAnywhere, meta = (UIMin = 1, UIMax = 100), Category = "TreeGeneration")
	int TreeSpawnCount = 0;//Number of trees per tile

	UPROPERTY(EditAnywhere, Category = "TreeGeneration")
	int TreeMaxTries = 100; //Maximum number of tries to generate a new location

	UPROPERTY(EditAnywhere, Category = "TreeGeneration")
	TArray<class UFoliageDataAsset*> TreeData; //Trees and their respective data for generation

	UPROPERTY(EditAnywhere, Category = "TreeGeneration")
	bool bDrawTreeDebug = false;//Number of trees per tile

	UPROPERTY(EditAnywhere, meta = (UIMin = 1, UIMax = 100), Category = "GrassGeneration")
	int GrassSpawnCount = 0;//Number of Grass per tile

	UPROPERTY(EditAnywhere, Category = "GrassGeneration")
	int GrassMaxTries = 100; //Maximum number of tries to generate a new location

	UPROPERTY(EditAnywhere, Category = "GrassGeneration")
	TArray<class UFoliageDataAsset*> GrassData; //Grass types and their respective data for generation

	/**
	 * Initializes tiles for the specified drawing distance
	 *
	 */
	UFUNCTION(CallInEditor)
	void GenerateTiles();

	/**
	 * Is called to update the tiles of the landscape
	 * 
	 * \param NewCenterIndex is the index of the tile that will be used as the new center
	 */
	void UpdateTiles(FTileIndex NewCenterIndex);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;	

private:

	UPROPERTY(EditAnywhere)
	FTileIndex CenterTileIndex;

	TMap<FTileIndex, AProceduralTile*> Tiles; //A tiles that currently exist

	FTileGenerationParams TileGenerationParams;

	/**
	 * .
	 * Delets all tiles in the Tiles-Map
	 */
	void DeleteAllTiles();

	/**
	 * Sets up the parameters needed for the tile generation.
	 * 
	 * \return the parameters that will be used for the tile generation
	 */
	FTileGenerationParams SetupTileGenerationParams();
};
