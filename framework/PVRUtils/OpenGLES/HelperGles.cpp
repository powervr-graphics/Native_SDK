/*!
\brief Contains utility functions to facilitate tasks to create API objects form assets
\file PVRUtils/OpenGLES/HelperGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "HelperGles.h"

namespace pvr {
namespace utils {

void WriteVertexAttributes(uint8_t* srcData, uint8_t* destData, uint32_t nbVertices, uint32_t vertexStride, uint32_t attributeOffset, uint32_t attributeSize)
{
	uint32_t writePosition = attributeOffset;
	uint32_t readPosition = 0;
	for (uint32_t v = 0; v < nbVertices; v++)
	{
		memcpy(destData + writePosition, srcData + readPosition, attributeSize);
		writePosition += vertexStride;
		readPosition += attributeSize;
	}
}

/// <summary>Default constructor.</summary>
VertexStreamDescription::VertexStreamDescription() { this->vertexConfig.topology = pvr::PrimitiveTopology::TriangleList; }

/// <summary>Add vertex stream data type.</summary>
/// <param name="bufferBinding">Binding index for the new vertex description field to be added.</param>
/// <param name="dataType">Data type to be added.</param>
/// <param name="width">How many components the new field has.</param>
/// <param name="name">Name of the field.</param>
/// <param name="semantic">Its index defined through a #define in the shader.</param>
void VertexStreamDescription::Add(uint16_t bufferBinding, pvr::DataType dataType, uint8_t width, const std::string& name, DataSemantic semantic)
{
	if (currentDataOffset.find(bufferBinding) == this->currentDataOffset.end()) currentDataOffset[bufferBinding] = 0;

	uint32_t index = static_cast<uint32_t>(this->vertexConfig.attributes.size());
	this->vertexConfig.addVertexAttribute(bufferBinding, pvr::utils::VertexAttributeInfo(index, dataType, width, currentDataOffset[bufferBinding], name.c_str()));
	this->semantics.push_back(semantic);

	currentDataOffset[bufferBinding] += width * pvr::dataTypeSize(dataType);

	this->vertexConfig.setInputBinding(bufferBinding, currentDataOffset[bufferBinding]);
}

/// <summary>Getter of the vertex configuration.</summary>
/// <returns>Reference to vertexConfig member variable.</returns>
const pvr::utils::VertexConfiguration& VertexStreamDescription::GetVertexConfig() const { return this->vertexConfig; }

/// <summary>Getter to know whether a vertex descriptor has a specific channel.</summary>
/// <returns>True if the channel exists.</returns>
bool VertexStreamDescription::HasChannel(DataSemantic semantic) const
{
	pvr::utils::VertexAttributeInfoWithBinding dummy;
	return this->RetrieveChannelDescription(semantic, dummy);
}

/// <summary>Getter to know whether a vertex descriptor has a channel description.</summary>
/// <returns>True if the channel description exists.</returns>
bool VertexStreamDescription::RetrieveChannelDescription(DataSemantic semantic, pvr::utils::VertexAttributeInfoWithBinding& out) const
{
	for (std::size_t i = 0; i < this->vertexConfig.attributes.size(); i++)
	{
		if (this->semantics[i] == semantic)
		{
			out = this->vertexConfig.attributes[i];
			return true;
		}
	}
	return false;
}

/// <summary>Getter to know the amount of bindings for the current vertex stream description.</summary>
/// <returns>Amount of bindings for the current vertex stream description.</returns>
uint16_t VertexStreamDescription::GetBindingCount() const { return static_cast<uint16_t>(this->vertexConfig.bindings.size()); }

/// <summary>Getter to know the vertex stride for a particular binding.</summary>
/// <returns>Vertex stride for a particular binding.</returns>
uint32_t VertexStreamDescription::GetBindingVertexStride(uint16_t binding) const { return this->currentDataOffset.at(binding); }

/// <summary>Getter to know the semantic for a particular binding.</summary>
/// <returns>Semantic for a particular binding.</returns>
uint16_t VertexStreamDescription::GetSemanticBinding(DataSemantic semantic) const
{
	if (this->semanticBindingLut.find(semantic) == this->semanticBindingLut.end()) { return 0; }
	return this->semanticBindingLut.at(semantic);
}

void writeVertices(uint8_t* input, uint8_t* output, const VertexStreamDescription& description, VertexStreamDescription::DataSemantic semantic, uint32_t nbVertices)
{
	pvr::utils::VertexAttributeInfoWithBinding attributeInfo;
	bool found = description.RetrieveChannelDescription(semantic, attributeInfo);

	WriteVertexAttributes(input, output, nbVertices, description.GetBindingVertexStride(attributeInfo.binding), attributeInfo.offsetInBytes,
		attributeInfo.width * pvr::dataTypeSize(attributeInfo.format));
}

bool RetrieveTexcoords(const pvr::assets::Model::Mesh& mesh, uint32_t texcoordLayer, std::vector<glm::vec2>& uv)
{
	const std::string attributeName = pvr::strings::createFormatted("UV%d", texcoordLayer);

	const uint32_t nbVertices = mesh.getNumVertices();
	uv.resize(nbVertices);

	const pvr::assets::Mesh::VertexAttributeData* originalUvAttribute = mesh.getVertexAttributeByName(attributeName);
	if (originalUvAttribute == nullptr)
	{
		for (uint32_t i = 0; i < nbVertices; i++) { uv[i] = glm::vec2(0.f, 0.f); }
		return false;
	}
	const pvr::StridedBuffer& uvBuffer = mesh.getVertexData(originalUvAttribute->getDataIndex());
	const uint32_t stride = mesh.getStride(originalUvAttribute->getDataIndex());

	for (uint32_t i = 0; i < nbVertices; i++) { memcpy((uint8_t*)glm::value_ptr(uv[i]), uvBuffer.data() + originalUvAttribute->getOffset() + i * stride, sizeof(float) * 2); }

	return true;
}

bool RetrieveColours(const pvr::assets::Model::Mesh& mesh, std::vector<glm::u16vec4>& colours)
{
	const std::string attributeName = "COLOR_0";

	const uint32_t nbVertices = mesh.getNumVertices();
	colours.resize(nbVertices);

	const pvr::assets::Mesh::VertexAttributeData* originalColorAttribute = mesh.getVertexAttributeByName(attributeName);

	// Return some default values
	if (originalColorAttribute == nullptr)
	{
		for (uint32_t i = 0; i < nbVertices; i++) { colours[i] = glm::u16vec4(65535); }
		return false;
	}

	const pvr::StridedBuffer& colorBuffer = mesh.getVertexData(originalColorAttribute->getDataIndex());
	const uint32_t stride = mesh.getStride(originalColorAttribute->getDataIndex());

	if (originalColorAttribute->getVertexLayout().dataType == pvr::DataType::Float32)
	{
		// Convert from float to ui16
		for (uint32_t i = 0; i < nbVertices; i++)
		{
			const uint32_t inputAddress = originalColorAttribute->getOffset() + i * stride;
			glm::vec4 fcolour;
			memcpy((uint8_t*)glm::value_ptr(fcolour), colorBuffer.data() + inputAddress, sizeof(glm::vec4));

			glm::u16vec4 ucolour = glm::u16vec4(fcolour * 65535.f);
			colours[i] = ucolour;
		}
	}
	else
	{
		// No conversion needed, just copy
		for (uint32_t i = 0; i < nbVertices; i++)
		{
			memcpy((uint8_t*)glm::value_ptr(colours[i]), colorBuffer.data() + originalColorAttribute->getOffset() + i * stride, sizeof(uint16_t) * 4);
		}
	}
	return true;
}

bool RetrieveTangents(const pvr::assets::Model::Mesh& mesh, std::vector<glm::vec4>& tangents, bool forceNormalization)
{
	static const std::string& tangentAttributeName = "TANGENT";

	const uint32_t nbVertices = mesh.getNumVertices();
	tangents.resize(nbVertices);

	const pvr::assets::Mesh::VertexAttributeData* originalTangentAttribute = mesh.getVertexAttributeByName(tangentAttributeName);
	if (originalTangentAttribute == nullptr)
	{
		for (uint32_t i = 0; i < nbVertices; i++) { tangents[i] = glm::vec4(1.f, 0.f, 0.f, 1.f); }
		return false;
	}
	const pvr::StridedBuffer& tangentBuffer = mesh.getVertexData(originalTangentAttribute->getDataIndex());
	const uint32_t tangentStride = mesh.getStride(originalTangentAttribute->getDataIndex());
	const uint32_t tangentElements = originalTangentAttribute->getN();

	for (uint32_t i = 0; i < nbVertices; i++)
	{
		glm::vec4 tangent = glm::vec4(1.f, 0.f, 0.f, 1.f);

		memcpy((uint8_t*)glm::value_ptr(tangent), tangentBuffer.data() + originalTangentAttribute->getOffset() + i * tangentStride, sizeof(float) * tangentElements);

		if (forceNormalization)
		{
			glm::vec3 tan3 = glm::vec3(tangent);
			tan3 = glm::normalize(tan3);
			tangent = glm::vec4(tan3, tangent.w);
		}

		tangents[i] = tangent;
	}

	return true;
}

bool RetrieveNormals(const pvr::assets::Model::Mesh& mesh, std::vector<glm::vec3>& normals, bool forceNormalization)
{
	static const std::string& normalAttributeName = "NORMAL";

	const uint32_t nbVertices = mesh.getNumVertices();
	normals.resize(nbVertices);

	const pvr::assets::Mesh::VertexAttributeData* originalNormalAttribute = mesh.getVertexAttributeByName(normalAttributeName);
	if (originalNormalAttribute == nullptr)
	{
		for (uint32_t i = 0; i < nbVertices; i++) { normals[i] = glm::vec3(0.f, 1.f, 0.f); }
		return false;
	}
	const pvr::StridedBuffer& posBuffer = mesh.getVertexData(originalNormalAttribute->getDataIndex());
	const uint32_t stride = mesh.getStride(originalNormalAttribute->getDataIndex());

	for (uint32_t i = 0; i < nbVertices; i++)
	{
		memcpy((uint8_t*)glm::value_ptr(normals[i]), posBuffer.data() + originalNormalAttribute->getOffset() + i * stride, sizeof(float) * 3);
		if (forceNormalization) { normals[i] = glm::normalize(normals[i]); }
	}

	return true;
}

bool RetrievePositions(const pvr::assets::Model::Mesh& mesh, std::vector<glm::vec3>& positions)
{
	static const std::string& positionAttributeName = "POSITION";

	const uint32_t nbVertices = mesh.getNumVertices();
	positions.resize(nbVertices);

	const pvr::assets::Mesh::VertexAttributeData* originalPosAttribute = mesh.getVertexAttributeByName(positionAttributeName);
	if (originalPosAttribute == nullptr) { return false; }
	const pvr::StridedBuffer& posBuffer = mesh.getVertexData(originalPosAttribute->getDataIndex());
	const uint32_t stride = mesh.getStride(originalPosAttribute->getDataIndex());

	for (uint32_t i = 0; i < nbVertices; i++)
	{
		memcpy((uint8_t*)glm::value_ptr(positions[i]), posBuffer.data() + originalPosAttribute->getOffset() + i * stride, sizeof(float) * 3);
	}

	return true;
}

bool RetrieveBoneIndicesAndWeights(
	const pvr::assets::Model::Mesh& mesh, uint32_t bonesPerVertex, pvr::DataType indexType, std::vector<uint8_t>& boneIndices, std::vector<float>& boneWeights)
{
	static const std::string& boneIndexAttributeName = "JOINTS_0";
	static const std::string& boneWeightAttributeName = "WEIGHTS_0";
	boneIndices.clear();
	if (!mesh.getMeshInfo().isSkinned) return false;

	const uint32_t nbVertices = mesh.getNumVertices();

	const pvr::assets::Mesh::VertexAttributeData* originalBoneIndexAttribute = mesh.getVertexAttributeByName(boneIndexAttributeName);
	const pvr::StridedBuffer& boneIndexBuffer = mesh.getVertexData(originalBoneIndexAttribute->getDataIndex());
	const uint32_t boneIndexStride = mesh.getStride(originalBoneIndexAttribute->getDataIndex());
	const pvr::assets::Mesh::VertexAttributeData* originalBoneWeightsAttribute = mesh.getVertexAttributeByName(boneWeightAttributeName);
	const pvr::StridedBuffer& boneWeightBuffer = mesh.getVertexData(originalBoneWeightsAttribute->getDataIndex());
	const uint32_t bonWeightStride = mesh.getStride(originalBoneWeightsAttribute->getDataIndex());

	// Check how many bytes are used for a single bone index
	const pvr::DataType boneIndexType = originalBoneIndexAttribute->getVertexLayout().dataType;
	uint32_t originalBoneIndexSize = pvr::dataTypeSize(boneIndexType);
	uint32_t requestedBoneIndexSize = pvr::dataTypeSize(indexType);
	// Check how many bones are used for a vertex
	const uint32_t originalBonesPerVertex = originalBoneIndexAttribute->getN();
	const uint32_t minBonesPerVertex = glm::min(originalBonesPerVertex, bonesPerVertex);

	boneIndices.resize(requestedBoneIndexSize * bonesPerVertex * nbVertices);
	boneWeights.resize(bonesPerVertex * nbVertices);

	for (uint32_t i = 0; i < nbVertices; i++)
	{
		// Index handling
		const uint32_t outputIndexVertexStart = i * requestedBoneIndexSize * bonesPerVertex;
		const uint32_t inputIndexVertexStart = i * boneIndexStride + originalBoneIndexAttribute->getOffset();
		for (uint32_t b = 0; b < bonesPerVertex; b++)
		{
			// Read the bone index
			uint32_t boneIndex = 0;
			if (b < originalBonesPerVertex) { memcpy((uint8_t*)&boneIndex, boneIndexBuffer.data() + inputIndexVertexStart + originalBoneIndexSize * b, originalBoneIndexSize); }
			// Write the bone index
			memcpy(boneIndices.data() + outputIndexVertexStart + requestedBoneIndexSize * b, &boneIndex, requestedBoneIndexSize);
		}

		// Weight handling
		{
			const uint32_t outputWeightVertexStart = i * bonesPerVertex;
			const uint32_t inputWeightVertexStart = i * bonWeightStride + originalBoneWeightsAttribute->getOffset();

			glm::vec4 weights(0.f);
			// Read the weights
			memcpy((uint8_t*)glm::value_ptr(weights), boneWeightBuffer.data() + inputWeightVertexStart, minBonesPerVertex * sizeof(float));

			// TODO: should we renormalize weights when truncating num bones?

			// Write the weights
			memcpy(boneWeights.data() + outputWeightVertexStart, glm::value_ptr(weights), bonesPerVertex * sizeof(float));
		}
	}

	return true;
}

void convertMeshesData(const VertexStreamDescription& description, std::vector<pvr::assets::Model::Mesh>::iterator meshIter, std::vector<pvr::assets::Model::Mesh>::iterator meshIterEnd)
{
	std::vector<uint8_t> boneIndices;
	std::vector<float> boneWeights;
	std::vector<glm::vec3> positions, normals;
	std::vector<glm::vec4> tangents;
	std::vector<glm::vec2> uv0, uv1;
	std::vector<glm::u16vec4> colours;

	const uint32_t bonesPerVertex = 4; // TODO: probably should be a parameter

	while (meshIter != meshIterEnd)
	{
		const uint32_t nbVertices = meshIter->getNumVertices();

		// Allocate some space for the vertex data
		std::vector<std::vector<uint8_t>> vertexData(description.GetBindingCount());
		for (uint16_t binding = 0; binding < description.GetBindingCount(); binding++) { vertexData[binding].resize(nbVertices * description.GetBindingVertexStride(binding)); }

		// Retrieve the basic mesh data
		{
			if (description.HasChannel(VertexStreamDescription::POSITION))
			{
				RetrievePositions(*meshIter, positions);
				writeVertices((uint8_t*)positions.data(), vertexData[description.GetSemanticBinding(VertexStreamDescription::POSITION)].data(), description,
					VertexStreamDescription::POSITION, nbVertices);
			}
			if (description.HasChannel(VertexStreamDescription::NORMAL))
			{
				RetrieveNormals(*meshIter, normals, true);
				writeVertices((uint8_t*)normals.data(), vertexData[description.GetSemanticBinding(VertexStreamDescription::NORMAL)].data(), description,
					VertexStreamDescription::NORMAL, nbVertices);
			}
			if (description.HasChannel(VertexStreamDescription::TANGENT))
			{
				RetrieveTangents(*meshIter, tangents, true);
				writeVertices((uint8_t*)tangents.data(), vertexData[description.GetSemanticBinding(VertexStreamDescription::TANGENT)].data(), description,
					VertexStreamDescription::TANGENT, nbVertices);
			}
			if (description.HasChannel(VertexStreamDescription::COLOR))
			{
				RetrieveColours(*meshIter, colours);
				writeVertices((uint8_t*)colours.data(), vertexData[description.GetSemanticBinding(VertexStreamDescription::COLOR)].data(), description,
					VertexStreamDescription::COLOR, nbVertices);
			}
			if (description.HasChannel(VertexStreamDescription::UV0))
			{
				RetrieveTexcoords(*meshIter, 0, uv0);
				writeVertices(
					(uint8_t*)uv0.data(), vertexData[description.GetSemanticBinding(VertexStreamDescription::UV0)].data(), description, VertexStreamDescription::UV0, nbVertices);
			}
			if (description.HasChannel(VertexStreamDescription::UV1))
			{
				bool uv1Found = RetrieveTexcoords(*meshIter, 1, uv1);
				writeVertices((uint8_t*)(uv1Found ? uv1.data() : uv0.data()), vertexData[description.GetSemanticBinding(VertexStreamDescription::UV1)].data(), description,
					VertexStreamDescription::UV1, nbVertices);
			}
		}

		// Deal with the skinning data if relevant
		if (meshIter->getMeshInfo().isSkinned)
		{
			if (description.HasChannel(VertexStreamDescription::BONE_WEIGHTS) && description.HasChannel(VertexStreamDescription::BONE_INDICES))
			{
				pvr::utils::VertexAttributeInfoWithBinding indicesDesc;
				description.RetrieveChannelDescription(VertexStreamDescription::BONE_INDICES, indicesDesc);
				// Retrieve and sanitise the existing skinning data
				RetrieveBoneIndicesAndWeights(*meshIter, bonesPerVertex, indicesDesc.format, boneIndices, boneWeights);
				writeVertices((uint8_t*)boneWeights.data(), vertexData[description.GetSemanticBinding(VertexStreamDescription::BONE_WEIGHTS)].data(), description,
					VertexStreamDescription::BONE_WEIGHTS, nbVertices);
				writeVertices((uint8_t*)boneIndices.data(), vertexData[description.GetSemanticBinding(VertexStreamDescription::BONE_INDICES)].data(), description,
					VertexStreamDescription::BONE_INDICES, nbVertices);
			}
		}

		// Build the new mesh
		{
			// Clear the existing vertex data
			meshIter->clearAllData();

			// Add the basic mesh data
			const pvr::utils::VertexConfiguration& vertexConfig = description.GetVertexConfig();
			for (uint16_t binding = 0; binding < description.GetBindingCount(); binding++)
			{
				const uint32_t stride = description.GetBindingVertexStride(binding);
				// Initialise the binding
				meshIter->addData((uint8_t*)vertexData[binding].data(), static_cast<uint32_t>(vertexData[binding].size()), stride, 0);
				meshIter->setStride(binding, stride);
				// Add the individual attributes
				for (std::size_t i = 0; i < vertexConfig.attributes.size(); i++)
				{
					const pvr::utils::VertexAttributeInfoWithBinding& attr = vertexConfig.attributes[i];
					if (attr.binding == binding) { meshIter->addVertexAttribute(attr.attribName, attr.format, attr.width, attr.offsetInBytes, attr.binding); }
				}
			}
		}

		++meshIter;
	}
}

} // namespace utils
} // namespace pvr
//!\endcond
