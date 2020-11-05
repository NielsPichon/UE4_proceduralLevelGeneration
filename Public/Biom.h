// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Public/Prop.h"
#include "Biom.generated.h"

UCLASS()
class TREASUREHUNT_API ABiom : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABiom();

	//Threshold in height at which bioms change
	UPROPERTY(EditAnywhere, Category = "Bioms")
		float biomSeparation;

	//Color of each biom. If the length is different from that of the previous vector, a random color will be used
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bioms")
		FLinearColor biomColor;

	//Color of each biom. If the length is different from that of the previous vector, a random color will be used
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bioms")
		FLinearColor biomMapColor;

	//Ressources to pick from
	UPROPERTY(EditAnywhere, Category = "Bioms")
		TArray<TSubclassOf<AProp>> ressources;
	
	//Probability to spawn each ressource type
	UPROPERTY(EditAnywhere, Category = "Bioms")
		TArray<float> spawnProbabilities;

	TSubclassOf<AProp> GetRandomProp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
