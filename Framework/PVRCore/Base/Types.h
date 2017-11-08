/*!
\brief Basic file used in the PowerVR Framework. Defines several types used throughout the Framework (sized
arithmetic types, enumerations, character types).
\file PVRCore/Base/Types.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#pragma warning(disable:4512)
#pragma warning(disable:4480)
#pragma warning(disable:4100)

//!\cond NO_DOXYGEN
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef GLM_FORCE_SSE2
#define GLM_FORCE_SSE2
#endif
#endif
#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
//!\endcond

#if defined (X11)
// undef these macros from the xlib files, they are breaking the framework types.
#undef Success
#undef Enum
#undef None
#undef Always
#undef byte
#undef char8
#undef ShaderStageFlags
#undef capability
#endif

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
#include <stdint.h>
#include <limits>
#include <assert.h>
#include <type_traits>
#include "../External/glm/glm.hpp"
#include "../External/glm/gtc/type_ptr.hpp"
#include "../External/glm/gtc/matrix_inverse.hpp"
#include "../External/glm/gtc/matrix_access.hpp"
#include "../External/glm/gtx/quaternion.hpp"
#include "../External/glm/gtx/transform.hpp"
#include "../External/glm/gtx/transform.hpp"
#include "../External/glm/gtx/simd_vec4.hpp"
#include "../External/glm/gtx/simd_mat4.hpp"
#include "../External/glm/gtx/fast_trigonometry.hpp"

/*! \brief Macro that defines all common bitwise operators for an enum-class */
#define DEFINE_ENUM_OPERATORS(type_) \
inline type_ operator | (type_ lhs, type_ rhs) \
{ \
    return static_cast<type_>(static_cast<std::underlying_type<type_>::type/**/>(lhs) | static_cast<std::underlying_type<type_>::type/**/>(rhs)); \
} \
inline void operator |= (type_& lhs, type_ rhs) \
{ \
    lhs = static_cast<type_>(static_cast<std::underlying_type<type_>::type/**/>(lhs) | static_cast<std::underlying_type<type_>::type/**/>(rhs)); \
} \
inline type_ operator & (type_ lhs, type_ rhs) \
{ \
    return static_cast<type_>(static_cast<std::underlying_type<type_>::type/**/>(lhs) & static_cast<std::underlying_type<type_>::type/**/>(rhs)); \
} \
inline void operator &= (type_& lhs, type_ rhs) \
{ \
    lhs = static_cast<type_>(static_cast<std::underlying_type<type_>::type/**/>(lhs) & static_cast<std::underlying_type<type_>::type/**/>(rhs)); \
}

namespace pvr {
//UTF types
/// <summary>A UTF-8 (unsigned) character. 8-bit unsigned Integer.</summary>
typedef unsigned char          utf8;
/// <summary>A UTF-16 (unsigned) character. 16-bit unsigned Integer.</summary>
typedef unsigned short         utf16;
/// <summary>A UTF-32 (unsigned) character. 32-bit unsigned Integer.</summary>
typedef unsigned int           utf32;

/// <summary>Enumeration of all API types supported by this implementation</summary>
enum class Api
{
	Unspecified = 0,
	//OpenGL,
	OpenGLES2,
	OpenGLES3,
	OpenGLES31,
	OpenGLESMaxVersion = OpenGLES31,
	Vulkan,
	NumApis
};

/// <summary>Get a string of the specific api enum</summary>
/// <param name="api">The api</param>
/// <returns>Api code</returns>
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
	return ApiCodes[static_cast<int>(api)];
}

/// <summary>Get minimum api that is the same family as the parameter. (e.g. Vulkan returns
/// Vulkan, while OpenGLES31 returns OpenGLES2) api family min version</summary>
/// <param name="api">The api</param>
/// <returns>Api family min</returns>
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
	return ApiCodes[static_cast<int>(api)];
}

/// <summary>Get the highest api version that is of the same family as the parameter</summary>
/// <param name="api">The api</param>
/// <returns>Api family Max</returns>
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
	return ApiCodes[static_cast<int>(api)];
}

/// <summary>Get the api name std::string of the given Enumeration</summary>
/// <param name="api">The api</param>
/// <returns>Api name std::string</returns>
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
	return ApiCodes[static_cast<int>(api)];
}


/// <summary>Enumeration of all the different descriptor types.</summary>
enum class DescriptorType : uint32_t
{
	// DO NOT RE-ARRANGE THIS
	Sampler, //!< A Sampler object
	CombinedImageSampler, //!< A descriptor that contains both and image and its sampler
	SampledImage, //!< Aka "Texture"
	StorageImage, //!< Aka "Image for Image Load Store"
	UniformTexelBuffer, //!< Aka Texture Buffer
	StorageTexelBuffer, //!< Also known as TextureBuffer
	UniformBuffer, //!< Aka UBO
	StorageBuffer, //!< Aka SSBO
	UniformBufferDynamic, //!< A UBO that can be bound one piece at a time
	StorageBufferDynamic, //!< A SSBO that can be bound one piece at a time
	InputAttachment, //!< An intermediate attachment that can be used between subpasses
	Count = 12,
	NumBits = 4
};

/// <summary>Enumeration of all supported buffer use types.</summary>
enum class BufferUsageFlags  : uint32_t
{
	TransferSrc         = 0x00000001, //!< Transfer Source
	TransferDest        = 0x00000002, //!< Transfer Destination
	UniformTexelBuffer  = 0x00000004, //!< Uniform Texel Buffer
	StorageTexelBuffer  = 0x00000008, //!< Storage Texel Buffer
	UniformBuffer       = 0x00000010, //!< UBO
	StorageBuffer       = 0x00000020, //!< SSBO
	IndexBuffer         = 0x00000040, //!< IBO
	VertexBuffer        = 0x00000080, //!< VBO
	IndirectBuffer      = 0x00000100, //!< A buffer that contains Draw Indirect commands
	Count               = 10
};
DEFINE_ENUM_OPERATORS(BufferUsageFlags)

