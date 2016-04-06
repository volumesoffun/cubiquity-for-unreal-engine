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

#ifndef VOLUME_H_
#define VOLUME_H_

#include "BackgroundTaskProcessor.h"
#include "CubiquityForwardDeclarations.h"
#include "Octree.h"
#include "Vector.h"
#include "VoxelTraits.h"
#include "WritePermissions.h"

#include "PolyVox/Array.h"
#include "PolyVox/Material.h"
#include "PolyVox/RawVolume.h"

#include "PolyVox/PagedVolume.h"

#include "PolyVox/CubicSurfaceExtractor.h"
#include "PolyVox/MarchingCubesSurfaceExtractor.h"

#include "SQLite/sqlite3.h"

namespace Cubiquity
{
	template <typename _VoxelType>
	class Volume
	{
	public:
		typedef _VoxelType VoxelType;

		Volume(const Region& region, const std::string& pathToNewVoxelDatabase, uint32_t baseNodeSize);
		Volume(const std::string& pathToExistingVoxelDatabase, WritePermission writePermission, uint32_t baseNodeSize);
		~Volume();

		// These functions just forward to the underlying PolyVox volume.
		uint32_t getWidth(void) const { return mPolyVoxVolume->getWidth(); }
		uint32_t getHeight(void) const { return mPolyVoxVolume->getHeight(); }
		uint32_t getDepth(void) const { return mPolyVoxVolume->getDepth(); }
		const Region& getEnclosingRegion(void) const { return mEnclosingRegion; }

		// Note this adds a border rather than calling straight through.
		VoxelType getVoxel(int32_t x, int32_t y, int32_t z) const;

		// This one's a bit of a hack... direct access to underlying PolyVox volume
		::PolyVox::PagedVolume<VoxelType>* _getPolyVoxVolume(void) const { return mPolyVoxVolume; }

		// Octree access
		Octree<VoxelType>* getOctree(void) { return mOctree; };
		OctreeNode<VoxelType>* getRootOctreeNode(void) { return mOctree->getRootNode(); }

		// Set voxel doesn't just pass straight through, it also validates the position and marks the voxel as modified.
		void setVoxel(int32_t x, int32_t y, int32_t z, VoxelType value, bool markAsModified);

		// Marks a region as modified so it will be regenerated later.
		void markAsModified(const Region& region);

		void acceptOverrideChunks(void)
		{
			mPolyVoxVolume->flushAll();
			m_pVoxelDatabase->acceptOverrideChunks();
		}
		
		void discardOverrideChunks(void)
		{
			mPolyVoxVolume->flushAll();
			m_pVoxelDatabase->discardOverrideChunks();
		}

		// Should be called before rendering a frame to update the meshes and octree structure.
		virtual bool update(const Vector3F& viewPosition, float lodThreshold);

		// It's a bit ugly that the background task processor is part of the volume class.
		// We do this because we want to clear it when the volume is destroyed, to avoid
		// the situation wher it continues to process tasks from the destroyed volume.
		// A better solution is needed here (smart pointers?).
		BackgroundTaskProcessor* mBackgroundTaskProcessor;

	protected:
		Octree<VoxelType>* mOctree;
		VoxelDatabase<VoxelType>* m_pVoxelDatabase;

	private:
		Volume& operator=(const Volume&);

		::PolyVox::Region mEnclosingRegion;
		::PolyVox::PagedVolume<VoxelType>* mPolyVoxVolume;

		//sqlite3* mDatabase;

		

		// Friend functions
		friend class Octree<VoxelType>;
		friend class OctreeNode<VoxelType>;
	};
}

#include "Volume.inl"

#endif //VOLUME_H_
