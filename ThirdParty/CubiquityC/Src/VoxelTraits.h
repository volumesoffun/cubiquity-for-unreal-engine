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

#ifndef CUBIQUITY_VOXELTRAITS_H_
#define CUBIQUITY_VOXELTRAITS_H_

#include "Color.h"
#include "MaterialSet.h"

namespace Cubiquity
{
	// We use traits to decide (for example) which vertex type should correspond to a given voxel type, 
	// or which surface extractor should be used for a given voxel type. Maybe it is useful to consider
	// putting some of this (the VoxelType to VertexType maybe?) into PolyVox.
	template<typename Type>
	class VoxelTraits;

	template<>
	class VoxelTraits<Color>
	{
	public:
		typedef ColoredCubesVertex VertexType;
		typedef ColoredCubicSurfaceExtractionTask SurfaceExtractionTaskType;
		static const bool IsColor = true;
		static const bool IsMaterialSet = false;
	};

	template<>
	class VoxelTraits<MaterialSet>
	{
	public:
		typedef TerrainVertex VertexType;
		typedef SmoothSurfaceExtractionTask SurfaceExtractionTaskType;
		static const bool IsColor = false;
		static const bool IsMaterialSet = true;
	};
}

#endif //CUBIQUITY_VOXELTRAITS_H_