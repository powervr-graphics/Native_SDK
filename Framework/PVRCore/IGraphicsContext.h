/*!*********************************************************************************************************************
\file         PVRCore\IGraphicsContext.h
\author       PowerVR by Imagination, Developer Technology Team.
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the interface that the very important GraphicsContext classes implement.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/Defines.h"
#include "PVRCore/ForwardDecApiObjects.h"
#include "PVRCore/OSManager.h"
#include "PVRCore/RefCounted.h"
#include "PVRCore/Rectangle.h"
#include "PVRCore/IPlatformContext.h"
#include <map>
#include <bitset>

namespace pvr {
class IGraphicsContext;
class Stream;

/*!********************************************************************************************************************
\brief        Main PVRAssets namespace. In the context of PVRCore, contains enumerations that conceptually belong to
PVRAssets but are shared between different PowerVR Framework projects.
***********************************************************************************************************************/
namespace assets {
class Texture;
struct Effect;
}//assets

/*!********************************************************************************************************************
\brief     Main PVRApi Namespace. In the PVRCore projects, contains interfaces and enumerations that conceptually
        belong to PVRApi but need to be shared with other PVR Framework projects (GraphicsContext interface, enums)
***********************************************************************************************************************/
namespace api {
typedef types::SamplerCreateParam SamplerCreateParam;
}


//!\cond NO_DOXYGEN
struct ApiCapabilitiesPrivate
{
	std::bitset<32> nativeSupport; //!< Will be set if natively supported
	std::bitset<32> extensionSupport; //!< Will be set if supported through extension
	uint16 maxglslesversion;
	uint32 uboOffsetAlignment;
	uint32 ssboOffsetAlignment;
	ApiCapabilitiesPrivate() : maxglslesversion(0), uboOffsetAlignment(0), ssboOffsetAlignment(0) {}
};
//!\endcond

/*!********************************************************************************************************************
\brief       Struct containing the API capabilities of a specified configuration.
***********************************************************************************************************************/
struct ApiCapabilities : private ApiCapabilitiesPrivate
{
public:
	enum Enum
	{
		MapBuffer = 0,  //!<Supports mapping a buffer
		MapBufferRange,
		ComputeShader,  //!<Supports compute shaders
		GeometryShader, //!<Supports compute shaders
		Sampler,    //!<Supports sampler objects separate from textures
		Ssbo,     //!<Supports buffers that can be read/write from a shader
		Ubo,      //!<Supports buffers that can be read from a shader
		AtomicBuffer, //!<Supports buffers that can accomodate atomic operations
		TexureStorage,  //!<Supports immutable storage textures
		Uniforms,   //!<Supports free-standing (non-buffer) uniforms
		UintUniforms, //!<Supports unsigned integer uniforms
		Texture3D,    //!<Supports 3D Textures
		Texture2DArray, //=10 //!<Supports 2D Array textures
		Texture2DMS,//!< Supports Multisample texture
		Texture2DArrayMS,//!< Supports Multisample texture
		TextureSwizzling,//!<Supports Texture Swizzling
		ImageStore,   //!<Supports write into textures from a shader
		ShaderAttributeReflection, //!<Supports querying the shader object for its attributes (reflection)
		ShaderAttributeExplicitBind, //!<Supports binding shader attributes to specific indexes from the shader
		InvalidateFrameBuffer,  //!<Supports explicitly discarding framebuffer contents
		ClearBuffer,  //!<Supports explicitly clearing a buffer
		DebugCallback,  //!<Supports setting a function as a debug callback to report errors
		AnisotropicFiltering, //!<Supports anisotropic texture filtering
		ShadowSamplers,     //!<Supports shadow samplers
		ShaderPixelLocalStorage, //=20  //!<Supports explicit Pixel Local Storage in the shader
		ShaderPixelLocalStorage2, //!<Supports pixel local storage2
		Instancing,       //!< Supports instanced rendering
		ClearTexImage, // Supports Clearing a texture without having it attached to an fbo
		Tessellation,
		BicubicFiltering, // Supports Bicubic filtering
		FramebufferTextureLayer // Supports specifying layers of a 2D texture array to attach to an fbo
		// CAREFUL! IF THIS BECOMES MORE T32 ENTRIES, THE BITSETS ABOVE MUST BE MADE BIGGER TO ACCOMODATE THEM!
	};

