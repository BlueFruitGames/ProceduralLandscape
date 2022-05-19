// Fill out your copyright notice in the Description page of Project Settings.


#include "FoliageGenerationThread.h"

#include "FoliageGenerationComponent.h"
#include "../TileGenerator.h"
#include "../ProceduralTile.h"

FFoliageGenerationThread::FFoliageGenerationThread(UFoliageGenerationComponent* FoliageGenerationComponent_In, ATileGenerator* TileGenerator_In, FTileIndex TileIndex_In, int TileSize_In, float TraceZStart_In, float TraceZEnd_In)
{
	FoliageGenerationComponent = FoliageGenerationComponent_In;
	TileGenerator = TileGenerator_In;
	TileIndex = TileIndex_In;
	TileSize = TileSize_In;
	TraceZStart = TraceZStart_In;
	TraceZEnd = TraceZEnd_In;
}

bool FFoliageGenerationThread::Init()
{
	return true;
}

uint32 FFoliageGenerationThread::Run()
{
	FoliageGenerationComponent->GenerateFoliage(false, TileIndex, TileSize, TraceZStart, TraceZEnd, FoliageInfos);
	TileGenerator->bIsFoliageThreadFinished = true;
	return 0;
}

void FFoliageGenerationThread::Stop()
{
}

