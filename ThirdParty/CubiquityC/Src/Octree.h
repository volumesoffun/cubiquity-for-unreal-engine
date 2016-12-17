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

#ifndef CUBIQUITY_OCTREE_H_
#define CUBIQUITY_OCTREE_H_

#include "Clock.h"
#include "ConcurrentQueue.h"
#include "CubiquityForwardDeclarations.h"
#include "Region.h"
#include "Task.h"
#include "Vector.h"
#include "VoxelTraits.h"

#include <vector>

namespace Cubiquity
{
	namespace OctreeConstructionModes
	{
		enum OctreeConstructionMode
		{
			BoundVoxels = 0,
			BoundCells = 1
		};
	}
	typedef OctreeConstructionModes::OctreeConstructionMode OctreeConstructionMode;

	template <typename VoxelType>
	class Octree
	{
		friend class OctreeNode<VoxelType>;

	public:
		static const uint16_t InvalidNodeIndex = 0xFFFF;

		Octree(Volume<VoxelType>* volume, OctreeConstructionMode octreeConstructionMode, unsigned int baseNodeSize);
		~Octree();

		template<typename VisitorType>
		void acceptVisitor(VisitorType visitor) { visitNode(getRootNode(), visitor); }

		OctreeNode<VoxelType>* getRootNode(void) { return mNodes[mRootNodeIndex]; }

		Volume<VoxelType>* getVolume(void) { return mVolume; }

		// This one feels hacky?
		OctreeNode<VoxelType>* getNodeFromIndex(uint16_t index) { return mNodes[index]; }

		bool update(const Vector3F& viewPosition, float lodThreshold);

		void markDataAsModified(int32_t x, int32_t y, int32_t z, Timestamp newTimeStamp);
		void markDataAsModified(const Region& region, Timestamp newTimeStamp);

		void buildOctreeNodeTree(uint16_t parent);
		void determineActiveNodes(OctreeNode<VoxelType>* octreeNode, const Vector3F& viewPosition, float lodThreshold);

		concurrent_queue<typename VoxelTraits<VoxelType>::SurfaceExtractionTaskType*, TaskSortCriterion> mFinishedSurfaceExtractionTasks;

		void setLodRange(int32_t minimumLOD, int32_t maximumLOD);

		// Note that the maximum LOD refers to the *most detailed* LOD, which is actually the *smallest* hieght
		// in the octree (the greatest depth). If confused, think how texture mipmapping works, where the most 
		// detailed MIP is number zero. Level zero is the raw voxel data and succesive levels downsample it.
		int32_t mMaximumLOD;
		int32_t mMinimumLOD;

	private:
		uint16_t createNode(Region region, uint16_t parent);

		template<typename VisitorType>
		void visitNode(OctreeNode<VoxelType>* node, VisitorType& visitor);

		void markAsModified(uint16_t index, int32_t x, int32_t y, int32_t z, Timestamp newTimeStamp);
		void markAsModified(uint16_t index, const Region& region, Timestamp newTimeStamp);

		Timestamp propagateTimestamps(uint16_t index);

		void scheduleUpdateIfNeeded(OctreeNode<VoxelType>* node, const Vector3F& viewPosition);

		void determineWhetherToRenderNode(uint16_t index);

		std::vector< OctreeNode<VoxelType>*> mNodes;

		uint16_t mRootNodeIndex;
		const unsigned int mBaseNodeSize;

		Volume<VoxelType>* mVolume;		

		// The extent of the octree may be significantly larger than the volume, but we only want to
		// create nodes which actually overlap the volume (otherwise they are guarenteed to be empty).
		Region mRegionToCover;

		OctreeConstructionMode mOctreeConstructionMode;
	};
}

#include "Octree.inl"

#endif //CUBIQUITY_OCTREE_H_
