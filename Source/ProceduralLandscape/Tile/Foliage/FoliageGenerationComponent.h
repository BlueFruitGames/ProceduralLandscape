// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "../ProceduralTile.h"
#include "FoliageGenerationComponent.generated.h"

struct FTileBounds {
	float XMin;
	float XMax;
	float YMin;
	float YMax;
};

struct FGeneratedFoliageInfo {
	FVector Location;
	UCurveFloat* GrowthCurve;
	float Radius;
	bool bIsTree;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROCEDURALLANDSCAPE_API UFoliageGenerationComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	UFoliageGenerationComponent();

	/**
	 * Sets up the membervariables and creates the HISMComponents
	 *
	 * \param TileIndex_In the index of the tile with which this component is associated
	 * \param FoliageData_In The relevant data for the different foliage types
	 * \param SpawnCount_In The max amount of foliage to be spawned
	 * \param MaxTries_In The max number of tries to generate a new location
	 * \param BatchSize_In the max count of instances to spawn per batch
	 * \param RandomSeed_In The random seed of the current landscape
	 * \param bAffectsLight If the lighting is affected by this foliage type
	 * \param bUseCulling If culling should be applied to the HISM components
	 * \param CullDistance the end point distance for culling
	 * \param bCollisionEnabled If collision should be applied to the foliage instances
	 */
	void SetupFoliageGeneration(FTileIndex TileIndex_In, TArray<class UFoliageDataAsset*> FoliageData_In, int SpawnCount_In, int MaxTries_In, int BatchSize_In, int RandomSeed_In, bool bAffectsLight, bool bUseCulling, float CullDistance, bool bCollisionEnabled);

	/**
	 * Generates randomly placed foliage locations or spawns them directly
	 *
	 * \param ExistingFoliageInfos Information about existing foliage on this tile
	 * \param bSpawnDirect If the instance should be spawned directly
	 * \param TileSize the size of a tile
	 * \param TraceZStart the z value where the line trace to find the ground should start
	 * \param TraceZEnd the z value where the line trace  to find the ground should start
	 * \param bDrawDebug if the radius of the foliage instance should be visualized
	 */
	void GenerateFoliage(TArray<FGeneratedFoliageInfo>& ExistingFoliageInfos, bool bSpawnDirect, int TileSize, float TraceZStart, float TraceZEnd, bool bDrawDebug = false);

	/**
	 * Removes all instances of all HISM components.
	 * 
	 */
	void ClearFoliage();

	/**
	 * Spawns new instances at the previously generated locations.
	 * 
	 * \return ture if all instances were spawned, false otherwise
	 */
	bool UpdateFoliage();

	bool GetIsGenerationFinished() {
		return bIsGenerationFinished;
	}

	FTileIndex GetTileIndex() {
		return TileIndex;
	}

private:
	//All HISM components of this TreeGenerationComponent
	UPROPERTY()
	TArray<class UHierarchicalInstancedStaticMeshComponent*> HISMComponents; 

	//The Index of the tile with which this component is associated
	UPROPERTY()
	FTileIndex TileIndex;

	//Number of foliage instances per tile
	UPROPERTY()
	int SpawnCount;

	//Max number of tries to generate a new location
	UPROPERTY()
	int MaxTries; 

	//Size of the individual batches to spawn new instances
	UPROPERTY()
	int BatchSize;

	//The random seed of the current landscape
	UPROPERTY()
	int RandomSeed;

	//Information about all foliage types to use
	UPROPERTY()
	TArray<UFoliageDataAsset*> FoliageData; 
	
	//The lock to regulate access to the InstancesToSpawn array
	FCriticalSection Lock;

	//Locations of the new instances
	TArray<FTransformArrayA2> InstancesToSpawn;

	//The indices for each HISM component for the following batch
	TArray<int> NextSpawnIndices;

	//If all foliage was already spawned
	bool bIsGenerationFinished = false;
	
	/**
	 * Checks if NewLocation overlaps with any location in GeneratedLocations
	 *
	 * \param NewLocation The new location to check if it is valid
	 * \param FoliageInfos The already existing foliage instances
	 * \param Distance the distance to the closest instance 
	 * \param ClosestRadius the radius of the closest instance
	 * \param GrowthCurve the GrowthCurve of the clostest instance
	 * \return true if the locations is inside the Radius of an existing tree, false otherwise
	 */
	virtual bool DoesOverlap(FVector NewLocation, TArray<FGeneratedFoliageInfo> FoliageInfos, float CurrentFoliageRadius, float& Distance, float& ClosestRadius, UCurveFloat*& GrowthCurve);

	/**
	 * Initializes the landscape bounds for each HISM component.
	 *
	 * \param TileIndex the current tile index
	 * \param TileSize the size of the tile
	 * \return TArray with the bounds for each HISM component
	 */
	TArray<FTileBounds> InitializeBounds(int TileSize);

	/**
	 * Generates a random location for a random HISM Component for a new instace.
	 *
	 * \param TileBounds Bounds for all hism components concerning the current tile
	 * \param RandomStream the random stream previously created
	 * \param TraceZStart start of the trace to determine Z location
	 * \param TraceZEnd end of the trace to determine Z location
	 * \param HISMComponentIndex the inds of the HISM component for which the location was generated
	 * \param Location the generated location
	 */
	void GenerateRandomInstance(TArray<FTileBounds> TileBounds, FRandomStream& RandomStream, float TraceZStart, float TraceZEnd, int& HISMComponentIndex, FVector& Location);
		
};
