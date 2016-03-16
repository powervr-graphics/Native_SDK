/*!*********************************************************************************************************************
\file         PVRCore\Types.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Basic file used in the PowerVR Framework. Defines several types used throughout the Framework (sized arithmetic
              types, enumerations, character types).
***********************************************************************************************************************/
#pragma once

#pragma warning(disable:4512)
#pragma warning(disable:4480)
#pragma warning(disable:4100)

#include <vector>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <memory>
#include <algorithm>
#include <functional>
#include <string>

#include "PVRCore/HalfFloat.h"
#include "PVRCore/Assert_.h"


#if defined(_MSC_VER)
#define PVR_THREAD_LOCAL __declspec(thread)
#elif defined(__ANDROID__) //Unfortunately, no out-of-the box support for TLS on android.
#define PVR_THREAD_LOCAL
#elif true||defined(TARGET_OS_IPHONE)||defined(TARGET_IPHONE_SIMULATOR)
#define PVR_THREAD_LOCAL
#elif defined(__GNUC__) || defined(__clang__)
#define PVR_THREAD_LOCAL __thread
#elif (__cplusplus>=201103L)
#define PVR_THREAD_LOCAL thread_local
#else
#error Could not detect compiler type, and compiler does not report C++11 compliance for thread local storage. Please edit this file to add the corresponding thread local storage keyword in this place.
#endif

