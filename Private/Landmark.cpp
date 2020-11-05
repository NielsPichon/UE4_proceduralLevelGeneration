// Fill out your copyright notice in the Description page of Project Settings.


#include "public/Landmark.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"

// Sets default values
ALandmark::ALandmark()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	landmarkMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Landmark"));
	landmarkMeshComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ALandmark::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALandmark::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

