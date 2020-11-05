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

