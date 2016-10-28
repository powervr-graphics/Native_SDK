/*!*********************************************************************************************************************
\file         PVRCore/ForwardDecApiObjects.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief			Contains forwards declarations of pvr::api objects
***********************************************************************************************************************/
#pragma once
#include "PVRCore/CoreIncludes.h"
#include "PVRCore/RefCounted.h"

namespace pvr {

class IGraphicsContext;
class IPlatformContext;

enum class FrameworkCaps
{
	MaxColorAttachments      = 8, //!< Max Color attachment supported by the fbo
    MaxDepthStencilAttachments = 8,
	MaxInputAttachments      = 8,
	MaxResolveAttachments    = 8,
	MaxPreserveAttachments	 = 8,
	MaxDescriptorSetBindings = 4,
	MaxSwapChains			 = 4,
};


namespace legacyPfx {
namespace impl {
class EffectApi_;
}
}


namespace api{

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
}
//!\endcond
//SPECIAL CASE
class MemoryBarrierSet;

class IBindable;
class IIndexBindable;
class AssetLoadingDelegate;
struct GraphicsPipelineCreateParam;
struct RenderPassCreateParam;
struct ComputePipelineCreateParam;
struct FboCreateParam;
struct DescriptorSetLayoutCreateParam;
struct DescriptorPoolCreateParam;
struct DescriptorSetUpdate;
struct PipelineLayoutCreateParam;
struct OnScreenFboCreateParam;
struct ImageDataFormat;

/*!********************************************************************************************************************
\brief        Framebuffer Object.
***********************************************************************************************************************/
typedef RefCountedResource<impl::Fbo_> Fbo;

/*!********************************************************************************************************************
\brief        Framebuffer Object Set.
***********************************************************************************************************************/
typedef Multi<Fbo, (uint32)FrameworkCaps::MaxSwapChains> FboSet;

/*!********************************************************************************************************************
\brief        Buffer Object.
***********************************************************************************************************************/
typedef RefCountedResource<impl::Buffer_> Buffer;

/*!********************************************************************************************************************
\brief        GraphicsPipeline.
***********************************************************************************************************************/
typedef RefCountedResource<impl::GraphicsPipeline_> GraphicsPipeline;

/*!********************************************************************************************************************
\brief        ComputePipeline.
***********************************************************************************************************************/
typedef RefCountedResource<impl::ComputePipeline_> ComputePipeline;

/*!********************************************************************************************************************
\brief        Parentable GraphicsPipeline can be used as a parent to other GraphicsPipeline objects.
***********************************************************************************************************************/
typedef RefCountedResource<impl::ParentableGraphicsPipeline_> ParentableGraphicsPipeline;

/*!********************************************************************************************************************
\brief        Sampler.
***********************************************************************************************************************/
typedef RefCountedResource<impl::Sampler_> Sampler;

/*!********************************************************************************************************************
\brief        An Effect that has been cooked to render with with a specified API.
***********************************************************************************************************************/
typedef RefCountedResource<legacyPfx::impl::EffectApi_> EffectApi;

/*!********************************************************************************************************************
\brief  A generic Buffer. Can be directly bound as a VBO /IBO or wrapped with a BufferView(SsboView, UboView) to
		be bound via a DescriptorSet
***********************************************************************************************************************/
typedef RefCountedResource<impl::BufferView_> BufferView;

/*!********************************************************************************************************************
\brief        An Shader object.
***********************************************************************************************************************/
typedef RefCountedResource<impl::Shader_> Shader;

/*!********************************************************************************************************************
\brief        An Renderpass object represents a drawing cycle that ends up rendering to a single FBO.
***********************************************************************************************************************/
typedef RefCountedResource<impl::RenderPass_> RenderPass;

/*!********************************************************************************************************************
\brief        A DescriptorSet represents a collection of resources (Textures, Buffers, Samplers, etc.) that can all be
bound together for use by a rendering run.
***********************************************************************************************************************/
typedef RefCountedResource<impl::DescriptorSet_> DescriptorSet;

/*!********************************************************************************************************************
\brief        A DescriptorSet Layout represents a "recipe" for a descriptor set. It is used for other objects to ensure
compatibility with a specific DescriptorSet family.
***********************************************************************************************************************/
typedef RefCountedResource<impl::DescriptorSetLayout_> DescriptorSetLayout;

/*!********************************************************************************************************************
\brief        A backing store for any kind of texture.
***********************************************************************************************************************/
typedef RefCountedResource<impl::TextureStore_> TextureStore;

/*!********************************************************************************************************************
\brief        Base class for the view of any kind of texture view.
***********************************************************************************************************************/
typedef RefCountedResource<impl::TextureView_> TextureView;

/*!********************************************************************************************************************
\brief        A descriptor pool represents a specific chunk of memory from which descriptor pools will be allocated.
It is intended that different threads will use different descriptor pools to avoid having contention and
the need to lock between them.
***********************************************************************************************************************/
typedef EmbeddedRefCountedResource<impl::DescriptorPool_> DescriptorPool;

/*!********************************************************************************************************************
\brief        A CommandBuffer(Base) represents a string of commands that will be submitted to the GPU in a batch.
***********************************************************************************************************************/
typedef RefCountedResource<impl::CommandBufferBase_>CommandBufferBase;
/*!********************************************************************************************************************
\brief        A CommandBuffer(Primary) is a CommandBuffer that can be submitted to the GPU and can contain secondary command buffers
***********************************************************************************************************************/
typedef RefCountedResource<impl::CommandBuffer_> CommandBuffer;
/*!********************************************************************************************************************
\brief        A SecondaryCommandBufferis a CommandBuffer that can only be submitted to a primary CommandBuffer and cannot contain a RenderPass
***********************************************************************************************************************/
typedef RefCountedResource<impl::SecondaryCommandBuffer_> SecondaryCommandBuffer;

/*!********************************************************************************************************************
\brief       A PipelineLayout represents the blueprint out of which a pipeline will be created, needed by other objects
to ensure compatibility with a family of GraphicsPipelines.
***********************************************************************************************************************/
typedef RefCountedResource<impl::PipelineLayout_> PipelineLayout;

/*!****************************************************************************************************************
\brief Forwared-declared reference-counted handle to a Buffer. For detailed documentation, see PVRApi module
\remarks Default constructor returns an empty handle that wraps a NULL object.
Use the IGraphicsContext's createBuffer to construct a Buffer.
As with all reference-counted handles, access with the arrow operator.
*******************************************************************************************************************/
typedef RefCountedResource<impl::Buffer_> Buffer;


/*!****************************************************************************************************************
\brief Forwared-declared reference-counted handle to a Buffer. For detailed documentation, see PVRApi module
\remarks Default constructor returns an empty handle that wraps a NULL object.
Use the IGraphicsContext's createBufferView to construct a Buffer.
As with all reference-counted handles, access with the arrow operator.
*******************************************************************************************************************/
typedef EmbeddedRefCountedResource<impl::CommandPool_> CommandPool;



/*!****************************************************************************************************************
\brief Forwared-declared reference-counted handle to a Buffer. For detailed documentation, see PVRApi module
\remarks Default constructor returns an empty handle that wraps a NULL object.
Use the IGraphicsContext's createBufferView to construct a Buffer.
As with all reference-counted handles, access with the arrow operator.
*******************************************************************************************************************/
typedef RefCountedResource<impl::Fence_> Fence;



/*!****************************************************************************************************************
\brief Forwared-declared reference-counted handle to a Buffer. For detailed documentation, see PVRApi module
\remarks Default constructor returns an empty handle that wraps a NULL object.
Use the IGraphicsContext's createBufferView to construct a Buffer.
As with all reference-counted handles, access with the arrow operator.
*******************************************************************************************************************/
typedef RefCountedResource<impl::Semaphore_> Semaphore;



/*!****************************************************************************************************************
\brief Forwared-declared reference-counted handle to a Buffer. For detailed documentation, see PVRApi module
\remarks Default constructor returns an empty handle that wraps a NULL object.
Use the IGraphicsContext's createBufferView to construct a Buffer.
As with all reference-counted handles, access with the arrow operator.
*******************************************************************************************************************/
typedef RefCountedResource<impl::Event_> Event;

typedef RefCountedResource<impl::EventSet_> EventSet;
typedef RefCountedResource<impl::FenceSet_> FenceSet;
typedef RefCountedResource<impl::SemaphoreSet_> SemaphoreSet;
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

}
//!\endcond
}
