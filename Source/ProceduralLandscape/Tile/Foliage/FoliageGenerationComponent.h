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
	float Radius;
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
	 * \param SpawnCount_In The max amount of foliage to be spawned
	 * \param TreeRadius_In The max distance between each tree
	 * \param MaxTries_In The max number of tries to generate a new location
	 * \param TreeMeshes_In The meshes for the foliage
	 */
	void SetupFoliageGeneration(int SpawnCount_In, int MaxTries_In, TArray<class UFoliageDataAsset*> FoliageData_In);

	/**
	 * Generates randomly placed foliage
	 *
	 * \param TileIndex the tile index to spawn trees for
	 * \param TileSize the size of a tile
	 * \param TraceZStart the z value where the line trace to find the ground should start
	 * \param TraceZEnd the z value where the line trace  to find the ground should start
	 * \param RandomSeed the random seed of the current game
	 */
	void GenerateFoliage(struct FTileIndex TileIndex, int TileSize, float TraceZStart, float TraceZEnd, int RandomSeed, TArray<FGeneratedFoliageInfo>& ExistingFoliageInfos, bool bDrawDebug = false);

	/**
	 * Removes all trees in all HISMComponents.
	 *
	 */
	void ClearFoliage();

private:
	/**
	 * Checks if NewLocation overlaps with any location in GeneratedLocations
	 *
	 * \param NewLocation The new location to check if it is valid
	 * \param GeneratedLocations The already existing locations
	 * \return true if the locations is inside the Radius of an existing tree, false otherwise
	 */
	virtual bool DoesOverlap(FVector NewLocation, TArray<FGeneratedFoliageInfo>& FoliageInfos, float CurrentFoliageRadius);

	UPROPERTY()
		TArray<class UHierarchicalInstancedStaticMeshComponent*> HISMComponents; //All HISM Components of this TreeGenerationComponent

	UPROPERTY()
		int SpawnCount;//Number of foliage instances per tile

	UPROPERTY()
		int MaxTries; //Maximum number of tries to generate a new location

	UPROPERTY()
		TArray<UFoliageDataAsset*> FoliageData; //All foliage meshes which are currently used

		
};
