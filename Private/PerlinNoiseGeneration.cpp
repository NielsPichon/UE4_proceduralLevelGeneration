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


#include "Public/PerlinNoiseGeneration.h"
#include <cmath>


PerlinNoiseGeneration::PerlinNoiseGeneration(int _seed, int _mapSize, int _octaves, float _persistance, float _baseFreq)
{
	mapSize = _mapSize;
	octaves = _octaves;
	persistance = _persistance;
	baseFreq = _baseFreq;

	// We generate a map of size the map size (to the square) times the number of octaves.
	// This works because we use few octaves and small maps.
	gradient = new float[mapSize * mapSize * 2 * octaves * octaves];

	srand(_seed);

	// For each point on the map
	for (int i = 0; i < mapSize * octaves; i++) {
		for (int j = 0; j < mapSize * octaves; j++) {
			// Generate 2 random (gaussian) noise values
			// which will represent the direction of the gradient (in the xy plane)
			// for each pixel
			float a = (float)(rand() % 10000) / 10000.0;
			float b = (float)(rand() % 10000) / 10000.0;
			if (a == b && a == 0)
				a = 1;
			gradient[2 * (i * mapSize * octaves + j)] = a / sqrt(a * a + b * b);
			gradient[2 * (i * mapSize * octaves + j) + 1] = b / sqrt(a * a + b * b);
		}
	}
}

PerlinNoiseGeneration::~PerlinNoiseGeneration()
{
}


// The bellow function is a handmade tailored interpolant 
// between 0 and 1 in an S shaped manner (obtained by solving a 4th order 
// polynomial with 0 derivative in 0 and 1 and going through 0 and 1 at these points
// https://tinyurl.com/y23km6u5
float PerlinNoiseGeneration::Fade(float t) {
	return (t * t * t * (t * (t * 6 - 15) + 10));
}


// Simple linear interpolation
float PerlinNoiseGeneration::Lerp(float a, float b, float t) {
	return (1 - t) * a + b * t;
}

// Computes the dot product of the distance and gradient vectors.
float PerlinNoiseGeneration::DotGridGradient(int ix, int iy, float x, float y) {
	// Compute the distance vector
	float dx = x - (float)ix;
	float dy = y - (float)iy;

	// Compute the dot-product
	return (dx * gradient[2 * (iy + ix * mapSize)] + dy * gradient[2 * (iy + ix * mapSize) + 1]) /*/ sqrt(2)*/;
}

// Compute Perlin noise at coordinates x, y
float PerlinNoiseGeneration::Perlin(float x, float y) {

	//we shift the points to avoid falling on a 0
	float freqShift = 1 / (2 * baseFreq);

	//set frequency and shift
	float X = x / baseFreq + freqShift;
	float Y = y / baseFreq + freqShift;

	// Determine grid cell coordinates
	int x0 = (int)floor(X);
	int x1 = x0 + 1;
	int y0 = (int)floor(Y);
	int y1 = y0 + 1;

	// Determine interpolation weights
	// Could also use higher order polynomial/s-curve here
	float sx = PerlinNoiseGeneration::Fade(X - (float)x0);
	float sy = PerlinNoiseGeneration::Fade(Y - (float)y0);

	// Interpolate between grid point gradients
	float n0, n1, ix0, ix1, value;

	n1 = PerlinNoiseGeneration::DotGridGradient(x1, y0, X, Y);
	n0 = PerlinNoiseGeneration::DotGridGradient(x0, y0, X, Y);
	ix0 = PerlinNoiseGeneration::Lerp(n0, n1, sx);

	n0 = PerlinNoiseGeneration::DotGridGradient(x0, y1, X, Y);
	n1 = PerlinNoiseGeneration::DotGridGradient(x1, y1, X, Y);
	ix1 = PerlinNoiseGeneration::Lerp(n0, n1, sx);

	value = PerlinNoiseGeneration::Lerp(ix0, ix1, sy);
	return value;
}


// Perlin noise generation
float PerlinNoiseGeneration::PerlinNoiseValue(float x, float y) {
	float noise = 0;
	float persist = 1;
	float frequency = 1;
	float totAmplitude = 0;
	for (int i = 0; i < octaves; i++) {
		noise += persist  * PerlinNoiseGeneration::Perlin(x * frequency, y * frequency);
		totAmplitude += persist;
		persist *= persistance;
		frequency *= 2;
	}

	return (noise / totAmplitude + 1.0) * 0.5;
}
