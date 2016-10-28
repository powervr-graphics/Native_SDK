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
#include <type_traits>
#include "../External/glm/glm.hpp"
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


#define DEFINE_ENUM_OR_OPERATORS(type_) \
inline type_ operator | (type_ lhs, type_ rhs) \
{ \
    return (type_)(static_cast<std::underlying_type<type_>::type/**/>(lhs) | static_cast<std::underlying_type<type_>::type/**/>(rhs)); \
} \
inline type_ operator |= (type_ lhs, type_ rhs) \
{ \
    lhs = (type_)(static_cast<std::underlying_type<type_>::type/**/>(lhs) | static_cast<std::underlying_type<type_>::type/**/>(rhs)); \
return lhs; \
}

#define DEFINE_ENUM_AND_OPERATORS(type_) \
inline type_ operator & (type_ lhs, type_ rhs) \
{ \
    return (type_)(static_cast<std::underlying_type<type_>::type/**/>(lhs) & static_cast<std::underlying_type<type_>::type/**/>(rhs)); \
} \
inline type_ operator &= (type_ lhs, type_ rhs) \
{ \
    lhs = (type_)(static_cast<std::underlying_type<type_>::type/**/>(lhs) & static_cast<std::underlying_type<type_>::type/**/>(rhs)); \
return lhs; \
}


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
typedef wchar_t                wchar;

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

/*!*********************************************************************************************************************
\brief         Enumeration of all API types supported by this implementation
***********************************************************************************************************************/
enum class Api
{
	Unspecified = 0,
	//OpenGL,
	OpenGLES2,
	OpenGLES3,
	OpenGLES31,
	OpenGLESMaxVersion = OpenGLES31,
	Vulkan,
	Count
};

/*!*******************************************************************************************
\brief  Get the api code
\return Api code
*********************************************************************************************/
inline const char* apiCode(Api api)
{
	static const char* ApiCodes[] =
	{
		"",
		"ES2",
		"ES3",
		"ES31",
		"vk",
	};
	return ApiCodes[(int)api];
}

/*!*******************************************************************************************
\brief  Get the api family min version
\return Api family min
*********************************************************************************************/
inline Api apiFamilyMin(Api api)
{
	static const Api ApiCodes[] =
	{
		Api::Unspecified,
		Api::OpenGLES2,
		Api::OpenGLES2,
		Api::OpenGLES2,
		Api::Vulkan,
	};
	return ApiCodes[(int)api];
}

/*!*******************************************************************************************
\brief  Get the api family max version
\return Api family Max
*********************************************************************************************/
inline Api apiFamilyMax(Api api)
{
	static const Api ApiCodes[] =
	{
		Api::Unspecified,
		Api::OpenGLES31,
		Api::OpenGLES31,
		Api::OpenGLES31,
		Api::Vulkan,
	};
	return ApiCodes[(int)api];
}

/*!*******************************************************************************************
\brief  Get the api name string of the given Enumeration
\return Api name string
*********************************************************************************************/
inline const char* apiName(Api api)
{
	static const char* ApiCodes[] =
	{
		"Unknown",
		"OpenGL ES 2.0",
		"OpenGL ES 3.0",
		"OpenGL ES 3.1",
		"Vulkan",
	};
	return ApiCodes[(int)api];
}


