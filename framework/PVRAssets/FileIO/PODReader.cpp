/*!
\brief Implementation of methods of the PODReader class.
\file PVRAssets/FileIO/PODReader.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRAssets/FileIO/PODReader.h"
#include "PVRAssets/FileIO/PODDefines.h"
#include "PVRAssets/Model.h"
#include "PVRCore/Log.h"
#include "PVRAssets/Helper.h"
#include "PVRCore/Stream.h"
#include <cstdio>
#include <algorithm>
using std::vector;

namespace { // LOCAL FUNCTIONS
using namespace pvr;
using namespace assets;
template<typename T>
void readBytes(Stream& stream, T& data)
{
	stream.readExact(sizeof(T), 1, &data);
}

template<typename T>
void readByteArray(Stream& stream, T* data, uint32_t count)
{
	for (uint32_t i = 0; i < count; ++i)
	{
		readBytes(stream, data[i]);
	}
}

template<typename T>
void readByteArrayIntoTypedMem(Stream& stream, TypedMem& mem, uint32_t count)
{
	mem.allocate(GpuDatatypesHelper::Metadata<T>::dataTypeOf(), count);
	readByteArray(stream, mem.rawAs<T>(), count);
}
template<typename T>
void readByteArrayIntoFreeValue(Stream& stream, FreeValue& mem, uint32_t count)
{
	debug_assertion(count * sizeof(T) <= 64, "PODReader: Error trying to read more than 64 bytes into FreeValue");
	mem.setDataType(GpuDatatypesHelper::Metadata<T>::dataTypeOf());
	readByteArray(stream, mem.rawAs<T>(), count);
}

template<typename T>
void read4Bytes(Stream& stream, T& data)
{
	// PVR_STATIC_ASSERT(read4BytesSizeAssert, sizeof(T) == 4)
	unsigned char ub[4];
	stream.readExact(4, 1, &ub);
	unsigned int p;
	p = static_cast<unsigned int>((ub[3] << 24) | (ub[2] << 16) | (ub[1] << 8) | ub[0]);
	memcpy(&data, &p, 4);
}

template<typename T>
void read4BytesIntoFreeVal(Stream& stream, FreeValue& value)
{
	value.setDataType(GpuDatatypesHelper::Metadata<T>::dataTypeOf());
	read4Bytes(stream, *value.rawAs<T>());
}
template<typename T>
void read4BytesIntoTypedMem(Stream& stream, TypedMem& value)
{
	value.allocate(GpuDatatypesHelper::Metadata<T>::dataTypeOf());
	read4Bytes(stream, value.rawAs<T>());
}

template<typename T>
void read4ByteArray(Stream& stream, T* data, uint32_t count)
{
	// PVR_STATIC_ASSERT(read4ByteArraySizeAssert, sizeof(T) == 4)
	for (uint32_t i = 0; i < count; ++i)
	{
		read4Bytes(stream, data[i]);
	}
}

template<typename VectorType>
void read4ByteArrayIntoGlmVector(Stream& stream, FreeValue& value)
{
	value.setDataType(GpuDatatypesHelper::Metadata<VectorType>::dataTypeOf());
	read4ByteArray(stream, &(*static_cast<VectorType*>(value.raw()))[0], getNumVecElements(value.dataType()));
}

template<typename T>
void read2Bytes(Stream& stream, T& data)
{
	// PVR_STATIC_ASSERT(read2BytesSizeAssert, sizeof(T) == 2)
	unsigned char ub[2];
	size_t dataRead;
	stream.read(2, 1, &ub, dataRead);
	unsigned short* p = reinterpret_cast<unsigned short*>(&data);
	*p = static_cast<unsigned short>((ub[1] << 8) | ub[0]);
}

template<typename T>
void read2ByteArray(Stream& stream, T* data, uint32_t count)
{
	// PVR_STATIC_ASSERT(read2ByteArraySizeAssert, sizeof(T) == 2)
	for (uint32_t i = 0; i < count; ++i)
	{
		read2Bytes(stream, data[i]);
	}
}

template<typename T, typename vector_T>
void readByteArrayIntoVector(Stream& stream, std::vector<vector_T>& data, uint32_t count)
{
	debug_assertion(sizeof(vector_T) <= sizeof(T), "Wrong size of vector type in PODReader");
	data.resize(count * sizeof(T) / sizeof(vector_T));
	readByteArray<T>(stream, reinterpret_cast<T*>(data.data()), count);
}
template<typename T, typename vector_T>
void read2ByteArrayIntoVector(Stream& stream, std::vector<vector_T>& data, uint32_t count)
{
	debug_assertion(sizeof(vector_T) <= sizeof(T), "Wrong size of vector type in PODReader");
	data.resize(count * sizeof(T) / sizeof(vector_T));
	read2ByteArray<T>(stream, reinterpret_cast<T*>(data.data()), count);
}
template<typename T, typename vector_T>
void read4ByteArrayIntoVector(Stream& stream, std::vector<vector_T>& data, uint32_t count)
{
	debug_assertion(sizeof(vector_T) <= sizeof(T), "Wrong size of vector type in PODReader");
	data.resize(count * sizeof(T) / sizeof(vector_T));
	read4ByteArray<T>(stream, reinterpret_cast<T*>(data.data()), count);
}

void readByteArrayIntoStringHash(Stream& stream, StringHash& data, uint32_t count)
{
	std::vector<char> data1;
	data1.resize(count);
	readByteArray(stream, reinterpret_cast<char*>(data1.data()), count);
	data.assign(data1.data());
}

template<typename T>
bool read4BytesChecked(Stream& stream, T& data)
{
	// PVR_STATIC_ASSERT(read4BytesSizeAssert, sizeof(T) == 4)
	unsigned char ub[4];
	size_t dataRead;
	stream.read(4, 1, &ub, dataRead);
	if (dataRead != 1)
	{
		return false;
	}
	unsigned int p;
	p = static_cast<unsigned int>((ub[3] << 24) | (ub[2] << 16) | (ub[1] << 8) | ub[0]);
	memcpy(&data, &p, 4);
	return true;
}

bool readTag(Stream& stream, uint32_t& identifier, uint32_t& dataLength)
{
	return read4BytesChecked(stream, identifier) && read4BytesChecked(stream, dataLength);
}

void readVertexIndexData(Stream& stream, assets::Mesh& mesh)
{
	uint32_t identifier, dataLength, size(0);
	std::vector<uint8_t> data;
	IndexType type(IndexType::IndexType16Bit);
	while (readTag(stream, identifier, dataLength))
	{
		if (identifier == (pod::e_meshVertexIndexList | pod::c_endTagMask))
		{
			mesh.addFaces(data.data(), size, type);
			return;
		}
		switch (identifier)
		{
		case pod::e_blockDataType:
		{
			uint32_t tmp;
			read4Bytes(stream, tmp);
			switch (static_cast<DataType>(tmp))
			{
			case DataType::UInt32:
				type = IndexType::IndexType32Bit;
				break;
			case DataType::UInt16:
				type = IndexType::IndexType16Bit;
				break;
			default:
			{
				throw InvalidDataError("[PODReader::readVertexIndexData]: Unrecognised Index data type");
			}
			}
			continue;
		}
		case pod::e_blockData:
			switch (type)
			{
			case IndexType::IndexType16Bit:
				read2ByteArrayIntoVector<uint16_t>(stream, data, dataLength / 2);
				break;
			case IndexType::IndexType32Bit:
				read4ByteArrayIntoVector<uint32_t>(stream, data, dataLength / 4);
				break;
			}
			size = dataLength;
			break;
		default:
		{
			// Unhandled data, skip it
			stream.seek(dataLength, Stream::SeekOriginFromCurrent);
		}
		}
	}
}

void readVertexData(Stream& stream, assets::Mesh& mesh, const char* const semanticName, uint32_t blockIdentifier, int32_t dataIndex, bool& existed)
{
	existed = false;
	uint32_t identifier, dataLength, numComponents(0), stride(0), offset(0);
	DataType type(DataType::None);
	while (readTag(stream, identifier, dataLength))
	{
		if (identifier == (blockIdentifier | pod::c_endTagMask))
		{
			if (numComponents != 0) // Is there a Vertex Attribute to add?
			{
				existed = true;
				mesh.setStride(dataIndex, stride);
				if (mesh.addVertexAttribute(semanticName, type, numComponents, offset, dataIndex) == -1)
				{
					throw InvalidDataError("[PODReader::readVertexData] : Add Vertex Attribute [" + std::string(semanticName) + "] failed - Vertex attribute already added");
				}
			}
			else
			{
				existed = false;
			}
			return;
		}
		switch (identifier)
		{
		case pod::e_blockDataType:
		{
			uint32_t tmp;
			read4Bytes(stream, tmp);
			type = static_cast<DataType>(tmp);
			continue;
		}
		case pod::e_blockNumComponents:
			read4Bytes(stream, numComponents);
			break;
		case pod::e_blockStride:
			read4Bytes(stream, stride);
			break;
		case pod::e_blockData:
			if (dataIndex == -1) // This POD file isn't using interleaved data so this data block must be valid vertex data
			{
				std::vector<uint8_t> data;
				switch (dataTypeSize(type))
				{
				case 1:
					readByteArrayIntoVector<uint8_t>(stream, data, dataLength);
					break;
				case 2:
					read2ByteArrayIntoVector<uint16_t>(stream, data, dataLength / 2);
					break;
				case 4:
					read2ByteArrayIntoVector<uint32_t>(stream, data, dataLength / 4);
					break;
				default:
				{
					throw InvalidDataError("[PODReader::readVertexData] : Vertex DataType width was >4");
				}
				}
				dataIndex = mesh.addData(data.data(), dataLength, stride);
			}
			else
			{
				read4Bytes(stream, offset);
			}
			break;
		default:
		{
			// Unhandled data, skip it
			stream.seek(dataLength, Stream::SeekOriginFromCurrent);
		}
		}
	}
}

void readTextureIndex(Stream& stream, const StringHash& semantic, assets::Model::Material::InternalData& data)
{
	int32_t tmp = -1;
	read4Bytes<int32_t>(stream, tmp);
	if (tmp >= 0)
	{
		data.textureIndices[semantic] = tmp;
	}
}

void readMaterialBlock(Stream& stream, assets::Model::Material& material)
{
	uint32_t identifier, dataLength;
	assets::Model::Material::InternalData& materialInternalData = material.getInternalData();
	while (readTag(stream, identifier, dataLength))
	{
		switch (identifier)
		{
		case pod::e_sceneMaterial | pod::c_endTagMask:
			return;
		case pod::e_materialName | pod::c_startTagMask:
			readByteArrayIntoStringHash(stream, materialInternalData.name, dataLength);
			break;
		case pod::e_materialOpacity | pod::c_startTagMask:
			read4BytesIntoFreeVal<int32_t>(stream, materialInternalData.materialSemantics["OPACITY"]);
			break;
		case pod::e_materialAmbientColor | pod::c_startTagMask:
			read4ByteArrayIntoGlmVector<glm::vec3>(stream, materialInternalData.materialSemantics["AMBIENT"]);
			break;
		case pod::e_materialDiffuseColor | pod::c_startTagMask:
			read4ByteArrayIntoGlmVector<glm::vec3>(stream, materialInternalData.materialSemantics["DIFFUSE"]);
			break;
		case pod::e_materialSpecularColor | pod::c_startTagMask:
			read4ByteArrayIntoGlmVector<glm::vec3>(stream, materialInternalData.materialSemantics["SPECULAR"]);
			break;
		case pod::e_materialShininess | pod::c_startTagMask:
			read4BytesIntoFreeVal<float>(stream, materialInternalData.materialSemantics["SHININESS"]);
			break;
		case pod::e_materialEffectFile | pod::c_startTagMask:
			readByteArrayIntoStringHash(stream, materialInternalData.effectFile, dataLength);
			break;
		case pod::e_materialEffectName | pod::c_startTagMask:
			readByteArrayIntoStringHash(stream, materialInternalData.effectName, dataLength);
			break;
		case pod::e_materialDiffuseTextureIndex | pod::c_startTagMask:
			readTextureIndex(stream, "DIFFUSEMAP", materialInternalData);
			break;
		case pod::e_materialAmbientTextureIndex | pod::c_startTagMask:
			readTextureIndex(stream, "AMBIENTMAP", materialInternalData);
			break;
		case pod::e_materialSpecularColorTextureIndex | pod::c_startTagMask:
			readTextureIndex(stream, "SPECULARCOLORMAP", materialInternalData);
			break;
		case pod::e_materialSpecularLevelTextureIndex | pod::c_startTagMask:
			readTextureIndex(stream, "SPECULARLEVELMAP", materialInternalData);
			break;
		case pod::e_materialBumpMapTextureIndex | pod::c_startTagMask:
			readTextureIndex(stream, "NORMALMAP", materialInternalData);
			break;
		case pod::e_materialEmissiveTextureIndex | pod::c_startTagMask:
			readTextureIndex(stream, "EMISSIVEMAP", materialInternalData);
			break;
		case pod::e_materialGlossinessTextureIndex | pod::c_startTagMask:
			readTextureIndex(stream, "GLOSSINESSMAP", materialInternalData);
			break;
		case pod::e_materialOpacityTextureIndex | pod::c_startTagMask:
			readTextureIndex(stream, "OPACITYMAP", materialInternalData);
			break;
		case pod::e_materialReflectionTextureIndex | pod::c_startTagMask:
			readTextureIndex(stream, "REFLECTIONMAP", materialInternalData);
			break;
		case pod::e_materialRefractionTextureIndex | pod::c_startTagMask:
			readTextureIndex(stream, "REFRACTIONMAP", materialInternalData);
			break;
		case pod::e_materialBlendingRGBSrc | pod::c_startTagMask:
		{
			uint32_t tmp;
			read4Bytes(stream, tmp);
			materialInternalData.materialSemantics["BLENDFUNCSRCCOLOR"].setValue(tmp);
			break;
		}
		case pod::e_materialBlendingAlphaSrc | pod::c_startTagMask:
		{
			uint32_t tmp;
			read4Bytes(stream, tmp);
			materialInternalData.materialSemantics["BLENDFUNCSRCALPHA"].setValue(tmp);
			break;
		}
		case pod::e_materialBlendingRGBDst | pod::c_startTagMask:
		{
			uint32_t tmp;
			read4Bytes(stream, tmp);
			materialInternalData.materialSemantics["BLENDFUNCDSTCOLOR"].setValue(tmp);
			break;
		}
		case pod::e_materialBlendingAlphaDst | pod::c_startTagMask:
		{
			uint32_t tmp;
			read4Bytes(stream, tmp);
			materialInternalData.materialSemantics["BLENDFUNCDSTALPHA"].setValue(tmp);
			break;
		}
		case pod::e_materialBlendingRGBOperation | pod::c_startTagMask:
		{
			uint32_t tmp;
			read4Bytes(stream, tmp);
			materialInternalData.materialSemantics["BLENDOPCOLOR"].setValue(tmp);
			break;
		}
		case pod::e_materialBlendingAlphaOperation | pod::c_startTagMask:
		{
			uint32_t tmp;
			read4Bytes(stream, tmp);
			materialInternalData.materialSemantics["BLENDOPALPHA"].setValue(tmp);
			break;
		}
		case pod::e_materialBlendingRGBAColor | pod::c_startTagMask:
			read4ByteArrayIntoGlmVector<glm::vec4>(stream, materialInternalData.materialSemantics["BLENDCOLOR"]);
			break;
		case pod::e_materialBlendingFactorArray | pod::c_startTagMask:
			read4ByteArrayIntoGlmVector<glm::vec4>(stream, materialInternalData.materialSemantics["BLENDFACTOR"]);
			break;
		case pod::e_materialFlags | pod::c_startTagMask:
			read4BytesIntoFreeVal<int32_t>(stream, materialInternalData.materialSemantics["FLAGS"]);
			break;
		case pod::e_materialUserData | pod::c_startTagMask:
			readByteArrayIntoVector<uint8_t>(stream, materialInternalData.userData, dataLength);
			break;
		default:
			// Unhandled data, skip it
			stream.seek(dataLength, Stream::SeekOriginFromCurrent);
			break;
		}
	}
}

void readTextureBlock(Stream& stream, assets::Model::Texture& texture)
{
	uint32_t identifier, dataLength;
	while (readTag(stream, identifier, dataLength))
	{
		switch (identifier)
		{
		case pod::e_sceneTexture | pod::c_endTagMask:
			return;
		case pod::e_textureFilename | pod::c_startTagMask:
		{
			StringHash s;
			readByteArrayIntoStringHash(stream, s, dataLength);
			texture.setName(s);
			break;
		}
		break;
		default:
		{
			// Unhandled data, skip it
			stream.seek(dataLength, Stream::SeekOriginFromCurrent);
		}
		}
	}
}

void readCameraBlock(Stream& stream, Camera& camera)
{
	uint32_t identifier, dataLength;
	Camera::InternalData& cameraInternalData = camera.getInternalData();
	while (readTag(stream, identifier, dataLength))
	{
		switch (identifier)
		{
		case pod::e_sceneCamera | pod::c_endTagMask:
			return;
		case pod::e_cameraTargetObjectIndex | pod::c_startTagMask:
			read4Bytes(stream, cameraInternalData.targetNodeIdx);
			break;
		case pod::e_cameraFOV | pod::c_startTagMask:
		{
			if (cameraInternalData.FOVs.size())
			{
				stream.seek(dataLength, Stream::SeekOriginFromCurrent);
			}
			else
			{
				read4ByteArrayIntoVector<float, float>(stream, cameraInternalData.FOVs, 1);
			}
			break;
		}
		case pod::e_cameraFarPlane | pod::c_startTagMask:
			read4Bytes(stream, cameraInternalData.farClip);
			break;
		case pod::e_cameraNearPlane | pod::c_startTagMask:
			read4Bytes(stream, cameraInternalData.nearClip);
			break;
		case pod::e_cameraFOVAnimation | pod::c_startTagMask:
		{
			read4ByteArrayIntoVector<float, float>(stream, cameraInternalData.FOVs, dataLength / sizeof(*cameraInternalData.FOVs.data()));
			break;
		}
		default:
			stream.seek(dataLength, Stream::SeekOriginFromCurrent);
			break;
		}
	}
}

void readLightBlock(Stream& stream, Light& light)
{
	uint32_t identifier, dataLength;
	Light::InternalData& lightInternalData = light.getInternalData();
	while (readTag(stream, identifier, dataLength))
	{
		switch (identifier)
		{
		case pod::e_sceneLight | pod::c_endTagMask:
			return;
		case pod::e_lightTargetObjectIndex | pod::c_startTagMask:
			read4Bytes(stream, lightInternalData.spotTargetNodeIdx);
			break;
		case pod::e_lightColor | pod::c_startTagMask:
			read4ByteArray(stream, &lightInternalData.color[0], sizeof(lightInternalData.color) / sizeof(lightInternalData.color[0]));
			break;
		case pod::e_lightType | pod::c_startTagMask:
		{
			uint32_t tmp;
			read4Bytes(stream, tmp);
			lightInternalData.type = static_cast<Light::LightType>(tmp);
			break;
		}
		case pod::e_lightConstantAttenuation | pod::c_startTagMask:
			read4Bytes(stream, lightInternalData.constantAttenuation);
			break;
		case pod::e_lightLinearAttenuation | pod::c_startTagMask:
			read4Bytes(stream, lightInternalData.linearAttenuation);
			break;
		case pod::e_lightQuadraticAttenuation | pod::c_startTagMask:
			read4Bytes(stream, lightInternalData.quadraticAttenuation);
			break;
		case pod::e_lightFalloffAngle | pod::c_startTagMask:
			read4Bytes(stream, lightInternalData.falloffAngle);
			break;
		case pod::e_lightFalloffExponent | pod::c_startTagMask:
			read4Bytes(stream, lightInternalData.falloffExponent);
			break;
		default:
			stream.seek(dataLength, Stream::SeekOriginFromCurrent);
			break;
		}
	}
}

void readNodeBlock(Stream& stream, assets::Model::Node& node)
{
	uint32_t identifier, dataLength;
	assets::Model::Node::InternalData& nodeInternData = node.getInternalData();
	Animation::InternalData& animInternData = nodeInternData.animation.getInternalData();
	animInternData.numFrames = 0;
	bool isOldFormat = false;
	float pos[3] = { 0, 0, 0 };
	float rotation[4] = { 0, 0, 0, 1 };
	float scale[7] = { 1, 1, 1, 0, 0, 0, 0 };
	float matrix[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	while (readTag(stream, identifier, dataLength))
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
					animInternData.numFrames = (std::max)(animInternData.numFrames, static_cast<uint32_t>(animInternData.positions.size()));
				}
				if (animInternData.rotations.size())
				{
					animInternData.flags |= Animation::HasRotationAnimation;
				}
				else
				{
					animInternData.rotations.resize(4);
					memcpy(animInternData.rotations.data(), rotation, sizeof(pos));
					animInternData.numFrames = (std::max)(animInternData.numFrames, static_cast<uint32_t>(animInternData.rotations.size()));
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
				animInternData.numFrames = (std::max)(animInternData.numFrames, static_cast<uint32_t>(animInternData.scales.size()));
				if (animInternData.matrices.size())
				{
					animInternData.flags |= Animation::HasMatrixAnimation;
				}
				else
				{
					animInternData.matrices.resize(16);
					memcpy(animInternData.matrices.data(), matrix, sizeof(matrix));
				}
				animInternData.numFrames = (std::max)(animInternData.numFrames, static_cast<uint32_t>(animInternData.matrices.size()));
			}
			return;
		}
		case pod::e_nodeIndex | pod::c_startTagMask:
			read4Bytes(stream, nodeInternData.objectIndex);
			break;
		case pod::e_nodeName | pod::c_startTagMask:
			readByteArrayIntoStringHash(stream, nodeInternData.name, dataLength);
			break;
		case pod::e_nodeMaterialIndex | pod::c_startTagMask:
			read4Bytes(stream, nodeInternData.materialIndex);
			break;
		case pod::e_nodeParentIndex | pod::c_startTagMask:
			read4Bytes(stream, nodeInternData.parentIndex);
			break;
		// START OLD FORMAT --- DEPRECATED
		case pod::e_nodePosition | pod::c_startTagMask:
			read4ByteArray(stream, &pos[0], 3);
			isOldFormat = true;
			break;
		case pod::e_nodeRotation | pod::c_startTagMask:
			read4ByteArray(stream, &rotation[0], 4);
			isOldFormat = true;
			break;
		case pod::e_nodeScale | pod::c_startTagMask:
			read4ByteArray(stream, &scale[0], 3);
			isOldFormat = true;
			break;
		case pod::e_nodeMatrix | pod::c_startTagMask:
			read4ByteArray(stream, &matrix[0], 16);
			isOldFormat = true;
			break;
		// END OLD FORMAT
		case pod::e_nodeAnimationPosition | pod::c_startTagMask:
			read4ByteArrayIntoVector<float, float>(stream, animInternData.positions, dataLength / sizeof(float));
			animInternData.numFrames = std::max(animInternData.numFrames, static_cast<uint32_t>(animInternData.positions.size()) / 3);
			break;
		case pod::e_nodeAnimationRotation | pod::c_startTagMask:
			read4ByteArrayIntoVector<float, float>(stream, animInternData.rotations, dataLength / sizeof(float));
			animInternData.numFrames = std::max(animInternData.numFrames, static_cast<uint32_t>(animInternData.rotations.size()) / 4);
			break;
		case pod::e_nodeAnimationScale | pod::c_startTagMask:
			read4ByteArrayIntoVector<float, float>(stream, animInternData.scales, dataLength / sizeof(float));
			animInternData.numFrames = std::max(animInternData.numFrames, static_cast<uint32_t>(animInternData.scales.size()) / 7);
			break;
		case pod::e_nodeAnimationMatrix | pod::c_startTagMask:
			read4ByteArrayIntoVector<float, float>(stream, animInternData.matrices, dataLength / sizeof(float));
			animInternData.numFrames = std::max(animInternData.numFrames, static_cast<uint32_t>(animInternData.matrices.size()) / 16);
			break;
		case pod::e_nodeAnimationFlags | pod::c_startTagMask:
			read4Bytes(stream, animInternData.flags);
			break;
		case pod::e_nodeAnimationPositionIndex | pod::c_startTagMask:
			read4ByteArrayIntoVector<uint32_t, uint32_t>(stream, animInternData.positionIndices, dataLength / sizeof(uint32_t));
			animInternData.numFrames = std::max(animInternData.numFrames, static_cast<uint32_t>(animInternData.positionIndices.size()));
			break;
		case pod::e_nodeAnimationRotationIndex | pod::c_startTagMask:
			read4ByteArrayIntoVector<uint32_t, uint32_t>(stream, animInternData.rotationIndices, dataLength / sizeof(uint32_t));
			animInternData.numFrames = std::max(animInternData.numFrames, static_cast<uint32_t>(animInternData.rotationIndices.size()));
			break;
		case pod::e_nodeAnimationScaleIndex | pod::c_startTagMask:
			read4ByteArrayIntoVector<uint32_t, uint32_t>(stream, animInternData.scaleIndices, dataLength / sizeof(uint32_t));
			animInternData.numFrames = std::max(animInternData.numFrames, static_cast<uint32_t>(animInternData.scaleIndices.size()));
			break;
		case pod::e_nodeAnimationMatrixIndex | pod::c_startTagMask:
			read4ByteArrayIntoVector<uint32_t, uint32_t>(stream, animInternData.matrixIndices, dataLength / sizeof(uint32_t));
			animInternData.numFrames = std::max(animInternData.numFrames, static_cast<uint32_t>(animInternData.matrixIndices.size()));
			break;
		case pod::e_nodeUserData | pod::c_startTagMask:
			readByteArrayIntoVector<uint8_t>(stream, nodeInternData.userData, dataLength);
			break;
		default:
			stream.seek(dataLength, Stream::SeekOriginFromCurrent);
			break;
		}
	}
}

static void fixInterleavedEndiannessUsingVertexData(StridedBuffer& interleaved, const assets::Mesh::VertexAttributeData& data, uint32_t numVertices)
{
	if (!data.getN())
	{
		return;
	}
	size_t ui32TypeSize = dataTypeSize(data.getVertexLayout().dataType);
	char ub[4];
	uint8_t* pData = interleaved.data() + static_cast<size_t>(data.getOffset());
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
		throw InvalidDataError("[PODReader::fixInterleavedEndiannessUsingVertexData] Interleaved endianness fix - data type had width >4!");
	}
	};
}

static void fixInterleavedEndianness(assets::Mesh::InternalData& data, int32_t interleavedDataIndex)
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
		if (static_cast<int32_t>(vertexData.getDataIndex()) == interleavedDataIndex) // It should do
		{
			fixInterleavedEndiannessUsingVertexData(interleavedData, vertexData, data.primitiveData.numVertices);
		}
	}
}

void readMeshBlock(Stream& stream, assets::Mesh& mesh)
{
	bool exists = false;
	uint32_t identifier, dataLength, numUVWs(0), podUVWs(0), numBoneBatches(0);
	int32_t interleavedDataIndex(-1);
	assets::Mesh::InternalData& meshInternalData = mesh.getInternalData();
	meshInternalData.numBones = 0;
	while (readTag(stream, identifier, dataLength))
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
			fixInterleavedEndianness(meshInternalData, interleavedDataIndex);
			return;
		}
		case pod::e_meshNumVertices | pod::c_startTagMask:
			read4Bytes(stream, meshInternalData.primitiveData.numVertices);
			break;
		case pod::e_meshNumFaces | pod::c_startTagMask:
			read4Bytes(stream, meshInternalData.primitiveData.numFaces);
			break;
		case pod::e_meshNumUVWChannels | pod::c_startTagMask:
			read4Bytes(stream, podUVWs);
			break;
		case pod::e_meshStripLength | pod::c_startTagMask:
		{
			read4ByteArrayIntoVector<uint32_t, uint32_t>(stream, meshInternalData.primitiveData.stripLengths, dataLength / sizeof(*meshInternalData.primitiveData.stripLengths.data()));
			break;
		}
		case pod::e_meshNumStrips | pod::c_startTagMask:
		{
			int32_t numstr(0);
			read4Bytes(stream, numstr);
			if ((size_t)numstr != meshInternalData.primitiveData.stripLengths.size())
			{
				throw InvalidDataError("[PODReader::readMeshBlock]: The number of Triangle Strip Lengths was different to the actual number of triangle strips.");
			}
			break;
		}
		case pod::e_meshInterleavedDataList | pod::c_startTagMask:
		{
			UInt8Buffer data;
			readByteArrayIntoVector<uint8_t>(stream, data, dataLength);
			interleavedDataIndex = mesh.addData(data.data(), static_cast<uint32_t>(data.size()), 0);
			break;
		}
		case pod::e_meshBoneBatchIndexList | pod::c_startTagMask:
			read4ByteArrayIntoVector<uint32_t>(stream, meshInternalData.boneBatches.batches, dataLength / sizeof(meshInternalData.boneBatches.batches[0]));
			break;
		case pod::e_meshNumBoneIndicesPerBatch | pod::c_startTagMask:
			read4ByteArrayIntoVector<uint32_t>(stream, meshInternalData.boneBatches.numBones, dataLength / sizeof(meshInternalData.boneBatches.numBones[0]));
			break;
		case pod::e_meshBoneOffsetPerBatch | pod::c_startTagMask:
			read4ByteArrayIntoVector<uint32_t>(stream, meshInternalData.boneBatches.offsets, dataLength / sizeof(meshInternalData.boneBatches.offsets[0]));
			break;
		case pod::e_meshMaxNumBonesPerBatch | pod::c_startTagMask:
			read4Bytes(stream, meshInternalData.boneBatches.boneBatchStride);
			break;
		case pod::e_meshNumBoneBatches | pod::c_startTagMask:
		{
			read4Bytes(stream, numBoneBatches);
			break;
		}
		case pod::e_meshUnpackMatrix | pod::c_startTagMask:
		{
			float m[16];
			read4ByteArray(stream, &m[0], 16);
			meshInternalData.unpackMatrix = glm::make_mat4(&m[0]);
			break;
		}
		case pod::e_meshVertexIndexList | pod::c_startTagMask:
			readVertexIndexData(stream, mesh);
			break;
		case pod::e_meshVertexList | pod::c_startTagMask:
			readVertexData(stream, mesh, "POSITION", identifier, interleavedDataIndex, exists);
			break;
		case pod::e_meshNormalList | pod::c_startTagMask:
			readVertexData(stream, mesh, "NORMAL", identifier, interleavedDataIndex, exists);
			break;
		case pod::e_meshTangentList | pod::c_startTagMask:
			readVertexData(stream, mesh, "TANGENT", identifier, interleavedDataIndex, exists);
			break;
		case pod::e_meshBinormalList | pod::c_startTagMask:
			readVertexData(stream, mesh, "BINORMAL", identifier, interleavedDataIndex, exists);
			break;
		case pod::e_meshUVWList | pod::c_startTagMask:
		{
			char semantic[256];
			sprintf(semantic, "UV%i", numUVWs++);
			readVertexData(stream, mesh, semantic, identifier, interleavedDataIndex, exists);
			break;
		}
		case pod::e_meshVertexColorList | pod::c_startTagMask:
			readVertexData(stream, mesh, "VERTEXCOLOR", identifier, interleavedDataIndex, exists);
			break;
		case pod::e_meshBoneIndexList | pod::c_startTagMask:
			readVertexData(stream, mesh, "BONEINDEX", identifier, interleavedDataIndex, exists);
			if (exists)
			{
				meshInternalData.primitiveData.isSkinned = true;
			}
			break;
		case pod::e_meshBoneWeightList | pod::c_startTagMask:
			readVertexData(stream, mesh, "BONEWEIGHT", identifier, interleavedDataIndex, exists);
			if (exists)
			{
				meshInternalData.primitiveData.isSkinned = true;
				meshInternalData.numBones = mesh.getVertexAttributeByName("BONEWEIGHT")->getN();
			}
			break;
		default:
		{
			// Unhandled data, skip it
			stream.seek(dataLength, Stream::SeekOriginFromCurrent);
		}
		}
	}
	assertion(0, "NOT IMPLEMENTED YET");
	if (meshInternalData.boneBatches.numBones.size() != numBoneBatches)
	{
		throw InvalidDataError("[PODReader::readMeshBlock]: Number of bone batches was incorrect.");
	}
}

void readSceneBlock(Stream& stream, assets::Model& model)
{
	uint32_t identifier, dataLength, temporaryInt;
	assets::Model::InternalData& modelInternalData = model.getInternalData();
	uint32_t numCameras(0), numLights(0), numMaterials(0), numMeshes(0), numTextures(0), numNodes(0);
	while (readTag(stream, identifier, dataLength))
	{
		switch (identifier)
		{
		case pod::Scene | pod::c_endTagMask:
		{
			if (numCameras != modelInternalData.cameras.size())
			{
				throw InvalidDataError("[PODReader::readSceneBlock]: Unknown error - Number of cameras was incorrect.");
			}
			if (numLights != modelInternalData.lights.size())
			{
				throw InvalidDataError("[PODReader::readSceneBlock]: Unknown error - Number of lights was incorrect.");
			}
			if (numMaterials != modelInternalData.materials.size())
			{
				throw InvalidDataError("[PODReader::readSceneBlock]: Unknown error - Number of materials was incorrect.");
			}
			if (numMeshes != modelInternalData.meshes.size())
			{
				throw InvalidDataError("[PODReader::readSceneBlock]: Unknown error - Number of meshes was incorrect.");
			}
			if (numTextures != modelInternalData.textures.size())
			{
				throw InvalidDataError("[PODReader::readSceneBlock]: Unknown error - Number of textures was incorrect.");
			}
			if (numNodes != modelInternalData.nodes.size())
			{
				throw InvalidDataError("[PODReader::readSceneBlock]: Unknown error - Number of nodes was incorrect.");
			}
			return;
		}
		case pod::e_sceneClearColor | pod::c_startTagMask:
			read4ByteArray(stream, &modelInternalData.clearColor[0], sizeof(modelInternalData.clearColor) / sizeof(modelInternalData.clearColor[0]));
			break;
		case pod::e_sceneAmbientColor | pod::c_startTagMask:
			read4ByteArray(stream, &modelInternalData.ambientColor[0], sizeof(modelInternalData.ambientColor) / sizeof(modelInternalData.ambientColor[0]));
			break;
		case pod::e_sceneNumCameras | pod::c_startTagMask:
			read4Bytes(stream, temporaryInt);
			modelInternalData.cameras.resize(temporaryInt);
			break;
		case pod::e_sceneNumLights | pod::c_startTagMask:
		{
			read4Bytes(stream, temporaryInt);
			modelInternalData.lights.resize(temporaryInt);
		}
		break;
		case pod::e_sceneNumMeshes | pod::c_startTagMask:
			read4Bytes(stream, temporaryInt);
			modelInternalData.meshes.resize(temporaryInt);
			break;
		case pod::e_sceneNumNodes | pod::c_startTagMask:
			read4Bytes(stream, temporaryInt);
			modelInternalData.nodes.resize(temporaryInt);
			break;
		case pod::e_sceneNumMeshNodes | pod::c_startTagMask:
			read4Bytes(stream, modelInternalData.numMeshNodes);
			break;
		case pod::e_sceneNumTextures | pod::c_startTagMask:
			read4Bytes(stream, temporaryInt);
			modelInternalData.textures.resize(temporaryInt);
			break;
		case pod::e_sceneNumMaterials | pod::c_startTagMask:
			read4Bytes(stream, temporaryInt);
			modelInternalData.materials.resize(temporaryInt);
			break;
		case pod::e_sceneNumFrames | pod::c_startTagMask:
			read4Bytes(stream, modelInternalData.numFrames);
			break;
		case pod::e_sceneCamera | pod::c_startTagMask:
			readCameraBlock(stream, modelInternalData.cameras[numCameras++]);
			break;
		case pod::e_sceneLight | pod::c_startTagMask:
			readLightBlock(stream, modelInternalData.lights[numLights++]);
			break;
		case pod::e_sceneMesh | pod::c_startTagMask:
			readMeshBlock(stream, modelInternalData.meshes[numMeshes++]);
			break;
		case pod::e_sceneNode | pod::c_startTagMask:
			readNodeBlock(stream, modelInternalData.nodes[numNodes++]);
			break;
		case pod::e_sceneTexture | pod::c_startTagMask:
			readTextureBlock(stream, modelInternalData.textures[numTextures++]);
			break;
		case pod::e_sceneMaterial | pod::c_startTagMask:
			readMaterialBlock(stream, modelInternalData.materials[numMaterials++]);
			break;
		case pod::e_sceneFlags | pod::c_startTagMask:
			read4Bytes(stream, modelInternalData.flags);
			break;
		case pod::e_sceneFPS | pod::c_startTagMask:
			read4Bytes(stream, modelInternalData.FPS);
			break;
		case pod::e_sceneUserData | pod::c_startTagMask:
			readByteArrayIntoVector<uint8_t>(stream, modelInternalData.userData, dataLength);
			break;
		case pod::e_sceneUnits | pod::c_startTagMask:
			read4Bytes(stream, modelInternalData.units);
			break;
		default:
			stream.seek(dataLength, Stream::SeekOriginFromCurrent);
		}
	}
}
} // namespace

namespace pvr {
namespace assets {
PODReader::PODReader() : _modelsToLoad(true) {}

void PODReader::readNextAsset(assets::Model& asset)
{
	uint32_t identifier, dataLength;
	while (readTag(*_assetStream, identifier, dataLength))
	{
		switch (identifier)
		{
		case pod::PODFormatVersion | pod::c_startTagMask:
		{
			// Is the version std::string in the file the same length as ours?
			if (dataLength != pod::c_PODFormatVersionLength)
			{
				throw InvalidDataError("[PODReader::readNextAsset]: File Version Mismatch");
			}
			// ... it is. Check to see if the std::string matches
			char filesVersion[pod::c_PODFormatVersionLength];
			_assetStream->readExact(1, dataLength, &filesVersion[0]);
			if (strcmp(filesVersion, pod::c_PODFormatVersion) != 0)
			{
				throw InvalidDataError("[PODReader::readNextAsset]: File Version Mismatch");
			}
		}
			continue;
		case pod::Scene | pod::c_startTagMask:
			readSceneBlock(*_assetStream, asset);
			asset.initCache();
			return;
		default:
			// Unhandled data, skip it
			_assetStream->seek(dataLength, Stream::SeekOriginFromCurrent);
		}
	}
}
bool PODReader::hasAssetsLeftToLoad()
{
	return _modelsToLoad;
}

bool PODReader::canHaveMultipleAssets()
{
	return false;
}

bool PODReader::isSupportedFile(Stream& assetStream)
{
	if (!assetStream.isopen())
	{
		if (assetStream.isopen() != true)
		{
			return false;
		}
	}
	uint32_t identifier, dataLength;
	size_t dataRead;
	while (readTag(assetStream, identifier, dataLength) == true)
	{
		switch (identifier)
		{
		case pod::PODFormatVersion | pod::c_startTagMask:
		{
			// Is the version std::string in the file the same length as ours?
			if (dataLength != pod::c_PODFormatVersionLength)
			{
				return false;
			}
			// ... it is. Check to see if the std::string matches
			char filesVersion[pod::c_PODFormatVersionLength];
			assetStream.read(1, dataLength, &filesVersion[0], dataRead);
			if (dataRead != dataLength)
			{
				return false;
			}
			if (strcmp(filesVersion, pod::c_PODFormatVersion) != 0)
			{
				return false;
			}
			return true;
		}
		default:
			assetStream.seek(dataLength, Stream::SeekOriginFromCurrent);
			break;
		}
	}
	return false;
}

vector<std::string> PODReader::getSupportedFileExtensions()
{
	vector<std::string> extensions;
	extensions.push_back("pvr");
	return vector<std::string>(extensions);
}

} // namespace assets
} // namespace pvr
//!\endcond
