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

	UPROPERTY(EditAnywhere, Category="General")
	int RandomSeed = 1;//The global RandomSeed for every landscape operation which involes random number generation

	UPROPERTY(EditAnywhere, Category = "General", meta = (UIMin = 0))
	int DrawDistance = 1;//How many layers of additional Tiles are generated

	UPROPERTY(EditAnywhere, Category = "General", meta = (UIMin = 1))
	bool bReloadInEditor;//Should the mesh be updated in the editor if changes are made to it's properties?

	UPROPERTY(EditAnywhere, Category = "General")
	bool bGenerateTrees = true;//If trees should also be generated

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

	UPROPERTY(EditAnywhere, Category = "General")
	class ATreeGenerator* TreeGenerator; //The current instance of the ATreeGenerator in the scene

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