namespace types {

/*!*******************************************************************************************
\brief  Enumeration containing all possible API object types (Images, Buffers etc.).
*********************************************************************************************/
enum class ApiObjectType
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

/*!*********************************************************************************************
\brief  An enumeration that defines data types used throughout the Framework.
        Commonly used in places where raw data are used to define the types actually contained.
***********************************************************************************************/
enum class DataType
{
	None,           //< none
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
inline uint32 dataTypeSize(DataType type)
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
	case DataType::DEC3N://(1D/2D/3D).
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
inline uint32  dataTypeComponentCount(DataType type)
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
inline bool  dataTypeIsNormalised(DataType type)
{
	return (type == DataType::Int8Norm || type == DataType::UInt8Norm ||
	        type == DataType::Int16Norm || type == DataType::UInt16Norm);
}

/*!***************************************************************************
\brief Enumeration of Colorspaces (Linear, SRGB).
*****************************************************************************/
enum class ColorSpace
{
	lRGB,
	sRGB,
	NumSpaces
};



namespace GpuDatatypes {
enum class Standard { std140 };

enum class BaseType  { Integer = 0, Float = 1 };
enum class  VectorWidth {  Scalar = 0, Vec2 = 1, Vec3 = 2, Vec4 = 3, };
enum class  MatrixColumns { OneCol = 0, Mat2x = 1, Mat3x = 2, Mat4x = 3 };

enum class Bits
{
	Integer = 0, Float = 1,
	BitScalar = 0, BitVec2 = 2, BitVec3 = 4, BitVec4 = 6,
	BitOneCol = 0, BitMat2x = 8, BitMat3x = 16, BitMat4x = 24,
	ShiftType = 0, MaskType = 1, NotMaskType = ~MaskType,
	ShiftVec = 1, MaskVec = (3 << ShiftVec), NotMaskVec = ~MaskVec,
	ShiftCols = 3, MaskCols = (3 << ShiftCols), NotMaskCols = ~MaskCols
};
DEFINE_ENUM_OR_OPERATORS(Bits)

enum Enum
{
	integer = (uint32)Bits::Integer | (uint32)Bits::BitScalar | (uint32)Bits::BitOneCol, uinteger = integer, boolean = integer,
	ivec2 = (uint32)Bits::Integer | (uint32)Bits::BitVec2 | (uint32)Bits::BitOneCol, uvec2 = ivec2, bvec2 = ivec2,
	ivec3 = (uint32)Bits::Integer | (uint32)Bits::BitVec3 | (uint32)Bits::BitOneCol, uvec3 = ivec3, bvec3 = ivec3,
	ivec4 = (uint32)Bits::Integer | (uint32)Bits::BitVec4 | (uint32)Bits::BitOneCol, uvec4 = ivec4, bvec4 = ivec4,
	float32 = (uint32)Bits::Float | (uint32)Bits::BitScalar | (uint32)Bits::BitOneCol,
	vec2 = (uint32)Bits::Float | (uint32)Bits::BitVec2 | (uint32)Bits::BitOneCol,
	vec3 = (uint32)Bits::Float | (uint32)Bits::BitVec3 | (uint32)Bits::BitOneCol,
	vec4 = (uint32)Bits::Float | (uint32)Bits::BitVec4 | (uint32)Bits::BitOneCol,
	mat2x2 = (uint32)Bits::Float | (uint32)Bits::BitVec2 | (uint32)Bits::BitMat2x,
	mat2x3 = (uint32)Bits::Float | (uint32)Bits::BitVec3 | (uint32)Bits::BitMat2x,
	mat2x4 = (uint32)Bits::Float | (uint32)Bits::BitVec4 | (uint32)Bits::BitMat2x,
	mat3x2 = (uint32)Bits::Float | (uint32)Bits::BitVec2 | (uint32)Bits::BitMat3x,
	mat3x3 = (uint32)Bits::Float | (uint32)Bits::BitVec3 | (uint32)Bits::BitMat3x,
	mat3x4 = (uint32)Bits::Float | (uint32)Bits::BitVec4 | (uint32)Bits::BitMat3x,
	mat4x2 = (uint32)Bits::Float | (uint32)Bits::BitVec2 | (uint32)Bits::BitMat4x,
	mat4x3 = (uint32)Bits::Float | (uint32)Bits::BitVec3 | (uint32)Bits::BitMat4x,
	mat4x4 = (uint32)Bits::Float | (uint32)Bits::BitVec4 | (uint32)Bits::BitMat4x,
	none = 0xFFFFFFFF
};

inline GpuDatatypes::Enum operator & (GpuDatatypes::Enum lhs, Bits rhs)
{
	return (GpuDatatypes::Enum)(uint32(lhs) & uint32(rhs));
}

inline GpuDatatypes::Enum operator >> (GpuDatatypes::Enum lhs, Bits rhs)
{
	return (GpuDatatypes::Enum)(uint32(lhs) >> uint32(rhs));
}

inline GpuDatatypes::Enum operator << (GpuDatatypes::Enum lhs, Bits rhs)
{
	return (GpuDatatypes::Enum)(uint32(lhs) << uint32(rhs));
}

inline uint32 getNumVecElements(GpuDatatypes::Enum type)
{
	return (uint32)VectorWidth((uint32)((type & Bits::MaskVec) >> Bits::ShiftVec) + 1);
}

inline uint32 getNumMatrixColumns(GpuDatatypes::Enum type)
{
	return (uint32)MatrixColumns((uint32)((type & Bits::MaskCols) >> Bits::ShiftCols) + 1);
}

inline uint32 getAlignment(GpuDatatypes::Enum type)
{
	uint32 vectype = type & Bits::MaskVec;
	return (vectype == (uint32)Bits::BitScalar ? 4 : vectype == (uint32)Bits::BitVec2 ? 8 : 16);
}

inline uint32 getVectorSelfAlignedSize(GpuDatatypes::Enum type)
{
	return getAlignment(type);
}

inline uint32 getVectorUnalignedSize(GpuDatatypes::Enum type)
{
	return 4 * getNumVecElements(type);
}

inline BaseType getBaseType(GpuDatatypes::Enum type)
{
	return BaseType(type & 1);
}

// Returns a datatype that has:
// 1) The most permissive base type (float>int)
// 2) The largest of the two vector widths
// 3) The most of the two matrix colums heighs
inline GpuDatatypes::Enum mergeDatatypesBigger(GpuDatatypes::Enum type1, GpuDatatypes::Enum type2)
{
	uint32 baseTypeBits = (std::max)(type1 & Bits::MaskType, type2 & Bits::MaskType);
	uint32 vectorWidthBits = (std::max)(type1 & Bits::MaskVec, type2 & Bits::MaskVec);
	uint32 matrixColBits = (std::max)(type1 & Bits::MaskCols, type2 & Bits::MaskCols);
	return GpuDatatypes::Enum(baseTypeBits | vectorWidthBits | matrixColBits);
}

// Returns a datatype that has:
// 1) The most permissive base type (float>int)
// 2) The smaller of the two vector widths
// 3) The less of the two matrix colums heighs
inline GpuDatatypes::Enum mergeDatatypesSmaller(GpuDatatypes::Enum type1, GpuDatatypes::Enum type2)
{
	uint32 baseTypeBits = (std::max)(type1 & Bits::MaskType, type2 & Bits::MaskType);
	uint32 vectorWidthBits = (std::min)(type1 & Bits::MaskVec, type2 & Bits::MaskVec);
	uint32 matrixColBits = (std::min)(type1 & Bits::MaskCols, type2 & Bits::MaskCols);
	return GpuDatatypes::Enum(baseTypeBits | vectorWidthBits | matrixColBits);
}

inline uint32 getSelfAlignedSize(GpuDatatypes::Enum type)
{
	uint32 isMatrix = (getNumMatrixColumns(type) > 1);

	return (std::max)(getVectorSelfAlignedSize(type), 16u * isMatrix) * getNumMatrixColumns(type);
}

inline uint32 getSelfAlignedArraySize(GpuDatatypes::Enum type)
{
	return (std::max)(getVectorSelfAlignedSize(type), 16u) * getNumMatrixColumns(type);
}

inline uint32 getSize(GpuDatatypes::Enum type, uint32 arrayElements = 1)
{
	uint32 numElements = getNumMatrixColumns(type) * arrayElements;

	PVR_ASSERTION(numElements > 0);
	uint32 selfAlign = (std::max)(getVectorSelfAlignedSize(type), 16u) * numElements * (numElements > 1); //Equivalent to: if (arrayElements!=0) selfAlign = getSelfAl... else selfAlign=0
	uint32 unaligned = getVectorUnalignedSize(type) * (numElements == 1);//Equivalent to: if (arrayElements==0) unaligned = getUnaligned....

	return selfAlign + unaligned;
}

inline uint32 getCpuPackedSize(GpuDatatypes::Enum type, uint32 arrayElements = 1)
{
	return getVectorUnalignedSize(type) * getNumMatrixColumns(type) * arrayElements;
}

/**********************************************************************************************
\brief    Aligns an offset (previousTotalSize) with the alignment of a type. It's called
getOffsetAfter, because it is commonly used to find the offset of an item when the previous
total size is known (i.e. align a vec4 to a struct whose previous size was 30 bytes? returns
32 bytes...)
**********************************************************************************************/
inline uint32 getOffsetAfter(GpuDatatypes::Enum type, uint32 previousTotalSize)
{
	uint32 align = getAlignment(type);

	uint32 diff = previousTotalSize % align;
	diff += (align * (diff == 0)); // REMOVE BRANCHING -- equal to : if(diff==0) { diff+=8 }
	return previousTotalSize - diff + align;
}

/**********************************************************************************************
\brief    Returns the new size of a hypothetical struct whose old size was previousTotalSize,
and to which "arrayElement" new items of type "type" are added
**********************************************************************************************/
inline uint32 getTotalSizeAfter(GpuDatatypes::Enum type, uint32 arrayElements, uint32 previousTotalSize)
{
	PVR_ASSERTION(arrayElements > 0);
	// Arrays pads their last element to their alignments. Standalone objects do not. vec3[3] is NOT the same as vec3;vec3;vec3;
	uint32 selfAlignedSize = getSelfAlignedArraySize(type) * arrayElements * (arrayElements != 1); //Equivalent to: if (arrayElements!=1) selfAlign = getSelfAl... else selfAlign=0
	// Other elements do not.
	uint32 unalignedSize = getSize(type) * (arrayElements == 1);//Equivalent to: if (arrayElements==1) unaligned = getUnaligned.... else unaligned=0

	return getOffsetAfter(type, previousTotalSize) + selfAlignedSize + unalignedSize;
}

inline DataType toDataType(GpuDatatypes::Enum type)
{
	return getBaseType(type) == BaseType::Float ? DataType::Float32 : DataType::Int32;
}


template<typename T> struct Metadata; // Template specializations in FreeValue.h

}

/*!*********************************************************************************************
\brief  Enumeration containing all possible Primitive topologies (Point, line trianglelist etc.).
***********************************************************************************************/
enum class PrimitiveTopology : uint32
{
	//POSITION-SENSITIVE. Do not renumber unless also refactoring ConvertToVkTypes, ConvertToGlesTypes.
	PointList,
	LineList,
	LineStrip,
	LineLoop, // line loop. Supported only for ES
	TriangleList, //< triangle list
	TriangleStrip,//< triangle strip
	TriangleFan,//< triangle fan
	LineListWithAdjacency,
	LineStripWithAdjacency,
	TriangleListWithAdjacency,
	TriangleStripWithAdjacency,
	TriPatchList,//< triangle patch list
	QuadPatchList,//< quad patch list};
	IsoLineList,//< isoline list};
	None
};


/*!*********************************************************************************************
\brief  Enumeration all possible values of operations to be performed on initially Loading a
        Framebuffer Object.
***********************************************************************************************/
enum class LoadOp : uint32
{
	Load,  //< Load the contents from the fbo from previous
	Clear, //< Clear the fbo
	Ignore, //< Ignore writing to the fbo and keep old data
};


/*!*********************************************************************************************
\brief  Enumerates all possible values of operations to be performed when Storing to a
        Framebuffer Object.
***********************************************************************************************/
enum class StoreOp : uint32
{
	Store,//< write the source to the destination
	Ignore,//< don't write the source to the destination
};



/*!*******************************************************************************************
\brief Enumeration of the "aspect" (or "semantics") of an image: Color, Depth, Stencil.
**********************************************************************************************/
enum class ImageAspect : uint32
{