	/*!
	   \brief Return true if capability is supported natively
	   \param capability Capability
	 */
	bool nativelySupports(Enum capability) const
	{
		return nativeSupport[capability];
	}

	/*!
	   \brief Return true if capability is supported through extension
	   \param capability Capability
	 */
	bool supportsThroughExtension(Enum capability) const
	{
		return extensionSupport[capability];
	}

	/*!
	   \brief Return true if capability is supported either natively or through extension
	   \param capability
	 */
	bool supports(Enum capability) const
	{
		return nativelySupports(capability) || supportsThroughExtension(capability);
	}

	/*!
	   \brief Return max Glsl version supported
	 */
	uint16 maxGlslVersion() const
	{
		return maxglslesversion;
	}

	/*!
	   \brief Return ubo dynamic offset alignment
	 */
	uint32 uboDynamicOffsetAlignment() const
	{
		return uboOffsetAlignment;
	}

	/*!
	   \brief Return ssbo dynamic offset alignment
	 */
	uint32 ssboDynamicOffsetAlignment() const
	{
		return ssboOffsetAlignment;
	}
};


//!\cond NO_DOXYGEN
class GraphicsPipelineContainer
{
	friend class ::pvr::api::impl::GraphicsPipeline_;
	api::impl::GraphicsPipeline_* boundGraphicsPipeline_;
protected:
	void setBoundGraphicsPipeline(api::impl::GraphicsPipeline_* pipeline)
	{
		boundGraphicsPipeline_ = pipeline;
	}
public:
	api::impl::GraphicsPipeline_* getBoundGraphicsPipeline_()
	{
		return boundGraphicsPipeline_;
	}
	GraphicsPipelineContainer() : boundGraphicsPipeline_(NULL) { }
};
//!\endcond

//!\cond NO_DOXYGEN
class ComputePipelineContainer
{
	friend class ::pvr::api::impl::ComputePipeline_;
	api::impl::ComputePipeline_* boundComputePipeline;
protected:
	void setBoundComputePipeline(api::impl::ComputePipeline_* pipeline)
	{
		boundComputePipeline = pipeline;
	}
public:
	api::impl::ComputePipeline_* getBoundComputePipeline()
	{
		return boundComputePipeline;
	}
	ComputePipelineContainer() : boundComputePipeline(NULL) { }
};
//!\endcond

typedef pvr::EmbeddedRefCountedResource<IGraphicsContext> GraphicsContextStrongReference;
typedef pvr::RefCountedWeakReference<IGraphicsContext> GraphicsContext;

/*!****************************************************************************************************************
\brief  This function is implemented in PVRApi, in order to return a pointer to the actual Graphics Context.
\return A new GraphicsContext object. Its type will be dependent on the specific PVRApi library loaded.
*******************************************************************************************************************/
GraphicsContextStrongReference createGraphicsContext();


/*!****************************************************************************************************************
\brief  Interface for Graphics context. This interface will be used all over in user code to pass GraphicsContext
        objects wherever required. It represents a specific GPU configuration.
*******************************************************************************************************************/
class IGraphicsContext : public GraphicsPipelineContainer, public ComputePipelineContainer
{
	friend GraphicsContextStrongReference createGraphicsContext();
	friend class ::pvr::api::impl::ComputePipeline_;
	friend class ::pvr::api::impl::GraphicsPipeline_;
	friend class ::pvr::api::impl::ResetPipeline;

	//friend class Fbo getDefaultFbo(class GraphicsContext&);
public:
	/*!****************************************************************************************************************
	\brief  Default constructor. Object is uninitialized and unusable until init.
	*******************************************************************************************************************/
	IGraphicsContext(Api apiType = Api::Unspecified) : m_osManager(NULL), m_apiType(apiType)
	{
		((ApiCapabilitiesPrivate&)m_apiCapabilities).maxglslesversion = 200;
	}
	virtual ~IGraphicsContext() {}

