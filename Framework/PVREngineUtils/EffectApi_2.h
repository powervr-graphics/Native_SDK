/*!
\brief INTERNAL TO RenderManager.
\file PVREngineUtils/EffectApi.h
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVREngineUtils/StructuredMemory.h"
#include "PVRAssets/FileIO/PFXReader.h"
#include "PVRAssets/Effect_2.h"
#include "PVRApi/ApiObjects.h"
#include "PVRApi/ApiObjects/GraphicsPipeline.h"
//!cond NO_DOXYGEN
//EffectAPI does not work at all as an object - it needs the RenderManager to actually work.
//So it makes sense to be removed as a class and its functionality rolled into the RenderManager
namespace pvr {

class IGraphicsContext;
namespace utils {

namespace effect {
using ::pvr::assets::effect::PipelineCondition;

/// <summary>The ObjectSemantic struct. Contains semantic of a descriptor object</summary>
struct ObjectSemantic
{
	StringHash name;//< Object name
	uint16 set;//< Descriptor set index
	uint16 binding;//< Descriptor set binding index
	ObjectSemantic() {}
	ObjectSemantic(StringHash name, uint16 set, uint16 binding): name(name), set(set), binding(binding) {}
	bool operator<(const ObjectSemantic& rhs) const { return name < rhs.name; }
	bool operator==(const ObjectSemantic& rhs) const { return name == rhs.name; }
	bool operator!=(const ObjectSemantic& rhs) const { return name != rhs.name; }
};

/// <summary>Effect's uniform semantic</summary>
struct UniformSemantic : public assets::effect::UniformSemantic
{
	UniformSemantic() {}
	UniformSemantic(StringHash semantic, StringHash variableName) { this->semantic = semantic; this->variableName = variableName; }
	bool operator<(const UniformSemantic& rhs) const { return semantic < rhs.semantic; }
	bool operator==(const UniformSemantic& rhs) const { return semantic == rhs.semantic; }
	bool operator!=(const UniformSemantic& rhs) const { return semantic != rhs.semantic; }
};

/// <summary>Effect's attribute semantic</summary>
struct AttributeSemantic
{
	StringHash semantic;
};

/// <summary>Effect's buffer semantic</summary>
struct BufferEntrySemantic
{
	StringHash semantic;
	StringHash buffer;
	FreeValue value;
	// DEFINE WHERE AND WHAT IT IS
};

struct ConditionalPipeline
{
	DynamicArray<assets::effect::PipelineCondition> conditions;
	DynamicArray<StringHash> identifiers;
	StringHash pipeline;
};

struct SubpassGroup
{
	StringHash name;
	DynamicArray<ConditionalPipeline> pipelines;
};

struct Subpass
{
	DynamicArray<SubpassGroup> groups;
};

/// <summary>Effect's pass</summary>
struct Pass
{
	api::RenderPass renderPass;//!< renderpass to use
	api::FboSet fbos;//!< framebuffer objects (per swapchain) to render in to
	DynamicArray<Subpass> subpasses; //!< list of subpasses it contains
};

typedef assets::effect::TextureRef TextureRef;
struct TextureInfo : public TextureRef
{
	api::Sampler sampler;
};

/// <summary>Effect's buffer defitions</summary>
struct InputAttachmentInfo : public TextureRef
{
	api::TextureView tex;
	InputAttachmentInfo() {}
	InputAttachmentInfo(api::TextureView tex, StringHash textureName, uint8 set, uint8 binding, StringHash variableName) :
		tex(tex), TextureRef(textureName, set, binding, variableName) {}
};


/*!
   \brief Effect's buffer defitions
 */
struct BufferDef
{
	utils::StructuredMemoryView bufferView;
	types::BufferBindingUse allSupportedBindings;
	bool isDynamic;
	types::VariableScope scope;
	uint16 numBuffers;
	BufferDef() : allSupportedBindings(types::BufferBindingUse(0)), isDynamic(false), scope(types::VariableScope::Unknown), numBuffers(1) {}
};

typedef assets::effect::BufferRef BufferRef;

