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

#include "Raycasting.h"
#include "ColoredCubesVolume.h"
#include "TerrainVolume.h"

#include "PolyVox/Picking.h"

namespace Cubiquity
{
	template <typename VoxelType>
	class RaycastTestFunctor
	{
	public:
		RaycastTestFunctor()
		{
		}

		bool operator()(Vector3F pos, const VoxelType& voxel)
		{
		}
	};

	template <>
	class RaycastTestFunctor<MaterialSet>
	{
	public:
		RaycastTestFunctor()
		{
		}

		bool operator()(Vector3F pos, const MaterialSet& voxel)
		{
			mLastPos = pos;
			return voxel.getSumOfMaterials() <= MaterialSet::getMaxMaterialValue() / 2;
		}

		Vector3F mLastPos;
	};

	template <>
	class RaycastTestFunctor<Color>
	{
	public:
		RaycastTestFunctor()
		{
		}

		bool operator()(Vector3F pos, const Color& voxel)
		{
			return false;
		}

		Vector3F mLastPos;
	};

	class ColoredCubesRaycastTestFunctor
	{
	public:
		ColoredCubesRaycastTestFunctor()
		{
		}

		bool operator()(const ::PolyVox::PagedVolume<Color>::Sampler& sampler);

		Vector3I mLastPos;
	};

	bool ColoredCubesRaycastTestFunctor::operator()(const ::PolyVox::PagedVolume<Color>::Sampler& sampler)
	{
		mLastPos = sampler.getPosition();

		return sampler.getVoxel().getAlpha() == 0;
	}

	bool pickFirstSolidVoxel(ColoredCubesVolume* coloredCubesVolume, float startX, float startY, float startZ, float dirAndLengthX, float dirAndLengthY, float dirAndLengthZ, int32_t* resultX, int32_t* resultY, int32_t* resultZ)
	{
		::PolyVox::Vector3DFloat start(startX, startY, startZ);
		::PolyVox::Vector3DFloat dirAndLength(dirAndLengthX, dirAndLengthY, dirAndLengthZ);

		::PolyVox::PickResult result = ::PolyVox::pickVoxel(coloredCubesVolume->_getPolyVoxVolume(), start, dirAndLength, Color(0, 0, 0, 0));

		if(result.didHit)
		{
			*resultX = result.hitVoxel.getX();
			*resultY = result.hitVoxel.getY();
			*resultZ = result.hitVoxel.getZ();
			return true;
		}
		else
		{
			return false;
		}
	}

	bool pickLastEmptyVoxel(ColoredCubesVolume* coloredCubesVolume, float startX, float startY, float startZ, float dirAndLengthX, float dirAndLengthY, float dirAndLengthZ, int32_t* resultX, int32_t* resultY, int32_t* resultZ)
	{
		::PolyVox::Vector3DFloat start(startX, startY, startZ);
		::PolyVox::Vector3DFloat dirAndLength(dirAndLengthX, dirAndLengthY, dirAndLengthZ);

		::PolyVox::PickResult result = ::PolyVox::pickVoxel(coloredCubesVolume->_getPolyVoxVolume(), start, dirAndLength, Color(0, 0, 0, 0));

		if(result.didHit)
		{
			*resultX = result.previousVoxel.getX();
			*resultY = result.previousVoxel.getY();
			*resultZ = result.previousVoxel.getZ();
			return true;
		}
		else
		{
			return false;
		}
	}

	bool pickTerrainSurface(TerrainVolume* terrainVolume, float startX, float startY, float startZ, float dirAndLengthX, float dirAndLengthY, float dirAndLengthZ, float* resultX, float* resultY, float* resultZ)
	{
		Vector3F v3dStart(startX, startY, startZ);
		Vector3F v3dDirection(dirAndLengthX, dirAndLengthY, dirAndLengthZ);
		//v3dDirection *= length;

		RaycastTestFunctor<MaterialSet> raycastTestFunctor;
		::PolyVox::RaycastResult myResult = terrainRaycastWithDirection(dynamic_cast<TerrainVolume*>(terrainVolume)->_getPolyVoxVolume(), v3dStart, v3dDirection, raycastTestFunctor, 0.5f);
		if(myResult == ::PolyVox::RaycastResults::Interupted)
		{
			*resultX = raycastTestFunctor.mLastPos.getX();
			*resultY = raycastTestFunctor.mLastPos.getY();
			*resultZ = raycastTestFunctor.mLastPos.getZ();
			return true;
		}

		return false;
	}
}