	Color = 0x1,
	Depth = 0x2,
	Stencil = 0x4,
	Metadata = 0x8,
	DepthAndStencil = Depth | Stencil,
};
DEFINE_ENUM_OR_OPERATORS(ImageAspect)

/*!*********************************************************************************************************************
\brief Enumeration of the possible types of a Pipeline Binding point (Graphics, Compute).
***********************************************************************************************************************/
enum class PipelineBindPoint : uint32 { Graphics, Compute };

/*!*********************************************************************************************************************
\brief Enumeration of the possible way of recording commands for each subpasses of the render pass (Inline, SecondaryCommandBuffer).
***********************************************************************************************************************/
enum class RenderPassContents : uint32
{
	Inline,//< commands are recorded within the command buffer for the subpass
	SecondaryCommandBuffers//< commands are recorded in the secondary commandbuffer for the subpass
};


/*!*********************************************************************************************************************
\brief Enumeration of the possible binding points of a Framebuffer Object (Read, Write, ReadWrite).
***********************************************************************************************************************/
enum class FboBindingTarget : uint32
{
	Read = 1, //< bind Fbo for read
	Write = 2, //< bind fbo for write
	ReadWrite = 3 //< bind fbo for read and write
};


/*!*********************************************************************************************************************
\brief Logic operations (toggle, clear, and etc.).
***********************************************************************************************************************/
enum class LogicOp : uint32
{
	//DO NOT REARRANGE - Direct mapping with VkLogicOp. See ConvertToVk::logicOp
	Clear,
	And,
	AndReverse,
	Copy,
	AndInverted,
	NoOp5,
	Xor,
	Or,
	Nor,
	Equiv,
	Invert0,
	OrReverse,
	CopyInverted,
	OrInverted,
	Nand,
	Set,
	Count
};

/*!********************************************************************************************
\brief ChannelWriteMask enable/ disable writting to channel bits.
***********************************************************************************************/
enum class ColorChannel : uint32
{
	//DO NOT REARRANGE - Direct mapping to Vulkan
	R = 0x01, //< write to red channel
	G = 0x02, //< write to green channel
	B = 0x04, //< write to blue channel
	A = 0x08, //< write to alpha channel
	None = 0, //< don't write to any channel
	All = R | G | B | A //< write to all channel
};
DEFINE_ENUM_OR_OPERATORS(ColorChannel)
DEFINE_ENUM_AND_OPERATORS(ColorChannel)

/*!********************************************************************************************
\brief Step rate for a vertex attribute when drawing: Per vertex, per instance, per draw.
**********************************************************************************************/
enum class StepRate : uint32
{
	Vertex, //< Step rate Per vertex
	Instance,//< Step rate per instance
	Default = Vertex
};


/*!*********************************************************************************************
\brief Enumeration of Provoking Vertex modes.
***********************************************************************************************/
enum class ProvokingVertex : uint32
{
	First,
	Last,
	Default = First
};


/*!*********************************************************************************************************************
\brief Enumeration of all FrameBufferObject texture targets.
***********************************************************************************************************************/
enum class FboTextureTarget : uint32
{

