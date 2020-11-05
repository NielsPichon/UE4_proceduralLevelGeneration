// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Public/RessourceType.h"
#include "Components/TimelineComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Prop.generated.h"


UCLASS()
class TREASUREHUNT_API AProp : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProp();

	//CollisionCapsule
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UCapsuleComponent* CollisionCapsule;

	//Mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USkeletalMeshComponent* propMeshComponent;

	//Type of ressource this prop will yield
	UPROPERTY(EditAnywhere, Category = "Properties")
		ERessourceTypeEnum ressourceType = ERessourceTypeEnum::Wood;

	//Nb of harvesting loops to perform to deplete the ressource
	UPROPERTY(EditAnywhere, Category = "Properties")
		int harvestHit = 3;

	//does it yield ressources everyloop
	UPROPERTY(EditAnywhere, Category = "Properties")
		bool yieldEveryTime = true;

	//Max yield at each hit (or end if yieldEveryTime is set to false)
	UPROPERTY(EditAnywhere, Category = "Properties")
		int maxYield = 3;

	//Min yield at each hit (or end if yieldEveryTime is set to false)
	UPROPERTY(EditAnywhere, Category = "Properties")
		int minYield = 1;

	//Time before respawn in seconds
	UPROPERTY(EditAnywhere, Category = "Properties")
		float respawnTime = 5;

	UFUNCTION(BlueprintCallable)
		void Harvest(ERessourceTypeEnum& ressource, int& amount);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
		void Regrow();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
		void SetHarvested();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
		void TakeHit();

	//Method to snap object to closest surface vertically
	void MoveToClosestSurface();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	int lifePoints = 3;
	bool harvested = false;
	float timeSinceHarvested = 0;

};
