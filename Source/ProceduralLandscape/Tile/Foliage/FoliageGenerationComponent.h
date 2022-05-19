// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
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
	 * \param bUseCulling If culling should be applied to the HISM components
	 * \param CullDistance the end point distance for culling
	 * \param SpawnCount_In The max amount of foliage to be spawned
	 * \param MaxTries_In The max number of tries to generate a new location
	 * \param BatchSize_In the max count of instances to spawn per batch
	 * \param FoliageData_In The relevant data for the different foliage types
	 * \param RandomSeed_In The random seed of the current landscape
	 * \param bCollisionEnabled If collision should be applied to the foliage instances
	 */
	void SetupFoliageGeneration(bool bUseCulling, float CullDistance, int SpawnCount_In, int MaxTries_In, int BatchSize_In, TArray<class UFoliageDataAsset*> FoliageData_In, int RandomSeed_In, bool bCollisionEnabled);

	/**
	 * Generates randomly placed foliage locations or spawns them directly
	 *
	 * \param bSpawnDirect If the instance should be spawned directly
	 * \param TileIndex the tile index to spawn trees for
	 * \param TileSize the size of a tile
	 * \param TraceZStart the z value where the line trace to find the ground should start
	 * \param TraceZEnd the z value where the line trace  to find the ground should start
	 * \param ExistingFoliageInfos Information about existing foliage on this tile
	 * \param bDrawDebug if the radius of the foliage instance should be visualized
	 */
	void GenerateFoliage(bool bSpawnDirect, struct FTileIndex TileIndex, int TileSize, float TraceZStart, float TraceZEnd, TArray<FGeneratedFoliageInfo>& ExistingFoliageInfos,  bool bDrawDebug = false);

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

private:
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
	TArray<FTileBounds> InitializeBounds(struct FTileIndex TileIndex, int TileSize);

	void GenerateRandomInstance(TArray<FTileBounds> TileBounds, FRandomStream& RandomStream, float TraceZStart, float TraceZEnd, int& HISMComponentIndex, FVector& Location);

	//All HISM components of this TreeGenerationComponent
	UPROPERTY()
	TArray<class UHierarchicalInstancedStaticMeshComponent*> HISMComponents; 

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
		
};
