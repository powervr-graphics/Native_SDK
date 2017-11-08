/*!
\brief Contains framework types.
\file PVRVk/TypesVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/BindingsVk.h"
#include "PVRVk/RefCounted.h"
#include "PVRVk/Log.h"

namespace pvrvk {
/// <summary>INTERNAL. Defines the basic bitwise operators for an enumeration (AND and OR)</summary>
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

/// <summary>INTERNAL. Disable the Copy Constructor and the Copy Assignment Operator of the type</summary>
#define DECLARE_NO_COPY_SEMANTICS(TYPE)\
    TYPE(const TYPE&) = delete; \
    const TYPE& operator=(const TYPE&) = delete;

/// <summary>INTERNAL. Declare that a PVRVk as just a typedef of the corresponding Vulkan type</summary>
#define ALIAS_VK_TYPE(type) typedef Vk##type type;
ALIAS_VK_TYPE(FormatProperties)
ALIAS_VK_TYPE(SurfaceCapabilitiesKHR)
ALIAS_VK_TYPE(ImageFormatProperties)
ALIAS_VK_TYPE(PhysicalDeviceMemoryProperties)
ALIAS_VK_TYPE(PhysicalDeviceProperties)
ALIAS_VK_TYPE(PhysicalDeviceFeatures)
ALIAS_VK_TYPE(ExtensionProperties)
ALIAS_VK_TYPE(LayerProperties)
ALIAS_VK_TYPE(AllocationCallbacks)
/// <summary>Floating point Color data (rgba). Values from 0-1 inclusive</summary>
struct Color
{
	float color[4];//!< rgba
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
	float r()const { return color[0]; }

	/// <summary>Get green component (const)</summary>
	/// <returns>Green component</returns>
	float g()const { return color[1]; }

	/// <summary>Get blue component (const)</summary>
	/// <returns>Blue component</returns>
	float b()const { return color[2]; }

	/// <summary>Get alpha component (const)</summary>
	/// <returns>Alpha component</returns>
	float a()const { return color[3]; }
};

/// <summary>Contains clear color values (rgba).
/// This is used in Commandbuffer::clearColorImage
/// </summary>
struct ClearColorValue
{
	VkClearColorValue color;//!< Vulkan clear color value
	/// <summary>Constructor, Initialise with default r:0, g:0, b:0, a:1</summary>
	ClearColorValue() { color.float32[0] = color.float32[1] = color.float32[2] = 0; color.float32[3] = 1; }

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
	float r()const { return color.float32[0]; }

	/// <summary>Get green floating point component</summary>
	/// <returns>Green component</returns>
	float g()const { return color.float32[1]; }

	/// <summary>Get blue floating point component</summary>
	/// <returns>Blue component</returns>
	float b()const { return color.float32[2]; }

	/// <summary>Get alpha floating point component</summary>
	/// <returns>Alpha component</returns>
	float a()const { return color.float32[3]; }

	/// <summary>Get red interger component</summary>
	/// <returns>Red component</returns>
	int32_t r_i()const { return color.int32[0]; }

	/// <summary>Get green interger component</summary>
	/// <returns>Green component</returns>
	int32_t g_i()const { return color.int32[1]; }

	/// <summary>Get blue interger component</summary>
	/// <returns>Blue component</returns>
	int32_t b_i()const { return color.int32[2]; }

	/// <summary>Get alpha interger component</summary>
	/// <returns>alpha component</returns>
	int32_t a_i()const { return color.int32[3]; }

	/// <summary>Get red unsiged interger component</summary>
	/// <returns>Red component</returns>
	uint32_t r_ui()const { return color.uint32[0]; }

	/// <summary>Get green unsiged interger component</summary>
	/// <returns>Green component</returns>
	uint32_t g_ui()const { return color.uint32[1]; }

	/// <summary>Get blue unsiged interger component</summary>
	/// <returns>Blue component</returns>
	uint32_t b_ui()const { return color.uint32[2]; }

	/// <summary>Get alpha unsiged interger component</summary>
	/// <returns>Alpha component</returns>
	uint32_t a_ui()const { return color.uint32[3]; }
};

/// <summary>Enumeration of Texture dimensionalities.</summary>
enum class ImageViewType
{
	ImageView1D,            //!< 1 dimensional Image View
	ImageView2D,            //!< 2 dimensional Image View
	ImageView3D,            //!< 3 dimensional Image View
	ImageView2DCube,        //!< 2 dimensional cube Image View
	ImageView1DArray,       //!< 1 dimensional Image View
	ImageView2DArray,       //!< 2 dimensional Image View
	ImageView2DCubeArray,   //!< 2 dimensional Image View
	ImageViewUnknown,       //!< 3 dimensional Image View
};

namespace GpuDatatypesHelper {
/// <summary> A bit representing if a type is basically of integer or floating point format </summary>
enum class BaseType { Integer = 0, Float = 1 };

/// <summary> Two bits, representing the number of vector components (from scalar up to 4)</summary>
enum class  VectorWidth { Scalar = 0, Vec2 = 1, Vec3 = 2, Vec4 = 3, };

/// <summary> Three bits, representing the number of matrix columns (from not a matrix to 4)</summary>
enum class  MatrixColumns { OneCol = 0, Mat2x = 1, Mat3x = 2, Mat4x = 3 };

/// <summary> Contains bit enums for the expressiveness of the GpuDatatypes class' definition</summary>
enum class Bits : uint64_t
{
	Integer = 0, Float = 1,
	BitScalar = 0, BitVec2 = 2, BitVec3 = 4, BitVec4 = 6,
	BitOneCol = 0, BitMat2x = 8, BitMat3x = 16, BitMat4x = 24,
	ShiftType = 0, MaskType = 1, NotMaskType = static_cast<uint64_t>(~MaskType),
	ShiftVec = 1, MaskVec = (3 << ShiftVec), NotMaskVec = static_cast<uint64_t>(~MaskVec),
	ShiftCols = 3, MaskCols = (3 << ShiftCols), NotMaskCols = static_cast<uint64_t>(~MaskCols)
};
DEFINE_ENUM_OPERATORS(Bits)
}

/// <summary>A (normally hardware-supported) GPU datatype (e.g. vec4 etc.)</summary>
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

/// <summary>An enumeration that defines data types used throughout the Framework. Commonly used in places where
/// raw data are used to define the types actually contained.</summary>
enum class DataType
{
	None,           //!< none
	Float32,//!< float 1
	Int32,//!< Integer 2
	UInt16, //!< unsigned short 3
	RGBA,//!< rgba 4
	ARGB,//!< argb 5
	D3DCOLOR,//!< d3d color 6
	UBYTE4,//!< unsigned 4 char 7
	DEC3N,
	Fixed16_16,
	UInt8,//!< unsigned char 10
	Int16,//!< short 11
	Int16Norm,//!< short normalized 12
	Int8,//!< char 13
	Int8Norm,//!< char normalized 14
	UInt8Norm,//!< unsigned char normalized 15
	UInt16Norm,//!< unsigned short normalized
	UInt32,//!< unsigned int
	ABGR,//!< abgr
	Float16,//!< Half float
	Custom = 1000
};

/// <summary>Step rate for a vertex attribute when drawing: Per vertex, per instance, per draw.</summary>
enum class StepRate : uint32_t
{
	Vertex, //!< Step rate Per vertex
	Instance,//!< Step rate per instance
	Default = Vertex
};

namespace internals {
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
size_t insertSorted_overwrite(container& cont, typename container::iterator begin,
                              typename container::iterator end, const val& item)
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
}

/// <summary>Structure containing 2-dimensional offset data, x and y</summary>
struct Offset2D : public VkOffset2D
{
	/// <summary>Constructor. Initialise from VkOffset2D</summary>
	/// <param name="vkoff">Vulkan 2d offset</param>
	Offset2D(VkOffset2D vkoff) { x = vkoff.x; y = vkoff.y; }

	/// <summary>Constructor. Initialise with x,y offsets</summary>
	/// <param name="myx">Offset x, default 0.</param>
	/// <param name="myy">Offset y, default 0</param>
	Offset2D(int32_t myx = 0, int32_t myy = 0) { x = myx; y = myy; }
};

/// <summary>Structure containing 3-dimensional offset data, x, y and z</summary>
struct Offset3D : public VkOffset3D
{
	/// <summary>Constructor. Initialise from VkOffset3Ds</summary>
	/// <param name="vkoff">Vulkan 3d offset.</param>
	Offset3D(VkOffset3D vkoff) { x = vkoff.x; y = vkoff.y; z = vkoff.z; }

	/// <summary>Constructor. Initialise with x,y,z offsets</summary>
	/// <param name="myx">Offset x, default 0</param>
	/// <param name="myy">Offset y, default 0</param>
	/// <param name="myz">Offset z, default 0</param>
	Offset3D(int32_t myx = 0, int32_t myy = 0, int32_t myz = 0) { x = myx; y = myy; z = myz; }
};

/// <summary>2-dimensional extent(width and height)</summary>
struct Extent2D : public VkExtent2D
{
	/// <summary>Constructor. Initialise with VkExtent2D</summary>
	/// <param name="vkoff">Vulkan extent2D</param>
	Extent2D(VkExtent2D vkoff) { width = vkoff.width; height = vkoff.height; }

	/// <summary>Constructor. Initialise with width and height</summary>
	/// <param name="mywidth">Extent width</param>
	/// <param name="myheight">Extent height</param>
	Extent2D(uint32_t mywidth = 0, uint32_t myheight = 0) { width = mywidth; height = myheight; }
};

/// <summary>3-dimensional extent(width, height and depth)</summary>
struct Extent3D : public VkExtent3D
{
	/// <summary>Construct Extent3D from VkExtent3D</summary>
	/// <param name="vkoff">Vulkan extent3D</param>
	Extent3D(VkExtent3D vkoff) { width = vkoff.width; height = vkoff.height; depth = vkoff.depth; }

	/// <summary>Constructor, Initialise with width, height and depth.
	/// Default initialise (width:1, height:1, depth:1)
	/// </summary>
	/// <param name="mywidth">Width</param>
	/// <param name="myheight">heightS</param>
	/// <param name="mydepth">Depth</param>
	Extent3D(uint32_t mywidth = 1, uint32_t myheight = 1, uint32_t mydepth = 1)
	{
		width = mywidth; height = myheight; depth = mydepth;
	}

	/// <summary>Constructor.Initialise with extent and depth</summary>
	/// <param name="e2d">2-dimensional Extent</param>
	/// <param name="mydepth">Depth</param>
	Extent3D(Extent2D e2d, uint32_t mydepth = 1) { width = e2d.width; height = e2d.height; depth = mydepth; }
};

/// <summary>2-dimensional interger rectangle</summary>
struct Rect2Di : public VkRect2D
{
	/// <summary>Constructor. Initialise with x,y width and height</summary>
	/// <param name="myx">Offset x</param>
	/// <param name="myy">Offset y</param>
	/// <param name="mywidth">Width</param>
	/// <param name="myheight">Height</param>
	Rect2Di(int32_t myx, uint32_t myy, uint32_t mywidth, uint32_t myheight)
	{
		offset.x = myx; offset.y = myy; extent.width = mywidth; extent.height = myheight;
	}

	/// <summary>Constructor. Initialise with offset and extent</summary>
	/// <param name="myOffset">2D offset</param>
	/// <param name="myExtent">2D extent</param>
	Rect2Di(const Offset2D& myOffset, const Extent2D& myExtent)
	{
		offset = myOffset, extent = myExtent;
	}

	/// <summary>Constructor. Initialise to 0</summary>
	Rect2Di() { memset(this, 0, sizeof(*this)); }
};

/// <summary>Describes a view of an image.</summary>
struct ImageSubresourceRange : VkImageSubresourceRange
{
	/// <summary>Constructor.</summary>
	/// <param name="aspectMask">Bitmask of VkImageAspectFlagBits specifying which aspect(s) of the image are included in the view.</param>
	/// <param name="baseMipLevel">First mipmap level accessible to the view.</param>
	/// <param name="levelCount">Number of mipmap levels (starting from baseMipLevel) accessible to the view.</param>
	/// <param name="baseArrayLayer">First array layer accessible to the view.</param>
	/// <param name="layerCount">Number of array layers (starting from baseArrayLayer) accessible to the view.</param>
	ImageSubresourceRange(VkImageAspectFlags aspectMask = VkImageAspectFlags::e_MAX_ENUM, uint32_t baseMipLevel = 0,
	                      uint32_t levelCount = 1, uint32_t baseArrayLayer = 0, uint32_t layerCount = 1)
	{
		this->aspectMask = (aspectMask);
		this->baseMipLevel = (baseMipLevel);
		this->levelCount = (levelCount);
		this->baseArrayLayer = (baseArrayLayer);
		this->layerCount = (layerCount);
	}
};

/// <summary>Describes a portion of an image (aspectmask, mip level and array level)</summary>
struct ImageSubresource : VkImageSubresource
{
	/// <summary>Constructor. Initialise with 0</summary>
	ImageSubresource()
	{
		aspectMask = VkImageAspectFlags(0);
		mipLevel = 0;
		arrayLayer = 0;
	}
};

/// <summary>The ImageSubresourceLayers struct specify the specific image subresources of the image used for the source or destination image data</summary>
struct ImageSubresourceLayers: public VkImageSubresourceLayers
{
	/// <summary>Constructor. Default initialise baseArrayLayer, layerCount and mipLevel to 0 and aspect mask VkImageAspectFlags::e_COLOR_BIT</summary>
	ImageSubresourceLayers()
	{
		this->aspectMask = VkImageAspectFlags::e_COLOR_BIT;
		this->baseArrayLayer = 0;
		this->layerCount = 1;
		this->mipLevel = 0;
	}
};

/// <summary>Represents a blit operation through source and destination offsets</summary>
struct ImageBlitRange : public VkImageBlit
{
	/// <summary>Constructor. Undefined initialisation</summary>
	ImageBlitRange() {}

	/// <summary>Constructor</summary>
	/// <param name="srcOffset0">Blit source offset0 (xyz)</param>
	/// <param name="srcOffset1">Blit source offset1S (xyz)</param>
	/// <param name="dstOffset0">Blit destination offset0 (xyz)</param>
	/// <param name="dstOffset1">Blit destination offset1 (xyz)</param>
	/// <param name="srcSubResource">Source aspectMask, baseArrayLayer, layerCount, mipLevel</param>
	/// <param name="dstSubResource">Destination aspectMask, baseArrayLayer, layerCount, mipLevel</param>
	ImageBlitRange(const Offset3D& srcOffset0, const Offset3D& srcOffset1,
	               const Offset3D& dstOffset0, const Offset3D& dstOffset1,
	               const ImageSubresourceLayers& srcSubResource = ImageSubresourceLayers(),
	               const ImageSubresourceLayers& dstSubResource = ImageSubresourceLayers())
	{
		srcOffsets[0] = srcOffset0, srcOffsets[1] = srcOffset1;
		dstOffsets[0] = dstOffset0, dstOffsets[1] = dstOffset1;
		this->srcSubresource = srcSubResource;
		this->dstSubresource = dstSubResource;
	}

	/// <summary>Constructor</summary>
	/// <param name="srcOffsets">Pointer to source offsets (xyz)</param>
	/// <param name="dstOffsets">Pointer to destination offsets (xyz)</param>
	/// <param name="srcSubResource">Source aspectMask, baseArrayLayer, layerCount, mipLevel</param>
	/// <param name="dstSubResource">Destination aspectMask, baseArrayLayer, layerCount, mipLevel</param>
	ImageBlitRange(const Offset3D srcOffsets[2], const Offset3D dstOffsets[2],
	               const ImageSubresourceLayers& srcSubResource = ImageSubresourceLayers(),
	               const ImageSubresourceLayers& dstSubResource = ImageSubresourceLayers())
	{
		memcpy(this->srcOffsets, srcOffsets, sizeof(Offset3D) * 2);
		memcpy(this->dstOffsets, dstOffsets, sizeof(Offset3D) * 2);
		this->srcSubresource = srcSubResource;
		this->dstSubresource = dstSubResource;
	}
};

/// <summary>Contains information to resolve multisample image to a non-multisample image</summary>
struct ImageResolve : public VkImageResolve
{
	/// <summary>Constructor. Initialised with default values</summary>
	ImageResolve()
	{
		this->srcSubresource = ImageSubresourceLayers();
		this->dstSubresource = ImageSubresourceLayers();
		this->srcOffset = Offset3D();
		this->dstOffset = Offset3D();
		this->extent = Extent3D();
	}
};

/// <summary>Represents the geometric size (number of rows/columns/slices in x-y-z) and number of array and mipmap layers
/// of an image.</summary>
struct ImageLayersSize
{
	uint32_t numArrayLevels;//!< Number of array levels
	uint32_t numMipLevels;//!< Number of mip levels
	ImageLayersSize() : numArrayLevels(1), numMipLevels(1) {}

	/// <summary>Constructor (takes individual values)</summary>
	/// <param name="numArrayLevels">Number of array levels</param>
	/// <param name="numMipLevels">Number of mip levels</param>
	ImageLayersSize(uint32_t numArrayLevels, uint32_t numMipLevels) :
		numArrayLevels(numArrayLevels), numMipLevels(numMipLevels) {}
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
	ImageAreaSize(const ImageLayersSize& layersSize, const Extent3D& extents) :
		ImageLayersSize(layersSize), Extent3D(extents) {}
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
	ImageAreaOffset(const ImageSubresource& baseLayers, const Offset3D& offset) :
		ImageSubresource(baseLayers), Offset3D(offset) {}
};

/// <summary>2-dimensional floating point rectangle</summary>
struct Rect2Df
{
	/// <summary>2 dimensional offset which contains the x and y</summary>
	struct Offset
	{
		float x;//!< offset x
		float y;//!< offset y

		/// <summary>Constructor</summary>
		/// <param name="myx">Offset x</param>
		/// <param name="myy">Offset y</param>
		Offset(float myx, float myy) : x(myx), y(myy) {}
	};

	/// <summary>2 dimensional extent which contains the width and height</summary>
	struct Extent
	{
		float width;//!< extent width
		float height;//!< extent height

		/// <summary>Constructor</summary>
		/// <param name="mywidth">Width</param>
		/// <param name="myheight">Height</param>
		Extent(float mywidth, float myheight) : width(mywidth), height(myheight) {}
	};

	Offset offset;//!< offset x and y
	Extent extent;//!< Width and height relative to offset.

	/// <summary>Constructor, takes individual values</summary>
	/// <param name="myx">offset x</param>
	/// <param name="myy">offset y</param>
	/// <param name="mywidth">extent width</param>
	/// <param name="myheight">extent height</param>
	Rect2Df(float myx, float myy, float mywidth, float myheight) : offset(myx, myy), extent(mywidth, myheight) { }

	/// <summary>Constructor, sets to offset(0,0) and extent(1,1)</summary>
	Rect2Df() : offset(0.0f, 0.0f), extent(1.0f, 1.0f) { }
};

/// <summary>Containes application info used for creating vulkan instance</summary>
struct ApplicationInfo
{
	const char*        applicationName;//!< NULL or is a pointer to a null-terminated UTF-8 string containing the name of the application
	uint32_t           applicationVersion;//!< Unsigned integer variable containing the developer-supplied version number of the application
	const char*        engineName;//!< NULL or is a pointer to a null-terminated UTF-8 string containing the name of the engine (if any) used to create the application
	uint32_t           engineVersion;//!< Unsigned integer variable containing the developer-supplied version number of the engine used to create the application.
	uint32_t           apiVersion;//!< version of the Vulkan API against which the application expects to run. If apiVersion is 0 the implementation ignore it.

	/// <summary>Constructor</summary>
	ApplicationInfo() : applicationVersion(0), engineVersion(0) {}
};

/// <summary>Contains instant info used for creating Vulkan instance</summary>
struct InstanceCreateInfo
{
	VkInstanceCreateFlags     flags;//!< Reserved for future use
	const ApplicationInfo*    applicationInfo;//!< NULL or a pointer to an instance of ApplicationInfo. If not NULL, this information helps implementations recognize behavior inherent to classes of applications.
	std::vector<std::string>  enabledLayerNames;//!<  Array of null-terminated UTF-8 strings containing the names of layers to enable for the created instance
	std::vector<std::string>  enabledExtensionNames;//!<  Array of null-terminated UTF-8 strings containing the names of extensions to enable for the created instance

	/// <summary>Constructor. Default initialised to 0</summary>
	InstanceCreateInfo() : flags(VkInstanceCreateFlags(0)), applicationInfo(nullptr) {}
};

/// <summary>Contains information about the queues to create for a single queue family. A set of DeviceQueueCreateInfo structures for each queue families are passed in to DeviceCreateInfo when creating the device.</summary>
struct DeviceQueueCreateInfo
{
	uint32_t      queueFamilyIndex;//!< Queue family index the queue will be created from
	uint32_t      queueCount;//!< Number of queues to create from this family
	float         queuePriorities[16];//!< Array of queueCount normalized floating point values, specifying priorities of work that will be submitted to each created queue
};

/// <summary>Containes device create info.</summary>
struct DeviceCreateInfo
{
	VkDeviceCreateFlags                flags;//!< Reserved for future use
	std::vector<DeviceQueueCreateInfo> queueCreateInfos;//!< Pointer to an array of DeviceQueueCreateInfo structures describing the queues that are requested to be created along with the logical device
	std::vector<std::string>           enabledLayerNames;//!< Array of layers to enable
	std::vector<std::string>           enabledExtensionNames;//!< Array of extensions to enable
	const PhysicalDeviceFeatures*    enabledFeatures;//!< NULL or a pointer to a PhysicalDeviceFeatures structure that contains boolean indicators of all the features to be enabled

	/// <summary>Constructor. Default initialised to 0</summary>
	DeviceCreateInfo() : flags(VkDeviceCreateFlags(0)), enabledFeatures(nullptr) {}
};

/// <summary>Combines all vulkan flags of given type by bitwise ORing it.</summary>
template<typename Type>
struct CombineAllFlags
{};

/// <summary>Specialisation of CombineAllFlags which combines all the VkColorComponentFlags</summary>
template<> struct CombineAllFlags<VkColorComponentFlags>
{
	const static VkColorComponentFlags Flags =
	  VkColorComponentFlags(int(VkColorComponentFlags::e_R_BIT) |
	                        int(VkColorComponentFlags::e_G_BIT) |
	                        int(VkColorComponentFlags::e_B_BIT) |
	                        int(VkColorComponentFlags::e_A_BIT));//!< Combined color components flag
};

/// <summary>Pipeline stencil op state. Only used when creating a pipeline with non-dynamic stencil state</summary>
struct StencilOpState
{
	VkStencilOp    failOp;//!< VkStencilOp value specifying the action performed on samples that fail the stencil test.
	VkStencilOp    passOp;//!< VkStencilOp value specifying the action performed on samples that pass both the depth and stencil tests.
	VkStencilOp    depthFailOp;//!< VkStencilOp value specifying the action performed on samples that pass the stencil test and fail the depth test.
	VkCompareOp    compareOp;//!< VkCompareOp value specifying the comparison operator used in the stencil test.
	uint32_t       compareMask;//!< Selects the bits of the unsigned integer stencil values participating in the stencil test.
	uint32_t       writeMask;//!< Selects the bits of the unsigned integer stencil values updated by the stencil test in the stencil framebuffer attachment.
	uint32_t       reference;//!< Integer reference value that is used in the unsigned stencil comparison

	/// <summary>Constructor, sets from individual values</summary>
	/// <param name="passOp">Action performed on samples that pass both the depth and stencil tests.</param>
	/// <param name="depthFailOp">Action performed on samples that pass the stencil test and fail the depth test.</param>
	/// <param name="failOp">Action performed on samples that fail the stencil test.</param>
	/// <param name="compareOp">Comparison operator used in the stencil test.</param>
	/// <param name="compareMask">Selects the bits of the unsigned Integer stencil values during in the stencil test.
	/// </param>
	/// <param name="writeMask">Selects the bits of the unsigned Integer stencil values updated by the stencil test in the
	/// stencil framebuffer attachment</param>
	/// <param name="reference">Integer reference value that is used in the unsigned stencil comparison.</param>
	StencilOpState(VkStencilOp passOp = VkStencilOp::e_KEEP,
	               VkStencilOp depthFailOp = VkStencilOp::e_KEEP,
	               VkStencilOp failOp = VkStencilOp::e_KEEP,
	               VkCompareOp compareOp = VkCompareOp::e_ALWAYS,
	               uint32_t compareMask = 0xff, uint32_t writeMask = 0xff, uint32_t reference = 0)
		: passOp(passOp), depthFailOp(depthFailOp),
		  failOp(failOp), compareMask(compareMask),
		  writeMask(writeMask), reference(reference), compareOp(compareOp) {}
};

/// <summary>Contains the copy information of source and destination image.
/// An array of ImageCopy is passed in to Commandbuffer::copyImage for copy operations</summary>
struct ImageCopy
{
	ImageSubresourceLayers      srcSubresource;//!< ImageSubresourceLayers structures specifying the image subresources of the images used for the source image data
	Offset3D          srcOffset;//!< Select the initial x, y, and z offsets in texels of the sub-regions of the source image data.
	ImageSubresourceLayers      dstSubresource;//!< ImageSubresourceLayers structures specifying the image subresources of the images used for the destination image data
	Offset3D          dstOffset;//!< Select the initial x, y, and z offsets in texels of the sub-regions of the destination image data.
	Extent3D                    imageExtent;//!< Size in texels of the source image to copy in width, height and depth

	/// <summary>Constructor. Default initialised</summary>
	ImageCopy() : srcSubresource(), srcOffset(), dstSubresource(), dstOffset(), imageExtent() {}

	/// <summary>Constructor</summary>
	/// <param name="srcSubresource">ImageSubresourceLayers structure specifying the image subresources of the images used for the source image data.</param>
	/// <param name="srcOffset">Select the initial x, y, and z offsets in texels of the sub-regions of the source image data</param>
	/// <param name="dstSubresource">ImageSubresourceLayers structure specifying the image subresources of the images used for the destination image data.</param>
	/// <param name="dstOffset">Select the initial x, y, and z offsets in texels of the sub-regions of the destination image data</param>
	/// <param name="extent">Size in texels of the source image to copy in width, height and depth.</param>
	ImageCopy(ImageSubresourceLayers srcSubresource, Offset3D srcOffset,
	          ImageSubresourceLayers dstSubresource, Offset3D dstOffset,
	          Extent3D extent)
		: srcSubresource(srcSubresource), srcOffset(srcOffset), dstSubresource(dstSubresource),
		  dstOffset(dstOffset), imageExtent(extent)
	{}
};

/// <summary>Contains the copy information from an buffer to image and vice versa.
/// An array of BufferImageCopy is passed in to Commandbuffer::copyImageToBuffer and Commandbuffer::copyBufferToImage</summary>
struct BufferImageCopy
{
	VkDeviceSize                bufferOffset;//!< Offset in bytes from the start of the buffer object where the image data is copied from or to.
	uint32_t                    bufferRowLength;//!< Specify the data in buffer memory as a subregion of a larger two- or three-dimensional image, and control the addressing calculations of data in buffer memory. If the value is zero, that aspect of the buffer memory is considered to be tightly packed according to the imageExtent.
	uint32_t                    bufferImageHeight;//!< Specify the data in buffer memory as a subregion of a larger two- or three-dimensional image, and control the addressing calculations of data in buffer memory. If the value is zero, that aspect of the buffer memory is considered to be tightly packed according to the imageExtent.
	ImageSubresourceLayers      imageSubresource;//!< ImageSubresourceLayers used to specify the specific image subresources of the image used for the source or destination image data.
	Offset3D                    imageOffset;//!< Selects the initial x, y, z offsets in texels of the sub-region of the source or destination image data.
	Extent3D                    imageExtent;//!< Size in texels of the image to copy in width, height and depth.

	/// <summary>Constructor. Default initialized</summary>
	BufferImageCopy(): bufferOffset(0), bufferRowLength(0), bufferImageHeight(0) {}

	/// <summary>Constructor Offset in bytes from the start of the buffer object where the image data is copied from or to.</summary>
	/// <param name="bufferOffset">Specify the data in buffer memory as a subregion of a larger two- or three-dimensional image, and control the addressing calculations of data in buffer memory. If the value is zero, that aspect of the buffer memory is considered to be tightly packed according to the imageExtent.</param>
	/// <param name="bufferRowLength">Specify the data in buffer memory as a subregion of a larger two- or three-dimensional image, and control the addressing calculations of data in buffer memory. If the value is zero, that aspect of the buffer memory is considered to be tightly packed according to the imageExtent.</param>
	/// <param name="bufferImageHeight">ImageSubresourceLayers used to specify the specific image subresources of the image used for the source or destination image data.</param>
	/// <param name="imageSubresource">Selects the initial x, y, z offsets in texels of the sub-region of the source or destination image data.</param>
	/// <param name="imageOffset">Selects the initial x, y, z offsets in texels of the sub-region of the source or destination image data.</param>
	/// <param name="imageExtent">Size in texels of the image to copy in width, height and depth.</param>
	BufferImageCopy(VkDeviceSize  bufferOffset, uint32_t bufferRowLength,
	                uint32_t bufferImageHeight, ImageSubresourceLayers imageSubresource,
	                Offset3D imageOffset, Extent3D imageExtent)
		: bufferOffset(bufferOffset), bufferRowLength(bufferRowLength), bufferImageHeight(bufferImageHeight),
		  imageSubresource(imageSubresource), imageOffset(imageOffset), imageExtent(imageExtent)
	{
	}
};

/// <summary>Pipeline vertex input binding description. Each vertex input binding is specified by an instance of the VertexInputBindingDescription structure.</summary>
struct VertexInputBindingDescription
{
	uint32_t             binding;//!< Binding number that this structure describes.
	uint32_t             stride;//!< Distance in bytes between two consecutive elements within the buffer.
	VkVertexInputRate    inputRate;//!< VertexInputRate value specifying whether vertex attribute addressing is a function of the vertex index or of the instance index.

	/// <summary>Constructor.</summary>
	/// <param name="binding">Binding number that this structure describes.</param>
	/// <param name="stride">Binding number that this structure describes.</param>
	/// <param name="inputRate">VertexInputRate value specifying whether vertex attribute addressing is a function of the vertex index or of the instance index.</param>
	VertexInputBindingDescription(uint32_t binding = 0, uint32_t stride = 0,
	                              VkVertexInputRate inputRate = VkVertexInputRate::e_VERTEX)
		: binding(binding), stride(stride), inputRate(inputRate)
	{}
};

/// <summary>Contains rgba component swizzle values</summary>
struct ComponentMapping
{
	VkComponentSwizzle    r;//!< VkComponentSwizzle specifying the component value placed in the R component of the output vector
	VkComponentSwizzle    g;//!< VkComponentSwizzle specifying the component value placed in the G component of the output vector
	VkComponentSwizzle    b;//!< VkComponentSwizzle specifying the component value placed in the B component of the output vector
	VkComponentSwizzle    a;//!< VkComponentSwizzle specifying the component value placed in the A component of the output vector

	/// <summary>Constructor</summary>
	/// <param name="r">VkComponentSwizzle specifying the component value placed in the R component of the output vector</param>
	/// <param name="g">VkComponentSwizzle specifying the component value placed in the G component of the output vector</param>
	/// <param name="b">VkComponentSwizzle specifying the component value placed in the B component of the output vector</param>
	/// <param name="a">VkComponentSwizzle specifying the component value placed in the A component of the output vector</param>
	ComponentMapping(
	  VkComponentSwizzle r = VkComponentSwizzle::e_R,
	  VkComponentSwizzle g = VkComponentSwizzle::e_G,
	  VkComponentSwizzle b = VkComponentSwizzle::e_B,
	  VkComponentSwizzle a = VkComponentSwizzle::e_A)
		: r(r), g(g), b(b), a(a)
	{}
};

/// <summary>Pipeline vertex input attribute description
/// Each vertex input attribute is specified by an instance of the VertexInputAttributeDescription structure.
/// </summary>
struct VertexInputAttributeDescription
{
	uint32_t    location; //!< Shader binding location number for this attribute
	uint32_t    binding;//!< Binding number which this attribute takes its data from
	VkFormat    format;//!< Size and type of the vertex attribute dat
	uint32_t    offset;//!< Byte offset of this attribute relative to the start of an element in the vertex input binding.

	/// <summary>Constructor</summary>
	VertexInputAttributeDescription() {}

	/// <summary>Constructor</summary>
	/// <param name="location">Shader binding location number for this attribute</param>
	/// <param name="binding">Binding number which this attribute takes its data from</param>
	/// <param name="format">Size and type of the vertex attribute dat</param>
	/// <param name="offset">Byte offset of this attribute relative to the start of an element in the vertex input binding.</param>
	VertexInputAttributeDescription(uint16_t location, uint32_t binding, VkFormat format, uint32_t offset) :
		location(location), binding(binding), format(format), offset(offset) {}
};

/// <summary>PushConstantRange structures defining a set of push constant ranges for use in a single pipeline layout.
/// In addition to descriptor set layouts, a pipeline layout also describes how many push constants can be accessed by each stage of the pipeline.
/// NOTE: Push constants represent a high speed path to modify constant data in pipelines that is expected
/// to outperform memory-backed resource updates.
/// The layout of the push constant variables is specified in the shader
/// </summary>
struct PushConstantRange
{
	VkShaderStageFlags stage;//!< Set of stage flags describing the shader stages that will access a range of push constants. If a particular stage is not included in the range, then accessing members of that range of push constants from the corresponding shader stage will result in undefined data being read.
	uint32_t offset;//!< Start offset consumed by the range. It is in units of bytes and must be a multiple of 4
	uint32_t size;//!< Range size. It is in units of bytes and must be a multiple of 4

	/// <summary>Constructor. Default initialised to 0 with stage flags VkShaderStageFlags::e_ALL</summary>
	PushConstantRange() : offset(0), size(0), stage(VkShaderStageFlags::e_ALL) {}

	/// <summary>Constructor</summary>
	/// <param name="stage">Set of stage flags describing the shader stages that will access a range of push constants.
	/// If a particular stage is not included in the range, then accessing members of that range of push constants from the
	/// corresponding shader stage will result in undefined data being read.</param>
	/// <param name="offset">Start offset consumed by the range. It is in units of bytes and must be a multiple of 4</param>
	/// <param name="size">Range size. It is in units of bytes and must be a multiple of 4</param>
	PushConstantRange(VkShaderStageFlags stage, uint32_t offset, uint32_t size) :
		stage(stage), offset(offset), size(size) {}
};

/// <summary>Contains pipeline's per target attachment states</summary>
struct PipelineColorBlendAttachmentState : public VkPipelineColorBlendAttachmentState
{
	/// <summary>Create a blending state. Separate color/alpha factors.</summary>
	/// <param name="blendEnable">Controls whether blending is enabled for the corresponding color attachment.
	/// If blending is not enabled, the source fragment’s color for that attachment is passed through unmodified. Default: false</param>
	/// <param name="srcBlendColor">selects which blend factor is used to determine the source factors (Sr,Sg,Sb). Default: VkBlendFactor::e_ONE</param>
	/// <param name="dstBlendColor">Selects which blend factor is used to determine the destination factors (Dr,Dg,Db). Default: VkBlendFactor::e_ZERO</param>
	/// <param name="srcBlendAlpha">Selects which blend factor is used to determine the source factor Sa: VkBlendFactor::e_ONE</param>
	/// <param name="dstBlendAlpha">Selects which blend factor is used to determine the destination factor Da. VkBlendFactor::e_ZERO</param>
	/// <param name="blendOpColor">Selects which blend operation is used to calculate the RGB values to write to the color attachment. Default: VVkBlendOp::e_ADD</param>
	/// <param name="blendOpAlpha">Selects which blend operation is use to calculate the alpha values to write to the color attachment. Default: VkBlendOp::e_ADD</param>
	/// <param name="channelWriteMask">Bitmask of VkColorComponentFlagBits specifying which of the R, G, B, and/or A components are enabled for writing. Default: All</param>
	PipelineColorBlendAttachmentState(
	  VkBool32 blendEnable = false, VkBlendFactor srcBlendColor = VkBlendFactor::e_ONE,
	  VkBlendFactor dstBlendColor = VkBlendFactor::e_ZERO,
	  VkBlendFactor srcBlendAlpha = VkBlendFactor::e_ONE,
	  VkBlendFactor dstBlendAlpha = VkBlendFactor::e_ZERO,
	  VkBlendOp blendOpColor = VkBlendOp::e_ADD,
	  VkBlendOp blendOpAlpha = VkBlendOp::e_ADD,
	  VkColorComponentFlags channelWriteMask = CombineAllFlags<VkColorComponentFlags>::Flags)
	{
		this->blendEnable = blendEnable;
		this->srcColorBlendFactor = srcBlendColor;
		this->dstColorBlendFactor = dstBlendColor;
		this->srcAlphaBlendFactor = srcBlendAlpha;
		this->dstAlphaBlendFactor = dstBlendAlpha;
		this->colorBlendOp = blendOpColor;
		this->alphaBlendOp = blendOpAlpha;
		this->colorWriteMask = channelWriteMask;
	}

	/// <summary>Create a blending state. Common color and alpha factors.</summary>
	/// <param name="blendEnable">Controls whether blending is enabled for the corresponding color attachment.
	/// If blending is not enabled, the source fragment’s color for that attachment is passed through unmodified. Default: false</param>
	/// <param name="srcBlendFactor">selects which blend factor is used to determine the source factors (Sr,Sg,Sb,Sa). Default: VkBlendFactor::e_ONE</param>
	/// <param name="dstBlendFactor">Selects which blend factor is used to determine the destination factors (Dr,Dg,Db,Da). Default: VkBlendFactor::e_ZERO</param>
	/// <param name="blendOpColorAlpha">Selects which blend operation is used to calculate the RGB values to write to the color attachment. Default: VVkBlendOp::e_ADD</param>
	/// <param name="channelWriteMask">Bitmask of VkColorComponentFlagBits specifying which of the R, G, B, and/or A components are enabled for writing. Default: All</param>
	PipelineColorBlendAttachmentState(bool blendEnable, VkBlendFactor srcBlendFactor,
	                                  VkBlendFactor dstBlendFactor, VkBlendOp blendOpColorAlpha,
	                                  VkColorComponentFlags channelWriteMask = CombineAllFlags<VkColorComponentFlags>::Flags)
	{
		this->blendEnable = blendEnable;
		this->srcColorBlendFactor = srcBlendFactor;
		this->dstColorBlendFactor = dstBlendFactor;
		this->srcAlphaBlendFactor = srcBlendFactor;
		this->dstAlphaBlendFactor = dstBlendFactor;
		this->colorBlendOp = blendOpColorAlpha;
		this->alphaBlendOp = blendOpColorAlpha;
		this->colorWriteMask = channelWriteMask;
	}
};

/// <summary>Vulkan display type.</summary>
#if defined(ANDROID)
typedef ANativeWindow* NativeWindow;//!< Android native window
typedef NativeWindow NativeDisplay;//!< Android display
#elif defined(WIN32)
typedef void* NativeWindow;//!< Windows Native window
typedef void* NativeDisplay;//!< Windows native display
#elif defined(X11)
typedef void* NativeWindow;//!< X11 native window
typedef void* NativeDisplay;//!< X11 native display
#else
typedef void* NativeWindow;//!< Nullws Native window
typedef VkDisplayKHR NativeDisplay;//!< Nullws native display
#endif
typedef VkSurfaceKHR NativeSurface;//!< Vulkan surface

const uint32_t SubpassExternal = ~0u;//!< A special constant used as a subpass externl in Subpass dependecies

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
		memcpy(bytes, &depth, 4);
		memcpy(bytes + 4, &stencil, 4);
	}

	/// <summary>Constructor. Initialise with rgba floating point</summary>
	/// <param name="r">Red component</param>
	/// <param name="g">Green component</param>
	/// <param name="b">Blue component</param>
	/// <param name="a">Alpha component</param>
	ClearValue(float r, float g, float b, float a)
	{
		memcpy(bytes, &r, 4);
		memcpy(bytes + 4, &g, 4);
		memcpy(bytes + 8, &b, 4);
		memcpy(bytes + 12, &a, 4);
	}

	/// <summary>Constructor. Initialise with rgba interger</summary>
	/// <param name="r">Red component</param>
	/// <param name="g">Green component</param>
	/// <param name="b">Blue component</param>
	/// <param name="a">Alpha component</param>
	ClearValue(int32_t r, int32_t g, int32_t b, int32_t a)
	{
		memcpy(bytes, &r, 4);
		memcpy(bytes + 4, &g, 4);
		memcpy(bytes + 8, &b, 4);
		memcpy(bytes + 12, &a, 4);
	}

	/// <summary>Constructor. Initialise with rgba unsigned interger</summary>
	/// <param name="r">Red component</param>
	/// <param name="g">Green component</param>
	/// <param name="b">Blue component</param>
	/// <param name="a">Alpha component</param>
	ClearValue(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
	{
		memcpy(bytes, &r, 4);
		memcpy(bytes + 4, &g, 4);
		memcpy(bytes + 8, &b, 4);
		memcpy(bytes + 12, &a, 4);
	}

	/// <summary>Set rgba color clear value</summary>
	/// <param name="r">Red component</param>
	/// <param name="g">Green component</param>
	/// <param name="b">Blue component</param>
	/// <param name="a">Alpha component</param>
	void SetColorValue(float r, float g, float b, float a)
	{
		memcpy(bytes, &r, sizeof(float));
		memcpy(bytes, &g, sizeof(float));
		memcpy(bytes, &b, sizeof(float));
		memcpy(bytes, &a, sizeof(float));
	}

	/// <summary>Set depth stencil clear value</summary>
	/// <param name="depth">Depth clear value, default 1</param>
	/// <param name="stencil">Stencil clear value, default 0</param>
	void SetDepthStencilValue(float depth = 1.f, uint32_t stencil = 0u)
	{
		memcpy(bytes, &depth, sizeof(float));
		memcpy(bytes, &stencil, sizeof(uint32_t));
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
struct ClearAttachment
{
	VkImageAspectFlags    aspectMask;//!< Mask selecting the color, depth and/or stencil aspects of the attachment to be cleared.
	uint32_t              colorAttachment;//!< Index of the renderpass color attachment. It is only meaningful if VkImageAspectFlags::e_ASPECT_COLOR_BIT is set in aspectMask
	ClearValue            clearValue; //!< color or depth/stencil value to clear the attachment

	/// <summary>Constructor. Initialisation undefined.</summary>
	ClearAttachment() {}

	/// <summary>Constructor.</summary>
	/// <param name="aspectMask">Mask selecting the color, depth and/or stencil aspects of the attachment to be cleared.</param>
	/// <param name="colorAttachment">Index of the renderpass color attachment. It is only meaningful if VkImageAspectFlags::e_ASPECT_COLOR_BIT is set in aspectMask</param>
	/// <param name="clearValue">color or depth/stencil value to clear the attachment</param>
	ClearAttachment(VkImageAspectFlags aspectMask, uint32_t colorAttachment, const ClearValue& clearValue) :
		aspectMask(aspectMask), colorAttachment(colorAttachment), clearValue(clearValue)
	{}

	/// <summary>Create stencil-clear-attachment factory function</summary>
	/// <param name="stencil">Stencil clear value</param>
	/// <returns>Returns stencil clear attachment</returns>
	static ClearAttachment createStencilClearAttachment(uint32_t stencil)
	{
		return ClearAttachment(VkImageAspectFlags::e_STENCIL_BIT, 0, ClearValue::createStencilClearValue(stencil));
	}

	/// <summary>Create depth-clear-attachment factory function</summary>
	/// <param name="depth">Depth clear value</param>
	/// <param name="stencil">Stencil clear value</param>
	/// <returns>Returns depth-stencil clear attachment</returns>
	static ClearAttachment createDepthStencilClearAttachment(float depth, uint32_t stencil)
	{
		return ClearAttachment(VkImageAspectFlags::e_DEPTH_BIT | VkImageAspectFlags::e_STENCIL_BIT,
		                       0, ClearValue::createDepthStencilClearValue(depth, stencil));
	}

	/// <summary>Create color-clear-attachmemnt factory function</summary>
	/// <param name="colorAttachment">Index of the renderpass color attachment.</param>
	/// <param name="clearValue">Color clear values</param>
	/// <returns>Returns color clear attachment</returns>
	static ClearAttachment createColorClearAttachment(uint32_t colorAttachment, const ClearValue& clearValue)
	{
		return ClearAttachment(VkImageAspectFlags::e_COLOR_BIT, colorAttachment, clearValue);
	}
};

/// <summary>ClearRect structures defining regions within each selected attachment to clear in Commandbuffer::clearAttachments command</summary>
struct ClearRect
{
	Rect2Di  rect;//!< Clear area
	uint32_t baseArrayLayer;//!< Base array layer to clear from
	uint32_t layerCount;//!< Number of layers to clear

	/// <summary>Constructor</summary>
	/// <param name="rect">Clear area</param>
	/// <param name="baseArrayLayer">Base array layer to clear from</param>
	/// <param name="layerCount">Number of layers to clear</param>
	ClearRect(const Rect2Di& rect = Rect2Di(), uint32_t baseArrayLayer = 0, uint32_t layerCount = 1) :
		rect(rect), baseArrayLayer(baseArrayLayer), layerCount(layerCount) {}

	/// <summary>Constructor</summary>
	/// <param name="x">Clear area x</param>
	/// <param name="y">Clear area y</param>
	/// <param name="width">Clear area width</param>
	/// <param name="height">Clear area height</param>
	/// <param name="baseArrayLayer">Base array layer to clear from</param>
	/// <param name="layerCount">Number of layers to clear</param>
	ClearRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t baseArrayLayer = 0,
	          uint32_t layerCount = 1)
		: rect(x, y, width, height), baseArrayLayer(baseArrayLayer), layerCount(layerCount)
	{}
};

/// <summary>Return true if the format is a depth stencil format</summary>
/// <param name="format">Format to querry</param>
/// <returns>bool</returns>
inline bool isFormatDepthStencil(VkFormat format)
{
	return format >= VkFormat::e_D16_UNORM && format <= VkFormat::e_D32_SFLOAT_S8_UINT;
}

/// <summary>Viewport specifes the drawing region, min and max depth.
/// The viewport region x and y starts bottom left as similar to opengl
/// </summary>
struct Viewport
{
	float x;//!< region x
	float y;//!< region y
	float width;//!< region width
	float height;//!< region height
	float minDepth;//!< min depth
	float maxDepth;//!< max depth

	/// <summary>Constructor, sets from individual values</summary>
	/// <param name="x">viewport x</param>
	/// <param name="y">viewport y</param>
	/// <param name="width">viewport width</param>
	/// <param name="height">viewport height</param>
	/// <param name="minDepth">depth min</param>
	/// <param name="maxDepth">depth max</param>
	Viewport(float x = 0, float y = 0, float width = 1, float height = 1,
	         float minDepth = 0.f, float maxDepth = 1.f) :
		x(x), y(y), width(width), height(height), minDepth(minDepth), maxDepth(maxDepth) {}

	/// <summary>Constructor, from individual values</summary>
	/// <param name="rect">viewport</param>
	/// <param name="minDepth">depth min</param>
	/// <param name="maxDepth">depth max</param>
	Viewport(const Rect2Di& rect, float minDepth = 0.f, float maxDepth = 1.f) :
		x(static_cast<float>(rect.offset.x)), y(static_cast<float>(rect.offset.y)), width(static_cast<float>(rect.extent.width)), height(static_cast<float>(rect.extent.height)),
		minDepth(minDepth), maxDepth(maxDepth) {}
};

/// <summary>Contains information about the querried queue properties on a physical device.</summary>
struct QueueFamilyProperties
{
	VkQueueFlags  queueFlags;//!< Bitmask of VkQueueFlagBits indicating capabilities of the queues in this queue family.
	uint32_t    numQueues;//!< Unsigned integer count of queues in this queue family
	uint32_t    timestampValidBits;//!< Unsigned integer count of meaningful bits in the timestamps written via Commandbuffer::CmdWriteTimestamp. The valid range for the count is 36..64 bits, or a value of 0, indicating no support for timestamps. Bits outside the valid range are guaranteed to be zeros
	Extent3D    minImageTransferGranularity;//!< Minimum granularity supported for image transfer operations on the queues in this queue family
};


namespace impl {
//!\cond NO_DOXYGEN
inline VkImageSubresourceLayers convertToVk(const ImageSubresourceLayers& layers)
{
	return VkImageSubresourceLayers
	{
		layers.aspectMask,
		layers.mipLevel,
		layers.baseArrayLayer,
		layers.layerCount
	};
}

inline VkImageSubresource convertToVk(const ImageSubresource& resource)
{
	return VkImageSubresource
	{
		resource.aspectMask,
		resource.mipLevel,
		resource.arrayLayer
	};
}

inline VkImageSubresourceRange convertToVk(const ImageSubresourceRange& range)
{
	return VkImageSubresourceRange
	{
		range.aspectMask, range.baseMipLevel, range.levelCount, range.baseArrayLayer, range.layerCount
	};
}

inline VkImageCopy convertToVk(const ImageCopy& cpy)
{
	return VkImageCopy
	{
		convertToVk(cpy.srcSubresource),
		VkOffset3D{ cpy.srcOffset.x, cpy.srcOffset.y, cpy.srcOffset.z },
		convertToVk(cpy.dstSubresource),
		VkOffset3D{ cpy.dstOffset.x, cpy.dstOffset.y, cpy.dstOffset.z },
		VkExtent3D{ cpy.imageExtent.width, cpy.imageExtent.height, cpy.imageExtent.depth }
	};
}

inline VkBufferImageCopy convertToVk(const BufferImageCopy& cpy)
{
	return VkBufferImageCopy
	{
		cpy.bufferOffset,
		cpy.bufferRowLength,
		cpy.bufferImageHeight,
		convertToVk(cpy.imageSubresource),
		VkOffset3D{cpy.imageOffset.x, cpy.imageOffset.y, cpy.imageOffset.z},
		VkExtent3D{cpy.imageExtent.width, cpy.imageExtent.height, cpy.imageExtent.depth}
	};
}
//!\endcond
}
}
#undef DEFINE_ENUM_OPERATORS

