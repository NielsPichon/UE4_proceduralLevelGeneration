// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Public/Biom.h"


#include "Land.generated.h"

UCLASS()
class TREASUREHUNT_API ALand : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALand();

	//vertices of the land mesh
	UPROPERTY(BlueprintReadOnly)
		TArray<FVector> verts;

	UPROPERTY(BlueprintReadOnly)
		float globalScale = 1;

	//triangles of the land mesh
	UPROPERTY(BlueprintReadOnly)
		TArray<int> tris;

	//uvs of the land mesh
	UPROPERTY(BlueprintReadOnly)
		TArray<FVector2D> uvs;

	//position of the end verts of each edge
	UPROPERTY()
		TArray<FVector> edges;

	//ref to the corresponding biom
	UPROPERTY(BlueprintReadOnly)
		ABiom* biom;

	//Given a position and a radius, check wether an object can be spawned on the mesh
	UFUNCTION(BlueprintCallable)
		bool checkObjectFits(FVector position, float radius);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
