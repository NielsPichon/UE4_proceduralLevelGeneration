// MIT License

// Copyright (c) 2020 NielsPichon

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#include "public/Prop.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
//#include "DrawDebugHelpers.h"
#include "Engine/World.h"

// Sets default values
AProp::AProp()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionCapsule"));
	/*propMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("propMeshComponent"));*/
	propMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("propMeshComponent"));
	CollisionCapsule->SetupAttachment(RootComponent);
	propMeshComponent -> SetupAttachment(CollisionCapsule);
	//CollisionCapsule -> AttachToComponent(propMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);

	//propMeshComponent->SetRelativeLocation(FVector(0, 0, -(CollisionCapsule->GetUnscaledCapsuleHalfHeight())));

	//if (propMesh)
	//	propMeshComponent->SetStaticMesh(propMesh);

	lifePoints = harvestHit;
}

void AProp::Harvest(ERessourceTypeEnum& ressource, int& amount)
{
	if (!harvested)
	{
		lifePoints--;
		if (lifePoints == 0)
		{
			SetHarvested();
			harvested = true;
			timeSinceHarvested = 0;
		}
		else
		{
			TakeHit();
		}
	}
	ressource = ressourceType;
	amount = FMath::RandRange(minYield, maxYield);
}

void AProp::MoveToClosestSurface()
{
	//find the closest underlying surface with a raycast, and move to surface
	FVector direction = FVector(0, 0, -1);
	FHitResult* hit = new FHitResult;
	FVector start = this->GetActorLocation();
	FVector end = start + direction * 10000.0f;
	FCollisionQueryParams* traceParams = new FCollisionQueryParams;
	traceParams->AddIgnoredActor(this);

	//DrawDebugLine(GetWorld(), start, end, FColor::Red, true);

	if (GetWorld()->LineTraceSingleByChannel(*hit, start, end, ECC_Visibility, *traceParams))
	{
		this->SetActorLocation(hit->Location + CollisionCapsule->GetScaledCapsuleHalfHeight());
	}
	else
	{
		//this->Destroy();
		FString debug = "No hit ";
		debug.Append(FString::SanitizeFloat(start.Z));
		debug.Append(" ");
		debug.Append(FString::SanitizeFloat(end.Z));
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, debug);

	}
}

// Called when the game starts or when spawned
void AProp::BeginPlay()
{

	Super::BeginPlay();
}

// Called every frame
void AProp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//If harvested, add time to counter
	if (harvested)
		timeSinceHarvested += DeltaTime;

	//if has been harvested for longer than respawn time, regrow
	if (timeSinceHarvested >= respawnTime && harvested)
	{
		TArray<AActor*> overlappingActors;
		CollisionCapsule->GetOverlappingActors(overlappingActors, TSubclassOf<AActor>::TSubclassOf(APawn::StaticClass()));

		if (overlappingActors.Num() == 0) {
			harvested = false;
			lifePoints = harvestHit;

			//regrow animation (simple scaleUp on timeline)
			Regrow();
		}
	}

}