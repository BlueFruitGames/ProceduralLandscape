// Fill out your copyright notice in the Description page of Project Settings.


#include "TreeGenerator.h"

#include "DrawDebugHelpers.h"
#include "Math/RandomStream.h"

// Sets default values
ATreeGenerator::ATreeGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATreeGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATreeGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATreeGenerator::GenerateTrees(FTileIndex TileIndex, int TileSize)
{
	float XMin = TileIndex.X * TileSize - TileSize / 2 + TreeRadius;
	float XMax = TileIndex.X * TileSize + TileSize / 2 - TreeRadius;
	float YMin = TileIndex.Y * TileSize - TileSize / 2 + TreeRadius;
	float YMax = TileIndex.Y * TileSize + TileSize / 2 - TreeRadius;

	FRandomStream RandomStream(TileIndex.X*10000 + TileIndex.Y);
	TArray<FVector> GeneratedLocations;
	int Count = 0;
	int Tries = 0;
	while (Count < SpawnCount && Tries < MaxTries) {
		float XPos = RandomStream.FRandRange(XMin, XMax);
		float YPos = RandomStream.FRandRange(YMin, YMax);
		FVector Location(XPos, YPos, 0);
		if (DoesOverlap(Location, GeneratedLocations)) {
			Tries += 1;
		}
		else {
			DrawDebugSphere(GetWorld(), Location, TreeRadius, 8, FColor::Red, false, 10, 0, 2);
			GeneratedLocations.Add(Location);
			Tries = 0;
			Count += 1;
		}
	}
	
}

bool ATreeGenerator::DoesOverlap(FVector NewLocation, TArray<FVector> GeneratedLocations)
{
	for (FVector CurrentLocation : GeneratedLocations) {
		float Distance = FMath::Abs(FVector::Dist(CurrentLocation, NewLocation));
		if (Distance < TreeRadius * 2) return true;
	}
	return false;
}

