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

#include "OctreeNode.h"
#include "Volume.h"
#include "MainThreadTaskProcessor.h"

#include <algorithm>

namespace Cubiquity
{
	template <typename VoxelType>
	class PropagateTimestampsVisitor
	{
	public:
		PropagateTimestampsVisitor()
		{
			subtreeTimestamp = 0;
		}

		bool preChildren(OctreeNode<VoxelType>* octreeNode)
		{
			// Don't actually do any work here, just make sure all children get processed.
			return true;
		}

		void postChildren(OctreeNode<VoxelType>* octreeNode)
		{
			// Set timestamp to max of our own timestamps, and those of our children.
			octreeNode->mNodeOrChildrenLastChanged = (std::max)({ subtreeTimestamp,
				octreeNode->mStructureLastChanged, octreeNode->mPropertiesLastChanged, octreeNode->mMeshLastChanged });

			// This will get propagatd back to the parent as the visitor is passed by reference.
			subtreeTimestamp = octreeNode->mNodeOrChildrenLastChanged;
		}

	private:
		// The visitor has no direct access to the children, so we
		// use this to propagate the timestamp back up to the parent.
		Timestamp subtreeTimestamp;
	};

	template <typename VoxelType>
	class ScheduleUpdateIfNeededVisitor
	{
	public:
		ScheduleUpdateIfNeededVisitor(const Vector3F& viewPosition)
			:mViewPosition(viewPosition)
		{
		}

		bool preChildren(OctreeNode<VoxelType>* octreeNode)
		{
			if
			(
				(octreeNode->isMeshUpToDate() == false) && 
				(octreeNode->isSceduledForUpdate() == false) && 
				(
					(octreeNode->mLastSurfaceExtractionTask == 0) || 
					(octreeNode->mLastSurfaceExtractionTask->mProcessingStartedTimestamp < Clock::getTimestamp())
				) && 
				(octreeNode->isActive() &&
				(octreeNode->mHeight <= octreeNode->mOctree->mMinimumLOD) && // Remember that min and max 
				(octreeNode->mHeight >= octreeNode->mOctree->mMaximumLOD))   // are counter-intuitive here!
			)
			{
				octreeNode->mLastSceduledForUpdate = Clock::getTimestamp();

				octreeNode->mLastSurfaceExtractionTask = new typename VoxelTraits<VoxelType>::SurfaceExtractionTaskType(octreeNode, octreeNode->mOctree->getVolume()->_getPolyVoxVolume());

				// If the node was rendered last frame then this update is probably the result of an editing operation, rather than
				// the node only just becoming visible. For editing operations it is important to process them immediatly so that we
				// don't see temporary cracks in the mesh as different parts up updated at different times.
				//if(node->mExtractOnMainThread) //This flag should still be set from last frame.
				{
					// We're going to process immediatly, but the completed task will still get queued in the finished
					// queue, and we want to make sure it's the first out. So we still set a priority and make it high.
					octreeNode->mLastSurfaceExtractionTask->mPriority = (std::numeric_limits<uint32_t>::max)();

					if (octreeNode->renderThisNode()) // Still set from last frame. If we rendered it then we will probably want it again.
					{
						gMainThreadTaskProcessor.addTask(octreeNode->mLastSurfaceExtractionTask);
					}
					else
					{
						octreeNode->mOctree->getVolume()->mBackgroundTaskProcessor->addTask(octreeNode->mLastSurfaceExtractionTask);
					}
				}
				/*else
				{
				// Note: tasks get sorted by their distance from the camera at the time they are added. If we
				// want to account for the camera moving then we would have to sort the task queue each frame.
				Vector3F regionCentre = static_cast<Vector3F>(node->mRegion.getCentre());
				float distance = (viewPosition - regionCentre).length(); //We don't use distance squared to keep the values smaller
				node->mLastSurfaceExtractionTask->mPriority = (std::numeric_limits<uint32_t>::max)() - static_cast<uint32_t>(distance);
				gBackgroundTaskProcessor.addTask(node->mLastSurfaceExtractionTask);
				}*/
			}

			return true;
		}

