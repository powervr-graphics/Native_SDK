/*!
\brief Contains utility functions to facilitate tasks to create PVRApi objects form assets
\file PVREngineUtils/AssetUtils.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/EffectApi.h"
#include "PVREngineUtils/StructuredMemory.h"
#include "PVRAssets/Model.h"
#include "PVRAssets/FileIO/PFXParser.h"
#include "PVRAssets/TextureLoad.h"
#include <iterator>

namespace pvr {

/// <summary>Contains functionality (especially free-standing functions) used to facilitate and simplify common tasks,
/// such as automated generation of VBOs for specific meshes, or tying together Effects and Meshes to automate
/// Pipeline creation.</summary>
namespace utils {
/// <summary>Represents a shader Explicit binding, tying a Semantic name to an Attribute Index.</summary>
struct VertexBindings
{
	std::string semanticName;//< effect semantic
	int16 binding;//< binding id
};

/// <summary>Represents a shader Reflective binding, tying a Semantic name to an Attribute variable name.</summary>
struct VertexBindings_Name
{
	StringHash semantic; //< effect semantic
	StringHash variableName;//< shader attribute name
};

/// <summary>Internal function. Retrieve veretex binding info from an effect file to make it easier to digest to the
/// convenience functions.</summary>
/// <param name="effect">The effect from which to retrieve the bindings info</param>
/// <returns>A vector of the bindings that the effect contains.</returns>
inline static std::vector<VertexBindings_Name> getVertexBindingsFromEffect(const assets::Effect& effect)
{
	std::vector<VertexBindings_Name> retval;
	for (std::vector<assets::EffectSemantic>::const_iterator it = effect.attributes.begin(); it != effect.attributes.end(); ++it)
	{
		VertexBindings_Name vn = { it->semantic.c_str(), it->variableName };
		retval.push_back(vn);
	}
	return retval;
}

/// <summary>Use a assets::Mesh object and a assets::Effect object to generate the Input Assembly information for
/// a pipeline object.</summary>
/// <param name="mesh">The mesh object</param>
/// <param name="effect">The effect object</param>
/// <param name="inoutDesc">The GraphicsPipelineCreateParam object. Only the vertexInput and InputAssembler
/// objects are modified.</param>
/// <param name="outNumBuffers">OPTIONAL. Return the number of buffers used by the mesh</param>
/// <remarks>This function allows the user to automatically generate the Input Assembly data that a Pipeline object
/// requires from the semantic info of a Mesh and an Effect. It will corellate the Attribute Semantics provided by
/// the Mesh with the Attribute Semantics required by the Effect, and create suitable input bindings into the
/// provided GraphicsPipelineCreateParam object. The user then does not need to set the Vertex Attributes, Input
/// Bindings and Primitive Topology of this object manually. The rest of the GraphicsPipelineCreateParam must be
/// set normally.</remarks>
inline void createInputAssemblyFromMeshAndEffect(const assets::Mesh& mesh, assets::Effect effect,
    api::GraphicsPipelineCreateParam& inoutDesc, uint16* outNumBuffers = NULL)
{
	std::vector<VertexBindings_Name> bindingMap = getVertexBindingsFromEffect(effect);
	if (outNumBuffers) { *outNumBuffers = 0; }
	int16 current = 0;

	while (current < (int16)bindingMap.size())
	{
		auto attr = mesh.getVertexAttributeByName(bindingMap[current].semantic.c_str());
		if (attr)
		{
			assets::VertexAttributeLayout layout = attr->getVertexLayout();
			uint32 stride = mesh.getStride(attr->getDataIndex());

			if (outNumBuffers) { *outNumBuffers = uint16(std::max<int32>(attr->getDataIndex() + 1, *outNumBuffers)); }
			inoutDesc.vertexInput
			.addVertexAttribute(current, attr->getDataIndex(), layout, bindingMap[current].variableName.c_str())
			.setInputBinding(attr->getDataIndex(), stride, types::StepRate::Vertex);
			inoutDesc.inputAssembler.setPrimitiveTopology(mesh.getMeshInfo().primitiveType);
		}
		else
		{
			Log("Could not find Attribute with Semantic %s in the supplied mesh. It is required for effect '%s' from file '%s'. Will render without binding it, erroneously.",
			    bindingMap[current].semantic.c_str(),
			    effect.material.getEffectName().size() != 0 ? effect.material.getEffectName().c_str() : "UNKNOWN",
			    effect.material.getEffectFile().size() != 0 ? effect.material.getEffectFile().c_str() : "UNKNOWN");
		}
		++current;
	}
	inoutDesc.inputAssembler.setPrimitiveTopology(mesh.getMeshInfo().primitiveType);
}

inline void createInputAssemblyFromMesh(const assets::Mesh& mesh, const VertexBindings* bindingMap, uint16 numBindings,
	api::pipelineCreation::VertexInputCreateParam& vertexCreateParam, api::pipelineCreation::InputAssemblerStateCreateParam& inputAssemblerCreateParam,
	uint16* outNumBuffers = NULL)
{
	vertexCreateParam.clear();
	if (outNumBuffers) { *outNumBuffers = 0; }
	int16 current = 0;
	while (current < numBindings)
	{
		auto attr = mesh.getVertexAttributeByName(bindingMap[current].semanticName.c_str());
		if (attr)
		{
			assets::VertexAttributeLayout layout = attr->getVertexLayout();
			uint32 stride = mesh.getStride(attr->getDataIndex());
			if (outNumBuffers) { *outNumBuffers = uint16(std::max<int32>(attr->getDataIndex() + 1, *outNumBuffers)); }
			vertexCreateParam
				.addVertexAttribute(bindingMap[current].binding, attr->getDataIndex(), layout)
				.setInputBinding(attr->getDataIndex(), stride, types::StepRate::Vertex);
		}
		else
		{
			Log("Could not find Attribute with Semantic %s in the supplied mesh. Will render without binding it, erroneously.",
				bindingMap[current].semanticName.c_str());
		}
		++current;
	}
	inputAssemblerCreateParam.setPrimitiveTopology(mesh.getMeshInfo().primitiveType);
}

inline void createInputAssemblyFromMesh(const assets::Mesh& mesh, const VertexBindings_Name* bindingMap, uint16 numBindings,
	api::pipelineCreation::VertexInputCreateParam& vertexCreateParam, api::pipelineCreation::InputAssemblerStateCreateParam& inputAssemblerCreateParam,
	uint16* outNumBuffers = NULL)
{
	vertexCreateParam.clear();
	if (outNumBuffers) { *outNumBuffers = 0; }
	int16 current = 0;
	//In this scenario, we will be using our own indexes instead of user provided ones, correlating them by names.
	vertexCreateParam.clear();
	while (current < numBindings)
	{
		auto attr = mesh.getVertexAttributeByName(bindingMap[current].semantic);
		if (attr)
		{
			assets::VertexAttributeLayout layout = attr->getVertexLayout();
			uint32 stride = mesh.getStride(attr->getDataIndex());

			if (outNumBuffers) { *outNumBuffers = (uint16)std::max<int32>(attr->getDataIndex() + 1, *outNumBuffers); }
			vertexCreateParam
				.addVertexAttribute(current, attr->getDataIndex(), layout, bindingMap[current].variableName.c_str())
				.setInputBinding(attr->getDataIndex(), stride, types::StepRate::Vertex);
			inputAssemblerCreateParam.setPrimitiveTopology(mesh.getMeshInfo().primitiveType);
		}
		else
		{
			Log("Could not find Attribute with Semantic %s in the supplied mesh. Will render without binding it, erroneously.",
				bindingMap[current].semantic.c_str());
		}
		++current;
	}
}

/// <summary>Use a assets::Mesh object and a list of Attribute Semantics mapping to binding points to generate the
/// Input Assembly into a GraphicsPipelineCreateParam object.</summary>
/// <param name="mesh">The mesh object</param>
/// <param name="bindingMap">A user provided list that maps Attribute Semantic names (strings) to shader Binding
/// Points(ints)</param>
/// <param name="numBindings">number of binding map in array</param>
/// <param name="inoutDesc">The GraphicsPipelineCreateParam object. Only the vertexInput and InputAssembler
/// objects are modified.</param>
/// <param name="outNumBuffers">OPTIONAL. Return the number of buffers used by the mesh</param>
/// <remarks>This function allows the user to automatically generate the Input Assembly data that a Pipeline object
/// requires from the semantic info of a Mesh and an list of explicit bindings. These bindings are usually created
/// by the user, who needs to corellate the Attribute Semantics of the Mesh with the explicit Binding Points of
/// these attributes into a shader. This function will then create suitable input bindings into the provided
/// GraphicsPipelineCreateParam object. The user then does not need to set the Vertex Attributes, Input Bindings
/// and Primitive Topology of this object. The rest of the GraphicsPipelineCreateParam must be set normally.
/// </remarks>
inline void createInputAssemblyFromMesh(const assets::Mesh& mesh, const VertexBindings* bindingMap, uint16 numBindings,
                                        api::GraphicsPipelineCreateParam& inoutDesc,
                                        uint16* outNumBuffers = NULL)
{
	createInputAssemblyFromMesh(mesh, bindingMap, numBindings, inoutDesc.vertexInput, inoutDesc.inputAssembler, outNumBuffers);
}

/// <summary>Use a assets::Mesh object and a list of Attribute Semantics mapping to shader variable names to
/// generate the Input Assembly into a GraphicsPipelineCreateParam object.</summary>
/// <param name="mesh">The mesh</param>
/// <param name="bindingMap">A user-provided list that maps Attribute Semantics (strings) to shader Variable
/// Names(strings)</param>
/// <param name="numBindings">number of bindings in the array</param>
/// <param name="inoutDesc">The GraphicsPipelineCreateParam object. Only the vertexInput and InputAssembler
/// objects are modified.</param>
/// <param name="outNumBuffers">OPTIONAL. Return the number of buffers used by the mesh</param>
/// <remarks>This function allows the user to automatically generate the Input Assembly data that a Pipeline object
/// requires from the semantic info of a Mesh and an list that connects Semantics from the mesh to attribute
/// (input) variables in the vertext shader. These bindings are usually created by the user, who needs to corellate
/// the Attribute Semantics of the Mesh with the shader vertex attributes. This function will then create suitable
/// input bindings into the provided GraphicsPipelineCreateParam object. The user then does not need to set the
/// Vertex Attributes, Input Bindings and Primitive Topology of this object. The rest of the
/// GraphicsPipelineCreateParam must be set normally.</remarks>
inline void createInputAssemblyFromMesh(const assets::Mesh& mesh, const VertexBindings_Name* bindingMap, uint16 numBindings,
                                        api::GraphicsPipelineCreateParam& inoutDesc, uint16* outNumBuffers = NULL)
{
	createInputAssemblyFromMesh(mesh, bindingMap, numBindings, inoutDesc.vertexInput, inoutDesc.inputAssembler, outNumBuffers);
}

/// <summary>Use a assets::Mesh object and a list of Attribute Semantics mapping to binding points to generate the
/// Input Assembly into a VertexRayPipelineCreateParam object.</summary>
/// <param name="mesh">The mesh object</param>
/// <param name="bindingMap">A user provided list that maps Attribute Semantic names (strings) to shader Binding
/// Points(ints)</param>
/// <param name="numBindings">number of binding map in array</param>
/// <param name="inoutDesc">The VertexRayPipelineCreateParam object. Only the vertexInput and InputAssembler
/// objects are modified.</param>
/// <param name="outNumBuffers">OPTIONAL. Return the number of buffers used by the mesh</param>
/// <remarks>This function allows the user to automatically generate the Input Assembly data that a Pipeline object
/// requires from the semantic info of a Mesh and an list of explicit bindings. These bindings are usually created
/// by the user, who needs to corellate the Attribute Semantics of the Mesh with the explicit Binding Points of
/// these attributes into a shader. This function will then create suitable input bindings into the provided
/// VertexRayPipelineCreateParam object. The user then does not need to set the Vertex Attributes, Input Bindings
/// and Primitive Topology of this object. The rest of the VertexRayPipelineCreateParam must be set normally.
/// </remarks>
inline void createInputAssemblyFromMesh(const assets::Mesh& mesh, const VertexBindings* bindingMap, uint16 numBindings,
	api::VertexRayPipelineCreateParam& inoutDesc,
	uint16* outNumBuffers = NULL)
{
	createInputAssemblyFromMesh(mesh, bindingMap, numBindings, inoutDesc.vertexInput, inoutDesc.inputAssembler, outNumBuffers);
}

/// <summary>Use a assets::Mesh object and a list of Attribute Semantics mapping to shader variable names to
/// generate the Input Assembly into a VertexRayPipelineCreateParam object.</summary>
/// <param name="mesh">The mesh</param>
/// <param name="bindingMap">A user-provided list that maps Attribute Semantics (strings) to shader Variable
/// Names(strings)</param>
/// <param name="numBindings">number of bindings in the array</param>
/// <param name="inoutDesc">The VertexRayPipelineCreateParam object. Only the vertexInput and InputAssembler
/// objects are modified.</param>
/// <param name="outNumBuffers">OPTIONAL. Return the number of buffers used by the mesh</param>
/// <remarks>This function allows the user to automatically generate the Input Assembly data that a Pipeline object
/// requires from the semantic info of a Mesh and an list that connects Semantics from the mesh to attribute
/// (input) variables in the vertext shader. These bindings are usually created by the user, who needs to corellate
/// the Attribute Semantics of the Mesh with the shader vertex attributes. This function will then create suitable
/// input bindings into the provided VertexRayPipelineCreateParam object. The user then does not need to set the
/// Vertex Attributes, Input Bindings and Primitive Topology of this object. The rest of the
/// VertexRayPipelineCreateParam must be set normally.</remarks>
inline void createInputAssemblyFromMesh(const assets::Mesh& mesh, const VertexBindings_Name* bindingMap, uint16 numBindings,
                                        api::VertexRayPipelineCreateParam& inoutDesc, uint16* outNumBuffers = NULL)
{
	createInputAssemblyFromMesh(mesh, bindingMap, numBindings, inoutDesc.vertexInput, inoutDesc.inputAssembler, outNumBuffers);
}

/// <summary>Auto generates a single VBO and a single IBO from all the vertex data of a mesh.</summary>
/// <param name="context">The device context where the buffers will be generated on</param>
/// <param name="mesh">The mesh whose data will populate the buffers</param>
/// <param name="outVbo">The VBO handle where the data will be put. No buffer needs to have been created on the
/// handle</param>
/// <param name="outIbo">The IBO handle where the data will be put. No buffer needs to have been created on the
/// handle. If no face data is present on the mesh, the handle will be null.</param>
/// <remarks>This utility function will read all vertex data from a mesh's data elements and create a single VBO.
/// It is commonly used for a single set of interleaved data. If data are not interleaved, they will be packed on
/// the same VBO, each interleaved block (Data element on the mesh) will be appended at the end of the buffer, and
/// the offsets will need to be calculated by the user when binding the buffer.</remarks>
inline void createSingleBuffersFromMesh(GraphicsContext& context, const assets::Mesh& mesh, api::Buffer& outVbo,
                                        api::Buffer& outIbo)
{
	size_t total = 0;
	for (uint32 i = 0; i < mesh.getNumDataElements(); ++i)
	{
		total += mesh.getDataSize(i);
	}

	outVbo = context->createBuffer((uint32)mesh.getDataSize(0), types::BufferBindingUse::VertexBuffer, true);

	size_t current = 0;
	for (uint32 i = 0; i < mesh.getNumDataElements(); ++i)
	{
		outVbo->update((void*)mesh.getData(i), (uint32)current, (uint32)mesh.getDataSize(i));
		current += mesh.getDataSize(i);
	}

	if (mesh.getNumFaces())
	{
		outIbo = context->createBuffer((uint32)mesh.getFaces().getDataSize(), types::BufferBindingUse::IndexBuffer, true);
		outIbo->update((void*)mesh.getFaces().getData(), 0, mesh.getFaces().getDataSize());
	}

	else
	{
		outIbo.reset();
	}
}

/// <summary>Auto generates a set of VBOs and a single IBO from all the vertex data of a mesh.</summary>
/// <param name="context">The device context where the buffers will be generated on</param>
/// <param name="mesh">The mesh whose data will populate the buffers</param>
/// <param name="outVbos">Reference to a std::vector of VBO handles where the data will be put. Buffers will be appended
/// at the end.</param>
/// <param name="outIbo">The IBO handle where the data will be put. No buffer needs to have been created on the
/// handle. If no face data is present on the mesh, the handle will be null.</param>
/// <remarks>This utility function will read all vertex data from the mesh and create one Buffer for each data
/// element (block of interleaved data) in the mesh. It is thus commonly used for for meshes containing multiple
/// sets of interleaved data (for example, a VBO with static and a VBO with streaming data).</remarks>
inline void createMultipleBuffersFromMesh(GraphicsContext& context, const assets::Mesh& mesh, std::vector<api::Buffer>& outVbos,
    api::Buffer& outIbo)
{
	for (uint32 i = 0; i < mesh.getNumDataElements(); ++i)
	{
		outVbos.push_back(context->createBuffer((uint32)mesh.getDataSize(i), types::BufferBindingUse::VertexBuffer, true));
		outVbos.back()->update((void*)mesh.getData(i), 0, (uint32)mesh.getDataSize(0));
	}
	if (mesh.getNumFaces())
	{
		outIbo = context->createBuffer(mesh.getFaces().getDataSize(), types::BufferBindingUse::IndexBuffer, true);
		outIbo->update((void*)mesh.getFaces().getData(), 0, mesh.getFaces().getDataSize());
	}
}

//inline void createSkyBox(GraphicsContext& device, bool adjustUV, uint32 texSize, api::Buffer& outBuffer)
//{
//
//	//float32 vertices[24 * 5];
//	//outBuffer = device.createBuffer(sizeof(vertices), types::BufferBindingUse::VertexBuffer);
//	//float32 unit(1.f);
//	//float32 a0(0.0f), a1(unit);
//	//if (adjustUV)
//	//{
//	//	float32 oneover(1.0f / texSize);
//	//	a0 = 4.0f * oneover;
//	//	a1 = unit - a0;
//	//}
//
//	//// Front
//	//vertices[0] = -unit, vertices[1] = +unit, vertices[2] = -unit;
//	//vertices[3] = a0, vertices[4] = a1;
//
//	//vertices[5] = +unit, vertices[6] = +unit, vertices[7] = -unit;
//	//vertices[8] = a1, vertices[9] = a1;
//
//	//vertices[5] = +unit, vertices[6] = +unit, vertices[7] = -unit;
//	//vertices[8] = a1, vertices[9] = a1;
//
//	//SetVertex(Vertices, 2, -unit, -unit, -unit);
//	//SetUV(UVs, 2, a0, a0);
//
//	//SetVertex(Vertices, 3, +unit, -unit, -unit);
//	//SetUV(UVs, 3, a1, a0);
//
//	//// Right
//	//SetVertex(Vertices, 4, +unit, +unit, -unit);
//	//SetVertex(Vertices, 5, +unit, +unit, +unit);
//	//SetVertex(Vertices, 6, +unit, -unit, -unit);
//	//SetVertex(Vertices, 7, +unit, -unit, +unit);
//	//SetUV(UVs, 4, a0, a1);
//	//SetUV(UVs, 5, a1, a1);
//	//SetUV(UVs, 6, a0, a0);
//	//SetUV(UVs, 7, a1, a0);
//
//	//// Back
//	//SetVertex(Vertices, 8, +unit, +unit, +unit);
//	//SetVertex(Vertices, 9, -unit, +unit, +unit);
//	//SetVertex(Vertices, 10, +unit, -unit, +unit);
//	//SetVertex(Vertices, 11, -unit, -unit, +unit);
//	//SetUV(UVs, 8, a0, a1);
//	//SetUV(UVs, 9, a1, a1);
//	//SetUV(UVs, 10, a0, a0);
//	//SetUV(UVs, 11, a1, a0);
//
//	//// Left
//	//SetVertex(Vertices, 12, -unit, +unit, +unit);
//	//SetVertex(Vertices, 13, -unit, +unit, -unit);
//	//SetVertex(Vertices, 14, -unit, -unit, +unit);
//	//SetVertex(Vertices, 15, -unit, -unit, -unit);
//	//SetUV(UVs, 12, a0, a1);
//	//SetUV(UVs, 13, a1, a1);
//	//SetUV(UVs, 14, a0, a0);
//	//SetUV(UVs, 15, a1, a0);
//
//	//// Top
//	//SetVertex(Vertices, 16, -unit, +unit, +unit);
//	//SetVertex(Vertices, 17, +unit, +unit, +unit);
//	//SetVertex(Vertices, 18, -unit, +unit, -unit);
//	//SetVertex(Vertices, 19, +unit, +unit, -unit);
//	//SetUV(UVs, 16, a0, a1);
//	//SetUV(UVs, 17, a1, a1);
//	//SetUV(UVs, 18, a0, a0);
//	//SetUV(UVs, 19, a1, a0);
//
//	//// Bottom
//	//SetVertex(Vertices, 20, -unit, -unit, -unit);
//	//SetVertex(Vertices, 21, +unit, -unit, -unit);
//	//SetVertex(Vertices, 22, -unit, -unit, +unit);
//	//SetVertex(Vertices, 23, +unit, -unit, +unit);
//	//SetUV(UVs, 20, a0, a1);
//	//SetUV(UVs, 21, a1, a1);
//	//SetUV(UVs, 22, a0, a0);
//	//SetUV(UVs, 23, a1, a0);
//
//	//for (int i = 0; i < 24 * 3; i++) { (*Vertices)[i] = VERTTYPEMUL((*Vertices)[i], f2vt(scale)); }
//}
//


/// <summary>Auto generates a set of VBOs and a set of IBOs from the vertex data of multiple meshes and uses
/// std::inserter provided by the user to insert them to any container.</summary>
/// <param name="context">The device context where the buffers will be generated on</param>
/// <param name="meshIter">Iterator for a collection of meshes.</param>
/// <param name="meshIterEnd">End Iterator for meshIter.</param>
/// <param name="outVbos">std::inserter for a collection of api::Buffer handles. It will be used to insert one VBO per mesh.
/// </param>
/// <param name="outIbos">std::inserter for a collection of api::Buffer handles. It will be used to insert one IBO per mesh.
/// If face data is not present on the mesh, a null handle will be inserted.</param>
/// <remarks>This utility function will read all vertex data from a mesh's data elements and create a single VBO.
/// It is commonly used for a single set of interleaved data (mesh.getNumDataElements() == 1). If more data
/// elements are present (i.e. more than a single interleaved data element) , they will be packed in the sameVBO,
/// with each interleaved block (Data element ) appended at the end of the buffer. It is then the user's
/// responsibility to use the buffer correctly with the API (for example use bindbufferbase and similar) with the
/// correct offsets. The std::inserter this function requires can be created from any container with an insert()
/// function with (for example, for insertion at the end of a vector) std::inserter(std::vector,
/// std::vector::end()) .</remarks>
template<typename MeshIterator_, typename VboInsertIterator_, typename IboInsertIterator_>
inline void createSingleBuffersFromMeshes(
  GraphicsContext& context,
  MeshIterator_ meshIter, MeshIterator_ meshIterEnd,
  VboInsertIterator_ outVbos,
  IboInsertIterator_ outIbos)
{
	int i = 0;
	while (meshIter != meshIterEnd)
	{
		size_t total = 0;
		for (uint32 ii = 0; ii < meshIter->getNumDataElements(); ++ii)
		{
			total += meshIter->getDataSize(ii);
		}

		api::Buffer vbo = context->createBuffer((uint32)total, types::BufferBindingUse::VertexBuffer, true);
		size_t current = 0;
		for (size_t ii = 0; ii < meshIter->getNumDataElements(); ++ii)
		{
			vbo->update((const void*)meshIter->getData(uint32(ii)), (uint32)current,
			            (uint32)meshIter->getDataSize(uint32(ii)));
			current += meshIter->getDataSize((uint32)ii);
		}

		outVbos = vbo;
		if (meshIter->getNumFaces())
		{
			api::Buffer ibo = context->createBuffer(meshIter->getFaces().getDataSize(), types::BufferBindingUse::IndexBuffer, true);
			ibo->update((void*)meshIter->getFaces().getData(), 0,  meshIter->getFaces().getDataSize());
			outIbos = ibo;
		}
		else
		{
			outIbos = api::Buffer();
		}
		++outVbos;
		++outIbos;
		++i;
		++meshIter;
	}
}

/// <summary>Auto generates a set of VBOs and a set of IBOs from the vertex data of multiple meshes and insert them
/// at the specified spot in a user-provided container.</summary>
/// <param name="context">The device context where the buffers will be generated on</param>
/// <param name="meshIter">Iterator for a collection of meshes.</param>
/// <param name="meshIterEnd">End Iterator for meshIter.</param>
/// <param name="outVbos">Collection of api::Buffer handles. It will be used to insert one VBO per mesh.</param>
/// <param name="outIbos">Collection of api::Buffer handles. It will be used to insert one IBO per mesh. If face data is
/// not present on the mesh, a null handle will be inserted.</param>
/// <param name="vbos_where">Iterator on outVbos - the position where the insertion will happen.</param>
/// <param name="ibos_where">Iterator on outIbos - the position where the insertion will happen.</param>
/// <remarks>This utility function will read all vertex data from a mesh's data elements and create a single VBO.
/// It is commonly used for a single set of interleaved data (mesh.getNumDataElements() == 1). If more data
/// elements are present (i.e. more than a single interleaved data element) , they will be packed in the sameVBO,
/// with each interleaved block (Data element ) appended at the end of the buffer. It is then the user's
/// responsibility to use the buffer correctly with the API (for example use bindbufferbase and similar) with the
/// correct offsets.</remarks>
template<typename MeshIterator_, typename VboContainer_, typename IboContainer_>
inline void createSingleBuffersFromMeshes(
  GraphicsContext& context,
  MeshIterator_ meshIter, MeshIterator_ meshIterEnd,
  VboContainer_& outVbos, typename VboContainer_::iterator vbos_where,
  IboContainer_& outIbos, typename IboContainer_::iterator ibos_where)
{
	createSingleBuffersFromMeshes(context, meshIter, meshIterEnd, std::inserter(outVbos, vbos_where), std::inserter(outIbos, ibos_where));
}

/// <summary>Auto generates a set of VBOs and a set of IBOs from the vertex data of the meshes of a model and
/// inserts them into containers provided by the user using std::inserters.</summary>
/// <param name="context">The device context where the buffers will be generated on</param>
/// <param name="model">The model whose meshes will be used to generate the Buffers</param>
/// <param name="vbos">An insert iterator to a std::Buffer container for the VBOs. Vbos will be inserted using
/// this iterator.</param>
/// <param name="ibos">An insert iterator to an std::Buffer container for the IBOs. Ibos will be inserted using
/// this iterator.</param>
/// <remarks>This utility function will read all vertex data from the VBO. It is usually preferred for meshes
/// meshes containing a single set of interleaved data. If multiple data elements (i.e. sets of interleaved data),
/// each block will be successively placed after the other. The std::inserter this function requires can be created
/// from any container with an insert() function with (for example, for insertion at the end of a vector)
/// std::inserter(std::vector, std::vector::end()) .</remarks>
template<typename VboInsertIterator_, typename IboInsertIterator_>
inline void createSingleBuffersFromModel(
  GraphicsContext& context, const assets::Model& model, VboInsertIterator_ vbos, IboInsertIterator_ ibos)
{
	createSingleBuffersFromMeshes(context, model.beginMeshes(), model.endMeshes(), vbos, ibos);
}

/// <summary>Auto generates a set of VBOs and a set of IBOs from the vertex data of the meshes of a model and
/// appends them at the end of containers provided by the user.</summary>
/// <param name="context">The device context where the buffers will be generated on</param>
/// <param name="model">The model whose meshes will be used to generate the Buffers</param>
/// <param name="vbos">A container of api::Buffer handles. The VBOs will be inserted at the end of this
/// container.</param>
/// <param name="ibos">A container of api::Buffer handles. The IBOs will be inserted at the end of this
/// container.</param>
/// <remarks>This utility function will read all vertex data from the VBO. It is usually preferred for meshes
/// meshes containing a single set of interleaved data. If multiple data elements (i.e. sets of interleaved data),
/// each block will be successively placed after the other.</remarks>
template<typename VboContainer_, typename IboContainer_>
inline void appendSingleBuffersFromModel(
  GraphicsContext& context, const assets::Model& model, VboContainer_& vbos, IboContainer_& ibos)
{
	createSingleBuffersFromMeshes(context, model.beginMeshes(), model.endMeshes(),
	                              std::inserter(vbos, vbos.end()), std::inserter(ibos, ibos.end()));
}


inline void create3dPlaneMesh(uint32 width, uint32 length, bool vertexAttribTex, bool vertexAttribNormal,
    assets::Mesh& outMesh)
{
	const float32 halfWidth = width * .5f;
	const float32 halfLength = length * .5f;

	glm::vec3 normal[4]=
	{
		glm::vec3(0.0f,1.0f,0.0f),
		glm::vec3(0.0f,1.0f,0.0f),
		glm::vec3(0.0f,1.0f,0.0f),
		glm::vec3(0.0f,1.0f,0.0f)
	};

	glm::vec2 texCoord[4] =
	{
		glm::vec2(0.0f,1.0f),
		glm::vec2(0.0f,0.0f),
		glm::vec2(1.0f,0.0f),
		glm::vec2(1.0f,1.0f),
	};

	glm::vec3 pos[4] =
	{
		glm::vec3(-halfWidth,0.0f,-halfLength),
		glm::vec3(-halfWidth,0.0f,halfLength),
		glm::vec3(halfWidth,0.0f,halfLength),
		glm::vec3(halfWidth,0.0f,-halfLength)
	};

	uint32 indexData[] =
	{
		0,1,2,
		0,2,3
	};

	pvr::float32 vertData[32];
	uint32 offset = 0 ;

	for(uint32 i = 0; i < 4; ++i)
	{
		memcpy(&vertData[offset], &pos[i], sizeof(pos[i]));
		offset += 3;
		if(vertexAttribNormal)
		{
			memcpy(&vertData[offset], &normal[i], sizeof(normal[i]));
			offset += 3;
		}
		if(vertexAttribTex)
		{
			memcpy(&vertData[offset], &texCoord[i], sizeof(texCoord[i]));
			offset += 2;
		}
	}

	pvr::uint32 stride =
	    sizeof(glm::vec3) +
	    (vertexAttribNormal ? sizeof(glm::vec3) : 0) +
	    (vertexAttribTex ? sizeof(glm::vec2) : 0);

	outMesh.addData((const byte*)vertData,sizeof(vertData),stride, 0);
	outMesh.addFaces((const byte*)indexData, sizeof(indexData), types::IndexType::IndexType32Bit);
	offset = 0;
	outMesh.addVertexAttribute("POSITION", types::DataType::Float32, 3, offset, 0);
	offset += sizeof(float32) * 3;
	if(vertexAttribNormal)
	{
		outMesh.addVertexAttribute("NORMAL", types::DataType::Float32, 3,  offset, 0);
		offset += sizeof(float32) * 2;
	}
	if(vertexAttribTex)
	{
		outMesh.addVertexAttribute("UV0", types::DataType::Float32, 2, offset, 0);
	}
	outMesh.setPrimitiveType(types::PrimitiveTopology::TriangleList);
	outMesh.setStride(0, stride);
	outMesh.setNumFaces(ARRAY_SIZE(indexData) / 3);
	outMesh.setNumVertices(ARRAY_SIZE(pos));
}




}
}
