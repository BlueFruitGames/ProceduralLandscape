// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralTile.h"
#include "Foliage/FoliageGenerationThread.h"

#include "TileGenerator.generated.h"

UCLASS()
class PROCEDURALLANDSCAPE_API ATileGenerator : public AActor
{
	GENERATED_BODY()

	friend class FFoliageGenerationThread;
	
public:	
	// Sets default values for this actor's properties
	ATileGenerator();

	//The class which is used for the player character
	UPROPERTY(EditAnywhere, Category = "General")
	TSubclassOf<ACharacter> PlayerClass; 

	//The global RandomSeed for every landscape operation which involes random number generation
	UPROPERTY(EditAnywhere, Category="General")
	int RandomSeed = 1;

	//How many layers of additional Tiles are generated
	UPROPERTY(EditAnywhere, Category = "General", meta = (UIMin = 0))
	int DrawDistance = 1;

	//Width of the tile
	UPROPERTY(EditAnywhere, Category = "General", meta = (UIMin = 1))
	int TileSize;

	//Count of vertices on each axis
	UPROPERTY(EditAnywhere, Category = "General", meta = (UIMin = 2, UIMax = 100))
	int TileResolution;

	//Material that will be applied to each tile
	UPROPERTY(EditAnywhere, Category = "General")
	UMaterialInterface* LandscapeMaterial;

	//Should the mesh be updated in the editor if changes are made to it's properties?
	UPROPERTY(EditAnywhere, Category = "General", meta = (UIMin = 1))
	bool bReloadInEditor;

	//Influence of the PerlinNoise
	UPROPERTY(EditAnywhere, Category = "MajorNoise", meta = (UIMin = 0))
	int MajorNoiseStrength; 

	//X and Y offset of the PerlinNoise
	UPROPERTY(EditAnywhere, Category = "MajorNoise")
	FVector2D MajorNoiseOffset;

	//X and Y scale of the PerlinNoise
	UPROPERTY(EditAnywhere, Category = "MajorNoise")
	FVector2D MajorNoiseScale;

	//Randomness for the strength value
	UPROPERTY(EditAnywhere, Category = "MajorNoise|Randomness")
	float MajorNoiseStrengthDeviation = 0.0f;

	//Randomness for the offset value
	UPROPERTY(EditAnywhere, Category = "MajorNoise|Randomness")
	float MajorNoiseOffsetDeviation = 0.0f;

	//Randomness for the scale value
	UPROPERTY(EditAnywhere, Category = "MajorNoise|Randomness")
	float MajorNoiseScaleDeviation = 0.0f; 

	//The influence of the PerlinNoise
	UPROPERTY(EditAnywhere, Category = "MinorNoise", meta = (UIMin = 0))
	int MinorNoiseStrength;

	//X and Y offset of the PerlinNoise
	UPROPERTY(EditAnywhere, Category = "MinorNoise")
	FVector2D MinorNoiseOffset;

	//X and Y scale of the PerlinNoise
	UPROPERTY(EditAnywhere, Category = "MinorNoise")
	FVector2D MinorNoiseScale;

	//Randomness for the strength value
	UPROPERTY(EditAnywhere, Category = "MinorNoise|Randomness")
	float MinorNoiseStrengthDeviation = 0.0f;

	//Randomness for the offset value
	UPROPERTY(EditAnywhere, Category = "MinorNoise|Randomness")
	float MinorNoiseOffsetDeviation = 0.0f;

	//Randomness for the scale value
	UPROPERTY(EditAnywhere, Category = "MinorNoise|Randomness")
	float MinorNoiseScaleDeviation = 0.0f;

	//If trees should be generated
	UPROPERTY(EditAnywhere, Category = "Foliage|General")
	bool bGenerateTrees = false;

	//If grass should be generated
	UPROPERTY(EditAnywhere, Category = "Foliage|General")
	bool bGenerateGrass = false;

	// If bushes should be generated
	UPROPERTY(EditAnywhere, Category = "Foliage|General")
	bool bGenerateBushes = false;

	//If bushes should also generated
	UPROPERTY(EditAnywhere, Category = "Foliage|General")
	bool bGenerateBranches = false;

	//Time until spawning a new foliage batch
	UPROPERTY(EditAnywhere, Category = "Foliage|General")
	float FoliageUpdateCooldown = 0.25;

	//Should occlusion culling be applied?
	UPROPERTY(EditAnywhere, Category = "Foliage|General")
	bool bUseCulling = false;

	//End distance of occlusion culling
	UPROPERTY(EditAnywhere, Category = "Foliage|General", meta = (EditCondition = "bUseCulling"))
	float FoliageCullDistance = 1000;

	//Number of trees per tile
	UPROPERTY(EditAnywhere, Category = "Foliage|TreeGeneration", meta = (UIMin = 1, UIMax = 100, EditCondition = "bGenerateTrees"))
	int TreeSpawnCount = 0;

	//Max number of tries to generate a new tree location
	UPROPERTY(EditAnywhere, Category = "Foliage|TreeGeneration", meta = (EditCondition = "bGenerateTrees"))
	int TreeMaxTries = 100; 

	//How many trees to spawn each batch
	UPROPERTY(EditAnywhere, Category = "Foliage|TreeGeneration", meta = (EditCondition = "bGenerateTrees"))
	int TreeBatchSize = 50;

