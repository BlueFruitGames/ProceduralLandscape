// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FoliageDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class PROCEDURALLANDSCAPE_API UFoliageDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	UStaticMesh* FoliageMesh;

	UPROPERTY(EditAnywhere)
	UCurveFloat* GrowthCurve;

	UPROPERTY(EditAnywhere)
	bool bIsTree;

	UPROPERTY(EditAnywhere)
	float Radius;

	UPROPERTY(EditAnywhere)
	float Scale = 1;

	UPROPERTY(EditAnywhere)
	float ScaleRandomDiviation;

};