namespace pvr {

/*!*******************************************************************************************
\brief  8-bit integer unsigned type.
*********************************************************************************************/
typedef unsigned char          byte;

/*!*******************************************************************************************
\brief  Character type. 8-bit integer signed type on all currently supported platforms.
*********************************************************************************************/
typedef char                   char8;

/*!*******************************************************************************************
\brief  Wide-character type. Platform dependent.
*********************************************************************************************/
typedef	wchar_t                wchar;

#if defined(_UNICODE)
typedef wchar             tchar;
#else
typedef char8             tchar;
#endif
/*!*******************************************************************************************
\brief  String of basic characters.
*********************************************************************************************/
typedef std::basic_string<char> string;

//UTF types
/*!*******************************************************************************************
\brief  A UTF-8 (unsigned) character. 8-bit unsigned integer.
*********************************************************************************************/
typedef unsigned char          utf8;
/*!*******************************************************************************************
\brief  A UTF-16 (unsigned) character. 16-bit unsigned integer.
*********************************************************************************************/
typedef unsigned short         utf16;
/*!*******************************************************************************************
\brief  A UTF-32 (unsigned) character. 32-bit unsigned integer.
*********************************************************************************************/
typedef unsigned int           utf32;

//Signed Integer Types
/*!*******************************************************************************************
\brief  8-bit signed integer.
*********************************************************************************************/
typedef signed char            int8;

/*!*******************************************************************************************
\brief  16-bit signed integer.
*********************************************************************************************/
typedef signed short           int16;

/*!*******************************************************************************************
\brief  32-bit signed integer.
*********************************************************************************************/
typedef signed int             int32;
#if defined(_WIN32)

/*!*******************************************************************************************
\brief  64-bit signed integer.
*********************************************************************************************/
typedef signed __int64         int64;
#elif defined(__GNUC__)
__extension__
/*!*******************************************************************************************
\brief  64-bit signed integer.
*********************************************************************************************/
typedef signed long long       int64;
#else
/*!*******************************************************************************************
\brief  64-bit signed integer.
*********************************************************************************************/
typedef signed long long       int64;
#endif

//Unsigned Integer Types
/*!*******************************************************************************************
\brief  8-bit unsigned integer.
*********************************************************************************************/
typedef unsigned char          uint8;

/*!*******************************************************************************************
\brief  16-bit unsigned integer.
*********************************************************************************************/
typedef unsigned short         uint16;

/*!*******************************************************************************************
\brief  32-bit unsigned integer.
*********************************************************************************************/
typedef unsigned int           uint32;
//#if defined(_WIN32)
//typedef unsigned __int64       uint64;
//#else
typedef unsigned long long     uint64;
//#endif

//Floating Point
/*!*******************************************************************************************
\brief  16-bit floating point number (half-float).
*********************************************************************************************/
typedef HalfFloat           float16;

/*!*******************************************************************************************
\brief  32-bit floating point number (single-precision float).
*********************************************************************************************/
typedef float                  float32;

/*!*******************************************************************************************
\brief  64-bit floating point number (double-precision float).
*********************************************************************************************/
typedef double                 float64;


namespace internal {
// If the size of various types does not equal the expected size, throw a compiler error
PVR_STATIC_ASSERT(sizeof(byte) == 1, byte_size_invalid);
PVR_STATIC_ASSERT(sizeof(char8) == 1, char8_size_invalid);
PVR_STATIC_ASSERT(sizeof(utf8) == 1, utf8_size_invalid);
PVR_STATIC_ASSERT(sizeof(utf16) == 2, utf16_size_invalid);
PVR_STATIC_ASSERT(sizeof(utf32) == 4, utf32_size_invalid);
PVR_STATIC_ASSERT(sizeof(int8) == 1, int8_size_invalid);
PVR_STATIC_ASSERT(sizeof(int16) == 2, int16_size_invalid);
PVR_STATIC_ASSERT(sizeof(int32) == 4, int32_size_invalid);
PVR_STATIC_ASSERT(sizeof(int64) == 8, int64_size_invalid);
PVR_STATIC_ASSERT(sizeof(uint8) == 1, uint8_size_invalid);
PVR_STATIC_ASSERT(sizeof(uint16) == 2, uint16_size_invalid);
PVR_STATIC_ASSERT(sizeof(uint32) == 4, uint32_size_invalid);
PVR_STATIC_ASSERT(sizeof(uint64) == 8, uint64_size_invalid);
PVR_STATIC_ASSERT(sizeof(float16) == 2, float16_size_invalid);
PVR_STATIC_ASSERT(sizeof(float32) == 4, float32_size_invalid);
PVR_STATIC_ASSERT(sizeof(float64) == 8, float64_size_invalid);
}

/*!*********************************************************************************************************************
\brief         Enumeration of all API types supported by this implementation
***********************************************************************************************************************/
namespace Api {
enum Enum
{
	Unspecified = 0,
	//OpenGL,
	OpenGLES2,
	OpenGLES3,
	OpenGLES31,
	OpenGLESMaxVersion = OpenGLES31,
	Vulkan,
	Count,
};

/*!*******************************************************************************************
\brief  Get the api code
\return Api code
*********************************************************************************************/
inline const char* getApiCode(Enum api)
{
	static const char* ApiCodes[] =
	{
		"",
		"ES2",
		"ES3",
		"ES31",
		"VK",
	};
	return ApiCodes[api];
}

/*!*******************************************************************************************
\brief  Get the api family min version
\return Api family min
*********************************************************************************************/
inline Enum getApiFamilyMin(Enum api)
{
	static Enum ApiCodes[] =
	{
		Unspecified,
		OpenGLES2,
		OpenGLES2,
		OpenGLES2,
		Vulkan,
	};
	return ApiCodes[api];
}

/*!*******************************************************************************************
\brief  Get the api family max version
\return Api family Max
*********************************************************************************************/
inline Enum getApiFamilyMax(Enum api)
{
	static Enum ApiCodes[] =
	{
		Unspecified,
		OpenGLES31,
		OpenGLES31,
		OpenGLES31,
		Vulkan,
	};
	return ApiCodes[api];
}

/*!*******************************************************************************************
\brief  Get the api name string of the given Enumeration
\return Api name string
*********************************************************************************************/
inline const char* getApiName(Enum api)
{
	static const char* ApiCodes[] =
	{
		"Unknown",
		"OpenGL ES 2.0",
		"OpenGL ES 3.0",
		"OpenGL ES 3.1",
		"Vulkan",
	};
	return ApiCodes[api];
}
}

namespace types {

/*!*******************************************************************************************
\brief  Enumeration containing all possible API object types (Images, Buffers etc.).
*********************************************************************************************/
namespace ApiObjectType {
enum Enum : unsigned char
{
	UBO,
	SSBO,
	Texture,
	Sampler,
	Image,
	TexBO,
	ImageBO,
	NumTypes
};
}

/*!*********************************************************************************************
\brief  An enumeration that defines data types used throughout the Framework.
        Commonly used in places where raw data are used to define the types actually contained.
***********************************************************************************************/
namespace DataType {
enum Enum
{
	None = 0,//< none
	Float32,//< float 1
	Int32,//< integer 2
	UInt16, //< unsigned short 3
	RGBA,//< rgba 4
	ARGB,//< argb 5
	D3DCOLOR,//< d3d color 6
	UBYTE4,//< unsigned 4 byte 7
	DEC3N,
	Fixed16_16,
	UInt8,//< unsigned byte 10
	Int16,//< short 11
	Int16Norm,//< short normalized 12
	Int8,//< byte 13
	Int8Norm,//< byte normalized 14
	UInt8Norm,//< unsigned byte normalized 15
	UInt16Norm,//< unsigned short normalized
	UInt32,//< unsigned int
	ABGR,//< abgr

