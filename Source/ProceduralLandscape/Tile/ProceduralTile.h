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

	UPROPERTY(EditAnywhere, meta = (UIMin = 1))
	bool bReloadInEditor;//Should the mesh be updated in the editor if changes are made to it's properties?

	UPROPERTY(EditAnywhere, meta = (UIMin = 1))
	int TileSize; //Width of the tile

	UPROPERTY(EditAnywhere, meta = (UIMin = 2, UIMax = 100))
	int TileResolution;//Count of vertices on each axis

	UPROPERTY(EditAnywhere, meta = (UIMin = 0))
	int MajorNoiseStrength; //The influence of the PerlinNoise

	UPROPERTY(EditAnywhere)
	FVector2D MajorNoiseOffset; //X and Y offset of the PerlinNoise

	UPROPERTY(EditAnywhere)
	FVector2D MajorNoiseScale;//X and Y scale of the PerlinNoise

	UPROPERTY(EditAnywhere, meta = (UIMin = 0))
	int MinorNoiseStrength; //The influence of the PerlinNoise

	UPROPERTY(EditAnywhere)
	FVector2D MinorNoiseOffset; //X and Y offset of the PerlinNoise

	UPROPERTY(EditAnywhere)
	FVector2D MinorNoiseScale;//X and Y scale of the PerlinNoise

protected:
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
	float GetZOffset(float XPos, float YPos, FVector2D NoiseScale, FVector2D NoiseOffset, float NoiseStrength, float Offset);

	//Maps a position to the UV-Space
	float MapToUV(float Pos, float Offset);
};
