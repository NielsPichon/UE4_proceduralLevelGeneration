// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RockMesh.generated.h"

UCLASS()
class TREASUREHUNT_API ARockMesh : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARockMesh();
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UStaticMesh*> MeshCandidates;

	//Mesh
	UPROPERTY(EditAnywhere, Category = "Properties")
		UStaticMeshComponent* RockMesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
