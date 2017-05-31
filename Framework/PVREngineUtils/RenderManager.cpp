/*!
\brief Contains the implementations for the PowerVR RenderManager and other Rendering helpers
\file PVREngineUtils/RenderManager.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN

#include "PVREngineUtils/RenderManager.h"
#include "PVRApi/ApiUtils.h"
#include <algorithm>
namespace pvr {
namespace utils {

typedef DynamicArray<AttributeLayout> AttributeConfiguration;

struct PipelineSet
{
	std::vector<StringHash> pipelines;
	bool operator<(const PipelineSet& rhs)const
	{
		return std::lexicographical_compare(pipelines.begin(), pipelines.end(), rhs.pipelines.begin(), rhs.pipelines.end());
	}
	PipelineSet() {}
	PipelineSet(std::set<StringHash>& set)
	{
		pipelines.resize((uint32)set.size());
		uint32 count = 0;
		for (auto& setitem : set)
		{
			pipelines[count++] = setitem;
		}
	}
};


////////// PIPELINE SELECTION ///////////
namespace {
inline std::pair<StringHash, effect::PipelineDef*> selectPipelineForSubpassGroupMeshMaterial(
  effect::EffectApi& effect, const effect::SubpassGroup& pipes, const assets::Mesh& mesh,
  const assets::Material& material)
{

	bool incompatible = true;
	auto cond_pipe_it = pipes.pipelines.begin();
	//This loop will break if a compatible pipeline was found.
	for (; cond_pipe_it != pipes.pipelines.end(); ++cond_pipe_it)
	{
		incompatible = false; //Start with the assumption that the pipeline is compatible
		//This loop will break if a compatible pipeline was found.
		for (auto condition = cond_pipe_it->conditions.begin(); condition != cond_pipe_it->conditions.end() && !incompatible; ++condition)
		{
			//This loop will complete if the pipeline is incompatible, and the outer loop will move to the next pipeline.
			//Otherwise, if no incompatible conditions are found, it will break while "incompatible=false", which will also break the outer loop
			//and cond_pipe_it will be the first compatible pipeline.
			switch (condition->type)
			{
			case utils::effect::PipelineCondition::AttributeRequired:
				incompatible = (mesh.getVertexAttributeByName(condition->value) == 0);
				break;
			case utils::effect::PipelineCondition::AttributeRequiredNo:
				incompatible = (mesh.getVertexAttributeByName(condition->value) != 0);
				break;
			case utils::effect::PipelineCondition::UniformRequired:
				incompatible = !material.hasSemantic(condition->value);
				break;
			case utils::effect::PipelineCondition::UniformRequiredNo:
				incompatible = material.hasSemantic(condition->value);
				break;
			}
		}
		if (!incompatible) { break; }
	}

	if (!incompatible) // cond_pipe_it!=pass.subpasses[subpass].pipelines.end()
	{
		effect::PipelineDef* pipeDef =  effect->getPipelineDefinition(cond_pipe_it->pipeline);
		pipeDef->createParam.inputAssembler.setPrimitiveTopology(mesh.getPrimitiveType());
		return std::make_pair(cond_pipe_it->pipeline, pipeDef);
	}


	Log("failed to find a compatible pipeline for a mesh with material %s", material.getName().c_str());
	return std::make_pair(StringHash(), (utils::effect::PipelineDef*)NULL);
}

inline std::pair<StringHash, const effect::PipelineDef*> selectPipelineForSubpassGroupMeshMaterial(
  effect::EffectApi& effect, uint32 passId, uint32 subpassId, uint32 subpassGroupId, const assets::Mesh& mesh, const assets::Material& material)
{
	auto& pass = effect->getPass(passId);
	return selectPipelineForSubpassGroupMeshMaterial(effect, pass.subpasses[subpassId].groups[subpassGroupId], mesh, material);
}
}

////////// ATTRIBUTES AND VBO MANAGEMENT
namespace {
// A reswizzler is the function we will call to read Vertex data with a specific layout from a piece of memory, into another piece of memory, with a different layout.
typedef void(*Reswizzler)(byte* to, byte* from, uint32_t toOffset, uint32_t fromOffset, uint32_t toWidth,
                          uint32_t fromWidth, uint32_t tostride, uint32_t fromstride, uint32_t items);

template<typename Fromtype, typename Totype>
void attribToAttrib(
  byte* to, byte* from, uint32_t toOffset, uint32_t fromOffset, uint32_t toWidth, uint32_t fromWidth,
  uint32_t tostride, uint32_t fromstride, uint32_t items)
{
	uint_fast16_t width = std::min(fromWidth, toWidth);
	for (uint_fast32_t item = 0; item < items; ++item)
	{
		unsigned char* tmpTo = to + toOffset + item * tostride;
		unsigned char* tmpFrom = from + fromOffset + item * fromstride;
		uint32_t vec = 0;
		for (; vec < width; ++vec)
		{
			Fromtype from_value = *reinterpret_cast<Fromtype*>(tmpFrom + vec * sizeof(Fromtype));
			Totype to_value = (Totype)from_value;
			*reinterpret_cast<Totype*>(tmpTo + vec * sizeof(Totype)) = to_value;
		}

		for (; vec < 3 && vec < toWidth; ++vec)
		{
			*reinterpret_cast<Totype*>(tmpTo + vec * sizeof(Totype)) = (Totype)0;
		}
		for (; vec < toWidth; ++vec)
		{
			*reinterpret_cast<Totype*>(tmpTo + vec * sizeof(Totype)) = (Totype)1;
		}
	}
}

struct Remapper { void* to; void* from; uint32_t tooffset, fromoffset, towidth, fromwidth, tostride, fromstride; };

template<typename Fromtype, typename Totype>
inline void attribToAttrib(Remapper remap, size_t numitems)
{
	attribToAttrib<Fromtype, Totype>((unsigned char*)remap.to, (unsigned char*)remap.from,
	                                 remap.tooffset, remap.fromoffset, remap.towidth, remap.fromwidth,
	                                 remap.tostride, remap.fromstride, numitems);
}

Reswizzler selectReswizzler(types::DataType fromType, types::DataType toType)
{
	switch (fromType)
	{
	case types::DataType::Float32:
		switch (toType)
		{
		case types::DataType::Float32:
			return &(attribToAttrib<float32, float32>);
		case types::DataType::Int32:
		case types::DataType::UInt32:
			return &(attribToAttrib<float32, int32>);
		case types::DataType::Int16:
		case types::DataType::UInt16:
			return &(attribToAttrib<float32, int16>);
		case types::DataType::Int8:
		case types::DataType::UInt8:
			return &(attribToAttrib<float32, int8>);

		case types::DataType::Int8Norm:
		case types::DataType::UInt8Norm:
		case types::DataType::Int16Norm:
		case types::DataType::UInt16Norm:
		case types::DataType::Fixed16_16:
			assertion(false, "Unsupported POD Vertex Datatype");
			break;

		}
	case types::DataType::Int32:
	case types::DataType::UInt32:
		switch (toType)
		{
		case types::DataType::Float32:
			return &(attribToAttrib<int32, float32>);
		case types::DataType::Int32:
		case types::DataType::UInt32:
			return &(attribToAttrib<int32, int32>);
		case types::DataType::Int16:
		case types::DataType::UInt16:
			return &(attribToAttrib<int32, int16>);
		case types::DataType::Int8:
		case types::DataType::UInt8:
			return &(attribToAttrib<int32, int8>);
			break;

		case types::DataType::Int8Norm:
		case types::DataType::UInt8Norm:
		case types::DataType::Int16Norm:
		case types::DataType::UInt16Norm:
		case types::DataType::Fixed16_16:
			assertion(false, "Unsupported POD Vertex Datatype");
			break;

		}
	case types::DataType::Int16:
	case types::DataType::UInt16:
		switch (toType)
		{
		case types::DataType::Float32:
			return &(attribToAttrib<int16, float32>);
		case types::DataType::Int32:
		case types::DataType::UInt32:
			return &(attribToAttrib<int16, int32>);
		case types::DataType::Int16:
		case types::DataType::UInt16:
			return &(attribToAttrib<int16, int16>);
		case types::DataType::Int8:
		case types::DataType::UInt8:
			return &(attribToAttrib<int16, int8>);

		case types::DataType::Int8Norm:
		case types::DataType::UInt8Norm:
		case types::DataType::Int16Norm:
		case types::DataType::UInt16Norm:
		case types::DataType::Fixed16_16:
			assertion(false, "Unsupported POD Vertex Datatype");
			break;

		}
	case types::DataType::Int8:
	case types::DataType::UInt8:
		switch (toType)
		{
		case types::DataType::Float32:
			return &(attribToAttrib<int8, float32>);
		case types::DataType::Int32:
		case types::DataType::UInt32:
			return &(attribToAttrib<int8, int32>);
		case types::DataType::Int16:
		case types::DataType::UInt16:
			return &(attribToAttrib<int8, int16>);
		case types::DataType::Int8:
		case types::DataType::UInt8:
			return &(attribToAttrib<int8, int8>);


		case types::DataType::Int8Norm:
		case types::DataType::UInt8Norm:
		case types::DataType::Int16Norm:
		case types::DataType::UInt16Norm:
		case types::DataType::Fixed16_16:
			assertion(false, "Unsupported POD Vertex Datatype");
			break;

		}
		break;

	case types::DataType::Int8Norm:
	case types::DataType::UInt8Norm:
	case types::DataType::Int16Norm:
	case types::DataType::UInt16Norm:
	case types::DataType::Fixed16_16:
		assertion(false, "Unsupported POD Vertex Datatype");
		break;
	}
	return NULL;
}

inline void populateVbos(AttributeConfiguration& attribConfig, pvr::DynamicArray<api::Buffer>& vbos, assets::Mesh& mesh)
{
	Reswizzler reswizzler;
	uint32 numVertices = mesh.getNumVertices();


	std::vector<byte> ptrs[16];
	for (uint32 i = 0; i < vbos.size(); ++i)
	{
		ptrs[i].resize(vbos[i].isNull() ? 0 : vbos[i]->getSize());
	}

	for (uint32 binding = 0; binding < attribConfig.size(); ++binding)
	{
		for (uint32 attribute = 0; attribute < attribConfig[binding].size(); ++attribute)
		{
			auto& attrib = attribConfig[binding][attribute];
			const auto& mattrib = mesh.getVertexAttributeByName(attrib.semantic);

			if (mattrib == NULL) { continue; }
			uint32 mbinding = mattrib->getDataIndex();
			types::DataType mdatatype = mattrib->getVertexLayout().dataType;
			uint32 mwidth = mattrib->getVertexLayout().width;
			byte* mptr = mesh.getData(mbinding);

			byte* ptr = ptrs[binding].data();

			reswizzler = selectReswizzler(mdatatype, attrib.datatype);

			reswizzler(ptr, mptr, attrib.offset, mattrib->getOffset(), attrib.width, mwidth,
			           attribConfig[binding].stride, mesh.getStride(mbinding), numVertices);
		}
	}

	for (uint32 i = 0; i < vbos.size(); ++i)
	{
		if (vbos[i].isValid())
		{
			vbos[i]->update(ptrs[i].data(), 0, vbos[i]->getSize());
		}
	}

}

inline bool createVbos(RenderManager& renderman, const std::map<assets::Mesh*, AttributeConfiguration*>& meshAttribConfig)
{
	GraphicsContext& ctx = renderman.getContext();

	auto& apiModels = renderman.renderModels();

	for (uint32 model_id = 0; model_id < apiModels.size(); ++model_id)
	{
		for (uint32 mesh_id = 0; mesh_id < apiModels[model_id].assetModel->getNumMeshes(); ++mesh_id)
		{
			RendermanMesh& apimesh = apiModels[model_id].meshes[mesh_id];
			assets::Mesh& mesh = *apimesh.assetMesh;

			const auto& found = meshAttribConfig.find(&mesh);
			if (found == meshAttribConfig.end())
			{
				Log("Renderman: Failed to create a vbo for the mesh id %d, model id %d", mesh_id , model_id);
				continue;
			}

			auto& attribConfig = *found->second;

			apimesh.ibo = ctx->createBuffer(mesh.getFaces().getDataSize(), types::BufferBindingUse::IndexBuffer, true);
			apimesh.indexType = mesh.getFaces().getDataType();

			assertion(apimesh.ibo.isValid(), strings::createFormatted(
			            "RenderManager: Could not create IBO for mesh [%d] of model [%d]", mesh_id, model_id));
			apimesh.ibo->update((void*)mesh.getFaces().getData(), 0, mesh.getFaces().getDataSize());

			uint32 size = attribConfig.size();
			size = (mesh.getVertexData().empty() ? 0 : size);// make sure the mesh has a vertex data.
			apimesh.vbos.resize(size);
			for (uint32 vbo_id = 0; vbo_id < size; ++vbo_id)
			{
				if (attribConfig[vbo_id].size() == 0) { continue; } // empty binding
				uint32 size = attribConfig[vbo_id].stride * mesh.getNumVertices();

				apimesh.vbos[vbo_id] = ctx->createBuffer(size, types::BufferBindingUse::VertexBuffer, true);
				populateVbos(attribConfig, apimesh.vbos, mesh);

				assertion(apimesh.vbos[vbo_id].isValid(), strings::createFormatted(
				            "RenderManager: Could not create VBO[%d] for mesh [%d] of model [%d]", mesh_id, model_id));
			}
		}
	}
	return true;
}

inline void addVertexAttributesToVboLayout(std::vector<StringHash>& inner, const std::vector<StringHash>& outer)
{
	for (auto it_outer = outer.begin(); it_outer < outer.end(); ++it_outer)
	{
		bool found = false;
		for (auto it_inner = inner.begin(); !found && it_inner < inner.end(); ++it_inner)
		{
			if (*it_outer == *it_inner)
			{
				found = true;
			}
		}
		if (!found) { inner.push_back(std::move(*it_outer)); }
	}
}

inline std::vector<StringHash> getVertexBindingsForPipeline(const utils::effect::EffectApi& effect, const StringHash& pipelineName)
{
	std::vector<StringHash> retval;
	auto& pipe = effect->getEffectAsset().versionedPipelines.find(effect->getApiString())->second.find(pipelineName)->second;
	for (auto it = pipe.attributes.begin(); it != pipe.attributes.end(); ++it)
	{
		StringHash vn = it->semantic;
		retval.push_back(vn);
	}
	return retval;
}

inline std::vector<StringHash> getAllActiveVertexAttributesForMeshAndEffect(utils::effect::EffectApi& effect,
    const assets::Mesh& mesh, const assets::Material& material)
{
	//TO OPTIMIZE: This function should be called per SUBPASS, not for the whole effect
	std::vector<StringHash> attributes;

	for (uint32 pass = 0; pass < effect->getNumPasses(); ++pass)
	{
		for (uint32 subpass = 0; subpass < effect->getPass(pass).subpasses.size(); ++subpass)
		{
			for (uint32 subpassGroup = 0; subpassGroup < effect->getPass(pass).subpasses[subpass].groups.size(); ++subpassGroup)
			{
				std::pair<StringHash, const effect::PipelineDef*> pipe =
				  selectPipelineForSubpassGroupMeshMaterial(effect, pass, subpass, subpassGroup, mesh, material);
				addVertexAttributesToVboLayout(attributes, getVertexBindingsForPipeline(effect, pipe.first));
			}
		}
	}
	return attributes;
}


inline AttributeConfiguration getVertexBindingsForPipeNoStride(const utils::effect::EffectApi& effect, const StringHash& pipelineName)
{
	AttributeConfiguration retval;
	auto& pipe = effect->getEffectAsset().versionedPipelines.find(effect->getApiString())->second.
	             find(pipelineName)->second;

	//Assuming up to 32 (!!!) VBO bindings...
	for (auto it = pipe.attributes.begin(); it != pipe.attributes.end(); ++it)
	{
		uint16 binding = it->vboBinding;
		if (binding >= retval.size())
		{
			retval.resize(it->vboBinding + 1);
		}
		uint32 width = types::GpuDatatypes::getNumMatrixColumns(it->dataType) * types::GpuDatatypes::getNumVecElements(it->dataType);
		types::DataType datatype = types::DataType::None;
		if (retval[binding].size() <= it->location)
		{
			retval[binding].resize(it->location + 1);
		}
		retval[binding][it->location] = Attribute(it->semantic, datatype, (uint16)width, (uint16)retval[binding].stride, it->variableName);
	}
	return retval;
}

inline AttributeConfiguration getVertexBindingsForPipe(const utils::effect::EffectApi& effect, const StringHash& pipelineName)
{
	AttributeConfiguration retval;
	auto& pipe = effect->getEffectAsset().versionedPipelines.find(effect->getApiString())->second.find(pipelineName)->second;
	uint32 count[32] = {};

	//Assuming up to 32 (!!!) VBO bindings...
	for (auto it = pipe.attributes.begin(); it != pipe.attributes.end(); ++it)
	{
		uint16 binding = it->vboBinding;
		if (binding >= retval.size())
		{
			retval.resize(it->vboBinding + 1);
		}

		uint32 width = types::GpuDatatypes::getNumMatrixColumns(it->dataType) * types::GpuDatatypes::getNumVecElements(it->dataType);
		types::DataType datatype = types::GpuDatatypes::toDataType(it->dataType);
		retval[binding].resize(count[binding] + 1);
		//retval[binding].stride temporarily contains the offset! Since we do packing, the "stride" is the offset of the last one...
		retval[binding][count[binding]++] = Attribute(it->semantic, datatype, (uint16)width, (uint16)retval[binding].stride, it->variableName);
		retval[binding].stride += width * dataTypeSize(datatype); //4 - we only support float32, int32;

	}
	return retval;
}

inline Attribute mergeAttribute(Attribute one, const Attribute& two)
{
	//Assume semantic1 = semantic2
	assertion(one.semantic == two.semantic, "RenderManager: Error processing effects. Attempted to merge attributes with different semantics");

	one.datatype = std::min(one.datatype, two.datatype);
	one.width = std::max(one.width, two.width);
	return one;
}

inline void fixVertexLayoutDatatypes(Attribute& one, const assets::VertexAttributeData& two)
{
	assertion(one.semantic == two.getSemantic(), "RenderManager: Error processing effects. Attempted to merge attributes with different semantics");
	one.datatype = (one.datatype == types::DataType::None ? two.getVertexLayout().dataType : std::min(one.datatype, two.getVertexLayout().dataType));
}

inline void mergeAttributeLayouts(AttributeLayout& inout_inner, AttributeLayout& willBeDestroyed_outer)
{
	std::vector<Attribute> inner(inout_inner.begin(), inout_inner.end());
	uint32 inner_initial_size = (uint32)inner.size(); //No point in checking the ones we just added - skip the end of the list.
	for (auto it_outer = willBeDestroyed_outer.begin(); it_outer < willBeDestroyed_outer.end(); ++it_outer)
	{
		bool found = false;
		for (uint32 inner_idx = 0; !found && inner_idx < inner_initial_size; ++inner_idx)
		{
			auto& it_inner = inner[inner_idx];
			if (it_outer->semantic == it_inner.semantic)
			{
				//Found in the inner list.
				it_inner = mergeAttribute(it_inner, *it_outer);
				found = true;
			}
		}
		if (!found)
		{
			inner.push_back(std::move(*it_outer)); //destructive >:D
		}
	}
	inout_inner.assign(inner.begin(), inner.end());
}

inline void calcOffsetsAndStride(AttributeConfiguration& config)
{

	//Assuming up to 32 (!!!) VBO bindings...
	for (auto& layout : config)
	{
		layout.stride = 0;
		for (auto& attrib : layout)
		{
			attrib.offset = (uint16)layout.stride;
			layout.stride += attrib.width * dataTypeSize(attrib.datatype);
		}
	}
}

inline void createAttributeConfigurations(RenderManager& renderman, std::map<PipelineSet, AttributeConfiguration>& pipeSets,
    std::map<StringHash, AttributeConfiguration*>& pipeToAttribMapping, std::map<assets::Mesh*, AttributeConfiguration*>& meshAttribConfig,
    bool datatypesFromModel)
{
	RendermanStructure& renderstruct = renderman.renderObjects();
	for (auto && renderman_effect : renderstruct.effects)
	{
		auto&& effect = renderman_effect.effect;
		// A pipeline combination
		for (auto pipeset = pipeSets.begin(); pipeset != pipeSets.end(); ++pipeset)
		{
			// A pipeline combination
			AttributeConfiguration& finalLayout = pipeset->second; //DO NOT CLEAR! It may already contain attributes from another run...

			for (auto pipe = pipeset->first.pipelines.begin(); pipe != pipeset->first.pipelines.end(); ++pipe)
			{
				auto pipe2bindings = getVertexBindingsForPipeNoStride(effect, *pipe);

				for (uint32 binding = 0; binding < pipe2bindings.size(); ++binding)
					if (pipe2bindings.size() >= binding)
					{
						if (binding >= finalLayout.size())
						{
							finalLayout.resize((uint32)binding + 1);
						}

						mergeAttributeLayouts(finalLayout[binding], pipe2bindings[binding]);
					}

				//Make sure the pipeline knows where to find its attribute
				pipeToAttribMapping[*pipe] = &pipeset->second;
			}
		}

		if (datatypesFromModel)
		{
			// Now, fix the attribute configurations by selecting the widest datatype provided by any of the models.
			auto& apiModels = renderman.renderModels();

			for (uint32 model_id = 0; model_id < apiModels.size(); ++model_id)
			{
				for (uint32 mesh_id = 0; mesh_id < apiModels[model_id].assetModel->getNumMeshes(); ++mesh_id)
				{
					assets::Mesh& mesh = *apiModels[model_id].meshes[mesh_id].assetMesh;
					const auto& found = meshAttribConfig.find(&mesh);
					if (found == meshAttribConfig.end()) { continue; }
					auto& attribConfig = *found->second;

					for (uint32 binding = 0; binding < attribConfig.size(); ++binding)
					{
						for (uint32 attribute = 0; attribute < attribConfig[binding].size(); ++attribute)
						{
							auto& attrib = attribConfig[binding][attribute];
							const auto mattrib = mesh.getVertexAttributeByName(attrib.semantic);

							if (mattrib == NULL) { continue; }
							else
							{
								fixVertexLayoutDatatypes(attrib, *mattrib);
							}

						}

					}
				}
			}
		}

		// ALL DONE - Fix the offsets and strides...
		for (auto pipeset = pipeSets.begin(); pipeset != pipeSets.end(); ++pipeset)
		{
			AttributeConfiguration& finalLayout = pipeset->second; //DO NOT CLEAR! It may already contain attributes from another run...
			for (auto binding = finalLayout.begin(); binding != finalLayout.end(); ++binding)
			{
				binding->stride = 0;
				for (auto vertex = binding->begin(); vertex != binding->end(); ++vertex)
				{
					vertex->offset = (uint16)binding->stride;
					binding->stride += types::dataTypeSize(vertex->datatype) * vertex->width;
				}
			}
		}
	}
}
}


////////// SEMANTICS - BUFFER ENTRIES - UNIFORMS ////////////
namespace {
inline void getUniformLocationsForPipeline(RendermanPipeline& pipeline)
{
	std::vector<decltype(pipeline.uniformSemantics)::iterator> erase_me;

	for (auto it_uniform = pipeline.uniformSemantics.begin(); it_uniform != pipeline.uniformSemantics.end(); ++it_uniform)
	{
		it_uniform->second.uniformLocation = pipeline.apiPipeline->getUniformLocation(it_uniform->second.variablename.c_str());
		if (it_uniform->second.uniformLocation == -1) { erase_me.push_back(it_uniform); }
	}
	for (auto& uniform_it : erase_me) //delete useless uniforms...
	{
		pipeline.uniformSemantics.erase(uniform_it);
	}
}

inline void getUniformLocationsForNodes(RendermanSubpassGroup& subpassGroup)
{
	for (auto& model : subpassGroup.subpassGroupModels)
	{
		for (auto& node : model.nodes)
		{
			std::vector<decltype(node.uniformSemantics)::iterator> erase_me;
			auto& apipipe = node.pipelineMaterial_->pipeline_->apiPipeline;
			for (auto it_uniform = node.uniformSemantics.begin(); it_uniform != node.uniformSemantics.end(); ++it_uniform)
			{
				it_uniform->second.uniformLocation = apipipe->getUniformLocation(it_uniform->second.variablename.c_str());
				if (it_uniform->second.uniformLocation == -1) { erase_me.push_back(it_uniform); }
			}
			for (auto& uniform_it : erase_me) //delete useless uniforms...
			{
				node.uniformSemantics.erase(uniform_it);
			}
		}
	}
}

inline void addSemanticLists(RendermanBufferBinding& buff, std::map<StringHash, StructuredMemoryView*>& bufferDefinitions,
                             std::map<StringHash, BufferEntrySemantic>& bufferEntries, bool checkDuplicates)
{
	if (!buff.semantic.empty())
	{
		auto it = bufferDefinitions.find(buff.semantic);
		if (checkDuplicates && it != bufferDefinitions.end())
		{
			debug_assertion(false, strings::createFormatted("DUPLICATE BUFFER SEMANTIC DETECTED: Buff: [%s] Semantic [%s]",
			                buff.bufferDefinition->name.c_str(), buff.semantic.c_str()).c_str());
		}
		if (it == bufferDefinitions.end())
		{
			bufferDefinitions[buff.semantic] = &buff.bufferDefinition->buffer;
		}
	}
	auto& bufferView = buff.bufferDefinition->buffer;
	auto& list = bufferView.getVariableList();
	for (size_t i = 0; i < list.size(); ++i)
	{
		if (list[i].getName().empty()) { continue; }
		auto it = bufferEntries.find(list[i].getName());
		if (checkDuplicates && it != bufferEntries.end())
		{
			debug_assertion(false, strings::createFormatted("DUPLICATE BUFFER ENTRY SEMANTIC DETECTED: Buff: [%s] Entry Semantic[%s]",
			                buff.bufferDefinition->name.c_str(), it->first.c_str()));
		}
		// add the buffer entry if not exist
		if (it == bufferEntries.end())
		{
			auto& newSemanticEntry = bufferEntries[list[i].getName()];
			newSemanticEntry.buffer = &buff.bufferDefinition->buffer;
			newSemanticEntry.entryIndex = (uint16)i;
			newSemanticEntry.setId = buff.set;
		}
	}
}

inline void addUniformSemanticLists(pvr::ContiguousMap<StringHash, utils::effect::UniformSemantic>& effectlist,
                                    pvr::ContiguousMap<StringHash, UniformSemantic>& newlist, bool checkDuplicates,
                                    types::VariableScope scope)
{
	for (auto& uniform : effectlist)
	{
		if (uniform.second.scope == scope)
		{
			auto& newUniform = newlist[uniform.first];
			newUniform.uniformLocation = uniform.second.arrayElements;
			newUniform.variablename = uniform.second.variableName;
			newUniform.memory.allocate(uniform.second.dataType, uniform.second.arrayElements);
			//memset(newUniform.memory.raw(), 0, types::GpuDatatypes::getSize(newUniform.memory.getDataType(), uniform.second.arrayElements));
		}
	}
}
}


/////////  PIPELINES /////////////
inline bool createPipelines(RenderManager& renderman, const std::map<StringHash, AttributeConfiguration*>& vertexConfigs)
{
	std::map<StringHash, api::GraphicsPipeline> pipelineApis;
	bool succeed = true;

	RendermanStructure& renderstruct = renderman.renderObjects();
	for (auto && renderman_effect : renderstruct.effects)
	{
		auto&& effect = renderman_effect.effect;
		//Here we fix the input assembly based on the collected data.
		for (auto pipeline = vertexConfigs.begin(); pipeline != vertexConfigs.end(); ++pipeline)
		{
			// per-pipe config
			auto pipedef = effect->getPipelineDefinition(pipeline->first);
			if (pipedef == NULL) { continue; }
			// COPY
			api::GraphicsPipelineCreateParam pipecp = pipedef->createParam;

			//Each VBO
			auto& attributeConfig = *pipeline->second;
			if (pipedef->attributes.size())
			{
				for (uint16 binding = 0; binding < attributeConfig.size(); ++binding)
				{
					auto& vbo = attributeConfig[binding];
					if (vbo.size() == 0) { continue; } //Empty VBO binding
					const api::VertexInputBindingInfo* inputBindingInfo = pipecp.vertexInput.getInputBinding(binding);
					pipecp.vertexInput.setInputBinding(binding, (uint16)vbo.stride,
					                                   (inputBindingInfo ? inputBindingInfo->stepRate : types::StepRate::Vertex));
					for (uint16 vertexId = 0; vertexId < vbo.size(); ++vertexId)
					{
						for (uint32 i = 0; i < pipedef->attributes.size(); ++i) // make sure it matches with the pipeline attribute
						{
							auto& vertex = vbo[vertexId];
							if (pipedef->attributes[i].semantic == vertex.semantic && vertexId == pipedef->attributes[i].location &&
							    binding == pipedef->attributes[i].vboBinding)
							{
								assets::VertexAttributeLayout nfo;
								nfo.dataType = vertex.datatype;
								nfo.offset = vertex.offset;
								nfo.width = (uint8)vertex.width;

								pipecp.vertexInput.addVertexAttribute(vertexId, binding, nfo, vertex.variableName.c_str());
							}
						}
					}
				}
			}

			api::GraphicsPipeline pipelineApi = effect->getContext()->createGraphicsPipeline(pipecp);

			if (pipelineApi.isNull())
			{
				Log("RenderManager: Failed to create graphics pipeline %s", pipeline->first.str().c_str());
				succeed = false;
			}

			assertion(pipelineApis.find(pipeline->first) == pipelineApis.end() ||
			          pipelineApis.find(pipeline->first)->second.isNull());

			pipelineApis[pipeline->first] = pipelineApi;
		}
	}

	//Map the newly created pipelines to the Rendering Structure
	//Now that the pipelines are created, we can set the Uniform Locations

	for (auto && renderman_effect : renderstruct.effects)
	{
		for (auto && pass_effect : renderman_effect.passes)
		{
			for (auto && subpass_effect : pass_effect.subpasses)
			{
				for (auto && subpassGroup_effect : subpass_effect.groups)
				{
					for (auto && pipeline_effect : subpassGroup_effect.pipelines)
					{
						pipeline_effect.apiPipeline = pipelineApis.find(pipeline_effect.name)->second;
						//Fix the per-pipeline uniform locations
						getUniformLocationsForPipeline(pipeline_effect);
					}
					//Fix the per-node uniform locations
					getUniformLocationsForNodes(subpassGroup_effect);
				}

			}
		}
	}
	return succeed;
}

inline bool createDescriptorSetsAndBuffers(RenderManager& renderman,
    const std::map<assets::Mesh*, AttributeConfiguration*>& meshAttribConfig, api::DescriptorPool& pool)
{
	GraphicsContext& ctx = renderman.getContext();
	RendermanStructure& renderstruct = renderman.renderObjects();
	uint16 log_effect = 0;
	for (auto& renderman_effect : renderstruct.effects)
	{
		auto& effect = renderman_effect.effect;

		for (RendermanBufferDefinition& bufdef : renderman_effect.bufferDefinitions) //The buffer is created per model.
		{
			// Skip if another pipeline has created them
			assertion(bufdef.buffer.getVariableList().size() != 0, strings::createFormatted(
			            "RenderManager::createAll() Creating descriptor sets : Buffer entry list for buffer [%d] was empty", bufdef.name.c_str()));


			bufdef.buffer.finalize(ctx, std::max<uint32>(1, bufdef.numDynamicClients), bufdef.allSupportedBindings, bufdef.isDynamic);

			for (uint32 swapIdx = 0; swapIdx < bufdef.numBuffers; ++swapIdx)
			{
				bufdef.buffer.createConnectedBuffer(swapIdx, ctx);
			}
		}
		uint16 log_pass = 0;
		for (auto& pass : renderman_effect.passes)
		{
			uint16 log_subpass = 0;
			for (auto& subpass : pass.subpasses)
			{
				for (auto& subpassGroup : subpass.groups)
				{
					// ROUND 2: Create and update the descriptor sets.
					uint16 log_model = 0;
					uint16 log_material = 0;
					for (auto& modeleffect : subpassGroup.subpassGroupModels)
					{
						for (auto& materialeffect : modeleffect.materialEffects)
						{
							for (auto& materialpipeline : materialeffect.materialSubpassPipelines)
							{
								if (materialpipeline.pipeline_ == NULL || !materialpipeline.pipeline_->apiPipeline.isValid())
								{
									continue;
								}
								auto& pipeline = *materialpipeline.pipeline_;
								auto& pipelayout = pipeline.apiPipeline->getPipelineLayout();
								auto& pipedef = *pipeline.pipelineInfo;
								int16 set_max = -1;
								for (auto& item : pipedef.textureSamplersByTexName) { set_max = std::max((int16)item.second.set, set_max); }
								for (auto& item : pipedef.textureSamplersByTexSemantic) { set_max = std::max((int16)item.second.set, set_max); }
								for (auto& item : pipedef.modelScopeBuffers) { set_max = std::max((int16)item.second.set, set_max); }
								for (auto& item : pipedef.effectScopeBuffers) { set_max = std::max((int16)item.second.set, set_max); }
								for (auto& item : pipedef.nodeScopeBuffers) { set_max = std::max((int16)item.second.set, set_max); }
								for (auto& item : pipedef.inputAttachments[0]) { set_max = std::max((int16)item.second.set, set_max); }
								for (uint16 set_id = 0; set_id < set_max + 1; ++set_id)
								{
									if (pipedef.descSetExists[set_id])
									{
										if (pipedef.descSetIsFixed[set_id]) // Descriptor set is "fixed", meaning it is shared by all renderables in the pipeline.
										{
											materialpipeline.sets[set_id] = pipedef.fixedDescSet[set_id];
										}
										else //Otherwise, create the descriptor set for this pipeline/material combination
										{
											const api::DescriptorSetLayout& descsetlayout = pipelayout->getDescriptorSetLayout(set_id);
											debug_assertion(descsetlayout.isValid(), strings::createFormatted(
											                  "RenderManager::createAll() Creating descriptor sets: Descriptor set layout was referenced but was NULL: "
											                  "Pipeline[%s] Set[%d]   found in... Effect[%d] Pass[%d] Spass[%d] Model[%d] Material[%d] .",
											                  pipeline.name.c_str(), set_id, log_effect, log_pass, log_subpass, log_model, log_material));

											uint32 swaplength = (pipedef.descSetIsMultibuffered[set_id] ? ctx->getSwapChainLength() : 1);
											for (uint32 swapchain = 0; swapchain < swaplength; ++swapchain)
											{
												materialpipeline.sets[set_id][swapchain] = pool->allocateDescriptorSet(descsetlayout);
											}
										}
									}
								}

								Multi<api::DescriptorSetUpdate> updates[4];


								// POPULATE THE INPUT  ATTCHMENTS
								for (uint32 swapindex = 0; swapindex < ctx->getSwapChainLength(); ++swapindex)
								{
									for (auto& inputEntry : pipedef.inputAttachments[swapindex])
									{
										const effect::InputAttachmentInfo& input = inputEntry.second;
										updates[input.set][swapindex].setInputImageAttachment(input.binding, input.tex);
									}
								}


								// POPULATE THE TEXTURES
								for (auto& tex : pipedef.textureSamplersByTexSemantic)
								{
									// Get texture name by texture semantic
									// Load the texture -> caching/unduplication
									// Add it to the set.
									// IGNORE UNDUPLICATION FOR NOW
									if (pipedef.descSetIsFixed[tex.second.set])
									{
										debug_assertion(!pipedef.descSetIsFixed[tex.second.set], strings::createFormatted(
										                  "RenderManager::createAll() Creating descriptor sets: Descriptor set "
										                  "was referenced but a semantic, but was marked FIXED Effect[%s] Pass[%d] "
										                  "Spass[%d] Pipeline[%s] Model[%d] Material[%d] Set[%d] TextureSemantic[%s].",
										                  log_effect, log_pass, log_subpass, pipeline.name.c_str(), log_model,
										                  log_material, tex.second.set, tex.first.c_str()));
									}
									int32 texIndex;
									if ((texIndex = materialeffect.material->assetMaterial->getTextureIndex(tex.first)) != -1)
									{
										const StringHash& texturePath = modeleffect.renderModel_->assetModel->getTexture(texIndex).getName();

										api::TextureView view;
										effect->getAssetLoadingDelegate()->effectOnLoadTexture(texturePath, view);

										uint32 swaplength = pipedef.descSetIsMultibuffered[tex.second.set] ? ctx->getSwapChainLength() : 1;

										for (uint32 swapindex = 0; swapindex < swaplength; ++swapindex)
										{
											updates[tex.second.set][swapindex].setCombinedImageSampler(tex.second.binding,
											    view, pipedef.textureSamplersByTexSemantic.find(tex.first)->second.sampler);
										}
									}
									else
									{
										Log(Log.Information, "RenderManager: Texture semantic [%s] was not found in model material [%s]. "
										    "The texture will need to be populated by the application", tex.first.c_str(),
										    materialeffect.material->assetMaterial->getName().c_str());
									}
								}

								// POPULATE THE BUFFERS
								for (auto& bufentry : pipeline.bufferBindings)
								{
									auto& buf = bufentry.second;
									auto& bufdef = *buf.bufferDefinition;
									if (bufdef.buffer.getVariableList().size() == 0)
									{
										assertion(false, strings::createFormatted(
										            "RenderManager::createAll() Creating descriptor sets : Buffer entry list for"
										            " buffer [%s] was empty", bufdef.name.c_str()));
									}
									uint32 swaplength = pipedef.descSetIsMultibuffered[buf.set] ? ctx->getSwapChainLength() : 1;
									for (uint32 swapindex = 0; swapindex < swaplength; ++swapindex)
									{
										switch (buf.type)
										{
										case types::DescriptorType::UniformBuffer:
											updates[buf.set][swapindex].setUbo(
											  buf.binding, bufdef.buffer.getConnectedBuffer(swapindex % bufdef.numBuffers)); break;

										case types::DescriptorType::UniformBufferDynamic:
											updates[buf.set][swapindex].setDynamicUbo(
											  buf.binding, bufdef.buffer.getConnectedBuffer(swapindex % bufdef.numBuffers)); break;

										case types::DescriptorType::StorageBuffer:
											updates[buf.set][swapindex].setSsbo(
											  buf.binding, bufdef.buffer.getConnectedBuffer(swapindex % bufdef.numBuffers)); break;

										case types::DescriptorType::StorageBufferDynamic:
											updates[buf.set][swapindex].setDynamicSsbo(
											  buf.binding, bufdef.buffer.getConnectedBuffer(swapindex % bufdef.numBuffers)); break;
										default:
											debug_assertion(false, "Invalid buffer type");
										}
									}
								}

								for (uint32 setid = 0; setid < 4; ++setid)
								{
									if (pipedef.descSetExists[setid])
									{
										int32 swaplength = ctx->getSwapChainLength();
										assertion(materialpipeline.sets[setid].size() > 0);

										// not be multibufferered with the number of sets less than the swapchain length
										assertion(!(pipedef.descSetIsMultibuffered[setid] && materialpipeline.sets[setid].size() < (uint32)swaplength));

										assertion(pipedef.descSetIsMultibuffered[setid] || materialpipeline.sets[setid].size() == 1);
										swaplength = pipedef.descSetIsMultibuffered[setid] ? swaplength : 1;
										if (materialpipeline.sets[setid].size())
										{
											for (uint32 swapindex = 0; swapindex < (uint32)swaplength; ++swapindex)
											{
												materialpipeline.sets[setid][swapindex]->update(updates[setid][swapindex]);
											}
										}
									}
								}
							}
						}
					}

				}

			}
		}
	}
	return true;
}

/////// FUNCTIONS TO ADD OBJECTS TO THE RENDER DATA STRUCTURE /////////
namespace {
inline RendermanBufferBinding makeRendermanBufferBinding(const StringHash& name, const utils::effect::BufferRef& ref,
    std::deque<RendermanBufferDefinition>& definitions)
{
	RendermanBufferBinding bufferbinding;
	bufferbinding.binding = ref.binding;
	bufferbinding.set = ref.set;
	bufferbinding.type = ref.type;
	bufferbinding.semantic = ref.semantic;

	struct BufferNameEquals
	{
		const StringHash& name;
		BufferNameEquals(const StringHash& name) : name(name) { }
		bool operator()(const RendermanBufferDefinition& rhs)const { return name == rhs.name; }
	};

	bufferbinding.bufferDefinition = &*std::find_if(definitions.begin(), definitions.end(), BufferNameEquals(name));
	return bufferbinding;
}

size_t addRendermanPipelineIfNotExists(RendermanSubpassGroup& subpassGroup, effect::PipelineDef* pipedef,
                                       RendermanEffect& renderEffect, bool& isnew)
{
	//Comparator for the next find
	struct cmppipe
	{
		utils::effect::PipelineDef* ptr;
		cmppipe(utils::effect::PipelineDef* ptr) : ptr(ptr) {}
		bool operator()(const RendermanPipeline& rhs) { return rhs.pipelineInfo == ptr; }
	};

	auto& container = subpassGroup.pipelines;

	auto found_pipe = std::find_if(container.begin(), container.end(), cmppipe(pipedef));
	size_t pipe_index = 0;
	if (found_pipe != container.end())
	{
		pipe_index = found_pipe - container.begin();
		isnew = false;
	}
	else
	{
		isnew = true;
		pipe_index = container.size();

		container.resize(container.size() + 1);
		RendermanPipeline& newEntry = container.back();
		newEntry.subpassGroup_ = &subpassGroup;
		newEntry.pipelineInfo = pipedef;

		addUniformSemanticLists(pipedef->uniforms, newEntry.uniformSemantics, true, types::VariableScope::Effect);
		addUniformSemanticLists(pipedef->uniforms, newEntry.uniformSemantics, true, types::VariableScope::Model);

		// Bomes are in Model Scopebuffers as they are global in .pod.
		for (auto& buffer : pipedef->modelScopeBuffers)
		{
			auto& tmp = newEntry.bufferBindings[buffer.first];
			tmp = makeRendermanBufferBinding(buffer.first, buffer.second, renderEffect.bufferDefinitions);

			addSemanticLists(tmp, newEntry.bufferSemantics, newEntry.bufferEntrySemantics, true);
		}
		for (auto& buffer : pipedef->nodeScopeBuffers)
		{
			auto& tmp = newEntry.bufferBindings[buffer.first];
			tmp = makeRendermanBufferBinding(buffer.first, buffer.second, renderEffect.bufferDefinitions);
			addSemanticLists(tmp, newEntry.bufferSemantics, newEntry.bufferEntrySemantics, true);
		}
		for (auto& buffer : pipedef->batchScopeBuffers)
		{
			auto& tmp = newEntry.bufferBindings[buffer.first];
			tmp = makeRendermanBufferBinding(buffer.first, buffer.second, renderEffect.bufferDefinitions);
			addSemanticLists(tmp, newEntry.bufferSemantics, newEntry.bufferEntrySemantics, true);
		}
		// Effects are added to the model
		for (auto& buffer : pipedef->effectScopeBuffers)
		{
			auto& tmp = newEntry.bufferBindings[buffer.first];
			//if (buffer.second.)
			tmp = makeRendermanBufferBinding(buffer.first, buffer.second, renderEffect.bufferDefinitions);
			//IT is expected to have duplicates, as multiple pipelines may attempt to add the same buffer,
			//in which case we must avoid duplication.
			addSemanticLists(tmp, renderEffect.bufferSemantics, renderEffect.bufferEntrySemantics, false);
		}

		uint32 current_buffer[4] = { 0, 0, 0, 0 };
		for (auto buffer_it = newEntry.bufferBindings.begin(); buffer_it != newEntry.bufferBindings.end(); ++buffer_it)
		{
			auto& buffer = buffer_it->second;
			if (buffer.bufferDefinition->scope == types::VariableScope::Node || buffer.bufferDefinition->scope == types::VariableScope::BoneBatch)
			{
				newEntry.bufferBindings.find(buffer_it->first)->second.node_dynamic_offset_address = buffer.node_dynamic_offset_address = (int16)current_buffer[buffer.set]++;
			}
			else
			{
				newEntry.bufferBindings.find(buffer_it->first)->second.node_dynamic_offset_address = buffer.node_dynamic_offset_address = (int16) - 1;
			}
		}
	}
	return pipe_index;
}

template<typename Container>
inline size_t addRendermanMaterialEffectIfNotExists(Container& model, RendermanMaterial* material, bool& isnew)
{
	//Comparator for the next find
	struct cmppipe
	{
		RendermanMaterial* ptr;
		cmppipe(RendermanMaterial* ptr) : ptr(ptr) {}
		bool operator()(const RendermanSubpassMaterial& rhs) { return rhs.material == ptr; }
	};

	auto found_item = std::find_if(model.materialEffects.begin(), model.materialEffects.end(), cmppipe(material));
	size_t item_index = 0;
	if (found_item != model.materialEffects.end())
	{
		isnew = false;
		item_index = found_item - model.materialEffects.begin();
	}
	else
	{
		isnew = true;
		item_index = model.materialEffects.size();
		RendermanSubpassMaterial newEntry;
		newEntry.material = material;
		newEntry.modelSubpass_ = &model;
		model.materialEffects.push_back(newEntry);
	}
	return item_index;
}

template<typename Container>
inline size_t addRendermanMeshEffectIfNotExists(Container& model, RendermanMesh* mesh, bool& isnew)
{
	//Comparator for the next find
	struct cmppipe
	{
		RendermanMesh* ptr;
		cmppipe(RendermanMesh* ptr) : ptr(ptr) {}
		bool operator()(const RendermanSubpassMesh& rhs) { return rhs.rendermesh_ == ptr; }
	};

	auto found_item = std::find_if(model.subpassMeshes.begin(), model.subpassMeshes.end(), cmppipe(mesh));
	size_t item_index = 0;
	if (found_item != model.subpassMeshes.end())
	{
		isnew = false;
		item_index = found_item - model.subpassMeshes.begin();
	}
	else
	{
		isnew = true;
		item_index = model.subpassMeshes.size();
		RendermanSubpassMesh newEntry;
		newEntry.rendermesh_ = mesh;
		newEntry.modelSubpass_ = &model;
		model.subpassMeshes.push_back(newEntry);
	}
	return item_index;
}

template<typename Container>
inline size_t addRendermanModelEffectIfNotExists(Container& container, RendermanModel* model,
    RendermanSubpassGroup* subpassGroup, bool& isnew)
{
	//Comparator for the next find
	struct cmppipe
	{
		RendermanModel* ptr;
		cmppipe(RendermanModel* ptr) : ptr(ptr) {}
		bool operator()(const RendermanSubpassGroupModel& rhs) { return rhs.renderModel_ == ptr; }
	};

	auto found_item = std::find_if(container.begin(), container.end(), cmppipe(model));
	size_t item_index = 0;
	if (found_item != container.end())
	{
		isnew = false;
		item_index = found_item - container.begin();
	}
	else
	{
		isnew = true;
		item_index = container.size();
		container.push_back(RendermanSubpassGroupModel());
		container.back().renderModel_ = model;
		container.back().renderSubpassGroup_ = subpassGroup;

	}
	return item_index;
}



inline size_t connectMaterialEffectWithPipeline(RendermanSubpassMaterial& rms, RendermanPipeline& pipe)
{
	struct cmp
	{
		RendermanPipeline* pipe; cmp(RendermanPipeline& pipe) : pipe(&pipe) {}
		bool operator()(RendermanMaterialSubpassPipeline& rmpipe) { return rmpipe.pipeline_ == pipe; }
	};
	auto found = std::find_if(rms.materialSubpassPipelines.begin(), rms.materialSubpassPipelines.end(), cmp(pipe));
	size_t pipe_index = found - rms.materialSubpassPipelines.begin();
	if (found == rms.materialSubpassPipelines.end())
	{
		assertion(std::find(pipe.subpassMaterials.begin(), pipe.subpassMaterials.end(), &rms) == pipe.subpassMaterials.end());
		RendermanMaterialSubpassPipeline rmep;
		rmep.pipeline_ = &pipe;
		rmep.materialSubpass_ = &rms;
		pipe.subpassMaterials.push_back(&rms);
		rms.materialSubpassPipelines.push_back(rmep);
	}
	else
	{
		assertion(std::find(pipe.subpassMaterials.begin(), pipe.subpassMaterials.end(), &rms) != pipe.subpassMaterials.end());
	}
	return pipe_index;
}

inline void addBufferDefinitions(RendermanEffect& renderEffect, utils::effect::EffectApi& effect)
{
	for (auto& buffer : effect->getBuffers())
	{
		renderEffect.bufferDefinitions.resize(renderEffect.bufferDefinitions.size() + 1);
		RendermanBufferDefinition& def = renderEffect.bufferDefinitions.back();
		def.allSupportedBindings = buffer.second.allSupportedBindings;
		def.isDynamic = buffer.second.isDynamic;
		def.buffer = buffer.second.bufferView;
		def.name = buffer.first;
		def.numBuffers = buffer.second.numBuffers;
		def.scope = buffer.second.scope;
	}
}

void addNodeDynamicClientToBuffers(RendermanSubpassGroupModel&, RendermanNode& node, RendermanPipeline& pipeline)
{
	//Model buffer references are sorted by pipeline. No worries there.
	uint32 current_buffer[(uint32)FrameworkCaps::MaxDescriptorSetBindings] = { 0, 0, 0, 0 };
	//Count, so that we resize only once.
	for (auto buffer_it = pipeline.bufferBindings.begin(); buffer_it != pipeline.bufferBindings.end(); ++buffer_it)
	{
		auto& buffer = buffer_it->second;
		if ((buffer.bufferDefinition->scope == types::VariableScope::Node ||
		     buffer.bufferDefinition->scope == types::VariableScope::BoneBatch) &&
		    (buffer.type == types::DescriptorType::UniformBufferDynamic ||
		     buffer.type == types::DescriptorType::StorageBufferDynamic))
		{
			++current_buffer[buffer.set];
		}

	}
	for (uint32 set = 0; set < (uint32)FrameworkCaps::MaxDescriptorSetBindings; ++set)
	{
		node.dynamicClientId[set].resize(current_buffer[set]);
		node.dynamicOffset[set].resize(current_buffer[set]);
		node.dynamicBuffer[set].resize(current_buffer[set]);
		current_buffer[set] = 0;
	}

	std::vector<RendermanBufferBinding> sortedBuffer;
	for (auto buffer_it = pipeline.bufferBindings.begin(); buffer_it != pipeline.bufferBindings.end(); ++buffer_it)
	{
		sortedBuffer.push_back(buffer_it->second);
	}

	// sort the buffer based on set and binding
	std::sort(sortedBuffer.begin(), sortedBuffer.end(),
			  [&](const RendermanBufferBinding& a, const RendermanBufferBinding& b)
	{
		if(a.set < b.set || (a.set == b.set && a.binding < b.binding)){ return true; }
		return false;
	});

	for (auto buffer_it = sortedBuffer.begin(); buffer_it != sortedBuffer.end(); ++buffer_it)
	{
		auto& buffer = *buffer_it;
		if (buffer.bufferDefinition->scope == types::VariableScope::Node ||
		    buffer.bufferDefinition->scope == types::VariableScope::BoneBatch)
		{
			uint32 clientId = 0;
			if ((buffer.type == types::DescriptorType::UniformBufferDynamic ||
			     buffer.type == types::DescriptorType::StorageBufferDynamic))
			{
				if (buffer.bufferDefinition->scope == types::VariableScope::Node)
				{
					clientId = buffer.bufferDefinition->numDynamicClients++;
				}
				else if (buffer.bufferDefinition->scope == types::VariableScope::BoneBatch)
				{
					clientId = node.batchId;
					buffer.bufferDefinition->numDynamicClients = std::max(node.batchId + 1, buffer.bufferDefinition->numDynamicClients);
				}


				node.dynamicClientId[buffer.set][current_buffer[buffer.set]] = clientId;
				node.dynamicOffset[buffer.set][current_buffer[buffer.set]] = buffer.bufferDefinition->buffer.getAlignedElementArrayOffset(clientId);
				node.dynamicBuffer[buffer.set][current_buffer[buffer.set]] = buffer.bufferDefinition;
				++current_buffer[buffer.set];
			}
		}
	}
}



inline void prepareDataStructures(RenderManager& renderman, std::map<assets::Mesh*, AttributeConfiguration*>& meshAttributeLayout,
                                  std::map<PipelineSet, AttributeConfiguration>& pipeSets)
{
	RendermanStructure& renderStructure = renderman.renderObjects();
	// PREPARE DATA STRUCTURES
	// We need to generate the entire rendering graph.
	// The structure is effect/pass/subpass/pipeline/model/node (with offshoots the materials, meshes etc)

	// Most importantly, we need to keep a list of all pipelines used for each MESH (not Node), so that we
	// can optimize the VBO layout. [pipeSets and meshAttributeLayout objects]

	//Use this in order to determine which pipelines need to use common vertex layouts
	std::map<assets::Mesh*, std::set<StringHash>/**/> setOfAllPipesUsedPerMesh;

	// Fix the size of the renderables
	for (std::deque<RendermanEffect>::iterator effect_it = renderStructure.effects.begin();
	     effect_it != renderStructure.effects.end(); ++effect_it)
	{
		// PHASE 1: Select combinations
		//a) What are all the distinct combinations of pipelines used? (Use to create the VBO layouts)

		effect::EffectApi& effectapi = effect_it->effect;

		addBufferDefinitions(*effect_it, effectapi);

		for (uint8 passId = 0; passId < effectapi->getNumPasses(); ++passId)
		{
			effect::Pass& effectpass = effectapi->getPass(passId);
			RendermanPass& renderingpass = effect_it->passes[passId];
			renderingpass.renderEffect_ = &*effect_it;

			for (uint8 subpassId = 0; subpassId < effectpass.subpasses.size(); ++subpassId)
			{
				effect::Subpass& effectsubpass = effectpass.subpasses[subpassId];
				RendermanSubpass& rendersubpass = renderingpass.subpasses[subpassId];
				rendersubpass.renderingPass_ = &renderingpass;// keep a pointer to the parent
				for (uint8 subpassGroupid = 0; subpassGroupid < effectsubpass.groups.size(); ++subpassGroupid)
				{
					effect::SubpassGroup& effectSubpassGroup = effectsubpass.groups[subpassGroupid];
					RendermanSubpassGroup& renderSubpassGroup = rendersubpass.groups[subpassGroupid];
					renderSubpassGroup.name = effectSubpassGroup.name;
					renderSubpassGroup.subpass_ = &rendersubpass;
					//CREATE THE RENDERNODE BY SELECTING THE PIPELINE FOR EACH MODEL NODE
					for (uint32 modelId = 0; modelId < renderSubpassGroup.allModels.size(); ++modelId)
					{
						RendermanModel& rendermodel = *renderSubpassGroup.allModels[modelId];

						for (uint32 nodeId = 0; nodeId < rendermodel.assetModel->getNumMeshNodes(); ++nodeId)
						{
							//There may be more than one models for this pipeline

							auto& assetnode = rendermodel.assetModel->getMeshNode(nodeId);
							auto& assetmesh = rendermodel.assetModel->getMesh(assetnode.getObjectId());
							auto& assetmaterial = rendermodel.assetModel->getMaterial(assetnode.getMaterialIndex());


							std::pair<StringHash, effect::PipelineDef*> pipe =
							  selectPipelineForSubpassGroupMeshMaterial(effectapi, effectSubpassGroup, assetmesh, assetmaterial);

							if (!pipe.second) { continue; }
							// Add this pipeline as one used by this mesh. Needed? YES, because meshes are NOT
							// arranged with their pipes - nodes are. We need this so that if a mesh is rendered
							// by different pipes (Think of a scene with a glass sphere and a wood sphere...)
							// we are keeping the attributes needed by all of them for the VBOs.
							setOfAllPipesUsedPerMesh[&assetmesh].insert(pipe.first);

							// Now, finish with the structure - add all items needed.
							// The structure is populated for now, not finalized. The actual API objects are
							// created later. For now, only the connections between objects are necessary.
							bool isNew = true;
							bool isPipeNew = true;

							size_t pipe_index = addRendermanPipelineIfNotExists(renderSubpassGroup, pipe.second, *effect_it, isPipeNew);

							RendermanPipeline& renderpipe = renderSubpassGroup.pipelines[pipe_index];

							renderpipe.name = pipe.first;
							renderpipe.pipelineInfo = pipe.second;

							size_t model_index = addRendermanModelEffectIfNotExists(renderSubpassGroup.subpassGroupModels,
							                     &rendermodel, &renderSubpassGroup, isNew);

							RendermanSubpassGroupModel& rendermodeleffect = renderSubpassGroup.subpassGroupModels[model_index];
							RendermanMaterial& rendermaterial = rendermodel.materials[assetnode.getMaterialIndex()];

							RendermanMesh& rendermesh = rendermodel.meshes[assetnode.getObjectId()];

							size_t rendermateffect_index = addRendermanMaterialEffectIfNotExists(rendermodeleffect, &rendermaterial, isNew);
							RendermanSubpassMaterial& rendermaterialeffect = rendermodeleffect.materialEffects[rendermateffect_index];

							size_t rendermateffectpipe_index = connectMaterialEffectWithPipeline(rendermaterialeffect, renderpipe);
							RendermanMaterialSubpassPipeline& rendermaterialeffectpipe = rendermaterialeffect.materialSubpassPipelines[rendermateffectpipe_index];

							size_t rendermesheffect_index = addRendermanMeshEffectIfNotExists(rendermodeleffect, &rendermesh, isNew);
							RendermanSubpassMesh& rendermesheffect = rendermodeleffect.subpassMeshes[rendermesheffect_index];

							// keep a pointer to the pipeline used by this mesh.
							rendermesheffect.usedByPipelines.insert(&renderpipe);

							for (uint32 batch_id = 0; batch_id < std::max(assetmesh.getNumBoneBatches(), 1u); ++batch_id)
							{
								rendermodeleffect.nodes.push_back(RendermanNode());

								RendermanNode& node = rendermodeleffect.nodes.back();
								node.assetNode = assets::getNodeHandle(rendermodel.assetModel, nodeId);
								node.assetNodeId = nodeId;
								node.pipelineMaterial_ = &rendermaterialeffectpipe;
								node.subpassMesh_ = &rendermesheffect;
								node.batchId = batch_id;

								addNodeDynamicClientToBuffers(rendermodeleffect, node, renderpipe);
								addUniformSemanticLists(pipe.second->uniforms, node.uniformSemantics, true, types::VariableScope::Node);
								addUniformSemanticLists(pipe.second->uniforms, node.uniformSemantics, true, types::VariableScope::BoneBatch);
							}
						}

					}
				}

				//TODO TO BE REMOVED
//				if(allmodels.size() == 0)
//				{
//					for(const effect::ConditionalPipeline& pipe : effectsubpass.pipelines)
//					{
//						std::set<pvr::StringHash> set{pipe.pipeline};
//						pipeSets[PipelineSet(set)] = AttributeConfiguration();
//					}
//				}

			}
		}
	}
	// CAUTION: Attribute configurations are NOT created yet.
	// Pipesets exists to remove the duplication that will happen due to the mapping.
	// So, using the "pipeSet" as a key, we will check each and every pipe combination to ensure
	// that no duplicates remain, and then we map based on the pointer to the mesh.
	// In case you are wondering, the second part either inserts or ignores the key (i.e. the set
	// of pipelines we are looking for), and returns the address to it in the set. So we remove
	// all duplication this way.
	for (auto it = setOfAllPipesUsedPerMesh.begin(); it != setOfAllPipesUsedPerMesh.end(); ++it)
	{
		meshAttributeLayout[it->first] = &pipeSets[it->second];
	}


	int i = 0;

}
}
inline void fixDynamicOffsets(RenderManager& renderman)
{
	for (auto& effect : renderman.renderObjects().effects)
	{
		for (auto& pass : effect.passes)
		{
			for (auto& subpass : pass.subpasses)
			{
				for (auto& subpassGroup : subpass.groups)
				{
					for (auto& modeleffect : subpassGroup.subpassGroupModels)
					{
						for (auto& node : modeleffect.nodes)
						{
							//uint32 dynamicoffsetids[(uint32)FrameworkCaps::MaxDescriptorSetBindings] = { 0, 0, 0, 0 };
							//int32 assert_last_binding[(uint32)FrameworkCaps::MaxDescriptorSetBindings] = { -1, -1, -1, -1 };
							for (uint32 setid = 0; setid < (uint32)FrameworkCaps::MaxDescriptorSetBindings; ++setid)
							{
								for (uint32 dynamicClient = 0; dynamicClient < node.dynamicClientId[setid].size(); ++dynamicClient)
								{
									//What is the dynamic offset that should be used by this node
									node.dynamicOffset[setid][dynamicClient] =
									  node.dynamicBuffer[setid][dynamicClient]->buffer.getAlignedElementArrayOffset(node.dynamicClientId[setid][dynamicClient]);
									//Updates the number of nodes that reference this.
								}
							}
						}
					}
				}
			}
		}
	}
}