/// <summary>Infer the BufferUsageFlags that are suitable for the typical use of an object</summary>
/// <param name="descType">A descriptor type</param>
/// <returns>The typical usage flags for <paramref name="descType/></returns>
inline BufferUsageFlags descriptorTypeToBufferUsage(DescriptorType descType)
{
	if (descType == DescriptorType::UniformBuffer || descType == DescriptorType::UniformBufferDynamic)
	{
		return BufferUsageFlags::UniformBuffer;
	}
	return BufferUsageFlags::StorageBuffer;
}

/// <summary>Checks if a descriptor type is dynamic (a dynamic UBO or dynamic SSBO)</summary>
/// <param name="descType">A descriptor type</param>
/// <returns>True if descType is UniformBufferDynamic or StorageBufferDynamic, otherwise false</returns>
inline bool isDescriptorTypeDynamic(DescriptorType descType)
{
	return (descType == DescriptorType::UniformBufferDynamic || descType == DescriptorType::StorageBufferDynamic);
}

template<typename t1, typename t2>
inline t1 align(t1 numberToAlign, t2 alignment)
{
	if (alignment)
	{
		t1 align1 = numberToAlign % (t1)alignment;
		if (!align1) { align1 += (t1)alignment; }
		numberToAlign += t1(alignment) - align1;
	}
	return numberToAlign;
}


/// <summary>An enumeration that defines data types used throughout the Framework. Commonly used in places where
/// raw data are used to define the types actually contained.</summary>
enum class DataType
{
	None,   //!< None, or unknown
	Float32,//!< 32 bit floating point number
	Int32,//!< 32 bit Integer
	UInt16, //!< 16 bit Unsigned Integer (aka Unsigned Short)
	RGBA,//!< 32 bit (4 channels x 8bpc), in Red,Green,Blue,Alpha order
	ARGB,//!< 32 bit (4 channels x 8bpc), in Alpha,Red,Green,Blue order
	D3DCOLOR,//!< Direct3D color format
	UBYTE4,//!< Direct3D UBYTE4 format
	DEC3N,//!< Direct3D DEC3N format
	Fixed16_16, //!< 32 bit Fixed Point (16 + 16)
	UInt8,//!< Unsigned 8 bit integer (aka unsigned char/byte)
	Int16,//!< Signed 16 bit integer (aka short)
	Int16Norm,//!< Signed 16 bit integer scaled to a value from -1..1 (aka normalized short)
	Int8,//!< Signed 8 bit integer (aka char / byte)
	Int8Norm,//!< Signed 8 bit integer, interpreted by scaling to -1..1 (aka normalized byte)
	UInt8Norm,//!< Unsigned 8 bit integer,  interpreted by scaling to 0..1 (aka unsigned normalized byte)
	UInt16Norm,//!< Unsigned 16 bit integer,  interpreted by scaling to 0..1 (aka unsigned normalized short)
	UInt32,//!< Unsigned 32 bit integer (aka Unsigned Int)
	ABGR,//!< 32 bit (4 channels x 8 bpc), in Alpha,Blue,Green,Red order
	Float16,//!< 16 bit  IEEE 754-2008 floating point number (aka Half)
	Custom = 1000
};