	TextureTarget2d,
	TextureTargetCubeMapPositiveX,
	TextureTargetCubeMapNegativeX,
	TextureTargetCubeMapPositiveY,
	TextureTargetCubeMapNegativeY,
	TextureTargetCubeMapPositiveZ,
	TextureTargetCubeMapNegativeZ,
	Unknown
};


/*!*********************************************************************************************************************
\brief Enumeration of polygon filling modes.
***********************************************************************************************************************/
enum class FillMode : uint32
{
	Fill,///< enum value. fill polygon front, solid
	WireFrame,///< fill front, wireframe
	Points,///< fill back wireframe
	NumFillMode,
	Default = Fill
};


/*!*********************************************************************************************************************
\brief Enumeration of Face facing (front, back...).
***********************************************************************************************************************/
enum class Face : uint32
{
	//DO NOT REARRANGE - DIRECT TO VULKAN
	None = 0,
	Front = 1,
	Back = 2,
	FrontBack = 3,
	DefaultCullFace = None
};


/*!*********************************************************************************************************************
\brief Enumeration of the six faces of a Cube
***********************************************************************************************************************/
enum class CubeFace : uint32
{
	PositiveX = 0,
	NegativeX,
	PositiveY,
	NegativeY,
	PositiveZ,
	NegativeZ
};

/*!*********************************************************************************************************************
\brief Enumeration of Face facing (front, back...).
***********************************************************************************************************************/
enum class StencilFace : uint32
{
	//DO NOT REARRANGE - DIRECT TO VULKAN
	Front = 1,
	Back = 2,
	FrontBack = 3,
};


/*!*********************************************************************************************************************
\brief Enumeration of the blend operations (determine how a new pixel (source color) is combined with a pixel already in the
       framebuffer (destination color).
***********************************************************************************************************************/
enum class BlendOp : uint32
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


/*!*********************************************************************************************************************
\brief Buffer mapping flags.
***********************************************************************************************************************/
enum class MapBufferFlags : uint32
{
	Read = 1, Write = 2, Unsynchronised = 4
};

DEFINE_ENUM_OR_OPERATORS(MapBufferFlags)
DEFINE_ENUM_AND_OPERATORS(MapBufferFlags)

/*!*********************************************************************************************************************
\brief Specfies how the rgba blending facors are computed for source and destination fragments.
***********************************************************************************************************************/
enum class  BlendFactor : uint8
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
	DefaultSrcRgba = One,
	DefaultDestRgba = Zero
};

enum class DynamicState : uint32
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


/*!*********************************************************************************************************************
\brief Enumeration of Interpolation types for Samplers (nearest, linear).
***********************************************************************************************************************/
enum class InterpolationMode : uint8
{
	Nearest, Linear
};

/*!********************************************************************************************
\brief Enumeration of the different front face to winding order correlations.
***********************************************************************************************/
enum class PolygonWindingOrder : uint8
{
//DO NOT REARRANGE - VULKAN DIRECT MAPPING
	FrontFaceCCW, FrontFaceCW, Default = FrontFaceCCW
};

/*!********************************************************************************************
\brief Enumeration of the different stencil operations.
***********************************************************************************************/
enum class StencilOp : uint8
{
	//DO NOT REARRANGE - VULKAN DIRECT MAPPING
	Keep,
	Zero,
	Replace,
	IncrementClamp,
	DecrementClamp,
	Invert,
	IncrementWrap,
	DecrementWrap,
	NumStencilOp,

