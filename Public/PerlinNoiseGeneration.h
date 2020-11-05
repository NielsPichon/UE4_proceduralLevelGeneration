// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class TREASUREHUNT_API PerlinNoiseGeneration
{
public:
	PerlinNoiseGeneration(int seed, int mapSize, int octaves, float persistance, float baseFreq);
	~PerlinNoiseGeneration();
	float Perlin(float x, float y);
	float PerlinNoiseValue(float x, float y);
	

private:
	float Fade(float t);
	float Lerp(float a, float b, float t);
	float DotGridGradient(int ix, int iy, float x, float y);
	float* gradient;
	int mapSize;
	int octaves;
	float persistance;
	float baseFreq;
};