/// <summary>Return the Size of a DataType.</summary>
/// <param name="type">The Data type</param>
/// <returns>The size of the Datatype in bytes.</returns>
inline uint32_t dataTypeSize(DataType type)
{
	switch (type)
	{
	default:
		assert(false);
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
/// <summary>Return the number of components in a datatype.</summary>
/// <param name="type">The datatype</param>
/// <returns>The number of components (e.g. float is 1, vec3 is 3)</returns>
inline uint32_t  numDataTypeComponents(DataType type)
{
	switch (type)
	{
	default:
		assert(false);
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

/// <summary>Return if the format is Normalized (represents a range between 0..1 for unsigned types or between -1..1
/// for signed types)</summary>
/// <param name="type">The format to test.</param>
/// <returns>True if the format is Normalised.</returns>
/// <remarks>A Normalised format is a value that is stored as an Integer, but that actually represents a value
/// from 0..1 or -1..1 instead of the numeric value of the Integer. For example, for a normalised unsigned char
/// value, the value 0 represents 0.0, the value 127 represents 0.5 and the value 255 represents 1.0.</remarks>
inline bool  dataTypeIsNormalised(DataType type)
{
	return (type == DataType::Int8Norm || type == DataType::UInt8Norm ||
	        type == DataType::Int16Norm || type == DataType::UInt16Norm);
}

/// <summary>Enumeration of Colorspaces (Linear, SRGB).</summary>
enum class ColorSpace
{
	lRGB, //!< Linear RGB colorspace
	sRGB, //!< sRGB colorspace
	NumSpaces
};

/// <summary>Groups functionality that has to do with bit calculations/sizes/offsets of glsl types</summary>
namespace GpuDatatypesHelper {
/// <summary> A bit representing if a type is basically of integer or floating point format </summary>
enum class BaseType { Integer = 0, Float = 1 };
/// <summary> Two bits, representing the number of vector components (from scalar up to 4)</summary>
enum class  VectorWidth { Scalar = 0, Vec2 = 1, Vec3 = 2, Vec4 = 3, };
/// <summary> Three bits, representing the number of matrix columns (from not a matrix to 4)</summary>
enum class  MatrixColumns { OneCol = 0, Mat2x = 1, Mat3x = 2, Mat4x = 3 };

/// <summary> Contains bit enums for the expressiveness of the GpuDatatypes class' definition</summary>
enum class Bits : uint32_t
{
	Integer = 0, Float = 1,
	BitScalar = 0, BitVec2 = 2, BitVec3 = 4, BitVec4 = 6,
	BitOneCol = 0, BitMat2x = 8, BitMat3x = 16, BitMat4x = 24,
	ShiftType = 0, MaskType = 1, NotMaskType = static_cast<uint32_t>(~MaskType),
	ShiftVec = 1, MaskVec = (3 << ShiftVec), NotMaskVec = static_cast<uint32_t>(~MaskVec),
	ShiftCols = 3, MaskCols = (3 << ShiftCols), NotMaskCols = static_cast<uint32_t>(~MaskCols)
};
DEFINE_ENUM_OPERATORS(Bits)
};

/// <summary>A (normally hardware-supported) GPU datatype (e.g. vec4 etc.)</summary>
enum class GpuDatatypes : uint32_t
{
	Integer = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Integer) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitScalar) |
	          static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitOneCol), uinteger = Integer, boolean = Integer,
	Float = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitScalar) |
	        static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitOneCol),
	ivec2 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Integer) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec2) |
	        static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitOneCol), uvec2 = ivec2, bvec2 = ivec2,
	ivec3 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Integer) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec3) |
	        static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitOneCol), uvec3 = ivec3, bvec3 = ivec3,
	ivec4 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Integer) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec4) |
	        static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitOneCol), uvec4 = ivec4, bvec4 = ivec4,
	vec2 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec2) |
	       static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitOneCol),
	vec3 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec3) |
	       static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitOneCol),
	vec4 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec4) |
	       static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitOneCol),
	mat2x2 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec2) |
	         static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitMat2x),
	mat2x3 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec3) |
	         static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitMat2x),
	mat2x4 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec4) |
	         static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitMat2x),
	mat3x2 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec2) |
	         static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitMat3x),
	mat3x3 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec3) |
	         static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitMat3x),
	mat3x4 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec4) |
	         static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitMat3x),
	mat4x2 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec2) |
	         static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitMat4x),
	mat4x3 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec3) |
	         static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitMat4x),
	mat4x4 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec4) |
	         static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitMat4x),
	none = 0xFFFFFFFF,
	structure = none
};
/// <summary>Bitwise operator AND. Typical semantics. Allows AND between GpuDatatypes and Bits</summary>
/// <param name="lhs">Left hand side</param>
/// <param name="rhs">Right hand side</param>
/// <returns>lhs AND rhs</returns>
inline GpuDatatypes operator & (GpuDatatypes lhs, GpuDatatypesHelper::Bits rhs)
{
	return static_cast<GpuDatatypes>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

/// <summary>Bitwise operator RIGHT SHIFT. Typical semantics. Allows RIGHT SHIFT of GpuDatatypes by Bits</summary>
/// <param name="lhs">Left hand side</param>
/// <param name="rhs">Right hand side</param>
/// <returns>lhs RIGHT SHIFT rhs</returns>
inline GpuDatatypes operator >> (GpuDatatypes lhs, GpuDatatypesHelper::Bits rhs)
{
	return static_cast<GpuDatatypes>(static_cast<uint32_t>(lhs) >> static_cast<uint32_t>(rhs));
}

/// <summary>Bitwise operator LEFT SHIFT. Typical semantics. Allows LEFT SHIFT of GpuDatatypes by Bits</summary>
/// <param name="lhs">Left hand side</param>
/// <param name="rhs">Right hand side</param>
/// <returns>lhs LEFT SHIFT rhs</returns>
inline GpuDatatypes operator << (GpuDatatypes lhs, GpuDatatypesHelper::Bits rhs)
{
	return static_cast<GpuDatatypes>(static_cast<uint32_t>(lhs) << static_cast<uint32_t>(rhs));
}

/// <summary>Get the number of colums (1..4) of the type</summary>
/// <param name="type">The datatype to test</param>
/// <returns>The number of matrix colums (1..4) of the type. 1 implies not a matrix</returns>
inline uint32_t getNumMatrixColumns(GpuDatatypes type)
{
	return static_cast<uint32_t>(GpuDatatypesHelper::MatrixColumns(static_cast<uint32_t>((type & GpuDatatypesHelper::Bits::MaskCols) >> GpuDatatypesHelper::Bits::ShiftCols) + 1));
}

/// <summary>Get required alignment of this type as demanded by std140 rules</summary>
/// <param name="type">The datatype to test</param>
/// <returns>The required alignment of the type based on std140 (see the GLSL spec)</returns>
inline uint32_t getAlignment(GpuDatatypes type)
{
	uint32_t vectype = static_cast<uint32_t>(type & GpuDatatypesHelper::Bits::MaskVec);
	return (vectype == static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitScalar) ? 4 : vectype == static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec2) ? 8 : 16);
}

/// <summary>Get the size of a type, including padding, assuming the next item is of the same type</summary>
/// <param name="type">The datatype to test</param>
/// <returns>The size plus padding of this type</returns>
inline uint32_t getVectorSelfAlignedSize(GpuDatatypes type)
{
	return getAlignment(type);
}

/// <summary>Get the number of vector elements (i.e. Rows) of a type. (e.g. vec2=>2)</summary>
/// <param name="type">The datatype to test</param>
/// <returns>The number of vector elements.</returns>
inline uint32_t getNumVecElements(GpuDatatypes type)
{
	return static_cast<uint32_t>(GpuDatatypesHelper::VectorWidth(static_cast<uint32_t>((type & GpuDatatypesHelper::Bits::MaskVec) >> GpuDatatypesHelper::Bits::ShiftVec) + 1));
}

