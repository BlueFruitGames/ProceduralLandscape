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

void UFoliageGenerationComponent::SetupFoliageGeneration(bool bUseCulling, float CullDistance, int SpawnCount_In, int MaxTries_In, int BatchSize_In, TArray<UFoliageDataAsset*> FoliageData_In, int RandomSeed_In, bool bCollisionEnabled)
{
	SpawnCount = SpawnCount_In;
	MaxTries = MaxTries_In;
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
		if (bUseCulling) {
			CurrentHISMComponent->SetCullDistances(0, CullDistance);
		}
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
	InstancesToSpawn.Empty();

	for (UHierarchicalInstancedStaticMeshComponent* HISMComponent : HISMComponents) {
		InstancesToSpawn.Add(FTransformArrayA2());
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
			FVector Scale;
			if (FoliageData[HISMComponentIndex]->bUniformScale) {
				float Rand = RandomStream.FRandRange(-FoliageData[HISMComponentIndex]->ScaleRandomDiviationUniform, FoliageData[HISMComponentIndex]->ScaleRandomDiviationUniform);
				Scale = FVector(FoliageData[HISMComponentIndex]->ScaleUniform + Rand);
			}
			else {
				float ScaleX = RandomStream.FRandRange(FoliageData[HISMComponentIndex]->Scale.X - FoliageData[HISMComponentIndex]->ScaleRandomDiviation.X, FoliageData[HISMComponentIndex]->Scale.X + FoliageData[HISMComponentIndex]->ScaleRandomDiviation.X);
				float ScaleY = RandomStream.FRandRange(FoliageData[HISMComponentIndex]->Scale.Y - FoliageData[HISMComponentIndex]->ScaleRandomDiviation.Y, FoliageData[HISMComponentIndex]->Scale.Y + FoliageData[HISMComponentIndex]->ScaleRandomDiviation.Y);
				float ScaleZ = RandomStream.FRandRange(FoliageData[HISMComponentIndex]->Scale.Z - FoliageData[HISMComponentIndex]->ScaleRandomDiviation.Z, FoliageData[HISMComponentIndex]->Scale.Z + FoliageData[HISMComponentIndex]->ScaleRandomDiviation.Z);
				Scale = FVector(ScaleX, ScaleY, ScaleZ);
			}
			
			Transform.MultiplyScale3D(Scale * GrowthFactor);
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
				HISMComponents[HISMComponentIndex]->AddInstance(Transform);
			}
			else {
				InstancesToSpawn[HISMComponentIndex].Add(Transform);
			}

			Tries = 0;
			Count += 1;
		}
	}

	Lock.Unlock();
}

void UFoliageGenerationComponent::ClearFoliage()
{
	for (UHierarchicalInstancedStaticMeshComponent* HISMComponent : HISMComponents) {
		HISMComponent->ClearInstances();
	}
}

bool UFoliageGenerationComponent::UpdateFoliage()
{
	bool bSuccess = true;
	if (Lock.TryLock()) {
		for (int i = 0; i < HISMComponents.Num(); ++i) {
			if (InstancesToSpawn[i].Num() > 0) {
				if (InstancesToSpawn[i].Num() <= BatchSize) {
					HISMComponents[i]->AddInstances(InstancesToSpawn[i], false, true);
					InstancesToSpawn[i].Empty();
				}
				else {
					FTransformArrayA2 CurrentBatch;
					for (int j = 0; j < BatchSize; ++j) {
						CurrentBatch.Add(InstancesToSpawn[i][0]);
						InstancesToSpawn[i].RemoveAt(0);
					}
					HISMComponents[i]->AddInstances(CurrentBatch, false, true);
					bSuccess = false;
				}
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



