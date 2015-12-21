/*!*********************************************************************************************************************
\file         PVRCore/ForwardDecApiObjects.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief			Contains forwards declarations of pvr::api objects
***********************************************************************************************************************/
#pragma once
#include "PVRCore/CoreIncludes.h"
#include "PVRApi/ApiObjectTypes.h"

namespace pvr {

class IGraphicsContext;
class IPlatformContext;

namespace api {

//!\cond NO_DOXYGEN
namespace impl {
class ResetPipeline;
class GraphicsPipelineImpl;
class ComputePipelineImpl;
class ParentableGraphicsPipelineImpl;
class TextureStoreImpl;
class FboImpl;
class BufferImpl;
class SamplerImpl;
class EffectApiImpl;
class TextureViewImpl;
class BufferViewImpl;
class UboViewImpl;
class SsboViewImpl;
class AtomicBufferViewImpl;
class ShaderImpl;
class RenderPassImpl;
struct ColorAttachmentViewImpl;
struct DepthStencilViewImpl;
class DescriptorSetImpl;
class DescriptorSetLayoutImpl;
struct DescriptorSetLayoutChainDescriptorImpl;
class TextureStoreImpl;
class DescriptorPoolImpl;
class CommandBufferBaseImpl;
class CommandBufferImpl;
class SecondaryCommandBufferImpl;
class PipelineLayoutImpl;
}
//!\endcond

class IBindable;
class IIndexBindable;
class AssetLoadingDelegate;
struct GraphicsPipelineCreateParam;
struct RenderPassCreateParam;
struct ComputePipelineCreateParam;
struct FboCreateParam;
struct DescriptorSetLayoutCreateParam;
struct ColorAttachmentViewCreateParam;
struct DepthStencilViewCreateParam;
struct DescriptorPoolCreateParam;
struct DescriptorSetUpdateParam;
struct PipelineLayoutCreateParam;

/*!********************************************************************************************************************
\brief        Framebuffer Object.
***********************************************************************************************************************/
typedef RefCountedResource<impl::FboImpl> Fbo;

/*!********************************************************************************************************************
\brief        Buffer Object.
***********************************************************************************************************************/
typedef RefCountedResource<impl::BufferImpl> Buffer;

/*!********************************************************************************************************************
\brief        GraphicsPipeline.
***********************************************************************************************************************/
typedef RefCountedResource<impl::GraphicsPipelineImpl> GraphicsPipeline;

/*!********************************************************************************************************************
\brief        ComputePipeline.
***********************************************************************************************************************/
typedef RefCountedResource<impl::ComputePipelineImpl> ComputePipeline;

/*!********************************************************************************************************************
\brief        Parentable GraphicsPipeline can be used as a parent to other GraphicsPipeline objects.
***********************************************************************************************************************/
typedef RefCountedResource<impl::ParentableGraphicsPipelineImpl> ParentableGraphicsPipeline;

/*!********************************************************************************************************************
\brief        Sampler.
***********************************************************************************************************************/
typedef RefCountedResource<impl::SamplerImpl> Sampler;

/*!********************************************************************************************************************
\brief        An Effect that has been cooked to render with with a specified API.
***********************************************************************************************************************/
typedef RefCountedResource<impl::EffectApiImpl> EffectApi;

/*!********************************************************************************************************************
\brief  A generic Buffer. Can be directly bound as a VBO /IBO or wrapped with a BufferView(SsboView, UboView) to
		be bound via a DescriptorSet
***********************************************************************************************************************/
typedef RefCountedResource<impl::BufferViewImpl> BufferView;

/*!********************************************************************************************************************
\brief        A Uniform Buffer Object view of a Buffer.
***********************************************************************************************************************/
typedef RefCountedResource<impl::UboViewImpl> UboView;

/*!********************************************************************************************************************
\brief        A Shader Storage Buffer Object view of a Buffer.
***********************************************************************************************************************/
typedef RefCountedResource<impl::SsboViewImpl> SsboView;

/*!********************************************************************************************************************
\brief        An Atomic Buffer Object view of a Buffer.
***********************************************************************************************************************/
typedef RefCountedResource<impl::AtomicBufferViewImpl> AtomicBufferView;

/*!********************************************************************************************************************
\brief        An Shader object.
***********************************************************************************************************************/
typedef RefCountedResource<impl::ShaderImpl> Shader;

/*!********************************************************************************************************************
\brief        An Renderpass object represents a drawing cycle that ends up rendering to a single FBO.
***********************************************************************************************************************/
typedef RefCountedResource<impl::RenderPassImpl> RenderPass;

/*!********************************************************************************************************************
\brief        An ColorAttachmentView object.
***********************************************************************************************************************/
typedef RefCountedResource<impl::ColorAttachmentViewImpl> ColorAttachmentView;

/*!********************************************************************************************************************
\brief        A DepthStencilView object.
***********************************************************************************************************************/
typedef RefCountedResource<impl::DepthStencilViewImpl> DepthStencilView;

/*!********************************************************************************************************************
\brief        A DescriptorSet represents a collection of resources (Textures, Buffers, Samplers, etc.) that can all be
bound together for use by a rendering run.
***********************************************************************************************************************/
typedef RefCountedResource<impl::DescriptorSetImpl> DescriptorSet;

/*!********************************************************************************************************************
\brief        A DescriptorSet Layout represents a "recipe" for a descriptor set. It is used for other objects to ensure
compatibility with a specific DescriptorSet family.
***********************************************************************************************************************/
typedef RefCountedResource<impl::DescriptorSetLayoutImpl> DescriptorSetLayout;

/*!********************************************************************************************************************
\brief        A backing store for any kind of texture.
***********************************************************************************************************************/
typedef RefCountedResource<impl::TextureStoreImpl> TextureStore;

/*!********************************************************************************************************************
\brief        Base class for the view of any kind of texture view.
***********************************************************************************************************************/
typedef RefCountedResource<impl::TextureViewImpl> TextureView;

/*!********************************************************************************************************************
\brief        A descriptor pool represents a specific chunk of memory from which descriptor pools will be allocated.
It is intended that different threads will use different descriptor pools to avoid having contention and
the need to lock between them.
***********************************************************************************************************************/
typedef RefCountedResource<impl::DescriptorPoolImpl> DescriptorPool;

/*!********************************************************************************************************************
\brief        A CommandBuffer(Base) represents a string of commands that will be submitted to the GPU in a batch.
***********************************************************************************************************************/
typedef RefCountedResource<impl::CommandBufferBaseImpl>CommandBufferBase;
/*!********************************************************************************************************************
\brief        A CommandBuffer(Primary) is a CommandBuffer that can be submitted to the GPU and can contain secondary command buffers
***********************************************************************************************************************/
typedef RefCountedResource<impl::CommandBufferImpl> CommandBuffer;
/*!********************************************************************************************************************
\brief        A SecondaryCommandBufferis a CommandBuffer that can only be submitted to a primary CommandBuffer and cannot contain a RenderPass
***********************************************************************************************************************/
typedef RefCountedResource<impl::SecondaryCommandBufferImpl> SecondaryCommandBuffer;

/*!********************************************************************************************************************
\brief       A PipelineLayout represents the blueprint out of which a pipeline will be created, needed by other objects
to ensure compatibility with a family of GraphicsPipelines.
***********************************************************************************************************************/
typedef RefCountedResource<impl::PipelineLayoutImpl> PipelineLayout;

/*!****************************************************************************************************************
\brief Reference-counted handle to a Uniform Buffer object view of a Buffer.
Default constructor returns an empty handle that wraps a NULL object.
Use the IGraphicsContext's createUbo to construct a UboView.
As with all reference-counted handles, access with the arrow operator.
*******************************************************************************************************************/
typedef RefCountedResource<impl::UboViewImpl> UboView;

/*!****************************************************************************************************************
\brief Reference-counted handle to a Shader Storage Buffer object view of a Buffer.
Default constructor returns an empty handle that wraps a NULL object.
Use the IGraphicsContext's createUbo to construct a SsboView.
As with all reference-counted handles, access with the arrow operator.
*******************************************************************************************************************/
typedef RefCountedResource<impl::SsboViewImpl> SsboView;



/*!****************************************************************************************************************
\brief Forwared-declared reference-counted handle to a Buffer. For detailed documentation, see PVRApi module
\remarks Default constructor returns an empty handle that wraps a NULL object.
Use the IGraphicsContext's createUbo to construct a Buffer.
As with all reference-counted handles, access with the arrow operator.
*******************************************************************************************************************/
typedef RefCountedResource<impl::BufferImpl> Buffer;

}

//!\cond NO_DOXYGEN
namespace native {
struct HFbo_;					//!< Handle to the api's specific Frame-Buffer-Object
struct HTexture_;				//!< Handle to the api's specific texture
struct HSampler_;				//!< Handle to the api's specific sampler
struct HBuffer_;				//!< Handle to the api's specifics buffer
struct HShader_;				//!< Handle to the api's specifics shader
struct HShaderProgram_;			//!< Handle to the api's specifics compiled shader program
struct HColorAttachmentView_;   //!< Handle to the api's specifics color attachment
struct HDepthStencilView_;      //!< Handle to the api's depth Stencil attachment view
struct HDescriptorSetLayout_;   //!< Handle to the api's DescriptorSetLayout, if it exists
struct HDescriptorSet_;         //!< Handle to the api's DescriptorSet, if it exists
struct HDescriptorPool_;        //!< Handle to the api's DescriptorPool, if it exists
struct HCommandBuffer_;         //!< Handle to the api's CommandBuffer, if it exists
struct HRenderPass_;            //!< Handle to the api's RenderPass, if it exists
struct HPipelineLayout_;        //!< Handle to the api's PipelineLayout, if it exists

typedef RefCountedResource<HFbo_> HFbo; //!< Pointer to the api's specific Frame-Buffer-Object
typedef RefCountedResource<HTexture_> HTexture; //!< Pointer to the api's specific Texture
typedef RefCountedResource<HSampler_> HSampler; //!< Pointer to the api's specific Sampler
typedef RefCountedResource<HBuffer_> HBuffer; //!< Pointer to the api's specific Buffer
typedef RefCountedResource<HShader_> HShader; //!< Pointer to the api's specific Shader
typedef RefCountedResource<HShaderProgram_> HShaderProgram;//!< Pointer to the api's specific Shader Program
typedef RefCountedResource<HColorAttachmentView_> HColorAttachmentView;//!< Pointer to the api's specific color Attachment view
typedef RefCountedResource<HDepthStencilView_> HDepthStencilView;//!< Pointer to the api's specific Depth Stencil Attachment view
typedef RefCountedResource<HDescriptorSetLayout_> HDescriptorSetLayout;//!< Pointer to the api's specific Descriptor Set Layout
typedef RefCountedResource<HDescriptorSet_> HDescriptorSet;//!< Pointer to the api's specific Descriptor Set
typedef RefCountedResource<HDescriptorPool_> HDescriptorPool;//!< Pointer to the api's specific Descriptor Pool
typedef RefCountedResource<HCommandBuffer_> HCommandBuffer;//!< Pointer to the api's specific  CommandBuffer
typedef RefCountedResource<HRenderPass_> HRenderPass;//!< Pointer to the api's specific  RenderPass
typedef RefCountedResource<HPipelineLayout_> HPipelineLayout;//!< Pointer to the api's specific  PipelineLayout

}
//!\endcond
}