/// <summary>Get the cpu-packed size of each vector element a type (disregarding matrix columns if they exist)</summary>
/// <param name="type">The datatype to test</param>
/// <returns>The size that a single column of <paramRef name="type"/> would take on the CPU</returns>
inline uint32_t getVectorUnalignedSize(GpuDatatypes type)
{
	return 4 * getNumVecElements(type);
}

/// <summary>Get the underlying element of a type (integer or float)</summary>
/// <param name="type">The datatype to test</param>
/// <returns>A BaseType enum (integer or float)</returns>
inline GpuDatatypesHelper::BaseType getBaseType(GpuDatatypes type)
{
	return GpuDatatypesHelper::BaseType(static_cast<uint32_t>(type) & 1);
}

/// <summary>Returns a datatype that is larger or equal to both of two types:
/// 1) Has the most permissive base type (float>int)
/// 2) Has the largest of the two vector widths
/// 3) Has the most of the two matrix colums heights</summary>
/// <param name="type1">The first type</param>
/// <param name="type2">The second type</param>
/// <returns>A type that can fit either of type1 or type1</returns>
inline GpuDatatypes mergeDatatypesBigger(GpuDatatypes type1, GpuDatatypes type2)
{
	uint32_t baseTypeBits = static_cast<uint32_t>((::std::max)(type1 & GpuDatatypesHelper::Bits::MaskType, type2 & GpuDatatypesHelper::Bits::MaskType));
	uint32_t vectorWidthBits = static_cast<uint32_t>((::std::max)(type1 & GpuDatatypesHelper::Bits::MaskVec, type2 & GpuDatatypesHelper::Bits::MaskVec));
	uint32_t matrixColBits = static_cast<uint32_t>((::std::max)(type1 & GpuDatatypesHelper::Bits::MaskCols, type2 & GpuDatatypesHelper::Bits::MaskCols));
	return GpuDatatypes(baseTypeBits | vectorWidthBits | matrixColBits);
}

/// <summary>Returns a datatype that is smaller or equal to both of two types:
/// 1) Has the most permissive base type (float>int)
/// 2) Has the smaller of the two vector widths
/// 3) Has the least of the two matrix colums heights</summary>
/// <param name="type1">The first type</param>
/// <param name="type2">The second type</param>
/// <returns>A type that will truncate everything the two types don't share</returns>
inline GpuDatatypes mergeDatatypesSmaller(GpuDatatypes type1, GpuDatatypes type2)
{
	uint32_t baseTypeBits = static_cast<uint32_t>((::std::max)(type1 & GpuDatatypesHelper::Bits::MaskType, type2 & GpuDatatypesHelper::Bits::MaskType));
	uint32_t vectorWidthBits = static_cast<uint32_t>((::std::min)(type1 & GpuDatatypesHelper::Bits::MaskVec, type2 & GpuDatatypesHelper::Bits::MaskVec));
	uint32_t matrixColBits = static_cast<uint32_t>((::std::min)(type1 & GpuDatatypesHelper::Bits::MaskCols, type2 & GpuDatatypesHelper::Bits::MaskCols));
	return GpuDatatypes(baseTypeBits | vectorWidthBits | matrixColBits);
}

/// <summary>Returns "how many bytes will an object of this type take", if not an array.</summary>
/// <param name="type">The datatype to test</param>
/// <returns>The size of this type, aligned to its own alignment restrictions</returns>
inline uint32_t getSelfAlignedSize(GpuDatatypes type)
{
	uint32_t isMatrix = (getNumMatrixColumns(type) > 1);

	return (::std::max)(getVectorSelfAlignedSize(type), static_cast<uint32_t>(16) * isMatrix) * getNumMatrixColumns(type);
}

/// <summary>Returns "how many bytes will an object of this type take", if it is an array member (arrays have potentially stricter requirements).</summary>
/// <param name="type">The datatype to test</param>
/// <returns>The size of this type, aligned to max array alignment restrictions</returns>
inline uint32_t getSelfAlignedArraySize(GpuDatatypes type)
{
	return (::std::max)(getVectorSelfAlignedSize(type), static_cast<uint32_t>(16)) * getNumMatrixColumns(type);
}

/// <summary>Returns how many bytes an array of n objects of this type take, but arrayElements = 1
/// is NOT considered an array (is aligned as a single object, NOT an array of 1)</summary>
/// <param name="type">The datatype to test</param>
/// <param name="arrayElements">The number of array elements. 1 is NOT considered an array.</param>
/// <returns>The size of X elements takes</returns>
inline uint64_t getSize(GpuDatatypes type, uint32_t arrayElements = 1)
{
	uint64_t numElements = getNumMatrixColumns(type) * arrayElements;

	assert(numElements > 0);
	uint64_t selfAlign = (::std::max)(static_cast<uint64_t>(getVectorSelfAlignedSize(type)), static_cast<uint64_t>(16)) * numElements * (numElements > 1); //Equivalent to: if (arrayElements!=0) selfAlign = getVectorSelfAlignedSize(...) else selfAlign=0;
	uint64_t unaligned = getVectorUnalignedSize(type) * (numElements == 1);//Equivalent to: if (arrayElements==0) unaligned = getVectorUnalignedSize(...) else unaligned = 0;
	return selfAlign + unaligned;

	// *** THIS IS A BRANCHLESS EQUIVALENT OF THE FOLLOWING CODE ***
	//if (numElements > 1)
	//{
	// return (std::max)(getVectorSelfAlignedSize(type), 16u) * numElements;
	//}
	//else
	//{
	// return getVectorUnalignedSize(type);
	//}
}

