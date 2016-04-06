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

#ifndef COLOUREDCUBESVOLUME_H_
#define COLOUREDCUBESVOLUME_H_

#include "Color.h"
#include "Cubiquity.h"
#include "CubiquityForwardDeclarations.h"
#include "Volume.h"

namespace Cubiquity
{
	class ColoredCubesVolume : public Volume<Color>
	{
	public:
		typedef Color VoxelType;

		ColoredCubesVolume(const Region& region, const std::string& pathToNewVoxelDatabase, unsigned int baseNodeSize)
			:Volume<Color>(region, pathToNewVoxelDatabase, baseNodeSize)
		{
			m_pVoxelDatabase->setProperty("VoxelType", "Color");

			mOctree = new Octree<VoxelType>(this, OctreeConstructionModes::BoundVoxels, baseNodeSize);
		}

		ColoredCubesVolume(const std::string& pathToExistingVoxelDatabase, WritePermission writePermission, unsigned int baseNodeSize)
			:Volume<Color>(pathToExistingVoxelDatabase, writePermission, baseNodeSize)
		{
			std::string voxelType = m_pVoxelDatabase->getPropertyAsString("VoxelType", "");
			POLYVOX_THROW_IF(voxelType != "Color", std::runtime_error, "VoxelDatabase does not have the expected VoxelType of 'Color'");

			mOctree = new Octree<VoxelType>(this, OctreeConstructionModes::BoundVoxels, baseNodeSize);
		}

		virtual ~ColoredCubesVolume()
		{
			delete mOctree;
		}
	};
}

#endif //COLOUREDCUBESVOLUME_H_