	// Defaults

	DefaultStencilFailFront         = Keep,
	DefaultStencilFailBack          = Keep,

	DefaultDepthFailFront           = Keep,
	DefaultDepthFailBack            = Keep,

	DefaultDepthStencilPassFront    = Keep,
	DefaultDepthStencilPassBack     = Keep
};

/*!********************************************************************************************
\brief Enumeration of all the different descriptor types.
***********************************************************************************************/
enum class DescriptorType : uint8
{
	// DO NOT RE-ARRANGE THIS
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
	Count,
	numBits = 4
};

/*!*********************************************************************************************
\brief  Pre-defined Capability presense values.
***********************************************************************************************/
enum class Capability : uint8
{
	Unsupported,
	Immutable,
	Mutable
};

/*!*********************************************************************************************
\brief  An enumeration that defines a type that can use as an index, typically 16 or 32 bit int.
        Especially used in Model classes.
***********************************************************************************************/
enum class IndexType : uint32
{
	IndexType16Bit = (uint32)DataType::UInt16,//< 16 bit face data
	IndexType32Bit = (uint32)DataType::UInt32//< 32 bit face data
};

/*!*********************************************************************************************************************
\brief Return the Size of an IndexType in bytes.
\param[in] type The Index type
\return uint32
***********************************************************************************************************************/
inline uint32 indexTypeSizeInBytes(const IndexType type)
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

/*!*********************************************************************************************
\brief  An enumeration that defines Comparison operations (equal, less or equal etc.).
        Especially used in API classes for functions like depth testing.
***********************************************************************************************/
enum class ComparisonMode : uint32
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
	DefaultDepthFunc = Less,
	DefaultStencilOpFront = Always,
	DefaultStencilOpBack = Always,
};

/*!*********************************************************************************************************************
\brief        Enumeration describing a filtering type of a specific dimension. In order to describe the filtering mode
              properly, you would have to define a Minification filter, a Magnification filter and a Mipmapping
              minification filter. Possible values: Nearest, Linear, None.
***********************************************************************************************************************/
enum class SamplerFilter : uint8
{
	Nearest,//< nearest filter
	Linear,//< linear filter
	None,//< no filter
	Cubic,//< cubic filter
	Default = Linear,
	MipDefault = Linear,
	Size = 4
};

enum PackedSamplerFilter : int8
{
	PackNone,//< no filter
	PackNearestMipNone = (uint8)((uint8)SamplerFilter::Nearest | ((uint8)SamplerFilter::Nearest << 2) | ((uint8)SamplerFilter::None << 4)), //<
	PackNearestMipNearest = (uint8)((uint8)SamplerFilter::Nearest | ((uint8)SamplerFilter::Nearest << 2) | ((uint8)SamplerFilter::Nearest << 4)), //<
	PackNearestMipLinear = (uint8)((uint8)SamplerFilter::Nearest | ((uint8)SamplerFilter::Nearest << 2) | ((uint8)SamplerFilter::Linear << 4)), //<
	PackLinearMipNone = (uint8)((uint8)SamplerFilter::Linear | ((uint8)SamplerFilter::Linear << 2) | ((uint8)SamplerFilter::None << 4)), //<
	PackLinearMipNearest = (uint8)((uint8)SamplerFilter::Linear | ((uint8)SamplerFilter::Linear << 2) | ((uint8)SamplerFilter::Nearest << 4)), //<
	PackTrilinear = (uint8)((uint8)SamplerFilter::Linear | ((uint8)SamplerFilter::Linear << 2) | ((uint8)SamplerFilter::Linear << 4)), //<
	Size, //< number of supported filter
	PackDefault = PackTrilinear//< default filter
};

inline PackedSamplerFilter packSamplerFilter(SamplerFilter mini, SamplerFilter magni, SamplerFilter mip)
{
	return PackedSamplerFilter((PackedSamplerFilter)mini + ((PackedSamplerFilter)magni << 2) + ((PackedSamplerFilter)mip << 4));
}

inline void unpackSamplerFilter(PackedSamplerFilter packed, SamplerFilter& mini, SamplerFilter& magni, SamplerFilter& mip)
{
	mini = (SamplerFilter)(packed & 3);
	magni = (SamplerFilter)((packed >> 2) & 3);
	mip = (SamplerFilter)(packed >> 4);
}

