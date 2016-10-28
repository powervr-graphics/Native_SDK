/*!****************************************************************************************************************
\file         PVRApi/EffectApi.h
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        INTERNAL TO RenderManager.
*******************************************************************************************************************/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRAssets/FileIO/PFXReader.h"
#include "PVRAssets/Effect_2.h"
#include "PVRApi/ApiObjects.h"
#include "PVRApi/ApiObjects/GraphicsPipeline.h"
#include "PVRApi/StructuredMemory.h"

//EffectAPI does not work at all as an object - it needs the RenderManager to actually work.
//So it makes sense to be removed as a class and its functionality rolled into the RenderManager
namespace pvr {

class IGraphicsContext;
namespace api {

namespace effect {
using ::pvr::assets::effect::PipelineCondition;

/*!
\brief The ObjectSemantic struct. Contains semantic of a descriptor object
 */
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

/*!
\brief Effect's uniform semantic
 */
struct UniformSemantic : public assets::effect::UniformSemantic
{
	UniformSemantic() {}
	UniformSemantic(StringHash semantic, StringHash variableName) { this->semantic = semantic; this->variableName = variableName; }
	bool operator<(const UniformSemantic& rhs) const { return semantic < rhs.semantic; }
	bool operator==(const UniformSemantic& rhs) const { return semantic == rhs.semantic; }
	bool operator!=(const UniformSemantic& rhs) const { return semantic != rhs.semantic; }
};

/*!
   \brief Effect's attribute semantic
 */
struct AttributeSemantic
{
	StringHash semantic;
};

/*!
   \brief Effect's buffer semantic
 */
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

struct ConditionalPipelines
{
	DynamicArray<ConditionalPipeline> pipelines;
};

/*!
\brief Effect's pass
 */
struct Pass
{
	RenderPass renderPass;//!< renderpass to use
    FboSet fbos;//!< framebuffer objects (per swapchain) to render in to
    DynamicArray<ConditionalPipelines> subpasses; //!< list of subpasses it contains
};

typedef assets::effect::TextureRef TextureRef;
struct TextureInfo : public TextureRef
{
	Sampler sampler;
};

/*!
   \brief Effect's buffer defitions
 */
struct BufferDef
{
	utils::StructuredMemoryView bufferView;
	types::BufferViewTypes allSupportedBindings;
	types::VariableScope scope;
	uint16 numBuffers;
	BufferDef() : allSupportedBindings(types::BufferViewTypes(0)), scope(types::VariableScope::Unknown), numBuffers(1) {}
};

typedef assets::effect::BufferRef BufferRef;

/*!
\brief Effect's pipeline definitions. Contains a single pipeline data
 */
struct PipelineDef
{
	GraphicsPipelineCreateParam createParam;
	bool isCreateParamDone;
	Multi<DescriptorSet> fixedDescSet[4];
	bool descSetIsFixed[4]; // If it is "fixed", it means that it is set by the PFX and no members of it are exported through semantics
	bool descSetIsMultibuffered[4]; // If it is "fixed", it means that it is set by the PFX and no members of it are exported through semantics
	bool descSetExists[4]; // If it is "fixed", it means that it is set by the PFX and no members of it are exported through semantics
    pvr::ContiguousMap<StringHash, TextureInfo> textureSamplersByTexName; //!<First item is texture name
    pvr::ContiguousMap<StringHash, TextureInfo> textureSamplersByTexSemantic; //!<First item is texture semantic
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
		descSetExists[0] = descSetExists[1] = descSetExists[2] = descSetExists[3] = false;
	}
};



namespace impl {

/*!*****************************************************************************************************************
\brief Common API interface.
*******************************************************************************************************************/
class Effect_
{
public:
	typedef assets::effect::Effect AssetEffect;

	/*!****************************************************************************************************************
	\brief ctor.
	\param[in] context The context that API objects by this effect will be created on
	\param[in] effectDelegate A class that will be used to load assets required by this effect
	*******************************************************************************************************************/
	Effect_(GraphicsContext& context, AssetLoadingDelegate& effectDelegate);

	/*!****************************************************************************************************************
	\brief Create and initialize effect Api with a pvr::assets::effect::Effect object
	\param[in] effect The assets effect that is used by this Effect
	\return return pvr::Result::Success on success
	*******************************************************************************************************************/
	bool init(const assets::effect::Effect& effect);