	Custom = 1000
};

/*!*********************************************************************************************************************
\brief Return the Size of a DataType.
\param[in] type The Data type
\return The size of the Datatype in bytes.
***********************************************************************************************************************/
inline uint32 size(Enum type)
{
	switch (type)
	{
	default:
		PVR_ASSERTION(false);
		return 0;
	case DataType::Float32:
	case DataType::Int32:
	case DataType::UInt32:
	case DataType::RGBA:
	case DataType::ABGR:
	case DataType::ARGB:
	case DataType::D3DCOLOR:
	case DataType::UBYTE4:
	case DataType::DEC3N:
	case DataType::Fixed16_16:
		return 4;
	case DataType::Int16:
	case DataType::Int16Norm:
	case DataType::UInt16:
		return 2;
	case DataType::UInt8:
	case DataType::UInt8Norm:
	case DataType::Int8:
	case DataType::Int8Norm:
		return 1;
	}
}

/*!*********************************************************************************************************************
\brief Return the number of components in a datatype.
\param[in] type The datatype
\return The number of components (e.g. float32 is 1, vec3 is 3)
***********************************************************************************************************************/
inline uint32 componentCount(Enum type)
{
	switch (type)
	{
	default:
		PVR_ASSERTION(false);
		return 0;

	case DataType::Float32:
	case DataType::Int32:
	case DataType::UInt32:
	case DataType::Int16:
	case DataType::Int16Norm:
	case DataType::UInt16:
	case DataType::Fixed16_16:
	case DataType::Int8:
	case DataType::Int8Norm:
	case DataType::UInt8:
	case DataType::UInt8Norm:
		return 1;

	case DataType::DEC3N:
		return 3;

	case DataType::RGBA:
	case DataType::ABGR:
	case DataType::ARGB:
	case DataType::D3DCOLOR:
	case DataType::UBYTE4:
		return 4;
	}
}


/*!*********************************************************************************************
\brief       Return if the format is Normalized (represents a range between 0..1 for unsigned types
            or between -1..1 for signed types)
\param       type The format to test.
\return      True if the format is Normalised.
\description A Normalised format is a value that is stored as an Integer, but that actually
represents a value from 0..1 or -1..1 instead of the numeric value
of the integer. For example, for a normalised unsigned byte value, the value
0 represents 0.0, the value 127 represents 0.5 and the value 255 represents 1.0.
***********************************************************************************************/
inline bool isNormalised(Enum type)
{
	return (type == DataType::Int8Norm || type == DataType::UInt8Norm
	        || type == DataType::Int16Norm
	        || type == DataType::UInt16Norm);
}
}// DataType

/*!***************************************************************************
\brief Enumeration of Colorspaces (Linear, SRGB).
*****************************************************************************/
namespace ColorSpace {
enum Enum
{
	lRGB,
	sRGB,
	NumSpaces
};
}

/*!*********************************************************************************************
\brief  Enumeration containing all possible Primitive topologies (Point, line trianglelist etc.).
***********************************************************************************************/
namespace PrimitiveTopology {
enum Enum
{
	//POSITION-SENSITIVE. Do not renumber unless also refactoring ConvertToVkTypes, ConvertToGlesTypes.
	// TODO: ADD ADJACENCY
	// TODO: Remove QUAD patches and refactor.
	Points = 0,
	Lines,
	LineStrip,
	LineLoop,
	TriangleList, //< triangle list
	TriangleStrips,//< triangle strip
	TriangleFan,//< triangle patch list
	TriPatchList,//< triangle patch list
	QuadPatchList,//< quad patch list};
	None
};
}

/*!*********************************************************************************************
\brief  Enumeration all possible values of operations to be performed on initially Loading a
        Framebuffer Object.
***********************************************************************************************/
namespace LoadOp {
enum Enum
{
	Load,  //< Load the contents from the fbo from previous
	Clear, //< Clear the fbo
	Ignore, //< Ignore writing to the fbo and keep old data
};
}

/*!*********************************************************************************************
\brief  Enumerates all possible values of operations to be performed when Storing to a
        Framebuffer Object.
***********************************************************************************************/
namespace StoreOp {
enum Enum
{
	Store,//< write the source to the destination
	Ignore,//< don't write the source to the destination
};
}


/*!*******************************************************************************************
\brief Enumeration of the "aspect" (or "semantics") of an image: Color, Depth, Stencil.
**********************************************************************************************/
namespace ImageAspect {
typedef uint32 Bits;
enum Enum
{
	Color = 0x1,
	Depth = 0x2,
	Stencil = 0x4,
	Metadata = 0x8,
	DepthAndStencil = Depth | Stencil,
};
}

/*!*********************************************************************************************************************
\brief Enumeration of the possible types of a Pipeline Binding point (Graphics, Compute).
***********************************************************************************************************************/
namespace PipelineBindPoint {
enum Enum {	Graphics, Compute };
}//namespace PipelineBindPoint


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
	//DO NOT REARRANGE - Direct mapping with VkLogicOp. See ConvertToVk::logicOp
	Clear = 0,
	And = 1,
	AndReverse = 2,
	Copy = 3,
	AndInverted = 4,
	NoOp = 5,
	Xor = 6,
	Or = 7,
	Nor = 8,
	Equiv = 9,
	Invert = 10,
	OrReverse = 11,
	CopyInverted = 12,
	OrInverted = 13,
	Nand = 14,
	Set = 15,
	Count
};
}

