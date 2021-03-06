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

	bool operator!=(const FTileIndex& Other) const {
		return !Equals(Other);
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
	
	/**
	 * Sets up essential variables of the tile.
	 * 
	 * \param Tilegenerator_In the tile generator of the game instance
	 * \param PlayerClass_In the class that is used by the player
	 * \param Material the material that should be applied to the landscape
	 * \param bGenerateTrees if trees should be generated
	 * \param bGenerateGrass if grass should be generated
	 * \param bGenerateBushes if bushes should be generated
	 */
	void Setup(class ATileGenerator* Tilegenerator_In, TSubclassOf<class ACharacter> PlayerClass_In, UMaterialInterface* Material, bool bGenerateTrees = false, bool bGenerateGrass = false, bool bGenerateBushes = false);
	
	/**
	 * Procedurally generates a tile using ProceduralMeshComponent.
	 * 
	 * \param TileGenerationParams the parameters needed to generate a tile
	 * \param bIsUpdate if the tile should be updated or created
	 */
	void GenerateTile(FTileGenerationParams TileGenerationParams, bool bIsUpdate = false);

	/**
	 * Checks if the generation of locations for all foliage components is finished.
	 * 
	 * \return true if tile is marked as deleted or all foliage components have finished their generation
	 */
	bool IsGenerationFinished();

	class UFoliageGenerationComponent* GetTreeGenerationComponent() {
		return TreeGenerationComponent;
	}

	class UFoliageGenerationComponent* GetGrassGenerationComponent() {
		return GrassGenerationComponent;
	}

	class UFoliageGenerationComponent* GetBushGenerationComponent() {
		return BushGenerationComponent;
	}


	FTileIndex GetTileIndex() {
		return TileIndex;
	}

	float GetMaxZPosition(){
		return MaxZPosition;
	}

	float GetMinZPosition() {
		return MinZPosition;
	}

	void MarkToDelete() {
		bMarkedToDelete = true;
	}

private:
	//Component to procedurally create a tile
	UPROPERTY(VisibleAnywhere)
	class UProceduralMeshComponent* ProceduralMeshComponent; 

	//Trigger to detect relocation of the player
	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* BoxComponent;

	//Component for generating trees
	UPROPERTY(VisibleAnywhere)
	class UFoliageGenerationComponent* TreeGenerationComponent; 

	//Component for generating grass
	UPROPERTY(VisibleAnywhere)
	class UFoliageGenerationComponent* GrassGenerationComponent; 

	//Component for generating bushes
	UPROPERTY(VisibleAnywhere)
	class UFoliageGenerationComponent* BushGenerationComponent; 

	//Class that is used by the player actor
	UPROPERTY()
	TSubclassOf<class ACharacter> PlayerClass;

	//Pointer to the tile generator
	UPROPERTY()
	class ATileGenerator* TileGenerator; 

	//Index of this tile
	UPROPERTY()
	FTileIndex TileIndex;

	//Highest value for any vertex on the Z-axis
	UPROPERTY()
	float MaxZPosition;

	//Smallest value for any vertex on the Z-axis
	UPROPERTY()
	float MinZPosition;

	//Is the tile ready to be deleted
	UPROPERTY()
	bool bMarkedToDelete;

	/**
	 * Sets up all desired foliage generation components.
	 * 
	 * \param bGenerateTrees should trees be generated
	 * \param bGenerateGrass should grass be generated
	 * \param bGenerateBushes should bushes be generated
	 */
	void SetupFoliageComponents(bool bGenerateTrees, bool bGenerateGrass, bool bGenerateBushes);

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
	void SetupParamsCreation(FTileGenerationParams TileGenerationParams, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals, TArray<FVector2D>& UV0, TArray<FColor>& VertexColor);
	
	/**
	 * Sets up the Parameters for updating an existing mesh
	 * 
	 * \param TileGenerationParams the parameters that are needed for the generation of a new tile
	 * \param Vertices reference to the array that stores the vertices
	 * \param Normals  reference to the array that stores the normals
	 * \param UV0 reference to the array that stores the uvs
	 * \param VertexColor reference to the array that stores the vertex colors
	 */
	void SetupParamsUpdate(FTileGenerationParams TileGenerationParams, TArray<FVector>& Vertices, TArray<FVector>& Normals, TArray<FVector2D>& UV0, TArray<FColor>& VertexColor);
	
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
	void GenerateVertexInformation(TArray<FVector>& Vertices, TArray<FVector>& Normals, TArray<FVector2D>& UV0, TArray<FColor>& VertexColor, float& MinZOffset, float& MaxZOffset, FTileGenerationParams TileGenerationParams, int Row, int Column, float DistanceBetweenVertices);

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
