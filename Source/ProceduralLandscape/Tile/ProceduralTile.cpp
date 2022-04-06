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

void AProceduralTile::OnConstruction(const FTransform& Transform) {
	if (bReloadInEditor) {
		GenerateTile();
	}
}

// Called when the game starts or when spawned
void AProceduralTile::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AProceduralTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProceduralTile::GenerateTile() {
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FLinearColor> VertexColor;

	float StartOffset = float(TileSize) / 2;
	float DistanceBetweenVertices = float(TileSize) / (TileResolution - 1);

	for (int Row = 0; Row < TileResolution; ++Row) {
		for (int Column = 0; Column < TileResolution; ++Column) {

			float CurrentXOffset = StartOffset - DistanceBetweenVertices * Row;
			float CurrentYOffset = StartOffset - DistanceBetweenVertices * Column;

			float UPos = MapToUV(CurrentXOffset, StartOffset);
			float VPos = MapToUV(CurrentYOffset, StartOffset);

			float CurrentZOffset = GetZOffset(UPos, VPos, StartOffset);

			FVector CurrentLocation(CurrentXOffset, CurrentYOffset, CurrentZOffset);
			Vertices.Add(CurrentLocation);

			if (Row < TileResolution - 1 && Column < TileResolution - 1) {
				GenerateTriangles(Triangles, Row, Column);
			}
			Normals.Add(CalculateVertexNormal(CurrentXOffset, CurrentYOffset, StartOffset, DistanceBetweenVertices));
			UV0.Add(FVector2D(UPos, VPos));
			VertexColor.Add(FLinearColor(CurrentZOffset, 1 - CurrentZOffset, 0));
		}
	}
	ProceduralMeshComponent->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UV0, VertexColor, TArray<FProcMeshTangent>(), true);
}


void AProceduralTile::GenerateTriangles(TArray<int32>& Triangles, int CurrentRow, int CurrentColumn) {
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

FVector AProceduralTile::CalculateVertexNormal(float XPos, float YPos, float Offset, float DistanceBetweenVertices) {
	float Top = MapToUV(YPos + DistanceBetweenVertices, Offset);
	float Bottom = MapToUV(YPos - DistanceBetweenVertices, Offset);
	float Right = MapToUV(XPos + DistanceBetweenVertices, Offset);
	float Left = MapToUV(XPos - DistanceBetweenVertices, Offset);

	float ZTopLeft = GetZOffset(Left, Top, Offset);
	float ZTopRight = GetZOffset(Right, Top, Offset);
	float ZBottomLeft = GetZOffset(Left, Bottom, Offset);
	float ZBottomRight = GetZOffset(Right, Bottom, Offset);

	FVector TopLeftLocation(XPos - DistanceBetweenVertices, YPos + DistanceBetweenVertices, ZTopLeft);
	FVector TopRightLocation(XPos + DistanceBetweenVertices, YPos + DistanceBetweenVertices, ZTopRight);
	FVector BottomLeftLocation(XPos - DistanceBetweenVertices, YPos - DistanceBetweenVertices, ZBottomLeft);
	FVector BottomRightLocation(XPos + DistanceBetweenVertices, YPos - DistanceBetweenVertices, ZBottomRight);


	FVector Normal_01 = FVector::CrossProduct((BottomLeftLocation - TopLeftLocation), (TopRightLocation - TopLeftLocation));
	FVector Normal_02 = FVector::CrossProduct((BottomRightLocation - BottomLeftLocation), (TopRightLocation - BottomLeftLocation));
	return (Normal_02 + Normal_01 / 2).GetSafeNormal();
}

float AProceduralTile::GetZOffset(float XPos, float YPos, float Offset) {

	float ScaledXPos = MapToRange(XPos, 0, 1, 0, NoiseScale.X);
	float ScaledYPos = MapToRange(YPos, 0, 1, 0, NoiseScale.Y);

	float ZOffset = FMath::PerlinNoise2D(FVector2D(ScaledXPos + NoiseOffset.X, ScaledYPos + NoiseOffset.Y)) * NoiseStrength;
	return ZOffset;
}

float AProceduralTile::MapToUV(float Pos, float Offset) {
	return (Pos + Offset) / TileSize;
}

float AProceduralTile::MapToRange(float Value, float MinPrev, float MaxPrev, float MinNew, float MaxNew) {
	return MinNew + (Value - MinPrev) * (MaxNew - MinNew) / (MaxPrev - MinPrev);
}

