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

#ifndef TERRAINVOLUME_H_
#define TERRAINVOLUME_H_

#include "Cubiquity.h"
#include "CubiquityForwardDeclarations.h"
#include "Volume.h"

namespace Cubiquity
{
	class TerrainVolume : public Volume<MaterialSet>
	{
	public:
		typedef MaterialSet VoxelType;

		TerrainVolume(const Region& region, const std::string& pathToNewVoxelDatabase, unsigned int baseNodeSize)
			:Volume<MaterialSet>(region, pathToNewVoxelDatabase, baseNodeSize)
		{
			m_pVoxelDatabase->setProperty("VoxelType", "MaterialSet");

			mOctree = new Octree<VoxelType>(this, OctreeConstructionModes::BoundCells, baseNodeSize);
		}

		TerrainVolume(const std::string& pathToExistingVoxelDatabase, WritePermission writePermission, unsigned int baseNodeSize)
			:Volume<MaterialSet>(pathToExistingVoxelDatabase, writePermission, baseNodeSize)
		{
			std::string voxelType = m_pVoxelDatabase->getPropertyAsString("VoxelType", "");
			POLYVOX_THROW_IF(voxelType != "MaterialSet", std::runtime_error, "VoxelDatabase does not have the expected VoxelType of 'MaterialSet'");

			mOctree = new Octree<VoxelType>(this, OctreeConstructionModes::BoundCells, baseNodeSize);
		}

		virtual ~TerrainVolume()
		{
			delete mOctree;
		}
	};
}

#endif //TERRAINVOLUME_H_
