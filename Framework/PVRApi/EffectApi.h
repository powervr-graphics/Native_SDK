/*!****************************************************************************************************************
\file         PVRApi/EffectApi.h
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Main Interface for all LEGACY PFXEffect.
*******************************************************************************************************************/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRCore/BufferStream.h"
#include "PVRAssets/SkipGraph.h"
#include "PVRAssets/FileIO/PFXReader.h"
#include "PVRAssets/Effect.h"
#include "PVRApi/ApiObjects.h"
#include "PVRApi/ApiObjects/GraphicsPipeline.h"

namespace pvr {
class IGraphicsContext;
namespace legacyPfx {

/*!*****************************************************************************************************************
\brief        A struct containing GL uniform data.
*******************************************************************************************************************/
struct EffectApiSemantic
{
	int32 location;				/*!< Api uniform location */
	uint32 semanticIndex;			/*!< Index; for example two semantics might be LIGHTPOSITION0 and LIGHTPOSITION1 */
	string variableName;			/*!< The name of the variable referenced in shader code */
};


/*!*****************************************************************************************************************
\brief        Texture wrapper for texture2d, texture3d and sampler.
*******************************************************************************************************************/
struct EffectApiTextureSampler
{
	StringHash name;     //< texture name
	StringHash fileName; //< filename
	uint8 unit;	    //< The bound texture unit
	api::Sampler sampler;//< sampler
	void* userData; //< user data (Optional)
	uint32 flags;
	api::TextureView texture; //< Texture View
public:
	/*!****************************************************************************************************************
	\brief ctor
	*******************************************************************************************************************/
	EffectApiTextureSampler() : userData(NULL)
	{
	}

	/*!****************************************************************************************************************
	\brief Get texture type.
	*******************************************************************************************************************/
	types::ImageViewType getTextureViewType() const
	{
		return texture->getViewType();
	}

	/*!****************************************************************************************************************
	\brief Initialize this.
	\param[in] effectDelegate effect's asset provider
	*******************************************************************************************************************/
	Result init(api::AssetLoadingDelegate& effectDelegate)
	{
		return  effectDelegate.effectOnLoadTexture(fileName, texture) ? Result::Success : Result::NotFound;
	}
	virtual ~EffectApiTextureSampler() {}
};

/*!****************************************************************************************************************
\brief  Effect native shader program wrapper.
*******************************************************************************************************************/
struct EffectApiProgram
{
	native::HPipeline program;
	void* userData;

	EffectApiProgram() : userData(NULL) {}
};

/*!****************************************************************************************************************
\brief  Effect shader info.
*******************************************************************************************************************/
struct EffectApiShader
{
	BufferStream::ptr_type data; //< data stream
	types::ShaderType	type; //< shader type, e.g VertexShader, FragmentShader
	bool isBinary;//< is shader binary format
	types::ShaderBinaryFormat	binaryFormat; //< shader binary format

	/*!****************************************************************************************************************
	\brief ctor.
	*******************************************************************************************************************/
	EffectApiShader() : isBinary(false), binaryFormat(types::ShaderBinaryFormat::Unknown) {}
};

namespace impl {
/*!*****************************************************************************************************************
\brief Common API interface.
*******************************************************************************************************************/
class EffectApi_
{
public:
	/*!****************************************************************************************************************
	\brief ctor.
	\param[in] context The context that API objects by this effect will be created on
	\param[in] effectDelegate A class that will be used to load assets required by this effect
	*******************************************************************************************************************/
    EffectApi_(GraphicsContext& context, api::AssetLoadingDelegate& effectDelegate);

	/*!****************************************************************************************************************
	\brief Initialize effect Api with effect and PipelineCreateParam.
	\param[in] effect
	\param[in] pipeDesc
	\return return pvr::Result::Success on success
	*******************************************************************************************************************/
	Result init(const assets::Effect& effect, api::GraphicsPipelineCreateParam& pipeDesc);

	/*!*****************************************************************************************************************
	\brief Destructor.
	*******************************************************************************************************************/
	~EffectApi_() { if (m_isLoaded) { destroy(); } }

	/*!*****************************************************************************************************************
	\brief Deletes the managed resources.
	*******************************************************************************************************************/
	void destroy();

	/*!****************************************************************************************************************
	\brief Get pipeline (const).
	\return pvr::api::ParentableGraphicsPipeline
	*******************************************************************************************************************/
	const api::ParentableGraphicsPipeline& getPipeline()const { return m_pipe; }

	/*!****************************************************************************************************************
	\brief Get pipeline.
	\return pvr::api::ParentableGraphicsPipeline
	*******************************************************************************************************************/
	api::ParentableGraphicsPipeline& getPipeline() { return m_pipe; }

	/*!****************************************************************************************************************
	\brief Get texture by texture id.
	\param[in] texture texture id
	\return EffectApiTextureSampler
	*******************************************************************************************************************/
	const EffectApiTextureSampler& getTexture(size_t texture)const { return m_effectTexSamplers[texture]; }