/*!*********************************************************************************************************************
\brief        Enumeration describing default border colors for textures.
***********************************************************************************************************************/
enum class BorderColor : uint8
{
	TransparentBlack, //< Black Border with alpha 0 : (0,0,0,0)
	OpaqueBlack, //< Black border with alpha 1 : (0,0,0,1)
	OpaqueWhite, //< white border with alpha 1: (1,1,1,1)
	Count
};


/*!*********************************************************************************************************************
\brief   Enumeration for defining texture wrapping mode: Repeat, Mirror, Clamp, Border.
***********************************************************************************************************************/
enum class SamplerWrap  : uint8
{
	Repeat,//< repeat
	MirrorRepeat,//< mirror repeat
	Clamp,//< clamp
	Border,//< border
	MirrorClamp,//< mirror clamp
	Size,//< number of support sampler wrap
	Default = Repeat //< default wrap
};


enum class ImageBaseType { Image1D, Image2D, Image3D, Unallocated, Unknown, Count = Image3D + 1 };


/*!*********************************************************************************************************************
\brief Enumeration of Texture dimensionalities.
***********************************************************************************************************************/
enum class ImageViewType
{
	Unallocated,
	ImageView1D,            //!< 1 dimensional Image View
	ImageView2D,            //!< 2 dimensional Image View
	ImageView3D,            //!< 3 dimensional Image View
	ImageView2DCube,        //!< cube texture
	ImageView1DArray,           //!< 1 dimensional Image View
	ImageView2DArray,       //!< 2 dimensional Image View
	ImageView3DArray,       //!< 3 dimensional Image View
	ImageView2DCubeArray,   //!< 2 dimensional Image View
	ImageViewUnknown,       //!< 3 dimensional Image View
};

inline ImageBaseType imageViewTypeToImageBaseType(ImageViewType viewtype)
{
	switch (viewtype)
	{
	case ImageViewType::ImageView1D:
	case ImageViewType::ImageView1DArray:
		return ImageBaseType::Image1D;

	case ImageViewType::ImageView2D:
	case ImageViewType::ImageView2DCube:
	case ImageViewType::ImageView2DArray:
	case ImageViewType::ImageView2DCubeArray:
		return ImageBaseType::Image2D;

	case ImageViewType::ImageView3D:
	case ImageViewType::ImageView3DArray:
		return ImageBaseType::Image3D;

	default: return ImageBaseType::Unallocated;
	}
}

/*!********************************************************************************************************************
\brief        Enumeration of the binary shader formats.
***********************************************************************************************************************/
enum class ShaderBinaryFormat
{
	ImgSgx,
	Spv,
	Unknown,
	None
};


/*!********************************************************************************************************************
\brief        Enumeration of all supported shader types.
***********************************************************************************************************************/
enum class ShaderType
{
	UnknownShader = 0,//!< unknown shader type
	VertexShader,//!< vertex shader
	FragmentShader,//!< fragment shader
	ComputeShader,//!< compute shader
	TessControlShader,
	TessEvaluationShader,
	FrameShader,//!< frame shader
	RayShader,//!< ray shader
	GeometryShader,
	Count
};

/*!********************************************************************************************************************
\brief        Enumeration of Descriptor Set use types (once, dynamic).
***********************************************************************************************************************/
enum class DescriptorSetUsage
{
	OneShot,
	Static
};


/*!*********************************************************************************************************************
\brief Enumeration of all shader stages.
***********************************************************************************************************************/

enum class ShaderStageFlags : uint32
{
	Vertex = 0x00000001, //< Vertex Shader stage
	TesselationControl = 0x00000002,
	TesselationEvaluation = 0x00000004,
	Geometry = 0x00000008,
	Fragment = 0x00000010,//< Fragment Shader stage
	Compute = 0x00000020,//< Compute Shader stage
	AllGraphicsStages = 0x0000001F,//< Vertex + Fragment shader stage
	AllStages = 0x7FFFFFFF
};
DEFINE_ENUM_OR_OPERATORS(ShaderStageFlags)
DEFINE_ENUM_AND_OPERATORS(ShaderStageFlags)

/*!*********************************************************************************************************************
\brief Enumeration of all Pipeline stages.
***********************************************************************************************************************/

enum class PipelineStageFlags : uint32
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
	BottomOfPipeline = 0x00002000,
	Host = 0x00004000,
	AllGraphics = 0x00008000,
	AllCommands = 0x00010000,
};
DEFINE_ENUM_OR_OPERATORS(PipelineStageFlags)
DEFINE_ENUM_AND_OPERATORS(PipelineStageFlags)

enum class AccessFlags : uint32
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
DEFINE_ENUM_OR_OPERATORS(AccessFlags)
DEFINE_ENUM_AND_OPERATORS(AccessFlags)


enum class SampleCount : uint32
{
	Count1  = 0x00000001,
	Count2  = 0x00000002,
	Count4  = 0x00000004,
	Count8  = 0x00000008,
	Count16 = 0x00000010,
	Count32 = 0x00000020,
	Count64 = 0x00000040,
	Default = Count1
};
DEFINE_ENUM_OR_OPERATORS(SampleCount)
DEFINE_ENUM_AND_OPERATORS(SampleCount)

