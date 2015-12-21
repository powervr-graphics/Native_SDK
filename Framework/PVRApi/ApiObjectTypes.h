/*!*********************************************************************************************************************
\file         PVRApi\ApiObjectTypes.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Fundamental types and enumerations used by many API classes.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/Types.h"
#include "PVRCore/RefCounted.h"
#include "PVRAssets/SamplerDescription.h"

namespace pvr {
namespace api {
using assets::VertexAttributeLayout;

/*!*********************************************************************************************************************
\brief Enumeration of the possible types of FrameBufferObject attachment (depth, stencil, depthstencil, color).
***********************************************************************************************************************/
namespace FboAttachmentType {
enum Enum
{
	Depth, //!< Depth
	Stencil, //!< Stencil
	DepthStencil, //!< DepthStencil
	Color //!< Color
};
}

/*!*********************************************************************************************************************
\brief Enumeration of the possible types of a Pipeline Binding point (Graphics, Compute).
***********************************************************************************************************************/
namespace PipelineBindingPoint {
enum Enum {	Graphics, Compute };
}//namespace PipelineBindingPoint

/*!*********************************************************************************************************************
\brief Enumeration of the possible way of recording commands for each subpasses of the render pass (Inline, SecondaryCommandBuffer).
***********************************************************************************************************************/
namespace RenderPassContents {
enum Enum
{
	Inline,//< commands are recorded within the command buffer for the subpass
	SecondaryCommandBuffers//< commands are recorded in the secondary commandbuffer for the subpass
};
}

/*!*********************************************************************************************************************
\brief Enumeration of the possible binding points of a Framebuffer Object (Read, Write, ReadWrite).
***********************************************************************************************************************/
namespace FboBindingTarget {
enum Enum
{
	Read = 1, //< bind Fbo for read
	Write = 2, //< bind fbo for write
	ReadWrite = 3 //< bind fbo for read and write
};
}


/*!*********************************************************************************************************************
\brief Logic operations (toggle, clear, and etc.).
***********************************************************************************************************************/
namespace LogicOp {
enum Enum
{
	Copy,
	Clear,
	And,
	AndReverse,
	AndInverted,
	NoOp,
	Xor,
	Or,
	Nor,
	Equiv,
	Invert,
	OrReverse,
	CopyInverted,
	OrInverted,
	Nand,
	Set,

	Count
};
}

/*!********************************************************************************************
\brief ChannelWriteMask enable/ disable writting to channel bits.
***********************************************************************************************/
namespace ColorChannel {
enum Enum
{
	R = 0x01, //< write to red channel
	G = 0x02, //< write to green channel
	B = 0x04, //< write to blue channel
	A = 0x08, //< write to alpha channel
	None = 0, //< don't write to any channel
	All = R | G | B | A //< write to all channel
};
typedef pvr::uint32 Bits;
}

/*!********************************************************************************************
\brief Step rate for a vertex attribute when drawing: Per vertex, per instance, per draw.
**********************************************************************************************/
namespace StepRate {
enum Enum
{
	Vertex, //< Step rate Per vertex
	Instance,//< Step rate per instance
	Draw //< step rate per draw
};
}
/*!*********************************************************************************************
\brief Depth Range Mode.
***********************************************************************************************/
namespace DepthMode {
enum Enum
{
	ZeroToOne, //< min 0, max 1
	NegativeOneToOne //< min -1, max 1
};
}

/*!*********************************************************************************************
\brief Enumeration of Provoking Vertex modes.
***********************************************************************************************/
namespace ProvokingVertex {
enum Enum
{
	First, Last
};
}

/*!*********************************************************************************************
\brief Enumeration of Coordinate Origins.
***********************************************************************************************/
namespace CoordinateOrigin {
enum Enum
{
	UpperLeft, LowerLeft
};
}

/*!*********************************************************************************************************************
\brief Enumeration of all FrameBufferObject texture targets.
***********************************************************************************************************************/
namespace FboTextureTarget {
enum Enum
{
	TextureTarget2d, TextureTargetCubeMapPositiveX, TextureTargetCubeMapNegativeX, TextureTargetCubeMapPositiveY,
	TextureTargetCubeMapNegativeY, TextureTargetCubeMapPositiveZ, TextureTargetCubeMapNegativeZ, Unknown
};
}

/*!*********************************************************************************************************************
\brief Enumeration of polygon filling modes.
***********************************************************************************************************************/
namespace FillMode {
enum Enum
{
	FrontBackFill,///< enum value. fill polygon front and back, solid (default)
	FrontFill,///<	enum value. fill polygon front, solid
	BackFill,///< enum value. fill polygon back, solid
	FrontBackWireFrame,///< enum fill front and back, wire frame
	FrontWireFrame,///< fill front, wireframe
	BackWireFrame,///< fill back wireframe
	NumFillMode,
};
}

/*!*********************************************************************************************************************
\brief Enumeration of optimisation flags for CommandBuffers.
***********************************************************************************************************************/
namespace CmdBufferOptimizeFlags {
enum Enum
{
	SmallBatch          = 0x01,
	PipelineSwitch      = 0x02,
	DescriptorSetSwitch = 0x04,
	NoSimultaneousUse   = 0x10  //< secondarycommand buffer cannot be used more than once in a given primary command buffer
};
typedef pvr::uint32 Bits;
}


