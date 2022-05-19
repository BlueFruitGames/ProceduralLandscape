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
	// The mesh of this foliage type
	UPROPERTY(EditAnywhere)
	UStaticMesh* FoliageMesh;

	//Determines size of other foliage instances in the radius of this foliage type
	UPROPERTY(EditAnywhere)
	UCurveFloat* GrowthCurve;

	//Is this foliage type a tree?
	UPROPERTY(EditAnywhere)
	bool bIsTree;

	//The radius of this foliage type
	UPROPERTY(EditAnywhere)
	float Radius;

	//Should uniform scaling be applied?
	UPROPERTY(EditAnywhere)
	bool bUniformScale;	

	//Uniform scale value
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bUniformScale"))
	float ScaleUniform;

	//Random diviation of the scale
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bUniformScale"))
	float ScaleRandomDiviationUniform;

	//Scale of this foliage type
	UPROPERTY(EditAnywhere, meta = (EditCondition = "!bUniformScale"))
	FVector Scale;

	//Random diviation in each dimension
	UPROPERTY(EditAnywhere, meta = (EditCondition = "!bUniformScale"))
	FVector ScaleRandomDiviation;

};
