/*!
\brief Contains framework types.
\file PVRVk/TypesVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/HeadersVk.h"
#include "PVRVk/RefCounted.h"
#include "PVRVk/Log.h"
#include <vector>
#include <algorithm>

// Disable warning messages for 4351 (https://msdn.microsoft.com/en-us/library/1ywe7hcy.aspx).
#pragma warning(disable : 4351)
/// <summary> Internal helper class that is used for unknown size arrays, utilising a small statically
/// allocated space to avoid dynamic allocations if an array of "less-than" a number of items is required</summary>
/// <typeparam name="ElementType"> The type of items stored </typeparam>
/// <typeparam name="static_size"> The amount of statically allocated memory used. If overran, a dynamic allocation takes place.</typeparam>
template<typename ElementType, size_t static_size>
class ArrayOrVector
{
	ElementType _array[static_size];
	ElementType* _ptr;

public:
	/// <summary> Array indexing operator</summary>
	/// <param name="idx"> Array index</param>
	/// <returns> The indexed item </returns>
	ElementType& operator[](size_t idx)
	{
		return _ptr[idx];
	}
	/// <summary> Array indexing operator</summary>
	/// <param name="idx"> Array index</param>
	/// <returns> The indexed item </returns>
	const ElementType& operator[](size_t idx) const
	{
		return _ptr[idx];
	}
	/// <summary> Constructor. Takes the actual, runtime size of the array.</summary>
	/// <param name="num_items_required"> The size required for the array</param>
	ArrayOrVector(size_t num_items_required) : _array{}
	{
		if (num_items_required > static_size)
		{
			_ptr = new ElementType[num_items_required]();
		}
		else
		{
			_ptr = _array;
		}
	}
	/// <summary> Destructor</summary>
	~ArrayOrVector()
	{
		if (_ptr != _array)
		{
			delete[] _ptr;
		}
	}

	/// <summary> Returns the pointer to the actual storage used (whether static or dynamic)</summary>
	/// <returns> The pointer to the actual storage used.</returns>
	ElementType* get()
	{
		return _ptr;
	}
	/// <summary> Returns the pointer to the actual storage used (whether static or dynamic)</summary>
	/// <returns> The pointer to the actual storage used.</returns>
	const ElementType* get() const
	{
		return _ptr;
	}
};
#pragma warning(default : 4351)

namespace pvrvk {

/// <summary>Contains the maximum limitation this framework supports</summary>
namespace FrameworkCaps {
/// <summary>Contains the maximum limitation this framework supports</summary>
enum Enum
{
	MaxColorAttachments = 8, //!< Max Color attachment supported by the framebuffer
	MaxDepthStencilAttachments = 4, //!< Max depth-stencil attachments supported by the renderpass
	MaxInputAttachments = 8, //!< Max input attachment supported by the renderpass
	MaxResolveAttachments = 8, //!< Max resolve attachment supported by the renderpass
	MaxPreserveAttachments = 8, //!< Max preserve attachment supported by the renderpass
	MaxDescriptorSets = 8, //!< Max descriptor sets supported
	MaxDescriptorSetBindings = 4, //!< Max descriptor set bindings supported
	MaxDescriptorDynamicOffsets = 32, //!< Max descriptor set dynamic offsets supported
	MaxScissorRegions = 8, //!< Max scissor regions supported by the pipeline
	MaxViewportRegions = 8, //!< Max viewports regions supported by the pipeline
	MaxScissorViewports = 8, //!< Max scissor-viewports supported by the pipeline
	MaxSwapChains = 4, //!< Max swapchain supported
	MaxDynamicStates = 8, //!< Max Dynamic states supported
	MaxSpecialisationInfos = 7, //!< Max specialisation infos suported by the pipeline
	MaxSpecialisationInfoDataSize = 1024, //!< Max specialisation info data size suported by the pipeline
	MaxSpecialisationMapEntries = 4, //!< Max specialisation map entries suported by the pipeline
	MaxVertexAttributes = 8, //!< Max Vertex attribute supported by the pipeline
	MaxVertexBindings = 8, //!< Max Vertex bindings supported by the pipeline
};
} // namespace FrameworkCaps

/// <summary>INTERNAL. Defines the basic bitwise operators for an enumeration (AND and OR)</summary>
// clang-format off
#define DEFINE_ENUM_OPERATORS(type_) \
inline type_ operator | (type_ lhs, type_ rhs) \
{ \
    return (type_)(static_cast<std::underlying_type<type_>::type>(lhs) | static_cast<std::underlying_type<type_>::type>(rhs)); \
} \
inline void operator |= (type_& lhs, type_ rhs) \
{ \
    lhs = (type_)(static_cast<std::underlying_type<type_>::type>(lhs) | static_cast<std::underlying_type<type_>::type>(rhs)); \
} \
inline type_ operator & (type_ lhs, type_ rhs) \
{ \
    return (type_)(static_cast<std::underlying_type<type_>::type>(lhs) & static_cast<std::underlying_type<type_>::type>(rhs)); \
} \
inline void operator &= (type_& lhs, type_ rhs) \
{ \
    lhs = (type_)(static_cast<std::underlying_type<type_>::type>(lhs) & static_cast<std::underlying_type<type_>::type>(rhs)); \
}
// clang-format on

/// <summary>INTERNAL. Disable the Copy Constructor and the Copy Assignment Operator of the type</summary>
#define DECLARE_NO_COPY_SEMANTICS(TYPE) \
	TYPE(const TYPE&) = delete; \
	const TYPE& operator=(const TYPE&) = delete;

namespace internal {
//!\cond NO_DOXYGEN
/// <summary>Insert an item into its correct place in a sorted container, maintaining the sort,
/// using binary search. Overwrite if exists.</summary>
/// <typeparam name="container">The type of the container into which to insert</typeparam>
/// <typeparam name="val">The type of the value to insert</typeparam>
/// <typeparam name="cmp">The type of a comparison function to use for the sorting</typeparam>
/// <param name="cont">Container to insert the element into.</param>
/// <param name="begin">Container range begin</param>
/// <param name="end">Container range end</param>
/// <param name="item">Item to insert in to the container</param>
/// <returns>The index where the item was inserted</returns>
template<typename container, typename val, typename cmp>
size_t insertSorted_overwrite(container& cont, typename container::iterator begin, typename container::iterator end, const val& item, const cmp& compare)
{
	typename container::iterator it = std::lower_bound(begin, end, item, compare);
	int64_t offset = static_cast<int64_t>(it - begin);
	if (it != end && !(compare(*it, item) || compare(item, *it)))
	{
		*it = item;
	}
	else
	{
		cont.insert(it, item);
	}
	return (size_t)offset;
}

/// <summary>Insert an item into its correct place in a sorted container, maintaining the sort,
/// using binary search. Overwrite if exists.</summary>
/// <typeparam name="container">The type of the container into which to insert</typeparam>
/// <typeparam name="val">The type of the value to insert</typeparam>
/// <param name="cont">Container to insert the element into.</param>
/// <param name="begin">Container range begin</param>
/// <param name="end">Container range end</param>
/// <param name="item">Item to insert in to the container</param>
/// <returns>The index where the item was inserted</returns>
template<typename container, typename val>
size_t insertSorted_overwrite(container& cont, typename container::iterator begin, typename container::iterator end, const val& item)
{
	return insertSorted_overwrite(cont, begin, end, item, std::less<val>());
}

/// <summary>Insert an item into its correct place in a sorted container, maintaining the sort,
/// using binary search. Overwrite if exists.</summary>
/// <typeparam name="container">The type of the container into which to insert</typeparam>
/// <typeparam name="val">The type of the value to insert</typeparam>
/// <param name="cont">Container to insert the element into.</param>
/// <param name="item">Item to insert in to the container</param>
/// <returns>The index where the item was inserted</returns>
template<typename container, typename val>
size_t insertSorted_overwrite(container& cont, const val& item)
{
	return insertSorted_overwrite(cont, cont.begin(), cont.end(), item);
}

/// <summary>Insert an item into its correct place in a sorted container, maintaining the sort,
/// using binary search. Overwrite if exists.</summary>
/// <typeparam name="container">The type of the container into which to insert</typeparam>
/// <typeparam name="val">The type of the value to insert</typeparam>
/// <param name="cont">Container to insert the element into.</param>
/// <param name="item">Item to insert in to the container</param>
/// <param name="compare">Comparison operator used for sorting</param>
/// <returns>The index where the item was inserted</returns>
template<typename container, typename val, typename cmp>
size_t insertSorted_overwrite(container& cont, const val& item, const cmp& compare)
{
	return insertSorted_overwrite(cont, cont.begin(), cont.end(), item, compare);
}
//!\endcond
} // namespace internal

const uint32_t SubpassExternal = ~0u; //!< A special constant used as a subpass externl in Subpass dependecies

namespace GpuDatatypesHelper {
/// <summary> A bit representing if a type is basically of integer or floating point format </summary>
enum class BaseType
{
	Integer = 0,
	Float = 1
};

/// <summary> Two bits, representing the number of vector components (from scalar up to 4)</summary>
enum class VectorWidth
{
	Scalar = 0,
	Vec2 = 1,
	Vec3 = 2,
	Vec4 = 3,
};

/// <summary> Three bits, representing the number of matrix columns (from not a matrix to 4)</summary>
enum class MatrixColumns
{
	OneCol = 0,
	Mat2x = 1,
	Mat3x = 2,
	Mat4x = 3
};

/// <summary> Contains bit enums for the expressiveness of the GpuDatatypes class' definition</summary>
// clang-format off
enum class Bits : uint64_t
{
	Integer = 0, Float = 1,
	BitScalar = 0, BitVec2 = 2, BitVec3 = 4, BitVec4 = 6,
	BitOneCol = 0, BitMat2x = 8, BitMat3x = 16, BitMat4x = 24,
	ShiftType = 0, MaskType = 1, NotMaskType = static_cast<uint64_t>(~MaskType),
	ShiftVec = 1, MaskVec = (3 << ShiftVec), NotMaskVec = static_cast<uint64_t>(~MaskVec),
	ShiftCols = 3, MaskCols = (3 << ShiftCols), NotMaskCols = static_cast<uint64_t>(~MaskCols)
};
// clang-format on
DEFINE_ENUM_OPERATORS(Bits)
} // namespace GpuDatatypesHelper

/// <summary>A (normally hardware-supported) GPU datatype (e.g. vec4 etc.)</summary>
// clang-format off
enum class GpuDatatypes : uint64_t
{
	Integer = static_cast<uint64_t>(GpuDatatypesHelper::Bits::Integer) | static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitScalar) |
	static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitOneCol), uinteger = Integer, boolean = Integer,
	Float = static_cast<uint64_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitScalar) |
	static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitOneCol),
	ivec2 = static_cast<uint64_t>(GpuDatatypesHelper::Bits::Integer) | static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitVec2) |
	static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitOneCol), uvec2 = ivec2, bvec2 = ivec2,
	ivec3 = static_cast<uint64_t>(GpuDatatypesHelper::Bits::Integer) | static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitVec3) |
	static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitOneCol), uvec3 = ivec3, bvec3 = ivec3,
	ivec4 = static_cast<uint64_t>(GpuDatatypesHelper::Bits::Integer) | static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitVec4) |
	static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitOneCol), uvec4 = ivec4, bvec4 = ivec4,
	vec2 = static_cast<uint64_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitVec2) |
	static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitOneCol),
	vec3 = static_cast<uint64_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitVec3) |
	static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitOneCol),
	vec4 = static_cast<uint64_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitVec4) |
	static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitOneCol),
	mat2x2 = static_cast<uint64_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitVec2) |
	static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitMat2x),
	mat2x3 = static_cast<uint64_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitVec3) |
	static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitMat2x),
	mat2x4 = static_cast<uint64_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitVec4) |
	static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitMat2x),
	mat3x2 = static_cast<uint64_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitVec2) |
	static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitMat3x),
	mat3x3 = static_cast<uint64_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitVec3) |
	static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitMat3x),
	mat3x4 = static_cast<uint64_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitVec4) |
	static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitMat3x),
	mat4x2 = static_cast<uint64_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitVec2) |
	static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitMat4x),
	mat4x3 = static_cast<uint64_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitVec3) |
	static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitMat4x),
	mat4x4 = static_cast<uint64_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitVec4) |
	static_cast<uint64_t>(GpuDatatypesHelper::Bits::BitMat4x),
	none = 0xFFFFFFFF,
	structure = none
};
// clang-format on

/// <summary>An enumeration that defines data types used throughout the Framework. Commonly used in places where
/// raw data are used to define the types actually contained.</summary>
enum class DataType
{
	None, //!< none
	Float32, //!< float 1
	Int32, //!< Integer 2
	UInt16, //!< unsigned short 3
	RGBA, //!< rgba 4
	ARGB, //!< argb 5
	D3DCOLOR, //!< d3d color 6
	UBYTE4, //!< unsigned 4 char 7
	DEC3N,
	Fixed16_16,
	UInt8, //!< unsigned char 10
	Int16, //!< short 11
	Int16Norm, //!< short normalized 12
	Int8, //!< char 13
	Int8Norm, //!< char normalized 14
	UInt8Norm, //!< unsigned char normalized 15
	UInt16Norm, //!< unsigned short normalized
	UInt32, //!< unsigned int
	ABGR, //!< abgr
	Float16, //!< Half float
	Custom = 1000
};

/// <summary>Floating point Color data (rgba). Values from 0-1 inclusive</summary>
struct Color
{
private:
	float color[4]; //!< rgba

public:
	/// <summary>Constructor, default r:0, g:0, b:0, a:1</summary>
	/// <param name="r">Red</param>
	/// <param name="g">Green</param>
	/// <param name="b">Blue</param>
	/// <param name="a">Alpha</param>
	Color(float r = 0.f, float g = 0.f, float b = 0.f, float a = 1.f)
	{
		color[0] = r;
		color[1] = g;
		color[2] = b;
		color[3] = a;
	}

	/// <summary>Constructor. Initialize with rgba values</summary>
	/// <param name="rgba">Pointer to rgba values</param>
	Color(float rgba[4])
	{
		color[0] = rgba[0];
		color[1] = rgba[1];
		color[2] = rgba[2];
		color[3] = rgba[3];
	}

	/// <summary>Get the red component (const)</summary>
	/// <returns>The red component</returns>
	float getR() const
	{
		return color[0];
	}

	/// <summary>Get green component (const)</summary>
	/// <returns>Green component</returns>
	float getG() const
	{
		return color[1];
	}

	/// <summary>Get blue component (const)</summary>
	/// <returns>Blue component</returns>
	float getB() const
	{
		return color[2];
	}

	/// <summary>Get alpha component (const)</summary>
	/// <returns>Alpha component</returns>
	float getA() const
	{
		return color[3];
	}
};

/// <summary>Contains clear color values (rgba).
/// This is used in Commandbuffer::clearColorImage
/// </summary>
struct ClearColorValue
{
private:
	VkClearColorValue color; //!< Vulkan clear color value

public:
	/// <summary>Constructor, Initialise with default r:0, g:0, b:0, a:1</summary>
	ClearColorValue()
	{
		color.float32[0] = color.float32[1] = color.float32[2] = 0;
		color.float32[3] = 1;
	}

	/// <summary>Constructor, Intialise with rgba floating point</summary>
	/// <param name="r">red component</param>
	/// <param name="g">green component</param>
	/// <param name="b">blue component</param>
	/// <param name="a">alpha component</param>
	ClearColorValue(float r, float g, float b, float a)
	{
		color.float32[0] = r;
		color.float32[1] = g;
		color.float32[2] = b;
		color.float32[3] = a;
	}

	/// <summary>Constructor, initialise with interger rgba values</summary>
	/// <param name="r">red component</param>
	/// <param name="g">green component</param>
	/// <param name="b">blue component</param>
	/// <param name="a">alpha component</param>
	ClearColorValue(int32_t r, int32_t g, int32_t b, int32_t a)
	{
		color.int32[0] = r;
		color.int32[1] = g;
		color.int32[2] = b;
		color.int32[3] = a;
	}

	/// <summary>Constructor, initialise with unsigned interger rgba values</summary>
	/// <param name="r">red component</param>
	/// <param name="g">green component</param>
	/// <param name="b">blue component</param>
	/// <param name="a">alpha component</param>
	ClearColorValue(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
	{
		color.uint32[0] = r;
		color.uint32[1] = g;
		color.uint32[2] = b;
		color.uint32[3] = a;
	}

	/// <summary>Get red floating point component</summary>
	/// <returns>Red component</returns>
	float getR() const
	{
		return color.float32[0];
	}

	/// <summary>Set red floating point component</summary>
	/// <param name="r">Red component</param>
	void setR(const float r)
	{
		this->color.float32[0] = r;
	}

	/// <summary>Get green floating point component</summary>
	/// <returns>Green component</returns>
	float getG() const
	{
		return color.float32[1];
	}

	/// <summary>Set green floating point component</summary>
	/// <param name="g">green component</param>
	void setG(const float g)
	{
		this->color.float32[1] = g;
	}

	/// <summary>Get blue floating point component</summary>
	/// <returns>Blue component</returns>
	float getB() const
	{
		return color.float32[2];
	}

	/// <summary>Set blue floating point component</summary>
	/// <param name="b">Blue component</param>
	void setB(const float b)
	{
		this->color.float32[2] = b;
	}

	/// <summary>Get alpha floating point component</summary>
	/// <returns>Alpha component</returns>
	float getA() const
	{
		return color.float32[3];
	}

	/// <summary>Set alpha floating point component</summary>
	/// <param name="a">Alpha component</param>
	void setA(const float a)
	{
		this->color.float32[3] = a;
	}

	/// <summary>Get red interger component</summary>
	/// <returns>Red component</returns>
	int32_t getRInt() const
	{
		return color.int32[0];
	}

	/// <summary>Set red integer point component</summary>
	/// <param name="r">Red component</param>
	void setR(const uint32_t r)
	{
		this->color.int32[0] = r;
	}

	/// <summary>Get green interger component</summary>
	/// <returns>Green component</returns>
	int32_t getGInt() const
	{
		return color.int32[1];
	}

	/// <summary>Set green integer point component</summary>
	/// <param name="g">Green component</param>
	void setG(const uint32_t g)
	{
		this->color.int32[1] = g;
	}

	/// <summary>Get blue interger component</summary>
	/// <returns>Blue component</returns>
	int32_t getBInt() const
	{
		return color.int32[2];
	}

	/// <summary>Set blue integer point component</summary>
	/// <param name="b">Blue component</param>
	void setB(const uint32_t b)
	{
		this->color.int32[2] = b;
	}

	/// <summary>Get alpha interger component</summary>
	/// <returns>alpha component</returns>
	int32_t getAInt() const
	{
		return color.int32[3];
	}

	/// <summary>Set alpha integer point component</summary>
	/// <param name="a">alpha component</param>
	void setA(const uint32_t a)
	{
		this->color.int32[3] = a;
	}

	/// <summary>Get red unsiged interger component</summary>
	/// <returns>Red component</returns>
	uint32_t getRUint() const
	{
		return color.uint32[0];
	}

	/// <summary>Set red unsiged integer point component</summary>
	/// <param name="r">Red component</param>
	void setRUint(const uint32_t r)
	{
		this->color.uint32[0] = r;
	}

	/// <summary>Get green unsiged interger component</summary>
	/// <returns>Green component</returns>
	uint32_t getGUint() const
	{
		return color.uint32[1];
	}

	/// <summary>Set green unsiged integer point component</summary>
	/// <param name="g">Green component</param>
	void setGUint(const uint32_t g)
	{
		this->color.uint32[1] = g;
	}

	/// <summary>Get blue unsiged interger component</summary>
	/// <returns>Blue component</returns>
	uint32_t getBUint() const
	{
		return color.uint32[2];
	}

	/// <summary>Set blue unsiged integer point component</summary>
	/// <param name="b">Blue component</param>
	void setBUint(const uint32_t b)
	{
		this->color.uint32[2] = b;
	}

	/// <summary>Get alpha unsiged interger component</summary>
	/// <returns>Alpha component</returns>
	uint32_t getAUint() const
	{
		return color.uint32[3];
	}

	/// <summary>Set alpha unsiged integer point component</summary>
	/// <param name="a">Alpha component</param>
	void setAUint(const uint32_t a)
	{
		this->color.uint32[3] = a;
	}

	/// <summary>Get color clear value</summary>
	/// <returns>VkClearColorValue</returns>
	const VkClearColorValue& getColor() const
	{
		return color;
	}
};

/// <summary>Represents the geometric size (number of rows/columns/slices in x-y-z) and number of array and mipmap layers
/// of an image.</summary>
struct ImageLayersSize
{
private:
	uint32_t numArrayLevels; //!< Number of array levels
	uint32_t numMipLevels; //!< Number of mip levels

public:
	ImageLayersSize() : numArrayLevels(1), numMipLevels(1) {}

	/// <summary>Constructor (takes individual values)</summary>
	/// <param name="numArrayLevels">Number of array levels</param>
	/// <param name="numMipLevels">Number of mip levels</param>
	ImageLayersSize(uint32_t numArrayLevels, uint32_t numMipLevels) : numArrayLevels(numArrayLevels), numMipLevels(numMipLevels) {}

	/// <summary>Get the number of array levels</summary>
	/// <returns>Num array levels</returns>
	inline uint32_t getNumArrayLevels() const
	{
		return numArrayLevels;
	}

	/// <summary>Set Num array levels</summary>
	/// <param name="numArrayLevels">Number of array levels</param>
	inline void setNumArrayLevels(const uint32_t& numArrayLevels)
	{
		this->numArrayLevels = numArrayLevels;
	}

	/// <summary>Get num mip levels</summary>
	/// <returns>Number of mipmap levels</returns>
	inline uint32_t getNumMipLevels() const
	{
		return numMipLevels;
	}

	/// <summary>Set the number of mip map levels</summary>
	/// <param name="numMipLevels">Num mip map levels</param>
	inline void setNumMipLevels(const uint32_t& numMipLevels)
	{
		this->numMipLevels = numMipLevels;
	}
};

/// <summary>Represents the geometric size (number of rows/columns/slices in x-y-z) and number of array and mipmap layers
/// of an image.</summary>
struct ImageAreaSize : public Extent3D, public ImageLayersSize
{
	/// <summary>Constructor. Initialise with default values</summary>
	ImageAreaSize() {}

	/// <summary>Constructor. Initialise with layer size and extent</summary>
	/// <param name="layersSize">Image layer size</param>
	/// <param name="extents">Image extent in 3D</param>
	ImageAreaSize(const ImageLayersSize& layersSize, const Extent3D& extents) : Extent3D(extents), ImageLayersSize(layersSize) {}

	/// <summary>Constructor. Initialise with layer size and extent</summary>
	/// <param name="layersSize">Image layer size</param>
	/// <param name="extents">Image extent in 3D</param>
	ImageAreaSize(const ImageLayersSize& layersSize, const Extent2D& extents) : Extent3D(extents.getWidth(), extents.getHeight(), 1), ImageLayersSize(layersSize) {}
};

/// <summary>Represents the geometric offset (distance from 0,0,0 in x-y-z) and offset of array and mipmap layers of an
/// image.</summary>
struct ImageAreaOffset : public ImageSubresource, public Offset3D
{
	/// <summary>Constructor. Initialise with default values</summary>
	ImageAreaOffset() {}

	/// <summary>Constructor</summary>
	/// <param name="baseLayers">Image sub resource</param>
	/// <param name="offset">3-dimensional offset</param>
	ImageAreaOffset(const ImageSubresource& baseLayers, const Offset3D& offset) : ImageSubresource(baseLayers), Offset3D(offset) {}
};

/// <summary>2 dimensional offset which contains the x and y</summary>
struct Offset2Df
{
private:
	float x; //!< offset x
	float y; //!< offset y

public:
	/// <summary>Default Constructor</summary>
	Offset2Df()
	{
		setX(float());
		setY(float());
	}
	/// <summary>Constructor</summary>
	/// <param name="x">An offset x value</param>
	/// <param name="y">An offset y value</param>
	Offset2Df(float x, float y)
	{
		setX(x);
		setY(y);
	}
	/// <summary>Get the offset x</summary>
	/// <returns>The offset x</returns>
	inline float getX() const
	{
		return x;
	}
	/// <summary>Sets the offset x</summary>
	/// <param name="x">The new offset x</param>
	inline void setX(float x)
	{
		this->x = x;
	}
	/// <summary>Get the offset y</summary>
	/// <returns>The offset y</returns>
	inline float getY() const
	{
		return y;
	}
	/// <summary>Sets the offset y</summary>
	/// <param name="y">The new offset y</param>
	inline void setY(float y)
	{
		this->y = y;
	}
};

/// <summary>2 dimensional extent which contains the width and height</summary>
struct Extent2Df
{
private:
	float width; //!< extent width
	float height; //!< extent height

public:
	/// <summary>default Constructor</summary>
	Extent2Df()
	{
		setWidth(float());
		setHeight(float());
	}
	/// <summary>Constructor</summary>
	/// <param name="width">An extent width</param>
	/// <param name="height">An extent height</param>
	Extent2Df(float width, float height)
	{
		setWidth(width);
		setHeight(height);
	}
	/// <summary>Get the extent width</summary>
	/// <returns>The extent width</returns>
	inline float getWidth() const
	{
		return width;
	}
	/// <summary>Sets the extent width</summary>
	/// <param name="width">An extent width</param>
	inline void setWidth(float width)
	{
		this->width = width;
	}
	/// <summary>Get the extent height</summary>
	/// <returns>The extent height</returns>
	inline float getHeight() const
	{
		return height;
	}
	/// <summary>Sets the extent height</summary>
	/// <param name="height">An extent height</param>
	inline void setHeight(float height)
	{
		this->height = height;
	}
};

/// <summary>2-dimensional floating point rectangle</summary>
struct Rect2Df
{
private:
	Offset2Df offset; //!< offset x and y
	Extent2Df extent; //!< Width and height relative to offset.

public:
	/// <summary>Constructor, takes individual values</summary>
	/// <param name="myx">offset x</param>
	/// <param name="myy">offset y</param>
	/// <param name="mywidth">extent width</param>
	/// <param name="myheight">extent height</param>
	Rect2Df(float myx, float myy, float mywidth, float myheight) : offset(myx, myy), extent(mywidth, myheight) {}

	/// <summary>Constructor, sets to offset(0,0) and extent(1,1)</summary>
	Rect2Df() : offset(0.0f, 0.0f), extent(1.0f, 1.0f) {}

	/// <summary>Get Offset</summary>
	/// <returns>Offset</returns>
	inline const Offset2Df& getOffset() const
	{
		return offset;
	}

	/// <summary>Set Offset</summary>
	/// <param name="offset">Offset</param>
	inline void setOffset(const Offset2Df& offset)
	{
		this->offset = offset;
	}

	/// <summary>Get Extent</summary>
	/// <returns>Extent</returns>
	inline const Extent2Df& getExtent() const
	{
		return extent;
	}

	/// <summary>Set Extent</summary>
	/// <param name="extent">Extent</param>
	inline void setExtent(const Extent2Df& extent)
	{
		this->extent = extent;
	}
};

/// <summary>Containes application info used for creating vulkan instance</summary>
struct ApplicationInfo
{
private:
	std::string applicationName; //!< NULL or is a pointer to a null-terminated UTF-8 string containing the name of the application
	uint32_t applicationVersion; //!< Unsigned integer variable containing the developer-supplied version number of the application
	std::string engineName; //!< NULL or is a pointer to a null-terminated UTF-8 string containing the name of the engine (if any) used to create the application
	uint32_t engineVersion; //!< Unsigned integer variable containing the developer-supplied version number of the engine used to create the application.
	uint32_t apiVersion; //!< version of the Vulkan API against which the application expects to run. If apiVersion is 0 the implementation ignore it.

public:
	/// <summary>Constructor</summary>
	ApplicationInfo() : applicationVersion(0), engineVersion(0) {}

	/// <summary>Constructor</summary>
	/// <param name="applicationName">The name of the application</param>
	/// <param name="applicationVersion">The version of the application</param>
	/// <param name="engineName">The name of the engine</param>
	/// <param name="engineVersion">The version of the engine</param>
	/// <param name="apiVersion">The api version to be used by the application</param>
	ApplicationInfo(const std::string applicationName, uint32_t applicationVersion = 0, const std::string engineName = 0, uint32_t engineVersion = 0, uint32_t apiVersion = 0)
	{
		setApplicationName(applicationName);
		setApplicationVersion(applicationVersion);
		setEngineName(engineName);
		setEngineVersion(engineVersion);
		setApiVersion(apiVersion);
	}

	/// <summary>Get the Application name</summary>
	/// <returns>The application name</returns>
	inline const std::string& getApplicationName() const
	{
		return applicationName;
	}
	/// <summary>Sets the application name</summary>
	/// <param name="applicationName">The application name to use</param>
	inline void setApplicationName(const std::string& applicationName)
	{
		this->applicationName = applicationName;
	}
	/// <summary>Get the Application version</summary>
	/// <returns>The application version</returns>
	inline uint32_t getApplicationVersion() const
	{
		return applicationVersion;
	}
	/// <summary>Sets the application version</summary>
	/// <param name="applicationVersion">The application version to use</param>
	inline void setApplicationVersion(uint32_t applicationVersion)
	{
		this->applicationVersion = applicationVersion;
	}
	/// <summary>Get the Engine name</summary>
	/// <returns>The engine name</returns>
	inline const std::string& getEngineName() const
	{
		return engineName;
	}
	/// <summary>Sets the engine name</summary>
	/// <param name="engineName">The engine name to use</param>
	inline void setEngineName(const std::string& engineName)
	{
		this->engineName = engineName;
	}
	/// <summary>Get the Engine version</summary>
	/// <returns>The engine version</returns>
	inline uint32_t getEngineVersion() const
	{
		return engineVersion;
	}
	/// <summary>Sets the engine version</summary>
	/// <param name="engineVersion">The engine version to use</param>
	inline void setEngineVersion(uint32_t engineVersion)
	{
		this->engineVersion = engineVersion;
	}
	/// <summary>Get the api version</summary>
	/// <returns>The api version</returns>
	inline uint32_t getApiVersion() const
	{
		return apiVersion;
	}
	/// <summary>Sets the api version</summary>
	/// <param name="apiVersion">The api version to use</param>
	inline void setApiVersion(uint32_t apiVersion)
	{
		this->apiVersion = apiVersion;
	}
};

/// <summary>Contains instant info used for creating Vulkan instance</summary>
struct InstanceCreateInfo
{
private:
	InstanceCreateFlags flags; //!< Reserved for future use
	const ApplicationInfo* applicationInfo; //!< NULL or a pointer to an instance of ApplicationInfo. If not NULL, this information helps implementations recognize behavior
											//!< inherent to classes of applications.
	std::vector<std::string> enabledLayerNames; //!<  Array of null-terminated UTF-8 strings containing the names of layers to enable for the created instance
	std::vector<std::string> enabledExtensionNames; //!<  Array of null-terminated UTF-8 strings containing the names of extensions to enable for the created instance

public:
	/// <summary>Constructor. Default initialised to 0</summary>
	InstanceCreateInfo() : flags(InstanceCreateFlags(0)), applicationInfo(nullptr) {}

	/// <summary>Constructor</summary>
	/// <param name="applicationInfo">A pointer to an application info structure</param>
	/// <param name="flags">A set of InstanceCreateFlags to use</param>
	InstanceCreateInfo(const ApplicationInfo* applicationInfo, InstanceCreateFlags flags = InstanceCreateFlags::e_NONE)
		: flags(InstanceCreateFlags(flags)), applicationInfo(applicationInfo)
	{}

	/// <summary>Get the instance creation flags</summary>
	/// <returns>The instance creation flags</returns>
	inline const InstanceCreateFlags& getFlags() const
	{
		return flags;
	}
	/// <summary>Sets the instance creation flags</summary>
	/// <param name="flags">A set of InstanceCreateFlags to use</param>
	inline void setFlags(const InstanceCreateFlags& flags)
	{
		this->flags = flags;
	}
	/// <summary>Get the instance application info</summary>
	/// <returns>The instance application info</returns>
	inline const ApplicationInfo* getApplicationInfo() const
	{
		return applicationInfo;
	}
	/// <summary>Sets the application info pointer</summary>
	/// <param name="applicationInfo">A pointer to a new application info structure</param>
	inline void setApplicationInfo(const ApplicationInfo* applicationInfo)
	{
		this->applicationInfo = applicationInfo;
	}
	/// <summary>Get the number of enabled instance layers</summary>
	/// <returns>The number of enabled instance layers</returns>
	inline uint32_t getNumEnabledLayerNames() const
	{
		return static_cast<uint32_t>(enabledLayerNames.size());
	}
	/// <summary>Get the enabled instance layers</summary>
	/// <returns>The enabled instance layers</returns>
	inline const std::vector<std::string>& getEnabledLayerNames() const
	{
		return enabledLayerNames;
	}
	/// <summary>Get the enabled instance layer at the index specified</summary>
	/// <param name="index">The index of the enabled instance layer to retrieve</param>
	/// <returns>The enabled instance layer</returns>
	inline const std::string& getEnabledLayerName(uint32_t index) const
	{
		return enabledLayerNames[index];
	}
	/// <summary>Sets the enabled instance layers list</summary>
	/// <param name="enabledLayerNames">A list of instance layers to enable</param>
	inline void setEnabledLayers(const std::vector<std::string>& enabledLayerNames)
	{
		this->enabledLayerNames.resize(enabledLayerNames.size());
		std::copy(enabledLayerNames.begin(), enabledLayerNames.end(), this->enabledLayerNames.begin());
	}
	/// <summary>Adds a new instance layers to the list of layers to enable</summary>
	/// <param name="layer">A new instance layer to add to the existing list of instance layers to enable</param>
	inline void addEnabledLayer(const std::string& layer)
	{
		this->enabledLayerNames.push_back(layer);
	}
	/// <summary>Removes a particular instance layers from the list of layers to enable</summary>
	/// <param name="layer">The instance layer to remove from the existing list of instance layers to enable</param>
	inline void removeEnabledLayer(const std::string& layer)
	{
		this->enabledLayerNames.erase(std::remove(this->enabledLayerNames.begin(), this->enabledLayerNames.end(), layer), this->enabledLayerNames.end());
	}
	/// <summary>Get the number of enabled instance extensions</summary>
	/// <returns>The enabled instance extensions</returns>
	inline uint32_t getNumEnabledExtensionNames() const
	{
		return static_cast<uint32_t>(enabledExtensionNames.size());
	}
	/// <summary>Get the list of enabled instance extensions</summary>
	/// <returns>A list of enabled instance extensions</returns>
	inline const std::vector<std::string>& getEnabledExtensionNames() const
	{
		return enabledExtensionNames;
	}
	/// <summary>Retrieve the enabled instance extension at the index specified</summary>
	/// <param name="index">The index of the instance extension to retrieve</param>
	/// <returns>The enabled instance extension at the index specified</returns>
	inline const std::string& getEnabledExtensionName(uint32_t index) const
	{
		return enabledExtensionNames[index];
	}
	/// <summary>Sets the enabled instance extensions list</summary>
	/// <param name="enabledExtensionNames">A list of instance extensions to enable</param>
	inline void setEnabledExtensions(const std::vector<std::string>& enabledExtensionNames)
	{
		this->enabledExtensionNames.resize(enabledExtensionNames.size());
		std::copy(enabledExtensionNames.begin(), enabledExtensionNames.end(), this->enabledExtensionNames.begin());
	}
	/// <summary>Adds a new instance extension to the list of extensions to enable</summary>
	/// <param name="extensionName">A new instance extension to add to the existing list of instance extensions to enable</param>
	inline void addEnabledExtension(const std::string& extensionName)
	{
		this->enabledExtensionNames.push_back(extensionName);
	}
	/// <summary>Removes a particular instance extension from the list of extensions to enable</summary>
	/// <param name="extensionName">The instance extension to remove from the existing list of instance extensions to enable</param>
	inline void removeEnabledExtension(const std::string& extensionName)
	{
		this->enabledExtensionNames.erase(std::remove(this->enabledExtensionNames.begin(), this->enabledExtensionNames.end(), extensionName), this->enabledExtensionNames.end());
	}
};

/// <summary>Contains information about the queues to create for a single queue family. A set of DeviceQueueCreateInfo structures for each queue families are passed in to
/// DeviceCreateInfo when creating the device.</summary>
struct DeviceQueueCreateInfo
{
private:
	uint32_t queueFamilyIndex; //!< Queue family index the queue will be created from
	std::vector<float> queuePriorities; //!< Array of queueCount normalized floating point values, specifying priorities of work that will be submitted to each created queue

public:
	/// <summary>Constructor</summary>
	DeviceQueueCreateInfo()
	{
		setQueueFamilyIndex(static_cast<uint32_t>(-1));
	}
	/// <summary>Constructor. Each of queueCount queues will have a default priority of 1.0f</summary>
	/// <param name="queueFamilyIndex">The queue family index</param>
	/// <param name="queueCount">The number of queues to create in the particular family type</param>
	DeviceQueueCreateInfo(uint32_t queueFamilyIndex, uint32_t queueCount)
	{
		setQueueFamilyIndex(queueFamilyIndex);
		for (uint32_t i = 0; i < queueCount; i++)
		{
			addQueue();
		}
	}

	/// <summary>Constructor</summary>
	/// <param name="queueFamilyIndex">The queue family index</param>
	/// <param name="queuePriorities">A list of queue priorities specifying both the number of queues to create in the particular family type and their priority</param>
	DeviceQueueCreateInfo(uint32_t queueFamilyIndex, const std::vector<float>& queuePriorities)
	{
		setQueueFamilyIndex(queueFamilyIndex);
		this->queuePriorities.resize(queuePriorities.size());
		std::copy(queuePriorities.begin(), queuePriorities.end(), this->queuePriorities.begin());
	}

	/// <summary>Retrieve the queue family index</summary>
	/// <returns>The queue family index</returns>
	inline uint32_t getQueueFamilyIndex() const
	{
		return queueFamilyIndex;
	}
	/// <summary>Sets the queue family index</summary>
	/// <param name="queueFamilyIndex">The queue family index</param>
	inline void setQueueFamilyIndex(uint32_t queueFamilyIndex)
	{
		this->queueFamilyIndex = queueFamilyIndex;
	}
	/// <summary>Retrieve the number of queues created in the queue family index</summary>
	/// <returns>The number of queues in the particular queue family index</returns>
	inline const uint32_t getNumQueues() const
	{
		return static_cast<uint32_t>(queuePriorities.size());
	}
	/// <summary>Retrieve the queues created in the queue family index (const)</summary>
	/// <returns>The queues in the particular queue family index (const)</returns>
	inline const std::vector<float>& getQueuePriorities() const
	{
		return queuePriorities;
	}
	/// <summary>Retrieve the queues created in the queue family index</summary>
	/// <returns>The queues in the particular queue family index</returns>
	inline std::vector<float>& getQueuePriorities()
	{
		return queuePriorities;
	}
	/// <summary>Retrieve the queue priority of the queue at the specified index</summary>
	/// <param name="index">The index of the queue priority to get</param>
	/// <returns>The queue priority of the queue at the specified index</returns>
	inline float getQueuePriority(uint32_t index) const
	{
		return queuePriorities[index];
	}
	/// <summary>Adds a queue with a particular priority</summary>
	/// <param name="priority">The queue priority</param>
	inline void addQueue(float priority = 1.0f)
	{
		queuePriorities.push_back(priority);
	}
	/// <summary>Sets a queues priority</summary>
	/// <param name="index">The index of queue priority to set</param>
	/// <param name="priority">The new queue priority</param>
	inline void addQueueAtIndex(uint32_t index, float priority = 1.0f)
	{
		queuePriorities[index] = priority;
	}
	/// <summary>Clears all the queue priorities</summary>
	inline void clearQueues()
	{
		queuePriorities.clear();
	}
};

/// <summary>Containes device create info.</summary>
struct DeviceCreateInfo
{
private:
	DeviceCreateFlags flags; //!< Reserved for future use
	std::vector<DeviceQueueCreateInfo> queueCreateInfos; //!< Pointer to an array of DeviceQueueCreateInfo structures describing the queues that are requested to be created along with the logical device
	std::vector<std::string> enabledExtensionNames; //!< Array of extensions to enable
	const PhysicalDeviceFeatures* enabledFeatures; //!< NULL or a pointer to a PhysicalDeviceFeatures structure that contains boolean indicators of all the features to be enabled

public:
	/// <summary>Constructor. Default initialised to 0</summary>
	DeviceCreateInfo() : flags(DeviceCreateFlags(0)), enabledFeatures(nullptr) {}

	/// <summary>Get the device creation flags</summary>
	/// <returns>A set of DeviceCreateFlags</returns>
	inline const DeviceCreateFlags& getFlags() const
	{
		return flags;
	}
	/// <summary>Sets the device creation info flags</summary>
	/// <param name="flags">A set of DeviceCreateFlags specifying how the device will be created.</param>
	inline void setFlags(const DeviceCreateFlags& flags)
	{
		this->flags = flags;
	}

	/// <summary>Get the number of queue create info structures</summary>
	/// <returns>The number of device queue create infos</returns>
	inline uint32_t getNumDeviceQueueCreateInfos() const
	{
		return static_cast<uint32_t>(queueCreateInfos.size());
	}
	/// <summary>Get the queue create info structures</summary>
	/// <returns>The device queue create infos</returns>
	inline const std::vector<DeviceQueueCreateInfo>& getDeviceQueueCreateInfos() const
	{
		return queueCreateInfos;
	}
	/// <summary>Get the queue create info structure at the specified index (const)</summary>
	/// <param name="index">The specific index of the queue create info structure to set.</param>
	/// <returns>The device queue create info at the specified index (const)</returns>
	inline const DeviceQueueCreateInfo& getDeviceQueueCreateInfo(uint32_t index) const
	{
		return queueCreateInfos[index];
	}
	/// <summary>Get the queue create info structure at the specified index</summary>
	/// <param name="index">The specific index of the queue create info structure to get.</param>
	/// <returns>The device queue create info at the specified index</returns>
	inline DeviceQueueCreateInfo& getDeviceQueueCreateInfo(uint32_t index)
	{
		return queueCreateInfos[index];
	}
	/// <summary>Sets the device queue creation info structures</summary>
	/// <param name="queueCreateInfos">A list of DeviceQueueCreateInfo specifying the queue family indices and corresponding queues (and priorites) to create.</param>
	inline void setDeviceQueueCreateInfos(const std::vector<DeviceQueueCreateInfo>& queueCreateInfos)
	{
		this->queueCreateInfos.resize(queueCreateInfos.size());
		std::copy(queueCreateInfos.begin(), queueCreateInfos.end(), this->queueCreateInfos.begin());
	}
	/// <summary>Adds a new device queue creation info structure</summary>
	/// <param name="deviceQueueCreateInfo">A DeviceQueueCreateInfo specifying a queue family index and its corresponding queues (and priorites) to create.</param>
	inline void addDeviceQueue(DeviceQueueCreateInfo deviceQueueCreateInfo = DeviceQueueCreateInfo())
	{
		queueCreateInfos.push_back(deviceQueueCreateInfo);
	}
	/// <summary>Adds a new device queue creation info structure at the specified index</summary>
	/// <param name="index">The index of the device queue create info structure to set.</param>
	/// <param name="deviceQueueCreateInfo">A DeviceQueueCreateInfo specifying a queue family index and its corresponding queues (and priorites) to create.</param>
	inline void addDeviceQueueAtIndex(uint32_t index, DeviceQueueCreateInfo deviceQueueCreateInfo = DeviceQueueCreateInfo())
	{
		queueCreateInfos[index] = deviceQueueCreateInfo;
	}
	/// <summary>Clears the device queue create info structures</summary>
	inline void clearDeviceQueueCreateInfos()
	{
		queueCreateInfos.clear();
	}
	/// <summary>Get the number of enabled extension names</summary>
	/// <returns>The number of enabled extension names</returns>
	inline uint32_t getNumEnabledExtensionNames() const
	{
		return static_cast<uint32_t>(enabledExtensionNames.size());
	}
	/// <summary>Get the enabled extension names</summary>
	/// <returns>The enabled extension names</returns>
	inline const std::vector<std::string>& getEnabledExtensionNames() const
	{
		return enabledExtensionNames;
	}
	/// <summary>Get the enabled extension name at the specified index</summary>
	/// <param name="index">The specific index of the extension name to set.</param>
	/// <returns>The enabled extension name at the specified index</returns>
	inline const std::string& getEnabledExtensionName(uint32_t index) const
	{
		return enabledExtensionNames[index];
	}
	/// <summary>Setter for the enabled extension names list</summary>
	/// <param name="enabledExtensionNames">A list of extension names to enable.</param>
	inline void setEnabledExtensions(const std::vector<std::string>& enabledExtensionNames)
	{
		this->enabledExtensionNames.resize(enabledExtensionNames.size());
		std::copy(enabledExtensionNames.begin(), enabledExtensionNames.end(), this->enabledExtensionNames.begin());
	}
	/// <summary>Add a new extension name to enable</summary>
	/// <param name="extensionName">A new extension name to enable.</param>
	inline void addEnabledExtension(const std::string& extensionName)
	{
		this->enabledExtensionNames.push_back(extensionName);
	}
	/// <summary>Removes a particular extension name</summary>
	/// <param name="extensionName">The extension name to remove.</param>
	inline void removeEnabledExtension(const std::string& extensionName)
	{
		this->enabledExtensionNames.erase(std::remove(this->enabledExtensionNames.begin(), this->enabledExtensionNames.end(), extensionName), this->enabledExtensionNames.end());
	}
	/// <summary>Get a pointer to the physical device features structure</summary>
	/// <returns>A pointer to the enabled physical device features</returns>
	inline const PhysicalDeviceFeatures* getEnabledFeatures() const
	{
		return enabledFeatures;
	}
	/// <summary>Sets the enabled physical device features</summary>
	/// <param name="enabledFeatures">A pointer to a set of PhysicalDeviceFeatures.</param>
	inline void setEnabledFeatures(const PhysicalDeviceFeatures* enabledFeatures)
	{
		this->enabledFeatures = enabledFeatures;
	}
};

/// <summary>The ClearValue struct. Color or depth/stencil value to clear the attachment to.</summary>
struct ClearValue
{
private:
	uint8_t bytes[16];

public:
	/// <summary>Constructor. initialise with rgb: 0 and alpha 1 clear values</summary>
	ClearValue()
	{
		float one = 1.f;
		memset(bytes, 0, 12);
		memcpy(bytes + 12, &one, 4);
	}

	/// <summary>Constructor. Initialise with depth stencil clear values</summary>
	/// <param name="depth">Depth clear value</param>
	/// <param name="stencil">Stencil clear value</param>
	ClearValue(float depth, uint32_t stencil)
	{
		setDepthStencilValue(depth, stencil);
	}

	/// <summary>Constructor. Initialise with rgba floating point</summary>
	/// <param name="r">Red component</param>
	/// <param name="g">Green component</param>
	/// <param name="b">Blue component</param>
	/// <param name="a">Alpha component</param>
	ClearValue(float r, float g, float b, float a)
	{
		setColorValue(r, g, b, a);
	}

	/// <summary>Constructor. Initialise with rgba interger</summary>
	/// <param name="r">Red component</param>
	/// <param name="g">Green component</param>
	/// <param name="b">Blue component</param>
	/// <param name="a">Alpha component</param>
	ClearValue(int32_t r, int32_t g, int32_t b, int32_t a)
	{
		memcpy(bytes, &r, sizeof(int32_t));
		memcpy(bytes + sizeof(int32_t), &g, sizeof(int32_t));
		memcpy(bytes + sizeof(int32_t) * 2, &b, sizeof(int32_t));
		memcpy(bytes + sizeof(int32_t) * 3, &a, sizeof(int32_t));
	}

	/// <summary>Constructor. Initialise with rgba unsigned interger</summary>
	/// <param name="r">Red component</param>
	/// <param name="g">Green component</param>
	/// <param name="b">Blue component</param>
	/// <param name="a">Alpha component</param>
	ClearValue(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
	{
		memcpy(bytes, &r, sizeof(uint32_t));
		memcpy(bytes + sizeof(uint32_t), &g, sizeof(uint32_t));
		memcpy(bytes + sizeof(uint32_t) * 2, &b, sizeof(uint32_t));
		memcpy(bytes + sizeof(uint32_t) * 3, &a, sizeof(uint32_t));
	}

	/// <summary>Set rgba color clear value</summary>
	/// <param name="r">Red component</param>
	/// <param name="g">Green component</param>
	/// <param name="b">Blue component</param>
	/// <param name="a">Alpha component</param>
	void setColorValue(float r, float g, float b, float a)
	{
		memcpy(bytes, &r, sizeof(float));
		memcpy(bytes + sizeof(float), &g, sizeof(float));
		memcpy(bytes + sizeof(float) * 2, &b, sizeof(float));
		memcpy(bytes + sizeof(float) * 3, &a, sizeof(float));
	}

	/// <summary>Set depth stencil clear value</summary>
	/// <param name="depth">Depth clear value, default 1</param>
	/// <param name="stencil">Stencil clear value, default 0</param>
	void setDepthStencilValue(float depth = 1.f, uint32_t stencil = 0u)
	{
		memcpy(bytes, &depth, sizeof(float));
		memcpy(bytes + sizeof(float), &stencil, sizeof(uint32_t));
	}

	/// <summary>Get vulkan representation of this object</summary>
	/// <returns>Returns vulkan clear value</returns>
	VkClearValue toVkValue()
	{
		VkClearValue value;
		static_assert(sizeof(value) == sizeof(bytes), "VkClearValue wrong alignment");
		memcpy(&value, bytes, sizeof(value));
		return value;
	}

	/// <summary>Get vulkan representation of this object (const)</summary>
	/// <returns>Returns (const) vulkan clear value</returns>
	const VkClearValue toVkValue() const
	{
		VkClearValue value;
		static_assert(sizeof(value) == sizeof(bytes), "VkClearValue wrong alignment");
		memcpy(&value, bytes, sizeof(value));
		return value;
	}

	/// <summary>Create default depth stencil clear value factory function</summary>
	/// <returns>Returns default depth stencil clear value</returns>
	static ClearValue createDefaultDepthStencilClearValue()
	{
		return ClearValue(1.f, 0u);
	}

	/// <summary>Create stencil clear value factory function</summary>
	/// <param name="stencil">Stencil clear value</param>
	/// <returns>Returns stencil clear value</returns>
	static ClearValue createStencilClearValue(uint32_t stencil)
	{
		return ClearValue(1.f, stencil);
	}

	/// <summary>Create depth-stencil clear value factory function</summary>
	/// <param name="depth">Depth clear value</param>
	/// <param name="stencil">Stencil value</param>
	/// <returns>Returns depth-stencil clear value</returns>
	static ClearValue createDepthStencilClearValue(float depth, uint32_t stencil)
	{
		return ClearValue(depth, stencil);
	}
};

/// <summary>ClearAttachment structures defining the attachments to clear and the clear values to use. Used in Commandbuffer::ClearAttachments command</summary>
struct ClearAttachment : private VkClearAttachment
{
	/// <summary>Constructor. Initialisation undefined.</summary>
	ClearAttachment() {}

	/// <summary>Constructor.</summary>
	/// <param name="aspectMask">Mask selecting the color, depth and/or stencil aspects of the attachment to be cleared.</param>
	/// <param name="colorAttachment">Index of the renderpass color attachment. It is only meaningful if VkImageAspectFlags::e_ASPECT_COLOR_BIT is set in aspectMask</param>
	/// <param name="clearValue">color or depth/stencil value to clear the attachment</param>
	ClearAttachment(ImageAspectFlags aspectMask, uint32_t colorAttachment, const ClearValue& clearValue)
	{
		this->aspectMask = static_cast<VkImageAspectFlags>(aspectMask);
		this->colorAttachment = colorAttachment;
		this->clearValue = clearValue.toVkValue();
	}

	/// <summary>Create stencil-clear-attachment factory function</summary>
	/// <param name="stencil">Stencil clear value</param>
	/// <returns>Returns stencil clear attachment</returns>
	static ClearAttachment createStencilClearAttachment(uint32_t stencil)
	{
		return ClearAttachment(ImageAspectFlags::e_STENCIL_BIT, 0, ClearValue::createStencilClearValue(stencil));
	}

	/// <summary>Create depth-clear-attachment factory function</summary>
	/// <param name="depth">Depth clear value</param>
	/// <param name="stencil">Stencil clear value</param>
	/// <returns>Returns depth-stencil clear attachment</returns>
	static ClearAttachment createDepthStencilClearAttachment(float depth, uint32_t stencil)
	{
		return ClearAttachment(ImageAspectFlags::e_DEPTH_BIT | ImageAspectFlags::e_STENCIL_BIT, 0, ClearValue::createDepthStencilClearValue(depth, stencil));
	}

	/// <summary>Create color-clear-attachmemnt factory function</summary>
	/// <param name="colorAttachment">Index of the renderpass color attachment.</param>
	/// <param name="clearValue">Color clear values</param>
	/// <returns>Returns color clear attachment</returns>
	static ClearAttachment createColorClearAttachment(uint32_t colorAttachment, const ClearValue& clearValue)
	{
		return ClearAttachment(ImageAspectFlags::e_COLOR_BIT, colorAttachment, clearValue);
	}
	/// <summary>Get the attachment aspect masks</summary>
	/// <returns>The attachment aspect mask</returns>
	inline ImageAspectFlags getAspectMask() const
	{
		return reinterpret_cast<const ImageAspectFlags&>(aspectMask);
	}
	/// <summary>Set the attachment aspect masks</summary>
	/// <param name="aspectMask">The attachment aspect mask.</param>
	inline void setAspectMask(const ImageAspectFlags& aspectMask)
	{
		this->aspectMask = *reinterpret_cast<const VkImageAspectFlags*>(&aspectMask);
	}
	/// <summary>Get the attachment color attachment index</summary>
	/// <returns>The attachment color attachment index</returns>
	inline uint32_t getColorAttachment() const
	{
		return colorAttachment;
	}
	/// <summary>Set the attachment color attachment index</summary>
	/// <param name="colorAttachment">The attachment color attachment index.</param>
	inline void setColorAttachment(const uint32_t& colorAttachment)
	{
		this->colorAttachment = colorAttachment;
	}
	/// <summary>Get the attachment clear value</summary>
	/// <returns>The attachment value value</returns>
	inline const ClearValue& getClearValue() const
	{
		return reinterpret_cast<const ClearValue&>(clearValue);
	}
	/// <summary>Set the attachment clear value</summary>
	/// <param name="clearValue">The attachment clear value.</param>
	inline void setClearValue(const ClearValue& clearValue)
	{
		this->clearValue = *reinterpret_cast<const VkClearValue*>(&clearValue);
	}
};

/// <summary>Contains attachment configuration of a renderpass (format, loadop, storeop, samples).
/// </summary>
struct AttachmentDescription : private VkAttachmentDescription
{
	/// <summary>Constructor to undefined layouts/clear/store</summary>
	AttachmentDescription()
	{
		this->flags = static_cast<VkAttachmentDescriptionFlags>(AttachmentDescriptionFlags::e_NONE);
		this->format = static_cast<VkFormat>(pvrvk::Format::e_UNDEFINED);
		this->initialLayout = static_cast<VkImageLayout>(pvrvk::ImageLayout::e_UNDEFINED);
		this->finalLayout = static_cast<VkImageLayout>(pvrvk::ImageLayout::e_UNDEFINED);
		this->loadOp = static_cast<VkAttachmentLoadOp>(pvrvk::AttachmentLoadOp::e_CLEAR);
		this->storeOp = static_cast<VkAttachmentStoreOp>(pvrvk::AttachmentStoreOp::e_STORE);
		this->stencilLoadOp = static_cast<VkAttachmentLoadOp>(pvrvk::AttachmentLoadOp::e_CLEAR);
		this->stencilStoreOp = static_cast<VkAttachmentStoreOp>(pvrvk::AttachmentStoreOp::e_STORE);
		this->samples = static_cast<VkSampleCountFlagBits>(pvrvk::SampleCountFlags::e_1_BIT);
	}

	/// <summary>Constructor</summary>
	/// <param name="format">Attachment format</param>
	/// <param name="loadOp">Color/Depth attachment looad operation. Default is Clear. For performance, prefer Ignore if possible for
	/// your application.</param>
	/// <param name="storeOp">Color/Depth attachment store operator. Default is Store. For performance, prefer Ignore if possible
	/// for your application.</param>
	/// <param name="stencilLoadOp">Stencil load operation. Default is Clear. For performance, prefer Ignore if possible for
	/// your application.</param>
	/// <param name="stencilStoreOp">Stencil store operator. Default is Store. For performance, prefer Ignore if possible
	/// for your application.</param>
	/// <param name="numSamples">Number of samples. Default is 1.</param>
	/// <param name="initialLayout">The initial layout that the output image will be on. Must match the actual layout
	/// of the Image. Default is ColorAttachmentOptimal.</param>
	/// <param name="finalLayout">A layout to transition the image to at the end of this renderpass. Default is
	/// ColorAttachmentOptimal.</param>
	AttachmentDescription(pvrvk::Format format, pvrvk::ImageLayout initialLayout, pvrvk::ImageLayout finalLayout, pvrvk::AttachmentLoadOp loadOp, pvrvk::AttachmentStoreOp storeOp,
		pvrvk::AttachmentLoadOp stencilLoadOp, pvrvk::AttachmentStoreOp stencilStoreOp, pvrvk::SampleCountFlags numSamples)
	{
		this->flags = static_cast<VkAttachmentDescriptionFlags>(AttachmentDescriptionFlags::e_NONE);
		this->format = static_cast<VkFormat>(format);
		this->initialLayout = static_cast<VkImageLayout>(initialLayout);
		this->finalLayout = static_cast<VkImageLayout>(finalLayout);
		this->loadOp = static_cast<VkAttachmentLoadOp>(loadOp);
		this->storeOp = static_cast<VkAttachmentStoreOp>(storeOp);
		this->stencilLoadOp = static_cast<VkAttachmentLoadOp>(stencilLoadOp);
		this->stencilStoreOp = static_cast<VkAttachmentStoreOp>(stencilStoreOp);
		this->samples = static_cast<VkSampleCountFlagBits>(numSamples);
	}

	/// <summary>Create color description</summary>
	/// <param name="format">Color format</param>
	/// <param name="initialLayout">Color initial layout</param>
	/// <param name="finalLayout">Color final layout</param>
	/// <param name="loadOp">Attachment loadop</param>
	/// <param name="storeOp">Attachment storeop</param>
	/// <param name="numSamples">Number of samples</param>
	/// <returns>AttachmentDescription</returns>
	static AttachmentDescription createColorDescription(pvrvk::Format format, pvrvk::ImageLayout initialLayout = pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL,
		pvrvk::ImageLayout finalLayout = pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, pvrvk::AttachmentLoadOp loadOp = pvrvk::AttachmentLoadOp::e_CLEAR,
		pvrvk::AttachmentStoreOp storeOp = pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags numSamples = pvrvk::SampleCountFlags::e_1_BIT)
	{
		return AttachmentDescription(format, initialLayout, finalLayout, loadOp, storeOp, AttachmentLoadOp::e_DONT_CARE, AttachmentStoreOp::e_DONT_CARE, numSamples);
	}

	/// <summary>Create depthstencil description</summary>
	/// <param name="format">Attachment format</param>
	/// <param name="initialLayout">Attachment initial layout</param>
	/// <param name="finalLayout">Attachment final layout</param>
	/// <param name="loadOp">Depth load op.</param>
	/// <param name="storeOp">Depth store op</param>
	/// <param name="stencilLoadOp">Stencil load op </param>
	/// <param name="stencilStoreOp">Stencil store op</param>
	/// <param name="numSamples">Number of samples</param>
	/// <returns>AttachmentDescription</returns>
	static AttachmentDescription createDepthStencilDescription(pvrvk::Format format, pvrvk::ImageLayout initialLayout = pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		pvrvk::ImageLayout finalLayout = pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, pvrvk::AttachmentLoadOp loadOp = pvrvk::AttachmentLoadOp::e_CLEAR,
		pvrvk::AttachmentStoreOp storeOp = pvrvk::AttachmentStoreOp::e_DONT_CARE, pvrvk::AttachmentLoadOp stencilLoadOp = pvrvk::AttachmentLoadOp::e_CLEAR,
		pvrvk::AttachmentStoreOp stencilStoreOp = pvrvk::AttachmentStoreOp::e_DONT_CARE, pvrvk::SampleCountFlags numSamples = pvrvk::SampleCountFlags::e_1_BIT)
	{
		return AttachmentDescription(format, initialLayout, finalLayout, loadOp, storeOp, stencilLoadOp, stencilStoreOp, numSamples);
	}

	/// <summary>Get the AttachmentDescriptionFlags</summary>
	/// <returns>A set of AttachmentDescriptionFlags</returns>
	inline AttachmentDescriptionFlags getFlags() const
	{
		return reinterpret_cast<const AttachmentDescriptionFlags&>(flags);
	}
	/// <summary>Set the attachment description flags</summary>
	/// <param name="flags">The attachment description flags</param>
	inline void setFlags(const AttachmentDescriptionFlags& flags)
	{
		this->flags = *reinterpret_cast<const VkAttachmentDescriptionFlags*>(&flags);
	}
	/// <summary>Get the attachment Format</summary>
	/// <returns>The attachment format</returns>
	inline Format getFormat() const
	{
		return reinterpret_cast<const Format&>(format);
	}
	/// <summary>Set the attachment format</summary>
	/// <param name="format">The attachment format</param>
	inline void setFormat(const Format& format)
	{
		this->format = *reinterpret_cast<const VkFormat*>(&format);
	}
	/// <summary>Get the sample count for the attachment</summary>
	/// <returns>The sample count for the attachment</returns>
	inline SampleCountFlags getSamples() const
	{
		return reinterpret_cast<const SampleCountFlags&>(samples);
	}
	/// <summary>Set the attachment sample count flags</summary>
	/// <param name="samples">The attachment sample count flags</param>
	inline void setSamples(const SampleCountFlags& samples)
	{
		this->samples = *reinterpret_cast<const VkSampleCountFlagBits*>(&samples);
	}
	/// <summary>Get the attachment load operation</summary>
	/// <returns>The attachment load operation</returns>
	inline AttachmentLoadOp getLoadOp() const
	{
		return reinterpret_cast<const AttachmentLoadOp&>(loadOp);
	}
	/// <summary>Set the attachment load operation</summary>
	/// <param name="loadOp">The attachment load operation</param>
	inline void setLoadOp(const AttachmentLoadOp& loadOp)
	{
		this->loadOp = *reinterpret_cast<const VkAttachmentLoadOp*>(&loadOp);
	}
	/// <summary>Get the attachment store operation</summary>
	/// <returns>The attachment store operation</returns>
	inline AttachmentStoreOp getStoreOp() const
	{
		return reinterpret_cast<const AttachmentStoreOp&>(storeOp);
	}
	/// <summary>Set the attachment store operation</summary>
	/// <param name="storeOp">The attachment store operation</param>
	inline void setStoreOp(const AttachmentStoreOp& storeOp)
	{
		this->storeOp = *reinterpret_cast<const VkAttachmentStoreOp*>(&storeOp);
	}
	/// <summary>Get the attachment stencil load operation</summary>
	/// <returns>The attachment stencil load operation</returns>
	inline AttachmentLoadOp getStencilLoadOp() const
	{
		return reinterpret_cast<const AttachmentLoadOp&>(stencilLoadOp);
	}
	/// <summary>Set the attachment stencil load operation</summary>
	/// <param name="stencilLoadOp">The attachment stencil load operation</param>
	inline void setStencilLoadOp(const AttachmentLoadOp& stencilLoadOp)
	{
		this->stencilLoadOp = *reinterpret_cast<const VkAttachmentLoadOp*>(&stencilLoadOp);
	}
	/// <summary>Get the attachment stencil store operation</summary>
	/// <returns>The attachment stencil store operation</returns>
	inline AttachmentStoreOp getStencilStoreOp() const
	{
		return reinterpret_cast<const AttachmentStoreOp&>(stencilStoreOp);
	}
	/// <summary>Set the attachment stencil store operation</summary>
	/// <param name="stencilStoreOp">The attachment stencil store operation</param>
	inline void setStencilStoreOp(const AttachmentStoreOp& stencilStoreOp)
	{
		this->stencilStoreOp = *reinterpret_cast<const VkAttachmentStoreOp*>(&stencilStoreOp);
	}
	/// <summary>Get the attachment initial layout</summary>
	/// <returns>The attachment initial layout</returns>
	inline ImageLayout getInitialLayout() const
	{
		return reinterpret_cast<const ImageLayout&>(initialLayout);
	}
	/// <summary>Set the attachment initial layout</summary>
	/// <param name="initialLayout">The attachment initial layout</param>
	inline void setInitialLayout(const ImageLayout& initialLayout)
	{
		this->initialLayout = *reinterpret_cast<const VkImageLayout*>(&initialLayout);
	}
	/// <summary>Get the attachment final layout</summary>
	/// <returns>The attachment final layout</returns>
	inline ImageLayout getFinalLayout() const
	{
		return reinterpret_cast<const ImageLayout&>(finalLayout);
	}
	/// <summary>Set the attachment final layout</summary>
	/// <param name="finalLayout">The attachment final layout</param>
	inline void setFinalLayout(const ImageLayout& finalLayout)
	{
		this->finalLayout = *reinterpret_cast<const VkImageLayout*>(&finalLayout);
	}
};

/// <summary>Render pass subpass. Subpasses allow intermediate draws to be chained and communicating with techniques
/// like Pixel Local Storage without outputting to the FrameBuffer until the end of the RenderPass.</summary>
struct SubpassDescription
{
public:
	/// <summary>Constructor</summary>
	/// <param name="pipeBindPoint">The binding point for this subpass (Graphics, Compute). Default Graphics.</param>
	SubpassDescription(pvrvk::PipelineBindPoint pipeBindPoint = pvrvk::PipelineBindPoint::e_GRAPHICS)
		: _pipelineBindPoint(pipeBindPoint), _numInputAttachments(0), _numColorAttachments(0), _numResolveAttachments(0), _numPreserveAttachments(0)
	{
		for (uint32_t i = 0; i < FrameworkCaps::MaxPreserveAttachments; ++i)
		{
			_preserveAttachment[i] = static_cast<uint32_t>(-1);
		}
	}

	/// <summary>Set the pipeline binding point.</summary>
	/// <param name="bindingPoint">New pipeline binding point</param>
	/// <returns>Reference to this(allows chaining)</returns>
	SubpassDescription& setPipelineBindPoint(pvrvk::PipelineBindPoint bindingPoint)
	{
		_pipelineBindPoint = bindingPoint;
		return *this;
	}

	/// <summary>Activate the specified color output attachment of the framebuffer.</summary>
	/// <param name="bindingIndex">Corresponding fragment shader output location. The Index must start from 0 and must be
	/// consective.</param>
	/// <param name="attachmentReference">Attachment to activate as output</param>
	/// <returns>Reference to this(allows chaining)</returns>
	SubpassDescription& setColorAttachmentReference(uint32_t bindingIndex, const AttachmentReference& attachmentReference)
	{
		_numColorAttachments += static_cast<uint8_t>(setAttachment(bindingIndex, attachmentReference, _colorAttachment, static_cast<uint32_t>(FrameworkCaps::MaxColorAttachments)));
		return *this;
	}

	/// <summary>Set the specified color attachment as input.</summary>
	/// <param name="bindingIndex">Corresponding fragment shader input location. The Index must start from 0 and must be
	/// consective.</param>
	/// <param name="attachmentReference">Attachment to set as input</param>
	/// <returns>Reference to this(allows chaining)</returns>
	SubpassDescription& setInputAttachmentReference(uint32_t bindingIndex, const AttachmentReference& attachmentReference)
	{
		_numInputAttachments += static_cast<uint8_t>(setAttachment(bindingIndex, attachmentReference, _inputAttachment, static_cast<uint32_t>(FrameworkCaps::MaxInputAttachments)));
		return *this;
	}

	/// <summary>Activate the specified Resolve attachment of the framebuffer.</summary>
	/// <param name="bindingIndex">Corresponding fragment shader input location. The Index must start from 0 and must be
	/// consective.</param>
	/// <param name="attachmentReference">Attachment to set as resolve</param>
	/// <returns>this (allow chaining)</returns>
	SubpassDescription& setResolveAttachmentReference(uint32_t bindingIndex, const AttachmentReference& attachmentReference)
	{
		_numResolveAttachments +=
			static_cast<uint8_t>(setAttachment(bindingIndex, attachmentReference, _resolveAttachments, static_cast<uint32_t>(FrameworkCaps::MaxResolveAttachments)));
		return *this;
	}

	/// <summary>Set preserve attachment from the framebuffer.</summary>
	/// <param name="bindingIndex">The Index must start from 0 and must be consective.</param>
	/// <param name="preserveAttachment">Attachment to set as preserve</param>
	/// <returns>Reference to this(allows chaining)</returns>
	/// <returns>this (allow chaining)</returns>
	SubpassDescription& setPreserveAttachmentReference(uint32_t bindingIndex, const uint32_t preserveAttachment)
	{
		_numPreserveAttachments += (this->_preserveAttachment[bindingIndex] == static_cast<uint32_t>(-1) && preserveAttachment != static_cast<uint32_t>(-1)) ? 1 : 0;
		this->_preserveAttachment[bindingIndex] = preserveAttachment;
		return *this;
	}

	/// <summary>Sets depth stencil attachment reference.</summary>
	/// <param name="attachmentReference">New depth-stencil attachment reference</param>
	/// <returns>Reference to this(allows chaining)</returns>
	SubpassDescription& setDepthStencilAttachmentReference(const AttachmentReference& attachmentReference)
	{
		_depthStencilAttachment = attachmentReference;
		return *this;
	}

	/// <summary>Return number of color attachments</summary>
	/// <returns>Return number of color attachment references</returns>
	uint8_t getNumColorAttachmentReference() const
	{
		return _numColorAttachments;
	}

	/// <summary>Return number of input attachments</summary>
	/// <returns>Return number of input attachment references</returns>
	uint8_t getNumInputAttachmentReference() const
	{
		return _numInputAttachments;
	}

	/// <summary>Get number of resolve attachments (const)</summary>
	/// <returns>Number of resolve attachments</returns>
	uint8_t getNumResolveAttachmentReference() const
	{
		return _numResolveAttachments;
	}

	/// <summary>Return number of preserve attachments (const)</summary>
	//// <returns>Number of preserve attachments</returns>
	uint8_t getNumPreserveAttachmentReference() const
	{
		return _numPreserveAttachments;
	}

	/// <summary>Get pipeline binding point (const)</summary>
	/// <returns>Returns Pipeline binding point</returns>
	pvrvk::PipelineBindPoint getPipelineBindPoint() const
	{
		return _pipelineBindPoint;
	}

	/// <summary>Get input attachment id (const)</summary>
	/// <param name="index">Attachment index</param>
	/// <returns>Input attachment id</returns>
	const AttachmentReference& getInputAttachmentReference(uint8_t index) const
	{
		assertion(index < _numInputAttachments, "Invalid index");
		return _inputAttachment[index];
	}

	/// <summary>Get depth stencil attachment reference (const).</summary>
	/// <returns> Return depth-stencil attachment reference id</returns>
	const AttachmentReference& getDepthStencilAttachmentReference() const
	{
		return this->_depthStencilAttachment;
	}

	/// <summary>Get color attachment id (const)</summary>
	/// <param name="index">Attachment index</param>
	/// <returns>Color attachment id</returns>
	const AttachmentReference& getColorAttachmentReference(uint8_t index) const
	{
		assertion(index < _numColorAttachments, "Invalid index");
		return _colorAttachment[index];
	}

	/// <summary>Get resolve attachment id (const)</summary>
	/// <param name="index">Attachment index</param>
	/// <returns>Resolve attachment id</returns>
	const AttachmentReference& getResolveAttachmentReference(uint8_t index) const
	{
		assertion(index < _numResolveAttachments, "Invalid index");
		return _resolveAttachments[index];
	}

	/// <summary>Get preserve attachment id (const)</summary>
	/// <param name="index">Attachment index</param>
	/// <returns>Preserve attachment id</returns>
	uint32_t getPreserveAttachmentReference(uint8_t index) const
	{
		assertion(index < _numPreserveAttachments, "Invalid index");
		return _preserveAttachment[index];
	}

	/// <summary>Get all preserve attahments ids (const)</summary>(const)
	/// <returns>Return all preserve attachments id</returns>
	const uint32_t* getAllPreserveAttachments() const
	{
		return _preserveAttachment;
	}

	/// <summary>clear all entries</summary>
	/// <returns>Returns this object (allows chained calls)</returns>
	SubpassDescription& clear()
	{
		_numInputAttachments = _numResolveAttachments = _numPreserveAttachments = _numColorAttachments = 0;
		memset(_inputAttachment, 0, sizeof(_inputAttachment[0]) * (FrameworkCaps::MaxInputAttachments));
		memset(_colorAttachment, 0, sizeof(_colorAttachment[0]) * (FrameworkCaps::MaxColorAttachments));
		memset(_preserveAttachment, 0, sizeof(_preserveAttachment[0]) * (FrameworkCaps::MaxPreserveAttachments));
		return *this;
	}

private:
	uint32_t setAttachment(uint32_t bindingId, const AttachmentReference& newAttachment, AttachmentReference* attachments, uint32_t maxAttachment)
	{
		assertion(bindingId < maxAttachment, "Binding Id exceeds the max limit");
		const uint32_t oldId = attachments[bindingId].getAttachment();
		attachments[bindingId] = newAttachment;
		return (oldId == static_cast<uint32_t>(-1) ? 1 : 0);
	}

	pvrvk::PipelineBindPoint _pipelineBindPoint;
	AttachmentReference _inputAttachment[FrameworkCaps::MaxInputAttachments];
	AttachmentReference _colorAttachment[FrameworkCaps::MaxColorAttachments];
	AttachmentReference _resolveAttachments[FrameworkCaps::MaxResolveAttachments];
	uint32_t _preserveAttachment[FrameworkCaps::MaxPreserveAttachments];
	AttachmentReference _depthStencilAttachment;
	uint8_t _numInputAttachments;
	uint8_t _numColorAttachments;
	uint8_t _numResolveAttachments;
	uint8_t _numPreserveAttachments;
};
} // namespace pvrvk
#undef DEFINE_ENUM_OPERATORS