	//Trees and their respective data for generation
	UPROPERTY(EditAnywhere, Category = "Foliage|TreeGeneration", meta = (EditCondition = "bGenerateTrees"))
	TArray<class UFoliageDataAsset*> TreeData; 

	//Should the radius of the trees be visualized?
	UPROPERTY(EditAnywhere, Category = "Foliage|TreeGeneration", meta = (EditCondition = "bGenerateTrees"))
	bool bDrawTreeDebug = false;

	//Number of Grass per tile
	UPROPERTY(EditAnywhere, Category = "Foliage|GrassGeneration", meta = (UIMin = 1, UIMax = 100, EditCondition = "bGenerateGrass"))
	int GrassSpawnCount = 0;

	//Max number of tries to generate a new grass location
	UPROPERTY(EditAnywhere, Category = "Foliage|GrassGeneration", meta = (EditCondition = "bGenerateGrass"))
	int GrassMaxTries = 100; 

	//How much grass to spawn each batch
	UPROPERTY(EditAnywhere, Category = "Foliage|GrassGeneration", meta = (EditCondition = "bGenerateGrass"))
	int GrassBatchSize = 50; 

	//Grass types and their respective data for generation
	UPROPERTY(EditAnywhere, Category = "Foliage|GrassGeneration", meta = (EditCondition = "bGenerateGrass"))
	TArray<class UFoliageDataAsset*> GrassData; 

	//Number of bushes per tile
	UPROPERTY(EditAnywhere, Category = "Foliage|BushGeneration", meta = (UIMin = 1, UIMax = 100, EditCondition = "bGenerateBushes"))
	int BushSpawnCount = 0;

	//Max number of tries to generate a new bush location
	UPROPERTY(EditAnywhere, Category = "Foliage|BushGeneration", meta = (EditCondition = "bGenerateBushes"))
	int BushMaxTries = 100; 

	//How many bushes to spawn each batch
	UPROPERTY(EditAnywhere, Category = "Foliage|BushGeneration", meta = (EditCondition = "bGenerateBushes"))
	int BushBatchSize = 50;

	//Bush types and their respective data for generation
	UPROPERTY(EditAnywhere, Category = "Foliage|BushGeneration", meta = (EditCondition = "bGenerateBushes"))
	TArray<class UFoliageDataAsset*> BushData; 

	//Number of branches per tile
	UPROPERTY(EditAnywhere, Category = "Foliage|BranchGeneration", meta = (UIMin = 1, UIMax = 100, EditCondition = "bGenerateBranches"))
	int BranchSpawnCount = 0;

	//Maximum number of tries to generate a new branch location
	UPROPERTY(EditAnywhere, Category = "Foliage|BranchGeneration", meta = (EditCondition = "bGenerateBranches"))
	int BranchMaxTries = 100; 

	//How many branches to spawn each batch
	UPROPERTY(EditAnywhere, Category = "Foliage|BranchGeneration", meta = (EditCondition = "bGenerateBranches"))
	int BranchBatchSize = 50;

	//Branch types and their respective data for generation
	UPROPERTY(EditAnywhere, Category = "Foliage|BranchGeneration", meta = (EditCondition = "bGenerateBranches"))
	TArray<class UFoliageDataAsset*> BranchData; 

	/**
	 * Initializes tiles for the specified drawing distance
	 *
	 */
	UFUNCTION(CallInEditor)
	void InitializeTiles();

	/**
	 * Is called to update the tiles of the landscape
	 * 
	 * \param NewCenterIndex is the index of the tile that will be used as the new center
	 */
	void UpdateTiles(FTileIndex NewCenterIndex);

	virtual void Tick(float DeltaSeconds);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;	

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	//Current tile where the player is located
	UPROPERTY(EditAnywhere)
	FTileIndex CenterTileIndex;

	//Can a new foliage thread be started
	UPROPERTY()
	bool bIsFoliageThreadFinished = true;

	//All tiles that currently exist
	TMap<FTileIndex, AProceduralTile*> Tiles; 

	//Parameters that are needed for the generation of atile
	FTileGenerationParams TileGenerationParams;

	//Threads that create locations for a specific foliage component
	TArray<FFoliageGenerationThread*> FoliageGenerationThreads;

	TQueue<AProceduralTile*> TilesToDelete;

	//Currently running thread
	class FRunnableThread* RunningThread;

	//Current thread that generates new foliage instances
	class FFoliageGenerationThread* CurrentFoliageThread;

	//The FoliageInfos of the previous foliage generation
	TArray<struct FGeneratedFoliageInfo> LastGeneratedFoliageInfos;

	//Components for which the creation of new instances is already finished
	TArray<UFoliageGenerationComponent*> FoliageComponentsToUpdate;

	//TileIndex of the previous foliage component
	struct FTileIndex LastTileIndex;

	//Time passed since last update
	float CurrentUpdateTime = 0.f;

	/**
	 * Delets all tiles in the Tiles-Map
	 */
	void DeleteAllTiles();

	/**
	 * Sets up the parameters needed for the tile generation.
	 *
	 * \return the parameters that will be used for the tile generation
	 */
	FTileGenerationParams SetupTileGenerationParams();

	void SpawnNewFoliage();

	void InitializeFoliageThread();

	void DeleteSingleTile();

	void GenerateFoliage(FTileIndex CurrentTileIndex, AProceduralTile* CurrentTile);

	AProceduralTile* GenerateTile(FTileIndex CurrentTileIndex);

};