/*!********************************************************************************************
\brief ChannelWriteMask enable/ disable writting to channel bits.
***********************************************************************************************/
namespace ColorChannel {
enum Enum
{
	//DO NOT REARRANGE - Direct mapping to Vulkan
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
	Instance//< Step rate per instance
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
	Fill,///<	enum value. fill polygon front, solid
	WireFrame,///< fill front, wireframe
	Points,///< fill back wireframe
	NumFillMode,
};
}

/*!*********************************************************************************************************************
\brief Enumeration of Face facing (front, back...).
***********************************************************************************************************************/
namespace Face {
enum Enum
{
	//DO NOT REARRANGE - DIRECT TO VULKAN
	None = 0,
	Front = 1,
	Back = 2,
	FrontBack = 3,
};
}


/*!*********************************************************************************************************************
\brief Enumeration of the six faces of a Cube
***********************************************************************************************************************/
namespace CubeFace { enum Enum { PositiveX = 0, NegativeX, PositiveY, NegativeY, PositiveZ, NegativeZ }; };

/*!*********************************************************************************************************************
\brief Enumeration of Face facing (front, back...).
***********************************************************************************************************************/
namespace StencilFace {
enum Enum
{
	//DO NOT REARRANGE - DIRECT TO VULKAN
	Front = 1,
	Back = 2,
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
	//DO NOT REARRANGE - Direct mapping to Vulkan. See ConvertToVk::BlendOp
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
\brief Buffer mapping flags.
***********************************************************************************************************************/
namespace MapBufferFlags {
enum Enum { Read = 1, Write = 2, Unsynchronised = 4 };
}




/*!*********************************************************************************************************************
\brief Specfies how the rgba blending facors are computed for source and destination fragments.
***********************************************************************************************************************/
namespace BlendFactor {
enum Enum : uint8
{
	Zero = 0,
	One = 1,
	SrcColor = 2,
	OneMinusSrcColor = 3,
	DstColor = 4,
	OneMinusDstColor = 5,
	SrcAlpha = 6,
	OneMinusSrcAlpha = 7,
	DstAlpha = 8,
	OneMinusDstAlpha = 9,
	ConstantColor = 10,
	OneMinusConstantColor = 11,
	ConstantAlpha = 12,
	OneMinusConstantAlpha = 13,
	Src1Color = 15,
	OneMinusSrc1Color = 16,
	Src1Alpha = 17,
	OneMinusSrc1Alpha = 18,
	NumBlendFactor,
	Default = One
};
}

namespace DynamicState {
enum Enum
{
	//DO NOT REARRANGE - Direct mapping to Vulkan
	Viewport = 0,
	Scissor = 1,
	LineWidth = 2,
	DepthBias = 3,
	BlendConstants = 4,
	DepthBounds = 5,
	StencilCompareMask = 6,
	StencilWriteMask = 7,
	StencilReference = 8,
	Count
};
}

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
//DO NOT REARRANGE - VULKAN DIRECT MAPPING
enum Enum { FrontFaceCCW, FrontFaceCW, Default = FrontFaceCCW };
};

/*!********************************************************************************************
\brief Enumeration of the different stencil operations.
***********************************************************************************************/
namespace StencilOp {
enum Enum
{
	//DO NOT REARRANGE - VULKAN DIRECT MAPPING
	Keep,
	Zero,
	Replace,
	Increment,
	Decrement,
	Invert,
	IncrementWrap,
	DecrementWrap,
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
	StorageBufferDynamic, //< storage buffer's range can be offseted when binding descriptor set.
	InputAttachment,
	Count
};
}

/*!*********************************************************************************************
\brief  Pre-defined Capability presense values.
***********************************************************************************************/
namespace Capability {
enum Enum
{
	Unsupported,
	Immutable,
	Mutable
};
};

/*!*********************************************************************************************
\brief  An enumeration that defines a type that can use as an index, typically 16 or 32 bit int.
        Especially used in Model classes.
***********************************************************************************************/
namespace IndexType {
enum Enum
{
	IndexType16Bit = DataType::UInt16,//< 16 bit face data
	IndexType32Bit = DataType::UInt32//< 32 bit face data
};
/*!*********************************************************************************************************************
\brief Return the Size of an IndexType in bytes.
\param[in] type The Index type
\return uint32
***********************************************************************************************************************/
inline uint32 size(const IndexType::Enum type)
{
	switch (type)
	{
	default:
		PVR_ASSERTION(false);
		return false;
	case IndexType::IndexType16Bit: return 2;
	case IndexType::IndexType32Bit: return 4;
	}
}
}// namespace IndexType

/*!*********************************************************************************************
\brief  An enumeration that defines Comparison operations (equal, less or equal etc.).
        Especially used in API classes for functions like depth testing.
***********************************************************************************************/
namespace ComparisonMode {
enum Enum
{
	//DIRECT MAPPING FOR VULKAN - DO NOT REARRANGE
	Never = 0,
	Less = 1,
	Equal = 2,
	LessEqual = 3,
	Greater = 4,
	NotEqual = 5,
	GreaterEqual = 6,
	Always = 7,
	None = 8,
	NumComparisonMode,
	Default = Always,
	DefaultStencilOpFront = Always,
	DefaultStencilOpBack = Always,
};
}

/*!*********************************************************************************************************************
\brief        Enumeration describing a filtering type of a specific dimension. In order to describe the filtering mode
              properly, you would have to define a Minification filter, a Magnification filter and a Mipmapping
              minification filter. Possible values: Nearest, Linear, None.
***********************************************************************************************************************/
namespace SamplerFilter {
enum Enum
{
	Nearest,//< nearest filter
	Linear,//< linear filter
	None,//< no filter

