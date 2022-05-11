// Fill out your copyright notice in the Description page of Project Settings.


#include "GrassGenerationComponent.h"

#include "Foliage/FoliageDataAsset.h"
#include "ProceduralTile.h"
#include "TreeGenerationComponent.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"

#define COLLISION_GROUND ECC_GameTraceChannel1

// Sets default values for this component's properties
UGrassGenerationComponent::UGrassGenerationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UGrassGenerationComponent::SetupGrassGeneration(int SpawnCount_In, int MaxTries_In, TArray<UFoliageDataAsset*> GrassData_In)
{
	SpawnCount = SpawnCount_In;
	MaxTries = MaxTries_In;
	GrassData = GrassData_In;
	for (UFoliageDataAsset* GrassDatum : GrassData) {
		if (!GrassDatum || !GrassDatum->FoliageMesh) continue;
		FString CurrentComponentName = FString::Printf(TEXT("HISMComponent_%s"), *GrassDatum->FoliageMesh->GetName());
		UHierarchicalInstancedStaticMeshComponent* CurrentHISMComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, UHierarchicalInstancedStaticMeshComponent::StaticClass(), FName(CurrentComponentName));
		CurrentHISMComponent->SetWorldLocation(FVector(0, 0, 0));
		CurrentHISMComponent->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
		CurrentHISMComponent->SetStaticMesh(GrassDatum->FoliageMesh);
		CurrentHISMComponent->RegisterComponent();
		HISMComponents.Add(CurrentHISMComponent);
	}
}

void UGrassGenerationComponent::GenerateGrass(TArray<FGeneratedTreeInfo> GeneratedTreeInfos, FTileIndex TileIndex, int TileSize, float TraceZStart, float TraceZEnd, int RandomSeed, bool bDrawDebug)
{
	if (HISMComponents.Num() == 0) return;
	FRandomStream RandomStream((TileIndex.X * 10000 + TileIndex.Y) * RandomSeed + RandomSeed);
	TArray<FTileBounds> TileBounds;
	TArray<FVector> GeneratedLocations;
	int Count = 0;
	int Tries = 0;

	for (UFoliageDataAsset* GrassDatum : GrassData) {
		FTileBounds NewEntry;
		NewEntry.XMin = TileIndex.X * TileSize - TileSize / 2 + GrassDatum->Radius / 2;
		NewEntry.XMax = TileIndex.X * TileSize + TileSize / 2 - GrassDatum->Radius / 2;
		NewEntry.YMin = TileIndex.Y * TileSize - TileSize / 2 + GrassDatum->Radius / 2;
		NewEntry.YMax = TileIndex.Y * TileSize + TileSize / 2 - GrassDatum->Radius / 2;

		TileBounds.Add(NewEntry);
	}

	while (Count < SpawnCount && Tries < MaxTries) {
		int HISMComponentIndex = RandomStream.RandRange(0, HISMComponents.Num() - 1);
		UHierarchicalInstancedStaticMeshComponent* HISMComponent = HISMComponents[HISMComponentIndex];
		FTileBounds CurrentBounds = TileBounds[HISMComponentIndex];

		float XPos = RandomStream.FRandRange(CurrentBounds.XMin, CurrentBounds.XMax);
		float YPos = RandomStream.FRandRange(CurrentBounds.YMin, CurrentBounds.YMax);
		FVector Location(XPos, YPos, 0);
		if (DoesOverlap(GeneratedTreeInfos, Location, GeneratedLocations, GrassData[HISMComponentIndex]->Radius)) {
			Tries += 1;
		}
		else {
			FHitResult HitResult;
			FVector Start(XPos, YPos, TraceZStart);
			FVector End(XPos, YPos, TraceZEnd);
			if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, COLLISION_GROUND)) {
				Location = HitResult.Location;
				Location.Z -= 5;
			}
			else {

			}
			FTransform Transform;
			float HalfHeight = GrassData[HISMComponentIndex]->FoliageMesh->GetBounds().GetSphere().W / 2;
			Transform.MultiplyScale3D(FVector(1));
			Transform.SetLocation(Location);
			FRotator Rotation;
			Rotation.Yaw = RandomStream.FRandRange(-180, 180);
			Transform.SetRotation(Rotation.Quaternion());
			HISMComponent->AddInstance(Transform, true);
			if (bDrawDebug) DrawDebugCylinder(GetWorld(), Transform.GetLocation(), Transform.GetLocation() + Transform.Rotator().Vector().UpVector * HalfHeight * 2, GrassData[HISMComponentIndex]->Radius, 8, FColor::Red, false, 10, 0, 2);
			GeneratedLocations.Add(Location);
			FGeneratedTreeInfo GeneratedTreeInfo;
			GeneratedTreeInfo.Location = Location;
			GeneratedTreeInfo.Radius = GrassData[HISMComponentIndex]->Radius;

			GeneratedTreeInfos.Add(GeneratedTreeInfo);

			Tries = 0;
			Count += 1;
		}
	}
}

void UGrassGenerationComponent::ClearGrass()
{
	for (UHierarchicalInstancedStaticMeshComponent* HISMComponent : HISMComponents) {
		HISMComponent->ClearInstances();
	}
}

bool UGrassGenerationComponent::DoesOverlap(TArray<FGeneratedTreeInfo> GeneratedTreeInfos, FVector NewLocation, TArray<FVector> GeneratedLocations, float GrassRadius)
{
	for (FVector CurrentLocation : GeneratedLocations) {
		float Distance = FMath::Abs(FVector::Dist(CurrentLocation, NewLocation));
		if (Distance < GrassRadius * 2) return true;
	}

	for (FGeneratedTreeInfo GeneratedTreeInfo : GeneratedTreeInfos) {
		float Distance = FMath::Abs(FVector::Dist(GeneratedTreeInfo.Location, NewLocation));
		if (Distance < GeneratedTreeInfo.Radius) return true;
	}

	return false;
}
