// Copyright 2014 Volumes of Fun. All Rights Reserved.

#ifndef CUBIQUITY_HPP_
#define CUBIQUITY_HPP_

#include "CubiquityC.h"

#include <stdexcept>
#include <sstream>

namespace
{
	void validate(int32_t returnCode)
	{
		if (returnCode != CU_OK)
		{
			std::stringstream ss;
			ss << cuGetErrorCodeAsString(returnCode) << " : " << cuGetLastErrorMessage();
			std::string errorString(ss.str());
			throw std::runtime_error(errorString); //TODO throw correct exception type where possible
			//exit(EXIT_FAILURE);
		}
	}
}

#include <tuple>
#include <array>

namespace Cubiquity
{
	inline uint32_t currentTime()
	{
		uint32_t result;
		::validate(cuGetCurrentTime(&result));
		return result;
	}

	enum class VolumeType : uint8_t
	{
		ColoredCubes = CU_COLORED_CUBES,
		Terrain = CU_TERRAIN,
	};

	enum class WritePermissions : uint8_t
	{
		ReadOnly = CU_READONLY,
		ReadWrite = CU_READWRITE,
	};

	//A generic 3-vector for various types
	template<typename T>
	struct Vector
	{
		T x;
		T y;
		T z;
	};

	//`Color` is both the type used to store inside the ColoredCubesVolume as well as the type stored on each generated vertex
	class Color
	{
	public:
		Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) : m_color(cuMakeColor(red, green, blue, alpha)) {}
		Color(uint32_t color) : m_color(*(CuColor*)(&color)) {}  //Horrible horrible cast nonsense
		Color(CuColor color) : m_color(color) {}

		uint8_t red() const { return cuGetRed(m_color); }

		uint8_t green() const { return cuGetGreen(m_color); }

		uint8_t blue() const { return cuGetBlue(m_color); }

		uint8_t alpha() const { return cuGetAlpha(m_color); }

		std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> allComponents() const
		{
			std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> result;
			cuGetAllComponents(m_color, &std::get<0>(result), &std::get<1>(result), &std::get<2>(result), &std::get<3>(result));
			return result;
		}

		CuColor colorStruct() const { return m_color; }