/// <summary>Get a string with the glsl variable name of a type</summary>
/// <param name="type">The datatype to test</param>
/// <returns>A c-style string with the glsl variable keyword of <paramRef name="type"/></returns>
inline const char* toString(GpuDatatypes type)
{
	switch (type)
	{
	case GpuDatatypes::Integer: return "int";
	case GpuDatatypes::ivec2: return "ivec2";
	case GpuDatatypes::ivec3: return "ivec3";
	case GpuDatatypes::ivec4: return "ivec4";
	case GpuDatatypes::Float: return "float";
	case GpuDatatypes::vec2: return "vec2";
	case GpuDatatypes::vec3: return "vec3";
	case GpuDatatypes::vec4: return "vec4";
	case GpuDatatypes::mat2x2: return "mat2x2";
	case GpuDatatypes::mat2x3: return "mat2x3";
	case GpuDatatypes::mat2x4: return "mat2x4";
	case GpuDatatypes::mat3x2: return "mat3x2";
	case GpuDatatypes::mat3x3: return "mat3x3";
	case GpuDatatypes::mat3x4: return "mat3x4";
	case GpuDatatypes::mat4x2: return "mat4x2";
	case GpuDatatypes::mat4x3: return "mat4x3";
	case GpuDatatypes::mat4x4: return "mat4x4";
	case GpuDatatypes::none: return "NONE";
	default: return "UNKNOWN";
	}
}

/// <summary>Get the size of n array members of a type, packed in CPU</summary>
/// <param name="type">The datatype to test</param>
/// <param name="arrayElements">The number of array elements</param>
/// <returns>The base size of the type multiplied by arrayElements</returns>
inline uint64_t getCpuPackedSize(GpuDatatypes type, uint32_t arrayElements = 1)
{
	return getVectorUnalignedSize(type) * getNumMatrixColumns(type) * arrayElements;
}

/// <summary>Aligns an address/offset with the alignment of a type -- equivalently,
/// assuming you want to place a type after a known offset (i.e. calculating the
/// offset of an item inside a struct having already calculated its previous element)
/// (i.e. aligning a vec4 after an item that ends at 30 bytes returns 32 bytes...)
/// </summary>
/// <param name="type">The datatype to test</param>
/// <param name="previousTotalSize">The address/offset to align for that type</param>
/// <returns><paramRef name="previousTotalSize"/> aligned to the requirements of
/// <paramRef name="type"/></returns>
inline uint32_t getOffsetAfter(GpuDatatypes type, uint64_t previousTotalSize)
{
	uint32_t align = getAlignment(type);

	uint32_t diff = previousTotalSize % align;
	diff += (align * (diff == 0)); // REMOVE BRANCHING -- equal to : if(diff==0) { diff+=8 }
	return static_cast<uint32_t>(previousTotalSize) - diff + align;
}

/// <summary>Returns the new size of a hypothetical struct whose old size was previousTotalSize,
/// and to which "arrayElement" new items of type "type" are added</summary>
/// <param name="type">The datatype to add</param>
/// <param name="arrayElements">The number of items of type <paramRef name="type"/> to add</param>
/// <param name="previousTotalSize">The address/offset to align for that type</param>
/// <returns>The new size</returns>
inline uint64_t getTotalSizeAfter(GpuDatatypes type, uint32_t arrayElements, uint64_t previousTotalSize)
{
	assert(arrayElements > 0);
	// Arrays pads their last element to their alignments. Standalone objects do not. vec3[3] is NOT the same as vec3;vec3;vec3;
	uint64_t selfAlignedSize = getSelfAlignedArraySize(type) * arrayElements * (arrayElements != 1); //Equivalent to: if (arrayElements!=1) selfAlign = getSelfAl... else selfAlign=0
	// Other elements do not.
	uint64_t unalignedSize = getSize(type) * (arrayElements == 1);//Equivalent to: if (arrayElements==1) unaligned = getUnaligned.... else unaligned=0

	return getOffsetAfter(type, previousTotalSize) + selfAlignedSize + unalignedSize;
}

/// <summary>Get the Cpu Datatype <paramRef name="type"/> refers to (i.e. which CPU datatype must you
/// load in the data you upload to the GPU to correctly upload the same value  in the shader).</summary>
/// <param name="type">The type to convert</param>
/// <returns>A CPU type that has the same bit representation as one scalar element of type (i.e. mat4x4 returns "float")</returns>
inline DataType toDataType(GpuDatatypes type)
{
	return getBaseType(type) == GpuDatatypesHelper::BaseType::Float ? DataType::Float32 : DataType::Int32;
}

namespace GpuDatatypesHelper {
template<typename T> struct Metadata; // Template specializations in FreeValue.h
}

/// <summary>Enumeration containing all possible Primitive topologies (Point, line trianglelist etc.).</summary>
enum class PrimitiveTopology : uint32_t
{
	//POSITION-SENSITIVE. Do not renumber unless also refactoring ConvertToVkTypes.
	PointList, //!< Renders poins
	LineList, //!< Each two items render a separate line segment
	LineStrip, //!< Renders one continuous polyline (n vertices represent n-1 lines)
	TriangleList, //!< Each 3 vertices render one triangle
	TriangleStrip, //!< Renders one continuous triangle strip, (n vertices represent n-2 triangles in a strip configuration)
	TriangleFan, //!< Renders one continuous triangle fan (n vertices represent n-2 triangles in a fan configuration)
	LineListWithAdjacency, //!< Represents a list of lines, but contains adjacency info (2 additional vertices per 2 vertices: 4 vertices per line segment)
	LineStripWithAdjacency, //!< Represents a continuous strip of lines, but contains adjacency info (2 additional vertices: the vertex before the first and the vertex after the last line segment)
	TriangleListWithAdjacency, //!< Represents a triangle list with adjacency info (6 vertices per primitive).
	TriangleStripWithAdjacency,//!< Represents a triangle strip with adjacency info (1 additional adjacency vertex per triangle, plus the adjacent vertices of the first and last triangle sides of the list).
	PatchList,//!< A list of Patches, intended for tessellation
	Count
};