	/*!****************************************************************************************************************
	\brief  Image Format.
	*******************************************************************************************************************/
	enum ImageFormat
	{
		ImageFormatRGBA,
		ImageFormatBGRA
	};

	/*!****************************************************************************************************************
	\brief  Call this function to initialize the Context using the information of a specific OSManager (usually, the
	        Shell instance).
	*******************************************************************************************************************/
	virtual Result init(OSManager& osManager) = 0;

	/*!****************************************************************************************************************
	\brief  Release the resources held by this context.
	*******************************************************************************************************************/
	virtual void release() = 0;

	/*!****************************************************************************************************************
	\brief  Wait until all pending operations are done
	*******************************************************************************************************************/
	virtual void waitIdle() = 0;

	/*!****************************************************************************************************************
	\brief  take a screenshot in the specified buffer of the specified screen area.
	*******************************************************************************************************************/
	virtual bool screenCaptureRegion(uint32 x, uint32 y, uint32 w, uint32 h, byte* buffer, ImageFormat imageFormat) = 0;

	/*!****************************************************************************************************************
	\brief  Print information about this IGraphicsContext.
	*******************************************************************************************************************/
	virtual std::string getInfo()const = 0;

	/*!****************************************************************************************************************
	\brief  Gets the format of the presentation image.
	*******************************************************************************************************************/
	virtual const api::ImageDataFormat getPresentationImageFormat()const = 0;

	/*!****************************************************************************************************************
	\brief  Gets the format of the depth stencil image.
	*******************************************************************************************************************/
	virtual const api::ImageDataFormat getDepthStencilImageFormat()const = 0;

	/*!****************************************************************************************************************
	\brief  Get the IPlatformContext object powering this GraphicsContext.
	*******************************************************************************************************************/
	virtual IPlatformContext& getPlatformContext()const = 0;

	/*!****************************************************************************************************************
	\brief  Query if a specific extension is supported.
	\param extension A c-style string representing the extension.
	\return True if the extension is supported.
	*******************************************************************************************************************/
	virtual bool isExtensionSupported(const char8* extension) const = 0;

	/*!****************************************************************************************************************
	\brief  Create a GraphicsPipeline object associated with this context.
	\param  createParam The CreateParameters used to create the pipeline.
	\return A newly constructed GraphicsPipeline. Contains NULL if creation failed.
	*******************************************************************************************************************/
	virtual api::GraphicsPipeline createGraphicsPipeline(api::GraphicsPipelineCreateParam& createParam) = 0;

	/*!****************************************************************************************************************
	\brief  Create GraphicsPipeline as a child of another provided GraphicsPipeline.
	\param  createParam The CreateParameters used to create the pipeline. NOTE the create param won't inherit the states
	    from the parent pipeline. The Application must set the states that are to be inherited from parent pipeline
	    see ParentableGraphicsPipeline::getCreateParam() function
	\param  parent The parent pipeline used to create the new pipeline. If the pipeline does not belong to this context,
	        the results are undefined.
	\return A newly constructed GraphicsPipeline object, child to "parent". Contains NULL if creation failed.
	\description Use parenting to denote hierarchies of pipelines to take advantage of very important optimisations that
	        are performed when switching from a child to a parent or a sibling and vice versa.
	*******************************************************************************************************************/
	virtual api::GraphicsPipeline createGraphicsPipeline(api::GraphicsPipelineCreateParam& createParam,
	    api::ParentableGraphicsPipeline parent) = 0;

	/*!****************************************************************************************************************
	\brief  Create a new ComputePipeline object.
	\param  createParam The CreateParameters used to create the pipeline.
	\return A newly constructed ComputePipeline object. Contains NULL if creation failed.
	*******************************************************************************************************************/
	virtual api::ComputePipeline createComputePipeline(const api::ComputePipelineCreateParam& createParam) = 0;

	/*!****************************************************************************************************************
	\brief  Create a new parentable GraphicsPipeline object.
	\param  createParam The CreateParameters used to create the pipeline.
	\return A newly constructed ParentableGraphicsPipeline object. Contains NULL if creation failed.
	\description ParentableGraphicsPipeline s contain additional information that allows them to be used as parents
	             to other Graphics Pipelines.
	*******************************************************************************************************************/
	virtual api::ParentableGraphicsPipeline  createParentableGraphicsPipeline(const api::GraphicsPipelineCreateParam& createParam) = 0;

