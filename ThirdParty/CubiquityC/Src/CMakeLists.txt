################################################################################
# The MIT License (MIT)
#
# Copyright (c) 2016 David Williams and Matthew Williams
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
################################################################################

project(CubiquityC)

################################################################################
# Build SQLite - Leading underscore to avoid conflict with system version?
################################################################################
add_library(_sqlite3 STATIC Dependancies/SQLite/sqlite3.c)
set_target_properties(_sqlite3 PROPERTIES COMPILE_FLAGS "-fPIC")
add_definitions( -DSQLITE_OMIT_LOAD_EXTENSION ) # Avoids needing 'dl' on Linux

# Organise the Visual Studio folders.
SET_PROPERTY(TARGET _sqlite3 PROPERTY FOLDER "Core/Dependancies")

################################################################################
# Build PolyVox - Leading underscore to avoid conflict with system version?
################################################################################
set(POLYVOX_SRCS
	# PolyVox code
	#Dependancies/polyvox/library/PolyVox/source/AStarPathfinder.cpp
	#Dependancies/polyvox/library/PolyVox/source/Region.cpp
	
	#Dependancies/polyvox/library/PolyVox/source/Impl/ErrorHandling.cpp
	#Dependancies/polyvox/library/PolyVox/source/Impl/Logging.cpp
	#Dependancies/polyvox/library/PolyVox/source/Impl/MarchingCubesTables.cpp
	#Dependancies/polyvox/library/PolyVox/source/Impl/RandomUnitVectors.cpp
	#Dependancies/polyvox/library/PolyVox/source/Impl/RandomVectors.cpp
	#Dependancies/polyvox/library/PolyVox/source/Impl/Timer.cpp
	#Dependancies/polyvox/library/PolyVox/source/Impl/Utility.cpp
)

#add_library(_PolyVox STATIC ${POLYVOX_SRCS})

# Organise the Visual Studio folders.
#SET_PROPERTY(TARGET _PolyVox PROPERTY FOLDER "Core/Dependancies")

################################################################################
# Build Cubiquity
################################################################################
include_directories(
	Dependancies/
	Dependancies/polyvox/include/
)

set(SRCS
	# Main Cubiquity code
	BackgroundTaskProcessor.cpp
	Brush.cpp
	Clock.cpp
	Color.cpp
	ColoredCubesVolume.cpp
	ColoredCubicSurfaceExtractionTask.cpp
	Logging.cpp
	MainThreadTaskProcessor.cpp
	MaterialSet.cpp
	Raycasting.cpp
	SmoothSurfaceExtractionTask.cpp
	TerrainVolume.cpp
	TerrainVolumeEditor.cpp
	Task.cpp
	TerrainVolumeGenerator.cpp
	VoxelDatabase.cpp
	
	# Cubiquity 'C' interface
	CubiquityC.cpp
)

set(HEADERS
	# Main Cubiquity code
	BackgroundTaskProcessor.h
	BitField.h
	Brush.h
	Clock.h
	Color.h
	ColoredCubesVolume.h
	ColoredCubicSurfaceExtractionTask.h
	ConcurrentQueue.h
	Cubiquity.h
	CubiquityForwardDeclarations.h
	Exceptions.h
	Logging.h
	MainThreadTaskProcessor.h
	MaterialSet.h
	Octree.h
	Octree.inl
	OctreeNode.h
	OctreeNode.inl
	Raycasting.h
	Region.h
	SmoothSurfaceExtractionTask.h
	SQLiteUtils.h
	Task.h
	TaskProcessor.h
	TerrainVolume.h
	TerrainVolumeEditor.h
	TerrainVolumeGenerator.h
	Vector.h
	Volume.h
	Volume.inl
	VoxelDatabase.h
	VoxelDatabase.inl
	VoxelTraits.h
	WritePermissions.h	
	
	# Cubiquity 'C' interface
	CubiquityC.h
)

add_library(CubiquityC SHARED ${SRCS} ${HEADERS})

# Organise the Visual Studio folders.
SET_PROPERTY(TARGET CubiquityC PROPERTY FOLDER "Core")

# Shared library must export symbols
add_definitions( -DCUBIQUITYC_EXPORTS )

# Set LINK_FLAGS
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	set_target_properties(CubiquityC PROPERTIES LINK_FLAGS "-static-libstdc++ -static-libgcc")
endif()

# Set target_link_libraries
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	target_link_libraries(CubiquityC _sqlite3)
else()
	target_link_libraries(CubiquityC _sqlite3 pthread)
endif()

# The code below was copied from the old separate 'C' wrapper. I'm not sure we need it but it's here for safe keeping.
#if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	#
#elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
#	target_link_libraries(CubiquityC /home/matt/osxcross/build/libcxx-3.4/build/lib/libc++.a /home/matt/osxcross/target/SDK/MacOSX10.8.sdk/usr/lib/libc++abi.dylib)
#elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
	#
#endif()

install(TARGETS CubiquityC DESTINATION bin)
install(FILES CubiquityC.h DESTINATION include)