/// <summary>ChannelWriteMask enable/ disable writting to channel bits.</summary>
enum class ColorChannelFlags : uint32_t
{
	//DO NOT REARRANGE - Direct mapping to Vulkan
	R       = 0x01, //!< write to red channel
	G       = 0x02, //!< write to green channel
	B       = 0x04, //!< write to blue channel
	A       = 0x08, //!< write to alpha channel
	None    = 0, //!< don't write to any channel
	All     = R | G | B | A //< write to all channel
};
DEFINE_ENUM_OPERATORS(ColorChannelFlags)

/// <summary>Step rate for a vertex attribute when drawing: Per vertex, per instance, per draw.</summary>
enum class StepRate : uint32_t
{
	Vertex, //!< Step rate Per vertex
	Instance,//!< Step rate per instance
	Default = Vertex
};

/// <summary>Enumeration of Face facing (front, back...).</summary>
enum class Face : uint32_t
{
	//DO NOT REARRANGE - DIRECT TO VULKAN
	None = 0, //!< No faces
	Front = 1, //!< The front face
	Back = 2, //!< The back face
	FrontAndBack = 3, //!< Both faces
	Default = None
};

/// <summary>Enumeration of the six faces of a Cube</summary>
enum class CubeFace : uint32_t
{
	PositiveX = 0, //!<+x
	NegativeX, //!<-x
	PositiveY,//!<+y
	NegativeY,//!<-y
	PositiveZ,//!<+z
	NegativeZ//!<-z
};

/// <summary>Enumeration of the blend operations (determine how a new pixel (source color) is combined with a pixel
/// already in the framebuffer (destination color).</summary>
enum class BlendOp : uint32_t
{
	//DO NOT REARRANGE - Direct mapping to Vulkan. See convertToVk
	Add, //!<Addition
	Subtract, //!<Subtraction second from first
	ReverseSubtract, //!< Subtract first from second
	Min, //!< Minimum of the two
	Max, //!< Maximum of the two
	NumBlendFunc,
	Default = Add
};

/// <summary>Specfies how the rgba blending facors are computed for source and destination fragments.</summary>
enum class  BlendFactor : uint8_t
{
	Zero, //!<Zero
	One, //!<One
	SrcColor, //!<The colour of the incoming fragment
	OneMinusSrcColor, //!< 1 - (SourceColor)
	DstColor, //!< The color of the pixel already in the framebuffer
	OneMinusDstColor, //!< 1 - (Destination Color)
	SrcAlpha, //!< The alpha of the incoming fragment
	OneMinusSrcAlpha, //!< 1- (Source Alpha)
	DstAlpha, //!< The alpha of the pixel already in the framebuffer (requires an alpha channel)
	OneMinusDstAlpha, //!< 1- (Destination Alpha)
	ConstantColor, //!< A constant color provided by the api
	OneMinusConstantColor, //!< 1- (Constant Color)
	ConstantAlpha, //!< A constant alpha value provided by the api
	OneMinusConstantAlpha, //!< 1- (ConstantAlpha)
	Src1Color, //!< Source Color 1
	OneMinusSrc1Color, //!< 1 - (Source Color 1)
	Src1Alpha, //!< Source Alpha 1
	OneMinusSrc1Alpha, //!< 1 - (Source Alpha 1)
	NumBlendFactor,
	DefaultSrcRgba = One,
	DefaultDestRgba = Zero
};

/// <summary>Enumeration of the different front face to winding order correlations.</summary>
enum class PolygonWindingOrder : uint8_t
{
//DO NOT REARRANGE - VULKAN DIRECT MAPPING
	FrontFaceCCW, //!< Front face is the Counter Clockwise face
	FrontFaceCW,  //!< Front face is the Clockwise face
	Default = FrontFaceCCW
};

/// <summary>Enumeration of the different stencil operations.</summary>
enum class StencilOp : uint8_t
{
	//DO NOT REARRANGE - VULKAN DIRECT MAPPING
	Keep, //!< Keep existing value
	Zero, //!< Set to zero
	Replace, //!< Replace value with Ref
	IncrementClamp, //!< Increment until max value
	DecrementClamp, //!< Decrement until min value
	Invert, //!< Bitwise-not the existing value
	IncrementWrap, //!< Increment the existing value, wrap if >max
	DecrementWrap, //!< Decrement the existing value, wrap if <min
	NumStencilOp,

	// Defaults
	Default = Keep,
};

/// <summary>Capability supported values.</summary>
enum class Capability : uint8_t
{
	Unsupported, //!< The capability is unsupported
	Immutable, //!< The capability exists but cannot be changed
	Mutable //!< The capability is supported and can be changed
};

/// <summary>An enumeration that defines a type that can use as an index, typically 16 or 32 bit int. Especially
/// used in Model classes.</summary>
enum class IndexType : uint32_t
{
	IndexType16Bit = static_cast<uint32_t>(DataType::UInt16),//!< 16 bit index
	IndexType32Bit = static_cast<uint32_t>(DataType::UInt32)//!< 32 bit index
};