	Size, //< number of supported filter
	Default = Linear,//< default filter
	MipDefault = Linear//< default mip filter
};
}

/*!*********************************************************************************************************************
\brief        Enumeration describing default border colors for textures.
***********************************************************************************************************************/
namespace BorderColor {
enum Enum
{
	TransparentBlack, //< Black Border with alpha 0 : (0,0,0,0)
	OpaqueBlack, //< Black border with alpha 1 : (0,0,0,1)
	OpaqueWhite, //< white border with alpha 1: (1,1,1,1)
	Count
};
}

/*!*********************************************************************************************************************
\brief   Enumeration for defining texture wrapping mode: Repeat, Mirror, Clamp, Border.
***********************************************************************************************************************/
namespace SamplerWrap {
enum Enum
{
	Repeat,//< repeat
	MirrorRepeat,//< mirror repeat
	Clamp,//< clamp
	Border,//< border
	MirrorClamp,//< mirror clamp
	Size,//< number of support sampler wrap
	Default = Repeat //< default wrap
};
}

/*!*********************************************************************************************************************
\brief Enumeration of Texture dimensionalities (1D/2D/3D).
***********************************************************************************************************************/
namespace TextureDimension {
enum Enum
{
	Unallocated,
	Texture1D, //!< 1 dimesional texture
	Texture2D,//!< 2 dimanional texture
	Texture3D,//!< 3 dimesional texture
	Texture2DCube,//!< cube texture
	Texture1DArray,//!< 1 dimesional array texture
	Texture2DArray,//!< 2 dimesional array texture
	Texture3DArray,//!< 3 dimesional array texture
	Texture2DCubeArray,//!< 2 dimesional array texture
	TextureUnknown,//!< 3 dimesional array texture
};
}

/*!*****************************************************************************************************************
\brief  Enum values for defining whether a variable is float, integer or bool.
*******************************************************************************************************************/
namespace EffectDefaultDataInternalType {
enum Enum
{
	Float,//!< Float
	Integer,//!< Integer
	Boolean//!< Boolean
};
}

/*!*****************************************************************************************************************
\brief  Enumeration of the type of render required for an effect.
*******************************************************************************************************************/
namespace EffectPassType {
enum Enum
{
	Null,		//!< Null pass
	Camera,		//!< Camera
	PostProcess,	//!< Post-process
	EnvMapCube,	//!< Environment cube-map
	EnvMapSph,	//!< Environment sphere map
	Count		//!< Number of supported pass
};
}

/*!*****************************************************************************************************************
\brief  Enum values for the various variable types supported by Semantics.
*******************************************************************************************************************/
namespace SemanticDataType {
enum Enum
{
	Mat2,//!< 2x2 matrix
	Mat3,//!< 3x3 matrix
	Mat4,//!< 4x4 matrix
	Vec2,//!< 2d vector
	Vec3,//!< 3d vector
	Vec4,//!< 4d vector
	IVec2,//!< 2d integer vector
	IVec3,//!< 3d integer vector
	IVec4,//!< 4d integer vector
	BVec2,//!< 2d bool vector
	BVec3,//!< 3d bool vector
	BVec4,//!< 4d bool vector
	Float,//!< float
	Int1,//!< integer
	Bool1,//!< bool

