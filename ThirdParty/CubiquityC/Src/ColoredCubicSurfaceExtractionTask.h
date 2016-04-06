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

#ifndef CUBIQUITY_COLOUREDCUBICSURFACEEXTRACTIONTASK_H_
#define CUBIQUITY_COLOUREDCUBICSURFACEEXTRACTIONTASK_H_

#include "Clock.h"
#include "Color.h"
#include "CubiquityForwardDeclarations.h"
#include "Region.h"
#include "Task.h"
#include "Vector.h"
#include "VoxelTraits.h"

#include "PolyVox/PagedVolume.h"

namespace Cubiquity
{
	class ColoredCubicSurfaceExtractionTask : public Task
	{
	public:
		ColoredCubicSurfaceExtractionTask(OctreeNode< Color >* octreeNode, ::PolyVox::PagedVolume<Color>* polyVoxVolume);
		~ColoredCubicSurfaceExtractionTask();

		void process(void);

	public:
		OctreeNode< Color >* mOctreeNode;
		::PolyVox::PagedVolume<Color>* mPolyVoxVolume;
		ColoredCubesMesh* mPolyVoxMesh;
		Timestamp mProcessingStartedTimestamp;

		// Whether the task owns the mesh, or whether it has been passed to
		// the OctreeNode. Should probably switch this to use a smart pointer.
		bool mOwnMesh;
	};

