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


#include "Public/Land.h"

// Sets default values
ALand::ALand()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

bool ALand::checkObjectFits(FVector position, float radius)
{

	bool fits = true;

	//FString debug = "(";
	//debug.Append(FString::SanitizeFloat(position.X));
	//debug.Append(", ");
	//debug.Append(FString::SanitizeFloat(position.Y));
	//debug.Append(") (");

	//for each edge
	for (int i = 0; i < edges.Num() / 2; i+= 2) {

		//if one or the other extreme edge point is too close, it means the object doesn't fit
		if (FVector::Dist2D(position, edges[i]) < radius
			|| FVector::Dist2D(position, edges[i + 1]) < radius) 
		{
			//debug.Append(FString::SanitizeFloat(edges[i].X));
			//debug.Append(", ");
			//debug.Append(FString::SanitizeFloat(edges[i].Y));
			//debug.Append(") (");
			//debug.Append(FString::SanitizeFloat(edges[i + 1].X));
			//debug.Append(", ");
			//debug.Append(FString::SanitizeFloat(edges[i + 1].Y));
			//debug.Append(")");

			//if (GEngine)
			//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, debug);

			fits = false;
			break;
		}

		//get the projection of the point on the edge line so that we can check whether it lands on the edge segment or not
		FVector edgeDir = edges[i + 1] - edges[i];
		edgeDir.Normalize();

		//d is the distance from point 1 of the edge to the projection
		float d = ((position.X - edges[i].X) * edgeDir.X + (position.Y - edges[i].Y) * edgeDir.Y); //dot(vec(AC),vec(dir(AB))) = length(AH)
		float Xp = edges[i].X + d * edgeDir.X; //vec(AH) = lenght(AH) * dir(AB) + vec(A)
		float Yp = edges[i].Y + d * edgeDir.Y;
		FVector proj = FVector(Xp, Yp, position.Z);

		//if the projection is on the edge segment, we check whether the projection is far enough
		float edgeLength = FVector::Dist2D(edges[i], edges[i + 1]);
		if (abs(d) < edgeLength
			&& d > 0
			&& FVector::Dist2D(position, proj) < radius)
		{
			//debug.Append(FString::SanitizeFloat(edges[i].X));
			//debug.Append(", ");
			//debug.Append(FString::SanitizeFloat(edges[i].Y));
			//debug.Append(") (");
			//debug.Append(FString::SanitizeFloat(edges[i + 1].X));
			//debug.Append(", ");
			//debug.Append(FString::SanitizeFloat(edges[i + 1].Y));
			//debug.Append(") (");
			//debug.Append(FString::SanitizeFloat(proj.X));
			//debug.Append(", ");
			//debug.Append(FString::SanitizeFloat(proj.Y));
			//debug.Append(")");
			//if (GEngine)
			//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, debug);
			//if (GEngine)
			//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::SanitizeFloat(FVector::Dist2D(position, proj)));

			fits = false;
			break;
		}
	}

	return fits;
}

// Called when the game starts or when spawned
void ALand::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALand::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

