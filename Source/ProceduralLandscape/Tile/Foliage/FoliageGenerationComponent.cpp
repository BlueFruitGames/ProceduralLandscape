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

void UFoliageGenerationComponent::SetupFoliageGeneration(int SpawnCount_In, int MaxTries_In, int BatchSize_In, TArray<UFoliageDataAsset*> FoliageData_In, int RandomSeed_In, bool bCollisionEnabled)
{
	SpawnCount = SpawnCount_In;
	MaxTries = MaxTries_In;
	BatchSize = BatchSize_In;
	RandomSeed = RandomSeed_In;
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

void UFoliageGenerationComponent::GenerateFoliage(bool bSpawnDirect, FTileIndex TileIndex, int TileSize, float TraceZStart, float TraceZEnd, TArray<FGeneratedFoliageInfo>& FoliageInfos, bool bDrawDebug)
{
	Lock.Lock();
	TransformsToUpdate.Empty();
	TransformsToAdd.Empty();
	IndicesToRemove.Empty();
	TArray<int> UpdatedIndices;

	for (UHierarchicalInstancedStaticMeshComponent* HISMComponent : HISMComponents) {
		TransformsToUpdate.Add(FTransformArrayA2());
		TransformsToAdd.Add(FTransformArrayA2());
		IndicesToRemove.Add(TArray<int>());
		bRemoveInstances.Add(true);
		UpdatedIndices.Add(0);
	}


	if (HISMComponents.Num() == 0) return;
	FRandomStream RandomStream((TileIndex.X * 10000 + TileIndex.Y) * RandomSeed + RandomSeed);
	TArray<FTileBounds> TileBounds = InitializeBounds(TileIndex, TileSize);
	int Count = 0;
	int Tries = 0;

	while (Count < SpawnCount && Tries < MaxTries) {
		int HISMComponentIndex;
		FVector Location;
		GenerateRandomInstance(TileBounds, RandomStream, TraceZStart, TraceZEnd, HISMComponentIndex, Location);

		float Distance;
		float ClosestRadius;
		UCurveFloat* GrowthCurve = nullptr;
		bool bDoesOverlap = DoesOverlap(Location, FoliageInfos, FoliageData[HISMComponentIndex]->Radius, Distance, ClosestRadius, GrowthCurve);

		float GrowthFactor = 1;
		if (bDoesOverlap && GrowthCurve) {
			float NormalizedDistance = Distance / ClosestRadius;
			GrowthFactor = GrowthCurve->GetFloatValue(NormalizedDistance);
		}		

		if (bDoesOverlap && (!GrowthCurve || FoliageData[HISMComponentIndex]->bIsTree)) {
			Tries += 1;
		}
		else {
			FTransform Transform;
			float HalfHeight = FoliageData[HISMComponentIndex]->FoliageMesh->GetBounds().GetSphere().W / 2;
			float Scale = RandomStream.FRandRange(FoliageData[HISMComponentIndex]->Scale - FoliageData[HISMComponentIndex]->ScaleRandomDiviation, FoliageData[HISMComponentIndex]->Scale + FoliageData[HISMComponentIndex]->ScaleRandomDiviation);
			Transform.MultiplyScale3D(FVector(Scale * GrowthFactor));
			Transform.SetLocation(Location);
			FRotator Rotation;
			Rotation.Yaw = RandomStream.FRandRange(-180, 180);
			Transform.SetRotation(Rotation.Quaternion());
			if (bDrawDebug) DrawDebugCylinder(GetWorld(), Transform.GetLocation(), Transform.GetLocation() + Transform.Rotator().Vector().UpVector * HalfHeight * 2, FoliageData[HISMComponentIndex]->Radius, 8, FColor::Red, false, 10, 0, 2);
			FGeneratedFoliageInfo GeneratedFoliageInfo;
			GeneratedFoliageInfo.Location = Location;
			GeneratedFoliageInfo.Radius = FoliageData[HISMComponentIndex]->Radius;
			GeneratedFoliageInfo.GrowthCurve = FoliageData[HISMComponentIndex]->GrowthCurve;
			GeneratedFoliageInfo.bIsTree = FoliageData[HISMComponentIndex]->bIsTree;
			FoliageInfos.Add(GeneratedFoliageInfo);
			
			if (bSpawnDirect) {
				HISMComponents[HISMComponentIndex]->AddInstance(Transform, true);
			}
			else if (UpdatedIndices[HISMComponentIndex] >= HISMComponents[HISMComponentIndex]->GetInstanceCount()) {
				TransformsToAdd[HISMComponentIndex].Add(Transform);
				bRemoveInstances[HISMComponentIndex] = false;
			}
			else {
				TransformsToUpdate[HISMComponentIndex].Add(Transform);
			}
			
	
			UpdatedIndices[HISMComponentIndex] += 1;
			Tries = 0;
			Count += 1;
		}
	}

	for (int i = 0; i < HISMComponents.Num(); ++i) {
		UHierarchicalInstancedStaticMeshComponent* HISMComponent = HISMComponents[i];
		while (UpdatedIndices[i] < HISMComponent->GetInstanceCount()) {
			IndicesToRemove[i].Add(UpdatedIndices[i]);
			UpdatedIndices[i] += 1;
		}
	}

	Lock.Unlock();
}

bool UFoliageGenerationComponent::UpdateFoliage()
{
	bool bSuccess = true;
	if (Lock.TryLock()) {
		for (int i = 0; i < HISMComponents.Num(); ++i) {
			if (TransformsToUpdate[i].Num() > 0) {
				HISMComponents[i]->BatchUpdateInstancesTransforms(0, TransformsToUpdate[i], true);
				TransformsToUpdate[i].Empty();
			}
			if (IndicesToRemove[i].Num() > 0 && bRemoveInstances[i]) {
				HISMComponents[i]->RemoveInstances(IndicesToRemove[i]);
				IndicesToRemove[i].Empty();
				UE_LOG(LogTemp, Warning, TEXT("REmove"));
			}
			else if(TransformsToAdd[i].Num() > 0 && !bRemoveInstances[i]){
				HISMComponents[i]->AddInstances(TransformsToAdd[i], false, true);
				TransformsToAdd[i].Empty();
				UE_LOG(LogTemp, Warning, TEXT("Add"));
			}
			if(TransformsToUpdate[i].Num() > 0 || IndicesToRemove[i].Num() > 0 || TransformsToAdd[i].Num() > 0) {
				bSuccess = false;
			}
		}
		Lock.Unlock();
	}	
	else {
		bSuccess = false;
	}
	return bSuccess;
}

bool UFoliageGenerationComponent::DoesOverlap(FVector NewLocation, TArray<FGeneratedFoliageInfo> FoliageInfos, float CurrentFoliageRadius, float& Distance, float& ClosestRadius, UCurveFloat*& GrowthCurve)
{
	bool bDoesOverlap = false;
	float CurrentDistance;
	Distance = TNumericLimits<float>::Max();
	for (FGeneratedFoliageInfo& CurrentFoliageInfo : FoliageInfos) {
		CurrentDistance = FMath::Abs(FVector::Dist(CurrentFoliageInfo.Location, NewLocation));
		if (CurrentDistance < FMath::Max(CurrentFoliageRadius, CurrentFoliageInfo.Radius)) {
			bDoesOverlap = true;
			if (CurrentDistance < Distance && CurrentFoliageInfo.bIsTree) {
				Distance = CurrentDistance;
				ClosestRadius = CurrentFoliageInfo.Radius;
				GrowthCurve = CurrentFoliageInfo.GrowthCurve;
			}
		}
	}
	return bDoesOverlap;
}

TArray<FTileBounds> UFoliageGenerationComponent::InitializeBounds(FTileIndex TileIndex, int TileSize)
{
	TArray<FTileBounds> TileBounds;

	for (UFoliageDataAsset* FoliageDatum : FoliageData) {
		FTileBounds NewEntry;
		NewEntry.XMin = TileIndex.X * TileSize - TileSize / 2 + FoliageDatum->Radius / 2;
		NewEntry.XMax = TileIndex.X * TileSize + TileSize / 2 - FoliageDatum->Radius / 2;
		NewEntry.YMin = TileIndex.Y * TileSize - TileSize / 2 + FoliageDatum->Radius / 2;
		NewEntry.YMax = TileIndex.Y * TileSize + TileSize / 2 - FoliageDatum->Radius / 2;

		TileBounds.Add(NewEntry);
	}

	return TileBounds;
}

void UFoliageGenerationComponent::GenerateRandomInstance(TArray<FTileBounds> TileBounds, FRandomStream& RandomStream, float TraceZStart, float TraceZEnd, int& HISMComponentIndex, FVector& Location)
{
	HISMComponentIndex = RandomStream.RandRange(0, HISMComponents.Num() - 1);
	UHierarchicalInstancedStaticMeshComponent* HISMComponent = HISMComponents[HISMComponentIndex];
	FTileBounds CurrentBounds = TileBounds[HISMComponentIndex];

	float XPos = RandomStream.FRandRange(CurrentBounds.XMin, CurrentBounds.XMax);
	float YPos = RandomStream.FRandRange(CurrentBounds.YMin, CurrentBounds.YMax);
	Location = FVector(XPos, YPos, 0);
	FHitResult HitResult;
	FVector Start(XPos, YPos, TraceZStart);
	FVector End(XPos, YPos, TraceZEnd);
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, COLLISION_GROUND)) {
		HitResult.Location.Z -= 1;
		Location = HitResult.Location;
	}
}



