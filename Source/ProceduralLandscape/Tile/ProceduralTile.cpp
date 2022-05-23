// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralTile.h"

#include "ProceduralMeshComponent.h"
#include "TileGenerator.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"

#define COLLISION_GROUND ECC_GameTraceChannel1

AProceduralTile::AProceduralTile()
{
	ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("PorceduralMeshComponent"));
	ProceduralMeshComponent->SetCollisionProfileName("BlockAll");
	ProceduralMeshComponent->bAffectDistanceFieldLighting = true;
	ProceduralMeshComponent->bAffectDynamicIndirectLighting = true;
	ProceduralMeshComponent->SetCollisionResponseToChannel(COLLISION_GROUND, ECollisionResponse::ECR_Block);
	SetRootComponent(ProceduralMeshComponent);
	ProceduralMeshComponent->SetMobility(EComponentMobility::Static);

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetupAttachment(RootComponent);
	BoxComponent->SetMobility(EComponentMobility::Static);
}



void AProceduralTile::Setup(ATileGenerator* Tilegenerator_In, TSubclassOf<ACharacter> PlayerClass_In, UMaterialInterface* Material, bool bGenerateTrees, bool bGenerateGrass, bool bGenerateBushes)
{	
	TileGenerator = Tilegenerator_In;
	PlayerClass = PlayerClass_In.Get();
	if (ProceduralMeshComponent) ProceduralMeshComponent->SetMaterial(0, Material);
	if(BoxComponent) BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AProceduralTile::OnBeginOverlap);
	SetupFoliageComponents(bGenerateTrees, bGenerateGrass, bGenerateBushes);
}


void AProceduralTile::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->IsA(PlayerClass.Get())) {
		UE_LOG(LogTemp, Warning, TEXT("Overlap"));
		if (TileGenerator)
		{
			TileGenerator->UpdateTiles(TileIndex);
		}
		else UE_LOG(LogTemp, Warning, TEXT("No TileGenerator"));
	}
}

void AProceduralTile::GenerateTile(FTileGenerationParams TileGenerationParams, bool bIsUpdate) {
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FColor> VertexColor;

	TileIndex = TileGenerationParams.TileIndex;
	if (bIsUpdate) SetupParamsUpdate(TileGenerationParams, Vertices, Normals, UV0, VertexColor);
	else SetupParamsCreation(TileGenerationParams, Vertices, Triangles, Normals, UV0, VertexColor);

	if (ProceduralMeshComponent) {
		if (bIsUpdate) {
			ProceduralMeshComponent->UpdateMeshSection(0, Vertices, Normals, UV0, VertexColor, TArray<FProcMeshTangent>());
			ProceduralMeshComponent->AddCollisionConvexMesh(Vertices);
		}
		else {
			ProceduralMeshComponent->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, VertexColor, TArray<FProcMeshTangent>(), true);
			ProceduralMeshComponent->AddCollisionConvexMesh(Vertices);
		}
	}
}

void AProceduralTile::SetupFoliageComponents(bool bGenerateTrees, bool bGenerateGrass, bool bGenerateBushes)
{
	if (bGenerateTrees) {
		TreeGenerationComponent = NewObject<UFoliageGenerationComponent>(this, TEXT("TreeGenerationComponent"));
		TreeGenerationComponent->SetMobility(EComponentMobility::Static);
		TreeGenerationComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	}
	if (bGenerateGrass) {
		GrassGenerationComponent = NewObject<UFoliageGenerationComponent>(this, TEXT("GrassGenerationComponent"));
		GrassGenerationComponent->SetMobility(EComponentMobility::Static);
		GrassGenerationComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	}
	if (bGenerateBushes) {
		BushGenerationComponent = NewObject<UFoliageGenerationComponent>(this, TEXT("BushGenerationComponent"));
		BushGenerationComponent->SetMobility(EComponentMobility::Static);
		BushGenerationComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	}
}

void AProceduralTile::SetupParamsCreation(FTileGenerationParams TileGenerationParams, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals, TArray<FVector2D>& UV0, TArray<FColor>& VertexColor) {
	float StartOffset = float(TileGenerationParams.TileSize) / 2;
	float DistanceBetweenVertices = float(TileGenerationParams.TileSize) / (TileGenerationParams.TileResolution - 1);

	float MaxZOffset = 0;
	float MinZOffset = 0;

	for (int Row = 0; Row < TileGenerationParams.TileResolution; ++Row) {
		for (int Column = 0; Column < TileGenerationParams.TileResolution; ++Column) {
			GenerateVertexInformation(Vertices, Normals, UV0, VertexColor, MinZOffset, MaxZOffset, TileGenerationParams, Row, Column, DistanceBetweenVertices);
			if (Row < TileGenerationParams.TileResolution - 1 && Column < TileGenerationParams.TileResolution - 1) {
				GenerateTriangles(Triangles, Row, Column, TileGenerationParams.TileResolution);
			}
		}
	}
	float ZBoxExtent = (MaxZOffset - MinZOffset) / 2 + 10;
	float XYBoxExtent = TileGenerationParams.TileSize / 2 - 10;
	if (BoxComponent) BoxComponent->SetBoxExtent(FVector(XYBoxExtent, XYBoxExtent, ZBoxExtent));
	MaxZPosition = MaxZOffset;
	MinZPosition = MinZOffset;
}

