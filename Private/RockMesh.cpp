// Fill out your copyright notice in the Description page of Project Settings.


#include "public/RockMesh.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"

// Sets default values
ARockMesh::ARockMesh()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rock"));
	RockMesh->AttachToComponent(RootComponent,FAttachmentTransformRules::KeepWorldTransform);

	if (MeshCandidates.Num() > 0)
		RockMesh->SetStaticMesh(MeshCandidates[rand() % MeshCandidates.Num()]);

}

// Called when the game starts or when spawned
void ARockMesh::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARockMesh::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

