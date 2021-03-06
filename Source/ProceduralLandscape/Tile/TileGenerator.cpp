// Fill out your copyright notice in the Description page of Project Settings.


#include "TileGenerator.h"

#include "Foliage/FoliageGenerationComponent.h"
#include "Math/RandomStream.h"
#include "HAL/RunnableThread.h"


ATileGenerator::ATileGenerator()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATileGenerator::OnConstruction(const FTransform& Transform) {
	if (bReloadInEditor) {
		InitializeTiles();
	}
}

void ATileGenerator::BeginPlay()
{
	Super::BeginPlay();
	LastTileIndex = FTileIndex(0, 0);
	LastGeneratedFoliageInfos = TArray<FGeneratedFoliageInfo>();
	CenterTileIndex.X = 0;
	CenterTileIndex.Y = 0;
	InitializeTiles();
}

void ATileGenerator::Tick(float DeltaSeconds)
{
	CurrentUpdateTime += DeltaSeconds;
	SpawnNewFoliage();
	DeleteSingleTile();

	if (!bIsFoliageThreadFinished) return;
	InitializeFoliageThread();
}

void ATileGenerator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (CurrentFoliageThread && RunningThread) {
		RunningThread->Kill(true);
		RunningThread->WaitForCompletion();
		delete CurrentFoliageThread;
	}

	FFoliageGenerationThread* CurrentThread;
	while (FoliageGenerationThreads.IsValidIndex(0)) {
		CurrentThread = FoliageGenerationThreads[0];
		FoliageGenerationThreads.RemoveAt(0);
		delete CurrentThread;
	}
}

void ATileGenerator::InitializeTiles()
{
	DeleteAllTiles();
	SetupTileGenerationParams();
	for (int Row = CenterTileIndex.X - DrawDistance; Row <= CenterTileIndex.X + DrawDistance; ++Row) {
		for (int Column = CenterTileIndex.Y - DrawDistance; Column <= CenterTileIndex.Y + DrawDistance; ++Column) {
			FTileIndex CurrentTileIndex(Row, Column);
			AProceduralTile* CurrentTile = GenerateTile(CurrentTileIndex);
			GenerateFoliage(CurrentTileIndex, CurrentTile);
		}
	}
	SortFoliageGenerationThreads();
}

FTileGenerationParams ATileGenerator::SetupTileGenerationParams()
{
	FRandomStream RandomStream(RandomSeed);
	TileGenerationParams.TileSize = TileSize;
	TileGenerationParams.TileResolution = TileResolution;

	TileGenerationParams.MajorNoiseStrength = RandomStream.FRandRange(MajorNoiseStrength - MajorNoiseStrengthDeviation, MajorNoiseStrength + MajorNoiseStrengthDeviation);
	float MajorNoiseOffsetX = RandomStream.FRandRange(MajorNoiseOffset.X - MajorNoiseOffsetDeviation, MajorNoiseOffset.X + MajorNoiseOffsetDeviation);
	float MajorNoiseOffsetY = RandomStream.FRandRange(MajorNoiseOffset.Y - MajorNoiseOffsetDeviation, MajorNoiseOffset.Y + MajorNoiseOffsetDeviation);
	TileGenerationParams.MinorNoiseOffset = FVector2D(MajorNoiseOffsetX, MajorNoiseOffsetY);
	float MajorNoiseScaleX = RandomStream.FRandRange(MajorNoiseScale.X - MajorNoiseScaleDeviation, MajorNoiseScale.X + MajorNoiseScaleDeviation);
	float MajorNoiseScaleY = RandomStream.FRandRange(MajorNoiseScale.Y - MajorNoiseScaleDeviation, MajorNoiseScale.Y + MajorNoiseScaleDeviation);
	TileGenerationParams.MajorNoiseScale = FVector2D(MajorNoiseScaleX, MajorNoiseScaleY);

	TileGenerationParams.MinorNoiseStrength = RandomStream.FRandRange(MinorNoiseStrength - MinorNoiseStrengthDeviation, MinorNoiseStrength + MinorNoiseStrengthDeviation);
	float MinorNoiseOffsetX = RandomStream.FRandRange(MinorNoiseOffset.X - MinorNoiseOffsetDeviation, MinorNoiseOffset.X + MinorNoiseOffsetDeviation);
	float MinorNoiseOffsetY = RandomStream.FRandRange(MinorNoiseOffset.Y - MinorNoiseOffsetDeviation, MinorNoiseOffset.Y + MinorNoiseOffsetDeviation);
	TileGenerationParams.MinorNoiseOffset = FVector2D(MinorNoiseOffsetX, MinorNoiseOffsetY);
	float MinorNoiseScaleX = RandomStream.FRandRange(MinorNoiseScale.X - MinorNoiseScaleDeviation, MinorNoiseScale.X + MinorNoiseScaleDeviation);
	float MinorNoiseScaleY = RandomStream.FRandRange(MinorNoiseScale.Y - MinorNoiseScaleDeviation, MinorNoiseScale.Y + MinorNoiseScaleDeviation);
	TileGenerationParams.MinorNoiseScale = FVector2D(MinorNoiseScaleX, MinorNoiseScaleY);

	return TileGenerationParams;
}

