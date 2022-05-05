// Fill out your copyright notice in the Description page of Project Settings.


#include "TreeGenerationComponent.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"

// Sets default values for this component's properties
UTreeGenerationComponent::UTreeGenerationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UTreeGenerationComponent::SetupTreeGeneration(int SpawnCount_In, float TreeRadius_In, int MaxTries_In, TArray<UStaticMesh*> TreeMeshes_In)
{
	SpawnCount = SpawnCount_In;
	TreeRadius = TreeRadius_In;
	MaxTries = MaxTries_In;
	TreeMeshes = TreeMeshes_In;
	for (UStaticMesh* TreeMesh : TreeMeshes) {
		if (!TreeMesh) continue;
		FString CurrentComponentName = FString::Printf(TEXT("HISMComponent_%s"), *TreeMesh->GetName());
		UHierarchicalInstancedStaticMeshComponent* CurrentHISMComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, UHierarchicalInstancedStaticMeshComponent::StaticClass(), FName(CurrentComponentName));
		CurrentHISMComponent->SetWorldLocation(FVector(0, 0, 0));
		CurrentHISMComponent->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
		CurrentHISMComponent->SetStaticMesh(TreeMesh);
		CurrentHISMComponent->RegisterComponent();
		HISMComponents.Add(CurrentHISMComponent);
	}
}

void UTreeGenerationComponent::GenerateTrees(FTileIndex TileIndex, int TileSize, float TraceZStart, float TraceZEnd, int RandomSeed)
{
	if (HISMComponents.Num() == 0) return;
	float XMin = TileIndex.X * TileSize - TileSize / 2 + TreeRadius;
	float XMax = TileIndex.X * TileSize + TileSize / 2 - TreeRadius;
	float YMin = TileIndex.Y * TileSize - TileSize / 2 + TreeRadius;
	float YMax = TileIndex.Y * TileSize + TileSize / 2 - TreeRadius;

	FRandomStream RandomStream((TileIndex.X * 10000 + TileIndex.Y) * RandomSeed + RandomSeed);
	TArray<FVector> GeneratedLocations;
	int Count = 0;
	int Tries = 0;
	while (Count < SpawnCount && Tries < MaxTries) {
		float XPos = RandomStream.FRandRange(XMin, XMax);
		float YPos = RandomStream.FRandRange(YMin, YMax);
		FVector Location(XPos, YPos, 0);
		if (DoesOverlap(Location, GeneratedLocations)) {
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
			//DrawDebugSphere(GetWorld(), Location, TreeRadius, 8, FColor::Red, false, 10, 0, 2);
			FTransform Transform;
			Transform.MultiplyScale3D(FVector(0.5));
			Transform.SetLocation(Location);
			int HISMComponentIndex = RandomStream.RandRange(0, HISMComponents.Num() - 1);
			UHierarchicalInstancedStaticMeshComponent* HISMComponent = HISMComponents[HISMComponentIndex];
			int InstanceIndex = HISMComponent->AddInstance(Transform, true);

			GeneratedLocations.Add(Location);
			Tries = 0;
			Count += 1;
		}
	}
}

void UTreeGenerationComponent::ClearTrees()
{
	for (UHierarchicalInstancedStaticMeshComponent* HISMComponent : HISMComponents) {
		HISMComponent->ClearInstances();
	}
}

bool UTreeGenerationComponent::DoesOverlap(FVector NewLocation, TArray<FVector> GeneratedLocations)
{
	for (FVector CurrentLocation : GeneratedLocations) {
		float Distance = FMath::Abs(FVector::Dist(CurrentLocation, NewLocation));
		if (Distance < TreeRadius * 2) return true;
	}
	return false;
}

