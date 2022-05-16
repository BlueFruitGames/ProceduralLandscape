// Fill out your copyright notice in the Description page of Project Settings.


#include "FoliageGenerationComponent.h"

#include "FoliageDataAsset.h"
#include "../ProceduralTile.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"

#define COLLISION_GROUND ECC_GameTraceChannel1

// Sets default values for this component's properties
UFoliageGenerationComponent::UFoliageGenerationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
}

void UFoliageGenerationComponent::SetupFoliageGeneration(int SpawnCount_In, int MaxTries_In, TArray<UFoliageDataAsset*> FoliageData_In, bool bCollisionEnabled)
{
	SpawnCount = SpawnCount_In;
	MaxTries = MaxTries_In;
	FoliageData = FoliageData_In;
	for (UFoliageDataAsset* FoliageDatum : FoliageData) {
		if (!FoliageDatum || !FoliageDatum->FoliageMesh) continue;
		FString CurrentComponentName = FString::Printf(TEXT("HISMComponent_%s"), *FoliageDatum->FoliageMesh->GetName());
		UHierarchicalInstancedStaticMeshComponent* CurrentHISMComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, UHierarchicalInstancedStaticMeshComponent::StaticClass(), FName(CurrentComponentName));
		CurrentHISMComponent->SetWorldLocation(FVector(0, 0, 0));
		CurrentHISMComponent->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
		CurrentHISMComponent->SetStaticMesh(FoliageDatum->FoliageMesh);
		if (!bCollisionEnabled) {
			CurrentHISMComponent->SetCastShadow(false);
			CurrentHISMComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		CurrentHISMComponent->RegisterComponent();
		HISMComponents.Add(CurrentHISMComponent);
	}
}

void UFoliageGenerationComponent::GenerateFoliage(bool bSpawnOutside, FTileIndex TileIndex, int TileSize, float TraceZStart, float TraceZEnd, int RandomSeed, TArray<FGeneratedFoliageInfo>& FoliageInfos, int SpawnBatchSize, bool bSpawnDirect, bool bDrawDebug)
{
	
	if (HISMComponents.Num() == 0) return;
	FRandomStream RandomStream((TileIndex.X * 10000 + TileIndex.Y) * RandomSeed + RandomSeed);
	TArray<FTileBounds> TileBounds;
	int Count = 0;
	int Tries = 0;

	for (UFoliageDataAsset* FoliageDatum : FoliageData) {
		FTileBounds NewEntry;
		NewEntry.XMin = TileIndex.X * TileSize - TileSize / 2 + FoliageDatum->Radius / 2;
		NewEntry.XMax = TileIndex.X * TileSize + TileSize / 2 - FoliageDatum->Radius / 2;
		NewEntry.YMin = TileIndex.Y * TileSize - TileSize / 2 + FoliageDatum->Radius / 2;
		NewEntry.YMax = TileIndex.Y * TileSize + TileSize / 2 - FoliageDatum->Radius / 2;

		TileBounds.Add(NewEntry);
	}

	TMap<UHierarchicalInstancedStaticMeshComponent*, TArray<FTransformArrayA2>> NewInstanceBatchesPerComponent;
	for (UHierarchicalInstancedStaticMeshComponent* HISMComponent : HISMComponents) {
		TArray<FTransformArrayA2> Batches;
		FTransformArrayA2 Current;
		Batches.Add(Current);
		NewInstanceBatchesPerComponent.Add(HISMComponent, Batches);
	}

	while (Count < SpawnCount && Tries < MaxTries) {
		int HISMComponentIndex = RandomStream.RandRange(0, HISMComponents.Num() - 1);
		UHierarchicalInstancedStaticMeshComponent* HISMComponent = HISMComponents[HISMComponentIndex];
		FTileBounds CurrentBounds = TileBounds[HISMComponentIndex];

		float XPos = RandomStream.FRandRange(CurrentBounds.XMin, CurrentBounds.XMax);
		float YPos = RandomStream.FRandRange(CurrentBounds.YMin, CurrentBounds.YMax);
		FVector Location(XPos, YPos, 0);
		bool bDoesOverlap = DoesOverlap(Location, FoliageInfos, FoliageData[HISMComponentIndex]->Radius);
		if ( (bSpawnOutside && bDoesOverlap) || (!bSpawnOutside && !bDoesOverlap)) {
			Tries += 1;
			UE_LOG(LogTemp, Warning, TEXT("%d"), Tries);
		}
		else {
			FHitResult HitResult;
			FVector Start(XPos, YPos, TraceZStart);
			FVector End(XPos, YPos, TraceZEnd);
			if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, COLLISION_GROUND)) {
				HitResult.Location.Z -= 20;
				Location = HitResult.Location;
			}
			FTransform Transform;
			float HalfHeight = FoliageData[HISMComponentIndex]->FoliageMesh->GetBounds().GetSphere().W / 2;
			float Scale = RandomStream.FRandRange(FoliageData[HISMComponentIndex]->Scale - FoliageData[HISMComponentIndex]->ScaleRandomDiviation, FoliageData[HISMComponentIndex]->Scale + FoliageData[HISMComponentIndex]->ScaleRandomDiviation);
			Transform.MultiplyScale3D(FVector(Scale));
			Transform.SetLocation(Location);
			FRotator Rotation;
			Rotation.Yaw = RandomStream.FRandRange(-180, 180);
			Transform.SetRotation(Rotation.Quaternion());
			if (bDrawDebug) DrawDebugCylinder(GetWorld(), Transform.GetLocation(), Transform.GetLocation() + Transform.Rotator().Vector().UpVector * HalfHeight * 2, FoliageData[HISMComponentIndex]->Radius, 8, FColor::Red, false, 10, 0, 2);
			FGeneratedFoliageInfo GeneratedFoliageInfo;
			GeneratedFoliageInfo.Location = Location;
			GeneratedFoliageInfo.Radius = FoliageData[HISMComponentIndex]->Radius;
			FoliageInfos.Add(GeneratedFoliageInfo);
			
			if (bSpawnDirect) {
				HISMComponent->AddInstance(Transform, true);
			}
			else {
				TArray<FTransformArrayA2>* ComponentBatches = NewInstanceBatchesPerComponent.Find(HISMComponent);
				if ((*ComponentBatches)[ComponentBatches->Num() - 1].Num() < SpawnBatchSize) {
					(*ComponentBatches)[ComponentBatches->Num() - 1].Add(Transform);
				}
				else {
					FTransformArrayA2 NewBatch;
					NewBatch.Add(Transform);
					ComponentBatches->Add(NewBatch);
				}
			}


			Tries = 0;
			Count += 1;
		}
	}

	InstanceBatchesPerComponent = NewInstanceBatchesPerComponent;

	return;
}