	/*!****************************************************************************************************************
	\brief  Create a new Sampler object.
	\param  createParam The CreateParameters used to create the object.
	\return A newly constructed Sampler object. Contains NULL if creation failed.
	*******************************************************************************************************************/
	virtual api::Sampler createSampler(const api::SamplerCreateParam& createParam) = 0;


	/*!****************************************************************************************************************
	\brief  Create a new EffectApi object.
	\param  effectDesc The CreateParameters used to create the object.
	\param  pipeDesc The CreateParameters will be used to create the GraphicsPipeline for this effect.
	\param  effectDelegate The effectDelegate which will be used to load any required resources for the creation of this
	object. The effectDelegate is in most cases the main application class of the user, after implemented the
	AssetLoadingDelegate interface
	\return A newly constructed EffectAPI object. Contains NULL if creation failed.
	*******************************************************************************************************************/
	virtual api::EffectApi createEffectApi(::pvr::assets::Effect& effectDesc, api::GraphicsPipelineCreateParam& pipeDesc,
	                                       api::AssetLoadingDelegate& effectDelegate) = 0;

	/*!****************************************************************************************************************
	\brief  Create a new Texture object.
	\return A newly created, unallocated Texture. Contains NULL if creation failed.
	*******************************************************************************************************************/
	virtual api::TextureStore createTexture() = 0;

	/*!****************************************************************************************************************
	\brief  Create a new Texture object.
	\return A newly created, unallocated Texture. Contains NULL if creation failed.
	*******************************************************************************************************************/
	virtual api::TextureView createTextureView(const api::TextureStore& texture, types::ImageSubresourceRange range,
	    types::SwizzleChannels = types::SwizzleChannels()) = 0;

	/*!****************************************************************************************************************
	\brief  Create a new Texture object.
	\return A newly created, unallocated Texture. Contains NULL if creation failed.
	*******************************************************************************************************************/
	virtual api::TextureView createTextureView(const api::TextureStore& texture,
	    types::SwizzleChannels = types::SwizzleChannels()) = 0;

	/*!****************************************************************************************************************
	\brief  Create a new Shader-Accessible view to Buffer Object (Can be used as UBO/SSBO etc), with an existing backing buffer.
	    BufferViews allow Buffers to be added to DescriptorSets.
	\return A newly created BufferView object, wrapping the specified range of the provided buffer.
	\param  buffer The buffer that the UBO will be a view of.
	\param  offset The starting offset in "buffer" that the UBO will represent. Default: 0.
	\param  range The size of the original buffer that this view will represent/access. Default: the
	    remaining size of the buffer (minus offset).
	\description The UBO created will be representing, in Buffer, range of  (offset, offset+range)
	*******************************************************************************************************************/
	virtual api::BufferView createBufferView(const api::Buffer& buffer, uint32 offset = 0, uint32 range = 0xFFFFFFFFu) = 0;

	/*!*********************************************************************************************
	\brief  One-step create a new Buffer with a corresponding BufferView to the entire range of the buffer.
	\return A newly created BufferView of an entire a newly created Buffer object.
	\param  size The size of the new VBO,
	\param  bufferUsage A bitfield of all allowed buffer uses
	\param  isMappable Set to true to allocate the buffer from memory that can be mapped. If this feature is not required,
	and the buffer will be filled from a transfer (buffer copy or cmd buffer update), set to false and ensure bufferUsage
	contains the TransferDst flag.
	\description The UBO created will be representing, in Buffer, range of  (offset, offset+range)
	*******************************************************************************************************************/
	virtual api::BufferView createBufferAndView(uint32 size, types::BufferBindingUse bufferUsage, bool isMappable = false) = 0;

	/*!*********************************************************************************************
	\brief  Create a new buffer object, which can then be bound as a VBO/IBO and/or shared with
	        BufferView objects (SSBOView, VBOView)
	\return A newly constructed Buffer object
	\param  size The size, in bytes, of the buffer
	\param bufferUsage The intended uses of the Buffer (VBO,IBO,UBO, Copy etc.)
	\param isMappable  The buffer is mappable by the host
	*******************************************************************************************************************/
	virtual api::Buffer createBuffer(uint32 size, types::BufferBindingUse bufferUsage, bool isMappable = false) = 0;