/*!*********************************************************************************************************************
\brief Enumeration of Face facing (front, back...).
***********************************************************************************************************************/
namespace Face {
enum Enum
{
	None = 0,
	Back = 1,
	Front = 2,
	FrontBack = 3,
};
}

/*!*********************************************************************************************************************
\brief Enumeration of the blend operations (determine how a new pixel (source color) is combined with a pixel already in the
       framebuffer (destination color).
***********************************************************************************************************************/
namespace BlendOp {
enum Enum
{
	Add,
	Subtract,
	ReverseSubtract,
	Min,
	Max,
	NumBlendFunc,
	Default = Add
};
}

/*!*********************************************************************************************************************
\brief Specfies how the rgba blending facors are computed for source and destination fragments.
***********************************************************************************************************************/
namespace BlendFactor {
enum Enum : uint8
{
	Zero,
	One,
	SrcColor,
	OneMinusSrcColor,
	DstColor,
	OneMinusDstColor,
	SrcAlpha,
	OneMinusSrcAlpha,
	DstAlpha,
	OneMinusDstAlpha,
	ConstantColor,
	OneMinusConstantColor,
	ConstantAlpha,
	OneMinusConstantAlpha,
	SrcAlphaSaturate,
	NumBlendFactor,
	Default = One
};
}

/*!*********************************************************************************************************************
\brief Enumeration of comparison mode for comparison samplers.
***********************************************************************************************************************/
namespace SamplerCompareMode {
enum Enum { CompareRefToTexture, None };
};

/*!*********************************************************************************************************************
\brief Enumeration of Interpolation types for Samplers (nearest, linear).
***********************************************************************************************************************/
namespace InterpolationMode {
enum Enum { Nearest, Linear };
};

/*!********************************************************************************************
\brief Enumeration of the different front face to winding order correlations.
***********************************************************************************************/
namespace PolygonWindingOrder {
enum Enum { FrontFaceCW, FrontFaceCCW, Default = FrontFaceCCW };
};

/*!********************************************************************************************
\brief Enumeration of the different stencil operations.
***********************************************************************************************/
namespace StencilOp {
enum Enum {
	Keep,
	Zero,
	Replace,
	Increment, 
	IncrementWrap, 
	Decrement,
	DecrementWrap,
	Invert, 
	NumStencilOp,

	// Defaults

	DefaultStencilFailFront = Keep,
	DefaultStencilFailBack = Keep,

	DefaultDepthFailFront = Keep,
	DefaultDepthFailBack = Keep,

	DefaultDepthStencilPassFront = Keep,
	DefaultDepthStencilPassBack = Keep
};
}

/*!********************************************************************************************
\brief Enumeration of all the different descriptor types.
***********************************************************************************************/
namespace DescriptorType {
enum Enum
{
	Sampler,
	CombinedImageSampler,
	SampledImage,
	StorageImage,
	UniformTexelBuffer,
	StorageTexelBuffer,
	UniformBuffer, //< uniform buffer
	StorageBuffer,//< storagebuffer
	UniformBufferDynamic, //< uniform buffer's range can be offseted when binding descriptor set.
	StorageBufferDynamic, //< storag buffer's range can be offseted when binding descriptor set.
	Count
};
}

/*!******************************************************************************************************
\brief Enumeration of all the different output flags that can be used in the MemoryBarrier command.
*********************************************************************************************************/
namespace MemBarrierFlagout {
enum Enum
{
	CpuWrite = 0x01,//< Controls output coherency of CPU writes.
	ShaderWrite = 0x02,   //< Controls output coherency of generic shader writes.
	ColorAttachment = 0x04,   //< Controls output coherency of color attachment writes.
	DepthStencilAttachment = 0x08,   //< Controls output coherency of depth/stencil attachment writes.
	Transfer = 0x10,   //< Controls output coherency of transfer operations.
	AllBarrier = CpuWrite | ShaderWrite | ColorAttachment | DepthStencilAttachment | Transfer,
	Count = 5,
};
}

/*!******************************************************************************************************
\brief Enumeration of all the different input flags that can be used in the MemoryBarrier command.
*********************************************************************************************************/
namespace MemBarrierFlagIn {
enum Enum
{
	CpuRead = 0x00000001,   //< Controls input coherency of CPU reads.
	IndirectCommand = 0x00000002,   //< Controls input coherency of indirect command reads.
	IndexFetch = 0x00000004,   //< Controls input coherency of index fetches.
	VertexAttributeFetch = 0x00000008,   //< Controls input coherency of vertex attribute fetches.
	UniformRead = 0x00000010,   //< Controls input coherency of uniform buffer reads.
	ShaderRead = 0x00000020,   //< Controls input coherency of generic shader reads.
	ColorAttachment = 0x00000040,   //< Controls input coherency of color attachment reads.
	DepthStencilAttachment = 0x00000080,   //< Controls input coherency of depth/stencil attachment reads.
	Transfer = 0x00000100,   //< Controls input coherency of transfer operations.
	AllBarrier = CpuRead | IndirectCommand | IndexFetch | VertexAttributeFetch | UniformRead | ShaderRead |
	             ShaderRead | ColorAttachment | DepthStencilAttachment | Transfer,
	Count = 9
};
}
}// namespace api
}// namespace pvr