void AProceduralTile::SetupParamsUpdate(FTileGenerationParams TileGenerationParams, TArray<FVector>& Vertices, TArray<FVector>& Normals, TArray<FVector2D>& UV0, TArray<FColor>& VertexColor) {
	float StartOffset = float(TileGenerationParams.TileSize) / 2;
	float DistanceBetweenVertices = float(TileGenerationParams.TileSize) / (TileGenerationParams.TileResolution - 1);

	float MaxZOffset = 0;
	float MinZOffset = 0;

	for (int Row = 0; Row < TileGenerationParams.TileResolution; ++Row) {
		for (int Column = 0; Column < TileGenerationParams.TileResolution; ++Column) {
			GenerateVertexInformation(Vertices, Normals, UV0, VertexColor, MinZOffset, MaxZOffset, TileGenerationParams, Row, Column, DistanceBetweenVertices);
		}
	}
	float ZBoxExtent = (MaxZOffset - MinZOffset) / 2 + 10;
	float XYBoxExtent = TileGenerationParams.TileSize / 2 - 10;
	if (BoxComponent) BoxComponent->SetBoxExtent(FVector(XYBoxExtent, XYBoxExtent, ZBoxExtent));
	MaxZPosition = MaxZOffset;
	MinZPosition = MinZOffset;
}

