// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralTile.h"

#include "ProceduralMeshComponent.h"

// Sets default values
AProceduralTile::AProceduralTile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("PorceduralMeshComponent"));
	ProceduralMeshComponent->SetCollisionProfileName("BlockAll");
	SetRootComponent(ProceduralMeshComponent);
}

void AProceduralTile::GenerateTile(FTileGenerationParams TileGenerationParams, UMaterialInterface* Material) {
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FLinearColor> VertexColor;

	float StartOffset = float(TileGenerationParams.TileSize) / 2;
	float DistanceBetweenVertices = float(TileGenerationParams.TileSize) / (TileGenerationParams.TileResolution - 1);

	for (int Row = 0; Row < TileGenerationParams.TileResolution; ++Row) {
		for (int Column = 0; Column < TileGenerationParams.TileResolution; ++Column) {

			float CurrentXOffset = StartOffset - DistanceBetweenVertices * Row;
			float CurrentYOffset = StartOffset - DistanceBetweenVertices * Column;

			float UPos = MapToUV(CurrentXOffset + TileGenerationParams.TileIndex.X * TileGenerationParams.TileSize, StartOffset, TileGenerationParams.TileSize);
			float VPos = MapToUV(CurrentYOffset + TileGenerationParams.TileIndex.Y * TileGenerationParams.TileSize, StartOffset, TileGenerationParams.TileSize);

			float MicroZOffset = GetZOffset(UPos, VPos, TileGenerationParams.MinorNoiseScale, TileGenerationParams.MinorNoiseOffset, TileGenerationParams.MinorNoiseStrength, StartOffset);
			float LargeZOffset = GetZOffset(UPos, VPos, TileGenerationParams.MajorNoiseScale, TileGenerationParams.MajorNoiseOffset, TileGenerationParams.MajorNoiseStrength, StartOffset);
			float CurrentZOffset = LargeZOffset + MicroZOffset;

			FVector CurrentLocation(CurrentXOffset, CurrentYOffset, CurrentZOffset);
			Vertices.Add(CurrentLocation);

			if (Row < TileGenerationParams.TileResolution - 1 && Column < TileGenerationParams.TileResolution - 1) {
				GenerateTriangles(Triangles, Row, Column, TileGenerationParams.TileResolution);
			}
			Normals.Add(CalculateVertexNormal(CurrentXOffset + TileGenerationParams.TileIndex.X * TileGenerationParams.TileSize, CurrentYOffset + TileGenerationParams.TileIndex.Y * TileGenerationParams.TileSize, StartOffset, DistanceBetweenVertices, TileGenerationParams));
			UV0.Add(FVector2D(UPos, VPos));
			VertexColor.Add(FLinearColor(CurrentZOffset, 1 - CurrentZOffset, MicroZOffset));
		}
	}
	ProceduralMeshComponent->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UV0, VertexColor, TArray<FProcMeshTangent>(), true);
	ProceduralMeshComponent->SetMaterial(0, Material);
}


void AProceduralTile::GenerateTriangles(TArray<int32>& Triangles, int CurrentRow, int CurrentColumn, int TileResolution) {
	int Current = (CurrentRow * TileResolution) + CurrentColumn;
	int Right = Current + 1;
	int Lower = Current + TileResolution;
	int LowerRight = Lower + 1;
	AddTriangle(Triangles, Right, LowerRight, Current);
	AddTriangle(Triangles, Lower, Current, LowerRight);
}

void AProceduralTile::AddTriangle(TArray<int32>& Triangles, int32 V1, int32 V2, int32 V3) {
	Triangles.Add(V1);
	Triangles.Add(V2);
	Triangles.Add(V3);
}

