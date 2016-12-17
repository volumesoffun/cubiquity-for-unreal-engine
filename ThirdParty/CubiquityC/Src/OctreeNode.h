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

#ifndef OCTREE_NODE_H_
#define OCTREE_NODE_H_

#include "Clock.h"
#include "CubiquityForwardDeclarations.h"
#include "Region.h"
#include "Vector.h"
#include "VoxelTraits.h"

namespace Cubiquity
{
	template <typename VoxelType>
	class OctreeNode
	{
		friend class Octree<VoxelType>;

	public:	
		OctreeNode(Region region, uint16_t parent, Octree<VoxelType>* octree);
		~OctreeNode();

		OctreeNode* getChildNode(uint32_t childX, uint32_t childY, uint32_t childZ);
		OctreeNode* getParentNode(void);

		const ::PolyVox::Mesh< typename VoxelTraits<VoxelType>::VertexType, uint16_t >* getMesh(void);
		void setMesh(const ::PolyVox::Mesh< typename VoxelTraits<VoxelType>::VertexType, uint16_t >* mesh);

		bool isActive(void);
		void setActive(bool active);

		bool renderThisNode(void);
		void setRenderThisNode(bool render);

		bool isMeshUpToDate(void);
		bool isSceduledForUpdate(void);

		void updateFromCompletedTask(typename VoxelTraits<VoxelType>::SurfaceExtractionTaskType* completedTask);

		Region mRegion;
		Timestamp mDataLastModified;
		Timestamp mLastSceduledForUpdate;

		Timestamp mStructureLastChanged;
		Timestamp mPropertiesLastChanged;
		Timestamp mMeshLastChanged;
		Timestamp mNodeOrChildrenLastChanged;

		Octree<VoxelType>* mOctree;

		// Use flags here?
		
		bool mCanRenderNodeOrChildren;
		bool mIsLeaf;

		uint8_t mHeight; // Zero for leaf nodes.

		typename VoxelTraits<VoxelType>::SurfaceExtractionTaskType* mLastSurfaceExtractionTask;

		uint16_t mSelf;
		uint16_t children[2][2][2];

	private:
		uint16_t mParent;		

		bool mRenderThisNode;
		bool mActive;

		const ::PolyVox::Mesh< typename VoxelTraits<VoxelType>::VertexType, uint16_t >* mPolyVoxMesh;
	};
}

#include "OctreeNode.inl"

#endif //OCTREE_NODE_H_
