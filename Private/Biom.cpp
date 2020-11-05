// Fill out your copyright notice in the Description page of Project Settings.


#include "public/Biom.h"

// Sets default values
ABiom::ABiom()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

TSubclassOf<AProp> ABiom::GetRandomProp()
{
	if (ressources.Num() > 0)
	{
		//Make sure each ressource is assigned a spawn probability
		while (spawnProbabilities.Num() < ressources.Num())
			spawnProbabilities.Add(0);

		//then make sure that the sum of all propbabilities is below one
		float s = 0;
		for (float probability : spawnProbabilities)
		{
			s += probability;
		}
		if (s > 1)
		{
			for (float probability : spawnProbabilities)
			{
				probability /= s;
			}
		}

		//random float between 0 and 1
		float proba = (float)(rand() % 1000) / 1000.0;

		//find the object to spawn by sampling the ressources based on their individual probability
		int idx = 0;
		s = spawnProbabilities[0];
		while (proba > s)
		{
			idx++;
			if (idx >= spawnProbabilities.Num())
				break;
			s += spawnProbabilities[idx];
		}

		if (idx >= spawnProbabilities.Num())
			return nullptr;
		else
			return ressources[idx];
	}
	else
		return nullptr;
}

// Called when the game starts or when spawned
void ABiom::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABiom::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

