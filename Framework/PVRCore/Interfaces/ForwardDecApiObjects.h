/*!
\brief Contains forwards declarations of pvr::api objects
\file PVRCore/Interfaces/ForwardDecApiObjects.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/CoreIncludes.h"
#include "PVRCore/Base/RefCounted.h"

namespace pvr {

class IGraphicsContext;
class IPlatformContext;
struct ImageDataFormat;

enum class FrameworkCaps
{
	MaxColorAttachments      = 8, //!< Max Color attachment supported by the fbo
	MaxDepthStencilAttachments = 8,
	MaxInputAttachments      = 8,
	MaxResolveAttachments    = 8,
	MaxPreserveAttachments	 = 8,
	MaxDescriptorSetBindings = 4,
	MaxDescriptorDynamicOffsets = 32,
	MaxSwapChains			 = 4,
};

//!\cond NO_DOXYGEN
namespace legacyPfx {
namespace impl {
class EffectApi_;
}
}
//!\endcond

namespace api {

//!\cond NO_DOXYGEN
namespace impl {
class ResetPipeline;
class GraphicsPipeline_;
class ComputePipeline_;
class ParentableGraphicsPipeline_;
class TextureStore_;
class Fbo_;
class Buffer_;
class Sampler_;
class TextureView_;
class BufferView_;
class Shader_;
class RenderPass_;
class DescriptorSet_;
class DescriptorSetLayout_;
class DescriptorPool_;
class CommandBufferBase_;
class CommandBuffer_;
class SecondaryCommandBuffer_;
class PipelineLayout_;
class CommandPool_;
class Fence_;
class Semaphore_;
class Event_;
class SemaphoreSet_;
class FenceSet_;
class EventSet_;

class SceneHierarchy_;
class RayProgram_;
class IndirectRayPipeline_;
class VertexRayPipeline_;
class SceneTraversalPipeline_;

class ParentableIndirectRayPipeline_;
class ParentableVertexRayPipeline_;
class ParentableSceneTraversalPipeline_;
}
//!\endcond
//SPECIAL CASE
class MemoryBarrierSet;

class IBindable;
class IIndexBindable;
struct GraphicsPipelineCreateParam;
struct RenderPassCreateParam;
struct ComputePipelineCreateParam;
struct FboCreateParam;
struct DescriptorSetLayoutCreateParam;
struct DescriptorPoolCreateParam;
struct DescriptorSetUpdate;
struct PipelineLayoutCreateParam;
struct OnScreenFboCreateParam;

typedef Multi<FboCreateParam, (uint32)FrameworkCaps::MaxSwapChains> FboCreateParamSet;
typedef Multi<OnScreenFboCreateParam, (uint32)FrameworkCaps::MaxSwapChains> OnScreenFboCreateParamSet;
struct VertexRayPipelineCreateParam;
struct SceneTraversalPipelineCreateParam;
struct SceneHierarchyCreateParam;
struct IndirectRayPipelineCreateParam;

/// <summary>Framebuffer Object.</summary>
typedef RefCountedResource<impl::Fbo_> Fbo;

/// <summary>Framebuffer Object Set.</summary>
typedef Multi<Fbo, (uint32)FrameworkCaps::MaxSwapChains> FboSet;

/// <summary>Buffer Object.</summary>
typedef RefCountedResource<impl::Buffer_> Buffer;

/// <summary>GraphicsPipeline.</summary>
typedef RefCountedResource<impl::GraphicsPipeline_> GraphicsPipeline;

/// <summary>ComputePipeline.</summary>
typedef RefCountedResource<impl::ComputePipeline_> ComputePipeline;

/// <summary>IndirectRayPipeline.</summary>
typedef RefCountedResource<impl::IndirectRayPipeline_> IndirectRayPipeline;

/// <summary>VertexRayPipeline.</summary>
typedef RefCountedResource<impl::VertexRayPipeline_> VertexRayPipeline;

/// <summary>SceneTraversalPipeline.</summary>
typedef RefCountedResource<impl::SceneTraversalPipeline_> SceneTraversalPipeline;

/// <summary>Parentable GraphicsPipeline can be used as a parent to other GraphicsPipeline objects.</summary>
typedef RefCountedResource<impl::ParentableGraphicsPipeline_> ParentableGraphicsPipeline;

/// <summary>Parentable SceneTraversalPipeline can be used as a parent to other SceneTraversalPipeline objects.</summary>
typedef RefCountedResource<impl::ParentableSceneTraversalPipeline_> ParentableSceneTraversalPipeline;

/// <summary>Parentable IndirectRayPipeline can be used as a parent to other IndirectRayPipeline objects.</summary>
typedef RefCountedResource<impl::ParentableIndirectRayPipeline_> ParentableIndirectRayPipeline;

/// <summary>Parentable VertexRayPipeline can be used as a parent to other VertexRayPipeline objects.</summary>
typedef RefCountedResource<impl::ParentableVertexRayPipeline_> ParentableVertexRayPipeline;

/// <summary>Sampler.</summary>
typedef RefCountedResource<impl::Sampler_> Sampler;

/// <summary>A generic Buffer. Can be directly bound as a VBO /IBO or wrapped with a BufferView(SsboView,
/// UboView) to be bound via a DescriptorSet</summary>
typedef RefCountedResource<impl::BufferView_> BufferView;

/// <summary>An Shader object.</summary>
typedef RefCountedResource<impl::Shader_> Shader;

/// <summary>An Renderpass object represents a drawing cycle that ends up rendering to a single FBO.</summary>
typedef RefCountedResource<impl::RenderPass_> RenderPass;

/// <summary>A DescriptorSet represents a collection of resources (Textures, Buffers, Samplers, etc.) that can
/// all be bound together for use by a rendering run.</summary>
typedef RefCountedResource<impl::DescriptorSet_> DescriptorSet;

/// <summary>A DescriptorSet Layout represents a "recipe" for a descriptor set. It is used for other objects to
/// ensure compatibility with a specific DescriptorSet family.</summary>
typedef RefCountedResource<impl::DescriptorSetLayout_> DescriptorSetLayout;

typedef std::array<DescriptorSetLayout, (uint32)FrameworkCaps::MaxDescriptorSetBindings> DescriptorSetLayoutSet;

/// <summary>A backing store for any kind of texture.</summary>
typedef RefCountedResource<impl::TextureStore_> TextureStore;

/// <summary>Base class for the view of any kind of texture view.</summary>
typedef RefCountedResource<impl::TextureView_> TextureView;

/// <summary>A descriptor pool represents a specific chunk of memory from which descriptor pools will be
/// allocated. It is intended that different threads will use different descriptor pools to avoid having contention
/// and the need to lock between them.</summary>
typedef EmbeddedRefCountedResource<impl::DescriptorPool_> DescriptorPool;

/// <summary>A CommandBuffer(Base) represents a string of commands that will be submitted to the GPU in a batch.
/// </summary>
typedef RefCountedResource<impl::CommandBufferBase_>CommandBufferBase;
/// <summary>A CommandBuffer(Primary) is a CommandBuffer that can be submitted to the GPU and can contain
/// secondary command buffers</summary>
typedef RefCountedResource<impl::CommandBuffer_> CommandBuffer;
/// <summary>A SecondaryCommandBufferis a CommandBuffer that can only be submitted to a primary CommandBuffer
/// and cannot contain a RenderPass</summary>
typedef RefCountedResource<impl::SecondaryCommandBuffer_> SecondaryCommandBuffer;

/// <summary>A PipelineLayout represents the blueprint out of which a pipeline will be created, needed by other
/// objects to ensure compatibility with a family of GraphicsPipelines.</summary>
typedef RefCountedResource<impl::PipelineLayout_> PipelineLayout;

/// <summary>Forwared-declared reference-counted handle to a Buffer. For detailed documentation, see PVRApi module</summary>
/// <remarks>Default constructor returns an empty handle that wraps a NULL object. Use the IGraphicsContext's
/// createBuffer to construct a Buffer. As with all reference-counted handles, access with the arrow operator.
/// </remarks>
typedef RefCountedResource<impl::Buffer_> Buffer;


/// <summary>Forwared-declared reference-counted handle to a Buffer. For detailed documentation, see PVRApi module</summary>
/// <remarks>Default constructor returns an empty handle that wraps a NULL object. Use the IGraphicsContext's
/// createBufferView to construct a Buffer. As with all reference-counted handles, access with the arrow operator.
/// </remarks>
typedef EmbeddedRefCountedResource<impl::CommandPool_> CommandPool;



/// <summary>Forwared-declared reference-counted handle to a Buffer. For detailed documentation, see PVRApi module</summary>
/// <remarks>Default constructor returns an empty handle that wraps a NULL object. Use the IGraphicsContext's
/// createBufferView to construct a Buffer. As with all reference-counted handles, access with the arrow operator.
/// </remarks>
typedef RefCountedResource<impl::Fence_> Fence;



/// <summary>Forwared-declared reference-counted handle to a Buffer. For detailed documentation, see PVRApi module</summary>
/// <remarks>Default constructor returns an empty handle that wraps a NULL object. Use the IGraphicsContext's
/// createBufferView to construct a Buffer. As with all reference-counted handles, access with the arrow operator.
/// </remarks>
typedef RefCountedResource<impl::Semaphore_> Semaphore;



/// <summary>Forwared-declared reference-counted handle to a Buffer. For detailed documentation, see PVRApi module</summary>
/// <remarks>Default constructor returns an empty handle that wraps a NULL object. Use the IGraphicsContext's
/// createBufferView to construct a Buffer. As with all reference-counted handles, access with the arrow operator.
/// </remarks>
typedef RefCountedResource<impl::Event_> Event;

typedef RefCountedResource<impl::EventSet_> EventSet;
typedef RefCountedResource<impl::FenceSet_> FenceSet;
typedef RefCountedResource<impl::SemaphoreSet_> SemaphoreSet;

/// <summary>Forwared-declared reference-counted handle to a SceneHierarchy. For detailed documentation, see PVRApi module</summary>
/// <remarks>Default constructor returns an empty handle that wraps a NULL object. Use the IGraphicsContext's createSceneHierarchy
/// to construct a SceneHierarchy. As with all reference-counted handles, access with the arrow operator.
/// </remarks>
typedef EmbeddedRefCountedResource<impl::SceneHierarchy_> SceneHierarchy;
}

//!\cond NO_DOXYGEN
namespace native {
struct HContext_;				//!< Handle to the api's Context, for use with Utilities

struct HBuffer_;				//!< Handle to the api's specifics buffer
struct HBufferView_;            //!< Handle to the api's BufferView, if it exists
struct HColorAttachmentView_;   //!< Handle to the api's specifics color attachment
struct HCommandPool_;           //!< Handle to the api's CommandPool, if it exists
struct HCommandBuffer_;         //!< Handle to the api's CommandBuffer, if it exists
struct HDepthStencilView_;      //!< Handle to the api's depth Stencil attachment view
struct HDescriptorSetLayout_;   //!< Handle to the api's DescriptorSetLayout, if it exists
struct HDescriptorSet_;         //!< Handle to the api's DescriptorSet, if it exists
struct HDescriptorPool_;        //!< Handle to the api's DescriptorPool, if it exists
struct HFbo_;					//!< Handle to the api's specific Frame-Buffer-Object
struct HImageView_;             //!< Handle to the api's ImageView, if it exists
struct HPipeline_;              //!< Handle to the api's PipelineLayout, if it exists
struct HPipelineLayout_;        //!< Handle to the api's PipelineLayout, if it exists
struct HRenderPass_;            //!< Handle to the api's RenderPass, if it exists
struct HSampler_;				//!< Handle to the api's specific sampler
struct HShader_;				//!< Handle to the api's specifics shader
struct HTexture_;				//!< Handle to the api's specific texture
struct HFence_;
struct HSemaphore_;
struct HEvent_;
struct HIndirectPipeline_;        //!< Handle to the api's Indirect Pipeline, if it exists
struct HSceneHierarchy_;        //!< Handle to the api's Scene Hierarchy, if it exists

typedef RefCountedResource<HFbo_> HFbo; //!< Pointer to the api's specific Frame-Buffer-Object
typedef RefCountedResource<HTexture_> HTexture; //!< Pointer to the api's specific Texture
typedef RefCountedResource<HSampler_> HSampler; //!< Pointer to the api's specific Sampler
typedef RefCountedResource<HBuffer_> HBuffer; //!< Pointer to the api's specific Buffer
typedef RefCountedResource<HShader_> HShader; //!< Pointer to the api's specific Shader
typedef RefCountedResource<HDepthStencilView_> HDepthStencilView;//!< Pointer to the api's specific Depth Stencil Attachment view
typedef RefCountedResource<HDescriptorSetLayout_> HDescriptorSetLayout;//!< Pointer to the api's specific Descriptor Set Layout
typedef RefCountedResource<HDescriptorSet_> HDescriptorSet;//!< Pointer to the api's specific Descriptor Set
typedef RefCountedResource<HDescriptorPool_> HDescriptorPool;//!< Pointer to the api's specific Descriptor Pool
typedef EmbeddedRefCountedResource<HCommandPool_> HCommandPool;//!< Pointer to the api's specific Descriptor Pool
typedef RefCountedResource<HCommandBuffer_> HCommandBuffer;//!< Pointer to the api's specific  CommandBuffer
typedef RefCountedResource<HRenderPass_> HRenderPass;//!< Pointer to the api's specific  RenderPass
typedef RefCountedResource<HPipelineLayout_> HPipelineLayout;//!< Pointer to the api's specific  PipelineLayout
typedef RefCountedResource<HPipeline_> HPipeline;//!< Pointer to the api's specific  PipelineLayout
typedef RefCountedResource<HBufferView_> HBufferView;//!< Pointer to the api's specific  BufferView
typedef RefCountedResource<HImageView_> HImageView;//!< Pointer to the api's specific  ImageView
typedef RefCountedResource<HFence_> HFence;//!< Pointer to the api's specific  Fence
typedef RefCountedResource<HSemaphore_> HSemaphore;//!< Pointer to the api's specific  Semaphore
typedef RefCountedResource<HEvent_> HEvent;//!< Pointer to the api's specific  Event
typedef RefCountedResource<HIndirectPipeline_> HIndirectPipeline;//!< Pointer to the api's specific  Indirect Pipeline
typedef RefCountedResource<HSceneHierarchy_> HSceneHierarchy;//!< Pointer to the api's specific  Scene Hierarchy
}
//!\endcond
}