// Fill out your copyright notice in the Description page of Project Settings.


#include "TreeGenerationComponent.h"

#include "Foliage/FoliageDataAsset.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"

#define COLLISION_GROUND ECC_GameTraceChannel1

// Sets default values for this component's properties
UTreeGenerationComponent::UTreeGenerationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTreeGenerationComponent::SetupTreeGeneration(int SpawnCount_In, int MaxTries_In, TArray<UFoliageDataAsset*> TreeData_In)
{
	SpawnCount = SpawnCount_In;
	MaxTries = MaxTries_In;
	TreeData = TreeData_In;
	for (UFoliageDataAsset* TreeDatum : TreeData) {
		if (!TreeDatum || !TreeDatum->FoliageMesh) continue;
		FString CurrentComponentName = FString::Printf(TEXT("HISMComponent_%s"), *TreeDatum->FoliageMesh->GetName());
		UHierarchicalInstancedStaticMeshComponent* CurrentHISMComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, UHierarchicalInstancedStaticMeshComponent::StaticClass(), FName(CurrentComponentName));
		CurrentHISMComponent->SetWorldLocation(FVector(0, 0, 0));
		CurrentHISMComponent->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
		CurrentHISMComponent->SetStaticMesh(TreeDatum->FoliageMesh);
		CurrentHISMComponent->RegisterComponent();
		HISMComponents.Add(CurrentHISMComponent);
	}
}

TArray<FGeneratedTreeInfo> UTreeGenerationComponent::GenerateTrees(FTileIndex TileIndex, int TileSize, float TraceZStart, float TraceZEnd, int RandomSeed, bool bDrawDebug)
{
	if (HISMComponents.Num() == 0) return TArray<FGeneratedTreeInfo>();
	FRandomStream RandomStream((TileIndex.X * 10000 + TileIndex.Y) * RandomSeed + RandomSeed);
	TArray<FTileBounds> TileBounds;
	TArray<FVector> GeneratedLocations;
	int Count = 0;
	int Tries = 0;

	for (UFoliageDataAsset* TreeDatum : TreeData) {
		FTileBounds NewEntry;
		NewEntry.XMin = TileIndex.X * TileSize - TileSize / 2 + TreeDatum->Radius / 2;
		NewEntry.XMax = TileIndex.X * TileSize + TileSize / 2 - TreeDatum->Radius / 2;
		NewEntry.YMin = TileIndex.Y * TileSize - TileSize / 2 + TreeDatum->Radius / 2;
		NewEntry.YMax = TileIndex.Y * TileSize + TileSize / 2 - TreeDatum->Radius / 2;

		TileBounds.Add(NewEntry);
	}

	TArray<FGeneratedTreeInfo> GeneratedTreeInfos;

	while (Count < SpawnCount && Tries < MaxTries) {
		int HISMComponentIndex = RandomStream.RandRange(0, HISMComponents.Num() - 1);
		UHierarchicalInstancedStaticMeshComponent* HISMComponent = HISMComponents[HISMComponentIndex];
		FTileBounds CurrentBounds = TileBounds[HISMComponentIndex];

		float XPos = RandomStream.FRandRange(CurrentBounds.XMin, CurrentBounds.XMax);
		float YPos = RandomStream.FRandRange(CurrentBounds.YMin, CurrentBounds.YMax);
		FVector Location(XPos, YPos, 0);
		if (DoesOverlap(Location, GeneratedLocations, TreeData[HISMComponentIndex]->Radius)) {
			Tries += 1;
		}
		else {
			FHitResult HitResult;
			FVector Start(XPos, YPos, TraceZStart);
			FVector End(XPos, YPos, TraceZEnd);
			if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Camera)) {
				HitResult.Location.Z -= 20;
				Location = HitResult.Location;
			}
			FTransform Transform;
			float HalfHeight = TreeData[HISMComponentIndex]->FoliageMesh->GetBounds().GetSphere().W / 2;
			Transform.MultiplyScale3D(FVector(0.5));
			Transform.SetLocation(Location);
			FRotator Rotation;
			Rotation.Yaw = RandomStream.FRandRange(-180, 180);
			Transform.SetRotation(Rotation.Quaternion());
			HISMComponent->AddInstance(Transform, true);
			if (bDrawDebug) DrawDebugCylinder(GetWorld(), Transform.GetLocation(), Transform.GetLocation() + Transform.Rotator().Vector().UpVector * HalfHeight * 2, TreeData[HISMComponentIndex]->Radius, 8, FColor::Red, false, 10, 0, 2);
			GeneratedLocations.Add(Location);
			FGeneratedTreeInfo GeneratedTreeInfo;
			GeneratedTreeInfo.Location = Location;
			GeneratedTreeInfo.Radius = TreeData[HISMComponentIndex]->Radius;

			GeneratedTreeInfos.Add(GeneratedTreeInfo);

			Tries = 0;
			Count += 1;
		}
	}
	return GeneratedTreeInfos;
}

void UTreeGenerationComponent::ClearTrees()
{
	for (UHierarchicalInstancedStaticMeshComponent* HISMComponent : HISMComponents) {
		HISMComponent->ClearInstances();
	}
}

bool UTreeGenerationComponent::DoesOverlap(FVector NewLocation, TArray<FVector> GeneratedLocations, float TreeRadius)
{
	for (FVector CurrentLocation : GeneratedLocations) {
		float Distance = FMath::Abs(FVector::Dist(CurrentLocation, NewLocation));
		if (Distance < TreeRadius * 2) return true;
	}
	return false;
}

