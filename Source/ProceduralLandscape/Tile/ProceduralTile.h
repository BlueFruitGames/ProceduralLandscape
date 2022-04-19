// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralTile.generated.h"

USTRUCT()
struct FTileIndex
{
	GENERATED_BODY()

	UPROPERTY()
	int X;

	UPROPERTY()
	int Y;

	FTileIndex() : X(0), Y(0) {};

	FTileIndex(int X, int Y) : X(X), Y(Y) {};

	FTileIndex(const FTileIndex& Other) : FTileIndex(Other.X, Other.Y) {};

	bool operator==(const FTileIndex& Other) const
	{
		return Equals(Other);
	}

	bool Equals(const FTileIndex& Other) const
	{
		return X == Other.X && Y == Other.Y;
	}

};

#if UE_BUILD_DEBUG
uint32 GetTypeHash(const FTileIndex& Thing);
#else // optimize by inlining in shipping and development builds
FORCEINLINE uint32 GetTypeHash(const FTileIndex& Thing)
{
	uint32 Hash = FCrc::MemCrc32(&Thing, sizeof(FTileIndex));
	return Hash;
};
#endif

USTRUCT()
struct FTileGenerationParams
{
	GENERATED_BODY()

	UPROPERTY()
	FVector2D MajorNoiseScale;

	UPROPERTY()
	FVector2D MajorNoiseOffset;

	UPROPERTY()
	FVector2D MinorNoiseScale;

	UPROPERTY()
	FVector2D MinorNoiseOffset;

	UPROPERTY()
	FTileIndex TileIndex;

	UPROPERTY()
	int TileSize;

	UPROPERTY()
	int TileResolution;

	UPROPERTY()
	float MajorNoiseStrength;

	UPROPERTY()
	float MinorNoiseStrength;

};

UCLASS()
class PROCEDURALLANDSCAPE_API AProceduralTile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProceduralTile();

	UFUNCTION()
	void OnBeginOverlap (UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	//Sets up proberties which aren't changing at runtime
	void Setup(class ATileGenerator* Tilegenerator_In, TSubclassOf<class ACharacter> PlayerClass_In, UMaterialInterface* Material);
	
	//Procedurally generates a tile using ProceduralMeshComponent
	void GenerateTile(FTileGenerationParams TileGenerationParams, bool bIsUpdate = false);

private:
	UPROPERTY(VisibleAnywhere)
	class UProceduralMeshComponent* ProceduralMeshComponent;

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* BoxComponent;

	UPROPERTY()
	TSubclassOf<class ACharacter> PlayerClass;

	FTileIndex TileIndex;

	class ATileGenerator* TileGenerator;

	//Generates the triangles vor the vertex at (CurrentRow, CurrentColumn)
	void GenerateTriangles(TArray<int32>& Triangles, int CurrentRow, int CurrentColumn, int TileResolution);

	//Sets up the Parameters for creating a new mesh
	void SetupParamsCreation(FTileGenerationParams TileGenerationParams, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals, TArray<FVector2D>& UV0, TArray<FLinearColor>& VertexColor);
	
	//Sets up the Parameters for updating an existing mesh
	void SetupParamsUpdate(FTileGenerationParams TileGenerationParams, TArray<FVector>& Vertices, TArray<FVector>& Normals, TArray<FVector2D>& UV0, TArray<FLinearColor>& VertexColor);
	
	void GenerateVertexInformation(TArray<FVector>& Vertices, TArray<FVector>& Normals, TArray<FVector2D>& UV0, TArray<FLinearColor>& VertexColor, float& MinZOffset, float& MaxZOffset, FTileGenerationParams TileGenerationParams, int Row, int Column, float StartOffset, float DistanceBetweenVertices);

	//Adds a triangle for the Vertices V1, V2, V3
	void AddTriangle(TArray<int32>& Triangles, int32 V1, int32 V2, int32 V3);

	//Calculates the normal of vertex at Position (XPos, YPos)
	FVector CalculateVertexNormal(float XPos, float YPos, float Offset, float DistanceBetweenVertices, FTileGenerationParams TileGenerationParams);

	//Caclulates the Z-Offset at Position(XPos,YPos) using the perlin noise function
	float GetZOffset(float XPos, float YPos, FVector2D NoiseScale, FVector2D NoiseOffset, float NoiseStrength, float Offset);

	// Maps a value from range (MinPrev,MaxPrev) to the value in the new range (MinNew, MaxNew)
	float MapToRange(float Value, float MinPrev, float MaxPrev, float MinNew, float MaxNew);

	//Maps a position to the UV-Space
	float MapToUV(float Pos, float Offset, int TileSize);
};