	Count,//!< number of supported semantic type
	None,

	// Conceptual data types
	RGB,//!< Semantic RGB
	RGBA//!< Semantic RGBA
};
}// namespace SemanticDefaultDataType

/*!*****************************************************************************************************************
\brief   Enumeration Describes the type of different Effect Passes.
*******************************************************************************************************************/
namespace EffectPassView {
enum Enum
{
	Current,			//!< The scene's active camera is used
	PodCamera,		//!< The specified camera is used
	None				//!< No specified view
};
}// namespace EffectPassView

/*!********************************************************************************************************************
\brief        Enumeration of the binary shader formats.
***********************************************************************************************************************/
namespace ShaderBinaryFormat {
enum Enum
{
	ImgSgx,
	Spv,
	Unknown,
	None
};
}// namespace ShaderBinaryFormat

/*!********************************************************************************************************************
\brief        Enumeration of all supported shader types.
***********************************************************************************************************************/
namespace ShaderType {
enum Enum
{
	UnknownShader = 0,//!< unknown shader type
	VertexShader,//!< vertex shader
	FragmentShader,//!< fragment shader
	ComputeShader,//!< compute shader
	TesselationShader,
	FrameShader,//!< frame shader
	RayShader,//!< ray shader
	Count
};
}// ShaderType



/*!********************************************************************************************************************
\brief        Enumeration of all supported buffer access type flags.
***********************************************************************************************************************/
namespace BufferUse {
enum Flags
{
	CPU_READ = 1,
	CPU_WRITE = 2,
	GPU_READ = 4,
	GPU_WRITE = 8,