	/*!****************************************************************************************************************
	\brief  Create a new primary CommandBuffer on the default CommandPool
	\return A newly created CommandBuffer. Null if failed.
	*******************************************************************************************************************/
	virtual api::CommandBuffer createCommandBufferOnDefaultPool() = 0;

	/*!*********************************************************************************************
	\brief  Create a new secondary CommandBuffer on the default CommandPool
	\return A newly created CommandBuffer. Null if failed.
	************************************************************************************************/
	virtual api::SecondaryCommandBuffer createSecondaryCommandBufferOnDefaultPool() = 0;

	/*!*********************************************************************************************
	\brief Create Shader from source. Accepts an array of custom preprocessor definition directives.
	\param shaderSrc The shader source, as a stream.
	\param shaderType The type of shader to create
	\param defines Optional. A pointer to an array of c-style strings containing  preprocessor definition directives
	\param numDefines Optional. The number of items in the defines array.
	************************************************************************************************/
	virtual api::Shader createShader(const Stream& shaderSrc, types::ShaderType shaderType,
	                                 const char* const* defines = NULL, uint32 numDefines = 0) = 0;

	/*!*********************************************************************************************
	\brief Create Shader from binary.
	\param shaderData The shader binary data, as a stream
	\param shaderType The type of shader to create
	\param binaryFormat The format of the data.
	************************************************************************************************/
	virtual api::Shader createShader(Stream& shaderData, types::ShaderType shaderType,
	                                 types::ShaderBinaryFormat binaryFormat) = 0;

	/*!**************************************************************************************************
	\brief  Create a Framebuffer Object with specified parameters and backing image. Not for on-screen
	        rendering.
	\param  createParam The creation parameters for this Fbo.
	\return A newly created FBO. Null if failed.
	\description  This method is used to create a typical GL-style FBO (an object that is normally used
	        to render into a texture).
	*****************************************************************************************************/
	virtual api::Fbo createFbo(const api::FboCreateParam& createParam) = 0;

	/*!**************************************************************************************************
	\brief  Create a Framebuffer Object with specified parameters and backing image. Not for on-screen
	        rendering.
	\param  createParams The creation parameters for this Fbo.
	\return A newly created FBO. Null if failed.
	\description  This method is used to create a typical GL-style FBO (an object that is normally used
	        to render into a texture).
	*****************************************************************************************************/
	virtual api::FboSet createFboSet(const Multi<api::FboCreateParam>& createParams) = 0;


	/*!***************************************************************************************************
	\brief Create an FBO that represents the actual backbuffer (i.e. an fbo to be used for on-screen
	rendering), using the RenderPass to use for it.
	compatible with the actual BackBuffer format and options - this is the user's responsibility.
	Allows the user to specify additional color view attachments for the FBO to use.
	\param[in] renderPass The renderpass that the new FBO will use. Must be compatible with the backbuffer.
	\param[in] onScreenFboCreateParam An additional set of creation parameters to use for the fbo.
	\param[in] swapIndex The Index of the SwapBuffer Image (backbuffer) that the new FBO will be attached to.
	\return A new FBO who can be used to write to the Screen.
	\description This version uses a RenderPass that was provided by the user. The color and Depth/Stencil
	formats must be compatible with the backbuffer, otherwise results are undefined.
	*****************************************************************************************************/
	virtual api::Fbo createOnScreenFboWithRenderPass(uint32 swapIndex, const api::RenderPass& renderPass,
	    const api::OnScreenFboCreateParam& onScreenFboCreateParam) = 0;

	/*!***************************************************************************************************
	\brief Create aset of FBOs (one per backbuffer image) that represents the actual backbuffer
	(i.e. an fbo to be used for on-screen rendering), using the RenderPass to use for it.
	compatible with the actual BackBuffer format and options - this is the user's responsibility.
	\param[in] renderPass The renderpass that the new FBO will use. Must be compatible with the backbuffer.
	\return A new FBO who can be used to write to the Screen.
	\description This version uses a RenderPass that was provided by the user. The color and Depth/Stencil
	formats must be compatible with the backbuffer, otherwise results are undefined.
	*****************************************************************************************************/
	virtual api::FboSet createOnScreenFboSetWithRenderPass(const api::RenderPass& renderPass) = 0;