//////////////// SEMANTICS //////////////// SEMANTICS //////////////// SEMANTICS ////////////////
namespace {

static const StringHash VIEWMATRIX_STR("VIEWMATRIX");
static const StringHash VIEWPROJECTIONMATRIX_STR("VIEWPROJECTIONMATRIX");

#define CAMERA(idxchar, idx) \
    case HashCompileTime<'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N', 'M', 'A', 'T', 'R', 'I', 'X', idxchar>::value:\
    case HashCompileTime<'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N', 'M', 'T', 'X', idxchar>::value:\
    case HashCompileTime<'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N', idxchar>::value:\
    case HashCompileTime<'P', 'E', 'R', 'S', 'P', 'E', 'C', 'T', 'I', 'V', 'E', 'M', 'A', 'T', 'R', 'I', 'X', idxchar>::value:\
    case HashCompileTime<'P', 'E', 'R', 'S', 'P', 'E', 'C', 'T', 'I', 'V', 'E', 'M', 'T', 'X', idxchar>::value:\
    case HashCompileTime<'P', 'E', 'R', 'S', 'P', 'E', 'C', 'T', 'I', 'V', 'E', idxchar>::value:\
{ return &getPerspectiveMatrix##idx; } break;\
    case HashCompileTime<'V', 'I', 'E', 'W', 'M', 'A', 'T', 'R', 'I', 'X', idxchar>::value:\
    case HashCompileTime<'V', 'I', 'E', 'W', 'M', 'T', 'X', idxchar>::value:\
    case HashCompileTime<'V', 'I', 'E', 'W', idxchar>::value:\
{ return &getViewMatrix##idx; } break;\
    case HashCompileTime<'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N', 'M', 'A', 'T', 'R', 'I', 'X', idxchar>::value:\
    case HashCompileTime<'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N', 'M', 'T', 'X', idxchar>::value:\
    case HashCompileTime<'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N', idxchar>::value:\
    case HashCompileTime<'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'M', 'A', 'T', 'R', 'I', 'X', idxchar>::value:\
    case HashCompileTime<'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'M', 'T', 'X', idxchar>::value:\
    case HashCompileTime<'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', idxchar>::value:\
    case HashCompileTime<'V', 'P', 'M', 'A', 'T', 'R', 'I', 'X', idxchar>::value:\
{ return &getViewProjectionMatrix##idx; } break;\


#define LIGHT0_9(idxchar,idx) \
    case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'P', 'O', 'S', 'I', 'T', 'I', 'O', 'N', idxchar>::value: \
    case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'P', 'O', 'S', idxchar>::value: { return getLightPosition##idx; } break; \
    case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'D', 'I', 'R', 'E', 'C', 'T', 'I', 'O', 'N', idxchar>::value: \
    case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'D', 'I', 'R', idxchar>::value: { return getLightDirection##idx; } break; \
    case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'C', 'O', 'L', 'O', 'R', idxchar>::value: \
    case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'C', 'O', 'L', 'O', 'U', 'R', idxchar>::value: {return getLightColour##idx; }break; \


#define LIGHT10_99(idxchar0,idxchar1,idx) \
    case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'P', 'O', 'S', 'I', 'T', 'I', 'O', 'N', idxchar0, idxchar1>::value: \
    case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'P', 'O', 'S', idxchar0, idxchar1>::value: return getLightPosition##idx; break; \
    case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'D', 'I', 'R', 'E', 'C', 'T', 'I', 'O', 'N', idxchar0, idxchar1>::value: \
    case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'D', 'I', 'R', idxchar0, idxchar1>::value: return getLightDirection##idx; break; \
    case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'C', 'O', 'L', 'O', 'R', idxchar0, idxchar1>::value: \
    case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'C', 'O', 'L', 'O', 'U', 'R', idxchar0, idxchar1>::value: return getLightColour##idx; break; \

#define BONEMTX0_9(idxchar, idx) \
    case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'X', idxchar>::value:\
    case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'T', 'X', idxchar>::value:\
    case HashCompileTime<'B', 'O', 'N', 'E', idxchar>::value: return &getBoneMatrix##idx; break;\
    case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'X', 'I', 'T', idxchar> ::value:\
    case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'T', 'X', 'I', 'T', idxchar> ::value:\
    case HashCompileTime<'B', 'O', 'N', 'E', 'I', 'T', idxchar> ::value: return &getBoneMatrixIT##idx;break;\

#define BONEMTX10_99(idxchar0,idxchar1, idx) \
    case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'X', idxchar0, idxchar1>::value: \
    case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'T', 'X', idxchar0, idxchar1>::value: \
    case HashCompileTime<'B', 'O', 'N', 'E', idxchar0, idxchar1>::value: return &getBoneMatrix##idx; break;\
    case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'X', 'I', 'T', idxchar0, idxchar1> ::value: \
    case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'T', 'X', 'I', 'T', idxchar0, idxchar1> ::value: \
    case HashCompileTime<'B', 'O', 'N', 'E', 'I', 'T', idxchar0, idxchar1> ::value: return &getBoneMatrixIT##idx;break;\

}

//////////// FUNCTIONS THAT WILL READ SEMANTIC INFORMATION FROM THE MODEL ///////
/////////   (NodeSemanticSetters / ModelSemanticSetters ////////////
namespace {

inline bool getPerspectiveMatrix(TypedMem& memory, const RendermanModel& rmodel, uint32 cameraIndex)
{
	auto& ctx = rmodel.mgr_->getContext();
	const assets::Model& model = *rmodel.assetModel;

	if (model.getNumCameraNodes() <= cameraIndex) { return false; }
	glm::vec3 from, to, up(0.0f, 1.0f, 0.0f);
	float fov, nearClip, farClip;
	model.getCameraProperties(cameraIndex, fov, from, to, up, nearClip, farClip); // vTo is calculated from the rotation
	memory.setValue(pvr::utils::getPerspectiveMatrix(*ctx, fov, nearClip, farClip));
	return true;
}
inline bool getViewMatrix(TypedMem& memory, const RendermanModel& rmodel, uint32 cameraIndex)
{
	const assets::Model& model = *rmodel.assetModel;

	if (model.getNumCameraNodes() <= cameraIndex) { return false; }
	glm::vec3 from, to, up(0.0f, 1.0f, 0.0f);
	glm::mat4x4 view;
	float fov, nearClip, farClip;

	model.getCameraProperties(cameraIndex, fov, from, to, up, nearClip, farClip); // vTo is calculated from the rotation

	//We build the model view matrix from the camera position, target and an up vector.
	//For this we use glm::lookAt().
	memory.setValue(glm::lookAt(from, to, up));
	return true;
}
inline bool getViewProjectionMatrix(TypedMem&  memory, const RendermanModel& rmodel, uint32 cameraIndex)
{
	auto& ctx = rmodel.mgr_->getContext();
	const assets::Model& model = *rmodel.assetModel;

	if (model.getNumCameraNodes() <= cameraIndex) { return false; }
	glm::vec3 from, to, up(0.0f, 1.0f, 0.0f);
	glm::mat4x4 view;
	float fov, nearClip, farClip;

	model.getCameraProperties(cameraIndex, fov, from, to, up, nearClip, farClip); // vTo is calculated from the rotation

	//We build the model view matrix from the camera position, target and an up vector.
	//For this we use glm::lookAt().
	view = glm::lookAt(from, to, up);

	auto& attribs = ctx->getDisplayAttributes();

	// Set up the View * Projection Matrix
	bool isRotated = attribs.isScreenRotated();
	if (isRotated)
	{
		memory.setValue(
		  pvr::math::perspective(ctx->getApiType(), fov, (pvr::float32)attribs.height / attribs.width, nearClip, farClip,
		                         glm::pi<pvr::float32>() * .5f) * view);
	}
	else
	{
		memory.setValue(
		  pvr::math::perspective(ctx->getApiType(), fov, (pvr::float32)attribs.width / attribs.height, nearClip, farClip) * view);
	}

	return true;
}

inline bool getLightPosition(TypedMem& mem, const RendermanModel& rmodel, int32 lightNodeId)
{
	const assets::Model& model = *rmodel.assetModel;

	if (model.getNumLightNodes() <= (uint32)lightNodeId) { return false; }
	mem.setValue(model.getLightPosition(lightNodeId));
	return true;
}
inline bool getLightDirection(TypedMem& mem, const RendermanModel& rmodel, int32 lightNodeId)
{
	const assets::Model& model = *rmodel.assetModel;

	if (model.getNumLightNodes() <= (uint32)lightNodeId) { return false; }
	mem.allocate(types::GpuDatatypes::vec3);
	model.getLightDirection(lightNodeId, mem.interpretValueAs<glm::vec3>());
	return true;
}
inline bool getLightColour(TypedMem& mem, const RendermanModel& rmodel, int32 lightNodeId)
{
	const assets::Model& model = *rmodel.assetModel;

	if (model.getNumLightNodes() <= (uint32)lightNodeId) { return false; }
	mem.setValue(model.getLight(lightNodeId).getColor());
	return true;
}

inline bool getBoneMatrix(TypedMem& mem, const RendermanNode& node, uint32 boneid)
{
	auto& rmesh = node.toRendermanMesh();
	auto& assetmesh = *rmesh.assetMesh;

	debug_assertion(assetmesh.getMeshInfo().isSkinned && (assetmesh.getNumBoneBatches() > node.batchId &&
	                assetmesh.getBatchBoneCount(node.batchId) > boneid), "OUT OF BOUNDS");
	int i32BoneNodeID = assetmesh.getBatchBone(node.batchId, boneid);
	mem.setValue(rmesh.renderModel_->assetModel->getBoneWorldMatrix(rmesh.assetMeshId, i32BoneNodeID));
	return true;
}

inline bool getBoneMatrixIT(TypedMem& mem, const RendermanNode& node, uint32 boneid)
{
	auto& rmesh = node.toRendermanMesh();
	auto& assetmesh = *rmesh.assetMesh;

	debug_assertion(assetmesh.getMeshInfo().isSkinned && (assetmesh.getNumBoneBatches() > node.batchId &&
	                assetmesh.getBatchBoneCount(node.batchId) > boneid), "OUT OF BOUNDS");
	int i32BoneNodeID = assetmesh.getBatchBone(node.batchId, boneid);
	mem.setValue(glm::inverseTranspose(glm::mat3(rmesh.renderModel_->assetModel->getBoneWorldMatrix(rmesh.assetMeshId, i32BoneNodeID))));
	return true;
}

inline bool getBoneMatrices(TypedMem& mem, const RendermanNode& node)
{
	auto& rmesh = node.toRendermanMesh();
	auto& assetmesh = *rmesh.assetMesh;

	mem.allocate(types::GpuDatatypes::mat4x4, assetmesh.getBatchBoneCount(node.batchId));
	debug_assertion(assetmesh.getMeshInfo().isSkinned && (assetmesh.getNumBoneBatches() > node.batchId), "OUT OF BOUNDS");
	for (uint32 boneid = mem.arrayElements(); boneid > 0;/*done in the loop*/)
	{
		--boneid;
		int i32BoneNodeID = assetmesh.getBatchBone(node.batchId, boneid);
		mem.setValue(rmesh.renderModel_->assetModel->getBoneWorldMatrix(rmesh.assetMeshId, i32BoneNodeID), boneid);
	}

	return true;
}

inline bool getBoneMatricesIT(TypedMem& mem, const RendermanNode& node)
{
	auto& rmesh = node.toRendermanMesh();
	auto& assetmesh = *rmesh.assetMesh;
	debug_assertion(assetmesh.getMeshInfo().isSkinned && (assetmesh.getNumBoneBatches() > node.batchId), "OUT OF BOUNDS");
	mem.allocate(types::GpuDatatypes::mat3x3, assetmesh.getBatchBoneCount(node.batchId));
	for (uint32 boneid = mem.arrayElements(); boneid > 0;/*done in the loop*/)
	{
		--boneid;
		int i32BoneNodeID = assetmesh.getBatchBone(node.batchId, boneid);
		mem.setValue(glm::inverseTranspose(glm::mat3(rmesh.renderModel_->assetModel->getBoneWorldMatrix(rmesh.assetMeshId, i32BoneNodeID))), boneid);
	}
	return true;
}

inline bool getBoneCount(TypedMem& mem, const RendermanNode& node)
{
	mem.setValue(node.toRendermanMesh().assetMesh->getBoneCount());
	return true;
}

#define BONEFUNC(idx) bool getBoneMatrix##idx(TypedMem& mem, const RendermanNode& node) { return getBoneMatrix(mem, node, idx); }\
    bool getBoneMatrixIT##idx(TypedMem& mem, const RendermanNode& node) { return getBoneMatrixIT(mem, node, idx); }

BONEFUNC(0)BONEFUNC(1)BONEFUNC(2)BONEFUNC(3)BONEFUNC(4)BONEFUNC(5)BONEFUNC(6)BONEFUNC(7)BONEFUNC(8)BONEFUNC(9)
BONEFUNC(10)BONEFUNC(11)BONEFUNC(12)BONEFUNC(13)BONEFUNC(14)BONEFUNC(15)BONEFUNC(16)BONEFUNC(17)BONEFUNC(18)BONEFUNC(19)
BONEFUNC(20)BONEFUNC(21)BONEFUNC(22)BONEFUNC(23)BONEFUNC(24)BONEFUNC(25)BONEFUNC(26)BONEFUNC(27)BONEFUNC(28)BONEFUNC(29)
BONEFUNC(30)BONEFUNC(31)BONEFUNC(32)BONEFUNC(33)BONEFUNC(34)BONEFUNC(35)BONEFUNC(36)BONEFUNC(37)BONEFUNC(38)BONEFUNC(39)
BONEFUNC(40)BONEFUNC(41)BONEFUNC(42)BONEFUNC(43)BONEFUNC(44)BONEFUNC(45)BONEFUNC(46)BONEFUNC(47)BONEFUNC(48)BONEFUNC(49)
BONEFUNC(50)BONEFUNC(51)BONEFUNC(52)BONEFUNC(53)BONEFUNC(54)BONEFUNC(55)BONEFUNC(56)BONEFUNC(57)BONEFUNC(58)BONEFUNC(59)
BONEFUNC(60)BONEFUNC(61)BONEFUNC(62)BONEFUNC(63)BONEFUNC(64)BONEFUNC(65)BONEFUNC(66)BONEFUNC(67)BONEFUNC(68)BONEFUNC(69)
BONEFUNC(70)BONEFUNC(71)BONEFUNC(72)BONEFUNC(73)BONEFUNC(74)BONEFUNC(75)BONEFUNC(76)BONEFUNC(77)BONEFUNC(78)BONEFUNC(79)
BONEFUNC(80)BONEFUNC(81)BONEFUNC(82)BONEFUNC(83)BONEFUNC(84)BONEFUNC(85)BONEFUNC(86)BONEFUNC(87)BONEFUNC(88)BONEFUNC(89)
BONEFUNC(90)BONEFUNC(91)BONEFUNC(92)BONEFUNC(93)BONEFUNC(94)BONEFUNC(95)BONEFUNC(96)BONEFUNC(97)BONEFUNC(98)BONEFUNC(99)


#define LIGHTFUNC(idx) bool getLightPosition##idx(TypedMem& mem, const RendermanModel& model) { return getLightPosition(mem, model, idx); }\
    bool getLightDirection##idx(TypedMem& mem, const RendermanModel& model) { return getLightDirection(mem, model, idx); }\
    bool getLightColour##idx(TypedMem& mem, const RendermanModel& model) { return getLightColour(mem, model, idx); }\

LIGHTFUNC(0)LIGHTFUNC(1)LIGHTFUNC(2)LIGHTFUNC(3)LIGHTFUNC(4)LIGHTFUNC(5)LIGHTFUNC(6)LIGHTFUNC(7)LIGHTFUNC(8)LIGHTFUNC(9)
LIGHTFUNC(10)LIGHTFUNC(11)LIGHTFUNC(12)LIGHTFUNC(13)LIGHTFUNC(14)LIGHTFUNC(15)LIGHTFUNC(16)LIGHTFUNC(17)LIGHTFUNC(18)LIGHTFUNC(19)
LIGHTFUNC(20)LIGHTFUNC(21)LIGHTFUNC(22)LIGHTFUNC(23)LIGHTFUNC(24)LIGHTFUNC(25)LIGHTFUNC(26)LIGHTFUNC(27)LIGHTFUNC(28)LIGHTFUNC(29)
LIGHTFUNC(30)LIGHTFUNC(31)LIGHTFUNC(32)LIGHTFUNC(33)LIGHTFUNC(34)LIGHTFUNC(35)LIGHTFUNC(36)LIGHTFUNC(37)LIGHTFUNC(38)LIGHTFUNC(39)
LIGHTFUNC(40)LIGHTFUNC(41)LIGHTFUNC(42)LIGHTFUNC(43)LIGHTFUNC(44)LIGHTFUNC(45)LIGHTFUNC(46)LIGHTFUNC(47)LIGHTFUNC(48)LIGHTFUNC(49)
LIGHTFUNC(50)LIGHTFUNC(51)LIGHTFUNC(52)LIGHTFUNC(53)LIGHTFUNC(54)LIGHTFUNC(55)LIGHTFUNC(56)LIGHTFUNC(57)LIGHTFUNC(58)LIGHTFUNC(59)
LIGHTFUNC(60)LIGHTFUNC(61)LIGHTFUNC(62)LIGHTFUNC(63)LIGHTFUNC(64)LIGHTFUNC(65)LIGHTFUNC(66)LIGHTFUNC(67)LIGHTFUNC(68)LIGHTFUNC(69)
LIGHTFUNC(70)LIGHTFUNC(71)LIGHTFUNC(72)LIGHTFUNC(73)LIGHTFUNC(74)LIGHTFUNC(75)LIGHTFUNC(76)LIGHTFUNC(77)LIGHTFUNC(78)LIGHTFUNC(79)
LIGHTFUNC(80)LIGHTFUNC(81)LIGHTFUNC(82)LIGHTFUNC(83)LIGHTFUNC(84)LIGHTFUNC(85)LIGHTFUNC(86)LIGHTFUNC(87)LIGHTFUNC(88)LIGHTFUNC(89)
LIGHTFUNC(90)LIGHTFUNC(91)LIGHTFUNC(92)LIGHTFUNC(93)LIGHTFUNC(94)LIGHTFUNC(95)LIGHTFUNC(96)LIGHTFUNC(97)LIGHTFUNC(98)LIGHTFUNC(99)

#define CAMFUNC(idx) bool getPerspectiveMatrix##idx(TypedMem& mem, const RendermanModel& model) { return getPerspectiveMatrix(mem, model, idx); }\
    bool getViewMatrix##idx(TypedMem& mem, const RendermanModel& model) { return getViewMatrix(mem, model, idx); }\
    bool getViewProjectionMatrix##idx(TypedMem& mem, const RendermanModel& model) { return getViewProjectionMatrix(mem, model, idx); }

CAMFUNC(0)CAMFUNC(1)CAMFUNC(2)CAMFUNC(3)CAMFUNC(4)CAMFUNC(5)CAMFUNC(6)CAMFUNC(7)CAMFUNC(8)CAMFUNC(9)

inline bool getWorldMatrix(TypedMem& mem, const RendermanNode& node)
{
	mem.setValue(node.toRendermanMesh().renderModel_->assetModel->getWorldMatrix(node.assetNodeId));
	return true;
}

inline bool getWorldMatrixIT(TypedMem& mem, const RendermanNode& node)
{
	mem.setValue(glm::inverseTranspose(glm::mat3(node.toRendermanMesh().renderModel_->assetModel->getWorldMatrix(node.assetNodeId))));
	return true;
}


inline bool getModelViewMatrix(TypedMem& mem, const RendermanNode& node)
{
	getWorldMatrix(mem, node);
	TypedMem viewmtx;
	node.toRendermanMesh().renderModel_->getModelSemantic(VIEWMATRIX_STR, viewmtx);
	mem.interpretValueAs<glm::mat4>() = viewmtx.interpretValueAs<glm::mat4>() * mem.interpretValueAs<glm::mat4>();
	return true;
}

inline bool getModelViewProjectionMatrix(TypedMem& mem, const RendermanNode& node)
{
	getWorldMatrix(mem, node);
	TypedMem viewprojmtx;
	if (node.subpassMesh_->rendermesh_->renderModel_->getModelSemantic(VIEWPROJECTIONMATRIX_STR, viewprojmtx))
	{
		mem.interpretValueAs<glm::mat4>() = viewprojmtx.interpretValueAs<glm::mat4>() * mem.interpretValueAs<glm::mat4>();
	}
	return true;
}
}


/////////// MEMBER FUNCTIONS OF THE RENDERMANAGER ///////////////

//RENDERNODE

bool RendermanNode::getNodeSemantic(const StringHash& semantic, TypedMem& mem) const
{
	return getNodeSemanticSetter(semantic)(mem, *this);
}

NodeSemanticSetter RendermanNode::getNodeSemanticSetter(const StringHash& semantic) const
{
	switch (semantic.getHash())
	{
	case HashCompileTime<'W', 'O', 'R', 'L', 'D', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'W', 'O', 'R', 'L', 'D', 'M', 'T', 'X'>::value:
	case HashCompileTime<'W', 'O', 'R', 'L', 'D'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'M', 'T', 'X'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'W', 'O', 'R', 'L', 'D', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'W', 'O', 'R', 'L', 'D', 'M', 'T', 'X'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'W', 'O', 'R', 'L', 'D'>::value:
	{
		return &getWorldMatrix;
	}
	break;
	case HashCompileTime<'W', 'O', 'R', 'L', 'D', 'I', 'T', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'W', 'O', 'R', 'L', 'D', 'I', 'T', 'M', 'T', 'X'>::value:
	case HashCompileTime<'W', 'O', 'R', 'L', 'D', 'I', 'T'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'I', 'T', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'I', 'T', 'M', 'T', 'X'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'I', 'T'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'W', 'O', 'R', 'L', 'D', 'I', 'T', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'W', 'O', 'R', 'L', 'D', 'I', 'T', 'M', 'T', 'X'>::value:
	case HashCompileTime<'W', 'O', 'R', 'L', 'D', 'M', 'A', 'T', 'R', 'I', 'X', 'I', 'T'>::value:
	case HashCompileTime<'W', 'O', 'R', 'L', 'D', 'M', 'T', 'X', 'I', 'T'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'M', 'A', 'T', 'R', 'I', 'X', 'I', 'T'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'M', 'T', 'X', 'I', 'T'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'W', 'O', 'R', 'L', 'D', 'M', 'A', 'T', 'R', 'I', 'X', 'I', 'T'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'W', 'O', 'R', 'L', 'D', 'M', 'T', 'X', 'I', 'T'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'W', 'O', 'R', 'L', 'D', 'I', 'T'>::value:
	{
		return &getWorldMatrixIT;
	}
	break;
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'V', 'I', 'E', 'W', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'V', 'I', 'E', 'W', 'M', 'T', 'X'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'V', 'I', 'E', 'W'>::value:
	case HashCompileTime<'M', 'V', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'M', 'V', 'M', 'T', 'X'>::value:
	case HashCompileTime<'M', 'V'>::value:
	{
		return &getModelViewMatrix;
	}
	break;
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N', 'M', 'T', 'X'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N'>::value:
	case HashCompileTime<'M', 'V', 'P', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'M', 'V', 'P', 'M', 'T', 'X'>::value:
	case HashCompileTime<'M', 'V', 'P'>::value:
	{
		return &getModelViewProjectionMatrix;
	}
	break;
	case HashCompileTime<'B', 'O', 'N', 'E', 'C', 'O', 'U', 'N', 'T'>::value:
	case HashCompileTime<'N', 'U', 'M', 'B', 'O', 'N', 'E', 'S'>::value:
	{
		return &getBoneCount;
	}
	break;
	case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'C', 'E', 'S'>::value: \
	case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'X', 'A', 'R', 'R', 'A', 'Y'>::value: \
	case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'X'>::value: \
	case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'T', 'X'>::value: \
	case HashCompileTime<'B', 'O', 'N', 'E'>::value: return &getBoneMatrices; break; \
	case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'C', 'E', 'S', 'I', 'T'> ::value: \
	case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'C', 'E', 'S', 'A', 'R', 'R', 'A', 'Y', 'I', 'T'> ::value: \
	case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'X', 'A', 'R', 'R', 'A', 'Y', 'I', 'T'> ::value: \
	case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'C', 'E', 'S', 'I', 'T', 'A', 'R', 'R', 'A', 'Y'> ::value: \
	case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'X', 'I', 'T', 'A', 'R', 'R', 'A', 'Y'> ::value: \
	case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'X', 'I', 'T'> ::value: \
	case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'T', 'X', 'I', 'T'> ::value: \
	case HashCompileTime<'B', 'O', 'N', 'E', 'I', 'T'> ::value: return &getBoneMatricesIT; break; \

		BONEMTX0_9('0', 0)BONEMTX0_9('1', 1)BONEMTX0_9('2', 2)BONEMTX0_9('3', 3)BONEMTX0_9('4', 4)BONEMTX0_9('5', 5)BONEMTX0_9('6', 6)BONEMTX0_9('7', 7)BONEMTX0_9('8', 8)BONEMTX0_9('9', 9)
		BONEMTX10_99('1', '0', 10)BONEMTX10_99('1', '1', 11)BONEMTX10_99('1', '2', 12)BONEMTX10_99('1', '3', 13)BONEMTX10_99('1', '4', 14)BONEMTX10_99('1', '5', 15)BONEMTX10_99('1', '6', 16)BONEMTX10_99('1', '7', 17)BONEMTX10_99('1', '8', 18)BONEMTX10_99('1', '9', 19)
		BONEMTX10_99('2', '0', 20)BONEMTX10_99('2', '1', 21)BONEMTX10_99('2', '2', 22)BONEMTX10_99('2', '3', 23)BONEMTX10_99('2', '4', 24)BONEMTX10_99('2', '5', 25)BONEMTX10_99('2', '6', 26)BONEMTX10_99('2', '7', 27)BONEMTX10_99('2', '8', 28)BONEMTX10_99('2', '9', 29)
		BONEMTX10_99('3', '0', 30)BONEMTX10_99('3', '1', 31)BONEMTX10_99('3', '2', 32)BONEMTX10_99('3', '3', 33)BONEMTX10_99('3', '4', 34)BONEMTX10_99('3', '5', 35)BONEMTX10_99('3', '6', 36)BONEMTX10_99('3', '7', 37)BONEMTX10_99('3', '8', 38)BONEMTX10_99('3', '9', 39)
		BONEMTX10_99('4', '0', 40)BONEMTX10_99('4', '1', 41)BONEMTX10_99('4', '2', 42)BONEMTX10_99('4', '3', 43)BONEMTX10_99('4', '4', 44)BONEMTX10_99('4', '5', 45)BONEMTX10_99('4', '6', 46)BONEMTX10_99('4', '7', 47)BONEMTX10_99('4', '8', 48)BONEMTX10_99('4', '9', 49)
		BONEMTX10_99('5', '0', 50)BONEMTX10_99('5', '1', 51)BONEMTX10_99('5', '2', 52)BONEMTX10_99('5', '3', 53)BONEMTX10_99('5', '4', 54)BONEMTX10_99('5', '5', 55)BONEMTX10_99('5', '6', 56)BONEMTX10_99('5', '7', 57)BONEMTX10_99('5', '8', 58)BONEMTX10_99('5', '9', 59)
		BONEMTX10_99('6', '0', 60)BONEMTX10_99('6', '1', 61)BONEMTX10_99('6', '2', 62)BONEMTX10_99('6', '3', 63)BONEMTX10_99('6', '4', 64)BONEMTX10_99('6', '5', 65)BONEMTX10_99('6', '6', 66)BONEMTX10_99('6', '7', 67)BONEMTX10_99('6', '8', 68)BONEMTX10_99('6', '9', 69)
		BONEMTX10_99('7', '0', 70)BONEMTX10_99('7', '1', 71)BONEMTX10_99('7', '2', 72)BONEMTX10_99('7', '3', 73)BONEMTX10_99('7', '4', 74)BONEMTX10_99('7', '5', 75)BONEMTX10_99('7', '6', 76)BONEMTX10_99('7', '7', 77)BONEMTX10_99('7', '8', 78)BONEMTX10_99('7', '9', 79)
		BONEMTX10_99('8', '0', 80)BONEMTX10_99('8', '1', 81)BONEMTX10_99('8', '2', 82)BONEMTX10_99('8', '3', 83)BONEMTX10_99('8', '4', 84)BONEMTX10_99('8', '5', 85)BONEMTX10_99('8', '6', 86)BONEMTX10_99('8', '7', 87)BONEMTX10_99('8', '8', 88)BONEMTX10_99('8', '9', 89)
		BONEMTX10_99('9', '0', 90)BONEMTX10_99('9', '1', 91)BONEMTX10_99('9', '2', 92)BONEMTX10_99('9', '3', 93)BONEMTX10_99('9', '4', 94)BONEMTX10_99('9', '5', 95)BONEMTX10_99('9', '6', 96)BONEMTX10_99('9', '7', 97)BONEMTX10_99('9', '8', 98)BONEMTX10_99('9', '9', 99)
	}
	return NULL;
}

bool RendermanNode::updateNodeValueSemantic(const StringHash& semantic, const FreeValue& value, uint32 swapid)
{
	return toRendermanPipeline().updateBufferEntryNodeSemantic(semantic, value, swapid, *this);
}

//RENDERMODEL

ModelSemanticSetter RendermanModel::getModelSemanticSetter(const StringHash& semantic) const
{
	switch (semantic.getHash())
	{
	case HashCompileTime<'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N', 'M', 'T', 'X'>::value:
	case HashCompileTime<'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N'>::value:
	case HashCompileTime<'P', 'E', 'R', 'S', 'P', 'E', 'C', 'T', 'I', 'V', 'E', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'P', 'E', 'R', 'S', 'P', 'E', 'C', 'T', 'I', 'V', 'E', 'M', 'T', 'X'>::value:
	case HashCompileTime<'P', 'E', 'R', 'S', 'P', 'E', 'C', 'T', 'I', 'V', 'E'>::value:
		return &getPerspectiveMatrix0;
		break;
	case HashCompileTime<'V', 'I', 'E', 'W', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'V', 'I', 'E', 'W', 'M', 'T', 'X'>::value:
	case HashCompileTime<'V', 'I', 'E', 'W'>::value:
		return &getViewMatrix0;
		break;
	case HashCompileTime<'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N', 'M', 'T', 'X'>::value:
	case HashCompileTime<'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N'>::value:
	case HashCompileTime<'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'M', 'T', 'X'>::value:
	case HashCompileTime<'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J'>::value:
	case HashCompileTime<'V', 'P', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
		return &getViewProjectionMatrix0;
		break;
		// Expands to definitions like the above, but suffixed with a character: VIEWPROJECTION0,VIEWPROJECTION1,VIEWMATRIX0,VIEWMATRIX1 etc, each referencing a different camera.
		CAMERA('0', 0)CAMERA('1', 1)CAMERA('2', 2)CAMERA('3', 3)CAMERA('4', 4)CAMERA('5', 5)CAMERA('6', 6)CAMERA('7', 7)CAMERA('8', 8)CAMERA('9', 9)
		break;
	case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'P', 'O', 'S', 'I', 'T', 'I', 'O', 'N'>::value:
	case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'P', 'O', 'S'>::value:
		return &getLightPosition0;
		break;
	case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'D', 'I', 'R', 'E', 'C', 'T', 'I', 'O', 'N'>::value:
	case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'D', 'I', 'R'>::value:
		return &getLightDirection0;
		break;
	case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'C', 'O', 'L', 'O', 'R'>::value:
	case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'C', 'O', 'L', 'O', 'U', 'R'>::value:
		return &getLightColour0;
		break;
		LIGHT0_9('0', 0)LIGHT0_9('1', 1)LIGHT0_9('2', 2)LIGHT0_9('3', 3)LIGHT0_9('4', 4)LIGHT0_9('5', 5)LIGHT0_9('6', 6)LIGHT0_9('7', 7)LIGHT0_9('8', 8)LIGHT0_9('9', 9)
	break;  default:
		break;
	}
	return NULL;
}

bool RendermanModel::getModelSemantic(const StringHash& semantic, TypedMem& memory) const
{
	auto setter = getModelSemanticSetter(semantic);
	return setter ? setter(memory, *this) : false;
}

/////////// RENDERING COMMANDS - (various classes of the RenderManager) ///////////

void RenderManager::recordAllRenderingCommands(api::CommandBuffer& cbuff, uint16 swapIdx, bool recordBeginEndRenderpass, bool recordUpdateUniforms)
{
	for (auto& effect : renderStructure.effects)
	{
		effect.recordRenderingCommands(cbuff, swapIdx, recordBeginEndRenderpass, recordUpdateUniforms);
	}
}

void RendermanEffect::recordRenderingCommands(api::CommandBuffer& cbuff, uint16 swapIdx, bool beginEndRenderpass, bool recordUpdateUniforms)
{
	for (auto& pass : passes)
	{
		pass.recordRenderingCommands(cbuff, swapIdx, beginEndRenderpass, recordUpdateUniforms);
	}
}

void RendermanPass::recordRenderingCommands(api::CommandBuffer& cbuff, uint16 swapIdx, bool beginEndRendermanPass,
    bool recordUpdateUniforms)
{
	if (beginEndRendermanPass)
	{
		// use the clear color from the model if found, else use the default
		glm::vec4 clearColor(.0f, .0f, .0f, 1.0f);
		for (uint32 i = 0; i < subpasses.size(); ++i)
		{
			for (uint32 j = 0; j < subpasses[i].groups.size(); ++j)
			{
				for (RendermanModel* model : subpasses[i].groups[j].allModels)
				{
					if (model)
					{
						memcpy(&clearColor, model->assetModel->getInternalData().clearColor,
						       sizeof(model->assetModel->getInternalData().clearColor));
						i = j = uint32(-1);// break the outer loop as well.
						break;
					}
				}
			}
		}
		recordRenderingCommands_(cbuff, swapIdx, recordUpdateUniforms, &clearColor);
	}
	else
	{
		recordRenderingCommands_(cbuff, swapIdx, recordUpdateUniforms, NULL);
	}
}

void RendermanPass::recordRenderingCommands_(api::CommandBuffer& cbuff, uint16 swapIdx, bool recordUpdateUniforms, const glm::vec4* clearColor)
{
	if (clearColor) { cbuff->beginRenderPass(fbo[swapIdx], true, *clearColor); }
	bool first = true;
	for (auto& subpass : subpasses)
	{
		subpass.recordRenderingCommands(cbuff, swapIdx, !first, recordUpdateUniforms);
		first = false;
	}
	if (clearColor) { cbuff->endRenderPass(); }
}


void RendermanSubpassGroup::recordRenderingCommands(api::CommandBufferBase cbuff, uint16 swapIdx, bool recordUpdateUniforms)
{
	for (auto& spmodels : subpassGroupModels)
	{
		spmodels.recordRenderingCommands(cbuff, swapIdx, recordUpdateUniforms);
	}
}

void RendermanSubpassGroupModel::recordRenderingCommands(api::CommandBufferBase cbuff, uint16 swapIdx, bool recordUpdateUniforms)
{
	api::DescriptorSet::ElementType* prev_sets[4];

	bool bindSets[(uint32)FrameworkCaps::MaxDescriptorSetBindings] = { true, true, true, true };

	uint32* dynamicOffsets[(uint32)FrameworkCaps::MaxDescriptorSetBindings] { NULL, NULL, NULL, NULL };

	api::GraphicsPipeline::ElementType* prev_pipeline = NULL;
	for (auto& node : nodes)
	{
		auto& renderpipeline = *node.pipelineMaterial_->pipeline_;
		api::GraphicsPipeline& pipeline = renderpipeline.apiPipeline;

		bool bindPipeline = (!prev_pipeline || pipeline.get() != prev_pipeline);
		prev_pipeline = pipeline.get();

		for (uint32 setid = 0; setid < (uint32)FrameworkCaps::MaxDescriptorSetBindings; ++setid)
		{
			if (!renderpipeline.pipelineInfo->descSetExists[setid]) { bindSets[setid] = false; continue; }
			uint32 setswapid = renderpipeline.pipelineInfo->descSetIsMultibuffered[setid] ? swapIdx : 0;

			bindSets[setid] = (bindPipeline || node.pipelineMaterial_->sets[setid][setswapid].get() !=
			                   prev_sets[setid] || node.dynamicOffset[setid].data() != dynamicOffsets[setid]);

			if (bindSets[setid])
			{
				prev_sets[setid] = node.pipelineMaterial_->sets[setid][setswapid].get();
				dynamicOffsets[setid] = node.dynamicOffset[setid].data();
			}
		}

		node.recordRenderingCommands(cbuff, swapIdx, bindPipeline, bindSets, recordUpdateUniforms);
	}
}

void RendermanNode::recordRenderingCommands(api::CommandBufferBase cbuff, uint16 swapidx, bool recordBindPipeline,
    bool* recordBindDescriptorSets, bool recordUpdateUniforms, bool recordBindVboIbo, bool recordDrawCalls)
{
	auto& pipe = toRendermanPipeline();
	auto& rmesh = toRendermanMesh();
	if (!pipe.apiPipeline.isValid()) { return; }
	if (recordBindPipeline)
	{
		cbuff->bindPipeline(pipe.apiPipeline);
	}
	for (uint32 setid = 0; setid < (uint32)FrameworkCaps::MaxDescriptorSetBindings; ++setid)
	{
		if (!recordBindDescriptorSets || recordBindDescriptorSets[setid])
		{
			if (!pipe.pipelineInfo->descSetExists[setid]) { continue; }
			uint32 setswapid = pipe.pipelineInfo->descSetIsMultibuffered[setid] ? swapidx : 0;
			cbuff->bindDescriptorSet(pipe.apiPipeline->getPipelineLayout(), setid,
			    pipelineMaterial_->sets[setid][setswapid], dynamicOffset[setid].data(), dynamicOffset[setid].size());
		}
	}

	if (recordBindVboIbo)
	{
		if (rmesh.vbos.size() > 0) { cbuff->bindVertexBuffer(rmesh.vbos[0], 0, 0); }
		if (rmesh.ibo.isValid()) {cbuff->bindIndexBuffer(rmesh.ibo, 0, rmesh.indexType);}
	}
	if (recordUpdateUniforms)
	{
		if (recordBindPipeline)
		{
			for (auto& uniform : pipe.uniformSemantics)
			{
				recordUpdateUniformSemanticToExternalMemory(cbuff, uniform.second.uniformLocation, uniform.second.memory);
			}
			for (auto& uniform : pipe.backToRendermanEffect().uniformSemantics)
			{
				recordUpdateUniformSemanticToExternalMemory(cbuff, uniform.second.uniformLocation, uniform.second.memory);
			}
		}

		for (auto& uniform : uniformSemantics)
		{
			recordUpdateUniformSemanticToExternalMemory(cbuff, uniform.second.uniformLocation, uniform.second.memory);
		}
	}
	if (recordDrawCalls)
	{
		pvr::assets::Mesh& mesh = *rmesh.assetMesh;
		if (rmesh.ibo.isValid())
		{
			cbuff->drawIndexed(mesh.getBatchFaceOffset(batchId) * 3, mesh.getNumFaces(batchId) * 3);
		}
		else
		{
			cbuff->drawArrays(0, mesh.getNumVertices());
		}
	}
}

////////// RENDERING COMMANDS ///////// RENDERING COMMANDS ///////// RENDERING COMMANDS /////////



bool RenderManager::buildRenderObjects()
{
	// Distinct combinations of pipelines used for each mesh -> Used for the attribute layouts.
	std::map<PipelineSet, AttributeConfiguration> pipeSets;

	std::map<StringHash, AttributeConfiguration*> pipeToAttribMapping;

	// PREPARE DATA STRUCTURES
	// Creation of the renderables will have to be 2 passes: We will need to go through the entire list
	// ones to spot any duplicates etc. between pipelines, attribute layouts etc.
	// And then we will need to go through the list another time to actually create the pipelines and
	// map them to the meshes.

	prepareDataStructures(*this, meshAttributeLayout, pipeSets);


	//FOR EACH AND EVERY DISTINCT COMBINATION OF PIPELINES, create the attribute layouts. Do the merging, mapping etc.
	createAttributeConfigurations(*this, pipeSets, pipeToAttribMapping, meshAttributeLayout, true);

	// PHASE 3: Create the pipelines. We could not do that in the previous phase as we did not have the complete
	// picture of which meshes render with what pipelines, in order to generate the correct input assembly.
	createPipelines(*this, pipeToAttribMapping);

	// PHASE 4: Create the VBOs. Same. We also remap the actual data.
	createVbos(*this, meshAttributeLayout);

	// PHASE 5: Create all the descriptor sets, populate them with the UBOs/SSBOs, and the textures
	createDescriptorSetsAndBuffers(*this, meshAttributeLayout, context->getDefaultDescriptorPool());

	// PHASE 5: Fix the dynamic offsets for all nodes. This is necessary, because the dynamic setup only happens after the last call.
	fixDynamicOffsets(*this);
	// PHASE 7: ????

	// PHASE 8: Profit!
	return true;
}
}
}

//!\endcond