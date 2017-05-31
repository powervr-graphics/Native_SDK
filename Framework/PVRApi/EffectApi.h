<<<<<<< HEAD
/*!****************************************************************************************************************
\file         PVRApi/EffectApi.h
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Main Interface for all LEGACY PFXEffect.
*******************************************************************************************************************/
=======
/*!
\brief Main Interface for all LEGACY PFXEffect.
\file PVRApi/EffectApi.h
\copyright Copyright (c) Imagination Technologies Limited.
*/
>>>>>>> 1776432f... 4.3
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRCore/IO/BufferStream.h"
#include "PVRAssets/SkipGraph.h"
#include "PVRAssets/FileIO/PFXReader.h"
#include "PVRAssets/Effect.h"
#include "PVRApi/ApiObjects.h"
#include "PVRApi/ApiObjects/GraphicsPipeline.h"


//!\cond NO_DOXYGEN
namespace pvr {
<<<<<<< HEAD
class IGraphicsContext;
=======
>>>>>>> 1776432f... 4.3
namespace legacyPfx {

/// <summary>A struct containing GL uniform data.</summary>
struct EffectApiSemantic
{
	int32 location;       /*!< Api uniform location */
	uint32 semanticIndex;     /*!< Index; for example two semantics might be LIGHTPOSITION0 and LIGHTPOSITION1 */
	string variableName;      /*!< The name of the variable referenced in shader code */
};


/// <summary>Texture wrapper for texture2d, texture3d and sampler.</summary>
struct EffectApiTextureSampler
{
	StringHash name;     //< texture name
	StringHash fileName; //< filename
	uint8 unit;     //< The bound texture unit
	api::Sampler sampler;//< sampler
	void* userData; //< user data (Optional)
	uint32 flags;
	api::TextureView texture; //< Texture View
public:
	/// <summary>ctor</summary>
	EffectApiTextureSampler() : userData(NULL)
	{
	}

<<<<<<< HEAD
	/*!****************************************************************************************************************
	\brief Get texture type.
	*******************************************************************************************************************/
=======
	/// <summary>Get texture type.</summary>
>>>>>>> 1776432f... 4.3
	types::ImageViewType getTextureViewType() const
	{
		return texture->getViewType();
	}

<<<<<<< HEAD
	/*!****************************************************************************************************************
	\brief Initialize this.
	\param[in] effectDelegate effect's asset provider
	*******************************************************************************************************************/
	Result init(api::AssetLoadingDelegate& effectDelegate)
=======
	/// <summary>Initialize this.</summary>
	/// <param name="effectDelegate">effect's asset provider</param>
	Result init(utils::AssetLoadingDelegate& effectDelegate)
>>>>>>> 1776432f... 4.3
	{
		return  effectDelegate.effectOnLoadTexture(fileName, texture) ? Result::Success : Result::NotFound;
	}
	virtual ~EffectApiTextureSampler() {}
};

/// <summary>Effect native shader program wrapper.</summary>
struct EffectApiProgram
{
	native::HPipeline program;
	void* userData;

	EffectApiProgram() : userData(NULL) {}
};

/// <summary>Effect shader info.</summary>
struct EffectApiShader
{
	BufferStream::ptr_type data; //< data stream
<<<<<<< HEAD
	types::ShaderType	type; //< shader type, e.g VertexShader, FragmentShader
	bool isBinary;//< is shader binary format
	types::ShaderBinaryFormat	binaryFormat; //< shader binary format

	/*!****************************************************************************************************************
	\brief ctor.
	*******************************************************************************************************************/
=======
	types::ShaderType type; //< shader type, e.g VertexShader, FragmentShader
	bool isBinary;//< is shader binary format
	types::ShaderBinaryFormat binaryFormat; //< shader binary format

