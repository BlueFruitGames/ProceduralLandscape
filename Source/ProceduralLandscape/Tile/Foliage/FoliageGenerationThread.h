// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "../ProceduralTile.h"
#include "FoliageGenerationComponent.h"

#include "HAL/Runnable.h"

/**
 * 
 */
class PROCEDURALLANDSCAPE_API FFoliageGenerationThread : public FRunnable
{
public:
	FFoliageGenerationThread(UFoliageGenerationComponent* FoliageGenerationComponent_In, class ATileGenerator* TileGenerator_In, FTileIndex TileIndex_In, int TileSize_In, float TraceZStart_In, float TraceZEnd_In);

	bool Init() override;

	uint32 Run() override;

	void Stop() override;

	UFoliageGenerationComponent* GetFoliageGenerationComponent() {
		return FoliageGenerationComponent;
	}


	TArray<FGeneratedFoliageInfo> GetFoliageInfos() {
		return FoliageInfos;
	}

	void SetFoliageInfos(TArray<FGeneratedFoliageInfo> FoliageInfos_In) {
		FoliageInfos = FoliageInfos_In;
	}

	FTileIndex GetTileIndex() {
		return TileIndex;
	}


private:
	//Pointer to the tile generator that initialized this thread
	class ATileGenerator* TileGenerator;

	//FoliageGenerationComponent that is associated with this thread
	UFoliageGenerationComponent* FoliageGenerationComponent;

	//Index of the tile to generate foliage for
	FTileIndex TileIndex;

	//The size of the tile
	int TileSize;

	//Start location of the trace to located the Z position on a tile 
	float TraceZStart;

	//End location of the trace to located the Z position on a tile 
	float TraceZEnd;

	//The information about the foliage associated with this tile 
	TArray<FGeneratedFoliageInfo> FoliageInfos;
};