	template< typename SrcPolyVoxVolumeType, typename DstPolyVoxVolumeType>
	void rescaleCubicVolume(SrcPolyVoxVolumeType* pVolSrc, const Region& regSrc, DstPolyVoxVolumeType* pVolDst, const Region& regDst)
	{
		POLYVOX_ASSERT(regSrc.getWidthInVoxels() == regDst.getWidthInVoxels() * 2, "Wrong size!");
		POLYVOX_ASSERT(regSrc.getHeightInVoxels() == regDst.getHeightInVoxels() * 2, "Wrong size!");
		POLYVOX_ASSERT(regSrc.getDepthInVoxels() == regDst.getDepthInVoxels() * 2, "Wrong size!");

		typename SrcPolyVoxVolumeType::Sampler srcSampler(pVolSrc);
		typename DstPolyVoxVolumeType::Sampler dstSampler(pVolDst);

		// First of all we iterate over all destination voxels and compute their color as the
		// average of the colors of the eight corresponding voxels in the higher resolution version.
		for(int32_t z = 0; z < regDst.getDepthInVoxels(); z++)
		{
			for(int32_t y = 0; y < regDst.getHeightInVoxels(); y++)
			{
				for(int32_t x = 0; x < regDst.getWidthInVoxels(); x++)
				{
					Vector3I srcPos = regSrc.getLowerCorner() + (Vector3I(x, y, z) * 2);
					Vector3I dstPos = regDst.getLowerCorner() + Vector3I(x, y, z);

					uint32_t noOfSolidVoxels = 0;
					uint32_t averageOf8Red = 0;
					uint32_t averageOf8Green = 0;
					uint32_t averageOf8Blue = 0;
					for(int32_t childZ = 0; childZ < 2; childZ++)
					{
						for(int32_t childY = 0; childY < 2; childY++)
						{
							for(int32_t childX = 0; childX < 2; childX++)
							{
								srcSampler.setPosition(srcPos + Vector3I(childX, childY, childZ));

								Color child = srcSampler.getVoxel();

								if(child.getAlpha () > 0)
								{
									noOfSolidVoxels++;
									averageOf8Red += child.getRed();
									averageOf8Green += child.getGreen();
									averageOf8Blue += child.getBlue();
								}
							}
						}
					}

					// We only make a voxel solid if the eight corresponding voxels are also all solid. This
					// means that higher LOD meshes actually shrink away which ensures cracks aren't visible.
					if(noOfSolidVoxels > 7)
					{
						Color color;
						color.setColor(averageOf8Red / noOfSolidVoxels, averageOf8Green / noOfSolidVoxels, averageOf8Blue / noOfSolidVoxels, 255);
						pVolDst->setVoxel(dstPos, color);
					}
					else
					{
						Color color;
						color.setColor(0,0,0,0);
						pVolDst->setVoxel(dstPos, color);
					}
				}
			}
		}

		// At this point the results are usable, but we have a problem with thin structures disappearing.
		// For example, if we have a solid blue sphere with a one voxel thick layer of red voxels on it,
		// then we don't care that the shape changes then the red voxels are lost but we do care that the
		// color changes, as this is very noticable. Our solution is o process again only those voxels
		// which lie on a material-air boundary, and to recompute their color using a larger naighbourhood
		// while also accounting for how visible the child voxels are.
		for(int32_t z = 0; z < regDst.getDepthInVoxels(); z++)
		{
			for(int32_t y = 0; y < regDst.getHeightInVoxels(); y++)
			{
				for(int32_t x = 0; x < regDst.getWidthInVoxels(); x++)
				{
					Vector3I dstPos = regDst.getLowerCorner() + Vector3I(x, y, z);

					dstSampler.setPosition(dstPos);

					//Skip empty voxels
					if(dstSampler.getVoxel().getAlpha() > 0)
					{
						//Only process voxels on a material-air boundary.
						if((dstSampler.peekVoxel0px0py1nz().getAlpha() == 0) ||
						   (dstSampler.peekVoxel0px0py1pz().getAlpha() == 0) || 
						   (dstSampler.peekVoxel0px1ny0pz().getAlpha() == 0) ||
						   (dstSampler.peekVoxel0px1py0pz().getAlpha() == 0) ||
						   (dstSampler.peekVoxel1nx0py0pz().getAlpha() == 0) || 
						   (dstSampler.peekVoxel1px0py0pz().getAlpha() == 0))
						{
							Vector3I srcPos = regSrc.getLowerCorner() + (Vector3I(x, y, z) * 2);

							uint32_t totalRed = 0;
							uint32_t totalGreen = 0;
							uint32_t totalBlue = 0;
							uint32_t totalExposedFaces = 0;

							// Look ate the 64 (4x4x4) children
							for(int32_t childZ = -1; childZ < 3; childZ++)
							{
								for(int32_t childY = -1; childY < 3; childY++)
								{
									for(int32_t childX = -1; childX < 3; childX++)
									{
										srcSampler.setPosition(srcPos + Vector3I(childX, childY, childZ));

										Color child = srcSampler.getVoxel();

										if(child.getAlpha () > 0)
										{
											// For each small voxel, count the exposed faces and use this
											// to determine the importance of the color contribution.
											uint32_t exposedFaces = 0;
											if(srcSampler.peekVoxel0px0py1nz().getAlpha() == 0) exposedFaces++;
											if(srcSampler.peekVoxel0px0py1pz().getAlpha() == 0) exposedFaces++;
											if(srcSampler.peekVoxel0px1ny0pz().getAlpha() == 0) exposedFaces++;
											if(srcSampler.peekVoxel0px1py0pz().getAlpha() == 0) exposedFaces++;
											if(srcSampler.peekVoxel1nx0py0pz().getAlpha() == 0) exposedFaces++;
											if(srcSampler.peekVoxel1px0py0pz().getAlpha() == 0) exposedFaces++;

											totalRed += child.getRed() * exposedFaces;
											totalGreen += child.getGreen() * exposedFaces;
											totalBlue += child.getBlue() * exposedFaces;

											totalExposedFaces += exposedFaces;
										}							
									}
								}
							}

							// Avoid divide by zero if there were no exposed faces.
							if(totalExposedFaces == 0) totalExposedFaces++;

							Color color;
							color.setColor(totalRed / totalExposedFaces, totalGreen / totalExposedFaces, totalBlue / totalExposedFaces, 255);
							pVolDst->setVoxel(dstPos, color);
						}
					}
				}
			}
		}
	}
}

#endif //CUBIQUITY_COLOUREDCUBICSURFACEEXTRACTIONTASK_H_