	/*!****************************************************************************************************************
	\brief Get the exact string that the Effect object is using to define its API.
	*******************************************************************************************************************/
	const StringHash& getApiString() const { return apiString; }

    /*!****************************************************************************************************************
    \brief Get number of passes
     ******************************************************************************************************************/
	uint32 getNumPasses() const { return (uint32)passes.size(); }

	/*!****************************************************************************************************************
	\brief Get the context that this Effect object belongs to.
	*******************************************************************************************************************/
	GraphicsContext& getContext() { return context; }

	/*!****************************************************************************************************************
	\brief Get the context that this Effect object belongs to.
	*******************************************************************************************************************/
	const GraphicsContext& getContext() const { return context; }

	/*!****************************************************************************************************************
	\brief Get pipeline (const).
	\return pvr::api::ParentableGraphicsPipeline
	*******************************************************************************************************************/
	PipelineLayout getPipelineLayout(const StringHash& name) const
	{
		auto it = pipelineDefinitions.find(name);
		if (it == pipelineDefinitions.end()) { return PipelineLayout(); }
		return it->second.createParam.pipelineLayout;
	}

	/*!****************************************************************************************************************
	\brief Get pipeline (const).
	\return pvr::api::ParentableGraphicsPipeline
	*******************************************************************************************************************/
	const Pass& getPass(uint32 passIndex) const
	{
		return passes[passIndex];
	}

	/*!****************************************************************************************************************
	\brief Get pipeline (const).
	\return pvr::api::ParentableGraphicsPipeline
	*******************************************************************************************************************/
	const std::vector<Pass>& getPasses() const
	{
		return passes;
	}

	/*!****************************************************************************************************************
	\brief Get pipeline (const).
	\return pvr::api::ParentableGraphicsPipeline
	*******************************************************************************************************************/
	Pass& getPass(uint32 passIndex)
	{
		return passes[passIndex];
	}

    /*!****************************************************************************************************************
    \brief Get Buffer
    \param name Buffer name
    *****************************************************************************************************************/
	BufferDef* getBuffer(const StringHash& name)
	{
		auto it = bufferDefinitions.find(name);
		return (it != bufferDefinitions.end() ? &it->second : NULL);
	}

    /*!****************************************************************************************************************
    \brief Get buffer (const)
    \param name Buffer name
    *****************************************************************************************************************/
	const BufferDef* getBuffer(const StringHash& name) const
	{
		auto it = bufferDefinitions.find(name);
		return it != bufferDefinitions.end() ? &it->second : NULL;
	}

    /*!
    \brief Get all buffers (const)
     */
	const std::map<StringHash, BufferDef>& getBuffers() const
	{
		return bufferDefinitions;
	}

	/*!****************************************************************************************************************
	\brief Get pipeline (const).
	\return pvr::api::ParentableGraphicsPipeline
	*******************************************************************************************************************/
	TextureView getTexture(const StringHash& name) const
	{
		auto it = textures.find(name);
		if (it == textures.end()) { return TextureView(); }
		return it->second;
	}

