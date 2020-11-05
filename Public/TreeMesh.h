// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TreeMesh.generated.h"

UCLASS()
class TREASUREHUNT_API ATreeMesh : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATreeMesh();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UStaticMesh*> MeshCandidates;

	//Mesh
	UPROPERTY(EditAnywhere, Category = "Properties")
		UStaticMeshComponent* TreeMesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