/// <summary>Effect's pipeline definitions. Contains a single pipeline data</summary>
struct PipelineDef
{
	api::GraphicsPipelineCreateParam createParam;
	bool isCreateParamDone;
	Multi<api::DescriptorSet> fixedDescSet[4];
	bool descSetIsFixed[4]; // If it is "fixed", it means that it is set by the PFX and no members of it are exported through semantics
	bool descSetIsMultibuffered[4]; // If it is "fixed", it means that it is set by the PFX and no members of it are exported through semantics
	bool descSetExists[4]; // If it is "fixed", it means that it is set by the PFX and no members of it are exported through semantics
	pvr::ContiguousMap<StringHash, TextureInfo> textureSamplersByTexName; //!<First item is texture name
	pvr::ContiguousMap<StringHash, TextureInfo> textureSamplersByTexSemantic; //!<First item is texture semantic
	pvr::ContiguousMap<StringHash, InputAttachmentInfo> inputAttachments[(uint32)FrameworkCaps::MaxSwapChains];
	pvr::ContiguousMap<StringHash, BufferRef> modelScopeBuffers; //!<First item is buffer name
	pvr::ContiguousMap<StringHash, BufferRef> effectScopeBuffers; //!<First item is buffer name
	pvr::ContiguousMap<StringHash, BufferRef> nodeScopeBuffers; //!<First item is buffer name
	pvr::ContiguousMap<StringHash, BufferRef> batchScopeBuffers; //!<First item is buffer name
	pvr::ContiguousMap<StringHash, ObjectSemantic> textures;
	pvr::ContiguousMap<StringHash, UniformSemantic> uniforms;
	std::vector<assets::effect::AttributeSemantic> attributes;//!< Effect attributes
	PipelineDef() : isCreateParamDone(false)
	{
		descSetIsFixed[0] = descSetIsFixed[1] = descSetIsFixed[2] = descSetIsFixed[3] = true;
		descSetIsMultibuffered[0] = descSetIsMultibuffered[1] = descSetIsMultibuffered[2] = descSetIsMultibuffered[3] = false;
		memset(descSetExists, 0, sizeof(bool) * ARRAY_SIZE(descSetExists));
	}
};



namespace impl {

/// <summary>Common API interface.</summary>
class Effect_
{
public:
	typedef assets::effect::Effect AssetEffect;

	/// <summary>ctor.</summary>
	/// <param name="context">The context that API objects by this effect will be created on</param>
	/// <param name="effectDelegate">A class that will be used to load assets required by this effect</param>
	Effect_(const GraphicsContext& context, AssetLoadingDelegate& effectDelegate);

	/// <summary>Create and initialize effect Api with a pvr::assets::effect::Effect object</summary>
	/// <param name="effect">The assets effect that is used by this Effect</param>
	/// <returns>return pvr::Result::Success on success</returns>
	bool init(const assets::effect::Effect& effect);

	/// <summary>Get the exact string that the Effect object is using to define its API.</summary>
	/// <returns>The exact string that the Effect object is using to define its API.</returns>
	const StringHash& getApiString() const { return apiString; }

	/// <summary>Get number of passes</summary>
	/// <returns>The number of passes</returns>
	uint32 getNumPasses() const { return (uint32)passes.size(); }

	/// <summary>Get the context that this Effect object belongs to.</summary>
	/// <returns>The context that this Effect object belongs to.</returns>
	GraphicsContext& getContext() { return context; }

	/// <summary>Get the context that this Effect object belongs to.</summary>
	/// <returns>The context that this Effect object belongs to.</returns>
	const GraphicsContext& getContext() const { return context; }

	/// <summary>Get a pipeline layout by its pipeline name.</summary>
	/// <param name="name">The name of a pipeline (as defined in the Effect).</param>
	/// <returns>The pipeline layout.</returns>
	api::PipelineLayout getPipelineLayout(const StringHash& name) const
	{
		auto it = pipelineDefinitions.find(name);
		if (it == pipelineDefinitions.end()) { return api::PipelineLayout(); }
		return it->second.createParam.pipelineLayout;
	}

	/// <summary>Get a reference to one of the effect's passes</summary>
	/// <param name="passIndex">The index of the pass (the order with which it was defined/added to the effect)
	/// </param>
	/// <returns>The pass with the specified index. If it does not exist, undefined behaviour.</returns>
	const Pass& getPass(uint32 passIndex) const
	{
		return passes[passIndex];
	}

	/// <summary>Get a reference to one of the effect's passes</summary>
	/// <param name="passIndex">The index of the pass (the order with which it was defined/added to the effect)
	/// </param>
	/// <returns>The pass with the specified index. If it does not exist, undefined behaviour.</returns>
	Pass& getPass(uint32 passIndex)
	{
		return passes[passIndex];
	}

	/// <summary>Get all passes (implementation defined)</summary>
	/// <returns>An implementation-defined container of all passes in this effect.</returns>
	const std::vector<Pass>& getPasses() const
	{
		return passes;
	}

	/// <summary>Get a reference to a Buffer. Null if not exists.</summary>
	/// <param name="name">The name of the Buffer</param>
	/// <returns>A pointer to the buffer. If a buffer with the specifed name is not found, NULL.</returns>
	BufferDef* getBuffer(const StringHash& name)
	{
		auto it = bufferDefinitions.find(name);
		return (it != bufferDefinitions.end() ? &it->second : NULL);
	}

