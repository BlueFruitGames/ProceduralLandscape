// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralTile.generated.h"

UCLASS()
class PROCEDURALLANDSCAPE_API AProceduralTile : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AProceduralTile();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, meta = (UIMin = 1))
		int TileSize;

	UPROPERTY(EditAnywhere, meta = (UIMin = 2, UIMax = 100))
		int TileResolution;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY()
		class UProceduralMeshComponent* ProceduralMeshComponent;

	UFUNCTION(CallInEditor)
		void GenerateTile();

	void GenerateTriangles(TArray<int32>& Triangles, int CurrentRow, int CurrentColumn);

	void AddTriangles(TArray<int32>& Triangles, int32 V1, int32 V2, int32 V3);

};
