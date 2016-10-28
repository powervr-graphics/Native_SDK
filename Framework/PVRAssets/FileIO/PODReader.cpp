/*!*********************************************************************************************************************
\file         PVRAssets\FileIO\PODReader.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementation of methods of the PODReader class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRAssets/FileIO/PODReader.h"
#include "PVRAssets/FileIO/PODDefines.h"
#include "PVRAssets/Model.h"
#include "PVRCore/Log.h"
//#include "PVRAssets/assets::Model.h"
#include "PVRAssets/Helper.h"
//#include "PVRAssets/assets::Model/Light.h"
//#include "PVRAssets/assets::Model/assets::Mesh.h"
#include "PVRCore/Stream.h"
#include <cstdio>
#include <algorithm>
using std::vector;

namespace { // LOCAL FUNCTIONS
using namespace pvr;
using namespace assets;
using namespace pvr::types;
template <typename T>
bool readBytes(Stream& stream, T& data)
{
	size_t dataRead;
	return stream.read(sizeof(T), 1, &data, dataRead);
}

template <typename T>
bool readByteArray(Stream& stream, T* data, uint32 count)
{
	for (uint32 i = 0; i < count; ++i)
	{
		bool result = readBytes(stream, data[i]);
		if (!result) { return result; }
	}
	return true;
}

template <typename T>
bool read4Bytes(Stream& stream, T& data)
{
	//PVR_STATIC_ASSERT(read4BytesSizeAssert, sizeof(T) == 4)
	unsigned char ub[4];
	size_t dataRead;
	if (stream.read(4, 1, &ub, dataRead))
	{
		unsigned int* p = reinterpret_cast<unsigned int*>(&data);
		*p = static_cast<unsigned int>((ub[3] << 24) | (ub[2] << 16) | (ub[1] << 8) | ub[0]);
		return true;
	}
	return false;
}

template <typename T>
bool read4ByteArray(Stream& stream, T* data, uint32 count)
{
	//PVR_STATIC_ASSERT(read4ByteArraySizeAssert, sizeof(T) == 4)
	for (uint32 i = 0; i < count; ++i)
	{
		if (!read4Bytes(stream, data[i])) { return false; }
	}
	return true;
}

template <typename T>
bool read2Bytes(Stream& stream, T& data)
{
	//PVR_STATIC_ASSERT(read2BytesSizeAssert, sizeof(T) == 2)
	unsigned char ub[2];
	size_t dataRead;
	if (stream.read(2, 1, &ub, dataRead))
	{
		unsigned short* p = reinterpret_cast<unsigned short*>(&data);
		*p = static_cast<unsigned short>((ub[1] << 8) | ub[0]);
		return true;
	}
	return false;
}

template <typename T>
bool read2ByteArray(Stream& stream, T* data, uint32 count)
{
	//PVR_STATIC_ASSERT(read2ByteArraySizeAssert, sizeof(T) == 2)
	for (uint32 i = 0; i < count; ++i)
	{
		bool result = read2Bytes(stream, data[i]);
		if (!result) { return result; }
	}
	return true;
}

template <typename T, typename vector_T>
bool readByteArrayIntoVector(Stream& stream, std::vector<vector_T>& data, uint32 count)
{
	assertion(sizeof(vector_T) <= sizeof(T));
	data.resize(count * sizeof(T) / sizeof(vector_T));
	return readByteArray<T>(stream, reinterpret_cast<T*>(data.data()), count);
}
template <typename T, typename vector_T>
bool read2ByteArrayIntoVector(Stream& stream, std::vector<vector_T>& data, uint32 count)
{
	assertion(sizeof(vector_T) <= sizeof(T));
	data.resize(count * sizeof(T) / sizeof(vector_T));
	return read2ByteArray<T>(stream, reinterpret_cast<T*>(data.data()), count);
}
template <typename T, typename vector_T>
bool read4ByteArrayIntoVector(Stream& stream, std::vector<vector_T>& data, uint32 count)
{
	assertion(sizeof(vector_T) <= sizeof(T));
	data.resize(count * sizeof(T) / sizeof(vector_T));
	return read4ByteArray<T>(stream, reinterpret_cast<T*>(data.data()), count);
}
bool readByteArrayIntoString1(Stream& stream, std::string& data, uint32 count)
{
	std::vector<char8> data1; data1.resize(count);
	bool res = readByteArray(stream, reinterpret_cast<char8*>(data1.data()), count);
	data.assign(data1.data());
	return res;
}

bool readByteArrayIntoStringHash(Stream& stream, StringHash& data, uint32 count)
{
	std::vector<char8> data1; data1.resize(count);
	bool res = readByteArray(stream, reinterpret_cast<char8*>(data1.data()), count);
	data.assign(data1.data());
	return res;
}

bool readTag(Stream& stream, uint32& identifier, uint32& dataLength)
{
	if (!read4Bytes(stream, identifier)) { return false; }
	if (!read4Bytes(stream, dataLength)) { return false; }
	return true;
}


bool readVertexIndexData(Stream& stream, assets::Mesh& mesh)
{
	bool result;
	uint32 identifier, dataLength, size(0);
	std::vector<byte> data;
	IndexType type(IndexType::IndexType16Bit);
	while ((result = readTag(stream, identifier, dataLength)))
	{
		if (identifier == (pod::e_meshVertexIndexList | pod::c_endTagMask))
		{
			mesh.addFaces(data.data(), size, type);
			return true;
		}
		switch (identifier)
		{
		case pod::e_blockDataType:
		{
			uint32 tmp;
			if (!read4Bytes(stream, tmp)) { return false; }
			switch (static_cast<DataType >(tmp))
			{
			case DataType::UInt32:
				type = IndexType::IndexType32Bit;
				break;
			case DataType::UInt16:
				type = IndexType::IndexType16Bit;
				break;
			default:
			{
				assertion(false); // Unrecognised data type
			}
			}
			continue;
		}
		case pod::e_blockData:
			switch (type)
			{
			case IndexType::IndexType16Bit:
			{
				result = read2ByteArrayIntoVector<uint16>(stream, data, dataLength / 2);
				break;
			}
			case IndexType::IndexType32Bit:
			{
				result = read4ByteArrayIntoVector<uint32>(stream, data, dataLength / 4);
				break;
			}
			}
			if (!result) { return result; }
			size = dataLength;
			break;
		default:
		{
			// Unhandled data, skip it
			result = stream.seek(dataLength, Stream::SeekOriginFromCurrent);
		}
		}
		if (!result) { break; }
	}
	return result;
}

bool readVertexData(Stream& stream, assets::Mesh& mesh, const char8* const semanticName, uint32 blockIdentifier, int32 dataIndex, bool& existed)
{
	existed = false;
	uint32 identifier, dataLength, numComponents(0), stride(0), offset(0);
	DataType type(DataType::None);
	while (readTag(stream, identifier, dataLength))
	{
		if (identifier == (blockIdentifier | pod::c_endTagMask))
		{
			if (numComponents != 0) // Is there a Vertex Attribute to add?
			{
				existed = true;
				mesh.setStride(dataIndex, stride);
				return (mesh.addVertexAttribute(semanticName, type, numComponents, offset, dataIndex) != -1);
			}
			else
			{
				existed = false;
				return true;
			}
		}
		switch (identifier)
		{
		case pod::e_blockDataType:
		{
			uint32 tmp;
			if (!read4Bytes(stream, tmp)) { return false; }
			type = static_cast<DataType>(tmp);
			continue;
		}
		case pod::e_blockNumComponents:
			if (!read4Bytes(stream, numComponents)) { return false; }
			break;
		case pod::e_blockStride:
			if (!read4Bytes(stream, stride)) { return false; }
			break;
		case pod::e_blockData:
			if (dataIndex == -1)   // This POD file isn't using interleaved data so this data block must be valid vertex data
			{
				std::vector<byte> data;
				switch (dataTypeSize(type))
				{
				case 1:
				{
					if (!readByteArrayIntoVector<byte>(stream, data, dataLength)) { return false; }
					break;
				}
				case 2:
				{
					if (!read2ByteArrayIntoVector<uint16>(stream, data, dataLength / 2)) { return false; }
					break;
				}
				case 4:
				{
					if (!read2ByteArrayIntoVector<uint32>(stream, data, dataLength / 4)) { return false; }
					break;
				}
				default:
				{
					assertion(false);
					Log(Log.Error, "Unknown error reading POD file - data type width >4");
					return false;
				}
				}
				dataIndex = mesh.addData(data.data(), dataLength, stride);

			}
			else
			{
				if (!read4Bytes(stream, offset)) { return false; }
			}
			break;
		default:
		{
			// Unhandled data, skip it
			if (!stream.seek(dataLength, Stream::SeekOriginFromCurrent)) { return false; }
		}
		}
	}
	return true;
}

bool readMaterialBlock(Stream& stream, assets::Model::Material& material)
{
	bool result;
	uint32 identifier, dataLength;
	assets::Model::Material::InternalData& materialInternalData = material.getInternalData();
	while ((result = readTag(stream, identifier, dataLength)))
	{
		switch (identifier)
		{
		case pod::e_sceneMaterial | pod::c_endTagMask:
			return true;
		case pod::e_materialName | pod::c_startTagMask:
		{
			if (!readByteArrayIntoStringHash(stream, materialInternalData.name, dataLength)) { return false; }
			break;
		}
		break;
		case pod::e_materialDiffuseTextureIndex | pod::c_startTagMask:
			result = read4Bytes(stream, materialInternalData.diffuseTextureIndex);
			break;
		case pod::e_materialOpacity | pod::c_startTagMask:
			result = read4Bytes(stream, materialInternalData.opacity);
			break;
		case pod::e_materialAmbientColor | pod::c_startTagMask:
			result = read4ByteArray(stream, &materialInternalData.ambient[0],
			                        sizeof(materialInternalData.ambient) / sizeof(materialInternalData.ambient[0]));
			break;
		case pod::e_materialDiffuseColor | pod::c_startTagMask:
			result = read4ByteArray(stream, &materialInternalData.diffuse[0],
			                        sizeof(materialInternalData.diffuse) / sizeof(materialInternalData.diffuse[0]));
			break;
		case pod::e_materialSpecularColor | pod::c_startTagMask:
			result = read4ByteArray(stream, &materialInternalData.specular[0],
			                        sizeof(materialInternalData.specular) / sizeof(materialInternalData.specular[0]));
			break;
		case pod::e_materialShininess | pod::c_startTagMask:
			result = read4Bytes(stream, materialInternalData.shininess);
			break;
		case pod::e_materialEffectFile | pod::c_startTagMask:
		{
			if (!readByteArrayIntoStringHash(stream, materialInternalData.effectFile, dataLength)) { return false; }
			break;
		}
		break;
		case pod::e_materialEffectName | pod::c_startTagMask:
		{
			if (!readByteArrayIntoStringHash(stream, materialInternalData.effectName, dataLength)) { return false; }
			break;
		}
		break;
		case pod::e_materialAmbientTextureIndex | pod::c_startTagMask:
			result = read4Bytes(stream, materialInternalData.ambientTextureIndex);
			break;
		case pod::e_materialSpecularColorTextureIndex | pod::c_startTagMask:
			result = read4Bytes(stream, materialInternalData.specularColorTextureIndex);
			break;
		case pod::e_materialSpecularLevelTextureIndex | pod::c_startTagMask:
			result = read4Bytes(stream, materialInternalData.specularLevelTextureIndex);
			break;
		case pod::e_materialBumpMapTextureIndex | pod::c_startTagMask:
			result = read4Bytes(stream, materialInternalData.bumpMapTextureIndex);
			break;
		case pod::e_materialEmissiveTextureIndex | pod::c_startTagMask:
			result = read4Bytes(stream, materialInternalData.emissiveTextureIndex);
			break;
		case pod::e_materialGlossinessTextureIndex | pod::c_startTagMask:
			result = read4Bytes(stream, materialInternalData.glossinessTextureIndex);
			break;
		case pod::e_materialOpacityTextureIndex | pod::c_startTagMask:
			result = read4Bytes(stream, materialInternalData.opacityTextureIndex);
			break;
		case pod::e_materialReflectionTextureIndex | pod::c_startTagMask:
			result = read4Bytes(stream, materialInternalData.reflectionTextureIndex);
			break;
		case pod::e_materialRefractionTextureIndex | pod::c_startTagMask:
			result = read4Bytes(stream, materialInternalData.refractionTextureIndex);
			break;
		case pod::e_materialBlendingRGBSrc | pod::c_startTagMask:
		{
			uint32 tmp;
			result = read4Bytes(stream, tmp);
			if (!result) { return result; }
			materialInternalData.blendSrcRGB = static_cast<assets::Model::Material::BlendFunction>(tmp);
			break;
		}
		case pod::e_materialBlendingAlphaSrc | pod::c_startTagMask:
		{
			uint32 tmp;
			result = read4Bytes(stream, tmp);
			if (!result) { return result; }
			materialInternalData.blendSrcA = static_cast<assets::Model::Material::BlendFunction>(tmp);
			break;
		}
		case pod::e_materialBlendingRGBDst | pod::c_startTagMask:
		{
			uint32 tmp;
			result = read4Bytes(stream, tmp);
			if (!result) { return result; }
			materialInternalData.blendDstRGB = static_cast<assets::Model::Material::BlendFunction>(tmp);
			break;
		}
		case pod::e_materialBlendingAlphaDst | pod::c_startTagMask:
		{
			uint32 tmp;
			result = read4Bytes(stream, tmp);
			if (!result) { return result; }
			materialInternalData.blendDstA = static_cast<assets::Model::Material::BlendFunction>(tmp);
			break;
		}
		case pod::e_materialBlendingRGBOperation | pod::c_startTagMask:
		{
			uint32 tmp;
			result = read4Bytes(stream, tmp);
			if (!result) { return result; }
			materialInternalData.blendOpRGB = static_cast<assets::Model::Material::BlendOperation>(tmp);
			break;
		}
		case pod::e_materialBlendingAlphaOperation | pod::c_startTagMask:
		{
			uint32 tmp;
			result = read4Bytes(stream, tmp);
			if (!result) { return result; }
			materialInternalData.blendOpRGB = static_cast<assets::Model::Material::BlendOperation>(tmp);
			break;
		}
		case pod::e_materialBlendingRGBAColor | pod::c_startTagMask:
			result = read4ByteArray(stream, &materialInternalData.blendColor[0],
			                        sizeof(materialInternalData.blendColor) / sizeof(materialInternalData.blendColor[0]));
			break;
		case pod::e_materialBlendingFactorArray | pod::c_startTagMask:
			result = read4ByteArray(stream, &materialInternalData.blendFactor[0],
			                        sizeof(materialInternalData.blendFactor) / sizeof(materialInternalData.blendFactor[0]));
			break;
		case pod::e_materialFlags | pod::c_startTagMask:
			result = read4Bytes(stream, materialInternalData.flags);
			break;
		case pod::e_materialUserData | pod::c_startTagMask:
			result = readByteArrayIntoVector<byte>(stream, materialInternalData.userData, dataLength);
			if (!result) { return result; }
			break;
		default:
		{
			// Unhandled data, skip it
			result = stream.seek(dataLength, Stream::SeekOriginFromCurrent);
		}
		}
		if (!result) { return result; }
	}
	return result;
}

bool readTextureBlock(Stream& stream, assets::Model::Texture& texture)
{
	bool result;
	uint32 identifier, dataLength;
	while ((result = readTag(stream, identifier, dataLength)))
	{
		switch (identifier)
		{
		case pod::e_sceneTexture | pod::c_endTagMask:
			return true;
		case pod::e_textureFilename | pod::c_startTagMask:
		{
			StringHash s;
			result = readByteArrayIntoStringHash(stream, s, dataLength);
			texture.setName(s);
			if (!result) { return result; }
			break;
		}
		break;
		default:
		{
			// Unhandled data, skip it
			result = stream.seek(dataLength, Stream::SeekOriginFromCurrent);
		}
		}
		if (!result) { return result; }
	}
	return result;
}

bool readCameraBlock(Stream& stream, Camera& camera)
{
	bool result;
	uint32 identifier, dataLength;
	Camera::InternalData& cameraInternalData = camera.getInternalData();
	while ((result = readTag(stream, identifier, dataLength)) == true)
	{
		switch (identifier)
		{
		case pod::e_sceneCamera | pod::c_endTagMask:
			return true;
		case pod::e_cameraTargetObjectIndex | pod::c_startTagMask:
			result = read4Bytes(stream, cameraInternalData.targetNodeIdx);
			break;
		case pod::e_cameraFOV | pod::c_startTagMask:
		{
			if (cameraInternalData.FOVs.size())
			{
				result = stream.seek(dataLength, Stream::SeekOriginFromCurrent);
			}
			else
			{
				result = read4ByteArrayIntoVector<float32, float32>(stream, cameraInternalData.FOVs, 1);
				if (!result) { return result; }
			}
			break;
		}
		case pod::e_cameraFarPlane | pod::c_startTagMask:
			result = read4Bytes(stream, cameraInternalData.farClip);
			break;
		case pod::e_cameraNearPlane | pod::c_startTagMask:
			result = read4Bytes(stream, cameraInternalData.nearClip);
			break;
		case pod::e_cameraFOVAnimation | pod::c_startTagMask:
		{
			result = read4ByteArrayIntoVector<float32, float32>(stream, cameraInternalData.FOVs,
			         dataLength / sizeof(*cameraInternalData.FOVs.data()));
			if (!result) { return result; }
			break;
		}
		default:
		{
			// Unhandled data, skip it
			result = stream.seek(dataLength, Stream::SeekOriginFromCurrent);
		}
		}
		if (!result) { return result; }
	}
	return result;
}

bool readLightBlock(Stream& stream, Light& light)
{
	bool result;
	uint32 identifier, dataLength;
	Light::InternalData& lightInternalData = light.getInternalData();
	while ((result = readTag(stream, identifier, dataLength)))
	{
		switch (identifier)
		{
		case pod::e_sceneLight | pod::c_endTagMask:
		{
			return true;
		}
		case pod::e_lightTargetObjectIndex | pod::c_startTagMask:
			result = read4Bytes(stream, lightInternalData.spotTargetNodeIdx);
			break;
		case pod::e_lightColor | pod::c_startTagMask:
			result = read4ByteArray(stream, &lightInternalData.color[0],
			                        sizeof(lightInternalData.color) / sizeof(lightInternalData.color[0]));
			break;
		case pod::e_lightType | pod::c_startTagMask:
		{
			uint32 tmp;
			result = read4Bytes(stream, tmp);
			if (!result) { return result; }
			lightInternalData.type = static_cast<Light::LightType>(tmp);
			break;
		}
		case pod::e_lightConstantAttenuation | pod::c_startTagMask:
			result = read4Bytes(stream, lightInternalData.constantAttenuation);
			break;
		case pod::e_lightLinearAttenuation | pod::c_startTagMask:
			result = read4Bytes(stream, lightInternalData.linearAttenuation);
			break;
		case pod::e_lightQuadraticAttenuation | pod::c_startTagMask:
			result = read4Bytes(stream, lightInternalData.quadraticAttenuation);
			break;
		case pod::e_lightFalloffAngle | pod::c_startTagMask:
			result = read4Bytes(stream, lightInternalData.falloffAngle);
			break;
		case pod::e_lightFalloffExponent | pod::c_startTagMask:
			result = read4Bytes(stream, lightInternalData.falloffExponent);
			break;
		default:
		{
			// Unhandled data, skip it
			result = stream.seek(dataLength, Stream::SeekOriginFromCurrent);
		}
		}
		if (!result) { return result; }
	}
	return result;
}

bool readNodeBlock(Stream& stream, assets::Model::Node& node)
{
	bool result;
	uint32 identifier, dataLength;
	assets::Model::Node::InternalData& nodeInternData = node.getInternalData();
	Animation::InternalData& animInternData = nodeInternData.animation.getInternalData();
	animInternData.numberOfFrames = 0;
	bool isOldFormat = false;
	float32 pos[3] = { 0, 0, 0 };
	float32 rotation[4] = { 0, 0, 0, 1 };
	float32 scale[7] = { 1, 1, 1, 0, 0, 0, 0 };
	float32 matrix[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	while ((result = readTag(stream, identifier, dataLength)))
	{
		switch (identifier)
		{
		case pod::e_sceneNode | pod::c_endTagMask:
		{
			if (isOldFormat)
			{
				if (animInternData.positions.size())
				{
					animInternData.flags |= Animation::HasPositionAnimation;
				}
				else
				{
					animInternData.positions.resize(3);
					memcpy(animInternData.positions.data(), pos, sizeof(pos));
					animInternData.numberOfFrames = (std::max)(animInternData.numberOfFrames, (uint32)animInternData.positions.size());
				}
				if (animInternData.rotations.size())
				{
					animInternData.flags |= Animation::HasRotationAnimation;
				}
				else
				{
					animInternData.rotations.resize(4);
					memcpy(animInternData.rotations.data(), rotation, sizeof(pos));
					animInternData.numberOfFrames = (std::max)(animInternData.numberOfFrames, (uint32)animInternData.rotations.size());
				}
				if (animInternData.scales.size())
				{
					animInternData.flags |= Animation::HasScaleAnimation;
				}
				else
				{
					animInternData.scales.resize(7);
					memcpy(animInternData.scales.data(), scale, sizeof(scale));
				}
				animInternData.numberOfFrames = (std::max)(animInternData.numberOfFrames, (uint32)animInternData.scales.size());
				if (animInternData.matrices.size())
				{
					animInternData.flags |= Animation::HasMatrixAnimation;
				}
				else
				{
					animInternData.matrices.resize(16);
					memcpy(animInternData.matrices.data(), matrix, sizeof(matrix));
				}
				animInternData.numberOfFrames = (std::max)(animInternData.numberOfFrames, (uint32)animInternData.matrices.size());
			}
			return true;
		}
		case pod::e_nodeIndex | pod::c_startTagMask:
			result = read4Bytes(stream, nodeInternData.objectIndex);
			break;
		case pod::e_nodeName | pod::c_startTagMask:
		{
			result = readByteArrayIntoStringHash(stream, nodeInternData.name, dataLength);
			if (!result) { return result; }
			break;
		}
		case pod::e_nodeMaterialIndex | pod::c_startTagMask:
			result = read4Bytes(stream, nodeInternData.materialIndex);
			break;
		case pod::e_nodeParentIndex | pod::c_startTagMask:
			result = read4Bytes(stream, nodeInternData.parentIndex);
			break;
		//START OLD FORMAT
		case pod::e_nodePosition | pod::c_startTagMask:// Deprecated
			result = read4ByteArray(stream, &pos[0], 3);
			isOldFormat = true;
			break;
		case pod::e_nodeRotation | pod::c_startTagMask:// Deprecated
			result = read4ByteArray(stream, &rotation[0], 4);
			isOldFormat = true;
			break;
		case pod::e_nodeScale | pod::c_startTagMask:// Deprecated
			result = read4ByteArray(stream, &scale[0], 3);
			isOldFormat = true;
			break;
		case pod::e_nodeMatrix | pod::c_startTagMask:	// Deprecated
			result = read4ByteArray(stream, &matrix[0], 16);
			isOldFormat = true;
			break;
		//END OLD FORMAT
		case pod::e_nodeAnimationPosition | pod::c_startTagMask:
			result = read4ByteArrayIntoVector<float32, float32>(stream, animInternData.positions, dataLength / sizeof(float32));
			animInternData.numberOfFrames = std::max(animInternData.numberOfFrames, (uint32)animInternData.positions.size() / 3);
			break;
		case pod::e_nodeAnimationRotation | pod::c_startTagMask:
			result = read4ByteArrayIntoVector<float32, float32>(stream, animInternData.rotations, dataLength / sizeof(float32));
			animInternData.numberOfFrames = std::max(animInternData.numberOfFrames, (uint32)animInternData.rotations.size() / 4);
			break;
		case pod::e_nodeAnimationScale | pod::c_startTagMask:
			result = read4ByteArrayIntoVector<float32, float32>(stream, animInternData.scales, dataLength / sizeof(float32));
			animInternData.numberOfFrames = std::max(animInternData.numberOfFrames, (uint32)animInternData.scales.size() / 7);
			break;
		case pod::e_nodeAnimationMatrix | pod::c_startTagMask:
			result = read4ByteArrayIntoVector<float32, float32>(stream, animInternData.matrices, dataLength / sizeof(float32));
			animInternData.numberOfFrames = std::max(animInternData.numberOfFrames, (uint32)animInternData.matrices.size() / 16);
			break;
		case pod::e_nodeAnimationFlags | pod::c_startTagMask:
			result = read4Bytes(stream, animInternData.flags);
			break;
		case pod::e_nodeAnimationPositionIndex | pod::c_startTagMask:
			result = read4ByteArrayIntoVector<uint32, uint32>(stream, animInternData.positionIndices, dataLength / sizeof(uint32));
			animInternData.numberOfFrames = std::max(animInternData.numberOfFrames, (uint32)animInternData.positionIndices.size());
			break;
		case pod::e_nodeAnimationRotationIndex | pod::c_startTagMask:
			result = read4ByteArrayIntoVector<uint32, uint32>(stream, animInternData.rotationIndices, dataLength / sizeof(uint32));
			animInternData.numberOfFrames = std::max(animInternData.numberOfFrames, (uint32)animInternData.rotationIndices.size());
			break;
		case pod::e_nodeAnimationScaleIndex | pod::c_startTagMask:
			result = read4ByteArrayIntoVector<uint32, uint32>(stream, animInternData.scaleIndices, dataLength / sizeof(uint32));
			animInternData.numberOfFrames = std::max(animInternData.numberOfFrames, (uint32)animInternData.scaleIndices.size());
			break;
		case pod::e_nodeAnimationMatrixIndex | pod::c_startTagMask:
			result = read4ByteArrayIntoVector<uint32, uint32>(stream, animInternData.matrixIndices, dataLength / sizeof(uint32));
			animInternData.numberOfFrames = std::max(animInternData.numberOfFrames, (uint32)animInternData.matrixIndices.size());
			break;
		case pod::e_nodeUserData | pod::c_startTagMask:
			result = readByteArrayIntoVector<byte>(stream, nodeInternData.userData, dataLength);
			if (!result) { return result; }
			break;
		default:
		{
			// Unhandled data, skip it
			result = stream.seek(dataLength, Stream::SeekOriginFromCurrent);
		}
		}
		if (!result) { return result; }
	}
	return result;
}

static void fixInterleavedEndiannessUsingVertexData(StridedBuffer& interleaved, const assets::Mesh::VertexAttributeData& data, uint32 numVertices)
{
	if (!data.getN())
	{
		return;
	}
	size_t ui32TypeSize = dataTypeSize(data.getVertexLayout().dataType);
	unsigned char ub[4];
	unsigned char* pData = interleaved.data() + static_cast<size_t>(data.getOffset());
	switch (ui32TypeSize)
	{
	case 1:
		return;
	case 2:
	{
		for (unsigned int i = 0; i < numVertices; ++i)
		{
			for (unsigned int j = 0; j < data.getN(); ++j)
			{
				ub[0] = pData[ui32TypeSize * j + 0];
				ub[1] = pData[ui32TypeSize * j + 1];
				((unsigned short*)pData)[j] = (unsigned short)((ub[1] << 8) | ub[0]);
			}
			pData += interleaved.stride;
		}
	}
	break;
	case 4:
	{
		for (unsigned int i = 0; i < numVertices; ++i)
		{
			for (unsigned int j = 0; j < data.getN(); ++j)
			{
				ub[0] = pData[ui32TypeSize * j + 0];
				ub[1] = pData[ui32TypeSize * j + 1];
				ub[2] = pData[ui32TypeSize * j + 2];
				ub[3] = pData[ui32TypeSize * j + 3];
				((unsigned int*)pData)[j] = (unsigned int)((ub[3] << 24) | (ub[2] << 16) | (ub[1] << 8) | ub[0]);
			}
			pData += interleaved.stride;
		}
	}
	break;
	default:
	{
		assertion(false);
	}
	};
}

static void fixInterleavedEndianness(assets::Mesh::InternalData& data, int32 interleavedDataIndex)
{
	if (interleavedDataIndex == -1 || utils::isLittleEndian())
	{
		return;
	}
	StridedBuffer& interleavedData = data.vertexAttributeDataBlocks[interleavedDataIndex];
	assets::Mesh::VertexAttributeContainer::iterator walk = data.vertexAttributes.begin();
	for (; walk != data.vertexAttributes.end(); ++walk)
	{
		assets::Mesh::VertexAttributeData& vertexData = walk->value;
		if (static_cast<int32>(vertexData.getDataIndex()) == interleavedDataIndex)   // It should do
		{
			fixInterleavedEndiannessUsingVertexData(interleavedData, vertexData, data.primitiveData.numVertices);
		}
	}
}

bool readMeshBlock(Stream& stream, assets::Mesh& mesh)
{
	bool result;
	bool exists = false;
	uint32 identifier, dataLength, numUVWs(0), podUVWs(0), boneBatchesCount(0);
	int32 interleavedDataIndex(-1);
	assets::Mesh::InternalData& meshInternalData = mesh.getInternalData();
	meshInternalData.boneCount = 0;
	while ((result = readTag(stream, identifier, dataLength)) == true)
	{
		switch (identifier)
		{
		case pod::e_sceneMesh | pod::c_endTagMask:
		{
			meshInternalData.primitiveData.isIndexed = (meshInternalData.faces.getDataSize() != 0);
			if (meshInternalData.primitiveData.stripLengths.size())
			{
				meshInternalData.primitiveData.primitiveType = PrimitiveTopology::TriangleStrip;
			}
			else
			{
				meshInternalData.primitiveData.primitiveType = PrimitiveTopology::TriangleList;
			}
			//if (numUVWs != podUVWs || numUVWs != mesh.getNumElementsOfSemantic("UV"))
			//{
			//	return false;
			//}
			fixInterleavedEndianness(meshInternalData, interleavedDataIndex);
			return true;
		}
		case pod::e_meshNumVertices | pod::c_startTagMask:
			result = read4Bytes(stream, meshInternalData.primitiveData.numVertices);
			break;
		case pod::e_meshNumFaces | pod::c_startTagMask:
			result = read4Bytes(stream, meshInternalData.primitiveData.numFaces);
			break;
		case pod::e_meshNumUVWChannels | pod::c_startTagMask:
			result = read4Bytes(stream, podUVWs);
			break;
		case pod::e_meshStripLength | pod::c_startTagMask:
		{
			result = read4ByteArrayIntoVector<uint32, uint32>(stream, meshInternalData.primitiveData.stripLengths,
			         dataLength / sizeof(*meshInternalData.primitiveData.stripLengths.data()));
			break;
		}
		case pod::e_meshNumStrips | pod::c_startTagMask:
		{
			int32 numstr(0);
			result = read4Bytes(stream, numstr);
			assertion((size_t)numstr == meshInternalData.primitiveData.stripLengths.size());
			break;
		}
		case pod::e_meshInterleavedDataList | pod::c_startTagMask:
		{
			UCharBuffer data;
			result = readByteArrayIntoVector<byte>(stream, data, dataLength);
			if (!result) { return result; }
			interleavedDataIndex = mesh.addData(data.data(), (uint32)data.size(), 0);
			break;
		}
		case pod::e_meshBoneBatchIndexList | pod::c_startTagMask:
			result = read4ByteArrayIntoVector<uint32>(stream, meshInternalData.boneBatches.batches,
			         dataLength / sizeof(meshInternalData.boneBatches.batches[0]));
			break;
		case pod::e_meshNumBoneIndicesPerBatch | pod::c_startTagMask:
			result = read4ByteArrayIntoVector<uint32>(stream, meshInternalData.boneBatches.boneCounts,
			         dataLength / sizeof(meshInternalData.boneBatches.boneCounts[0]));
			break;
		case pod::e_meshBoneOffsetPerBatch | pod::c_startTagMask:
			result = read4ByteArrayIntoVector<uint32>(stream, meshInternalData.boneBatches.offsets,
			         dataLength / sizeof(meshInternalData.boneBatches.offsets[0]));
			break;
		case pod::e_meshMaxNumBonesPerBatch | pod::c_startTagMask:
			result = read4Bytes(stream, meshInternalData.boneBatches.boneBatchStride);
			break;
		case pod::e_meshNumBoneBatches | pod::c_startTagMask:
		{
			result = read4Bytes(stream, boneBatchesCount);
			break;
		}
		case pod::e_meshUnpackMatrix | pod::c_startTagMask:
		{
			float m[16];
			result = read4ByteArray(stream, &m[0], 16);
			meshInternalData.unpackMatrix = glm::make_mat4(&m[0]);
			break;
		}
		case pod::e_meshVertexIndexList | pod::c_startTagMask:
			result = readVertexIndexData(stream, mesh);
			break;
		case pod::e_meshVertexList | pod::c_startTagMask:
			result = readVertexData(stream, mesh, "POSITION", identifier, interleavedDataIndex, exists);
			break;
		case pod::e_meshNormalList | pod::c_startTagMask:
			result = readVertexData(stream, mesh, "NORMAL", identifier, interleavedDataIndex, exists);
			break;
		case pod::e_meshTangentList | pod::c_startTagMask:
			result = readVertexData(stream, mesh, "TANGENT", identifier, interleavedDataIndex, exists);
			break;
		case pod::e_meshBinormalList | pod::c_startTagMask:
			result = readVertexData(stream, mesh, "BINORMAL", identifier, interleavedDataIndex, exists);
			break;
		case pod::e_meshUVWList | pod::c_startTagMask:
		{
			char8 semantic[256];
			sprintf(semantic, "UV%i", numUVWs++);
			result = readVertexData(stream, mesh, semantic, identifier, interleavedDataIndex, exists);
			break;
		}
		case pod::e_meshVertexColorList | pod::c_startTagMask:
			result = readVertexData(stream, mesh, "VERTEXCOLOR", identifier, interleavedDataIndex, exists);
			break;
		case pod::e_meshBoneIndexList | pod::c_startTagMask:
			result = readVertexData(stream, mesh, "BONEINDEX", identifier, interleavedDataIndex, exists);
			if (exists) { meshInternalData.primitiveData.isSkinned = true; }
			break;
		case pod::e_meshBoneWeightList | pod::c_startTagMask:
			result = readVertexData(stream, mesh, "BONEWEIGHT", identifier, interleavedDataIndex, exists);
			if (exists)
			{
				meshInternalData.primitiveData.isSkinned = true;
				meshInternalData.boneCount = mesh.getVertexAttributeByName("BONEWEIGHT")->getN();
			}
			break;
		default:
		{
			// Unhandled data, skip it
			result = stream.seek(dataLength, Stream::SeekOriginFromCurrent);
		}
		}
		if (!result) { return result; }
	}
	assertion(0 ,  "NOT IMPLEMENTED YET");
	if (meshInternalData.boneBatches.boneCounts.size() != boneBatchesCount)
	{
		result = false;
	}
	return result;
}

bool readSceneBlock(Stream& stream, assets::Model& model)
{
	bool result;
	uint32 identifier, dataLength, temporaryInt;
	assets::Model::InternalData& modelInternalData = model.getInternalData();
	uint32 numCameras(0), numLights(0), numMaterials(0), numMeshes(0), numTextures(0), numNodes(0);
	while ((result = readTag(stream, identifier, dataLength)) == true)
	{
		switch (identifier)
		{
		case pod::Scene | pod::c_endTagMask:
		{
			if (numCameras != modelInternalData.cameras.size())
			{
				assertion(0 ,  "Unknown Error");
				return false;
			}
			if (numLights != modelInternalData.lights.size())
			{
				assertion(0 ,  "Unknown Error");
				return false;
			}
			if (numMaterials != modelInternalData.materials.size())
			{
				assertion(0 ,  "Unknown Error");
				return false;
			}
			if (numMeshes != modelInternalData.meshes.size())
			{
				assertion(0 ,  "Unknown Error");
				return false;
			}
			if (numTextures != modelInternalData.textures.size())
			{
				assertion(0 ,  "Unknown Error");
				return false;
			}
			if (numNodes != modelInternalData.nodes.size())
			{
				assertion(0 ,  "Unknown Error");
				return false;
			}
			return true;
		}
		case pod::e_sceneClearColor | pod::c_startTagMask:
			result = read4ByteArray(stream, &modelInternalData.clearColor[0],
			                        sizeof(modelInternalData.clearColor) / sizeof(modelInternalData.clearColor[0]));
			break;
		case pod::e_sceneAmbientColor | pod::c_startTagMask:
			result = read4ByteArray(stream, &modelInternalData.ambientColor[0],
			                        sizeof(modelInternalData.ambientColor) / sizeof(modelInternalData.ambientColor[0]));
			break;
		case pod::e_sceneNumCameras | pod::c_startTagMask:
			result = read4Bytes(stream, temporaryInt);
			if (!result) { return result; }
			modelInternalData.cameras.resize(temporaryInt);
			break;
		case pod::e_sceneNumLights | pod::c_startTagMask:
		{
			result = read4Bytes(stream, temporaryInt);
			if (!result) { return result; }
			modelInternalData.lights.resize(temporaryInt);
		}
		break;
		case pod::e_sceneNumMeshes | pod::c_startTagMask:
			result = read4Bytes(stream, temporaryInt);
			if (!result) { return result; }
			modelInternalData.meshes.resize(temporaryInt);
			break;
		case pod::e_sceneNumNodes | pod::c_startTagMask:
			result = read4Bytes(stream, temporaryInt);
			if (!result) { return result; }
			modelInternalData.nodes.resize(temporaryInt);
			break;
		case pod::e_sceneNumMeshNodes | pod::c_startTagMask:
			result = read4Bytes(stream, modelInternalData.numMeshNodes);
			break;
		case pod::e_sceneNumTextures | pod::c_startTagMask:
			result = read4Bytes(stream, temporaryInt);
			if (!result) { return result; }
			modelInternalData.textures.resize(temporaryInt);
			break;
		case pod::e_sceneNumMaterials | pod::c_startTagMask:
			result = read4Bytes(stream, temporaryInt);
			if (!result) { return result; }
			modelInternalData.materials.resize(temporaryInt);
			break;
		case pod::e_sceneNumFrames | pod::c_startTagMask:
			result = read4Bytes(stream, modelInternalData.numFrames);
			break;
		case pod::e_sceneCamera | pod::c_startTagMask:
			result = readCameraBlock(stream, modelInternalData.cameras[numCameras++]);
			break;
		case pod::e_sceneLight | pod::c_startTagMask:
			result = readLightBlock(stream, modelInternalData.lights[numLights++]);
			break;
		case pod::e_sceneMesh | pod::c_startTagMask:
			result = readMeshBlock(stream, modelInternalData.meshes[numMeshes++]);
			break;
		case pod::e_sceneNode | pod::c_startTagMask:
			result = readNodeBlock(stream, modelInternalData.nodes[numNodes++]);
			break;
		case pod::e_sceneTexture | pod::c_startTagMask:
			result = readTextureBlock(stream, modelInternalData.textures[numTextures++]);
			break;
		case pod::e_sceneMaterial | pod::c_startTagMask:
			result = readMaterialBlock(stream, modelInternalData.materials[numMaterials++]);
			break;
		case pod::e_sceneFlags | pod::c_startTagMask:
			result = read4Bytes(stream, modelInternalData.flags);
			break;
		case pod::e_sceneFPS | pod::c_startTagMask:
			result = read4Bytes(stream, modelInternalData.FPS);
			break;
		case pod::e_sceneUserData | pod::c_startTagMask:
			result = readByteArrayIntoVector<byte>(stream, modelInternalData.userData, dataLength);
			if (!result) { return result; }
			break;
		case pod::e_sceneUnits | pod::c_startTagMask:
			result = read4Bytes(stream, modelInternalData.units);
			break;
		default:
		{
			// Unhandled data, skip it
			result = stream.seek(dataLength, Stream::SeekOriginFromCurrent);
		}
		}
		if (!result) { return result; }
	}
	return result;
}
//
//bool getInformation(Stream& stream, string* history, string* options)
//{
//	if (history == NULL && options == NULL)
//	{
//		return true; // Nothing to do
//	}
//	bool result;
//	if (!stream.isopen())
//	{
//		result = stream.open();
//		if (!result) { return result; }
//	}
//	uint32 identifier, dataLength;
//	size_t dataRead;
//	bool needHistory = history != NULL;
//	bool needOptions = options != NULL;
//	while ((result = readTag(stream, identifier, dataLength)) == true)
//	{
//		switch (identifier)
//		{
//		case pod::PODFormatVersion | pod::c_startTagMask:
//		{
//			// Is the version string in the file the same length as ours?
//			if (dataLength != pod::c_PODFormatVersionLength)
//			{
//				assertion(0, "POD FILE VERSION MISMATCH");
//				return false;
//			}
//			// ... it is. Check to see if the string matches
//			char8 filesVersion[pod::c_PODFormatVersionLength];
//			result = stream.read(1, dataLength, &filesVersion[0], dataRead);
//			if (!result) { return result; }
//			if (strcmp(filesVersion, pod::c_PODFormatVersion) != 0)
//			{
//				assertion(0, "POD FILE VERSION MISMATCH");
//				return false;
//			}
//		}
//		continue;
//		case pod::FileHistory | pod::c_startTagMask:
//			if (needHistory)
//			{
//				std::string tempString;
//				result = readByteArrayIntoString(stream, tempString, dataLength);
//				if (result) { *history = tempString; }
//				needHistory = false;
//			}
//			else
//			{
//				result = stream.seek(dataLength, Stream::SeekOriginFromCurrent);
//			}
//			break;
//		case pod::ExportOptions | pod::c_startTagMask:
//			if (needOptions)
//			{
//				std::string tempString = 0;
//				result = readByteArrayIntoString(stream, tempString, dataLength);
//				if (result) { *options = tempString; }
//				needOptions = false;
//			}
//			else
//			{
//				result = stream.seek(dataLength, Stream::SeekOriginFromCurrent);
//			}
//			break;
//		default:
//			// Unhandled data, skip it
//			result = stream.seek(dataLength, Stream::SeekOriginFromCurrent);
//		}
//		if (!result) { return result; }
//		if (!needHistory && !needOptions)
//		{
//			return true;
//		}
//	}
//	return false;
//}

}

namespace pvr {
namespace assets {
PODReader::PODReader() : m_modelsToLoad(true)
{
}

bool PODReader::readNextAsset(assets::Model& asset)
{
	bool result;
	uint32 identifier, dataLength;
	size_t dataRead;
	while ((result = readTag(*m_assetStream, identifier, dataLength)) == true)
	{
		switch (identifier)
		{
		case pod::PODFormatVersion | pod::c_startTagMask:
		{
			// Is the version string in the file the same length as ours?
			if (dataLength != pod::c_PODFormatVersionLength)
			{
				assertion(0 ,  "FileVersionMismatch");
				return false;
			}
			// ... it is. Check to see if the string matches
			char8 filesVersion[pod::c_PODFormatVersionLength];
			result = m_assetStream->read(1, dataLength, &filesVersion[0], dataRead);
			if (!result) { return result; }
			if (strcmp(filesVersion, pod::c_PODFormatVersion) != 0)
			{
				assertion(0 ,  "FileVersionMismatch");
				return false;
			}
		}
		continue;
		case pod::Scene | pod::c_startTagMask:
			result = readSceneBlock(*m_assetStream, asset);
			if (result) { asset.initCache(); }
			return result;
		default:
			// Unhandled data, skip it
			result = m_assetStream->seek(dataLength, Stream::SeekOriginFromCurrent);
			if (!result) { return result; }
		}
	}
	return result;
}
bool PODReader::hasAssetsLeftToLoad()
{
	return m_modelsToLoad;
}

bool PODReader::canHaveMultipleAssets()
{
	return false;
}

bool PODReader::isSupportedFile(Stream& assetStream)
{
	if (!assetStream.isopen())
	{
		if (assetStream.isopen() != true) { return false; }
	}
	uint32 identifier, dataLength;
	size_t dataRead;
	while (readTag(assetStream, identifier, dataLength) == true)
	{
		switch (identifier)
		{
		case pod::PODFormatVersion | pod::c_startTagMask:
		{
			// Is the version string in the file the same length as ours?
			if (dataLength != pod::c_PODFormatVersionLength) { return false; }
			// ... it is. Check to see if the string matches
			char8 filesVersion[pod::c_PODFormatVersionLength];
			if (assetStream.read(1, dataLength, &filesVersion[0], dataRead) != true) { return false; }
			if (strcmp(filesVersion, pod::c_PODFormatVersion) != 0) { return false; }
			return true;
		}
		continue;
		default:
			// Unhandled data, skip it
			assetStream.seek(dataLength, Stream::SeekOriginFromCurrent);
		}
	}
	return false;
}

vector<string> PODReader::getSupportedFileExtensions()
{
	vector<string> extensions;
	extensions.push_back("pvr");
	return vector<string>(extensions);
}

}
}
//!\endcond