/// <summary>Return the Size of an IndexType in bytes.</summary>
/// <param name="type">The Index type</param>
/// <returns>The number of bytes in an index type</returns>
inline uint32_t indexTypeSizeInBytes(const IndexType type)
{
	switch (type)
	{
	default:
		assert(false);
		return false;
	case IndexType::IndexType16Bit: return 2;
	case IndexType::IndexType32Bit: return 4;
	}
}

/// <summary>An enumeration that defines Comparison operations (equal, less or equal etc.). Especially used in
/// API classes for functions like depth testing.</summary>
enum class CompareOp : uint32_t
{
	//DIRECT MAPPING FOR VULKAN - DO NOT REARRANGE
	Never = 0, //!< Always false
	Less = 1,  //!< True if lhs<rhs
	Equal = 2,  //!< True if lhs==rhs
	LessEqual = 3,  //!< True if lhs<=rhs
	Greater = 4,  //!< True if lhs>rhs
	NotEqual = 5,  //!< True if lhs!=rhs
	GreaterEqual = 6,  //!< True if lhs>=rhs
	Always = 7,   //!< Always true
	NumComparisonMode,
	DefaultDepthFunc = Less,
	DefaultStencilFunc = Always,
};

/// <summary>Enumeration describing a filtering type of a specific dimension. In order to describe the filtering mode
/// properly, you would have to define a Minification filter, a Magnification filter and a Mipmapping minification
/// filter. Possible values: Nearest, Linear, Cubic, None.</summary>
enum class Filter : uint8_t
{
	Nearest,//!< Nearest neighbour
	Linear,//< Linear (average weighted by distance)
	None,//!< No filtering
	Cubic,//!< Bicubic filtering (IMG extension)
	Default = Linear,
	MipDefault = Linear,
	Size = 4
};

/// <summary>Enumeration for defining texture wrapping mode: Repeat, Mirror, Clamp, Border.</summary>
enum class SamplerAddressMode  : uint8_t
{
	Repeat,//!< repeat
	MirrorRepeat,//!< mirror repeat
	ClampToEdge,//!< clamp
	ClampToBorder,//!< border
	MirrorClampToEdge,//!< mirror clamp
	Size,
	Default = Repeat
};

/// <summary> Enumeration of mipmap modes supported for a sampler</summary>
enum class SamplerMipmapMode : uint8_t
{
	Nearest, //!< Nearest neighbour
	Linear, //!< Linear
	Count
};

/// <summary> This enum is made to pack all sampler filtering info in 8 bits for specific uses. Use "packSamplerFilter" and "unpackSamplerFilter".
/// NOTE: The defined values are only the most common cases - other 8 bit values are also valid (for example, different minification and magnification filters)</summary>
enum PackedSamplerFilter : int8_t
{
	PackNone,//< no filter
	PackNearestMipNone = static_cast<uint8_t>(static_cast<uint8_t>(Filter::Nearest) | (static_cast<uint8_t>(Filter::Nearest) << 2) | (static_cast<uint8_t>(SamplerMipmapMode::Nearest) << 4)), //<Nearest Neighbour, no mipmap use
	PackNearestMipNearest = static_cast<uint8_t>(static_cast<uint8_t>(Filter::Nearest) | (static_cast<uint8_t>(Filter::Nearest) << 2) | (static_cast<uint8_t>(SamplerMipmapMode::Nearest) << 4)), //<Nearest Neighbour per mipmap, nearest neighbour between mipmaps
	PackNearestMipLinear = static_cast<uint8_t>(static_cast<uint8_t>(Filter::Nearest) | (static_cast<uint8_t>(Filter::Nearest) << 2) | (static_cast<uint8_t>(SamplerMipmapMode::Linear) << 4)), //<Nearest Neighbour, linearly interpolate between mipmaps (OpenGL Default)
	PackLinearMipNone = static_cast<uint8_t>(static_cast<uint8_t>(Filter::Linear) | (static_cast<uint8_t>(Filter::Linear) << 2) | (static_cast<uint8_t>(SamplerMipmapMode::Nearest) << 4)), //< Bilinear, no mipmap use
	PackLinearMipNearest = static_cast<uint8_t>(static_cast<uint8_t>(Filter::Linear) | (static_cast<uint8_t>(Filter::Linear) << 2) | (static_cast<uint8_t>(SamplerMipmapMode::Nearest) << 4)), //< Bilinear, nearest neighbour between mipmaps
	PackTrilinear = static_cast<uint8_t>(static_cast<uint8_t>(Filter::Linear) | (static_cast<uint8_t>(Filter::Linear) << 2) | (static_cast<uint8_t>(SamplerMipmapMode::Linear) << 4)), //< Full Trilinear (bilinear, linearly interpolate between mipmaps)
	Size, //< number of supported filter
	PackDefault = PackTrilinear//< default filter
};

/// <summary>Pack a minification filter, a magnification filter and a mipmap filter into an 8 bit value</summary>
/// <param name="mini">The filtering mode that should be used for minification</param>
/// <param name="magni">The filtering mode that should be used for magnification</param>
/// <param name="mip">The filtering mode that should be used for mipmapping</param>
/// <returns>An 8 bit value representing the described sampler</returns>
inline PackedSamplerFilter packSamplerFilter(Filter mini, Filter magni, SamplerMipmapMode mip)
{
	return PackedSamplerFilter(static_cast<PackedSamplerFilter>(mini) + (static_cast<PackedSamplerFilter>(magni) << 2) + (static_cast<PackedSamplerFilter>(mip) << 4));
}