void ATileGenerator::UpdateTiles(FTileIndex NewCenterIndex)
{
	CenterTileIndex = NewCenterIndex;
	TArray<FTileIndex> TilesToRemove;
	TArray<FTileIndex> IndicesToGenerate;
	Tiles.GetKeys(TilesToRemove);

	for (int Row = CenterTileIndex.X - DrawDistance; Row <= CenterTileIndex.X + DrawDistance; ++Row) {
		for (int Column = CenterTileIndex.Y - DrawDistance; Column <= CenterTileIndex.Y + DrawDistance; ++Column) {
			FTileIndex CurrentTileIndex(Row, Column);
			if (Tiles.Find(CurrentTileIndex)) {
				TilesToRemove.Remove(CurrentTileIndex);
			}
			else if(!Tiles.Find(CurrentTileIndex)) {
				AProceduralTile* CurrentTile = GenerateTile(CurrentTileIndex);			
				GenerateFoliage(CurrentTileIndex, CurrentTile);
			}
		}
	}
	for (FTileIndex& IndexToRemove : TilesToRemove) {
		AProceduralTile* CurrentTile = *Tiles.Find(IndexToRemove);
		int i = 0;
		while (i < FoliageGenerationThreads.Num()) {
			if (FoliageGenerationThreads[i]->GetTileIndex() == IndexToRemove) {
				delete FoliageGenerationThreads[i];
				FoliageGenerationThreads.RemoveAt(i);
				continue;
			}
			++i;
		}
		CurrentTile->MarkToDelete();
		TilesToDelete.Enqueue(CurrentTile);
		Tiles.Remove(IndexToRemove);
	}
	SortFoliageGenerationThreads();
}

AProceduralTile* ATileGenerator::GenerateTile(FTileIndex CurrentTileIndex)
{
	TileGenerationParams.TileIndex = CurrentTileIndex;
	FVector TileLocation(CurrentTileIndex.X * TileSize, CurrentTileIndex.Y * TileSize, 0);
	FActorSpawnParameters SpawnParams;
	AProceduralTile* CurrentTile = GetWorld()->SpawnActor<AProceduralTile>(TileLocation, GetActorRotation(), SpawnParams);
	CurrentTile->Setup(this, PlayerClass, LandscapeMaterial, bGenerateTrees, bGenerateGrass, bGenerateBushes);
	CurrentTile->GenerateTile(TileGenerationParams);
	FString TileName = FString::Printf(TEXT("TILE %d,%d"), CurrentTileIndex.X, CurrentTileIndex.Y);
	CurrentTile->SetActorLabel(TileName);
	return CurrentTile;
}

void ATileGenerator::GenerateFoliage(FTileIndex CurrentTileIndex, AProceduralTile* CurrentTile)
{
	TArray <FGeneratedFoliageInfo> GeneratedFoliage;
	if (bGenerateTrees) {
		CurrentTile->GetTreeGenerationComponent()->SetupFoliageGeneration(CurrentTileIndex, TreeData, TreeSpawnCount, TreeMaxTries, TreeBatchSize, RandomSeed, true, bUseCulling, FoliageCullDistance, true);
		FFoliageGenerationThread* NewThread = new FFoliageGenerationThread(CurrentTile->GetTreeGenerationComponent(), this, CurrentTileIndex, TileSize, CurrentTile->GetMaxZPosition(), CurrentTile->GetMinZPosition());
		FoliageGenerationThreads.Add(NewThread);
	}

	if (bGenerateBushes) {
		CurrentTile->GetBushGenerationComponent()->SetupFoliageGeneration(CurrentTileIndex, BushData, BushSpawnCount, BushMaxTries, BushBatchSize, RandomSeed, true, bUseCulling, FoliageCullDistance, false);
		FFoliageGenerationThread* NewThread = new FFoliageGenerationThread(CurrentTile->GetBushGenerationComponent(), this, CurrentTileIndex, TileSize, CurrentTile->GetMaxZPosition(), CurrentTile->GetMinZPosition());
		FoliageGenerationThreads.Add(NewThread);
	}

	if (bGenerateGrass) {
		CurrentTile->GetGrassGenerationComponent()->SetupFoliageGeneration(CurrentTileIndex, GrassData, GrassSpawnCount, GrassMaxTries, GrassBatchSize, RandomSeed, false, bUseCulling, FoliageCullDistance, false);
		FFoliageGenerationThread* NewThread = new FFoliageGenerationThread(CurrentTile->GetGrassGenerationComponent(), this, CurrentTileIndex, TileSize, CurrentTile->GetMaxZPosition(), CurrentTile->GetMinZPosition());
		FoliageGenerationThreads.Add(NewThread);
	}
	Tiles.Add(CurrentTileIndex, CurrentTile);
}