void AProceduralTile::GenerateVertexInformation(TArray<FVector>& Vertices, TArray<FVector>& Normals, TArray<FVector2D>& UV0, TArray<FColor>& VertexColor, float& MinZOffset, float& MaxZOffset, FTileGenerationParams TileGenerationParams, int Row, int Column, float DistanceBetweenVertices)
{
	float CurrentXOffset = float(TileGenerationParams.TileSize) / 2 - DistanceBetweenVertices * Row;
	float CurrentYOffset = float(TileGenerationParams.TileSize) / 2 - DistanceBetweenVertices * Column;

	float UPos = MapToUV(CurrentXOffset + TileGenerationParams.TileIndex.X * TileGenerationParams.TileSize, TileGenerationParams.TileSize);
	float VPos = MapToUV(CurrentYOffset + TileGenerationParams.TileIndex.Y * TileGenerationParams.TileSize, TileGenerationParams.TileSize);

	float MicroZOffset = GetZOffset(UPos, VPos, TileGenerationParams.MinorNoiseScale, TileGenerationParams.MinorNoiseOffset, TileGenerationParams.MinorNoiseStrength);
	float LargeZOffset = GetZOffset(UPos, VPos, TileGenerationParams.MajorNoiseScale, TileGenerationParams.MajorNoiseOffset, TileGenerationParams.MajorNoiseStrength);
	float CurrentZOffset = LargeZOffset + MicroZOffset;

	if (CurrentZOffset < MinZOffset) MinZOffset = CurrentZOffset;
	if (CurrentZOffset > MaxZOffset) MaxZOffset = CurrentZOffset;

	FVector CurrentLocation(CurrentXOffset, CurrentYOffset, CurrentZOffset);
	Vertices.Add(CurrentLocation);

	Normals.Add(CalculateVertexNormal(CurrentXOffset + TileGenerationParams.TileIndex.X * TileGenerationParams.TileSize, CurrentYOffset + TileGenerationParams.TileIndex.Y * TileGenerationParams.TileSize, DistanceBetweenVertices, TileGenerationParams));
	UV0.Add(FVector2D(UPos, VPos));
	VertexColor.Add(FColor(CurrentZOffset, 1 - CurrentZOffset, MicroZOffset));
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

FVector AProceduralTile::CalculateVertexNormal(float XPos, float YPos, float DistanceBetweenVertices, FTileGenerationParams TileGenerationParams) {
	float Top = MapToUV(YPos + DistanceBetweenVertices, TileGenerationParams.TileSize);
	float Bottom = MapToUV(YPos - DistanceBetweenVertices,  TileGenerationParams.TileSize);
	float Right = MapToUV(XPos + DistanceBetweenVertices, TileGenerationParams.TileSize);
	float Left = MapToUV(XPos - DistanceBetweenVertices, TileGenerationParams.TileSize);

	float ZTopLeftMajor = GetZOffset(Left, Top, TileGenerationParams.MajorNoiseScale, TileGenerationParams.MajorNoiseOffset, TileGenerationParams.MajorNoiseStrength);
	float ZTopLeftMinor = GetZOffset(Left, Top, TileGenerationParams.MinorNoiseScale, TileGenerationParams.MinorNoiseOffset, TileGenerationParams.MinorNoiseStrength);
	float ZTopLeft = ZTopLeftMajor + ZTopLeftMinor;
	
	float ZTopRightMajor = GetZOffset(Right, Top, TileGenerationParams.MajorNoiseScale, TileGenerationParams.MajorNoiseOffset, TileGenerationParams.MajorNoiseStrength);
	float ZTopRightMinor = GetZOffset(Right, Top, TileGenerationParams.MinorNoiseScale, TileGenerationParams.MinorNoiseOffset, TileGenerationParams.MinorNoiseStrength);
	float ZTopRight = ZTopRightMajor + ZTopRightMinor;
	
	float ZBottomLeftMajor = GetZOffset(Left, Bottom, TileGenerationParams.MajorNoiseScale, TileGenerationParams.MajorNoiseOffset, TileGenerationParams.MajorNoiseStrength);
	float ZBottomLeftMinor = GetZOffset(Left, Bottom, TileGenerationParams.MinorNoiseScale, TileGenerationParams.MinorNoiseOffset, TileGenerationParams.MinorNoiseStrength);
	float ZBottomLeft = ZBottomLeftMajor + ZBottomLeftMinor;
	
	float ZBottomRightMajor = GetZOffset(Right, Bottom, TileGenerationParams.MajorNoiseScale, TileGenerationParams.MajorNoiseOffset, TileGenerationParams.MajorNoiseStrength);
	float ZBottomRightMinor = GetZOffset(Right, Bottom, TileGenerationParams.MinorNoiseScale, TileGenerationParams.MinorNoiseOffset, TileGenerationParams.MinorNoiseStrength);
	float ZBottomRight = ZBottomRightMajor + ZBottomRightMinor;

	FVector TopLeftLocation(XPos - DistanceBetweenVertices, YPos + DistanceBetweenVertices, ZTopLeft);
	FVector TopRightLocation(XPos + DistanceBetweenVertices, YPos + DistanceBetweenVertices, ZTopRight);
	FVector BottomLeftLocation(XPos - DistanceBetweenVertices, YPos - DistanceBetweenVertices, ZBottomLeft);
	FVector BottomRightLocation(XPos + DistanceBetweenVertices, YPos - DistanceBetweenVertices, ZBottomRight);


	FVector Normal_01 = FVector::CrossProduct((BottomLeftLocation - TopLeftLocation), (TopRightLocation - TopLeftLocation));
	FVector Normal_02 = FVector::CrossProduct((BottomRightLocation - BottomLeftLocation), (TopRightLocation - BottomLeftLocation));
	return (Normal_02 + Normal_01 / 2).GetSafeNormal();
}

float AProceduralTile::GetZOffset(float XPos, float YPos, FVector2D NoiseScale, FVector2D NoiseOffset, float NoiseStrength) {

	float ScaledXPos = MapToRange(XPos, 0, 1, 0, NoiseScale.X);
	float ScaledYPos = MapToRange(YPos, 0, 1, 0, NoiseScale.Y);

	float ZOffset = FMath::PerlinNoise2D(FVector2D(ScaledXPos + NoiseOffset.X, ScaledYPos + NoiseOffset.Y)) * NoiseStrength;
	return ZOffset;
}

float AProceduralTile::MapToUV(float Value, int TileSize) {
	return (Value + (float(TileSize) / 2)) / TileSize;
}

bool AProceduralTile::IsGenerationFinished()
{
	if (bMarkedToDelete) return true;
	bool bIsFinished = true;
	if (TreeGenerationComponent) bIsFinished &= TreeGenerationComponent->GetIsGenerationFinished();
	if (GrassGenerationComponent) bIsFinished &= GrassGenerationComponent->GetIsGenerationFinished();
	if (BushGenerationComponent) bIsFinished &= BushGenerationComponent->GetIsGenerationFinished();
	return bIsFinished;
}

float AProceduralTile::MapToRange(float Value, float MinPrev, float MaxPrev, float MinNew, float MaxNew) {
	return MinNew + (Value - MinPrev) * (MaxNew - MinNew) / (MaxPrev - MinPrev);
}

