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

#ifndef CUBIQUITY_SMOOTHSURFACEEXTRACTIONTASK_H_
#define CUBIQUITY_SMOOTHSURFACEEXTRACTIONTASK_H_

#include "CubiquityForwardDeclarations.h"
#include "OctreeNode.h"
#include "Task.h"

namespace Cubiquity
{
	class SmoothSurfaceExtractionTask : public Task
	{
	public:
		SmoothSurfaceExtractionTask(OctreeNode< MaterialSet >* octreeNode, ::PolyVox::PagedVolume<MaterialSet>* polyVoxVolume);
		~SmoothSurfaceExtractionTask();

		void process(void);

		void generateSmoothMesh(const Region& region, uint32_t lodLevel, TerrainMesh* resultMesh);

	public:
		OctreeNode< MaterialSet >* mOctreeNode;
		::PolyVox::PagedVolume<MaterialSet>* mPolyVoxVolume;
		TerrainMesh* mPolyVoxMesh;
		Timestamp mProcessingStartedTimestamp;

		// Whether the task owns the mesh, or whether it has been passed to
		// the OctreeNode. Should probably switch this to use a smart pointer.
		bool mOwnMesh;
	};

	void recalculateMaterials(TerrainMesh* mesh, const Vector3F& meshOffset, ::PolyVox::PagedVolume<MaterialSet>* volume);
	MaterialSet getInterpolatedValue(::PolyVox::PagedVolume<MaterialSet>* volume, const Vector3F& position);

	template< typename SrcPolyVoxVolumeType, typename DstPolyVoxVolumeType>
	void resampleVolume(uint32_t factor, SrcPolyVoxVolumeType* srcVolume, const Region& srcRegion, DstPolyVoxVolumeType* dstVolume, const Region& dstRegion)
	{
		POLYVOX_ASSERT(srcRegion.getWidthInCells() == dstRegion.getWidthInCells() * factor, "Destination volume must be half the size of source volume");
		POLYVOX_ASSERT(srcRegion.getHeightInCells() == dstRegion.getHeightInCells() * factor, "Destination volume must be half the size of source volume");
		POLYVOX_ASSERT(srcRegion.getDepthInCells() == dstRegion.getDepthInCells() * factor, "Destination volume must be half the size of source volume");

		for(int32_t dz = dstRegion.getLowerCorner().getZ(); dz <= dstRegion.getUpperCorner().getZ(); dz++)
		{
			for(int32_t dy = dstRegion.getLowerCorner().getY(); dy <= dstRegion.getUpperCorner().getY(); dy++)
			{
				for(int32_t dx = dstRegion.getLowerCorner().getX(); dx <= dstRegion.getUpperCorner().getX(); dx++)
				{
					int32_t sx = (dx - dstRegion.getLowerCorner().getX()) * factor + srcRegion.getLowerCorner().getX();
					int32_t sy = (dy - dstRegion.getLowerCorner().getY()) * factor + srcRegion.getLowerCorner().getY();
					int32_t sz = (dz - dstRegion.getLowerCorner().getZ()) * factor + srcRegion.getLowerCorner().getZ();

					const MaterialSet& srcVoxel = srcVolume->getVoxel(sx,sy,sz);
					dstVolume->setVoxel(dx,dy,dz,srcVoxel);
				}
			}
		}
	}
}

#endif //CUBIQUITY_SMOOTHSURFACEEXTRACTIONTASK_H_
