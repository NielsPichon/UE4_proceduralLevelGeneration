// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Land.h"
#include "Landmark.h"
#include "Engine/Texture2D.h"
#include "Biom.h"

#include "AMapGenerator.generated.h"

USTRUCT()
struct FEdgeData
{
	GENERATED_BODY()

	UPROPERTY()
		FVector2D vertexIndex;

	UPROPERTY()
		FVector2D faceIndex;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGenerationDoneDelegate);


UCLASS()
class TREASUREHUNT_API AAMapGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	//Size of the map which is square
	UPROPERTY(EditAnywhere, Category = "Map Parameters")
		int mapSize = 256;

	//Should the seed be randomized, which will give a new map type
	UPROPERTY(EditAnywhere, Category = "Map Parameters")
		bool randomSeed = true;

	//Seed for the pseudo random number generator
	UPROPERTY(EditAnywhere, Category = "Map Parameters")
		int seed = 0;

	//Number of terraces in the level
	UPROPERTY(EditAnywhere, Category = "Map Parameters")
		int mapLevels = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Parameters")
		float globalScale = 100;

	//Vertical scale multiplier
	UPROPERTY(EditAnywhere, Category = "Map Parameters")
		float heightScale = 10;
	
	//Should a river be added in the center
	UPROPERTY(EditAnywhere, Category = "Map Parameters")
		bool AddRiver = true;

	//The higher the factor, the wider the river
	UPROPERTY(EditAnywhere, Category = "Map Parameters")
		int riverWidthFactor = 4;

	//Number of octaves to use in the Perlin noise for the map generation
	UPROPERTY(EditAnywhere, Category = "Noise Parameters")
		int octaves = 3;
	
	//Persistance of the Perlin Noise octaves. The higher the persistance, the higher the height variation of small details
	UPROPERTY(EditAnywhere, Category = "Noise Parameters")
		float persistance = 0.5;

	//Base frequency of the noise. Low frequency means larger chunks with less variation
	UPROPERTY(EditAnywhere, Category = "Noise Parameters")
		float baseFrequency = 5;

	//Exponent at which the noise is set
	UPROPERTY(EditAnywhere, Category = "Noise Parameters")
		int noiseExponent = 1;

	//Threshold in height at which bioms change
	UPROPERTY(EditAnywhere, Category = "Bioms")
		TArray< TSubclassOf<ABiom>> bioms;

	//map color for the lowest biom
	UPROPERTY(EditAnywhere, Category = "Bioms")
		FColor lowestBiomMapColor;

	//map color for the highest biom
	UPROPERTY(EditAnywhere, Category = "Bioms")
		FColor highestBiomMapColor;

	//demarcation ratio between water and sand. Not great design but because bioms are spawned on begin play 
	//I couldn't find another way
	UPROPERTY(EditAnywhere, Category = "Bioms")
		float waterLine = 0.2;

	//Mesh for clouds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
		TArray<UStaticMesh*> cloudMeshes;

	//Amount of clouds to spawn
	UPROPERTY(EditAnywhere, Category = "Props")
		int cloudAmount = 10;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FGenerationDoneDelegate LandDoneDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Events")
		FGenerationDoneDelegate PropsDoneDelegate;

	UPROPERTY(BlueprintReadOnly, Category = "Map")
		TArray<ALand*> meshes;

	//should the map texture be smoothed
	UPROPERTY(EditAnywhere, Category = "Map")
		bool smoothMapTexture = false;

	//should the map texture be contoured
	UPROPERTY(EditAnywhere, Category = "Map")
		bool contourMapTexture = false;

	//should the contour be overlayed to the map
	UPROPERTY(EditAnywhere, Category = "Map")
		bool overlayContour = false;

	//should the contour be overlayed to the map
	UPROPERTY(EditAnywhere, Category = "Map")
		FLinearColor contourColor = FLinearColor::White;

	//Landmarks to chose from
	UPROPERTY(EditAnywhere, Category = "Missions")
		TArray<TSubclassOf<ALandmark>> potentialLandmarks;

	//Amount of landmarks to randomly place on the map
	UPROPERTY(EditAnywhere, Category = "Missions")
		int amountOfLandmarks = 10;

	UPROPERTY(BlueprintReadOnly, Category = "Missions")
		TArray<ALandmark*> landmarks;

	UFUNCTION(BlueprintCallable)
		void GenerateMapData();

	UFUNCTION(BlueprintCallable)
		int GetMeshCount();

	UFUNCTION(BlueprintCallable)
		void GetMeshData(int meshIdx, TArray<FVector>& verts, TArray<int>& tris, TArray <FVector2D>& uvs, FLinearColor& color);

	UFUNCTION(BlueprintCallable)
		float GetWaterHeight();

	UFUNCTION(BlueprintCallable)
		float GetStartHeight();

	UFUNCTION(BlueprintCallable)
		void GetCloudData(TArray<UStaticMesh*>& clouds, TArray<FVector>& position);

	UFUNCTION(BlueprintCallable)
		void GenerateProps();

	UFUNCTION(BlueprintCallable)
		void SpawnClouds();

	UFUNCTION(BlueprintCallable)
		bool CheckCanDig(FVector location, float holeRadidus);

	UFUNCTION(BlueprintCallable)
		void RemoveVertsWithinRadius(TArray<FVector> inVerts, TArray<int> inTris, TArray<FVector2D> inUVs, 
			FVector center, float radius, TArray<FVector>& outVerts, TArray<int>& outTris, TArray<FVector2D>& outUVs);

	UFUNCTION(BlueprintCallable)
		UTexture2D* GenerateMapTexture(int resolution = 100);


	// Sets default values for this actor's properties
	AAMapGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	bool GenerateNoise();
	void TerraceNoise();
	void ClusterNoise();
	void GenerateMesh();
	ALand* GenerateTopMesh(TArray<FVector2D> points, int depth);
	ALand* StoreEdge(ALand* mesh, TArray<FEdgeData> edges);
	void Inflate(TArray<FVector2D>& points);
	void Erode(TArray<FVector2D>& points);
	void clampMap(TArray<FVector2D>& points);
	TArray<FEdgeData> BuildEdge(int vertexCount, TArray<int> triangleArray);
	TArray<FEdgeData> BuildManifoldEdge(ALand* mesh);
	ALand* ExtrudeMesh(ALand* mesh, TArray<float> extrusion, TArray<FEdgeData> edges, bool invertFaces);
	ALand* PerformMeshExtrusion(ALand* mesh, TArray<float> extrusionMatrix);
	ALand* RemoveDuplicateVertices(ALand* mesh);
	ALand* SmoothEdge(ALand* mesh, TArray<FEdgeData> edges);
	void GenerateClouds();
	TArray<TArray<FVector2D>> GetContours(TArray<FVector2D> points);
	TArray<TArray<FVector2D>> IsolateOutterContours(TArray<TArray<FVector2D>> contours);
	int GetWindingNumber(FVector2D point, TArray<FVector2D> contour);
	int IsLeft(FVector2D P0, FVector2D P1, FVector2D P2);
	bool IsInner(TArray<int> isInsideContour, int idx);
	void PickLandmarks();
	void MatchLandToLandmarks();
	void GenerateRockAndTrees();
	void InitBioms();
	uint8* Smooth2DMap(uint8* Data);
	uint8* Contour2DMap(uint8* Data);
	uint8* ResampleMap(uint8* Data, int originalRes, int newRes);
	
	//2D noise map
	float* noiseMap;

	//array of clusters of points per level
	TArray<TArray<TArray<FVector2D>>> clusters;

	//clouds
	TArray<UStaticMesh*> cloudsDistribution;
	TArray<FVector> cloudsPosition;

	//Bioms
	TArray<ABiom*> inGameBioms;
};

