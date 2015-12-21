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


namespace {
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
};

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
	Count,
};


inline const char* getApiCode(Enum api)
{
	static const char* ApiCodes[] =
	{
		"",
		"ES2",
		"ES3",
		"ES31",
	};
	return ApiCodes[api];
}
inline const char* getApiName(Enum api)
{
	static const char* ApiCodes[] =
	{
		"Unknown",
		"OpenGL ES 2.0",
		"OpenGL ES 3.0",
		"OpenGL ES 3.1",
	};
	return ApiCodes[api];
}
}


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
};

/*!*********************************************************************************************
\brief  Enumeration containing all possible Primitive topologies (Point, line trianglelist etc.).
***********************************************************************************************/
namespace PrimitiveTopology {
enum Enum
{
	Points,
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
};

/*!*********************************************************************************************
\brief  Enumeration all possible values of operations to be performed on initially Loading a
        Framebuffer Object.
***********************************************************************************************/
namespace LoadOp {
enum Enum
{
	Load, //<
	Ignore, //< ignore writing to the fbo and keep old data
	Clear//< clear the fbo
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
	ResolveMsaa,//<
	Ignore,//< don't write the source to the destination
};
}

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
	AlreadyInitialised,
	NotInitialised,
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
\brief  A fixed size array data structure wrapper.
\param  T1 The type of component in the array.
\param  Size The size of the array.
***********************************************************************************************/
template<typename T1, size_t size>
struct StaticArray
{
private:
	T1 m_data[size];
public:
	/*!*********************************************************************************************
	\brief  Array indexing.
	***********************************************************************************************/
	const T1& operator[](size_t idx) const { return m_data[idx]; }
	/*!*********************************************************************************************
	\brief  Array indexing.
	***********************************************************************************************/
	T1& operator[](size_t idx) { return m_data[idx]; }
};

#define BIT(shift)((1) << (shift))

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
		PVR_ASSERT(false);
		return 0;
	case DataType::Float32:
		return static_cast<uint32>(sizeof(float));
	case DataType::Int32:
	case DataType::UInt32:
		return static_cast<uint32>(sizeof(int));
	case DataType::Int16:
	case DataType::Int16Norm:
	case DataType::UInt16:
		return static_cast<uint32>(sizeof(unsigned short));
	case DataType::RGBA:
		return static_cast<uint32>(sizeof(unsigned int));
	case DataType::ABGR:
		return static_cast<uint32>(sizeof(unsigned int));
	case DataType::ARGB:
		return static_cast<uint32>(sizeof(unsigned int));
	case DataType::D3DCOLOR:
		return static_cast<uint32>(sizeof(unsigned int));
	case DataType::UBYTE4:
		return static_cast<uint32>(sizeof(unsigned int));
	case DataType::DEC3N:
		return static_cast<uint32>(sizeof(unsigned int));
	case DataType::Fixed16_16:
		return static_cast<uint32>(sizeof(unsigned int));
	case DataType::UInt8:
	case DataType::UInt8Norm:
	case DataType::Int8:
	case DataType::Int8Norm:
		return static_cast<uint32>(sizeof(unsigned char));
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
		PVR_ASSERT(false);
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
		PVR_ASSERT(false);
		return false;
	case IndexType::IndexType16Bit: return 2;
	case IndexType::IndexType32Bit: return 4;
	}
}
};

/*!*********************************************************************************************
\brief  An enumeration that defines Comparison operations (equal, less or equal etc.).
        Especially used in API classes for functions like depth testing.
***********************************************************************************************/
namespace ComparisonMode {
enum Enum
{
	None = 0,
	LessEqual,
	Less,
	Equal,
	NotEqual,
	Greater,
	GreaterEqual,
	Always,
	Never,
	NumComparisonMode,
	Default = Always,

	DefaultStencilOpFront = Always,
	DefaultStencilOpBack = Always,

};
};

namespace assets {
/*!*********************************************************************************************
\brief       This class contains all the information of a Vertex Attribute's layout inside a
             block of memory, typically a Vertex Buffer Object. This informations is normally
			 the DataType of the attribute, the Offset (from the beginning of the array) and the
			 width (how many values of type DataType form an attribute).
***********************************************************************************************/
struct VertexAttributeLayout
{
	// Type of data stored, should this be an enum or should it be an int to allow for users to do their own data types
	DataType::Enum dataType;
	uint16 offset; // Should be 16 bit?
	uint8 width; // Number of values per vertex
	VertexAttributeLayout() {}
	VertexAttributeLayout(DataType::Enum dataType, uint8 width, uint16 offset) :
		dataType(dataType), offset(offset), width(width) {}
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
	OpaqueWhite, //< white border with alpha 1: (1,1,1,1)
	TransparentBlack, //< Black Border with alpha 0 : (0,0,0,0)
	OpaqueBlack, //< Black border with alpha 1 : (0,0,0,1)
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
\brief  Class wrapping an arithmetic type and providing bitwise operation for its bits
***********************************************************************************************************************/
template<typename Storage_>
class Bitfield
{
public:
	inline static bool isSet(Storage_ store, int8 bit)
	{
		return (store & (1 << bit)) != 0;
	}
	inline static void set(Storage_ store, int8 bit)
	{
		store |= (1 << bit);
	}
	inline static void clear(Storage_ store, int8 bit)
	{
		store &= (!(1 << bit));
	}

};


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
	TextureUnknown,//!< 3 dimesional array texture
};
}

/*!*********************************************************************************************************************
\brief Enumeration of the six faces of a Cube
***********************************************************************************************************************/
namespace CubeFace { enum Enum { PositiveX = 0, NegativeX, PositiveY, NegativeY, PositiveZ, NegativeZ }; };

inline float32 randomrange(float32 min, float32 max)
{
	float32 zero_to_one = float32(float64(rand()) / float64(RAND_MAX));
	float32 diff = max - min;
	return zero_to_one * diff + min;
}

#if defined(_MSC_VER)
#define PVR_ALIGNED __declspec(align(16))
#elif defined(__GNUC__) || defined (__clang__)
#define PVR_ALIGNED __attribute__((aligned(16)))
#else
#define PVR_ALIGNED alignas(16)
#endif

}
