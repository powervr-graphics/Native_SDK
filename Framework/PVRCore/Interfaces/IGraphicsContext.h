/*!
\brief Contains the interface that the very important GraphicsContext classes implement.
\file PVRCore/Interfaces/IGraphicsContext.h
\author PowerVR by Imagination, Developer Technology Team.
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Base/Defines.h"
#include "PVRCore/Interfaces/ForwardDecApiObjects.h"
#include "PVRCore/Interfaces/OSManager.h"
#include "PVRCore/Base/RefCounted.h"
#include "PVRCore/Maths.h"
#include "PVRCore/Interfaces/IPlatformContext.h"
#include "PVRCore/Maths.h"
#include <map>
#include <bitset>

namespace pvr {
class IGraphicsContext;
class ISharedContext;
class Stream;
class Texture;

/// <summary>Main PVRApi Namespace. In the PVRCore projects, contains interfaces and enumerations that conceptually
/// belong to PVRApi but need to be shared with other PVR Framework projects (GraphicsContext interface, enums)
/// </summary>
namespace api {
typedef types::SamplerCreateParam SamplerCreateParam;
struct TextureAndFence_
{
	api::TextureView texture;
	api::Fence fence;
	virtual ~TextureAndFence_() {}
};
typedef EmbeddedRefCountedResource<TextureAndFence_> TextureAndFence;
}



//!\cond NO_DOXYGEN
struct ApiCapabilitiesPrivate
{
	std::bitset<64> _nativeSupport; //!< Will be set if natively supported
	std::bitset<64> _extensionSupport; //!< Will be set if supported through extension
	uint16 _maxglslesversion;
	uint32 _uboOffsetAlignment;
	uint32 _ssboOffsetAlignment;

	// ray tracing limits
	uint32 _maxDescriptorSetIndirectRayPipelines;
	uint32 _maxPerStageDescriptorIndirectRayPipelines;
	uint32 _maxRayBlocks;
	uint32 _maxRayBlockComponents;
	uint32 _maxRayEmits;
	uint32 _maxRayInputComponents;
	uint32 _sceneHierarchyExtentRange;
	uint32 _sceneHierarchyExtentPrecision;
	uint32 _maxRayBounceLimit;
	uint32 _maxIndirectRayPipelines;
	uint32 _maxSceneHierarchyBuildSize;
	uint32 _maxSceneBindingPoints;
	uint32 _maxSceneHierarchyMergeSources;
	uint32 _discreteMergeQualities;
	bool _instancedSceneHierarchyGeometry;
	uint32 _timestampRaytracingAndSceneGenerator;
	uint32 _maxSizeOfSharedRayConstants;
	bool _decals;

	// ray tracing features
	bool _geometryShaderInSceneHierarchy;
	bool _tessellationShaderInSceneHierarchy;
	bool _frameStoresAndAtomics;
	bool _rayStoresAndAtomics;

	ApiCapabilitiesPrivate() : _maxglslesversion(0), _uboOffsetAlignment(0), _ssboOffsetAlignment(0) {}
};
//!\endcond

/// <summary>Struct containing the API capabilities of a specified configuration.</summary>
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
		ShaderPixelLocalStorage, //!<Supports explicit Pixel Local Storage in the shader
		ShaderPixelLocalStorage2, //!<Supports pixel local storage2
		Instancing,       //!< Supports instanced rendering
		ClearTexImageIMG, // Supports Clearing a texture without having it attached to an fbo
		ClearTexImageEXT, // Supports Clearing a texture without having it attached to an fbo
		Tessellation, //!< Supports Tessellation
		BicubicFiltering, //!< Supports Bicubic filtering
		FramebufferTextureLayer, //!< Supports specifying layers of a 2D texture array to attach to an fbo
		DepthBiasClamp, //<! Suports clamping of depth bias (a.k.a. Polygon Offset) values
		BlitFrameBuffer,
		FenceSync,
		// CAREFUL! IF THIS BECOMES MORE THAN 64 ENTRIES, THE BITSETS ABOVE MUST BE MADE BIGGER TO ACCOMODATE THEM!
	};

	/// <summary>Return true if capability is supported natively</summary>
	/// <param name="capability">The capability to test for support</param>
	/// <returns>True if the capability is supported natively , false if it is supported only through an extension or
	/// not at all</returns>
	bool nativelySupports(Enum capability) const
	{
		return _nativeSupport[capability];
	}

	/// <summary>Return true if capability is supported through extension</summary>
	/// <param name="capability">The capability to test for support</param>
	/// <returns>True if the capability is through an extension , false if it is supported natively only or not at all
	/// </returns>
	bool supportsThroughExtension(Enum capability) const
	{
		return _extensionSupport[capability];
	}

	/// <summary>Return true if capability is supported either natively or through extension</summary>
	/// <param name="capability">The capability to test for support</param>
	/// <returns>True if the capability is supported natively or through an extension, false if not supported at all
	/// </returns>
	bool supports(Enum capability) const
	{
		return nativelySupports(capability) || supportsThroughExtension(capability);
	}

	/// <summary>Return max Glsl version supported</summary>
	/// <returns>The max GLSL version supported</returns>
	uint16 maxGlslVersion() const
	{
		return _maxglslesversion;
	}

	/// <summary>Get the alignment of dynamic offsets. Any dynamic offsets passed to the
	/// CommandBuffer::bindDescriptorSets function corresponding to a Uniform Buffer Object must be an integer
	/// multiple of this number</summary>
	/// <returns>The mandatory alignment of dynamic UBOs</returns>
	uint32 uboDynamicOffsetAlignment() const
	{
		return _uboOffsetAlignment;
	}

	/// <summary>Get the alignment of dynamic offsets. Any dynamic offsets passed to the
	/// CommandBuffer::bindDescriptorSets function corresponding to a Shader Storage Buffer Object must be an integer
	/// multiple of this number</summary>
	/// <returns>The mandatory alignment of dynamic SSBOs</returns>
	uint32 ssboDynamicOffsetAlignment() const
	{
		return _ssboOffsetAlignment;
	}

	/// <summary>Gets the maximum size of shared ray contexts supported by the platform.</summary>
	/// <returns>The maximum size of shared ray constants supported by the platform</returns>
	uint32 maxSharedRayConstantsSize() const
	{
		return _maxSizeOfSharedRayConstants;
	}
};

class PipelineContainer
{
	pvr::api::impl::GraphicsPipeline_* _boundGraphicsPipeline;
	pvr::api::impl::ComputePipeline_* _boundComputePipeline;
	pvr::api::impl::VertexRayPipeline_* _boundVertexRayPipeline;
	pvr::api::impl::SceneTraversalPipeline_* _boundSceneTraversalPipeline;

	pvr::types::PipelineBindPoint _lastBindPoint;

public:
	void setBoundGraphicsPipeline(pvr::api::impl::GraphicsPipeline_* pipeline)
	{
		_boundGraphicsPipeline = pipeline;
		_lastBindPoint = pvr::types::PipelineBindPoint::Graphics;
	}
	void setBoundComputePipeline(pvr::api::impl::ComputePipeline_* pipeline)
	{
		_boundComputePipeline = pipeline;
		_lastBindPoint = pvr::types::PipelineBindPoint::Compute;
	}
	void setBoundVertexRayPipeline(pvr::api::impl::VertexRayPipeline_* pipeline)
	{
		_boundVertexRayPipeline = pipeline;
		_lastBindPoint = pvr::types::PipelineBindPoint::SceneGenerator;
	}
	void setBoundSceneTraversalPipeline(pvr::api::impl::SceneTraversalPipeline_* pipeline)
	{
		_boundSceneTraversalPipeline = pipeline;
		_lastBindPoint = pvr::types::PipelineBindPoint::RayTracing;
	}
	pvr::api::impl::GraphicsPipeline_* getBoundGraphicsPipeline()
	{
		return _boundGraphicsPipeline;
	}
	pvr::api::impl::ComputePipeline_* getBoundComputePipeline()
	{
		return _boundComputePipeline;
	}
	pvr::api::impl::VertexRayPipeline_* getBoundVertexRayPipeline()
	{
		return _boundVertexRayPipeline;
	}
	pvr::api::impl::SceneTraversalPipeline_* getBoundSceneTraversalPipeline_()
	{
		return _boundSceneTraversalPipeline;
	}

	pvr::types::PipelineBindPoint getLastPipelineBindingPoint()
	{
		return _lastBindPoint;
	}

	PipelineContainer() : _boundGraphicsPipeline(NULL), _boundComputePipeline(NULL),
		_boundVertexRayPipeline(NULL), _boundSceneTraversalPipeline(NULL),
		_lastBindPoint(pvr::types::PipelineBindPoint::None) { }
};
//!\endcond

typedef pvr::EmbeddedRefCountedResource<IGraphicsContext> GraphicsContextStrongReference;
typedef pvr::RefCountedWeakReference<IGraphicsContext> GraphicsContext;
typedef pvr::EmbeddedRefCountedResource<ISharedContext> SharedContext;

//#error CHECK THIS OUT

/// <summary>This function is implemented in PVRApi, in order to return a pointer to the actual Graphics Context.
/// </summary>
/// <returns>A new GraphicsContext object. Its type will be dependent on the specific PVRApi library loaded.
/// </returns>
GraphicsContextStrongReference PVR_API_FUNC createGraphicsContext();

/// <summary>Interface for Graphics context. This interface will be used all over in user code to pass GraphicsContext
/// objects wherever required. It represents a specific GPU configuration.</summary>
class IGraphicsContext : public PipelineContainer
{
	friend GraphicsContextStrongReference PVR_API_FUNC createGraphicsContext();
	friend class ::pvr::api::impl::ComputePipeline_;
	friend class ::pvr::api::impl::GraphicsPipeline_;
	friend class ::pvr::api::impl::VertexRayPipeline_;
	friend class ::pvr::api::impl::SceneTraversalPipeline_;
	friend class ::pvr::api::impl::IndirectRayPipeline_;
	friend class ::pvr::api::impl::ResetPipeline;

public:
	/// <summary>Default constructor. Note: Object is uninitialized and unusable until init() is called.</summary>
	IGraphicsContext(Api apiType = Api::Unspecified) : _osManager(NULL), _apiType(apiType)
	{
		((ApiCapabilitiesPrivate&)_apiCapabilities)._maxglslesversion = 200;
	}
	virtual ~IGraphicsContext() {}

	/// <summary>Image Format.</summary>
	enum ImageFormat
	{
		ImageFormatRGBA,
		ImageFormatBGRA
	};

	/// <summary>Call this function to initialize the Context using the information of a specific OSManager (usually,
	/// the Shell instance).</summary>
	virtual Result init(OSManager& osManager) = 0;

	/// <summary>Release the resources held by this context.</summary>
	virtual void release() = 0;

	/// <summary>Wait until all pending operations are done</summary>
	virtual void waitIdle() = 0;

	/// <summary>take a screenshot in the specified buffer of the specified screen area.</summary>
	virtual bool screenCaptureRegion(uint32 x, uint32 y, uint32 w, uint32 h, byte* buffer, ImageFormat imageFormat) = 0;

	/// <summary>Print information about this IGraphicsContext.</summary>
	virtual std::string getInfo()const = 0;

	/// <summary>Gets the format of the presentation image.</summary>
	virtual const ImageDataFormat getPresentationImageFormat() const = 0;

	/// <summary>Gets the format of the depth stencil image.</summary>
	virtual const ImageDataFormat getDepthStencilImageFormat() const = 0;

	/// <summary>Get the IPlatformContext object powering this GraphicsContext.</summary>
	IPlatformContext& getPlatformContext() const { return *_platformContext; }

	/// <summary>Query if a specific extension is supported.</summary>
	/// <param name="extension">A c-style string representing the extension.</param>
	/// <returns>True if the extension is supported.</returns>
	virtual bool isExtensionSupported(const char8* extension) const = 0;

	/// <summary>Create a GraphicsPipeline object associated with this context.</summary>
	/// <param name="createParam">The CreateParameters used to create the pipeline.</param>
	/// <returns>A newly constructed GraphicsPipeline. Contains NULL if creation failed.</returns>
	virtual api::GraphicsPipeline createGraphicsPipeline(const api::GraphicsPipelineCreateParam& createParam) = 0;

	/// <summary>Create GraphicsPipeline as a child of another provided GraphicsPipeline.</summary>
	/// <param name="createParam">The CreateParameters used to create the pipeline. NOTE the create param won't
	/// inherit the states from the parent pipeline. The Application must set the states that are to be inherited from
	/// parent pipeline see ParentableGraphicsPipeline::getCreateParam() function</param>
	/// <param name="parent">The parent pipeline used to create the new pipeline. If the pipeline does not belong to
	/// this context, the results are undefined.</param>
	/// <returns>A newly constructed GraphicsPipeline object, child to "parent". Contains NULL if creation failed.
	/// </returns>
	/// <remarks>Use parenting to denote hierarchies of pipelines to take advantage of very important optimisations
	/// that are performed when switching from a child to a parent or a sibling and vice versa.</remarks>
	virtual api::GraphicsPipeline createGraphicsPipeline(const api::GraphicsPipelineCreateParam& createParam,
	    api::ParentableGraphicsPipeline parent) = 0;

	/// <summary>Create a new parentable GraphicsPipeline object.</summary>
	/// <param name="createParam">The CreateParameters used to create the pipeline.</param>
	/// <returns>A newly constructed ParentableGraphicsPipeline object. Contains NULL if creation failed.</returns>
	/// <remarks>ParentableGraphicsPipeline s contain additional information that allows them to be used as parents to other Graphics
	/// Pipelines.</remarks>
	virtual api::ParentableGraphicsPipeline  createParentableGraphicsPipeline(const api::GraphicsPipelineCreateParam& createParam) = 0;

	virtual api::ParentableGraphicsPipeline createParentableGraphicsPipeline(const api::GraphicsPipelineCreateParam& desc,
	    const api::ParentableGraphicsPipeline& parent) = 0;



	/// <summary>Create a new ComputePipeline object.</summary>
	/// <param name="createParam">The CreateParameters used to create the pipeline.</param>
	/// <returns>A newly constructed ComputePipeline object. Contains NULL if creation failed.</returns>
	virtual api::ComputePipeline createComputePipeline(const api::ComputePipelineCreateParam& createParam) = 0;

	/// <summary>Create a new Sampler object.</summary>
	/// <param name="createParam">The CreateParameters used to create the object.</param>
	/// <returns>A newly constructed Sampler object. Contains NULL if creation failed.</returns>
	virtual api::Sampler createSampler(const api::SamplerCreateParam& createParam) = 0;

	/// <summary>Create a new Texture object.</summary>
	/// <returns>A newly created, unallocated Texture. Contains NULL if creation failed.</returns>
	virtual api::TextureStore createTexture() = 0;

	/// <summary>Create a new Texture object.</summary>
	/// <returns>A newly created, unallocated Texture. Contains NULL if creation failed.</returns>
	virtual api::TextureView createTextureView(const api::TextureStore& texture, types::ImageSubresourceRange range,
	    types::SwizzleChannels = types::SwizzleChannels()) = 0;

	/// <summary>Create a new Texture object.</summary>
	/// <returns>A newly created, unallocated Texture. Contains NULL if creation failed.</returns>
	virtual api::TextureView createTextureView(const api::TextureStore& texture,
	    types::SwizzleChannels = types::SwizzleChannels()) = 0;

	/// <summary>Create a new Shader-Accessible view to Buffer Object (Can be used as UBO/SSBO etc), with an existing
	/// backing buffer. BufferViews allow Buffers to be added to DescriptorSets.</summary>
	/// <param name="buffer">The buffer that the UBO will be a view of.</param>
	/// <param name="offset">The starting offset in "buffer" that the UBO will represent. Default: 0.</param>
	/// <param name="range">The size of the original buffer that this view will represent/access. Default: the
	/// remaining size of the buffer (minus offset).</param>
	/// <returns>A newly created BufferView object, wrapping the specified range of the provided buffer.</returns>
	/// <remarks>The UBO created will be representing, in Buffer, range of (offset, offset+range)</remarks>
	virtual api::BufferView createBufferView(const api::Buffer& buffer, uint32 offset = 0, uint32 range = 0xFFFFFFFFu) = 0;

	/// <summary>One-step create a new Buffer with a corresponding BufferView to the entire range of the buffer.</summary>
	/// <param name="size">The size of the new VBO,</param>
	/// <param name="bufferUsage">A bitfield of all allowed buffer uses</param>
	/// <param name="isMappable">Set to true to allocate the buffer from memory that can be mapped. If this feature is
	/// not required, and the buffer will be filled from a transfer (buffer copy or cmd buffer update), set to false
	/// and ensure bufferUsage contains the TransferDst flag.</param>
	/// <returns>A newly created BufferView of an entire a newly created Buffer object.</returns>
	/// <remarks>The UBO created will be representing, in Buffer, range of (offset, offset+range)</remarks>
	virtual api::BufferView createBufferAndView(uint32 size, types::BufferBindingUse bufferUsage, bool isMappable = false) = 0;

	/// <summary>Create a new buffer object, which can then be bound as a VBO/IBO and/or shared with BufferView objects
	/// (SSBOView, VBOView)</summary>
	/// <param name="size">The size, in bytes, of the buffer</param>
	/// <param name="bufferUsage">The intended uses of the Buffer (VBO,IBO,UBO, Copy etc.)</param>
	/// <param name="isMappable">The buffer is mappable by the host</param>
	/// <returns>A newly constructed Buffer object</returns>
	virtual api::Buffer createBuffer(uint32 size, types::BufferBindingUse bufferUsage, bool isMappable = false) = 0;

	/// <summary>Create a new primary CommandBuffer on the default CommandPool</summary>
	/// <returns>A newly created CommandBuffer. Null if failed.</returns>
	virtual api::CommandBuffer createCommandBufferOnDefaultPool() = 0;

	/// <summary>Create a new secondary CommandBuffer on the default CommandPool</summary>
	/// <returns>A newly created CommandBuffer. Null if failed.</returns>
	virtual api::SecondaryCommandBuffer createSecondaryCommandBufferOnDefaultPool() = 0;

	/// <summary>Create Shader from source. Accepts an array of custom preprocessor definition directives.</summary>
	/// <param name="shaderSrc">The shader source, as a stream.</param>
	/// <param name="shaderType">The type of shader to create</param>
	/// <param name="defines">Optional. A pointer to an array of c-style strings containing preprocessor definition
	/// directives</param>
	/// <param name="numDefines">Optional. The number of items in the defines array.</param>
	virtual api::Shader createShader(const Stream& shaderSrc, types::ShaderType shaderType,
	                                 const char* const* defines = NULL, uint32 numDefines = 0) = 0;

	/// <summary>Create Shader from binary.</summary>
	/// <param name="shaderData">The shader binary data, as a stream</param>
	/// <param name="shaderType">The type of shader to create</param>
	/// <param name="binaryFormat">The format of the data.</param>
	virtual api::Shader createShader(Stream& shaderData, types::ShaderType shaderType,
	                                 types::ShaderBinaryFormat binaryFormat) = 0;

	/// <summary>Create a Framebuffer Object with specified parameters and backing image. Not for on-screen rendering.
	/// </summary>
	/// <param name="createParam">The creation parameters for this Fbo.</param>
	/// <returns>A newly created FBO. Null if failed.</returns>
	/// <remarks>This method is used to create a typical GL-style FBO (an object that is normally used to render into a
	/// texture).</remarks>
	virtual api::Fbo createFbo(const api::FboCreateParam& createParam) = 0;

	/// <summary>Create a Framebuffer Object with specified parameters and backing image. Not for on-screen rendering.
	/// </summary>
	/// <param name="createParams">The creation parameters for this Fbo.</param>
	/// <returns>A newly created FBO. Null if failed.</returns>
	/// <remarks>This method is used to create a typical GL-style FBO (an object that is normally used to render into a
	/// texture).</remarks>
	virtual api::FboSet createFboSet(const Multi<api::FboCreateParam>& createParams) = 0;


	/// <summary>Create an FBO that represents the actual backbuffer (i.e. an fbo to be used for on-screen rendering),
	/// using the RenderPass to use for it. compatible with the actual BackBuffer format and options - this is the
	/// user's responsibility. Allows the user to specify additional color view attachments for the FBO to use.
	/// </summary>
	/// <param name="renderPass">The renderpass that the new FBO will use. Must be compatible with the backbuffer.
	/// </param>
	/// <param name="onScreenFboCreateParam">An additional set of creation parameters to use for the fbo.</param>
	/// <param name="swapIndex">The Index of the SwapBuffer Image (backbuffer) that the new FBO will be attached to.
	/// </param>
	/// <returns>A new FBO who can be used to write to the Screen.</returns>
	/// <remarks>This version uses a RenderPass that was provided by the user. The color and Depth/Stencil formats must
	/// be compatible with the backbuffer, otherwise results are undefined.</remarks>
	virtual api::Fbo createOnScreenFboWithRenderPass(uint32 swapIndex, const api::RenderPass& renderPass,
	    const api::OnScreenFboCreateParam& onScreenFboCreateParam) = 0;

	/// <summary>Create aset of FBOs (one per backbuffer image) that represents the actual backbuffer (i.e. an fbo to be
	/// used for on-screen rendering), using the RenderPass to use for it. compatible with the actual BackBuffer
	/// format and options - this is the user's responsibility.</summary>
	/// <param name="renderPass">The renderpass that the new FBO will use. Must be compatible with the backbuffer.
	/// </param>
	/// <returns>A new FBO who can be used to write to the Screen.</returns>
	/// <remarks>This version uses a RenderPass that was provided by the user. The color and Depth/Stencil formats must
	/// be compatible with the backbuffer, otherwise results are undefined.</remarks>
	virtual api::FboSet createOnScreenFboSetWithRenderPass(const api::RenderPass& renderPass) = 0;

	/// <summary>Create an FBO that represents the actual backbuffer (i.e. an fbo to be used for on-screen rendering),
	/// using the RenderPass to use for it. compatible with the actual BackBuffer format and options - this is the
	/// user's responsibility.</summary>
	/// <param name="renderPass">The renderpass that the new FBO will use. Must be compatible with the backbuffer.
	/// </param>
	/// <param name="onScreenFboCreateParams">A vector with additional color view attachments per swap chain.
	/// </param>
	/// <returns>A new FBO who can be used to write to the Screen.</returns>
	/// <remarks>This version uses a RenderPass that was provided by the user. The color and Depth/Stencil formats must
	/// be compatible with the backbuffer, otherwise results are undefined.</remarks>
	virtual api::FboSet createOnScreenFboSetWithRenderPass(const api::RenderPass& renderPass,
	    const api::OnScreenFboCreateParamSet& onScreenFboCreateParams) = 0;

	/// <summary>Create an FBO that represents the actual backbuffer (i.e. an fbo to be used for on-screen rendering),
	/// using the RenderPass to use for it. compatible with the actual BackBuffer format and options - this is the
	/// user's responsibility.</summary>
	/// <param name="renderPass">The renderpass that the new FBO will use. Must be compatible with the backbuffer.
	/// </param>
	/// <param name="swapIndex">The Index of the SwapBuffer Image (backbuffer) that the new FBO will be attached to.
	/// </param>
	/// <returns>A new FBO who can be used to write to the Screen.</returns>
	/// <remarks>This version uses a RenderPass that was provided by the user. The color and Depth/Stencil formats must
	/// be compatible with the backbuffer, otherwise results are undefined.</remarks>
	virtual api::Fbo createOnScreenFboWithRenderPass(uint32 swapIndex, const api::RenderPass& renderPass) = 0;

	/// <summary>Create an FBO that represents the actual backbuffer (i.e. an fbo to be used for on-screen rendering),
	/// using the most common parameters.</summary>
	/// <param name="colorLoadOp">The Load Operation for the color attachment (Default is LoadOp::Clear, clearing the
	/// color of the screen at every frame start)</param>
	/// <param name="colorStoreOp">The Store Operation for the color attachment (Default is StoreOp::Store, storing
	/// the color before buffer swapping)</param>
	/// <param name="depthLoadOp">The Load Operation for the depth (Default is LoadOp::Clear, clearing the Depth at
	/// every frame start)</param>
	/// <param name="depthStoreOp">The Store Operation for the depth buffer (Default is StoreOp::Ignore, discarding
	/// the Depth before buffer swapping)</param>
	/// <param name="stencilLoadOp">The Load Operation for the stencil buffer (Default is LoadOp::Clear, clearing the
	/// Stencil at every frame start)</param>
	/// <param name="stencilStoreOp">The Store Operation for the stencil buffer (Default is StoreOp::Ignore,
	/// discarding the Stencil before buffer swapping)</param>
	/// <returns>A new FBO who can be used to write to the Screen.</returns>
	/// <remarks>This version uses the most typical parameters used for the backbuffer, and also creates the RenderPass
	/// for the FBO. Uses a single subpass at color attachment 0. Use the FBO->getRenderPass method if you wish to
	/// retrieve the renderpass created with this method. WARNING: Defaults discard Depth and Stencil at the end of
	/// the renderpass, for performance. This may cause unintended results as it is different from default OpenGL. If
	/// you wish to preserve depth and/or stencil after the renderpass, please specify StoreOp:Store for depth and
	/// stencil.</remarks>
	virtual api::FboSet createOnScreenFboSet(
	  types::LoadOp colorLoadOp = types::LoadOp::Clear, types::StoreOp colorStoreOp = types::StoreOp::Store,
	  types::LoadOp depthLoadOp = types::LoadOp::Clear, types::StoreOp depthStoreOp = types::StoreOp::Ignore,
	  types::LoadOp stencilLoadOp = types::LoadOp::Clear, types::StoreOp stencilStoreOp = types::StoreOp::Ignore) = 0;

	/// <summary>Create an FBO that represents the actual backbuffer (i.e. an fbo to be used for on-screen rendering),
	/// using the most common parameters.</summary>
	/// <param name="swapIndex">Swapchain index</param>
	/// <param name="colorLoadOp">The Load Operation for the color attachment (Default is LoadOp::Clear, clearing the
	/// color of the screen at every frame start)</param>
	/// <param name="colorStoreOp">The Store Operation for the color attachment (Default is StoreOp::Store, storing
	/// the color before buffer swapping)</param>
	/// <param name="depthLoadOp">The Load Operation for the depth (Default is LoadOp::Clear, clearing the Depth at
	/// every frame start)</param>
	/// <param name="depthStoreOp">The Store Operation for the depth buffer (Default is StoreOp::Ignore, discarding
	/// the Depth before buffer swapping)</param>
	/// <param name="stencilLoadOp">The Load Operation for the stencil buffer (Default is LoadOp::Clear, clearing the
	/// Stencil at every frame start)</param>
	/// <param name="stencilStoreOp">The Store Operation for the stencil buffer (Default is StoreOp::Ignore,
	/// discarding the Stencil before buffer swapping)</param>
	/// <returns>A new FBO who can be used to write to the Screen.</returns>
	/// <remarks>This version uses the most typical parameters used for the backbuffer, and also creates the RenderPass
	/// for the FBO. Uses a single subpass at color attachment 0. Use the FBO->getRenderPass method if you wish to
	/// retrieve the renderpass created with this method. WARNING: Defaults discard Depth and Stencil at the end of
	/// the renderpass, for performance. This may cause unintended results as it is different from default OpenGL. If
	/// you wish to preserve depth and/or stencil after the renderpass, please specify StoreOp:Store for depth and
	/// stencil.</remarks>
	virtual api::Fbo createOnScreenFbo(
	  uint32 swapIndex, types::LoadOp colorLoadOp = types::LoadOp::Clear,
	  types::StoreOp colorStoreOp = types::StoreOp::Store, types::LoadOp depthLoadOp = types::LoadOp::Clear,
	  types::StoreOp depthStoreOp = types::StoreOp::Ignore, types::LoadOp stencilLoadOp = types::LoadOp::Clear,
	  types::StoreOp stencilStoreOp = types::StoreOp::Ignore) = 0;


	virtual api::RenderPass createOnScreenRenderpass(
	  types::LoadOp colorLoadOp = types::LoadOp::Clear,
	  types::StoreOp colorStoreOp = types::StoreOp::Store,
	  types::LoadOp depthLoadOp = types::LoadOp::Clear,
	  types::StoreOp depthStoreOp = types::StoreOp::Ignore,
	  types::LoadOp stencilLoadOp = types::LoadOp::Clear,
	  types::StoreOp stencilStoreOp = types::StoreOp::Ignore) = 0;


	/// <summary>Create a new render pass.</summary>
	/// <param name="renderPassDesc">The creation parameters used to create the renderPass.</param>
	/// <returns>A new RenderPass object. Contains NULL if failed.</returns>
	virtual api::RenderPass createRenderPass(const api::RenderPassCreateParam& renderPassDesc) = 0;

	//virtual api::RenderPass createOnScreenRenderPass(const api::RenderPassCreateParam rpInfo) = 0;

//	virtual api::RenderPass createOnScreenRenderPass(
//	  types::LoadOp colorLoadOp = types::LoadOp::Clear, types::StoreOp colorStoreOp = types::StoreOp::Store,
//	  types::LoadOp depthLoadOp = types::LoadOp::Clear, types::StoreOp depthStoreOp = types::StoreOp::Ignore,
//	  types::LoadOp stencilLoadOp = types::LoadOp::Clear, types::StoreOp stencilStoreOp = types::StoreOp::Ignore) = 0;

	/// <summary>Create a new DescriptorPool (descriptor set allocation pool).</summary>
	/// <param name="createParam">The creation parameters used to create the descriptor pool</param>
	/// <returns>A new DescriptorPool object. Contains NULL if failed.</returns>
	virtual api::DescriptorPool createDescriptorPool(const api::DescriptorPoolCreateParam& createParam) = 0;

	/// <summary>Create a new DescriptorSet on the PVRApi default allocation pool.</summary>
	/// <param name="layout">The layout of the DescriptorSet object</param>
	/// <returns>A new DescriptorSet object</returns>
	virtual api::DescriptorSet createDescriptorSetOnDefaultPool(const api::DescriptorSetLayout& layout) = 0;

	/// <summary>Create a new DescriptorSetLayout.</summary>
	/// <param name="createParam">The creation parameters used to create the descriptor set layout.</param>
	/// <returns>A new DescriptorSetLayout object. Contains NULL if failed.</returns>
	/// <remarks>A DescriptorSetLayout contains the information required to create "compatible" descriptor sets.
	/// </remarks>
	virtual api::DescriptorSetLayout createDescriptorSetLayout(const api::DescriptorSetLayoutCreateParam& createParam) = 0;

	/// <summary>Create a new PipelineLayout.</summary>
	/// <param name="createParam">The creation parameters used to create PipelineLayout.</param>
	/// <returns>A new DescriptorSetLayout object. Contains NULL if failed.</returns>
	/// <remarks>A PipelineLayout is required for several things, including binding descriptor sets.</remarks>
	virtual api::PipelineLayout createPipelineLayout(const api::PipelineLayoutCreateParam& createParam) = 0;

	/// <summary>Create a new CommandPool.</summary>
	virtual api::CommandPool createCommandPool() = 0;

	/// <summary>Get the default, automatically generated CommandPool</summary>
	virtual const api::CommandPool& getDefaultCommandPool() const = 0;

	/// <summary>Get the default, automatically generated CommandPool</summary>
	virtual api::CommandPool& getDefaultCommandPool() = 0;

	/// <summary>Get the default, automatically generated DescriptorPool.</summary>
	virtual const api::DescriptorPool& getDefaultDescriptorPool() const = 0;

	/// <summary>Get the default, automatically generated DescriptorPool.</summary>
	virtual api::DescriptorPool& getDefaultDescriptorPool() = 0;

	/// <summary>Create Fence</summary>
	/// <param name="createSignaled">Create Fence with signaled state</param>
	virtual api::Fence createFence(bool createSignaled = true) = 0;

	/// <summary>Create Semaphore</summary>
	virtual api::Semaphore createSemaphore() = 0;

	/// <summary>Upload a texture from a pvr::Texture object into the GPU </summary>
	/// <param name="texture">The texture to upload. Should be complete and contain valid data.</param>
	/// <param name="allowDecompress">Allow de-compress a compressed format if the format is not natively supported. If
	/// this is set to true, the texture is in a compressed format not supported by the GPU, and the format can
	/// be uncompressed in the CPU, the implementation will uncompress the texture and upload the uncompressed
	/// texture. If set to false, the implementation will return failure in any case the format is unsupported, even
	/// if it could have been uncompressed.</param>
	/// <returns>The api texture to upload into. Will contain a newly created textureview even if
	/// it contained another one before.</param>
	/// <returns>Result::Success on success, errorcode otherwise</returns>
	virtual api::TextureView uploadTexture(const ::pvr::Texture& texture, bool allowDecompress = true) = 0;


	/// <summary>Get ApiCapabilities object representing the capabilities of this GraphicsContext.</summary>
	bool hasApiCapability(ApiCapabilities::Enum capability) const
	{
		return _apiCapabilities.supports(capability);
	}

	/// <summary>Get ApiCapabilities object representing the capabilities of this GraphicsContext.</summary>
	bool hasApiCapabilityNatively(ApiCapabilities::Enum capability) const
	{
		return _apiCapabilities.nativelySupports(capability);
	}

	/// <summary>Get ApiCapabilities object representing the capabilities of this GraphicsContext.</summary>
	bool hasApiCapabilityExtension(ApiCapabilities::Enum capability) const
	{
		return _apiCapabilities.supportsThroughExtension(capability);
	}

	/// <summary>Get ApiCapabilities object representing the capabilities of this GraphicsContext.</summary>
	const ApiCapabilities& getApiCapabilities() const { return _apiCapabilities; }

	/// <summary>Get the Api of this GraphicsContext.</summary>
	Api getApiType() const { return _apiType; }

	/// <summary>Get the DisplayAttributes associated with this GraphicsContext.</summary>
	const platform::DisplayAttributes& getDisplayAttributes()const
	{
		assertion(_osManager != NULL,  "NULL OSManager");
		return _osManager->getDisplayAttributes();
	}

	/// <summary>Query if a specific DeviceQueue type is supported.</summary>
	/// <param name="queueType">The DeviceQueueType to query support for.</param>
	/// <returns>True if queueType is supported by this context.</returns>
	bool isQueueSupported(DeviceQueueType queueType)
	{
		return ((uint32)_osManager->getDeviceQueueTypesRequired() & static_cast<uint32>(queueType)) != 0;
	}
	/// <summary>Get the number of Multi buffer supported.</summary>
	uint32 getSwapChainLength() const { return getPlatformContext().getSwapChainLength(); }

	/// <summary>Get the current swapchain index</summary>
	uint32 getSwapChainIndex() const { return getPlatformContext().getSwapChainIndex(); }

	/// <summary>Get the last swap chain index</summary>
	uint32 getLastSwapChainIndex() const { return getPlatformContext().getLastSwapChainIndex(); }

	/*!*********************************************************************************************
	\brief  Create a new VertexRayPipeline.
	\param  desc The creation parameters used to create VertexRayPipeline.
	\return A new VertexRayPipeline object. Contains NULL if failed.
	\description A VertexRayPipeline encapsulates all the state and actions that take place when a ray intersects an object.
	************************************************************************************************/
	virtual api::VertexRayPipeline createVertexRayPipeline(const api::VertexRayPipelineCreateParam& desc) = 0;

	/*!*********************************************************************************************
	\brief  Create a new SceneTraversalPipeline.
	\param  desc The creation parameters used to create SceneTraversalPipeline.
	\return A new SceneTraversalPipeline object. Contains NULL if failed.
	\description A SceneTraversalPipeline encapsulates all the state required to emit rays.
	************************************************************************************************/
	virtual api::SceneTraversalPipeline createSceneTraversalPipeline(const api::SceneTraversalPipelineCreateParam& desc) = 0;

	/*!*********************************************************************************************
	\brief  Create a new IndirectRayPipeline.
	\param  desc The creation parameters used to create IndirectRayPipeline.
	\return A new IndirectRayPipeline object. Contains NULL if failed.
	\description A IndirectRayPipeline encapsulates all the state required for a default ray program to be created.
	************************************************************************************************/
	virtual api::IndirectRayPipeline createIndirectRayPipeline(const api::IndirectRayPipelineCreateParam& desc) = 0;

	/*!*********************************************************************************************
	\brief  Create a new SceneHierarchy.
	\param  createParam The creation parameters used to create SceneHierarchy.
	\return A new SceneHierarchy object. Contains NULL if failed.
	\description A SceneHierarchy is used to store the result of a scene hierarchy build.
	************************************************************************************************/
	virtual api::SceneHierarchy createSceneHierarchy(const api::SceneHierarchyCreateParam& createParam) = 0;

	virtual SharedContext createSharedContext(uint32 contextId) = 0;


protected:
	IPlatformContext* _platformContext;
	OSManager* _osManager;
	Api _apiType;
	ApiCapabilities _apiCapabilities;
};

class ISharedContext
{
protected:
	GraphicsContext _context;
	std::auto_ptr<ISharedPlatformContext> _platformContext;
	ISharedContext(const GraphicsContext& context, std::auto_ptr<ISharedPlatformContext> platformContext):
		_context(context), _platformContext(platformContext)
	{
	}

public:
	virtual ISharedPlatformContext& getSharedPlatformContext() = 0;
	GraphicsContext getGraphicsContext() { return _context; }
	virtual api::TextureAndFence uploadTextureDeferred(const ::pvr::Texture& texture, bool allowDecompress = true) = 0;
	//virtual api::TextureAndFence createGraphicsPipelineDeferred() = 0;

};
}