	DEFAULT = GPU_READ | GPU_WRITE,
	DYNAMIC = GPU_READ | CPU_WRITE,
	STAGING = GPU_WRITE | CPU_READ
};
}

/*!********************************************************************************************************************
\brief        Enumeration of all supported buffer use types.
***********************************************************************************************************************/
namespace BufferBindingUse {
typedef uint32 Bits;
enum Enum
{
	TransferSrc			= 0x00000001,
	TransferDest		= 0x00000002,
	UniformTexelBuffer  = 0x00000004,
	StorageTexelBuffer  = 0x00000008,
	UniformBuffer       = 0x00000010,
	StorageBuffer		= 0x00000020,
	IndexBuffer			= 0x00000040,
	VertexBuffer		= 0x00000080,
	IndirectBuffer		= 0x00000100,
	Count				= 10
};
}

/*!********************************************************************************************************************
\brief        Enumeration of Descriptor Pool use types (once, dynamic).
***********************************************************************************************************************/
//TODO MARKED AS DEPRECTAED
namespace DescriptorPoolUsage {
enum Enum
{
	OneShot,
	Dynamic
};
}

/*!********************************************************************************************************************
\brief        Enumeration of Descriptor Set use types (once, dynamic).
***********************************************************************************************************************/
namespace DescriptorSetUsage {
enum Enum
{
	OneShot,
	Static
};
}

/*!*********************************************************************************************************************
\brief Enumeration of all shader stages.
***********************************************************************************************************************/
namespace ShaderStageFlags {
typedef pvr::uint32 Bits;
enum Enum
{
	Vertex = 0x00000001, //< Vertex Shader stage
	TesselationControl = 0x00000002,
	TesselationEvaluation = 0x00000004,
	Geometry = 0x00000008,
	Fragment = 0x00000010,//< Fragment Shader stage
	Compute = 0x00000020,//< Compute Shader stage
	AllGraphicsStages = 0x0000001F,//< Vertex + Fragment shader stage
	AllStages = 0x7FFFFFFF,
	NUM_SHADER_STAGES = Compute
};
/*!*********************************************************************************************************************
\brief Bitwise OR shader stage flags.
\return The Bitwise OR of the operands
***********************************************************************************************************************/
inline types::ShaderStageFlags::Enum operator|(types::ShaderStageFlags::Enum lhs, types::ShaderStageFlags::Enum rhs)
{
	return (types::ShaderStageFlags::Enum)(static_cast<uint32>(lhs) | static_cast<uint32>(rhs));
}

/*!*********************************************************************************************************************
\brief Bitwise OR with assignment shader stage flags.
\return The left hand side of the operation, which has been bitwise ORed with the right hand side
***********************************************************************************************************************/
inline types::ShaderStageFlags::Enum& operator|=(types::ShaderStageFlags::Enum& lhs, types::ShaderStageFlags::Enum rhs)
{
	return lhs = (types::ShaderStageFlags::Enum)(static_cast<uint32>(lhs) | static_cast<uint32>(rhs));
}
}// ShaderStageFlags


/*!*********************************************************************************************************************
\brief Enumeration of all Pipeline stages.
***********************************************************************************************************************/
namespace PipelineStageFlags {
typedef uint32 Bits;
enum Enum
{
	TopOfPipeline = 0x00000001,
	DrawIndirect = 0x00000002,
	VertexInput = 0x00000004,
	VertexShader = 0x00000008,
	TessellationControl = 0x00000010,
	TessellationEvaluation = 0x00000020,
	GeometryShader = 0x00000040,
	FragmentShader = 0x00000080,
	EarlyFragmentTests = 0x00000100,
	LateFragmentTests = 0x00000200,
	ColorAttachmentOutput = 0x00000400,
	ComputeShader = 0x00000800,
	Transfer = 0x00001000,
	Host = 0x00002000,
	AllGraphics = 0x00004000,
	AllCommands = 0x00008000,
};
/*!*********************************************************************************************************************
\brief Bitwise OR shader stage flags.
\return The Bitwise OR of the operands
***********************************************************************************************************************/
inline types::PipelineStageFlags::Bits operator|(types::PipelineStageFlags::Enum lhs, types::PipelineStageFlags::Enum rhs)
{
	return (types::PipelineStageFlags::Bits)(static_cast<uint32>(lhs) | static_cast<uint32>(rhs));
}

/*!*********************************************************************************************************************
\brief Bitwise OR with assignment shader stage flags.
\return The left hand side of the operation, which has been bitwise ORed with the right hand side
***********************************************************************************************************************/
inline types::ShaderStageFlags::Enum& operator|=(types::ShaderStageFlags::Enum& lhs, types::ShaderStageFlags::Enum rhs)
{
	return lhs = (types::ShaderStageFlags::Enum)(static_cast<uint32>(lhs) | static_cast<uint32>(rhs));
}
}// ShaderStageFlags


/*!*********************************************************************************************************************
\brief Enumeration of all shader stages.
***********************************************************************************************************************/
namespace AccessFlags {
typedef pvr::uint32 Bits;
enum Enum
{
	IndirectCommandRead = 0x00000001,
	IndexRead = 0x00000002,
	VertexAttributeRead = 0x00000004,
	UniformRead = 0x00000008,
	InputAttachmentRead = 0x00000010,
	ShaderRead = 0x00000020,
	ShaderWrite = 0x00000040,
	ColorAttachmentRead = 0x00000080,
	ColorAttachmentWrite = 0x00000100,
	DepthStencilAttachmentRead = 0x00000200,
	DepthStencilAttachmentWrite = 0x00000400,
	TransferRead = 0x00000800,
	TransferWrite = 0x00001000,
	HostRead = 0x00002000,
	HostWrite = 0x00004000,
	MemoryRead = 0x00008000,
	MemoryWrite = 0x00010000,
};
/*!*********************************************************************************************************************
\brief Bitwise OR shader stage flags.
\return The Bitwise OR of the operands
***********************************************************************************************************************/
inline types::AccessFlags::Bits operator|(types::AccessFlags::Enum lhs, types::AccessFlags::Enum rhs)
{
	return (types::AccessFlags::Enum)(static_cast<uint32>(lhs) | static_cast<uint32>(rhs));
}

/*!*********************************************************************************************************************
\brief Bitwise OR with assignment shader stage flags.
\return The left hand side of the operation, which has been bitwise ORed with the right hand side
***********************************************************************************************************************/
inline types::ShaderStageFlags::Enum& operator|=(types::ShaderStageFlags::Enum& lhs, types::ShaderStageFlags::Enum rhs)
{
	return lhs = (types::ShaderStageFlags::Enum)(static_cast<uint32>(lhs) | static_cast<uint32>(rhs));
}
}// ShaderStageFlags

namespace ImageLayout {
enum Enum
{
	Undefined = 0,
	General = 1,
	ColorAttachmentOptimal = 2,
	DepthStencilAttachmentOptimal = 3,
	DepthStencilReadOnlyOptimal = 4,
	ShaderReadOnlyOptimal = 5,
	TransferSrcOptimal = 6,
	TransferDstOptimal = 7,
	Preinitialized = 8,
	PresentSrc = 1000001002,
};
};
}// namespace types

