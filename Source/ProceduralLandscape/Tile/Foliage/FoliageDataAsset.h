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
	float Radius;

};