	/// <summary>Get a reference to a Buffer. Null if not exists.</summary>
	/// <param name="name">The name of the Buffer</param>
	/// <returns>A pointer to the buffer. If a buffer with the specifed name is not found, NULL.</returns>
	const BufferDef* getBuffer(const StringHash& name) const
	{
		auto it = bufferDefinitions.find(name);
		return it != bufferDefinitions.end() ? &it->second : NULL;
	}

	/// <summary>Get the list of all buffers as a raw container</summary>
	/// <returns>An implementation-defined container with all the buffers</returns>
	const std::map<StringHash, BufferDef>& getBuffers() const
	{
		return bufferDefinitions;
	}

	/// <summary>Get a texture by its name</summary>
	/// <param name="name">The name of the texture</param>
	/// <returns>The texture. If not found, empty texture handle.</returns>
	api::TextureView getTexture(const StringHash& name) const
	{
		auto it = textures.find(name);
		if (it == textures.end()) { return api::TextureView(); }
		return it->second;
	}

	/// <summary>Get information about texture/sampler binding info by pipeline name and semantic</summary>
	/// <param name="pipelineName">The name of the pipeline</param>
	/// <param name="textureSemantic">The semantic of the texture</param>
	/// <param name="out_sampler">The texture sampler object will be stored here</param>
	/// <param name="out_setIdx">The descriptor set index of the texture will be stored here</param>
	/// <param name="out_bindingPoint[out]">The index of the texture in the set will be stored here.</param>
	/// <returns>Return true on success, false if not found.</returns>
	bool getTextureInfo(const StringHash& pipelineName, const StringHash& textureSemantic, api::Sampler& out_sampler,
	                    uint8& out_setIdx, uint8& out_bindingPoint) const
	{
		out_setIdx = (uint8) - 1; out_bindingPoint = (uint8) - 1;
		auto it = pipelineDefinitions.find(pipelineName);
		if (it == pipelineDefinitions.end())
		{
			Log("EffectApi::getSamplerForTextureBySemantic: Pipeline [%d] not found.", pipelineName.c_str());
			return false;
		}
		auto it2 = it->second.textureSamplersByTexSemantic.find(textureSemantic);
		if (it2 == it->second.textureSamplersByTexSemantic.end())
		{
			Log("EffectApi::getSamplerForTextureBySemantic: Texture with semantic [%d] not found for pipeline [%d].",
			    textureSemantic.c_str(), pipelineName.c_str());
			return false;
		}
		out_setIdx = it2->second.set; out_bindingPoint = it2->second.binding; out_sampler = it2->second.sampler;
		return true;
	}

	/// <summary>Get a Pipeline definition object</summary>
	/// <param name="pipelineName">The name of the pipeline object to get</param>
	/// <returns>Return pipeline defintion if found else return NULL.</returns>
	const PipelineDef* getPipelineDefinition(const StringHash& pipelineName) const
	{
		auto it = pipelineDefinitions.find(pipelineName);
		if (it == pipelineDefinitions.end())
		{
			Log("Pipeline definition %s referenced in Effect: %s not found ", pipelineName.c_str(), name.c_str());
			return NULL;
		}
		return &it->second;
	}


	/// <summary>Get a Pipeline definition object</summary>
	/// <param name="pipelineName">The name of the pipeline object to get</param>
	/// <returns>Return pipeline defintion if found else return NULL.</returns>
	PipelineDef* getPipelineDefinition(const StringHash& pipelineName)
	{
		auto it = pipelineDefinitions.find(pipelineName);
		if (it == pipelineDefinitions.end())
		{
			Log("EffectApi: Pipeline definition %s referenced in Effect: %s not found ", pipelineName.c_str(), name.c_str());
			return NULL;
		}
		return &it->second;
	}

	/// <summary>Get the create params for a pipeline object</summary>
	/// <param name="pipelineName">The name of the pipeline object to get</param>
	/// <returns>The create params of the object to get.</returns>
	const api::GraphicsPipelineCreateParam& getPipelineCreateParam(const StringHash& name) const
	{
		auto it = pipelineDefinitions.find(name);
		if (it == pipelineDefinitions.end()) { Log("Pipeline create param %s not found", name.c_str()); }
		return it->second.createParam;
	}

	/// <summary>Get the create params for a pipeline object</summary>
	/// <param name="pipelineName">The name of the pipeline object to get</param>
	/// <returns>The create params of the object to get.</returns>
	api::GraphicsPipelineCreateParam& getPipelineCreateParam(const StringHash& name)
	{
		auto it = pipelineDefinitions.find(name);
		if (it == pipelineDefinitions.end()) { Log("Pipeline create param %s not found", name.c_str());  }
		return it->second.createParam;
	}

	/// <summary>Set a uniform by semantic</summary>
	/// <param name="semantic">The semantic for the uniform to set</param>
	/// <param name="value">A FreeValue object containing the value to set to</param>
	/// <returns>Return true on success</returns>
	bool setUniform(const StringHash& semantic, const FreeValue& value);