	/*!****************************************************************************************************************
	\brief Get texture by name.
	\param[in] semantic name
	\return EffectApiTextureSampler
	*******************************************************************************************************************/
	const EffectApiTextureSampler& getTextureByName(const StringHash& semantic)const { return m_effectTexSamplers[semantic]; }

	/*!****************************************************************************************************************
	\brief Get texture id.
	\param[in] semantic texture name
	*******************************************************************************************************************/
	pvr::uint32 getTextureIndex(const StringHash& semantic) const { return (pvr::uint32)m_uniforms.getIndex(semantic); }

	/*!*****************************************************************************************************************
	\brief	 Returns a uniform semantic by id.
	\return	 const EffectApiSemantic&
	*******************************************************************************************************************/
	const EffectApiSemantic& getUniform(int32 idx) const { return m_uniforms[idx]; }

	/*!****************************************************************************************************************
	\brief Get uniform index by semantic.
	\param[in] semantic
	\return index
	*******************************************************************************************************************/
	pvr::uint32 getUniformIndex(const StringHash& semantic) const { return (pvr::uint32)m_uniforms.getIndex(semantic); }

	/*!*****************************************************************************************************************
	\brief	Returns a attribute semantic.
	\return	const EffectApiSemantic&
	*******************************************************************************************************************/
	const EffectApiSemantic& getAttribute(int32 idx) const { return m_attributes[idx]; }

	/*!****************************************************************************************************************
	\brief Return attribute index by semantic.
	\param[in] semantic
	\return index
	*******************************************************************************************************************/
	pvr::uint32 getAttributeIndex(const StringHash& semantic) const { return (uint32)m_attributes.getIndex(semantic); }

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

	/*!*****************************************************************************************************************
	\brief	Sets the default value for a uniform semantic. This value will be used if no uniform is explicitly set
	        by the user
	\param[in] name Name of a uniform
	\param[in] defaultValue The default value
	*******************************************************************************************************************/
	void setDefaultUniformValue(const char8* const name, const assets::EffectSemanticData& defaultValue);

	/*!*****************************************************************************************************************
	\brief	    Removes a given semantic ID from the 'known' uniform semantic list and re-parses the effect  update
	            the uniform table.
	\param[in]	uiSemanticID The semanticId to remove
	\return		pvr::Result::Success on success
	*******************************************************************************************************************/
	Result removeUniformSemantic(uint32 uiSemanticID);

	/*!****************************************************************************************************************
	\brief Return the name of the effect name.
	\return The name of the effect name.
	*******************************************************************************************************************/
	const std::string& getEffectName() const { return m_assetEffect.material.getEffectName(); }

	/*!****************************************************************************************************************
	\brief Return the filename of the effect.
	\return The filename of the effect.
	*******************************************************************************************************************/
	const std::string& getEffectFileName() const { return m_assetEffect.fileName; }

	/*!****************************************************************************************************************
	\brief Get the number of uniforms used by the effect.
	\return The number of uniforms used by the effect.
	*******************************************************************************************************************/
	uint32 getNumUnknownUniformsFound() const { return m_numUnknownUniforms; }

	/*!****************************************************************************************************************
	\brief Get the DescriptorSet used by the effect
	\return The DescriptorSet used by the effect
	*******************************************************************************************************************/
    const api::DescriptorSet& getDescriptorSet() const { return m_descriptorSet; }

	const assets::Effect& getEffectAsset()const {return m_assetEffect;}
private:
	Result loadShadersForEffect(api::Shader& vertexShader, api::Shader& fragmentShader);
	Result loadTexturesForEffect();
	//Build the uniform table from a list of known semantics.
	Result buildSemanticTables(uint32& uiUnknownSemantics);
	Result createDescriptors();

protected:
	bool m_isLoaded;
	EffectApiProgram m_program;			// Loaded program
	assets::Effect m_assetEffect;
	IndexedArray<EffectApiTextureSampler> m_effectTexSamplers; // Array of loaded textures
	IndexedArray<EffectApiSemantic, StringHash> m_uniforms; // Array of found uniforms
	IndexedArray<EffectApiSemantic, StringHash> m_attributes; // Array of found attributes
	api::ParentableGraphicsPipeline m_pipe;
    api::AssetLoadingDelegate* m_delegate;
	uint32 m_numUnknownUniforms;
	GraphicsContext m_context;
    api::DescriptorSetLayout m_descriptorSetLayout;
    api::DescriptorSet m_descriptorSet;

private:
	Result apiOnLoadTexture(const char* fileName, uint32 flags, native::HTexture* outTexHandle);
	EffectApiProgram& operator=(const EffectApiProgram&);

	uint32 loadSemantics(const IGraphicsContext* context, bool isAttribute);
};
}// impl
typedef RefCountedResource< impl::EffectApi_> EffectApi;
}// legacyPfx
}// pvr