/// <summary>Unpack a 8 bit PackedSamplerFilter value into a minification, magnification and mip filter mode</summary>
/// <param name="packed">The packed sampler filter to unpack</param>
/// <param name="mini">The filtering mode that should be used for minification</param>
/// <param name="magni">The filtering mode that should be used for magnification</param>
/// <param name="mip">The filtering mode that should be used for mipmapping</param>
/// <returns>An 8 bit value representing the described sampler</returns>
inline void unpackSamplerFilter(PackedSamplerFilter packed, Filter& mini, Filter& magni, SamplerMipmapMode& mip)
{
	mini = static_cast<Filter>(packed & 3);
	magni = static_cast<Filter>((packed >> 2) & 3);
	mip = static_cast<SamplerMipmapMode>(packed >> 4);
}

/// <summary>The dimension of an image.</summary>
enum class ImageType
{
	Image1D, //!<One-dimensional image
	Image2D, //!<Two-dimensional image
	Image3D, //!<Three-dimensional image
	Unallocated, //!<An image that has not been allocated yet
	Unknown,     //!<An image of unknown dimensions
	Count = Image3D + 1
};

/// <summary>Enumeration of Texture dimensionalities.</summary>
enum class ImageViewType
{
	ImageView1D,            //!< 1 dimensional Image View
	ImageView2D,            //!< 2 dimensional Image View
	ImageView3D,            //!< 3 dimensional Image View
	ImageView2DCube,        //!< cube texture
	ImageView1DArray,       //!< 1 dimensional Image View
	ImageView2DArray,       //!< 2 dimensional Image View
	ImageView2DCubeArray,   //!< 2 dimensional Image View
	ImageViewUnknown,       //!< 3 dimensional Image View
};

/// <summary> Map an ImageViewType (2dCube etc) to its base type (1d/2d/3d)</summary>
/// <param name="viewtype">The ImageViewType</param>
/// <returns>The base type</returns>
inline ImageType imageViewTypeToImageBaseType(ImageViewType viewtype)
{
	switch (viewtype)
	{
	case ImageViewType::ImageView1D:
	case ImageViewType::ImageView1DArray:
		return ImageType::Image1D;

	case ImageViewType::ImageView2D:
	case ImageViewType::ImageView2DCube:
	case ImageViewType::ImageView2DArray:
	case ImageViewType::ImageView2DCubeArray:
		return ImageType::Image2D;

	case ImageViewType::ImageView3D:
		return ImageType::Image3D;

	default: return ImageType::Unallocated;
	}
}

/// <summary>Enumeration of all supported shader types.</summary>
enum class ShaderType
{
	UnknownShader = 0,//!< unknown shader type
	VertexShader,//!< vertex shader
	FragmentShader,//!< fragment shader
	ComputeShader,//!< compute shader
	TessControlShader,
	TessEvaluationShader,
	GeometryShader,
	RayShader,
	FrameShader,
	Count
};

/// <summary>Enumeration of the "aspect" (or "semantics") of an image: Color, Depth, Stencil.</summary>
enum class ImageAspectFlags : uint32_t
{
	Color           = 0x1,
	Depth           = 0x2,
	Stencil         = 0x4,
	Metadata        = 0x8,
	DepthAndStencil = Depth | Stencil,
};
DEFINE_ENUM_OPERATORS(ImageAspectFlags)


/// <summary>Pre-defined Result codes (success and generic errors).</summary>
enum class Result
{
	Success,
	UnknownError,
	NotInitialized,
	//Shell Error
	InitializationError,
	UnsupportedRequest,

	ExitRenderFrame, // Used to exit the renderscene loop in the shell
};
/// <summary>Use this function to convert a Result into a std::string that is suitable for outputting.</summary>
/// <param name="result">The result</param>
/// <returns>A std::string suitable for writing out that represents this Result</returns>
inline const char* getResultCodeString(Result result)
{
	switch (result)
	{
	case Result::Success: return "Success";
	case Result::UnknownError: return"Unknown Error";
	case Result::ExitRenderFrame: return"Exit Render Scene";
	case Result::NotInitialized: return"Not initialized";
	case Result::InitializationError: return"Error while initializing";
	default: return"UNRECOGNIZED CODE";
	}
}

/// <summary>Represents a buffer of Unsigned Bytes. Used to store raw data.</summary>
typedef ::std::vector<uint8_t> UInt8Buffer;

/// <summary>Represents a buffer of Unsigned Bytes. Used to store raw data.</summary>
typedef ::std::vector<char> CharBuffer;

/// <summary>Representation of raw data. Used to store raw data that is logically grouped in blocks with a stride.</summary>
class StridedBuffer : public UInt8Buffer
{
public:
	uint16_t stride; //!< The stride of the buffer
};

/// <summary>Get a random Number between min and max</summary>
/// <param name="min">Minimum number (inclusive)</param>
/// <param name="max">Maximum number (inclusive)</param>
/// <returns>Random number</returns>
inline float randomrange(float min, float max)
{
	float zero_to_one = static_cast<float>(double(rand()) / double(RAND_MAX));
	float diff = max - min;
	return zero_to_one * diff + min;
}
//!\cond NO_DOXYGEN
#if defined(_MSC_VER)
#define PVR_ALIGNED __declspec(align(16))
#elif defined(__GNUC__) || defined (__clang__)
#define PVR_ALIGNED __attribute__((aligned(16)))
#else
#define PVR_ALIGNED alignas(16)
#endif
//!\endcond
}

/// <summary>ARRAY_SIZE(a) is a compile-time constant which represents the number of elements of the given
/// array. ONLY use ARRAY_SIZE for statically allocated arrays.</summary>
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#undef DEFINE_ENUM_OPERATORS