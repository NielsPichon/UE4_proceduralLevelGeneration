# UE4 Procedural Level Generation

This repo contains a collection of scripts which I made for a little game project using procedural stylized terrain generation.
While the game prototype never ended as a full pledge game, I thought that some of these scripts might be of use to some people. 

For access to the latest prototype where this code is used, please email niels.pichon@outlook.com

## Content

* `AMapGenerator.cpp`: This is the main file which governs them all. Essentially, it sequentially generates a square map from Perlin noise, 
then randomly picks some handmade landmarks (e.g. an Aztec temple, or some dynausaur rib cage in the sand) and spreads them randomly on the map,
mathes the noise level so that each landmark is on flat ground at the right altitude (defined in the landmark actor). It then tries desparately to
smooth the terrain so that there would not be too steep terrain. This part currently still has a couple bugs unfortunately which will leave some gaps open.
Then the noise is terrassed in a set number of levels, and each level is clustered. Finally the various clustered are extruded.
This file also contains everything need to spread the rocks, trees and clouds around the map with the appropriate appearance based on the different bioms.
* Other scripts to define the various other classes to be spawend randomly to inhabit the world


## License

MIT License

Copyright (c) 2020 NielsPichon

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
