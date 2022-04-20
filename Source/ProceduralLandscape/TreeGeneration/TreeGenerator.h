// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "../Tile/ProceduralTile.h"

#include "TreeGenerator.generated.h"

UCLASS()
class PROCEDURALLANDSCAPE_API ATreeGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATreeGenerator();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Generates random tree locations
	void GenerateTrees(FTileIndex TileIndex, int TileSize, int RandomSeed);

	UPROPERTY(EditAnywhere, meta = (UIMin = 1, UIMax = 100))
	int SpawnCount = 0;//Number of trees per tile

	UPROPERTY(EditAnywhere)
	float TreeRadius = 5;//Area around a tree where other trees can't spawn

	UPROPERTY(EditAnywhere)
	int MaxTries = 100; //Maximum number of tries to generate a new location

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:	
	//Checks if a new location overlaps with an existing one
	bool DoesOverlap(FVector NewLocation, TArray<FVector> GeneratedLocations);

};
