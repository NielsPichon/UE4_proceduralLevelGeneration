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


#include "public/AMapGenerator.h"

#include <cstdlib>
#include "Public/PerlinNoiseGeneration.h"
#include "Engine/StaticMesh.h"
#include <cmath>
#include "public/Prop.h"

// Sets default values
AAMapGenerator::AAMapGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AAMapGenerator::GenerateMapData()
{
	//init Bioms
	InitBioms();

		//Generate Noise Map
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Generating Noise Map"));
		
		if (GenerateNoise()) {

			//Generate Landmarks
			if (GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Picking landmarks"));
			PickLandmarks();

			//Have Landmarks impact noise
			if (GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Matching noise to landmarks"));
			MatchLandToLandmarks();

			//Terrace Noise
			if (GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Terracing noise"));
			TerraceNoise();

			//Cluster Noise
			if (GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Clustering noise"));
			ClusterNoise();

			//Generate Mesh
			if (GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Generating Mesh"));
			GenerateMesh();
		}

	//When done call Done event
	LandDoneDelegate.Broadcast();
}

int AAMapGenerator::GetMeshCount()
{
	return meshes.Num();
}

// A utility function to retrieve the geometry and color from a land
void AAMapGenerator::GetMeshData(int meshIdx, TArray<FVector>& verts, TArray<int>& tris, TArray<FVector2D>& uvs, FLinearColor& color)
{
	verts = meshes[meshIdx]->verts;
	tris = meshes[meshIdx]->tris;
	uvs = meshes[meshIdx]->uvs;
	color = meshes[meshIdx]->biom->biomColor;
}

// Returns the height of the separation between water and sand + one half of a level
float AAMapGenerator::GetWaterHeight()
{
	return (floor((mapLevels - 1) * waterLine) + 0.5) * heightScale * globalScale;
}


// Returns the height of the player start (max map height plus some handpicked height).
float AAMapGenerator::GetStartHeight()
{
	// TODO: make the 40 into a parameter
	return (mapLevels + 40) * heightScale * globalScale;
}

// Returns the position and type of the clouds
void AAMapGenerator::GetCloudData(TArray<UStaticMesh*>& clouds, TArray<FVector>& position)
{
	clouds = cloudsDistribution;
	position = cloudsPosition;
}

// Uses the perlin noise generator to generate the terrain
bool AAMapGenerator::GenerateNoise()
{
	//init the noise map to the right height
	noiseMap = new float[mapSize * mapSize];

	//if we want a random seed, randomize the seed
	if (randomSeed)
		seed = rand();

	//Create Perlin Noise Generator
	PerlinNoiseGeneration noiseGenerator = PerlinNoiseGeneration(seed, mapSize, octaves, persistance, baseFrequency);

	float minNoise = 100;
	float maxNoise = -1;
	//for each pixel generate a random value between 0 and the Number of plateau and normalize to values between 0 and 1
	for (int i = 0; i < mapSize; i++)
	{
		for (int j = 0; j < mapSize; j++) 
		{
			noiseMap[i * mapSize + j] = noiseGenerator.PerlinNoiseValue(i,j);

			// Store the current max and min for latter normalisation
			if (noiseMap[i * mapSize + j] > maxNoise) maxNoise = noiseMap[i * mapSize + j];
			if (noiseMap[i * mapSize + j] < minNoise) minNoise = noiseMap[i * mapSize + j];
		}
	}

	//normalize noise and create river
	for (int i = 0; i < mapSize; i++)
	{
		for (int j = 0; j < mapSize; j++)
		{
			//normalize (with some power to allow extra control over terrian steepness)
			noiseMap[i * mapSize + j] = FMath::Pow((noiseMap[i * mapSize + j] - minNoise) / (maxNoise - minNoise), noiseExponent);

			//Add river by forcing the center of the map to go to 0
			if (AddRiver) {
				float distToCenterX = fmin(1, riverWidthFactor * abs(j - ((float)mapSize / 2.0)) / ((float)mapSize / 2.0));
				noiseMap[i * mapSize + j] = noiseMap[i * mapSize + j] * distToCenterX;
			}
		}
	}

	return true;
}


// Simply clips the noise to the floor int value in order to create some terraces
void AAMapGenerator::TerraceNoise()
{
	for (int i = 0; i < mapSize; i++)
	{
		for (int j = 0; j < mapSize; j++)
		{
			//terrace
			noiseMap[i * mapSize + j] = floor(noiseMap[i * mapSize + j] * (mapLevels - 1)) / (float)(mapLevels - 1);
		}
	}
}

void AAMapGenerator::ClusterNoise()
{
	//init arrays
	clusters = TArray<TArray<TArray<FVector2D>>>();
	for (int i = 0; i < mapLevels; i++) {
		clusters.Add(TArray<TArray<FVector2D>>());
	}

	//for each pixel of the map
	for (int i = 0; i < mapSize; i++) {
		for (int j = 0; j < mapSize; j++) {

			int noiseValue = (int)floor(noiseMap[i * mapSize + j] * (mapLevels - 1));

			for (int k = 0; k <= noiseValue; k++)
			{
				//Add the new point in the right island
				if (clusters[k].Num() > 0)
				{
					clusters[k][0].Add(FVector2D(j, i));
					
					// The below commented method separates the noise in actual clusters of adjacent points.
					// This is not of use in this project anymore but might be useful latter
					
					//TArray<int> addedToList = TArray<int>();
					//int idx = 0;

					////if there already are some islands at that depth, look for islands containing neighbour and add to them
					//for(int k = 0; k < clusters[noiseValue].Num(); k++)
					//{
					//	TArray<FVector2D> cluster = clusters[noiseValue][k];

					//	//only test for pixels on left and top because the recursion starts from 0 and moves up
					//	if (cluster.Contains(FVector2D(j-1, i)) || cluster.Contains(FVector2D(j, i - 1)))
					//	{
					//		//only add to the first found parent to avoid duplication of the vertex
					//		if (addedToList.Num() == 0)
					//			cluster.Add(FVector2D(j, i));
					//		//still register as part of multiple clusters for future merging
					//		addedToList.Add(idx);
					//	}
					//	idx++;
					//}
					//if (addedToList.Num() > 1)
					//{
					//	//if has neighbours in multiple clusters, merge clusters
					//	for (int k = addedToList.Num() - 1; k > 0; k--)
					//	{
					//		clusters[noiseValue][addedToList[0]].Append(clusters[noiseValue][addedToList[k]]);
					//		clusters[noiseValue].RemoveAt(addedToList[k]);
					//	}
					//}
					//else if (addedToList.Num() == 0)
					//{
					//	//if was added to no cluster, it means it is in a new one
					//	TArray<FVector2D> newCluster = TArray<FVector2D>();
					//	newCluster.Add(FVector2D(j, i));
					//	clusters[noiseValue].Add(newCluster);
					//}
				}
				else
				{
					//if there is no cluster at that depth yet, add the point to a new island
					TArray<FVector2D> newCluster = TArray<FVector2D>();
					newCluster.Add(FVector2D(j, i));
					clusters[k].Add(newCluster);
				}
			}
		}
	}
}


// Turns a 2D cluster of points into a mesh by extrusion along the z axis
void AAMapGenerator::GenerateMesh()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, TEXT("Adding top meshes"));
	//Start by generating a top mesh for each cluster 
	int depth = 0;
	for (TArray<TArray<FVector2D>> cluster : clusters) {
		for (TArray<FVector2D> points : cluster) {
			meshes.Add(AAMapGenerator::GenerateTopMesh(points, depth));

			//find which biom the mesh is in
			int itt = 0;
			while (floor(inGameBioms[itt]->biomSeparation * (mapLevels-1)) < depth) itt++;

			meshes[meshes.Num() - 1]->biom = inGameBioms[itt];
		}
		depth++;
	}

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, TEXT("Extrude meshes"));

	//prepare extrusion matrices
	TArray<float> extrudeHeight = TArray<float>();
	extrudeHeight.Add(0);
	extrudeHeight.Add(-heightScale);

	//Then extrude each mesh
	for (int i = 1; i < meshes.Num(); i++) {
		meshes[i] = AAMapGenerator::PerformMeshExtrusion(meshes[i], extrudeHeight);
	}

	//finally scale meshes
	for (int i = 0; i < meshes.Num(); i++) {
		for (int j = 0; j < meshes[i]->verts.Num(); j++)
		{
			meshes[i]->verts[j] *= globalScale;
		}
	}
}

// Randomly selects and spawns rocks, trees and clouds (and in the future, some pickups, collectibles and other resources)
void AAMapGenerator::GenerateProps()
{
	AAMapGenerator::GenerateClouds();
	AAMapGenerator::GenerateRockAndTrees();

	//When done call Done event
	PropsDoneDelegate.Broadcast();
}


// Randomly spawn clouds
void AAMapGenerator::SpawnClouds()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Spawning Clouds"));


	for (int i = 0; i < cloudsDistribution.Num(); i++) {
		UStaticMeshComponent* newCloud = NewObject<UStaticMeshComponent>(this);
		if (newCloud) {
			newCloud->RegisterComponent();
			newCloud->SetWorldLocation(cloudsPosition[i] * globalScale);
			newCloud->SetWorldScale3D(FVector(5, 5, 5));
			newCloud->SetStaticMesh(cloudsDistribution[i]);
			newCloud->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}


// This is a buggy method to check whether a location is diggable, that is whether it is on ground and not
// to close to the edge.
// TODO: It is not working right now (may spawn overlapping an edge) and I am wondering whether
// this is because of fp issues or simply flawed logic
bool AAMapGenerator::CheckCanDig(FVector location, float holeRadius)
{
	bool canDig = true;

7	// cast the location and scale of hole to the map referential
	float Xc = location.X / globalScale;
	float Yc = location.Y / globalScale;

	float radius = holeRadius / globalScale;

	for (int i = 0; i < 10; i++) 
	{
		for (int j = 0; j < 10; j++) {
			float X = Xc + i * radius / 9 - radius * 0.5;
			float Y = Yc + j * radius / 9 - radius * 0.5;
			if (noiseMap[(int)(round(Y) * mapSize + round(X))]!= noiseMap[(int)(round(Yc) * mapSize + round(Xc))])
			{
				canDig = false;
				break;
			}
		}

		if (!canDig)
			break;
	}

	return canDig;
}

void AAMapGenerator::RemoveVertsWithinRadius(TArray<FVector> inVerts, TArray<int> inTris, TArray<FVector2D> inUVs, FVector centre, float radius, TArray<FVector>& outVerts, TArray<int>& outTris, TArray<FVector2D>& outUVs)
{
	outVerts = inVerts;
	outUVs = inUVs;
	outTris = inTris;

	int idx = 0;
	while (idx < outVerts.Num()) 
	{
		FVector scaledVert = outVerts[idx] * globalScale;
		float dist = FVector::DistXY(scaledVert, centre);

		//if within hole distance, delete vert and corresponding uv and tris
		if (dist <= radius) 
		{

			//if (GEngine)
				//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Removing vert"));

			outVerts.RemoveAt(idx);
			outUVs.RemoveAt(idx);

			int i = 0;
			//flick through tris to account for deleted vertex
			while (i < outTris.Num()) {
				//If refering to verts further in the array, just offset vert idx by one
				if (outTris[i] > idx)
				{
					outTris[i]--;
					i++;
				}
				//if refering to vert to remove, remove all 3 vertices of the triangle from the array
				else if (outTris[i] == idx) {
					outTris.RemoveAt((int)floor(i / 3) * 3);
					outTris.RemoveAt((int)floor(i / 3) * 3);
					outTris.RemoveAt((int)floor(i / 3) * 3);
					i = i - (i - (int)floor(i / 3) * 3);
				}
				else
					i++;
			}
		}
		else
		{
			//if (idx % 100 == 0)
			//{
			//	if (GEngine)
			//		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::SanitizeFloat(dist));
			//}
			idx++;
		}
	}
}

UTexture2D* AAMapGenerator::GenerateMapTexture(int resolution)
{
	//init the texture in 8 bit RGBA with the noise texture size
	UTexture2D* mapTexture = UTexture2D::CreateTransient(mapSize, mapSize, PF_B8G8R8A8);
	mapTexture->UpdateResource();

	//create the map data
	uint8* Data = new uint8[mapSize * mapSize * 4];
	for (int y = 0; y < mapSize; y++)
	{
		for (int x = 0; x < mapSize; x++) 
		{
			int location = x * mapSize + y;
			int imLocation = (mapSize - 1 - y) * mapSize + x;

			FLinearColor color = meshes[(int)floor(noiseMap[location] * (mapLevels - 1))] -> biom -> biomMapColor;

			//store pixel value
			Data[imLocation * 4 + 0] = (uint8)(color.B * (uint8)255);
			Data[imLocation * 4 + 1] = (uint8)(color.G * (uint8)255);
			Data[imLocation * 4 + 2] = (uint8)(color.R * (uint8)255);
			Data[imLocation * 4 + 3] = (uint8)255;
		}
	}

	Data = ResampleMap(Data, mapSize, resolution);

	if (smoothMapTexture)
		Data = Smooth2DMap(Data);
	else if (contourMapTexture)
		Data = Contour2DMap(Data);

	

	//define texture region to update (all of it basically)
	FUpdateTextureRegion2D* Region = new FUpdateTextureRegion2D;
	Region->DestX = 0;
	Region->DestY = 0;
	Region->SrcX = 0;
	Region->SrcY = 0;
	Region->Width = mapSize;
	Region->Height = mapSize;

	//create data cleanup function
	TFunction<void(uint8 * SrcData, const FUpdateTextureRegion2D * Regions)> DataCleanupFunc =
		[](uint8* SrcData, const FUpdateTextureRegion2D* Regions) {
		delete[] SrcData;
		delete[] Regions;
	};

	//update texture
	mapTexture->UpdateTextureRegions(0, 1, Region, mapSize * 4, 4, Data, DataCleanupFunc);

	return mapTexture;
}

ALand* AAMapGenerator::GenerateTopMesh(TArray<FVector2D> points, int depth)
{
	//offset the map points so that the overall mesh is centered on 0,0
	float leftCorner = ((float)mapSize - 1.0) / 2.0;
	float topCorner = ((float)mapSize - 1.0) / 2.0;

	//Erode and Inflate to remove holes of size 1. Also Inflate one extra time so that no gap is left between clusters
	//Inflate(points);
	//clampMap(points);

	
	//init the struct
	FActorSpawnParameters spawnParams;
	ALand* mesh = GetWorld()->SpawnActor<ALand>(FVector(0, 0, 0), FRotator(0, 0, 0));
	mesh->globalScale = globalScale;

	//for each point add a vertex, a corresponding uv point and up to 2 triangles
	int n = points.Num();
	for (int i = 0; i < n; i++)
	{
		float x = points[i].X;
		float y = points[i].Y;

		mesh->verts.Add(FVector(x - leftCorner, y - topCorner, depth * heightScale));
		mesh->uvs.Add(FVector2D(x / (float)mapSize, y / (float)mapSize));

		if (points.Contains(FVector2D(x, y + 1)) && points.Contains(FVector2D(x + 1, y)))
		{
			int j = points.Find(FVector2D(x, y + 1));
			int k = points.Find(FVector2D(x + 1, y));

			mesh->tris.Add(i);
			mesh->tris.Add(j);
			mesh->tris.Add(k);
		}
		else if (points.Contains(FVector2D(x + 1, y + 1)) && points.Contains(FVector2D(x, y + 1)))
		{
			int k = points.Find(FVector2D(x + 1, y + 1));
			int j = points.Find(FVector2D(x, y + 1));

			mesh->tris.Add(i);
			mesh->tris.Add(j);
			mesh->tris.Add(k);
		}
		if (points.Contains(FVector2D(x - 1, y + 1)) && points.Contains(FVector2D(x, y + 1)))
		{
			int j = points.Find(FVector2D(x - 1, y + 1));
			int k = points.Find(FVector2D(x, y + 1));

			mesh->tris.Add(i);
			mesh->tris.Add(j);
			mesh->tris.Add(k);
		}
		else if (points.Contains(FVector2D(x - 1, y)) && points.Contains(FVector2D(x, y + 1)))
		{
			int j = points.Find(FVector2D(x - 1, y));
			int k = points.Find(FVector2D(x, y + 1));

			mesh->tris.Add(i);
			mesh->tris.Add(j);
			mesh->tris.Add(k);
		}
	}

	return mesh;
}

ALand* AAMapGenerator::StoreEdge(ALand* mesh, TArray<FEdgeData> edges)
{
	mesh->edges = TArray<FVector>();

	for (FEdgeData edge : edges) {
		mesh->edges.Add(mesh->verts[(int)edge.vertexIndex.X] * globalScale);
		mesh->edges.Add(mesh->verts[(int)edge.vertexIndex.Y] * globalScale);
	}

	return mesh;
}

void AAMapGenerator::Inflate(TArray<FVector2D>& points)
{
	TArray<FVector2D> buffer = TArray<FVector2D>();

	////obtain outter contours
	//TArray<TArray<FVector2D>> contours = GetContours(points);
	//contours = IsolateOutterContours(contours);

	////solely expand outter contours
	//for (TArray<FVector2D> contour : contours) {
	//	for (FVector2D point : contour)
	//	{
	//		if (!points.Contains(FVector2D(point.X + 1, point.Y)) && !buffer.Contains(FVector2D(point.X + 1, point.Y))) {
	//			buffer.Add(FVector2D(point.X + 1, point.Y));
	//		}
	//		if (!points.Contains(FVector2D(point.X - 1, point.Y)) && !buffer.Contains(FVector2D(point.X - 1, point.Y)))
	//		{
	//			buffer.Add(FVector2D(point.X - 1, point.Y));
	//		}
	//		if (!points.Contains(FVector2D(point.X, point.Y + 1)) && !buffer.Contains(FVector2D(point.X, point.Y + 1)))
	//		{
	//			buffer.Add(FVector2D(point.X, point.Y + 1));
	//		}
	//		if (!points.Contains(FVector2D(point.X, point.Y - 1)) && !buffer.Contains(FVector2D(point.X, point.Y - 1)))
	//		{
	//			buffer.Add(FVector2D(point.X, point.Y - 1));
	//		}
	//	}
	//}

	for (FVector2D point : points)
		{
			if (!points.Contains(FVector2D(point.X + 1, point.Y)) && !buffer.Contains(FVector2D(point.X + 1, point.Y))
				&& noiseMap[(int)(point.Y * mapSize + point.X)] > noiseMap[int(point.Y * mapSize + (point.X+1))]) {
				buffer.Add(FVector2D(point.X + 1, point.Y));
			}
			if (!points.Contains(FVector2D(point.X - 1, point.Y)) && !buffer.Contains(FVector2D(point.X - 1, point.Y))
				&& noiseMap[(int)(point.Y * mapSize + point.X)] > noiseMap[int(point.Y * mapSize + (point.X - 1))])
			{
				buffer.Add(FVector2D(point.X - 1, point.Y));
			}
			if (!points.Contains(FVector2D(point.X, point.Y + 1)) && !buffer.Contains(FVector2D(point.X, point.Y + 1))
				&& noiseMap[(int)(point.Y * mapSize + point.X)] > noiseMap[int((point.Y + 1) * mapSize + point.X)])
			{
				buffer.Add(FVector2D(point.X, point.Y + 1));
			}
			if (!points.Contains(FVector2D(point.X, point.Y - 1)) && !buffer.Contains(FVector2D(point.X, point.Y - 1))
				&& noiseMap[(int)(point.Y * mapSize + point.X)] > noiseMap[int((point.Y - 1) * mapSize + point.X)])
			{
				buffer.Add(FVector2D(point.X, point.Y - 1));
			}
	}

	//for (FVector2D point : points)
	//{
	//	if (!points.Contains(FVector2D(point.X + 1, point.Y)) && !buffer.Contains(FVector2D(point.X + 1, point.Y))) {
	//		buffer.Add(FVector2D(point.X + 1, point.Y));
	//	}
	//	if (!points.Contains(FVector2D(point.X - 1, point.Y)) && !buffer.Contains(FVector2D(point.X - 1, point.Y)))
	//	{
	//		buffer.Add(FVector2D(point.X - 1, point.Y));
	//	}
	//	if (!points.Contains(FVector2D(point.X, point.Y + 1)) && !buffer.Contains(FVector2D(point.X, point.Y + 1)))
	//	{
	//		buffer.Add(FVector2D(point.X, point.Y + 1));
	//	}
	//	if (!points.Contains(FVector2D(point.X, point.Y - 1)) && !buffer.Contains(FVector2D(point.X, point.Y - 1)))
	//	{
	//		buffer.Add(FVector2D(point.X, point.Y - 1));
	//	}
	//}

	points.Append(buffer);
}

void AAMapGenerator::Erode(TArray<FVector2D>& points)
{
	TArray<FVector2D> buffer = TArray<FVector2D>();

	for (FVector2D point : points)
	{
		if (points.Contains(FVector2D(point.X + 1, point.Y)) 
			&& points.Contains(FVector2D(point.X - 1, point.Y))
			&& points.Contains(FVector2D(point.X, point.Y + 1))
			&& points.Contains(FVector2D(point.X, point.Y - 1))) 
		{
			buffer.Add(point);
		}
	}

	points = buffer;
}

void AAMapGenerator::clampMap(TArray<FVector2D>& points)
{
	int i = 0;
	while (i < points.Num()) {
		if (points[i].X < 0 || points[i].X > (float)mapSize - 1
			|| points[i].Y < 0 || points[i].Y > (float)mapSize - 1)
			points.RemoveAt(i);
		else
			i++;
	}
}

TArray<FEdgeData> AAMapGenerator::BuildEdge(int vertexCount, TArray<int> triangleArray)
{
	TArray<FEdgeData> Edges = TArray<FEdgeData>();

	int maxEdgeCount = triangleArray.Num();
	int* firstEdge = new int[vertexCount + maxEdgeCount];
	int nextEdge = vertexCount;
	int triangleCount = triangleArray.Num() / 3;

	for (int a = 0; a < vertexCount; a++)
		firstEdge[a] = -1;

	// First pass over all triangles. This finds all the edges satisfying the
	// condition that the first vertex index is less than the second vertex index
	// when the direction from the first vertex to the second vertex represents
	// a counterclockwise winding around the triangle to which the edge belongs.
	// For each edge found, the edge index is stored in a linked list of edges
	// belonging to the lower-numbered vertex index i. This allows us to quickly
	// find an edge in the second pass whose higher-numbered vertex index is i.
	FEdgeData* edgeArray = new FEdgeData[maxEdgeCount];

	int edgeCount = 0;
	for (int a = 0; a < triangleCount; a++)
	{
		int i1 = triangleArray[a * 3 + 2];
		for (int b = 0; b < 3; b++)
		{
			int i2 = triangleArray[a * 3 + b];
			if (i1 < i2)
			{
				FEdgeData newEdge = FEdgeData();
				newEdge.vertexIndex.X = i1;
				newEdge.vertexIndex.Y = i2;
				newEdge.faceIndex.X = a;
				newEdge.faceIndex.Y = a;
				edgeArray[edgeCount] = newEdge;

				int edgeIndex = firstEdge[i1];
				if (edgeIndex == -1)
				{
					firstEdge[i1] = edgeCount;
				}
				else
				{
					while (true)
					{
						int index = firstEdge[nextEdge + edgeIndex];
						if (index == -1)
						{
							firstEdge[nextEdge + edgeIndex] = edgeCount;
							break;
						}

						edgeIndex = index;
					}
				}

				firstEdge[nextEdge + edgeCount] = -1;
				edgeCount++;
			}

			i1 = i2;
		}
	}

	// Second pass over all triangles. This finds all the edges satisfying the
	// condition that the first vertex index is greater than the second vertex index
	// when the direction from the first vertex to the second vertex represents
	// a counterclockwise winding around the triangle to which the edge belongs.
	// For each of these edges, the same edge should have already been found in
	// the first pass for a different triangle. Of course we might have edges with only one triangle
	// in that case we just add the edge here
	// So we search the list of edges
	// for the higher-numbered vertex index for the matching edge and fill in the
	// second triangle index. The maximum number of comparisons in this search for
	// any vertex is the number of edges having that vertex as an endpoint.

	for (int a = 0; a < triangleCount; a++)
	{
		int i1 = triangleArray[a * 3 + 2];
		for (int b = 0; b < 3; b++)
		{
			int i2 = triangleArray[a * 3 + b];
			if (i1 > i2)
			{
				bool foundEdge = false;
				for (int edgeIndex = firstEdge[i2]; edgeIndex != -1; edgeIndex = firstEdge[nextEdge + edgeIndex])
				{
					FEdgeData edge = edgeArray[edgeIndex];
					if ((edge.vertexIndex.Y == i1) && (edge.faceIndex.X == edge.faceIndex.Y))
					{
						edgeArray[edgeIndex].faceIndex.Y = a;
						foundEdge = true;
						break;
					}
				}

				if (!foundEdge)
				{
					FEdgeData newEdge = FEdgeData();
					newEdge.vertexIndex.X = i1;
					newEdge.vertexIndex.Y = i2;
					newEdge.faceIndex.X = a;
					newEdge.faceIndex.Y = a;
					edgeArray[edgeCount] = newEdge;
					edgeCount++;
				}
			}

			i1 = i2;
		}
	}

	TArray<FEdgeData> compactedEdges = TArray<FEdgeData>();
	for (int e = 0; e < edgeCount; e++)
		compactedEdges.Add(edgeArray[e]);


	return compactedEdges;
}

TArray<FEdgeData> AAMapGenerator::BuildManifoldEdge(ALand* mesh)
{
	TArray<FEdgeData> edges = AAMapGenerator::BuildEdge(mesh->verts.Num(), mesh->tris);
	TArray<FEdgeData> culledEdges = TArray<FEdgeData>();

	//Only keep edges that are connected to	a single triangle
	for(FEdgeData edge : edges)
	{
		if (edge.faceIndex.X == edge.faceIndex.Y)
		{
			culledEdges.Add(edge);
		}
	}
	return culledEdges;
}

ALand* AAMapGenerator::ExtrudeMesh(ALand* mesh, TArray<float> extrusion, TArray<FEdgeData> edges, bool invertFaces)
{
	int extrudedVertexCount = edges.Num() * 2 * extrusion.Num();
	int triIndicesPerStep = edges.Num() * 6;
	int extrudedTriIndexCount = triIndicesPerStep * (extrusion.Num() - 1);

	TArray<FVector> inputVertices = mesh->verts;
	TArray<FVector2D> inputUV = mesh->uvs;
	TArray<int> inputTriangles = mesh->tris;

	FVector* vertices = new FVector[extrudedVertexCount + inputVertices.Num() * 2];
	FVector2D* uvs = new FVector2D[extrudedVertexCount + inputVertices.Num() * 2];
	int* triangles = new int[extrudedTriIndexCount + inputTriangles.Num() * 2];

	int vertCount = extrudedVertexCount + inputVertices.Num() * 2;
	int triangleCount = extrudedTriIndexCount + inputTriangles.Num() * 2;

	// Build extruded vertices
	int v = 0;
	for (int i = 0; i < extrusion.Num(); i++)
	{
		float extrude = extrusion[i];
		float vcoord = (float)i / (extrusion.Num() - 1);
		for(FEdgeData e : edges)
		{
			vertices[v + 0] = inputVertices[e.vertexIndex.X] - FVector(0, 0, -extrude);
			vertices[v + 1] = inputVertices[e.vertexIndex.Y] - FVector(0, 0, -extrude);

			uvs[v + 0] = FVector2D(inputUV[e.vertexIndex[0]].X, vcoord);
			uvs[v + 1] = FVector2D(inputUV[e.vertexIndex[1]].X, vcoord);

			v += 2;
		}
	}

	// Build cap vertices
	// * The bottom mesh we scale along it's negative extrusion direction. This way extruding a half sphere results in a capsule.
	for (int c = 0; c < 2; c++)
	{
		float extrude = extrusion[c == 0 ? 0 : extrusion.Num() - 1];
		int firstCapVertex = c == 0 ? extrudedVertexCount : extrudedVertexCount + inputVertices.Num();
		for (int i = 0; i < inputVertices.Num(); i++)
		{
			vertices[firstCapVertex + i] = inputVertices[i] - FVector(0, 0, -extrude);
			uvs[firstCapVertex + i] = inputUV[i];
		}
	}

	// Build extruded triangles
	for (int i = 0; i < extrusion.Num() - 1; i++)
	{
		int baseVertexIndex = (edges.Num() * 2) * i;
		int nextVertexIndex = (edges.Num() * 2) * (i + 1);
		for (int e = 0; e < edges.Num(); e++)
		{
			int triIndex = i * triIndicesPerStep + e * 6;

			triangles[triIndex + 0] = baseVertexIndex + e * 2;
			triangles[triIndex + 1] = nextVertexIndex + e * 2;
			triangles[triIndex + 2] = baseVertexIndex + e * 2 + 1;
			triangles[triIndex + 3] = nextVertexIndex + e * 2;
			triangles[triIndex + 4] = nextVertexIndex + e * 2 + 1;
			triangles[triIndex + 5] = baseVertexIndex + e * 2 + 1;
		}
	}

	// build cap triangles
	int triCount = inputTriangles.Num() / 3;
	// Top
	{
		int firstCapVertex = extrudedVertexCount;
		int firstCapTriIndex = extrudedTriIndexCount;
		for (int i = 0; i < triCount; i++)
		{
			triangles[i * 3 + firstCapTriIndex + 0] = inputTriangles[i * 3 + 1] + firstCapVertex;
			triangles[i * 3 + firstCapTriIndex + 1] = inputTriangles[i * 3 + 2] + firstCapVertex;
			triangles[i * 3 + firstCapTriIndex + 2] = inputTriangles[i * 3 + 0] + firstCapVertex;
		}
	}

	// Bottom
	{
		int firstCapVertex = extrudedVertexCount + inputVertices.Num();
		int firstCapTriIndex = extrudedTriIndexCount + inputTriangles.Num();
		for (int i = 0; i < triCount; i++)
		{
			triangles[i * 3 + firstCapTriIndex + 0] = inputTriangles[i * 3 + 0] + firstCapVertex;
			triangles[i * 3 + firstCapTriIndex + 1] = inputTriangles[i * 3 + 2] + firstCapVertex;
			triangles[i * 3 + firstCapTriIndex + 2] = inputTriangles[i * 3 + 1] + firstCapVertex;
		}
	}

	if (invertFaces)
	{
		for (int i = 0; i < triangleCount / 3; i++)
		{
			int temp = triangles[i * 3 + 0];
			triangles[i * 3 + 0] = triangles[i * 3 + 1];
			triangles[i * 3 + 1] = temp;
		}
	}

	//replace mesh data
	mesh->verts = TArray<FVector>();
	mesh->uvs = TArray<FVector2D>();
	mesh->tris = TArray<int>();
	for (int i = 0; i < vertCount; i++)
	{
		mesh->verts.Add(vertices[i]);
		mesh->uvs.Add(uvs[i]);
	}
	for (int i = 0; i < triangleCount; i++) {
		mesh->tris.Add(triangles[i]);
	}

	return mesh;
}

ALand* AAMapGenerator::PerformMeshExtrusion(ALand* mesh, TArray<float> extrusionHeight)
{
	TArray<FEdgeData> edges = AAMapGenerator::BuildManifoldEdge(mesh);
	//mesh = AAMapGenerator::SmoothEdge(mesh, edges);
	mesh = AAMapGenerator::StoreEdge(mesh, edges);
	mesh = AAMapGenerator::ExtrudeMesh(mesh, extrusionHeight, edges, false);
	mesh = AAMapGenerator::RemoveDuplicateVertices(mesh);

	

	return mesh;
}

ALand* AAMapGenerator::RemoveDuplicateVertices(ALand* mesh)
{
	TArray<FVector> verts = mesh->verts;
	TArray<FVector2D> uvs = mesh->uvs;
	TArray<int> tris = mesh->tris;

	float duplicateDist = 0.1f;

	int i = 0;
	while (i < verts.Num())
	{
		int j = i + 1;
		while (j < verts.Num())
		{
			//if vertices j and i are close enough, merge them
			float dist = (verts[i] - verts[j]).Size();
			if (dist < duplicateDist)
			{
				//asssign tris to vertex i instead of j and every vert that is larger than j ar shifted one index down to match the removal of a vert
				for (int k = 0; k < tris.Num(); k++)
				{
					if (tris[k] == j)
						tris[k] = i;
					else if (tris[k] > j)
						tris[k]--;
				}

				//remove index at j-th position
				verts.RemoveAt(j);
				uvs.RemoveAt(j);
			}
			else
				j++;
		}
		i++;
	}

	mesh->verts = verts;
	mesh->tris = tris;
	mesh->uvs = uvs;
	return mesh;
}

ALand* AAMapGenerator::SmoothEdge(ALand* mesh, TArray<FEdgeData> edges)
{
	TArray<FVector> smoothedVerts = mesh->verts;

	for (int i = 0; i < smoothedVerts.Num(); i++)
	{
		int count = 1;
		for(FEdgeData e : edges)
		{
			if (e.vertexIndex[0] == i)
			{
				smoothedVerts[i] += mesh->verts[e.vertexIndex[1]];
				count++;
			}
			else if (e.vertexIndex[1] == i)
			{
				smoothedVerts[i] += mesh->verts[e.vertexIndex[0]];
				count++;
			}
		}

		smoothedVerts[i] /= (float)count;
	}

	mesh->verts = smoothedVerts;
	return mesh;
}

void AAMapGenerator::GenerateClouds()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Generating Clouds"));

	if (cloudMeshes.Num() > 0) {

		//Pick a number cloud in cloudAmount +/- 50%
		int cloudNumber = (rand() % cloudAmount) + (cloudAmount / 2);

		for (int i = 0; i < cloudNumber; i++) {
			cloudsDistribution.Add(cloudMeshes[rand() % cloudMeshes.Num()]);
			cloudsPosition.Add(FVector(rand() % mapSize - (float)mapSize /2.0, rand() % mapSize - (float)mapSize / 2.0, (mapLevels + 10) * heightScale + rand() % 2));
		}
	}

	AAMapGenerator::SpawnClouds();
}

TArray<TArray<FVector2D>> AAMapGenerator::GetContours(TArray<FVector2D> points)
{
	TArray<TArray<FVector2D>> contours = TArray<TArray<FVector2D>>();
	int idx = 0;
	
	while (idx < points.Num()) {
		FVector2D startPoint;
		bool startFound = false;

		//start by looking for a contour point
		while (!startFound && idx < points.Num()) {
			startPoint = points[idx];
			//check if point is a contour point
			if (!points.Contains(FVector2D(startPoint.X, startPoint.Y + 1))
				|| !points.Contains(FVector2D(startPoint.X, startPoint.Y - 1))
				|| !points.Contains(FVector2D(startPoint.X + 1, startPoint.Y))
				|| !points.Contains(FVector2D(startPoint.X - 1, startPoint.Y))) {

				//if yes, check whether the point is already in a contour
				bool isContained = false;
				for (TArray<FVector2D> oldcontour : contours) {
					if (oldcontour.Contains(startPoint)) {
						isContained = true;
						break;
					}
				}

				//if is not contained, flag as new contour startpoint, else move on
				if (!isContained)
					startFound = true;
				else
				{
					idx++;
				}
			}
			//else move to next point
			else {
				idx++;
			}
		}
		//if no new contour start was found, stop
		if (!startFound || idx >= points.Num())
			break;
		//else, if a new start point is found, extract the associated contour
		else
		{
			TArray<FVector2D> contour = TArray<FVector2D>();
			//store the start point and follow contour untill looped to startPoint
			contour.Add(startPoint);
			bool closedLoop = false;
			float previousTheta = 0;
			float theta = 0;

			FVector2D currentPoint = startPoint;
			FVector2D previousPoint = startPoint;

			while (!closedLoop) {
				
				bool newContourPointFound = false;
				FVector2D candidate = FVector2D();

				//for each point, look at 360° to see if we can find a point that is on contour.
				while (theta < (previousTheta + 2 * PI) && !newContourPointFound)
				{
					//round to the closest integer, either 1 or -1 the cosine and sine of theta to explore the 8 points around the previous one
 					int x = (int)(cos(theta) + 0.5);
					int y = (int)(sin(theta) + 0.5);
					candidate = FVector2D(currentPoint.X + x, currentPoint.Y + y);

					if (points.Contains(candidate)) {
						if (candidate != previousPoint) {
							if (candidate == startPoint)
								break;

							//if the candidate is a contour point, set it as the new point of the contour
							if (!points.Contains(FVector2D(candidate.X, candidate.Y + 1))
								|| !points.Contains(FVector2D(candidate.X, candidate.Y - 1))
								|| !points.Contains(FVector2D(candidate.X + 1, candidate.Y))
								|| !points.Contains(FVector2D(candidate.X - 1, candidate.Y))) {
								newContourPointFound = true;
							}
							else {
								theta += PI / 4;
							}
						}
						else theta += PI / 4;
					}
					else theta += PI / 4;
				}

				//A new contour should have been found because there is no open contour.
				if (candidate == startPoint)
				{
					closedLoop = true;
				}
				else
				{
					if (!newContourPointFound)
					{
						if (GEngine)
							GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Something is wrong because a contour is open"));
						return TArray<TArray<FVector2D>>();
					}

					contour.Add(candidate);
					previousPoint = currentPoint;
					currentPoint = candidate;
					previousTheta = theta;
					
				}
			}

			//once the loop of the contour is closed, store the contour
			contours.Add(contour);
		}
		idx++;
	}

	return contours;
}

TArray<TArray<FVector2D>> AAMapGenerator::IsolateOutterContours(TArray<TArray<FVector2D>> contours)
{
	TArray<int> isInner = TArray<int>();
	int i = 0;

	//test every contour to know whether it is inner or not.
	for (TArray<FVector2D> contour : contours) {
		int j = 0;
		for (TArray<FVector2D> otherContour : contours) {
			if (j != i) {
				if (GetWindingNumber(contour[0], otherContour))
				{
					isInner[i] = j;
					break;
				}
			}
			j++;
		}
		i++;
	}

	TArray<TArray<FVector2D>> outterContours = TArray<TArray<FVector2D>>();
	for (int k = 0; k < contours.Num(); k++) 
	{
		if (!IsInner(isInner, k))
			outterContours.Add(contours[k]);
	}

	return TArray<TArray<FVector2D>>();
}

int AAMapGenerator::GetWindingNumber(FVector2D point, TArray<FVector2D> contour)
{
	int    wn = 0;    // the  winding number counter

	// loop through all edges of the polygon
	for (int i = 0; i < contour.Num(); i++) {   // edge from V[i] to  V[i+1]
		if (contour[i].Y <= point.Y) {          // start y <= P.y
			if (contour[i + 1].Y > point.Y)      // an upward crossing
				if (IsLeft(contour[i], contour[i + 1], point) > 0)  // P left of  edge
					++wn;            // have  a valid up intersect
		}
		else {                        // start y > P.y (no test needed)
			if (contour[i + 1].Y <= point.Y)     // a downward crossing
				if (IsLeft(contour[i], contour[i + 1], point) < 0)  // P right of  edge
					--wn;            // have  a valid down intersect
		}
	}
	return wn;
}

int AAMapGenerator::IsLeft(FVector2D P0, FVector2D P1, FVector2D P2)
{
	return ((P1.X - P0.X) * (P2.Y - P0.Y) - (P2.X - P0.X) * (P1.Y - P0.Y));
}

bool AAMapGenerator::IsInner(TArray<int> isInsideContour, int idx)
{
	//if is -1 that means it is an outter contour
	if (isInsideContour[idx] == -1)
		return false;
	//else
	else
		//if contour that contains is also inner, that means, contour is actually outter
		if (IsInner(isInsideContour, isInsideContour[idx]))
			return false;
		//if contour that contains is outter, it means this one is really inner
		else
			return true;
}

void AAMapGenerator::PickLandmarks()
{
	//prepare spawn offset
	float leftCorner = ((float)mapSize - 1.0f) / 2.0f;
	float topCorner = ((float)mapSize - 1.0f) / 2.0f;


	//create a buffer of potential landmarks
	TArray<TSubclassOf<ALandmark>> buffer = potentialLandmarks;

	if (amountOfLandmarks > potentialLandmarks.Num())
		amountOfLandmarks = potentialLandmarks.Num();

	//pick required amount of landmarks 
	for (int i = 0; i < amountOfLandmarks; i++)
	{
		//choose a random landmark
		int idx = FMath::RandRange(0, buffer.Num() - 1);
		UClass* selectedClass = *buffer[idx];

		//spawn landmark
		ALandmark* newLandmark = Cast<ALandmark>(GetWorld()->SpawnActor(selectedClass));
		int X = 0;
		int Y = 0;

		bool tooClose = true;
		int maxIttNum = 1000;
		int ittNb = 0;

		//generate random location on the map as long as too close to another landmark (within sum of radii that is)
		while (tooClose) 
		{
			//make sure we don't get stuck in an infinite loop because all landmarks are too close
			ittNb++;
			if (ittNb > maxIttNum) 
				break;

			//random location
			X = FMath::RandRange(newLandmark->radius, mapSize - newLandmark->radius);
			Y = FMath::RandRange(newLandmark->radius, mapSize - newLandmark->radius);

			tooClose = false;
			for (int j = 0; j < landmarks.Num(); j++) {
				//distance to other landmark
				float dist = sqrt((X - landmarks[j]->mapPosition.X) * (X - landmarks[j]->mapPosition.X)
					+ (Y - landmarks[j]->mapPosition.Y) * (Y - landmarks[j]->mapPosition.Y));
				//compare to sum of radii (and thetwice the amount of levels to make sure falloffs won't interfere)
				if (dist < newLandmark->radius + landmarks[j]->radius /*+ 2 * mapLevels*/) {
					tooClose = true;
					break;
				}
			}
		}
		if (ittNb > maxIttNum) 
		{
			newLandmark->Destroy();
			return;
		}
		else
		{
			//store location
			newLandmark->mapPosition = FVector2D(X, Y);

			//set world location and location
			newLandmark->SetActorLocation(FVector((Y - leftCorner) * globalScale, (X - topCorner) * globalScale, ((int)(newLandmark->baseHeight * (mapLevels - 1))) * heightScale * globalScale));
			//newLandmark->SetActorRotation(FRotator(0, 0, FMath::RandRange(0, 360)));

			//store new landmark
			landmarks.Add(newLandmark);

			//remove idx from buffer to avoid spawning again
			buffer.RemoveAt(idx);
		}
	}
}

void AAMapGenerator::MatchLandToLandmarks()
{
	TArray<FVector2D> lastAddedPoints = TArray<FVector2D>();
	int* mapMask = new int[mapSize * mapSize];

	//For each landmark, level ground to base landmark level
	for (ALandmark* landmark : landmarks) 
	{
		for (int i = 0; i < mapSize; i++)
		{
			for (int j = 0; j < mapSize; j++)
			{
				//compute distance to landmark center
				float dist = sqrt((i - landmark->mapPosition.X) * (i - landmark->mapPosition.X)
					+ (j - landmark->mapPosition.Y) * (j - landmark->mapPosition.Y));

				if (dist < landmark->radius) {
					noiseMap[i * mapSize + j] = landmark->baseHeight;

					//update the mask and store as outter ring points
					mapMask[i * mapSize + j] = 1;
					lastAddedPoints.Add(FVector2D(i, j));
				}
			}
		}
	}

	//propagate flattening (itterate until there is no new outter points
	while (lastAddedPoints.Num() > 0)
	{
		TArray<FVector2D> buffer = TArray<FVector2D>();
		//for each outter points
		for (FVector2D outterPoint : lastAddedPoints)
		{
			//check all 8 points around it
			for (int i = -1; i <= 1; i++) 
			{
				for (int j = -1; j <= 1; j++)
				{
					if (j != 0 || i != 0)
					{
						//if within map bounds
						if ((i + (int)outterPoint.X) < mapSize && (i + (int)outterPoint.X) >= 0
							&& (j + (int)outterPoint.Y) < mapSize && (j + (int)outterPoint.Y >= 0)) 
						{
							//if not yet tested
							if (mapMask[(i + (int)outterPoint.X) * mapSize + (j + (int)outterPoint.Y)] != 1) 
							{
								//set as tested
								mapMask[(i + (int)outterPoint.X) * mapSize + (j + (int)outterPoint.Y)] = 1;

								//register as edge point and flatten
								buffer.Add(FVector2D(i + outterPoint.X, j + outterPoint.Y));

								//check height difference
								float heightDifference = noiseMap[(i + (int)outterPoint.X) * mapSize + (j + (int)outterPoint.Y)]
									- noiseMap[(int)outterPoint.X * mapSize + (int)outterPoint.Y];

								//check wheter the height difference is greater than one level
								if (abs(heightDifference) > 1.0 / (float)(mapLevels - 1))
								{
									//check slope direction
									float sign = 1;
									if (heightDifference < 0)
										sign = -1;

									//assign new height
									noiseMap[(i + (int)outterPoint.X) * mapSize + (j + (int)outterPoint.Y)] 
										= noiseMap[(int)outterPoint.X * mapSize + (int)outterPoint.Y] 
										+ sign / (float)(mapLevels - 1);

									//clamp
									if (noiseMap[(i + (int)outterPoint.X) * mapSize + (j + (int)outterPoint.Y)] >= 1)
										noiseMap[(i + (int)outterPoint.X) * mapSize + (j + (int)outterPoint.Y)] = .99;
									else if (noiseMap[(i + (int)outterPoint.X) * mapSize + (j + (int)outterPoint.Y)] < 0)
										noiseMap[(i + (int)outterPoint.X) * mapSize + (j + (int)outterPoint.Y)] = 0;
								}
							}
						}
					}
				}
			}
		}
		lastAddedPoints = buffer;
	}
}

void AAMapGenerator::GenerateRockAndTrees()
{
	for (int i = 0; i < mapSize - 1; i++)
	{
		for (int j = 0; j < mapSize - 1; j++)
		{
			float x = i + 0.5;
			float y = j + 0.5;

			bool canSpawn = true;
			for (ALandmark* landmark : landmarks) 
			{
				float dist = sqrt((landmark->mapPosition.X - x) * (landmark->mapPosition.X - x) 
					+ (landmark->mapPosition.Y - y) * (landmark->mapPosition.Y - y));
				if (dist < landmark->radius)
				{
					canSpawn = false;
					break;
				}
			}
			
			if (canSpawn)
			{
				//int itt = 0;
				//while (floor(inGameBioms[itt]->biomSeparation * (mapLevels - 1)) < floor(noiseMap[i * mapSize + j] * (mapLevels - 1)))
				//{
				//	itt++;
				//	if (itt >= inGameBioms.Num())
				//		break;
				//}

				if (floor(noiseMap[i * mapSize + j] * (mapLevels - 1)) == floor(noiseMap[(i + 1) * mapSize + j] * (mapLevels - 1))
					&& floor(noiseMap[(i + 1) * mapSize + j] * (mapLevels - 1)) == floor(noiseMap[i * mapSize + (j + 1)] * (mapLevels - 1))
					&& floor(noiseMap[(i + 1) * mapSize + j] * (mapLevels - 1)) == floor(noiseMap[(i + 1) * mapSize + (j + 1)] * (mapLevels - 1)))
				{
					FVector position = FVector(y - ((float)(mapSize - 1) / 2.0), x - ((float)(mapSize - 1) / 2.0), floor(noiseMap[i * mapSize + j] * (mapLevels - 1) + 5) * heightScale) * globalScale;

					//get the class of the new ressource
					UClass* newPropClass = meshes[(int)(noiseMap[i * mapSize + j] * (mapLevels - 1))]->biom->GetRandomProp();

					//if there is actually a ressource to spawn, spawn it
					if (newPropClass)
					{
						AProp* newPropActor = Cast<AProp>(GetWorld()->SpawnActor(newPropClass));

						//set XY coordinate. Z is chosen to hover over the final destination
						newPropActor->SetActorLocation(position);

						//newPropActor->propMeshComponent->SetStaticMesh(newProp);
						newPropActor->MoveToClosestSurface();
					}
				}
			}
		}
	}
}

void AAMapGenerator::InitBioms()
{
	for (UClass* biom : bioms)
	{
		//spawn biom
		ABiom* newBiom = Cast<ABiom>(GetWorld()->SpawnActor(biom));

		if (inGameBioms.Num() > 0)
		{
			int itt = 0;
			while (inGameBioms[itt]->biomSeparation < newBiom->biomSeparation) 
			{
				itt++;
				if (itt >= inGameBioms.Num())
					break;
			}
			if (itt >= inGameBioms.Num())
				inGameBioms.Add(newBiom);
			else
				inGameBioms.Insert(newBiom, itt);
		}
		else
			inGameBioms.Add(newBiom);
	}
}

uint8* AAMapGenerator::Smooth2DMap(uint8* Data)
{
	uint8* smoothedData = new uint8[mapSize * mapSize * 4];

	for(int i = 0; i < mapSize; i++)
	{
		for (int j = 0; j < mapSize; j++)
		{
			if (i == 0 || i == mapSize - 1 || j == 0 || j == mapSize - 1)
			{
				for (int k = 0; k < 3; k++)
					smoothedData[(i * mapSize + j) * 4 + k] = Data[(i * mapSize + j) * 4 + 4];
			}
			else
			{
				for (int k = 0; k < 4; k++)
				{
					int s = 0;
					for (int i1 = -1; i1 < 2; i1++)
					{
						for (int j1 = -1; j1 < 2; j1++)
						{
							s += Data[((i + i1) * mapSize + (j + j1)) * 4 + k];
						}
					}

					s /= 9;

					smoothedData[(i * mapSize + j) * 4 + k] = s;
				}
				smoothedData[(i * mapSize + j) * 4 + 3] = 255;
			}
		}
	}

	return smoothedData;
}

uint8* AAMapGenerator::Contour2DMap(uint8* Data)
{
	uint8* smoothedData = new uint8[mapSize * mapSize * 4];

	for (int i = 0; i < mapSize; i++)
	{
		for (int j = 0; j < mapSize; j++)
		{
				//if not the same color as adjacent pixels, flag as contour 
				bool contour = false;
				for (int i1 = -1; i1 < 2; i1++)
				{
					for (int j1 = -1; j1 < 2; j1++)
					{
						if (i + i1 >= 0 && i + i1 <= mapSize - 1 && j + j1 >= 0 && j + j1 <= mapSize - 1)
						{

							if (Data[((i + i1) * mapSize + (j + j1)) * 4 + 1] != Data[(i * mapSize + j) * 4 + 1]
								|| Data[((i + i1) * mapSize + (j + j1)) * 4 + 2] != Data[(i * mapSize + j) * 4 + 2]
								|| Data[((i + i1) * mapSize + (j + j1)) * 4 + 0] != Data[(i * mapSize + j) * 4 + 0])
							{
								contour = true;
								break;
							}
						}
					}
					if (contour)
						break;
				}

				
				if (contour)
				{
					smoothedData[(i * mapSize + j) * 4 + 0] = (uint8)(contourColor.B  * 255);
					smoothedData[(i * mapSize + j) * 4 + 1] = (uint8)(contourColor.G * 255);
					smoothedData[(i * mapSize + j) * 4 + 2] = (uint8)(contourColor.R * 255);
					smoothedData[(i * mapSize + j) * 4 + 3] = (uint8)255;
				}
				else
				{
					for (int k = 0; k < 4; k++)
					{
						if (overlayContour)
							smoothedData[(i * mapSize + j) * 4 + k] = Data[(i * mapSize + j) * 4 + k];
						else
							smoothedData[(i * mapSize + j) * 4 + k] = 255;
					}
				}
				smoothedData[(i * mapSize + j) * 4 + 3] = 255;
			
		}
	}

	return smoothedData;
}

//Implements a nearest neighbour rescaling of the data
uint8* AAMapGenerator::ResampleMap(uint8* Data, int originalRes, int newRes)
{
	//if there is no change in resolution, simply return the original resolution
	if (originalRes == newRes || newRes <= 0)
		return Data;

	float ratio = (float)originalRes / (float)newRes;

	uint8* newData = new uint8[newRes * newRes * 4];

	for (int i = 0; i < newRes * newRes; i++)
	{
		int x = (int) i / newRes;
		int y = (int) i % newRes;

		//project the new pixel on the original map
		float originalX = floor(x * ratio);
		float originalY = floor(y * ratio);
		int oldLocation = (int)(originalY * originalRes + originalX);

		//store pixel value
		for (int j = 0; j < 3; j++)
		{
			newData[i * 4 + j] = Data[oldLocation * 4 + j];
		}
		newData[i * 4 + 3] = 255;

	}

	////for each pixel, find its projection in the original map 
	//for (int x = 0; x < newRes; x++)
	//{
	//	//project the new pixel on the original map
	//	float originalX = floor(x * ratio);

	//	for (int y = 0; y < newRes; y++)
	//	{
	//		//project the new pixel on the original map
	//		float originalY = floor(y * ratio);

	//		int oldLocation = (int)(originalY * originalRes + originalX);
	//		int newLocation = y * newRes + x;

	//		//store pixel value
	//		for (int i = 0; i < 3; i++)
	//		{
	//			newData[newLocation * 4 + i] = Data[oldLocation * 4 + i];
	//		}
	//		newData[newLocation * 4 + 3] = 255;
	//	}
	//}

	return newData;
}

// Called when the game starts or when spawned
void AAMapGenerator::BeginPlay()
{
	Super::BeginPlay();
	
	AAMapGenerator::GenerateNoise();
}

// Called every frame
void AAMapGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