FVector AProceduralTile::CalculateVertexNormal(float XPos, float YPos, float Offset, float DistanceBetweenVertices, FTileGenerationParams TileGenerationParams) {
	float Top = MapToUV(YPos + DistanceBetweenVertices, Offset, TileGenerationParams.TileSize);
	float Bottom = MapToUV(YPos - DistanceBetweenVertices, Offset, TileGenerationParams.TileSize);
	float Right = MapToUV(XPos + DistanceBetweenVertices, Offset, TileGenerationParams.TileSize);
	float Left = MapToUV(XPos - DistanceBetweenVertices, Offset, TileGenerationParams.TileSize);

	float ZTopLeftMajor = GetZOffset(Left, Top, TileGenerationParams.MajorNoiseScale, TileGenerationParams.MajorNoiseOffset, TileGenerationParams.MajorNoiseStrength ,Offset);
	float ZTopLeftMinor = GetZOffset(Left, Top, TileGenerationParams.MinorNoiseScale, TileGenerationParams.MinorNoiseOffset, TileGenerationParams.MinorNoiseStrength, Offset);
	float ZTopLeft = ZTopLeftMajor + ZTopLeftMinor;
	
	float ZTopRightMajor = GetZOffset(Right, Top, TileGenerationParams.MajorNoiseScale, TileGenerationParams.MajorNoiseOffset, TileGenerationParams.MajorNoiseStrength, Offset);
	float ZTopRightMinor = GetZOffset(Right, Top, TileGenerationParams.MinorNoiseScale, TileGenerationParams.MinorNoiseOffset, TileGenerationParams.MinorNoiseStrength, Offset);
	float ZTopRight = ZTopRightMajor + ZTopRightMinor;
	
	float ZBottomLeftMajor = GetZOffset(Left, Bottom, TileGenerationParams.MajorNoiseScale, TileGenerationParams.MajorNoiseOffset, TileGenerationParams.MajorNoiseStrength, Offset);
	float ZBottomLeftMinor = GetZOffset(Left, Bottom, TileGenerationParams.MinorNoiseScale, TileGenerationParams.MinorNoiseOffset, TileGenerationParams.MinorNoiseStrength, Offset);
	float ZBottomLeft = ZBottomLeftMajor + ZBottomLeftMinor;
	
	float ZBottomRightMajor = GetZOffset(Right, Bottom, TileGenerationParams.MajorNoiseScale, TileGenerationParams.MajorNoiseOffset, TileGenerationParams.MajorNoiseStrength, Offset);
	float ZBottomRightMinor = GetZOffset(Right, Bottom, TileGenerationParams.MinorNoiseScale, TileGenerationParams.MinorNoiseOffset, TileGenerationParams.MinorNoiseStrength, Offset);
	float ZBottomRight = ZBottomRightMajor + ZBottomRightMinor;

	FVector TopLeftLocation(XPos - DistanceBetweenVertices, YPos + DistanceBetweenVertices, ZTopLeft);
	FVector TopRightLocation(XPos + DistanceBetweenVertices, YPos + DistanceBetweenVertices, ZTopRight);
	FVector BottomLeftLocation(XPos - DistanceBetweenVertices, YPos - DistanceBetweenVertices, ZBottomLeft);
	FVector BottomRightLocation(XPos + DistanceBetweenVertices, YPos - DistanceBetweenVertices, ZBottomRight);


	FVector Normal_01 = FVector::CrossProduct((BottomLeftLocation - TopLeftLocation), (TopRightLocation - TopLeftLocation));
	FVector Normal_02 = FVector::CrossProduct((BottomRightLocation - BottomLeftLocation), (TopRightLocation - BottomLeftLocation));
	return (Normal_02 + Normal_01 / 2).GetSafeNormal();
}

float AProceduralTile::GetZOffset(float XPos, float YPos, FVector2D NoiseScale, FVector2D NoiseOffset, float NoiseStrength, float Offset) {

	float ScaledXPos = MapToRange(XPos, 0, 1, 0, NoiseScale.X);
	float ScaledYPos = MapToRange(YPos, 0, 1, 0, NoiseScale.Y);

	float ZOffset = FMath::PerlinNoise2D(FVector2D(ScaledXPos + NoiseOffset.X, ScaledYPos + NoiseOffset.Y)) * NoiseStrength;
	return ZOffset;
}

float AProceduralTile::MapToUV(float Pos, float Offset, int TileSize) {
	return (Pos + Offset) / TileSize;
}

float AProceduralTile::MapToRange(float Value, float MinPrev, float MaxPrev, float MinNew, float MaxNew) {
	return MinNew + (Value - MinPrev) * (MaxNew - MinNew) / (MaxPrev - MinPrev);
}