	private:
		const CuColor m_color;
	};

	//`MaterialSet` is both the type used to store inside the TerrainVolume as well as the type stored on each generated vertex
	class MaterialSet
	{
	public:
		MaterialSet() = default;
		MaterialSet(uint64_t materialSet) : m_materialSet(*(CuMaterialSet*)(&materialSet)) {}  //Horrible horrible cast nonsense
		MaterialSet(CuMaterialSet materialSet) : m_materialSet(materialSet) {}

		std::array<uint8_t, 8> materials() const
		{
			std::array<uint8_t, 8> byteArray;
			for (uint8_t i = 0; i < 8; i++)
			{
				byteArray[8 - i] = getMaterial(i);
			}
			return byteArray;
		}

		uint8_t getMaterial(uint8_t index) const
		{
			return m_materialSet.data >> (index * 8);
		}

		void setMaterial(uint8_t index, uint8_t value)
		{
			const uint64_t bitIndex = index * 8;
			const uint64_t value64 = value;
			uint64_t mask = (((uint64_t)1 << 8) - 1) << bitIndex;
			m_materialSet.data = (m_materialSet.data & ~mask) | ((value64 << bitIndex) & mask);
		}

		CuMaterialSet materialSetStruct() const { return m_materialSet; }

	private:
		CuMaterialSet m_materialSet;
	};

	class ColoredCubesVertex
	{
	public:
		ColoredCubesVertex() = delete;
		Vector<uint8_t> encodedPos() const { return{ m_vertex.encodedPosX, m_vertex.encodedPosY, m_vertex.encodedPosZ }; }

		Vector<float> position() const { return{ m_vertex.encodedPosX - 0.5, m_vertex.encodedPosY - 0.5, m_vertex.encodedPosZ - 0.5 }; }

		Color color() const { return{ m_vertex.data }; };

	private:
		const CuColoredCubesVertex m_vertex;
	};

	class TerrainVertex
	{
	public:
		TerrainVertex() = delete;
		Vector<uint16_t> encodedPos() const { return{ m_vertex.encodedPosX, m_vertex.encodedPosY, m_vertex.encodedPosZ }; }
		uint16_t encodedNormal() const { return{ m_vertex.encodedNormal }; }

		Vector<float> position() const { return{ (1.0 / 256.0) * m_vertex.encodedPosX, (1.0 / 256.0) * m_vertex.encodedPosY, (1.0 / 256.0) * m_vertex.encodedPosZ }; }

		Vector<float> normal() const
		{
			const uint16_t encodedNormal = m_vertex.encodedNormal;
			const uint16_t ux = (encodedNormal >> 8) & 0xFF;
			const uint16_t uy = (encodedNormal)& 0xFF;

			// Convert to floats in the range [-1.0f, +1.0f].
			const float ex = ux / 127.5f - 1.0f;
			const float ey = uy / 127.5f - 1.0f;

			// Reconstruct the original vector. This is a C++ implementation
			// of Listing 2 of http://jcgt.org/published/0003/02/01/
			float vx = ex;
			float vy = ey;
			const float vz = 1.0f - std::abs(ex) - std::abs(ey);

			if (vz < 0.0f)
			{
				const float refX = ((1.0f - std::abs(vy)) * (vx >= 0.0f ? +1.0f : -1.0f));
				const float refY = ((1.0f - std::abs(vx)) * (vy >= 0.0f ? +1.0f : -1.0f));
				vx = refX;
				vy = refY;
			}
			return {vx, vy, vz};
		}

		std::array<uint8_t, 8> materials() const
		{
			return{ m_vertex.material0, m_vertex.material1, m_vertex.material2, m_vertex.material3, m_vertex.material4, m_vertex.material5, m_vertex.material6, m_vertex.material7 };
		}

	private:
		const CuTerrainVertex m_vertex;
	};

	class OctreeNode
	{
	public:
		OctreeNode() = default;

		OctreeNode(uint32_t nodeHandle) : m_nodeHandle(nodeHandle)
		{
			cuGetOctreeNode(nodeHandle, &m_octreeNode);
		}

		bool hasChildNode(const Vector<uint32_t>& child) const
		{
			return m_octreeNode.childHandles[child.x][child.y][child.z] != 0xFFFFFFFF;
		}

		OctreeNode childNode(const Vector<uint32_t>& child) const
		{
			return OctreeNode(m_octreeNode.childHandles[child.x][child.y][child.z]);
		}

		bool hasMesh() const
		{
			return m_octreeNode.hasMesh != 0;
		}

		uint32_t meshLastChanged() const
		{
			return m_octreeNode.meshLastChanged;
		}

		uint32_t structureLastChanged() const
		{
			return m_octreeNode.structureLastChanged;
		}

		uint32_t propertiesLastChanged() const
		{
			return m_octreeNode.propertiesLastChanged;
		}

		uint32_t nodeOrChildrenLastChanged() const
		{
			return m_octreeNode.nodeOrChildrenLastChanged;
		}

		uint8_t height() const
		{
			return m_octreeNode.height;
		}

		Vector<int32_t> position() const
		{
			return { m_octreeNode.posX, m_octreeNode.posY, m_octreeNode.posZ };
		}

		bool renderThisNode() const
		{
			return m_octreeNode.renderThisNode != 0;
		}

		uint32_t handle() const
		{
			return m_nodeHandle;
		}

		template <typename VertexType>
		void getMesh(uint16_t* noOfVertices, VertexType** vertices, uint32_t* noOfIndices, uint16_t** indices) const
		{
			::validate(cuGetMesh(m_nodeHandle, noOfVertices, (void**)(vertices), noOfIndices, indices));
		}

	private:
		CuOctreeNode m_octreeNode;
		uint32_t m_nodeHandle = 0xFFFFFFFF;
	};

	class Volume
	{
	public:

		VolumeType volumeType() const
		{
			uint32_t result;
			::validate(cuGetVolumeType(m_volumeHandle, &result));
			return VolumeType(result);
		}

		bool update(const Vector<float>& eyePos, float lodThreshold)
		{
			uint32_t isUpToDate;
			::validate(cuUpdateVolume(m_volumeHandle, eyePos.x, eyePos.y, eyePos.z, lodThreshold, &isUpToDate));
			return isUpToDate != 0;
		}

		std::pair<Vector<int32_t>, Vector<int32_t>> enclosingRegion() const
		{
			Vector<int32_t> lower;
			Vector<int32_t> upper;
			::validate(cuGetEnclosingRegion(m_volumeHandle, &lower.x, &lower.y, &lower.z, &upper.x, &upper.y, &upper.z));
			return std::make_pair(lower, upper);
		}

		void acceptOverrideChunks()
		{
			::validate(cuAcceptOverrideChunks(m_volumeHandle));
		}

		void discardOverrideChunks()
		{
			::validate(cuDiscardOverrideChunks(m_volumeHandle));
		}

		bool hasRootOctreeNode() const
		{
			bool result;
			::validate(cuHasRootOctreeNode(m_volumeHandle, (uint32_t*)(&result)));
			return result;
		}

		OctreeNode rootOctreeNode() const
		{
			uint32_t result;
			::validate(cuGetRootOctreeNode(m_volumeHandle, &result));
			return {result};
		}

		void setLodRange(int32_t minimumLOD, int32_t maximumLOD)
		{
			::validate(cuSetLodRange(m_volumeHandle, minimumLOD, maximumLOD));
		}

		virtual ~Volume()
		{
			::validate(cuDeleteVolume(m_volumeHandle));
		}

	protected:
		uint32_t m_volumeHandle;
	};

	class TerrainVolume : public Volume
	{
	public:
		TerrainVolume(const Vector<int32_t>& lower, const Vector<int32_t>& upper, const std::string& pathToNewVoxelDatabase, uint32_t baseNodeSize)
		{
			::validate(cuNewEmptyTerrainVolume(lower.x, lower.y, lower.z, upper.x, upper.y,  upper.z, pathToNewVoxelDatabase.c_str(), baseNodeSize, &m_volumeHandle));
		}

		TerrainVolume(const std::string& pathToExistingVoxelDatabase, WritePermissions writePermissions, uint32_t baseNodeSize)
		{
			::validate(cuNewTerrainVolumeFromVDB(pathToExistingVoxelDatabase.c_str(), static_cast<uint32_t>(writePermissions), baseNodeSize, &m_volumeHandle));
		}

		MaterialSet getVoxel(const Vector<int32_t>& position) const
		{
			CuMaterialSet result;
			::validate(cuGetVoxel(m_volumeHandle, position.x, position.y, position.z, &result));
			return {result};
		}

		void setVoxel(const Vector<int32_t>& position, MaterialSet value)
		{
			CuMaterialSet newValue = value.materialSetStruct();
			::validate(cuSetVoxel(m_volumeHandle, position.x, position.y, position.z, std::move(&value)));
		}

		Vector<float> pickSurface(const Vector<float>& rayStart, const Vector<float>& rayDir, bool* success) const
		{
			Vector<float> result;
			::validate(cuPickTerrainSurface(m_volumeHandle, rayStart.x, rayStart.y, rayStart.z, rayDir.x, rayDir.y, rayDir.z, &result.x, &result.y, &result.z, (uint32_t*)success));
			return result;
		}

		void sculpt(const Vector<float>& brushPosition, float brushInnerRadius, float brushOuterRadius, float opacity)
		{
			::validate(cuSculptTerrainVolume(m_volumeHandle, brushPosition.x, brushPosition.y, brushPosition.z, brushInnerRadius, brushOuterRadius, opacity));
		}

		void blur(const Vector<float>& brushPosition, float brushInnerRadius, float brushOuterRadius, float opacity)
		{
			::validate(cuBlurTerrainVolume(m_volumeHandle, brushPosition.x, brushPosition.y, brushPosition.z, brushInnerRadius, brushOuterRadius, opacity));
		}

		void blur(const Vector<int32_t>& lowerCorner, const Vector<int32_t>& upperCorner)
		{
			::validate(cuBlurTerrainVolumeRegion(m_volumeHandle, lowerCorner.x, lowerCorner.y, lowerCorner.z, upperCorner.x, upperCorner.y, upperCorner.z));
		}

		void paint(const Vector<float>& brushPosition, float brushInnerRadius, float brushOuterRadius, float opacity, uint32_t materialIndex)
		{
			::validate(cuPaintTerrainVolume(m_volumeHandle, brushPosition.x, brushPosition.y, brushPosition.z, brushInnerRadius, brushOuterRadius, opacity, materialIndex));
		}

		void generateFloor(int32_t lowerLayerHeight, uint32_t lowerLayerMaterialIndex, int32_t upperLayerHeight, uint32_t upperLayerMaterialIndex)
		{
			::validate(cuGenerateFloor(m_volumeHandle, lowerLayerHeight, lowerLayerMaterialIndex, upperLayerHeight, upperLayerMaterialIndex));
		}
	}; 
	
	class ColoredCubesVolume : public Volume
	{
	public:
		ColoredCubesVolume(const Vector<int32_t>& lower, const Vector<int32_t>& upper, const std::string& pathToNewVoxelDatabase, uint32_t baseNodeSize)
		{
			::validate(cuNewEmptyColoredCubesVolume(lower.x, lower.y, lower.z, upper.x, upper.y, upper.z, pathToNewVoxelDatabase.c_str(), baseNodeSize, &m_volumeHandle));
		}

		ColoredCubesVolume(const std::string& pathToExistingVoxelDatabase, WritePermissions writePermissions, uint32_t baseNodeSize)
		{
			::validate(cuNewColoredCubesVolumeFromVDB(pathToExistingVoxelDatabase.c_str(), static_cast<uint32_t>(writePermissions), baseNodeSize, &m_volumeHandle));
		}

		Color getVoxel(const Vector<int32_t>& position) const
		{
			CuColor result;
			::validate(cuGetVoxel(m_volumeHandle, position.x, position.y, position.z, &result));
			return {result};
		}

		void setVoxel(const Vector<int32_t>& position, const Color& value)
		{
			CuColor newValue = value.colorStruct();
			::validate(cuSetVoxel(m_volumeHandle, position.x, position.y, position.z, std::move(&newValue)));
		}

		Vector<int32_t> pickFirstSolidVoxel(const Vector<float>& rayStart, const Vector<float>& rayDir, bool* success) const
		{
			Vector<int32_t> result;
			::validate(cuPickFirstSolidVoxel(m_volumeHandle, rayStart.x, rayStart.y, rayStart.z, rayDir.x, rayDir.y, rayDir.z, &result.x, &result.y, &result.z, (uint32_t*)success));
			return result;
		}

		Vector<int32_t> pickLastEmptyVoxel(const Vector<float>& rayStart, const Vector<float>& rayDir, bool* success) const
		{
			Vector<int32_t> result;
			::validate(cuPickLastEmptyVoxel(m_volumeHandle, rayStart.x, rayStart.y, rayStart.z, rayDir.x, rayDir.y, rayDir.z, &result.x, &result.y, &result.z, (uint32_t*)success));
			return result;
		}
	};
}

#endif //CUBIQUITY_HPP_
