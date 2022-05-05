// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class PROCEDURALLANDSCAPE_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	/**
	 * Moves thr character forward.
	 * 
	 * \param AxisValue current input on this input axis
	 */
	void MoveForward(float AxisValue);

	/**
	 * Moves the charcter to its right.
	 * 
	 * \param AxisValue current input on this input axis
	 */
	void MoveRight(float AxisValue);

	/**
	 * Turns a character around its Z-axis.
	 * 
	 * \param AxisValue current input on this input axis
	 */
	void Turn(float AxisValue);

	/**
	 * Turns a character around its Y-axis..
	 * 
	 * \param AxisValue current input on this input axis
	 */
	void LookUp(float AxisValue);
};
