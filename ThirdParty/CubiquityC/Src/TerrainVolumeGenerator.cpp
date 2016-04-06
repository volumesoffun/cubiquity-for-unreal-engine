/*******************************************************************************
* The MIT License (MIT)
*
* Copyright (c) 2016 David Williams and Matthew Williams
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*******************************************************************************/

#include "TerrainVolumeGenerator.h"

#include "TerrainVolume.h"

namespace Cubiquity
{
	void generateFloor(TerrainVolume* terrainVolume, int32_t lowerLayerHeight, uint32_t lowerLayerMaterial, int32_t upperLayerHeight, uint32_t upperLayerMaterial)
	{
		const Region& region = terrainVolume->getEnclosingRegion();

		for(int32_t y = region.getLowerY(); y < region.getUpperY(); y++)		
		{
			// Density decreases with increasing y, to create a floor rather than ceiling
			int32_t density = -y;
			// Add the offset to move the floor to the desired level
			density += upperLayerHeight;
			// 'Compress' the density field so that it changes more quickly
			// from fully empty to fully solid (over only a few voxels)
			density *= 64;
			// Account for the threshold not being at zero
			density += MaterialSet::getMaxMaterialValue() / 2;

			//Clamp resulting density
			density = (std::min)(density, static_cast<int32_t>(MaterialSet::getMaxMaterialValue()));
			density = (std::max)(density, 0);

			uint32_t index = (y <= lowerLayerHeight) ? lowerLayerMaterial : upperLayerMaterial;
			MaterialSet material;
			material.setMaterial(index, density);

			for(int32_t x = region.getLowerX(); x <= region.getUpperX(); x++)
			{
				for(int32_t z = region.getLowerZ(); z <= region.getUpperZ(); z++)
				{
					terrainVolume->setVoxel(x, y, z, material, false);
				}
			}
		}

		terrainVolume->markAsModified(region);
	}
}