	/*!***************************************************************************************************
	\brief Create an FBO that represents the actual backbuffer (i.e. an fbo to be used for on-screen
	rendering), using the RenderPass to use for it.
	compatible with the actual BackBuffer format and options - this is the user's responsibility.
	\param[in] renderPass The renderpass that the new FBO will use. Must be compatible with the backbuffer.
	\param[in] onScreenFboCreateParams A vector with additional color view attachments per swap chain.
	\return A new FBO who can be used to write to the Screen.
	\description This version uses a RenderPass that was provided by the user. The color and Depth/Stencil
	formats must be compatible with the backbuffer, otherwise results are undefined.
	*****************************************************************************************************/
	virtual api::FboSet createOnScreenFboSetWithRenderPass(const api::RenderPass& renderPass,
	    pvr::Multi<api::OnScreenFboCreateParam>& onScreenFboCreateParams) = 0;

	/*!***************************************************************************************************
	\brief Create an FBO that represents the actual backbuffer (i.e. an fbo to be used for on-screen
	rendering), using the RenderPass to use for it.
	compatible with the actual BackBuffer format and options - this is the user's responsibility.
	\param[in] renderPass The renderpass that the new FBO will use. Must be compatible with the backbuffer.
	\param[in] swapIndex The Index of the SwapBuffer Image (backbuffer) that the new FBO will be attached to.
	\return A new FBO who can be used to write to the Screen.
	\description This version uses a RenderPass that was provided by the user. The color and Depth/Stencil
	formats must be compatible with the backbuffer, otherwise results are undefined.
	*****************************************************************************************************/
	virtual api::Fbo createOnScreenFboWithRenderPass(uint32 swapIndex, const api::RenderPass& renderPass) = 0;

	/*!***************************************************************************************************
	\brief Create an FBO that represents the actual backbuffer (i.e. an fbo to be used for on-screen
	rendering), using the most common parameters.
	\param[in] colorLoadOp The Load Operation for the color attachment (Default is LoadOp::Clear, clearing the color of the screen at every frame start)
	\param[in] colorStoreOp The Store Operation for the color attachment (Default is StoreOp::Store, storing the color before buffer swapping)
	\param[in] depthLoadOp The Load Operation for the depth (Default is LoadOp::Clear, clearing the Depth at every frame start)
	\param[in] depthStoreOp The Store Operation for the depth buffer (Default is StoreOp::Ignore, discarding the Depth before buffer swapping)
	\param[in] stencilLoadOp The Load Operation for the stencil buffer (Default is LoadOp::Clear, clearing the Stencil at every frame start)
	\param[in] stencilStoreOp The Store Operation for the stencil buffer (Default is StoreOp::Ignore, discarding the Stencil before buffer swapping)
	\return A new FBO who can be used to write to the Screen.
	\description  This version uses the most typical parameters used for the backbuffer, and also
	creates the RenderPass for the FBO. Uses a single subpass at color attachment 0.
	Use the FBO->getRenderPass method if you wish to retrieve the renderpass created with this method.
	WARNING: Defaults discard Depth and Stencil at the end of the renderpass, for performance. This may
	cause unintended results as it is different from default OpenGL. If you wish to preserve depth and/or
	stencil after the renderpass, please specify StoreOp:Store for depth and stencil.
	*****************************************************************************************************/
	virtual api::FboSet createOnScreenFboSet(
	  types::LoadOp colorLoadOp = types::LoadOp::Clear, types::StoreOp colorStoreOp = types::StoreOp::Store,
	  types::LoadOp depthLoadOp = types::LoadOp::Clear, types::StoreOp depthStoreOp = types::StoreOp::Ignore,
	  types::LoadOp stencilLoadOp = types::LoadOp::Clear, types::StoreOp stencilStoreOp = types::StoreOp::Ignore) = 0;

