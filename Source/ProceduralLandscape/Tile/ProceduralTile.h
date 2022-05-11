// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralTile.generated.h"

USTRUCT()
struct FTileIndex
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int X;

	UPROPERTY(EditAnywhere)
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

	class UFoliageGenerationComponent* GetTreeGenerationComponent() {
		return TreeGenerationComponent;
	}

	class UFoliageGenerationComponent* GetGrassGenerationComponent() {
		return GrassGenerationComponent;
	}

	float GetMaxZPosition(){
		return MaxZPosition;
	}

	float GetMinZPosition() {
		return MinZPosition;
	}

private:
	UPROPERTY(VisibleAnywhere)
	class UProceduralMeshComponent* ProceduralMeshComponent; //component to procedurally create a tile

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* BoxComponent; //trigger to detect relocation of the player

	UPROPERTY(VisibleAnywhere)
	class UFoliageGenerationComponent* TreeGenerationComponent; //component for generating trees

	UPROPERTY(VisibleAnywhere)
	class UFoliageGenerationComponent* GrassGenerationComponent; //component for generating trees

	UPROPERTY()
	TSubclassOf<class ACharacter> PlayerClass; //Class that is used to activate the trigger

	UPROPERTY()
	class ATileGenerator* TileGenerator; //pointer to the tile generator

	UPROPERTY()
	FTileIndex TileIndex; //current index of this tile

	UPROPERTY()
	float MaxZPosition; //highest value for any vertex on the Z-axis

	UPROPERTY()
	float MinZPosition; //smallest value for any vertex on the Z-axis


	/**
	 * Generates the triangles for the vertex at postion CurrentRow, CurrentColumn
	 * 
	 * \param Triangles the reference to the triangles array
	 * \param CurrentRow the current row
	 * \param CurrentColumn the current column
	 * \param TileResolution the resolution of a tile
	 */
	void GenerateTriangles(TArray<int32>& Triangles, int CurrentRow, int CurrentColumn, int TileResolution);

	/**
	 * Sets up the Parameters for creating a new mesh
	 * 
	 * \param TileGenerationParams the parameters that are needed for the generation of a new tile
	 * \param Vertices reference to the array that stores the vertices
	 * \param Triangles reference to the array that stores the triangles
	 * \param Normals reference to the array that stores the normals
	 * \param UV0 reference to the array that stores the uvs
	 * \param VertexColor reference to the array that stores the vertex colors
	 */
	void SetupParamsCreation(FTileGenerationParams TileGenerationParams, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals, TArray<FVector2D>& UV0, TArray<FLinearColor>& VertexColor);
	
	/**
	 * Sets up the Parameters for updating an existing mesh
	 * 
	 * \param TileGenerationParams the parameters that are needed for the generation of a new tile
	 * \param Vertices reference to the array that stores the vertices
	 * \param Normals  reference to the array that stores the normals
	 * \param UV0 reference to the array that stores the uvs
	 * \param VertexColor reference to the array that stores the vertex colors
	 */
	void SetupParamsUpdate(FTileGenerationParams TileGenerationParams, TArray<FVector>& Vertices, TArray<FVector>& Normals, TArray<FVector2D>& UV0, TArray<FLinearColor>& VertexColor);
	
	/**
	 * Generates the information that is related to the vertices
	 * 
	 * \param Vertices reference to the array that stores the vertices
	 * \param Normals  reference to the array that stores the normals
	 * \param UV0 reference to the array that stores the uvs
	 * \param VertexColor reference to the array that stores the vertex colors
	 * \param MinZOffset refernce that will store the smallest Z value
	 * \param MaxZOffset reference that will store the highest Z value
	 * \param TileGenerationParams the parameters that are needed for the generation of a new tile
	 * \param Row current row
	 * \param Column current column
	 * \param DistanceBetweenVertices the distance between two vertices
	 */
	void GenerateVertexInformation(TArray<FVector>& Vertices, TArray<FVector>& Normals, TArray<FVector2D>& UV0, TArray<FLinearColor>& VertexColor, float& MinZOffset, float& MaxZOffset, FTileGenerationParams TileGenerationParams, int Row, int Column, float DistanceBetweenVertices);

	/**
	 * Adds a triangle for the Vertices V1, V2, V3
	 * 
	 * \param Triangles reference to the array that stores the triangles
	 * \param V1 index of the first bertex
	 * \param V2 index of the second vertex
	 * \param V3 index of the third vertex
	 */
	void AddTriangle(TArray<int32>& Triangles, int32 V1, int32 V2, int32 V3);

	/**
	 * Calculates the normal of vertex at Position (XPos, YPos)
	 * 
	 * \param XPos location on the X-axis
	 * \param YPos location on the Y-axis
	 * \param DistanceBetweenVertices the distance between two vertices
	 * \param TileGenerationParams the parameters that are needed for the generation of a new tile
	 * \return the normal vector of this vertex
	 */
	FVector CalculateVertexNormal(float XPos, float YPos, float DistanceBetweenVertices, FTileGenerationParams TileGenerationParams);

	/**
	 * Caclulates the Z-Offset at Position(XPos,YPos) using the perlin noise function
	 * 
	 * \param XPos location on the X-axis
	 * \param YPos location on the Y-axis
	 * \param NoiseScale scale of the the PerlineNoise
	 * \param NoiseOffset offset of the PerlineNoise
	 * \param NoiseStrength strength of the PerlineNoise
	 * \return the Z-Position of a vertex
	 */
	float GetZOffset(float XPos, float YPos, FVector2D NoiseScale, FVector2D NoiseOffset, float NoiseStrength);

	/**
	 * Maps a value from range (MinPrev,MaxPrev) to the value in the new range (MinNew, MaxNew)
	 * 
	 * \param Value current value
	 * \param MinPrev smallest value of the previous range
	 * \param MaxPrev highest value of the previous range
	 * \param MinNew smallest value of the new range
	 * \param MaxNew highest value of the new range
	 * \return Value mapped to the the new range 
	 */
	float MapToRange(float Value, float MinPrev, float MaxPrev, float MinNew, float MaxNew);

	/**
	 * Maps a Value to the UV-Space
	 * 
	 * \param Value value to map to UV space
	 * \param TileSize The size of a tile
	 * \return mapped Value to UV-Space
	 */
	float MapToUV(float Value, int TileSize);
};