bool UFoliageGenerationComponent::SpawnSingleBatch()
{
	TSet<UHierarchicalInstancedStaticMeshComponent*> Keys;
	InstanceBatchesPerComponent.GetKeys(Keys);

	if (Keys.Num() <= 0) return false;
	UHierarchicalInstancedStaticMeshComponent*  HISMComponent = Keys.Array()[0];

	TArray<FTransformArrayA2>* Batches = InstanceBatchesPerComponent.Find(HISMComponent);
	HISMComponent->AddInstances((*Batches)[0], false, true);
	Batches->RemoveAt(0);
	if (Batches->Num() <= 0) {
		InstanceBatchesPerComponent.Remove(HISMComponent);
		Keys.Array().Remove(0);
	}
	if (Keys.Array().Num() <= 0) {
		return false;
	}
	return true;	
}

void UFoliageGenerationComponent::SpawnAllBatches()
{
	TSet<UHierarchicalInstancedStaticMeshComponent*> Keys;
	InstanceBatchesPerComponent.GetKeys(Keys);

	if (Keys.Num() <= 0) return;

	for (UHierarchicalInstancedStaticMeshComponent* HISMComponent : Keys) {
		TArray<FTransformArrayA2>* Batches = InstanceBatchesPerComponent.Find(HISMComponent);
		while (Batches->Num() > 0) {
			HISMComponent->AddInstances((*Batches)[0], false, true);
			Batches->RemoveAt(0);
		}
		InstanceBatchesPerComponent.Remove(HISMComponent);
	}
}

void UFoliageGenerationComponent::ClearFoliage()
{
	for (UHierarchicalInstancedStaticMeshComponent* HISMComponent : HISMComponents) {
		HISMComponent->ClearInstances();
	}
}

bool UFoliageGenerationComponent::DoesOverlap(FVector NewLocation, TArray<FGeneratedFoliageInfo>& FoliageInfos, float CurrentFoliageRadius)
{
	for (FGeneratedFoliageInfo& CurrentFoliageInfo : FoliageInfos) {
		float Distance = FMath::Abs(FVector::Dist(CurrentFoliageInfo.Location, NewLocation));
		if (Distance < FMath::Max(CurrentFoliageRadius, CurrentFoliageInfo.Radius)) return true;
	}
	return false;
}