	/*!***************************************************************************************************
	\brief Create an FBO that represents the actual backbuffer (i.e. an fbo to be used for on-screen
	rendering), using the most common parameters.
	\param[in] swapIndex Swapchain index
	\param[in] colorLoadOp The Load Operation for the color attachment (Default is LoadOp::Clear, clearing the color of the screen at every frame start)
	\param[in] colorStoreOp The Store Operation for the color attachment (Default is StoreOp::Store, storing the color before buffer swapping)
	\param[in] depthLoadOp The Load Operation for the depth (Default is LoadOp::Clear, clearing the Depth at every frame start)
	\param[in] depthStoreOp The Store Operation for the depth buffer (Default is StoreOp::Ignore, discarding the Depth before buffer swapping)
	\param[in] stencilLoadOp The Load Operation for the stencil buffer (Default is LoadOp::Clear, clearing the Stencil at every frame start)
	\param[in] stencilStoreOp The Store Operation for the stencil buffer (Default is StoreOp::Ignore, discarding the Stencil before buffer swapping)
	\return A new FBO who can be used to write to the Screen.
	\description  This version uses the most typical parameters used for the backbuffer, and also
	creates the RenderPass for the FBO. Uses a single subpass at color attachment 0.
	Use the FBO->getRenderPass method if you wish to retrieve the renderpass created with this method.
	WARNING: Defaults discard Depth and Stencil at the end of the renderpass, for performance. This may
	cause unintended results as it is different from default OpenGL. If you wish to preserve depth and/or
	stencil after the renderpass, please specify StoreOp:Store for depth and stencil.
	*****************************************************************************************************/
	virtual api::Fbo createOnScreenFbo(
	  uint32 swapIndex, types::LoadOp colorLoadOp = types::LoadOp::Clear,
	  types::StoreOp colorStoreOp = types::StoreOp::Store, types::LoadOp depthLoadOp = types::LoadOp::Clear,
	  types::StoreOp depthStoreOp = types::StoreOp::Ignore, types::LoadOp stencilLoadOp = types::LoadOp::Clear,
	  types::StoreOp stencilStoreOp = types::StoreOp::Ignore) = 0;

	/*!*********************************************************************************************
	  \brief  Create a new render pass.
	  \param  renderPassDesc The creation parameters used to create the renderPass.
	  \return A new RenderPass object. Contains NULL if failed.
	  ************************************************************************************************/
	virtual api::RenderPass createRenderPass(const api::RenderPassCreateParam& renderPassDesc) = 0;

	/*!*********************************************************************************************
	\brief  Create a new DescriptorPool (descriptor set allocation pool).
	\param  createParam The creation parameters used to create the descriptor pool
	\return A new DescriptorPool object. Contains NULL if failed.
	************************************************************************************************/
	virtual api::DescriptorPool createDescriptorPool(const api::DescriptorPoolCreateParam& createParam) = 0;

	/*!*********************************************************************************************
	\brief  Create a new DescriptorSet on the PVRApi default allocation pool.
	\param  layout The layout of the DescriptorSet object
	\return A new DescriptorSet object
	************************************************************************************************/
	virtual api::DescriptorSet createDescriptorSetOnDefaultPool(const api::DescriptorSetLayout& layout) = 0;

	/*!*********************************************************************************************
	\brief  Create a new DescriptorSetLayout.
	\param  createParam The creation parameters used to create the descriptor set layout.
	\return A new DescriptorSetLayout object. Contains NULL if failed.
	\description A DescriptorSetLayout contains the information required to create "compatible"
	             descriptor sets.
	************************************************************************************************/
	virtual api::DescriptorSetLayout createDescriptorSetLayout(const api::DescriptorSetLayoutCreateParam& createParam) = 0;

	/*!*********************************************************************************************
	\brief  Create a new PipelineLayout.
	\param  createParam The creation parameters used to create PipelineLayout.
	\return A new DescriptorSetLayout object. Contains NULL if failed.
	\description A PipelineLayout is required for several things, including binding descriptor sets.
	************************************************************************************************/
	virtual api::PipelineLayout createPipelineLayout(const api::PipelineLayoutCreateParam& createParam) = 0;