void ATileGenerator::InitializeFoliageThread()
{
	if (RunningThread && CurrentFoliageThread) {
		UFoliageGenerationComponent* CurrentFoliageGenerationComponent = CurrentFoliageThread->GetFoliageGenerationComponent();
		LastTileIndex = CurrentFoliageThread->GetTileIndex();
		LastGeneratedFoliageInfos = CurrentFoliageThread->GetFoliageInfos();
		if (FoliageComponentsToUpdate.Find(CurrentFoliageGenerationComponent) < 0) {
			FoliageComponentsToUpdate.Add(CurrentFoliageGenerationComponent);
		}
		delete CurrentFoliageThread;
		RunningThread = nullptr;
	}
	else if (FoliageGenerationThreads.IsValidIndex(0)) {
		CurrentFoliageThread = FoliageGenerationThreads[0];
		FoliageGenerationThreads.RemoveAt(0);
		if (LastTileIndex == CurrentFoliageThread->GetTileIndex()) {
			CurrentFoliageThread->SetFoliageInfos(LastGeneratedFoliageInfos);
		}
		else {
			CurrentFoliageThread->SetFoliageInfos(TArray<FGeneratedFoliageInfo>());
		}
		bIsFoliageThreadFinished = false;
		CurrentFoliageThread->GetFoliageGenerationComponent()->SetVisibility(false, true);
		RunningThread = FRunnableThread::Create(CurrentFoliageThread, TEXT("FoliageGeneration"));
	}
}

void ATileGenerator::SpawnNewFoliage()
{
	if (CurrentUpdateTime >= FoliageUpdateCooldown && FoliageComponentsToUpdate.Num() > 0) {
		SortFoliageComponentsToUpdate();
		UFoliageGenerationComponent* CurrentFoliageComponent = FoliageComponentsToUpdate[0];
		if (FoliageComponentsToUpdate[0]->UpdateFoliage()) {
			FoliageComponentsToUpdate[0]->SetVisibility(true, true);
			FoliageComponentsToUpdate.RemoveAt(0);
		}
		CurrentUpdateTime = 0;
	}
}

void ATileGenerator::SortFoliageGenerationThreads()
{
	TArray<FFoliageGenerationThread*> SortedArray;

	for (FFoliageGenerationThread* CurrentThread : FoliageGenerationThreads) {
		int CurrentIndex = 0;
		int CurrentXDistance = FMath::Abs(CurrentThread->GetTileIndex().X - CenterTileIndex.X);
		int CurrentYDistance = FMath::Abs(CurrentThread->GetTileIndex().Y - CenterTileIndex.Y);
		int CurrentDistance = FMath::Max(CurrentXDistance, CurrentYDistance);
		while (CurrentIndex < SortedArray.Num()) {
			int OtherXDistance = FMath::Abs(SortedArray[CurrentIndex]->GetTileIndex().X - CenterTileIndex.X);
			int OtherYDistance = FMath::Abs(SortedArray[CurrentIndex]->GetTileIndex().Y - CenterTileIndex.Y);
			int OtherDistance = FMath::Max(OtherXDistance, OtherYDistance);
			if (CurrentDistance < OtherDistance) break;
			CurrentIndex += 1;
		}
		SortedArray.Insert(CurrentThread, CurrentIndex);
	}
	FoliageGenerationThreads = SortedArray;
}

void ATileGenerator::SortFoliageComponentsToUpdate()
{
	TArray<UFoliageGenerationComponent*> SortedArray;

	for (UFoliageGenerationComponent* CurrentComponent : FoliageComponentsToUpdate) {
		int CurrentIndex = 0;
		int CurrentXDistance = FMath::Abs(CurrentComponent->GetTileIndex().X - CenterTileIndex.X);
		int CurrentYDistance = FMath::Abs(CurrentComponent->GetTileIndex().Y - CenterTileIndex.Y);
		int CurrentDistance = FMath::Max(CurrentXDistance, CurrentYDistance);
		while (CurrentIndex < SortedArray.Num()) {
			int OtherXDistance = FMath::Abs(SortedArray[CurrentIndex]->GetTileIndex().X - CenterTileIndex.X);
			int OtherYDistance = FMath::Abs(SortedArray[CurrentIndex]->GetTileIndex().Y - CenterTileIndex.Y);
			int OtherDistance = FMath::Max(OtherXDistance, OtherYDistance);
			if (CurrentDistance < OtherDistance) break;
			CurrentIndex += 1;
		}
		SortedArray.Insert(CurrentComponent, CurrentIndex);
	}
	FoliageComponentsToUpdate = SortedArray;
}


void ATileGenerator::DeleteSingleTile()
{
	AProceduralTile* TileToDelete;
	if (TilesToDelete.Dequeue(TileToDelete)) {
		if (TileToDelete->IsGenerationFinished() && !Tiles.Find(TileToDelete->GetTileIndex())) {
			TileToDelete->Destroy();
		}
		else {
			TilesToDelete.Enqueue(TileToDelete);
		}
	}
}

void ATileGenerator::DeleteAllTiles() {
	TArray<AProceduralTile*> Values;
	Tiles.GenerateValueArray(Values);
	for (AProceduralTile* Tile : Values) {
		Tile->Destroy();
	}
	Tiles.Empty();
}