	/// <summary>ctor.</summary>
>>>>>>> 1776432f... 4.3
	EffectApiShader() : isBinary(false), binaryFormat(types::ShaderBinaryFormat::Unknown) {}
};

namespace impl {
/// <summary>Common API interface.</summary>
class EffectApi_
{
public:
<<<<<<< HEAD
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
=======
	/// <summary>ctor.</summary>
	/// <param name="context">The context that API objects by this effect will be created on</param>
	/// <param name="effectDelegate">A class that will be used to load assets required by this effect</param>
	EffectApi_(const GraphicsContext& context, utils::AssetLoadingDelegate& effectDelegate);

	/// <summary>Initialize effect Api with effect and PipelineCreateParam.</summary>
	/// <param name="effect">An assets effect to initialize with</param>
	/// <param name="pipeDesc">A pipeline create params object. At least some of the state will be overwritten.
	/// </param>
	/// <returns>return pvr::Result::Success on success</returns>
	Result init(const assets::Effect& effect, api::GraphicsPipelineCreateParam& pipeDesc);

	/// <summary>Destructor.</summary>
	~EffectApi_() { if (_isLoaded) { destroy(); } }

	/// <summary>Deletes the managed resources.</summary>
>>>>>>> 1776432f... 4.3
	void destroy();

	/// <summary>Get pipeline (const).</summary>
	/// <returns>pvr::api::ParentableGraphicsPipeline</returns>
	const api::ParentableGraphicsPipeline& getPipeline()const { return _pipe; }

	/// <summary>Get pipeline.</summary>
	/// <returns>pvr::api::ParentableGraphicsPipeline</returns>
	api::ParentableGraphicsPipeline& getPipeline() { return _pipe; }

	/// <summary>Get texture by texture id.</summary>
	/// <param name="texture">texture id</param>
	/// <returns>EffectApiTextureSampler</returns>
	const EffectApiTextureSampler& getTexture(size_t texture)const { return _effectTexSamplers[texture]; }

	/// <summary>Get texture by name.</summary>
	/// <param name="semantic">name</param>
	/// <returns>EffectApiTextureSampler</returns>
	const EffectApiTextureSampler& getTextureByName(const StringHash& semantic)const { return _effectTexSamplers[semantic]; }

	/// <summary>Get texture id.</summary>
	/// <param name="semantic">texture name</param>
	pvr::uint32 getTextureIndex(const StringHash& semantic) const { return (pvr::uint32)_uniforms.getIndex(semantic); }

	/// <summary>Returns a uniform semantic by id.</summary>
	/// <returns>This object</returns>
	const EffectApiSemantic& getUniform(int32 idx) const { return _uniforms[idx]; }

	/// <summary>Get uniform index by semantic. Returns -1 on not found.</summary>
	/// <param name="semantic">A semantic name to get the uniform index.</param>
	/// <returns>The index of the uniform. -1 on not found.</returns>
	pvr::uint32 getUniformIndex(const StringHash& semantic) const { return (pvr::uint32)_uniforms.getIndex(semantic); }

	/// <summary>Returns a attribute semantic.</summary>
	/// <returns>This object</returns>
	const EffectApiSemantic& getAttribute(int32 idx) const { return _attributes[idx]; }

	/// <summary>Return attribute index by semantic.</summary>
	/// <param name="semantic">The attribute semantic to get the name of.</param>
	/// <returns>The index of the attribute. -1 on not found.</returns>
	pvr::uint32 getAttributeIndex(const StringHash& semantic) const { return (uint32)_attributes.getIndex(semantic); }

	/// <summary>Set a texture to the specified index.</summary>
	/// <param name="index">The index of the texture to set</param>
	/// <param name="texture">The texture</param>
	void setTexture(uint32 index, const api::TextureView& texture);

	/// <summary>Set a sampler to the specified index.</summary>
	/// <param name="index">Index of the sampler set</param>
	/// <param name="sampler">The sampler</param>
	void setSampler(uint32 index, api::Sampler sampler);

