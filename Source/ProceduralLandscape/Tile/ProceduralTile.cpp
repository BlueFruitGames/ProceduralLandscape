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
			Vertices.Add(FVector(CurrentXOffset, CurrentYOffset, 0));

			if (Column < TileResolution - 1 && Column < TileResolution - 1) {
				GenerateTriangles(Triangles, Row, Column);
			}

			Normals.Add(FVector(0, 0, 1));

			float UPos = (CurrentXOffset + StartOffset) / TileSize;
			float VPos = (CurrentYOffset + StartOffset) / TileSize;
			UV0.Add(FVector2D(UPos, VPos));
		}
	}

	ProceduralMeshComponent->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UV0, TArray<FLinearColor>(), TArray<FProcMeshTangent>(), true);
}

void AProceduralTile::GenerateTriangles(TArray<int32>& Triangles, int CurrentRow, int CurrentColumn) {
	int Current = (CurrentRow * TileResolution) + CurrentColumn;
	int Right = Current + 1;
	int Lower = Current + TileResolution;
	int LowerRight = Lower + 1;
	AddTriangles(Triangles, Right, LowerRight, Current);
	AddTriangles(Triangles, Lower, Current, LowerRight);
}

void AProceduralTile::AddTriangles(TArray<int32>& Triangles, int32 V1, int32 V2, int32 V3) {
	Triangles.Add(V1);
	Triangles.Add(V2);
	Triangles.Add(V3);
}



