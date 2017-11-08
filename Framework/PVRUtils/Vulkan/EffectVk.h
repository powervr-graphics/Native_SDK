/*!
\brief INTERNAL TO RenderManager.
\file PVRUtils/Vulkan/EffectVk.h
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/PVRCore.h"
#include "PVRUtils/StructuredMemory.h"
#include "PVRAssets/Effect_2.h"
#include "PVRVk/GraphicsPipelineVk.h"
//!cond NO_DOXYGEN
//EffectAPI does not work at all as an object - it needs the RenderManager to actually work.
//So it makes sense to be removed as a class and its functionality rolled into the RenderManager
namespace pvr {

//!\cond NO_DOXYGEN
namespace utils {
struct ImageUploadResults;
}
//!\endcond
namespace effect {
using pvr::assets::effect::PipelineCondition;

/// <summary>The ObjectSemantic struct. Contains the semantic binding of a descriptor object, i.e. the
/// connection of a Buffer or Texture, with a semantic "string" understood by the application. This object
/// does not actually carry the semantic string as this will be the key to a map where it will be stored.
/// </summary>
struct ObjectSemantic
{
	StringHash name; //!< The name(identifier) of the object in the effect. NOT the semantic.
	uint16_t set; //!< Descriptor set index
	uint16_t binding;//!< Descriptor set binding index
	/// <summary>Constructor. Non initializing.</summary>
	ObjectSemantic() {}
	/// <summary>Constructor from individual members</summary>
	/// <param name="name">The name of the object in the effect. NOT the semantic.</param>
	/// <param name="set">The descriptor set where the object is bound.</param>
	/// <param name="binding">The index in the descriptor set where the object is bound.</param>
	ObjectSemantic(StringHash name, uint16_t set, uint16_t binding): name(name), set(set), binding(binding) {}
	/// <summary>Less than operator. Works by comparing the name field</summary>
	/// <param name="rhs">Right hand side of the operator</param>
	/// <returns>True if the names compare "less than", otherwise false</param>
	bool operator<(const ObjectSemantic& rhs) const { return name < rhs.name; }
	/// <summary>Equality operator. Works by comparing the name field</summary>
	/// <param name="rhs">Right hand side of the operator</param>
	/// <returns>True if the names compare "equal", otherwise false</param>
	bool operator==(const ObjectSemantic& rhs) const { return name == rhs.name; }
	/// <summary>Inequality operator. Works by comparing the name field</summary>
	/// <param name="rhs">Right hand side of the operator</param>
	/// <returns>True if the names compare "equal", otherwise false</param>
	bool operator!=(const ObjectSemantic& rhs) const { return name != rhs.name; }
};

/// <summary>Effect's uniform semantic. A Uniform semantic is intended to connect
/// a declared variable in the effect with a "semantic name" that the client
/// application may understand.</summary>
struct UniformSemantic : public assets::effect::UniformSemantic
{
	/// <summary>Constructor. Non-initializing.</summary>
	UniformSemantic() {}
	/// <summary>Constructor from individual fields.</summary>
	/// <param name="semantic">The semantic name of the uniform</param>
	/// <param name="variableName">The variable name (in the shader) of the uniform</param>
	UniformSemantic(StringHash semantic, StringHash variableName)
	{
		this->semantic = semantic; this->variableName = variableName;
	}

	/// <summary>Less than operator. Works by comparing the name field</summary>
	/// <param name="rhs">Right hand side of the operator</param>
	/// <returns>True if the names compare "less than", otherwise false</param>
	bool operator<(const UniformSemantic& rhs) const { return semantic < rhs.semantic; }
	/// <summary>Equality operator. Works by comparing the name field</summary>
	/// <param name="rhs">Right hand side of the operator</param>
	/// <returns>True if the names compare "equal", otherwise false</param>
	bool operator==(const UniformSemantic& rhs) const { return semantic == rhs.semantic; }
	/// <summary>Inequality operator. Works by comparing the name field</summary>
	/// <param name="rhs">Right hand side of the operator</param>
	/// <returns>True if the names compare "equal", otherwise false</param>
	bool operator!=(const UniformSemantic& rhs) const { return semantic != rhs.semantic; }
};

/// <summary>A conditional Pipeline is effectively a Pipeline entry in a SubpassGroup that
/// is decorated by "conditions" that decide which members of that subpass group will be
/// rendered with which pipeline. Conditions are usually the presense or absence of certain
/// semantics</summary>
struct ConditionalPipeline
{
	DynamicArray<assets::effect::PipelineCondition> conditions; //!< The conditions that, if all are satisfied, this pipeline will be selected
	DynamicArray<StringHash> identifiers; //!< This list contains "custom identifier" strings that decorate the pipeline. They are set with the "AdditionalExport" condition
	StringHash pipeline; //!< A name reference to a pipeline definition (its name).
};

/// <summary>The SubpassGroup is a part of a subpass that contains conditional pipelines. Objects are
/// added to specified groups and then the pipelines from that group get selected automatically</summary>
struct SubpassGroup
{
	StringHash name; //!< The name of the subpass group
	DynamicArray<ConditionalPipeline> pipelines; //!< The pipelines that make up that group
};

/// <summary>A Subpass represents a rendering operation to the Framebuffer or an intermediate result.
/// It is composed of subpass groups.</summary>
struct Subpass
{
	DynamicArray<SubpassGroup> groups; //!< The groups composing the subpass
};

/// <summary>A Pass represents a full rendering operation to a final, physical render target.
/// It is composed by one or more, implicit or explicit, subpasses. Its intermediate subpasses
/// may be rendering to intermediate results while the final one writes to the actual render target.</summary>
struct Pass
{
	pvrvk::RenderPass renderPass;//!< The actual Renderpass object to use
	pvrvk::Framebuffer framebuffers[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];//!< The Render Targets to use (one per swapchain)
	DynamicArray<Subpass> subpasses; //!< The list of subpasses composing this render pass
};

typedef assets::effect::TextureRef TextureRef; //!< A reference, in a pipeline, of a Texture

/// <summary>A reference to a texture plus its associated Sampler object</summary>
struct TextureInfo : public TextureRef
{
	pvrvk::Sampler sampler; //!< The sampler object used to sample the texture
};

/// <summary>A description of a Texture used as an Input attachment</summary>
struct InputAttachmentInfo : public TextureRef
{
	pvrvk::ImageView tex; //!< The image view associated with the attachment
	/// <summary>Constructor</summary>
	InputAttachmentInfo() {}

	/// <summary>Constructor from individual fields</summary>
	/// <param name="tex">The ImageView of the attachment</param>
	/// <param name="textureName">The name (identifier) of the Texture Definition this reference refers to</param>
	/// <param name="set">Which descriptor set to bind this reference to</param>
	/// <param name="binding">Which index in the descriptor set is this reference added to</param>
	/// <param name="variableName">How is this variable named in the shader.</param>
	InputAttachmentInfo(
	  pvrvk::ImageView tex, StringHash textureName, uint8_t set,
	  uint8_t binding, StringHash variableName) :
		tex(tex), TextureRef(textureName, set, binding, variableName) {}
};



/// <summary>A definition of a Buffer Object. All buffer references refer to such an object. It
/// contains all the information about the buffer object, including layout, the actual Vulkan object,
/// information about multibuffering etc.</summary>
struct BufferDef
{
	utils::StructuredBufferView bufferView; //!< The layout of the buffer memory (cooked, final)
	pvrvk::Buffer buffer; //!< The Vulka buffer object of this definition
	pvr::utils::StructuredMemoryDescription memoryDescription; //!< The layout of the buffer memory (initial data)
	BufferUsageFlags allSupportedBindings; //!< All the types of descriptor bindings this buffer allows
	bool isDynamic; //!< True if it a Dynamic buffer, otherwise false
	VariableScope scope; //!< The Scope of this entire buffer, if it is set as a whole (rather than by individual entries )
	uint16_t numBuffers; //!< The number of "instances" used if multibuffered (1 for single-buffered)
	BufferDef() :
		allSupportedBindings(BufferUsageFlags(0)),
		isDynamic(false), scope(VariableScope::Unknown), numBuffers(1) {}
};

typedef assets::effect::BufferRef BufferRef; //!< A reference to a Buffer object

/// <summary>Effect's pipeline definitions. Contains all the data a pipeline contains.</summary>
struct PipelineDef
{
	pvrvk::GraphicsPipelineCreateInfo createParam; //!< The Graphics Pipeline Create Param used by this pipeline

	Multi<pvrvk::DescriptorSet> fixedDescSet[4]; //!< A "Fixed" descriptor set is a descriptor set that does not export any semantics, and is set by the PFX. For example, a descriptor set containing only a texture "marble.pvr" which was set through the effect and has no semantic defined, would be fixed. A fixed descriptor set will exist in this field. Non-fixed set IDs will be null here.
	bool descSetIsFixed[4]; //!< Describes which set IDs are fixed. Equivalently, of the sets in fixedDescSet exist
	bool descSetIsMultibuffered[4]; //!< Describes which set IDs are multibuffered.
	bool descSetExists[4]; //!< Describes which set IDs are actually used.
	pvr::ContiguousMap<StringHash, TextureInfo> textureSamplersByTexName; //!< Mapping of textures to their texture names
	pvr::ContiguousMap<StringHash, TextureInfo> textureSamplersByTexSemantic; //!< Mapping of texture references to their texture semantics
	pvr::ContiguousMap<StringHash, InputAttachmentInfo> inputAttachments[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)]; //!< The inputs attachments used
	pvr::ContiguousMap<StringHash, BufferRef> modelScopeBuffers; //!< All model-scope buffers (buffers whose data are sourced by the model, or in general must be changed every time the model changes). First item is buffer name
	pvr::ContiguousMap<StringHash, BufferRef> effectScopeBuffers; //!< All effect-scope buffers (buffers whose data remain constant throughout the effect).First item is buffer name
	pvr::ContiguousMap<StringHash, BufferRef> nodeScopeBuffers; //!< All node-scope buffers (buffers whose data change with every node / are sourced by the node). First item is buffer name
	pvr::ContiguousMap<StringHash, BufferRef> batchScopeBuffers; //!< All batch-scope buffers (buffers whose data change with each bone batch. Normally, only legacy-exported bones should use that). First item is buffer name
	pvr::ContiguousMap<StringHash, ObjectSemantic> textures; //!< List of Texture semantics.
	pvr::ContiguousMap<StringHash, UniformSemantic> uniforms; //!< List of Uniform semantics.
	std::vector<assets::effect::AttributeSemantic> attributes;//!< List of Attribute semantics
	/// <summary>Constructor. Initializes to zero</summary>
	PipelineDef()
	{
		descSetIsFixed[0] = descSetIsFixed[1] = descSetIsFixed[2] = descSetIsFixed[3] = true;
		descSetIsMultibuffered[0] = descSetIsMultibuffered[1] = descSetIsMultibuffered[2] = descSetIsMultibuffered[3] = false;
		memset(descSetExists, 0, sizeof(bool) * ARRAY_SIZE(descSetExists));
	}
};
namespace impl {

/// <summary>The definition of an Effect. This class should normally not be used externally as it is specific to the RenderManager.</summary>
class Effect_
{
public:
	typedef assets::effect::Effect AssetEffect; //!< Import the effect asset for brevity

	/// <summary>Constructor.</summary>
	/// <param name="device">The context that API objects by this effect will be created on</param>
	Effect_(const pvrvk::DeviceWeakPtr& device);

	/// <summary>Create and initialize effect Api with a pvr::assets::effect::Effect object</summary>
	/// <param name="effect">The assets effect that is used by this Effect</param>
	/// <param name="swapchain">The swapchain object which represents on-screen memory</param>
	/// <param name="cmdBuffer">A Command Buffer to use for uploading textures and other operations while building</param>
	/// <param name="assetProvider">Normally, a pointer to your application class (it is automatically an IAssetProvider
	/// through PVRShell), used to load textures from the filesystem/assetsystem</param>
	/// <param name="uploadResults">A list of futures that will be populated with all the texture
	/// futures that the asynchronous loading provides</param>
	/// <returns>return pvr::Result::Success on success</returns>
	bool init(const assets::effect::Effect& effect, pvrvk::Swapchain& swapchain,
	          pvrvk::CommandBuffer& cmdBuffer, IAssetProvider& assetProvider,
	          std::vector<utils::ImageUploadResults>& uploadResults);

	/// <summary>Get the exact string that the Effect object is using to define its API.</summary>
	/// <returns>The exact string that the Effect object is using to define its API.</returns>
	const StringHash& getApiString() const { return _apiString; }

	/// <summary>Get number of passes</summary>
	/// <returns>The number of passes</returns>
	uint32_t getNumPasses() const { return static_cast<uint32_t>(_passes.size()); }

	/// <summary>Get the device that this Effect object belongs to.</summary>
	/// <returns>The device that this Effect object belongs to.</returns>
	pvrvk::DeviceWeakPtr& getDevice() { return _device; }

	/// <summary>Get the context that this Effect object belongs to.</summary>
	/// <returns>The context that this Effect object belongs to.</returns>
	const pvrvk::DeviceWeakPtr& getDevice() const { return _device; }

	/// <summary>Get a pipeline layout by its pipeline name.</summary>
	/// <param name="name">The name of a pipeline (as defined in the Effect).</param>
	/// <returns>The pipeline layout.</returns>
	pvrvk::PipelineLayout getPipelineLayout(const StringHash& name) const
	{
		auto it = _pipelineDefinitions.find(name);
		if (it == _pipelineDefinitions.end()) { return pvrvk::PipelineLayout(); }
		return it->second.createParam.pipelineLayout;
	}

	/// <summary>Get a reference to one of the effect's passes</summary>
	/// <param name="passIndex">The index of the pass (the order with which it was defined/added to the effect)
	/// </param>
	/// <returns>The pass with the specified index. If it does not exist, undefined behaviour.</returns>
	const Pass& getPass(uint32_t passIndex) const
	{
		return _passes[passIndex];
	}

	/// <summary>Get a reference to one of the effect's passes</summary>
	/// <param name="passIndex">The index of the pass (the order with which it was defined/added to the effect)
	/// </param>
	/// <returns>The pass with the specified index. If it does not exist, undefined behaviour.</returns>
	Pass& getPass(uint32_t passIndex)
	{
		return _passes[passIndex];
	}

	/// <summary>Get all passes (implementation defined)</summary>
	/// <returns>An implementation-defined container of all passes in this effect.</returns>
	const std::vector<Pass>& getPasses() const
	{
		return _passes;
	}

	/// <summary>Get a reference to a Buffer. Null if not exists.</summary>
	/// <param name="name">The name of the Buffer</param>
	/// <returns>A pointer to the buffer. If a buffer with the specifed name is not found, NULL.</returns>
	BufferDef* getBuffer(const StringHash& name)
	{
		auto it = _bufferDefinitions.find(name);
		return (it != _bufferDefinitions.end() ? &it->second : NULL);
	}

	/// <summary>Get a reference to a Buffer. Null if not exists.</summary>
	/// <param name="name">The name of the Buffer</param>
	/// <returns>A pointer to the buffer. If a buffer with the specifed name is not found, NULL.</returns>
	const BufferDef* getBuffer(const StringHash& name) const
	{
		auto it = _bufferDefinitions.find(name);
		return it != _bufferDefinitions.end() ? &it->second : NULL;
	}

	/// <summary>Get number of swapchain (const)</summary>
	/// <returns>const pvrvk::Swapchain&</returns>
	const pvrvk::Swapchain& getSwapchain()const
	{
		return _swapchain;
	}


	/// <summary>Get the list of all buffers as a raw container</summary>
	/// <returns>An implementation-defined container with all the buffers</returns>
	const std::map<StringHash, BufferDef>& getBuffers() const
	{
		return _bufferDefinitions;
	}

	/// <summary>Get a texture by its name</summary>
	/// <param name="name">The name of the texture</param>
	/// <returns>The texture. If not found, empty texture handle.</returns>
	pvrvk::ImageView getTexture(const StringHash& name) const
	{
		auto it = _textures.find(name);
		if (it == _textures.end()) { return pvrvk::ImageView(); }
		return it->second;
	}

	/// <summary>Get information about texture/sampler binding info by pipeline name and semantic</summary>
	/// <param name="pipelineName">The name of the pipeline</param>
	/// <param name="textureSemantic">The semantic of the texture</param>
	/// <param name="out_sampler">The texture sampler object will be stored here</param>
	/// <param name="out_setIdx">The descriptor set index of the texture will be stored here</param>
	/// <param name="out_bindingPoint">The index of the texture in the set will be stored here.</param>
	/// <returns>Return true on success, false if not found.</returns>
	bool getTextureInfo(
	  const StringHash& pipelineName, const StringHash& textureSemantic,
	  pvrvk::Sampler& out_sampler, uint8_t& out_setIdx, uint8_t& out_bindingPoint) const
	{
		out_setIdx = static_cast<uint8_t>(-1); out_bindingPoint = static_cast<uint8_t>(-1);
		auto it = _pipelineDefinitions.find(pipelineName);
		if (it == _pipelineDefinitions.end())
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
		auto it = _pipelineDefinitions.find(pipelineName);
		if (it == _pipelineDefinitions.end())
		{
			Log("Pipeline definition %s referenced in Effect: %s not found ", pipelineName.c_str(), _name.c_str());
			return nullptr;
		}
		return &it->second;
	}


	/// <summary>Get a Pipeline definition object</summary>
	/// <param name="pipelineName">The name of the pipeline object to get</param>
	/// <returns>Return pipeline defintion if found else return NULL.</returns>
	PipelineDef* getPipelineDefinition(const StringHash& pipelineName)
	{
		auto it = _pipelineDefinitions.find(pipelineName);
		if (it == _pipelineDefinitions.end())
		{
			Log("EffectApi: Pipeline definition %s referenced in Effect: %s not found ", pipelineName.c_str(), _name.c_str());
			return NULL;
		}
		return &it->second;
	}

	/// <summary>Get the create params for a pipeline object</summary>
	/// <param name="pipelineName">The name of the pipeline object to get</param>
	/// <returns>The create params of the object to get.</returns>
	const pvrvk::GraphicsPipelineCreateInfo& getPipelineCreateParam(const StringHash& pipelineName) const
	{
		auto it = _pipelineDefinitions.find(pipelineName);
		if (it == _pipelineDefinitions.end()) { Log("Pipeline create param %s not found", pipelineName.c_str()); }
		return it->second.createParam;
	}

	/// <summary>Get the create params for a pipeline object</summary>
	/// <param name="pipelineName">The name of the pipeline object to get</param>
	/// <returns>The create params of the object to get.</returns>
	pvrvk::GraphicsPipelineCreateInfo& getPipelineCreateParam(const StringHash& pipelineName)
	{
		auto it = _pipelineDefinitions.find(pipelineName);
		if (it == _pipelineDefinitions.end()) { Log("Pipeline create param %s not found", pipelineName.c_str());  }
		return it->second.createParam;
	}

	///// <summary>Set a uniform by semantic</summary>
	///// <param name="semantic">The semantic for the uniform to set</param>
	///// <param name="value">A FreeValue object containing the value to set to</param>
	///// <returns>Return true on success</returns>
	//bool setUniform(const StringHash& semantic, const FreeValue& value);

	///// <summary>Set Texture</summary>
	///// <param name="semanticId">Texture semantic id</param>
	///// <param name="texture">The texture to set</param>
	///// <returns>Return true on success</returns>
	//bool setTexture(const StringHash& semanticId, const pvrvk::ImageView& texture);

	///// <summary>Get a descriptor set of a pipeline</summary>
	///// <param name="pipelineName">The name of the pipeline whose set to get</param>
	///// <param name="setIndex">The index of the set in the pipeline</param>
	///// <returns>The set to get</returns>
	//const pvrvk::DescriptorSet& getDescriptorSet(const StringHash& pipelineName, uint32_t setIndex)const;

	///// <summary>Set a texture to the specified index.</summary>
	///// <param name="index">The index of the texture to set</param>
	///// <param name="texture">The texture to set</param>
	//void setTexture(uint32_t index, const pvrvk::ImageView& texture);

	///// <summary>Set a sampler to the specified index.</summary>
	///// <param name="index">Index of the sampler set</param>
	///// <param name="sampler">The sampler</param>
	//void setSampler(uint32_t index, pvrvk::Sampler sampler);

	/// <summary>Return the name of the effect name.</summary>
	/// <returns>The name of the effect name.</returns>
	const std::string& getEffectName() const { return _name; }

	///// <summary>Return the filename of the effect.</summary>
	///// <returns>The filename of the effect.</returns>
	//const std::string& getEffectFileName() const;

	///// <summary>Get the number of uniforms used by the effect.</summary>
	///// <returns>The number of uniforms used by the effect.</returns>
	//uint32_t getNumUnknownUniforms() const;

	/// <summary>Return the effect asset that was used to create this object</summary>
	/// <returns>The effect asset that was used to create this object</returns>
	const assets::effect::Effect& getEffectAsset()const { return _assetEffect; }

	/// <summary>Get the descriptor pool used by this object</summary>
	/// <returns>the descriptor pool used by this object</returns>
	pvrvk::DescriptorPool getDescriptorPool() { return _descriptorPool; }

	/// <summary>Register a uniform semantic</summary>
	/// <param name="pipeline">The pipeline to add the semantic for</param>
	/// <param name="semantic">The semantic to add</param>
	/// <param name="variableName">The variable name of the semantic</param>
	void registerUniformSemantic(StringHash pipeline, StringHash semantic, StringHash variableName);

	///// <summary>Register a buffer semantic</summary>
	///// <param name="pipeline">The pipeline to add the semantic for</param>
	///// <param name="semantic">The semantic name of the buffer</param>
	///// <param name="set">The index of the descriptor set to add the buffer to</param>
	///// <param name="binding">The index of the buffer in the descriptor set</param>
	//void registerBufferSemantic(StringHash pipeline, StringHash semantic, uint16_t set, uint16_t binding);

	/// <summary>Register a texture semantic</summary>
	/// <param name="pipeline">The pipeline to add the semantic for</param>
	/// <param name="semantic">The semantic name of the texture</param>
	/// <param name="set">The index of the descriptor set to add the texture to</param>
	/// <param name="binding">The index of the texture in the descriptor set</param>
	void registerTextureSemantic(StringHash pipeline, StringHash semantic, uint16_t set, uint16_t binding);

	///// <summary>Register a semantic that is accessed as an entry in a buffer</summary>
	///// <param name="pipeline">The pipeline to add the semantic for</param>
	///// <param name="semantic">The semantic name of the entry</param>
	///// <param name="entryIndex">The index of the semantic in its buffer</param>
	///// <param name="set">The index of the descriptor of the semantic's buffer</param>
	///// <param name="binding">The index of the semantic's buffer in its descriptor set</param>
	//void registerBufferEntrySemantic(StringHash pipeline, StringHash semantic, uint16_t entryIndex, uint16_t set, uint16_t binding);

private:
	pvrvk::DeviceWeakPtr _device;
	assets::effect::Effect _assetEffect;
	StringHash _apiString;
	StringHash _name;

	std::map<StringHash, pvrvk::ImageView> _textures;
	std::map<StringHash, BufferDef> _bufferDefinitions;
	std::map<StringHash, PipelineDef> _pipelineDefinitions;
	pvrvk::DescriptorPool _descriptorPool;
	std::vector<Pass> _passes;
	pvrvk::Swapchain         _swapchain;
	bool buildRenderObjects(pvrvk::CommandBuffer& texUploadCmdBuffer,
	                        IAssetProvider& assetProvider, std::vector<utils::ImageUploadResults>& uploadResults);

};
}// impl
typedef RefCountedResource<impl::Effect_> EffectApi; //!< A smart pointer to an EffectApi
}
}// pvr
//!endcond NO_DOXYGEN