	/// <summary>Sets the default value for a uniform semantic. This value will be used if no uniform is explicitly set
	/// by the user</summary>
	/// <param name="name">Name of a uniform</param>
	/// <param name="defaultValue">The default value</param>
	void setDefaultUniformValue(const char8* const name, const assets::EffectSemanticData& defaultValue);

<<<<<<< HEAD
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
=======
	/// <summary>Removes a given semantic ID from the 'known' uniform semantic list and re-parses the effect update the
	/// uniform table.</summary>
	/// <param name="uiSemanticID">The semanticId to remove</param>
	/// <returns>pvr::Result::Success on success</returns>
	Result removeUniformSemantic(uint32 uiSemanticID);

	/// <summary>Return the name of the effect name.</summary>
	/// <returns>The name of the effect name.</returns>
	const std::string& getEffectName() const { return _assetEffect.material.getEffectName(); }

	/// <summary>Return the filename of the effect.</summary>
	/// <returns>The filename of the effect.</returns>
	const std::string& getEffectFileName() const { return _assetEffect.fileName; }

	/// <summary>Get the number of uniforms used by the effect.</summary>
	/// <returns>The number of uniforms used by the effect.</returns>
	uint32 getNumUnknownUniformsFound() const { return _numUnknownUniforms; }

	/// <summary>Get the DescriptorSet used by the effect</summary>
	/// <returns>The DescriptorSet used by the effect</returns>
	const api::DescriptorSet& getDescriptorSet() const { return _descriptorSet; }

	const assets::Effect& getEffectAsset()const {return _assetEffect;}
>>>>>>> 1776432f... 4.3
private:
	Result loadShadersForEffect(api::Shader& vertexShader, api::Shader& fragmentShader);
	Result loadTexturesForEffect();
	//Build the uniform table from a list of known semantics.
	Result buildSemanticTables(uint32& uiUnknownSemantics);
	Result createDescriptors();

protected:
<<<<<<< HEAD
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
=======
	bool _isLoaded;
	EffectApiProgram _program;      // Loaded program
	assets::Effect _assetEffect;
	IndexedArray<EffectApiTextureSampler> _effectTexSamplers; // Array of loaded textures
	IndexedArray<EffectApiSemantic, StringHash> _uniforms; // Array of found uniforms
	IndexedArray<EffectApiSemantic, StringHash> _attributes; // Array of found attributes
	api::ParentableGraphicsPipeline _pipe;
	utils::AssetLoadingDelegate* _delegate;
	uint32 _numUnknownUniforms;
	GraphicsContext _context;
	api::DescriptorSetLayout _descriptorSetLayout;
	api::DescriptorSet _descriptorSet;
>>>>>>> 1776432f... 4.3

private:
	Result apiOnLoadTexture(const char* fileName, uint32 flags, native::HTexture* outTexHandle);
	EffectApiProgram& operator=(const EffectApiProgram&);

	uint32 loadSemantics(const IGraphicsContext* context, bool isAttribute);
};
}// impl


typedef RefCountedResource< impl::EffectApi_> EffectApi;
<<<<<<< HEAD
=======

/// <summary>Create a new EffectApi object.</summary>
/// <param name="effectDesc">The CreateParameters used to create the object.</param>
/// <param name="pipeDesc">The CreateParameters will be used to create the GraphicsPipeline for this effect.
/// </param>
/// <param name="effectDelegate">The effectDelegate which will be used to load any required resources for the
/// creation of this object. The effectDelegate is in most cases the main application class of the user, after
/// implemented the AssetLoadingDelegate interface</param>
/// <returns>A newly constructed EffectAPI object. Contains NULL if creation failed.</returns>
inline EffectApi createEffectApi(const GraphicsContext& ctx, ::pvr::assets::Effect& effectDesc,
                          api::GraphicsPipelineCreateParam& pipeDesc, utils::AssetLoadingDelegate& effectDelegate)
{
	EffectApi effect;
	effect.construct(ctx, effectDelegate);
	if (effect->init(effectDesc, pipeDesc) != Result::Success)
	{
		effect.reset();
	}
	return effect;
}


>>>>>>> 1776432f... 4.3
}// legacyPfx
}// pvr
 //!\endcond