    /*!
    \brief Get texture info
    \param pipelineName
    \param textureSemantic
    \param out_sampler
    \param out_setIdx
    \param out_bindingPoint
    \return Return true on success
     */
    bool getTextureInfo(const StringHash& pipelineName, const StringHash& textureSemantic, Sampler& out_sampler,
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

    /*!****************************************************************************************************************
    \brief Get Pipeline defintion (const)
    \param pipelineName Pipeline name
    \return Return pipeline defintion if found else return NULL.
    *****************************************************************************************************************/
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


    /*!****************************************************************************************************************
    \brief Get Pipeline defintion
    \param pipelineName Pipeline name
    \return Return pipeline defintion if found else return NULL.
    *****************************************************************************************************************/
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

	/*!****************************************************************************************************************
	\brief Get pipeline (const).
	\return pvr::api::ParentableGraphicsPipeline
	*******************************************************************************************************************/
	const GraphicsPipelineCreateParam& getPipelineCreateParam(const StringHash& name) const
	{
		auto it = pipelineDefinitions.find(name);
		if (it == pipelineDefinitions.end()) { Log("Pipeline create param %s not found", name.c_str()); }
		return it->second.createParam;
	}

	/*!****************************************************************************************************************
	\brief Get pipeline (const).
	\return pvr::api::ParentableGraphicsPipeline
	*******************************************************************************************************************/
	GraphicsPipelineCreateParam& getPipelineCreateParam(const StringHash& name)
	{
		auto it = pipelineDefinitions.find(name);
		if (it == pipelineDefinitions.end()) { Log("Pipeline create param %s not found", name.c_str());  }
		return it->second.createParam;
	}

    /*!
    \brief Set uniform
    \param semanticId Uniform semantic id
    \param value Uniform value
    \return Return true on success
     */
    bool setUniform(const StringHash& semanticId, FreeValue& value);

    /*!
    \brief Set Texture
    \param semanticId Texture semantic id
    \param texture
    \return Return true on success
     */
	bool setTexture(const StringHash& semanticId, TextureView& texture);

    /*!
    \brief Get descriptor set of a pipeline
    \param pipeline
    \param index
    \return
     */
    const DescriptorSet& getDescriptorSet(const StringHash& pipeline, uint32 index)const;

	/*!*****************************************************************************************************************
	\brief		Set a texture to the specified index.
	\param[in]	index The index of the texture to set
	\param[in]  texture The texture
	*******************************************************************************************************************/
	void setTexture(uint32 index, const api::TextureView& texture);

	/*!****************************************************************************************************************
	\brief	Set a sampler to the specified index.
	\param	index Index of the sampler set
	\param	sampler The sampler
	*******************************************************************************************************************/
	void setSampler(uint32 index, api::Sampler sampler);

	/*!****************************************************************************************************************
	\brief Return the name of the effect name.
	\return The name of the effect name.
	*******************************************************************************************************************/
	const std::string& getEffectName() const { return name; }

	/*!****************************************************************************************************************
	\brief Return the filename of the effect.
	\return The filename of the effect.
	*******************************************************************************************************************/
	const std::string& getEffectFileName() const;

	/*!****************************************************************************************************************
	\brief Get the number of uniforms used by the effect.
	\return The number of uniforms used by the effect.
	*******************************************************************************************************************/
	uint32 getNumUnknownUniformsFound() const;

    /*!
    \brief Return the effect asset
     */
    const assets::effect::Effect& getEffectAsset()const { return assetEffect; }

    /*!
    \brief Return the descriptor pool
     */
	DescriptorPool getDescriptorPool() { return descriptorPool; }

    /*!
       \brief Return the asset loading delegate
     */
	AssetLoadingDelegate* getAssetLoadingDelegate() { return delegate; }

    /*!
       \brief Register uniform semantic
       \param pipeline
       \param semantic
       \param variableName
     */
	void registerUniformSemantic(StringHash pipeline, StringHash semantic, StringHash variableName);

    /*!
       \brief Register buffer semantic
       \param pipeline
       \param semantic
       \param set
       \param binding
     */
	void registerBufferSemantic(StringHash pipeline, StringHash semantic, uint16 set, uint16 binding);

    /*!
       \brief Register texture semantic
       \param pipeline
       \param semantic
       \param set
       \param binding
     */
	void registerTextureSemantic(StringHash pipeline, StringHash semantic, uint16 set, uint16 binding);

    /*!
       \brief register buffer entry semantic
       \param pipeline
       \param semantic
       \param entryIndex
       \param set
       \param binding
     */
	void registerBufferEntrySemantic(StringHash pipeline, StringHash semantic, uint16 entryIndex, uint16 set, uint16 binding);

protected:
	GraphicsContext context;
	AssetLoadingDelegate* delegate;
	assets::effect::Effect assetEffect;
	StringHash apiString;
	StringHash name;

	std::map<StringHash, TextureView> textures;
	std::map<StringHash, BufferDef> bufferDefinitions;
	std::map<StringHash, PipelineDef> pipelineDefinitions;
	DescriptorPool descriptorPool;
	std::vector<Pass> passes;
private:
	Result apiOnLoadTexture(const char* fileName, uint32 flags, native::HTexture* outTexHandle);
};
}// impl
typedef RefCountedResource<impl::Effect_> EffectApi;
}
}// api
}// pvr
