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
	class ATileGenerator* TileGenerator;

	UFoliageGenerationComponent* FoliageGenerationComponent;


	FTileIndex TileIndex;
	int TileSize;
	float TraceZStart;
	float TraceZEnd;
	TArray<FGeneratedFoliageInfo> FoliageInfos;
};