	/*!*********************************************************************************************
	\brief  Create a new CommandPool.
	************************************************************************************************/
	virtual api::CommandPool createCommandPool() = 0;

	/*!*********************************************************************************************
	\brief  Get the default, automatically generated CommandPool
	************************************************************************************************/
	virtual const api::CommandPool& getDefaultCommandPool() const = 0;

	/*!*********************************************************************************************
	\brief  Get the default, automatically generated CommandPool
	************************************************************************************************/
	virtual api::CommandPool& getDefaultCommandPool() = 0;

	/*!*********************************************************************************************
	\brief  Get the default, automatically generated DescriptorPool.
	************************************************************************************************/
	virtual const api::DescriptorPool& getDefaultDescriptorPool() const = 0;

	/*!*********************************************************************************************
	\brief  Get the default, automatically generated DescriptorPool.
	************************************************************************************************/
	virtual api::DescriptorPool& getDefaultDescriptorPool() = 0;

	/*!*
	\brief Create Fence
	\param createSignaled Create Fence with signaled state
	\return
	 */
	virtual api::Fence createFence(bool createSignaled = true) = 0;

	/*!*
	\brief Create Semaphore
	\return
	 */
	virtual api::Semaphore createSemaphore() = 0;

	/*!****************************************************************************************************************
	\brief  Get ApiCapabilities object representing the capabilities of this GraphicsContext.
	*******************************************************************************************************************/
	bool hasApiCapability(ApiCapabilities::Enum capability) const
	{
		return m_apiCapabilities.supports(capability);
	}

	/*!****************************************************************************************************************
	\brief  Get ApiCapabilities object representing the capabilities of this GraphicsContext.
	*******************************************************************************************************************/
	bool hasApiCapabilityNatively(ApiCapabilities::Enum capability) const
	{
		return m_apiCapabilities.nativelySupports(capability);
	}

	/*!****************************************************************************************************************
	\brief  Get ApiCapabilities object representing the capabilities of this GraphicsContext.
	*******************************************************************************************************************/
	bool hasApiCapabilityExtension(ApiCapabilities::Enum capability) const
	{
		return m_apiCapabilities.supportsThroughExtension(capability);
	}

	/*!****************************************************************************************************************
	\brief  Get ApiCapabilities object representing the capabilities of this GraphicsContext.
	*******************************************************************************************************************/
	const ApiCapabilities& getApiCapabilities() const { return m_apiCapabilities; }

	/*!****************************************************************************************************************
	\brief  Get the Api of this GraphicsContext.
	*******************************************************************************************************************/
	Api getApiType() const { return m_apiType; }

	/*!****************************************************************************************************************
	\brief  Get the DisplayAttributes associated with this GraphicsContext.
	*******************************************************************************************************************/
	const platform::DisplayAttributes& getDisplayAttributes()const
	{
		assertion(m_osManager != NULL ,  "NULL OSManager");
		return m_osManager->getDisplayAttributes();
	}

	/*!****************************************************************************************************************
	\brief  Query if a specific DeviceQueue type is supported.
	\param queueType The DeviceQueueType to query support for.
	\return True if queueType is supported by this context.
	*******************************************************************************************************************/
	bool isQueueSupported(DeviceQueueType queueType)
	{
		return ((uint32)m_osManager->getDeviceQueueTypesRequired() & static_cast<uint32>(queueType)) != 0;
	}

	/*!*********************************************************************************************
	\brief  Get the number of Multi buffer supported.
	************************************************************************************************/
	uint32 getSwapChainLength()const { return getPlatformContext().getSwapChainLength(); }

	/*!*********************************************************************************************
	\brief  Get the current swapchain index
	************************************************************************************************/
	uint32 getSwapChainIndex()const { return getPlatformContext().getSwapChainIndex(); }

	/*!**********************************************************************************************
	\brief Get the last swap chain index
	*************************************************************************************************/
	uint32 getLastSwapChainIndex()const { return getPlatformContext().getLastSwapChainIndex(); }

protected:
	OSManager* m_osManager;
	Api m_apiType;
	ApiCapabilities m_apiCapabilities;
	GraphicsContext getWeakRef();
};
}