		void postChildren(OctreeNode<VoxelType>* octreeNode) {}

	private:
		Vector3F mViewPosition;
	};

	template <typename VoxelType>
	Octree<VoxelType>::Octree(Volume<VoxelType>* volume, OctreeConstructionMode octreeConstructionMode, unsigned int baseNodeSize)
		:mVolume(volume)
		, mRootNodeIndex(InvalidNodeIndex)
		, mBaseNodeSize(baseNodeSize)
		, mOctreeConstructionMode(octreeConstructionMode)
		, mMaximumLOD(0)
		, mMinimumLOD(2) // Must be *more* than maximum
	{
		mRegionToCover = mVolume->getEnclosingRegion();
		if(mOctreeConstructionMode == OctreeConstructionModes::BoundVoxels)
		{
			mRegionToCover.shiftUpperCorner(1, 1, 1);
		}
		else if(mOctreeConstructionMode == OctreeConstructionModes::BoundCells)
		{
			mRegionToCover.shiftLowerCorner(-1, -1, -1);
			mRegionToCover.shiftUpperCorner(1, 1, 1);
		}

		POLYVOX_ASSERT(::PolyVox::isPowerOf2(mBaseNodeSize), "Node size must be a power of two");

		uint32_t largestVolumeDimension = (std::max)(mRegionToCover.getWidthInVoxels(), (std::max)(mRegionToCover.getHeightInVoxels(), mRegionToCover.getDepthInVoxels()));
		if(mOctreeConstructionMode == OctreeConstructionModes::BoundCells)
		{
			largestVolumeDimension--;
		}

		uint32_t octreeTargetSize = ::PolyVox::upperPowerOfTwo(largestVolumeDimension);

		uint8_t maxHeightOfTree = ::PolyVox::logBase2((octreeTargetSize) / mBaseNodeSize) + 1;

		uint32_t regionToCoverWidth = (mOctreeConstructionMode == OctreeConstructionModes::BoundCells) ? mRegionToCover.getWidthInCells() : mRegionToCover.getWidthInVoxels();
		uint32_t regionToCoverHeight = (mOctreeConstructionMode == OctreeConstructionModes::BoundCells) ? mRegionToCover.getHeightInCells() : mRegionToCover.getHeightInVoxels();
		uint32_t regionToCoverDepth = (mOctreeConstructionMode == OctreeConstructionModes::BoundCells) ? mRegionToCover.getDepthInCells() : mRegionToCover.getDepthInVoxels();

		uint32_t widthIncrease = octreeTargetSize - regionToCoverWidth;
		uint32_t heightIncrease = octreeTargetSize - regionToCoverHeight;
		uint32_t depthIncrease = octreeTargetSize - regionToCoverDepth;

		Region octreeRegion(mRegionToCover);
	
		if(widthIncrease % 2 == 1)
		{
			octreeRegion.setUpperX(octreeRegion.getUpperX() + 1);
			widthIncrease--;
		}

		if(heightIncrease % 2 == 1)
		{
			octreeRegion.setUpperY(octreeRegion.getUpperY() + 1);
			heightIncrease--;
		}
		if(depthIncrease % 2 == 1)
		{
			octreeRegion.setUpperZ(octreeRegion.getUpperZ() + 1);
			depthIncrease--;
		}

		octreeRegion.grow(widthIncrease / 2, heightIncrease / 2, depthIncrease / 2);

		mRootNodeIndex = createNode(octreeRegion, InvalidNodeIndex);
		mNodes[mRootNodeIndex]->mHeight = maxHeightOfTree - 1;

		buildOctreeNodeTree(mRootNodeIndex);
	}

	template <typename VoxelType>
	Octree<VoxelType>::~Octree()
	{
		for(uint32_t ct = 0; ct < mNodes.size(); ct++)
		{
			delete mNodes[ct];
		}
	}

