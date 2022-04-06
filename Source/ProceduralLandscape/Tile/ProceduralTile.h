// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralTile.generated.h"

UCLASS()
class PROCEDURALLANDSCAPE_API AProceduralTile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProceduralTile();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, meta = (UIMin = 1))
	bool bReloadInEditor;//Should the mesh be updated in the editor if changes are made to it's properties?

	UPROPERTY(EditAnywhere, meta = (UIMin = 1))
	int TileSize; //Width of the tile

	UPROPERTY(EditAnywhere, meta = (UIMin = 2, UIMax = 100))
	int TileResolution;//Count of vertices on each axis

	UPROPERTY(EditAnywhere, meta = (UIMin = 0))
	int NoiseStrength; //The influence of the PerlinNoise

	UPROPERTY(EditAnywhere)
	FVector2D NoiseOffset; //X and Y offset of the PerlinNoise

	UPROPERTY(EditAnywhere)
	FVector2D NoiseScale;//X and Y scale of the PerlinNoise

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

private:
	UPROPERTY()
	class UProceduralMeshComponent* ProceduralMeshComponent;

	//Procedurally generates a tile using ProceduralMeshComponent
	UFUNCTION(CallInEditor)
	void GenerateTile();

	//Generates the triangles vor the vertex at (CurrentRow, CurrentColumn)
	void GenerateTriangles(TArray<int32>& Triangles, int CurrentRow, int CurrentColumn);

	//Adds a triangle for the Vertices V1, V2, V3
	void AddTriangle(TArray<int32>& Triangles, int32 V1, int32 V2, int32 V3);

	//Calculates the normal of vertex at Position (XPos, YPos)
	FVector CalculateVertexNormal(float XPos, float YPos, float Offset, float DistanceBetweenVertices);

	// Maps a value from range (MinPrev,MaxPrev) to the value in the new range (MinNew, MaxNew)
	float MapToRange(float Value, float MinPrev, float MaxPrev, float MinNew, float MaxNew);

	//Caclulates the Z-Offset at Position(XPos,YPos) using the perlin noise function
	float GetZOffset(float XPos, float YPos, float Offset);

	//Maps a position to the UV-Space
	float MapToUV(float Pos, float Offset);
};