enum class ImageUsageFlags : uint32
{
	// DO NOT REORDER THIS.
	TransferSrc = 0x00000001,
	TransferDest = 0x00000002,
	Sampled = 0x00000004,
	Storage = 0x00000008,
	ColorAttachment = 0x00000010,
	DepthStencilAttachment = 0x00000020,
	TransientAttachment = 0x00000040,
	InputAttachment = 0x00000080,
};
DEFINE_ENUM_OR_OPERATORS(ImageUsageFlags)
DEFINE_ENUM_AND_OPERATORS(ImageUsageFlags)

enum class ImageLayout  : uint32
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


enum class BufferViewTypes : uint32
{
	UniformBuffer = 0x1,
	StorageBuffer = 0x2,
	UniformBufferDynamic = 0x4,
	StorageBufferDynamic = 0x8,
};
DEFINE_ENUM_OR_OPERATORS(BufferViewTypes)
DEFINE_ENUM_AND_OPERATORS(BufferViewTypes)

inline BufferViewTypes descriptorTypeToBufferViewType(DescriptorType descType)
{
	return BufferViewTypes(1 << (static_cast<std::underlying_type<DescriptorType>::type/**/>(descType) - 6));
}


/*!********************************************************************************************************************
\brief        Enumeration of all supported buffer use types.
***********************************************************************************************************************/
enum class BufferBindingUse  : uint32
{
	TransferSrc = 0x00000001,
	TransferDest = 0x00000002,
	UniformTexelBuffer = 0x00000004,
	StorageTexelBuffer = 0x00000008,
	UniformBuffer = 0x00000010,
	StorageBuffer = 0x00000020,
	IndexBuffer = 0x00000040,
	VertexBuffer = 0x00000080,
	IndirectBuffer = 0x00000100,
	Count = 10
};

//inline BufferBindingUse operator | (BufferBindingUse lhs, BufferBindingUse rhs)
//{
//    return (BufferBindingUse)(static_cast<std::underlying_type<BufferBindingUse>::type/**/>(lhs) | static_cast<std::underlying_type<BufferBindingUse>::type/**/>(rhs));
//}
//inline BufferBindingUse operator |= (BufferBindingUse lhs, BufferBindingUse rhs)
//{
//    lhs = (BufferBindingUse)(static_cast<std::underlying_type<BufferBindingUse>::type/**/>(lhs) | static_cast<std::underlying_type<BufferBindingUse>::type/**/>(rhs));
//return lhs;
//}
DEFINE_ENUM_OR_OPERATORS(BufferBindingUse)
DEFINE_ENUM_AND_OPERATORS(BufferBindingUse)

inline BufferBindingUse bufferViewTypeToBufferBindingUse(BufferViewTypes viewType)
{
	uint32 retval =
	  (uint32)BufferBindingUse::UniformBuffer * (((uint32)viewType & (uint32)(BufferViewTypes::UniformBuffer | BufferViewTypes::UniformBufferDynamic)) != 0) |
	  (uint32)BufferBindingUse::StorageBuffer * (((uint32)viewType & (uint32)(BufferViewTypes::StorageBuffer | BufferViewTypes::StorageBufferDynamic)) != 0);
	return BufferBindingUse(retval);
}