	template <typename VoxelType>
	uint16_t Octree<VoxelType>::createNode(Region region, uint16_t parent)
	{
		OctreeNode< VoxelType >* node = new OctreeNode< VoxelType >(region, parent, this);

		if(parent != InvalidNodeIndex)
		{
			POLYVOX_ASSERT(mNodes[parent]->mHeight < 100, "Node height has gone below zero and wrapped around.");
			node->mHeight = mNodes[parent]->mHeight-1;
		}

		mNodes.push_back(node);
		POLYVOX_ASSERT(mNodes.size() < InvalidNodeIndex, "Too many octree nodes!");
		uint16_t index = mNodes.size() - 1;
		mNodes[index]->mSelf = index;
		return index;
	}

	template <typename VoxelType>
	bool Octree<VoxelType>::update(const Vector3F& viewPosition, float lodThreshold)
	{
		// This isn't a vistior because visitors only visit active nodes, and here we are setting them.
		determineActiveNodes(getRootNode(), viewPosition, lodThreshold);

		acceptVisitor(ScheduleUpdateIfNeededVisitor<VoxelType>(viewPosition));


		// Make sure any surface extraction tasks which were scheduled on the main thread get processed before we determine what to render.
		if (gMainThreadTaskProcessor.hasTasks())
		{
			gMainThreadTaskProcessor.processAllTasks(); //Doesn't really belong here
		}
		else
		{
			getVolume()->mBackgroundTaskProcessor->processOneTask(); //Doesn't really belong here
		}

		// This will include tasks from both the background and main threads.
		while(!mFinishedSurfaceExtractionTasks.empty())
		{
			typename VoxelTraits<VoxelType>::SurfaceExtractionTaskType* task;
			mFinishedSurfaceExtractionTasks.wait_and_pop(task);

			task->mOctreeNode->updateFromCompletedTask(task);

			if(task->mOctreeNode->mLastSurfaceExtractionTask == task)
			{
				task->mOctreeNode->mLastSurfaceExtractionTask = 0;
			}

			delete task;
		}

		//acceptVisitor(DetermineWhetherToRenderVisitor<VoxelType>());

		determineWhetherToRenderNode(mRootNodeIndex);

		acceptVisitor(PropagateTimestampsVisitor<VoxelType>());

		// If there are no pending tasks then return truem to indicate we are up to date.
		return (gMainThreadTaskProcessor.hasTasks() == false) && (getVolume()->mBackgroundTaskProcessor->hasTasks() == false);
	}

	template <typename VoxelType>
	void Octree<VoxelType>::markDataAsModified(int32_t x, int32_t y, int32_t z, Timestamp newTimeStamp)
	{
		markAsModified(mRootNodeIndex, x, y, z, newTimeStamp);
	}

	template <typename VoxelType>
	void Octree<VoxelType>::markDataAsModified(const Region& region, Timestamp newTimeStamp)
	{		
		markAsModified(mRootNodeIndex, region, newTimeStamp);
	}

	template <typename VoxelType>
	void Octree<VoxelType>::setLodRange(int32_t minimumLOD, int32_t maximumLOD)
	{
		POLYVOX_THROW_IF(minimumLOD < maximumLOD, std::invalid_argument, "Invalid LOD range. For LOD levels, the 'minimum' must be *more* than or equal to the 'maximum'");
		mMinimumLOD = minimumLOD;
		mMaximumLOD = maximumLOD;
	}