/*!*********************************************************************************************
\brief  Pre-defined Result codes (success and generic errors).
***********************************************************************************************/
namespace Result {
enum Enum
{
	Success,
	UnknownError,

	//Generic Errors
	OutOfMemory,
	InvalidArgument,
	AlreadyInitialized,
	NotInitialized,
	UnsupportedRequest,
	FileVersionMismatch,

	//Stream Errors
	NotReadable,
	NotWritable,
	EndOfStream,
	UnableToOpen,
	NoData,

	//Array Errors
	OutOfBounds,
	NotFound,

	//Map Errors
	KeyAlreadyExists,

	//Shell Error
	ExitRenderFrame, // Used to exit the renderscene loop in the shell

	//Resource Error
	InvalidData,
};
};
/*!*********************************************************************************************
\brief  Represents a buffer of Unsigned Bytes. Used to store raw data.
***********************************************************************************************/
typedef std::vector<byte> UCharBuffer;

/*!*********************************************************************************************
\brief  Represents a buffer of Signed Bytes. Used to store raw data.
***********************************************************************************************/
typedef std::vector<char8> CharBuffer;

/*!*********************************************************************************************
\brief  Representation of raw data. Used to store raw data that is logically grouped in blocks
        with a stride.
***********************************************************************************************/
class StridedBuffer : public UCharBuffer { public: uint16 stride; };

/*!*********************************************************************************************
\brief  Return a random Number between min and max
\return Random number
***********************************************************************************************/
inline float32 randomrange(float32 min, float32 max)
{
	float32 zero_to_one = float32(float64(rand()) / float64(RAND_MAX));
	float32 diff = max - min;
	return zero_to_one * diff + min;
}


#define BIT(shift)((1) << (shift))

#if defined(_MSC_VER)
#define PVR_ALIGNED __declspec(align(16))
#elif defined(__GNUC__) || defined (__clang__)
#define PVR_ALIGNED __attribute__((aligned(16)))
#else
#define PVR_ALIGNED alignas(16)
#endif
}