namespace PipelineDefaults {
/*!*********************************************************************************************************************
\brief Enumeration used for enabling/disabling depth and stencil states
***********************************************************************************************************************/
namespace DepthStencilStates {
static const bool DepthTestEnabled = false;
static const bool DepthWriteEnabled = true;
static const bool StencilTestEnabled = false;
static const bool DepthBoundTestEnabled = false;
static const bool UseDepthStencil = true;
static const uint32 ComparisonMask = 0xff;
static const uint32 StencilReadMask = 0xff;
static const uint32 StencilWriteMask = 0xff;
static const uint32 StencilReference = 0;
static const int32 StencilClearValue = 0;
static const float32 DepthClearValue = 1.f;
static const float32 DepthMin = 0.f;
static const float32 DepthMax = 1.f;
}

namespace Rasterizer {
static const bool RasterizerDiscardEnabled = false;
static const bool ProgramPointSizeEnabled = false;
static const bool DepthClipEnabled = true;
static const bool DepthBiasEnabled = false;
static const bool DepthBiasClampEnabled = false;
static const Face CullFace = pvr::types::Face::DefaultCullFace;
static const PolygonWindingOrder WindingOrder = pvr::types::PolygonWindingOrder::Default;
static const types::FillMode FillMode = pvr::types::FillMode::Default;
static const types::ProvokingVertex ProvokingVertex = pvr::types::ProvokingVertex::Default;
static const float32 LineWidth = 1.0f;
}

/*!*********************************************************************************************************************
\brief Enumeration used for setting tesselation defaults
***********************************************************************************************************************/
namespace Tesselation {
static const pvr::uint32 NumControlPoints = 3;
}

/*!*********************************************************************************************************************
\brief Stores defaults values used for initialising vertex attributes
***********************************************************************************************************************/
namespace VertexAttributeInfo {
static const pvr::uint16 Index = 0;
static const pvr::types::DataType Format = pvr::types::DataType::None;
static const pvr::uint8 Width = 0;
static const pvr::uint32 OffsetInBytes = 0;
static const pvr::string AttribName = "";
}

/*!*********************************************************************************************************************
\brief Stores defaults values used for initialising vertex input bindings
***********************************************************************************************************************/
namespace VertexInput {
static const pvr::uint16 StrideInBytes = 0;
static const pvr::string AttribName = "";
}

/*!*********************************************************************************************************************
\brief Stores defaults values used for initialising vertex attributes
***********************************************************************************************************************/
namespace ViewportScissor {
static const pvr::int32 OffsetX = 0;
static const pvr::int32 OffsetY = 0;
static const pvr::int32 Width = 0;
static const pvr::int32 Height = 0;

static const pvr::float32 MinDepth = 0.0f;
static const pvr::float32 MaxDepth = 1.0f;
static const bool ScissorTestEnabled = false;
}

/*!*********************************************************************************************************************
\brief Stores defaults values used for initialising input assembler state
***********************************************************************************************************************/
namespace InputAssembler {
static const pvr::types::PrimitiveTopology Topology = pvr::types::PrimitiveTopology::TriangleList;
static const bool DisableVertexReuse = true;
static const bool PrimitiveRestartEnabled = false;
static const pvr::uint32 PrimitiveRestartIndex = 0xFFFFFFFF;
}

/*!*********************************************************************************************************************
\brief Stores defaults values used for initialising color blend states
***********************************************************************************************************************/
namespace ColorBlend {
static const bool AlphaCoverageEnable = false;
static const bool LogicOpEnable = false;
static const pvr::types::LogicOp LogicOp = pvr::types::LogicOp::Set;
static const glm::vec4 BlendConstantRGBA(0.f);
static const bool BlendEnabled = false;
}

/*!*********************************************************************************************************************
\brief Stores defaults values used for initialising color channel writes
***********************************************************************************************************************/
namespace ColorWrite {
// enable writing to all channels
static const bool ColorMaskR = true;
static const bool ColorMaskG = true;
static const bool ColorMaskB = true;
static const bool ColorMaskA = true;
}

/*!*********************************************************************************************************************
\brief Stores defaults values used for initialising multi sample states
***********************************************************************************************************************/
namespace MultiSample {
static const bool Enabled = false;
static const bool SampleShading = false;
static const bool AlphaToCoverageEnable = false;
static const bool AlphaToOnEnable = false;
static const pvr::types::SampleCount RasterizationSamples = pvr::types::SampleCount::Default;
static const pvr::float32 MinSampleShading = 0.0f;
static const pvr::uint32 SampleMask = 0xffffffff;
}

/*!*********************************************************************************************************************
\brief Stores defaults values used for initialising shader stage states
***********************************************************************************************************************/
namespace ShaderStage {
static const pvr::string EntryPoint = "main";
}
}// namespace pipelineDefaults

/*!*********************************************************************************************************************
\brief Stores defaults values used for initialising descriptor bindings
***********************************************************************************************************************/
namespace DescriptorBindingDefaults {
static const pvr::int8 BindingId = -1;
static const pvr::int8 ArraySize = -1;
static const pvr::types::DescriptorType Type = pvr::types::DescriptorType::Count;
static const pvr::types::ShaderStageFlags ShaderStage = types::ShaderStageFlags::AllStages;
static const pvr::int8 MaxImages = 32;
static const pvr::int8 MaxUniformBuffers = 24;
static const pvr::int8 MaxStorageBuffers = 24;
}

/*!*********************************************************************************************************************
\brief Provides abstract type used for storing descriptors
***********************************************************************************************************************/
enum class DescriptorBindingType
{
	Image,
	UniformBuffer,
	StorageBuffer
};

/*!*********************************************************************************************************************
\brief Converts between pvr::types::DescriptorType and abstract pvr::types::DescriptorBindingType
***********************************************************************************************************************/
inline pvr::types::DescriptorBindingType getDescriptorTypeBinding(pvr::types::DescriptorType descType)
{
	switch (descType)
	{
	case pvr::types::DescriptorType::CombinedImageSampler:
	case pvr::types::DescriptorType::InputAttachment:
	case pvr::types::DescriptorType::SampledImage:
	case pvr::types::DescriptorType::StorageImage:
	case pvr::types::DescriptorType::Sampler:
		return pvr::types::DescriptorBindingType::Image;
		break;
	case pvr::types::DescriptorType::StorageBuffer:
	case pvr::types::DescriptorType::StorageBufferDynamic:
	case pvr::types::DescriptorType::StorageTexelBuffer:
		return pvr::types::DescriptorBindingType::StorageBuffer;
		break;
	case pvr::types::DescriptorType::UniformBuffer:
	case pvr::types::DescriptorType::UniformBufferDynamic:
	case pvr::types::DescriptorType::UniformTexelBuffer:
		return pvr::types::DescriptorBindingType::UniformBuffer;
		break;
    default:
        assert(false);
	}

	return pvr::types::DescriptorBindingType(-1);
}

}// namespace types

/*!*********************************************************************************************
\brief  Pre-defined Result codes (success and generic errors).
***********************************************************************************************/
enum class Result
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
#define BITS_TO_BYTE(bit)((bit) / (sizeof(pvr::byte)))
#if defined(_MSC_VER)
#define PVR_ALIGNED __declspec(align(16))
#elif defined(__GNUC__) || defined (__clang__)
#define PVR_ALIGNED __attribute__((aligned(16)))
#else
#define PVR_ALIGNED alignas(16)
#endif
}

// ARRAY_SIZE(a) is a compile-time constant which represents the number of elements of the given
// array. ONLY use ARRAY_SIZE for statically allocated arrays.
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
