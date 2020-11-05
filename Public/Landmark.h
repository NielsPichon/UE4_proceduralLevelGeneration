	// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Landmark.generated.h"

UCLASS()
class TREASUREHUNT_API ALandmark : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALandmark();

	//Percentage of map height at which the ground will be for this landmark
	UPROPERTY(EditAnywhere, Category = "Properties")
		float baseHeight = 0.1;

	//radius of the landmark (in pixels)
	UPROPERTY(EditAnywhere, Category = "Properties")
		int radius = 10;

	//Mesh
	UPROPERTY(EditAnywhere, Category = "Properties")
		UStaticMeshComponent* landmarkMeshComponent;

	FVector2D mapPosition;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