	template <typename VoxelType>
	void Octree<VoxelType>::buildOctreeNodeTree(uint16_t parent)
	{
		POLYVOX_ASSERT(mNodes[parent]->mRegion.getWidthInVoxels() == mNodes[parent]->mRegion.getHeightInVoxels(), "Region must be cubic");
		POLYVOX_ASSERT(mNodes[parent]->mRegion.getWidthInVoxels() == mNodes[parent]->mRegion.getDepthInVoxels(), "Region must be cubic");

		//We know that width/height/depth are all the same.
		uint32_t parentSize = static_cast<uint32_t>((mOctreeConstructionMode == OctreeConstructionModes::BoundCells) ? mNodes[parent]->mRegion.getWidthInCells() : mNodes[parent]->mRegion.getWidthInVoxels());

		if(parentSize > mBaseNodeSize)
		{
			Vector3I baseLowerCorner = mNodes[parent]->mRegion.getLowerCorner();
			int32_t childSize = (mOctreeConstructionMode == OctreeConstructionModes::BoundCells) ? mNodes[parent]->mRegion.getWidthInCells() / 2 : mNodes[parent]->mRegion.getWidthInVoxels() / 2;

			Vector3I baseUpperCorner;
			if(mOctreeConstructionMode == OctreeConstructionModes::BoundCells)
			{
				baseUpperCorner = baseLowerCorner + Vector3I(childSize, childSize, childSize);
			}
			else
			{
				baseUpperCorner = baseLowerCorner + Vector3I(childSize-1, childSize-1, childSize-1);
			}

			for(int z = 0; z < 2; z++)
			{
				for(int y = 0; y < 2; y++)
				{
					for(int x = 0; x < 2; x++)
					{
						Vector3I offset (x*childSize, y*childSize, z*childSize);
						Region childRegion(baseLowerCorner + offset, baseUpperCorner + offset);
						if(intersects(childRegion, mRegionToCover))
						{
							uint16_t octreeNode = createNode(childRegion, parent);
							mNodes[parent]->children[x][y][z] = octreeNode;
							buildOctreeNodeTree(octreeNode);
						}
					}
				}
			}
		}
	}

	template <typename VoxelType>
	void Octree<VoxelType>::markAsModified(uint16_t index, int32_t x, int32_t y, int32_t z, Timestamp newTimeStamp)
	{
		// Note - Can't this function just call the other version?

		OctreeNode<VoxelType>* node = mNodes[index];

		Region dilatedRegion = node->mRegion;
		dilatedRegion.grow(1); //FIXME - Think if we really need this dilation?

		if(dilatedRegion.containsPoint(x, y, z))
		{
			//mIsMeshUpToDate = false;
			node->mDataLastModified = newTimeStamp;

			for(int iz = 0; iz < 2; iz++)
			{
				for(int iy = 0; iy < 2; iy++)
				{
					for(int ix = 0; ix < 2; ix++)
					{
						uint16_t childIndex = node->children[ix][iy][iz];
						if(childIndex != InvalidNodeIndex)
						{
							markAsModified(childIndex, x, y, z, newTimeStamp);
						}
					}
				}
			}
		}
	}

	template <typename VoxelType>
	void Octree<VoxelType>::markAsModified(uint16_t index, const Region& region, Timestamp newTimeStamp)
	{
		OctreeNode<VoxelType>* node = mNodes[index];

		if(intersects(node->mRegion, region))
		{
			//mIsMeshUpToDate = false;
			node->mDataLastModified = newTimeStamp;

			for(int iz = 0; iz < 2; iz++)
			{
				for(int iy = 0; iy < 2; iy++)
				{
					for(int ix = 0; ix < 2; ix++)
					{
						uint16_t childIndex = node->children[ix][iy][iz];
						if(childIndex != InvalidNodeIndex)
						{
							markAsModified(childIndex, region, newTimeStamp);
						}
					}
				}
			}
		}
	}

