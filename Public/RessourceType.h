// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class ERessourceTypeEnum : uint8
{
	Wood	UMETA(DisplayName = "Wood"),
	Stone UMETA(DisplayName = "Stone"),
	Fish	UMETA(DisplayName = "Fish"),
	Flower UMETA(DisplayName = "Flower"),
	Fruit UMETA(DisplayName = "Fruit"),
	None UMETA(DisplayName = "None")
};

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class ECollectionMethodEnum : uint8
{
	chopWood	UMETA(DisplayName = "Chop Wood"),
	PickAxe UMETA(DisplayName = "Pick Axe"),
	Fish	UMETA(DisplayName = "Fish"),
	PickFlower UMETA(DisplayName = "Pick Flower")
};