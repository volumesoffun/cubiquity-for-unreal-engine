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

#include "Volume.h"

#include "ColoredCubicSurfaceExtractionTask.h"
#include "Octree.h"
#include "SmoothSurfaceExtractionTask.h"

#include "PolyVox/Region.h"
#include "PolyVox/Mesh.h"

#include <limits>
#include <sstream>

namespace Cubiquity
{
	template <typename VoxelType>
	OctreeNode<VoxelType>::OctreeNode(Region region, uint16_t parent, Octree<VoxelType>* octree)
		:mRegion(region)
		,mParent(parent)
		,mOctree(octree)
		,mRenderThisNode(false)
		,mCanRenderNodeOrChildren(false)
		,mActive(false)
		,mLastSceduledForUpdate(0) // The values of these few initialisations is important
		,mMeshLastChanged(1)	   // to make sure the node is set to an 'out of date' 
		,mDataLastModified(2)      // state which will then try to update.
		,mStructureLastChanged(1)
		,mPropertiesLastChanged(1)
		,mNodeOrChildrenLastChanged(1)
		,mPolyVoxMesh(0)
		,mHeight(0)
		,mLastSurfaceExtractionTask(0)
	{
		for(int z = 0; z < 2; z++)
		{
			for(int y = 0; y < 2; y++)
			{
				for(int x = 0; x < 2; x++)
				{
					children[x][y][z] = Octree<VoxelType>::InvalidNodeIndex;
				}
			}
		}
	}

	template <typename VoxelType>
	OctreeNode<VoxelType>::~OctreeNode()
	{
		delete mPolyVoxMesh;
	}

	template <typename VoxelType>
	OctreeNode<VoxelType>* OctreeNode<VoxelType>::getChildNode(uint32_t childX, uint32_t childY, uint32_t childZ)
	{
		uint16_t childIndex = children[childX][childY][childZ];
		if (childIndex != Octree<VoxelType>::InvalidNodeIndex)
		{
			OctreeNode<VoxelType>* child = mOctree->mNodes[children[childX][childY][childZ]];
			if (child->isActive())
			{
				return child;
			}
		}

		return 0;
	}

	template <typename VoxelType>
	OctreeNode<VoxelType>* OctreeNode<VoxelType>::getParentNode(void)
	{
		return mParent == Octree<VoxelType>::InvalidNodeIndex ? 0 : mOctree->mNodes[mParent];
	}

	template <typename VoxelType>
	const ::PolyVox::Mesh< typename VoxelTraits<VoxelType>::VertexType, uint16_t >* OctreeNode<VoxelType>::getMesh(void)
	{
		return mPolyVoxMesh;
	}

	template <typename VoxelType>
	void OctreeNode<VoxelType>::setMesh(const ::PolyVox::Mesh< typename VoxelTraits<VoxelType>::VertexType, uint16_t >* mesh)
	{
		if (mPolyVoxMesh)
		{
			delete mPolyVoxMesh;
			mPolyVoxMesh = 0;
		}

		mPolyVoxMesh = mesh;

		mMeshLastChanged = Clock::getTimestamp();

		/*if (mPolyVoxMesh == 0)
		{
			// Force the mesh to be updated next time it is needed.
			mDataLastModified = Clock::getTimestamp();
		}*/
	}

	template <typename VoxelType>
	bool OctreeNode<VoxelType>::isActive(void)
	{
		return mActive;
	}

	template <typename VoxelType>
	void OctreeNode<VoxelType>::setActive(bool active)
	{
		if (mActive != active)
		{
			mActive = active;

			// When a node is activated or deactivated it is the structure of the *parent* 
			// which has changed (i.e. the parent has gained or lost a child (this node).
			if (getParentNode())
			{
				getParentNode()->mStructureLastChanged = Clock::getTimestamp();
			}
		}
	}

	template <typename VoxelType>
	bool OctreeNode<VoxelType>::renderThisNode(void)
	{
		return mRenderThisNode;
	}

	template <typename VoxelType>
	void OctreeNode<VoxelType>::setRenderThisNode(bool render)
	{
		if (mRenderThisNode != render)
		{
			mRenderThisNode = render;
			mPropertiesLastChanged = Clock::getTimestamp();
		}
	}

	template <typename VoxelType>
	bool OctreeNode<VoxelType>::isMeshUpToDate(void)
	{
		return mMeshLastChanged > mDataLastModified;
	}

	template <typename VoxelType>
	bool OctreeNode<VoxelType>::isSceduledForUpdate(void)
	{
		//We are sceduled for an update if being sceduled was the most recent thing that happened.
		return (mLastSceduledForUpdate > mDataLastModified) && (mLastSceduledForUpdate > mMeshLastChanged);
	}

	template <typename VoxelType>
	void OctreeNode<VoxelType>::updateFromCompletedTask(typename VoxelTraits<VoxelType>::SurfaceExtractionTaskType* completedTask)
	{
		// Assign a new mesh if available
		/*if(completedTask->mPolyVoxMesh->getNoOfIndices() > 0)
		{*/
			setMesh(completedTask->mPolyVoxMesh);
			completedTask->mOwnMesh = false; // So the task doesn't delete the mesh
		/*}
		else // Otherwise it will just be deleted.
		{
			setMesh(0);
		}*/
	}
}
