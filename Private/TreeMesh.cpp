// Fill out your copyright notice in the Description page of Project Settings.


#include "public/TreeMesh.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"

// Sets default values
ATreeMesh::ATreeMesh()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TreeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tree"));
	TreeMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);

	if (MeshCandidates.Num() > 0)
		TreeMesh->SetStaticMesh(MeshCandidates[rand() % MeshCandidates.Num()]);
}

// Called when the game starts or when spawned
void ATreeMesh::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATreeMesh::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