	/// <summary>Set Texture</summary>
	/// <param name="semanticId">Texture semantic id</param>
	/// <param name="texture">The texture to set</param>
	/// <returns>Return true on success</returns>
	bool setTexture(const StringHash& semanticId, const api::TextureView& texture);

	/// <summary>Get a descriptor set of a pipeline</summary>
	/// <param name="pipelineName">The name of the pipeline whose set to get</param>
	/// <param name="setIndex">The index of the set in the pipeline</param>
	/// <returns>The set to get</returns>
	const api::DescriptorSet& getDescriptorSet(const StringHash& pipelineName, uint32 setIndex)const;

	/// <summary>Set a texture to the specified index.</summary>
	/// <param name="index">The index of the texture to set</param>
	/// <param name="texture">The texture to set</param>
	void setTexture(uint32 index, const api::TextureView& texture);

	/// <summary>Set a sampler to the specified index.</summary>
	/// <param name="index">Index of the sampler set</param>
	/// <param name="sampler">The sampler</param>
	void setSampler(uint32 index, api::Sampler sampler);

	/// <summary>Return the name of the effect name.</summary>
	/// <returns>The name of the effect name.</returns>
	const std::string& getEffectName() const { return name; }

	/// <summary>Return the filename of the effect.</summary>
	/// <returns>The filename of the effect.</returns>
	const std::string& getEffectFileName() const;

	/// <summary>Get the number of uniforms used by the effect.</summary>
	/// <returns>The number of uniforms used by the effect.</returns>
	uint32 getNumUnknownUniformsFound() const;

	/// <summary>Return the effect asset that was used to create this object</summary>
	/// <returns>The effect asset that was used to create this object</returns>
	const assets::effect::Effect& getEffectAsset()const { return assetEffect; }

	/// <summary>Get the descriptor pool used by this object</summary>
	/// <returns>the descriptor pool used by this object</returns>
	api::DescriptorPool getDescriptorPool() { return descriptorPool; }

	/// <summary>Get the asset loading delegate used by this object</summary>
	/// <returns>The asset loading delegate used by this object</returns>
	AssetLoadingDelegate* getAssetLoadingDelegate() { return delegate; }

	/// <summary>Register a uniform semantic</summary>
	/// <param name="pipeline">The pipeline to add the semantic for</param>
	/// <param name="semantic">The semantic to add</param>
	/// <param name="variableName">The variable name of the semantic</param>
	void registerUniformSemantic(StringHash pipeline, StringHash semantic, StringHash variableName);

	/// <summary>Register a buffer semantic</summary>
	/// <param name="pipeline">The pipeline to add the semantic for</param>
	/// <param name="semantic">The semantic name of the buffer</param>
	/// <param name="set">The index of the descriptor set to add the buffer to</param>
	/// <param name="binding">The index of the buffer in the descriptor set</param>
	void registerBufferSemantic(StringHash pipeline, StringHash semantic, uint16 set, uint16 binding);

	/// <summary>Register a texture semantic</summary>
	/// <param name="pipeline">The pipeline to add the semantic for</param>
	/// <param name="semantic">The semantic name of the texture</param>
	/// <param name="set">The index of the descriptor set to add the texture to</param>
	/// <param name="binding">The index of the texture in the descriptor set</param>
	void registerTextureSemantic(StringHash pipeline, StringHash semantic, uint16 set, uint16 binding);

	/// <summary>Register a semantic that is accessed as an entry in a buffer</summary>
	/// <param name="pipeline">The pipeline to add the semantic for</param>
	/// <param name="semantic">The semantic name of the entry</param>
	/// <param name="entryIndex">The index of the semantic in its buffer</param>
	/// <param name="set">The index of the descriptor of the semantic's buffer</param>
	/// <param name="binding">The index of the semantic's buffer in its descriptor set</param>
	void registerBufferEntrySemantic(StringHash pipeline, StringHash semantic, uint16 entryIndex, uint16 set, uint16 binding);

protected:
	GraphicsContext context;
	AssetLoadingDelegate* delegate;
	assets::effect::Effect assetEffect;
	StringHash apiString;
	StringHash name;

	std::map<StringHash, api::TextureView> textures;
	std::map<StringHash, BufferDef> bufferDefinitions;
	std::map<StringHash, PipelineDef> pipelineDefinitions;
	api::DescriptorPool descriptorPool;
	std::vector<Pass> passes;
private:
	Result apiOnLoadTexture(const char* fileName, uint32 flags, native::HTexture* outTexHandle);
};
}// impl
typedef RefCountedResource<impl::Effect_> EffectApi;
}
}// api
}// pvr
//!endcond NO_DOXYGEN