	template <typename VoxelType>
	void Octree<VoxelType>::determineActiveNodes(OctreeNode<VoxelType>* octreeNode, const Vector3F& viewPosition, float lodThreshold)
	{
		// FIXME - Should have an early out to set active to false if parent is false.

		OctreeNode<VoxelType>* parentNode = octreeNode->getParentNode();
		if (parentNode)
		{
			Vector3F regionCentre = static_cast<Vector3F>(parentNode->mRegion.getCentre());

			float distance = (viewPosition - regionCentre).length();

			Vector3I diagonal = parentNode->mRegion.getUpperCorner() - parentNode->mRegion.getLowerCorner();
			float diagonalLength = diagonal.length(); // A measure of our regions size

			float projectedSize = diagonalLength / distance;

			// As we move far away only the highest nodes will be larger than the threshold. But these may be too
			// high to ever generate meshes, so we set here a maximum height for which nodes can be set to inacive.
			bool active = (projectedSize > lodThreshold) || (octreeNode->mHeight >= mMinimumLOD);

			octreeNode->setActive(active);
		}
		else
		{
			octreeNode->setActive(true);
		}

		octreeNode->mIsLeaf = true;

		for (int iz = 0; iz < 2; iz++)
		{
			for (int iy = 0; iy < 2; iy++)
			{
				for (int ix = 0; ix < 2; ix++)
				{
					uint16_t childIndex = octreeNode->children[ix][iy][iz];
					if (childIndex != InvalidNodeIndex)
					{
						OctreeNode<VoxelType>* childNode = mNodes[childIndex];
						determineActiveNodes(childNode, viewPosition, lodThreshold);
					}

					// If we have (or have just created) an active and valid child then we are not a leaf.
					if (octreeNode->getChildNode(ix, iy, iz))
					{
						octreeNode->mIsLeaf = false;
					}
				}
			}
		}
	}

	template <typename VoxelType>
	void Octree<VoxelType>::determineWhetherToRenderNode(uint16_t index)
	{
		OctreeNode<VoxelType>* node = mNodes[index];
		if (node->mIsLeaf)
		{
			node->mCanRenderNodeOrChildren = node->isMeshUpToDate();
			node->setRenderThisNode(node->isMeshUpToDate());
			return;
		}

		bool canRenderAllChildren = true;
		for (int iz = 0; iz < 2; iz++)
		{
			for (int iy = 0; iy < 2; iy++)
			{
				for (int ix = 0; ix < 2; ix++)
				{
					uint16_t childIndex = node->children[ix][iy][iz];
					if (childIndex != InvalidNodeIndex)
					{
						OctreeNode<VoxelType>* childNode = mNodes[childIndex];
						if (childNode->isActive())
						{
							determineWhetherToRenderNode(childIndex);
							canRenderAllChildren = canRenderAllChildren && childNode->mCanRenderNodeOrChildren;
						}
						else
						{
							canRenderAllChildren = false;
						}
					}
				}
			}
		}

		node->mCanRenderNodeOrChildren = node->isMeshUpToDate() | canRenderAllChildren;

		if (canRenderAllChildren)
		{
			// If we can render all the children then don't render ourself.
			node->setRenderThisNode(false);
		}
		else
		{
			// As we can't render all children then we must render no children.
			for (int iz = 0; iz < 2; iz++)
			{
				for (int iy = 0; iy < 2; iy++)
				{
					for (int ix = 0; ix < 2; ix++)
					{
						OctreeNode<VoxelType>* childNode = node->getChildNode(ix, iy, iz);
						if (childNode)
						{
							childNode->setRenderThisNode(false);
						}
					}
				}
			}

			// So we render ourself if we can
			node->setRenderThisNode(node->isMeshUpToDate());
		}
	}

	template <typename VoxelType>
	template<typename VisitorType>
	void Octree<VoxelType>::visitNode(OctreeNode<VoxelType>* node, VisitorType& visitor)
	{
		bool processChildren = visitor.preChildren(node);

		if(processChildren)
		{
			for(int iz = 0; iz < 2; iz++)
			{
				for(int iy = 0; iy < 2; iy++)
				{
					for(int ix = 0; ix < 2; ix++)
					{
						OctreeNode<VoxelType>* childNode = node->getChildNode(ix, iy, iz);
						if (childNode)
						{
							visitNode(childNode, visitor);
						}
					}
				}
			}
		}

		visitor.postChildren(node);
	}
}