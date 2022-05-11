// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GrassGenerationComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROCEDURALLANDSCAPE_API UGrassGenerationComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGrassGenerationComponent();

	void SetupGrassGeneration(int SpawnCount_In, int MaxTries_In, TArray<class UFoliageDataAsset*> GrassData_In);

	void GenerateGrass(TArray<struct FGeneratedTreeInfo> GeneratedTreeInfos, struct FTileIndex TileIndex, int TileSize, float TraceZStart, float TraceZEnd, int RandomSeed, bool bDrawDebug);

	void ClearGrass();

private:
	UPROPERTY()
	TArray<class UHierarchicalInstancedStaticMeshComponent*> HISMComponents; //All HISM Components of this TreeGenerationComponent

	UPROPERTY()
	int SpawnCount;//Number of trees per tile

	UPROPERTY()
	int MaxTries; //Maximum number of tries to generate a new location

	UPROPERTY()
	TArray<class UFoliageDataAsset*> GrassData; //All tree meshes which are currently used

	bool DoesOverlap(TArray<struct FGeneratedTreeInfo> GeneratedTreeInfos, FVector NewLocation, TArray<FVector> GeneratedLocations, float GrassRadius);
		
};
