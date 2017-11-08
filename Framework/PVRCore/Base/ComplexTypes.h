/*!
\brief Contains structures, classes and enums used throughout the framework. Conceptually sits higher than Types.h
\file PVRCore/Base/ComplexTypes.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Base/Types.h"

#include <string>

namespace pvr {

/// <summary>Contains a full description of a Vertex Attribute: Index, format, number of elements, offset in the
/// buffer, optionally name. All values (except attributeName) must be set explicitly.</summary>
struct VertexAttributeInfo
{
	uint16_t index;     //!< Attribute index
	DataType format; //!< Data type of each element of the attribute
	uint8_t width;      //!< Number of elements in attribute, e.g 1,2,3,4
	uint32_t offsetInBytes; //!< Offset of the first element in the buffer
	std::string attribName;   //!< Optional: Name(in the shader) of the attribute

	/// <summary>Default constructor. Uninitialized values, except for AttributeName.</summary>
	VertexAttributeInfo() : index(0), format(DataType::None),
		width(0),
		offsetInBytes(0),
		attribName("") {}

	/// <summary>Create a new VertexAttributeInfo object.</summary>
	/// <param name="index">Attribute binding index</param>
	/// <param name="format">Attribute data type</param>
	/// <param name="width">Number of elements in attribute</param>
	/// <param name="offsetInBytes">Interleaved: offset of the attribute from the start of data of each vertex</param>
	/// <param name="attribName">Name of the attribute in the shader.</param>
	VertexAttributeInfo(uint16_t index, DataType format, uint8_t width,
	                    uint32_t offsetInBytes,
	                    const char* attribName = "") :
		index(index), format(format), width(width), offsetInBytes(offsetInBytes), attribName(attribName) {}

	/// <summary>Return true if the right hand object is equal to this</summary>
	/// <param name=rhs>The right hand side of the operator</param>
	/// <returns>True if index, format, width and offset are all equal, otherwise false</returns>
	bool operator==(VertexAttributeInfo const& rhs)const
	{
		return ((index == rhs.index) && (format == rhs.format) &&
		        (width == rhs.width) && (offsetInBytes == rhs.offsetInBytes));
	}

	/// <summary>Return true if the right hand object is not equal to this</summary>
	/// <param name=rhs>The right hand side of the operator</param>
	/// <returns>True if at least one of index, format, width and offset is not equal,
	/// otherwise false </returns>
	bool operator!=(VertexAttributeInfo const& rhs)const { return !((*this) == rhs); }

};


/// <summary>Information about a Buffer binding: Binding index, stride, (instance) step rate.</summary>
struct VertexInputBindingInfo
{
	uint16_t bindingId;//!< buffer binding index
	uint32_t strideInBytes; //!< buffer stride in bytes
	StepRate stepRate;//!< buffer step rate

	/// <summary>Construct with Uninitialized values.</summary>
	VertexInputBindingInfo() {}

	/// <summary>Add a buffer binding.</summary>
	/// <param name="bindId">Buffer binding point</param>
	/// <param name="strideInBytes">Buffer stride of each vertex attribute to the next</param>
	/// <param name="stepRate">Vertex Attribute Step Rate</param>
	VertexInputBindingInfo(uint16_t bindId, uint32_t strideInBytes, StepRate stepRate = StepRate::Vertex) :
		bindingId(bindId), strideInBytes(strideInBytes), stepRate(stepRate) {}
};


/// <summary>A container struct carrying Vertex Attribute information (vertex layout, plus binding point)</summary>
struct VertexAttributeInfoWithBinding : public VertexAttributeInfo
{
	/// <summary>The Vertex Buffer binding point this attribute is bound to</summary>
	uint16_t binding;

	/// <summary>Constructor</summary>
	VertexAttributeInfoWithBinding() {}

	/// <summary>Constructor from VertexAttributeInfo and Binding</summary>
	/// <param name="nfo">A vertexAttributeInfo</param>
	/// <param name="binding">The VBO binding index from where this vertex attribute will be sourced</param>
	VertexAttributeInfoWithBinding(const VertexAttributeInfo& nfo, uint16_t binding) :
		VertexAttributeInfo(nfo), binding(binding) {}

	/// <summary>Constructor from individual values</summary>
	/// <param name="index">The index of the vertex attribute</param>
	/// <param name="format">The vertex attribute format</param>
	/// <param name="width">The number of elements in the vertex attribute (e.g. 4 for a vec4)</param>
	/// <param name="offsetInBytes">The offset of the vertex attribute from the start of the buffer</param>
	/// <param name="binding">The VBO binding index from where this vertex attribute will be sourced</param>
	/// <param name="attribName">The Attribute name (optional, only required/supported in some apis)</param>
	VertexAttributeInfoWithBinding(
	  uint16_t index, DataType format, uint8_t width, uint32_t offsetInBytes, uint16_t binding,
	  const char* attribName = "")
		: VertexAttributeInfo(index, format, width, offsetInBytes, attribName), binding(binding) {}
};

/// <summary>The Vertical Synchronization (or lack thereof) modes, A.K.A. Presentation mode.</summary>
enum class VsyncMode
{
	Off,//!<The application does not synchronizes with the vertical sync. If application renders faster than the display refreshes, frames are wasted and tearing may be observed. FPS is uncapped. Maximum power consumption. If unsupported, "ON" value will be used instead. Minimum latency.
	On,//!<The application is always syncrhonized with the vertical sync. Tearing does not happen. FPS is capped to the display's refresh rate. For fast applications, battery life is improved. Always supported.
	Relaxed,//!<The application synchronizes with the vertical sync, but only if the application rendering speed is greater than refresh rate. Compared to OFF, there is no tearing. Compared to ON, the FPS will be improved for "slower" applications. If unsupported, "ON" value will be used instead. Recommended for most applications. Default if supported.
	Mailbox, //!<The presentation engine will always use the latest fully rendered image. Compared to OFF, no tearing will be observed. Compared to ON, battery power will be worse, especially for faster applications. If unsupported,  "OFF" will be attempted next.
	Half, //!<The application is capped to using half the vertical sync time. FPS artificially capped to Half the display speed (usually 30fps) to maintain battery. Best possible battery savings. Worst possibly performance. Recommended for specific applications where battery saving is critical.
};

/// <summary>Contains display configuration information (width, height, position, title, bpp etc.).</summary>
class DisplayAttributes
{
public:
	/// <summary> Unnamed enum for constants</summary>
	enum
	{
		PosDefault = -1 //!< Sentinel value for Default position
	};

	std::string  windowTitle; //!<Title of the application window

	uint32_t width; //!< Width of the rendering area (default 800)
	uint32_t height;  //!< Height of the rendering (default 600)
	uint32_t x; //!< Horizontal offset of the bottom-left area (default 0)
	uint32_t y; //!< Vertical offset of the bottom-left area (default 0)

	uint32_t depthBPP; //!< Number of bits per pixel in the depth buffer (default 16)
	uint32_t stencilBPP; //!< Number of bits per pixel of the stencil buffer (default 0: no stencil)

	uint32_t redBits; //!< Number of bits of the red channel of the framebuffer (default 8)
	uint32_t greenBits;//!< Number of bits of the green channel of the framebuffer (default 8)
	uint32_t blueBits;//!< Number of bits of the blue channel of the framebuffer (default 8)
	uint32_t alphaBits;//!< Number of bits of the alpha channel of the framebuffer (default 8)

	uint32_t aaSamples; //!< Number of (antialiasing) samples of the framebuffer (default 0)

	uint32_t configID; //!< Deprecated: EGL config id

	VsyncMode vsyncMode; //!< Type of syncrhonization mode (default On: Vsync)
	int32_t contextPriority; //!< Context priority, if supported (default High)
	int32_t swapLength; //!< Swapchain length, AKA number of framebuffer images (default 0: Minimum required for technique)

	bool forceColorBPP; //!< Require that the color channels of the framebuffer are exactly as requested in redBits/blueBits/greenBits/alphaBits (default false)
	bool fullscreen; //!< If true, application will be fullscreen (if supported). If false, application will be windowed (if supported). (default false)
	bool frameBufferSrgb; //!< If true and supported, attempt to use an sRGB framebuffer format (default false)

	// Default constructor
	DisplayAttributes() :
		width(800u),
		height(600u),
		x(static_cast<uint32_t>(0)),
		y(static_cast<uint32_t>(0)),
		depthBPP(32u),
		stencilBPP(0u),
		redBits(8u),
		greenBits(8u),
		blueBits(8u),
		alphaBits(8u),
		aaSamples(0u),
		configID(0u),
		vsyncMode(VsyncMode::On),
		contextPriority(2),
		swapLength(0),
		forceColorBPP(false),
		fullscreen(false),
		frameBufferSrgb(false)
	{
	}
	/// <summary>Checks if the screen is rotated.</summary>
	/// <returns>True if the screen is Portrait, otherwise (if landscape) false .</returns>
	bool isScreenRotated()const { return height > width; }
	/// <summary>Checks if full screen.</summary>
	/// <returns>True if full screen, otherwise false.</returns>
	bool isFullScreen()const { return fullscreen; }
};

/// <summary>Native display type.</summary>
typedef void* OSDisplay;

/// <summary>Native window type.</summary>
typedef void* OSWindow;

/// <summary>Native application type.</summary>
typedef void* OSApplication;

/// <summary>Native application data type.</summary>
typedef void* OSDATA;

/// <summary>Enumeration of texture Swizzle mask channels.</summary>
enum class Swizzle : uint8_t //DIRECT VULKAN MAPPING - DO NOT REARRANGE
{
	Identity = 0,
	//Unset = 0,
	Zero = 1,
	One = 2,
	R = 3,
	G = 4,
	B = 5,
	A = 6,
	Red = R,
	Green = G,
	Blue = B,
	Alpha = A,
};

/// <summary>The SwizzleChannels struct</summary>
struct SwizzleChannels
{
	Swizzle r;//!< Swizzle R channel
	Swizzle g;//!< Swizzle G channel
	Swizzle b;//!< Swizzle B channel
	Swizzle a;//!< Swizzle A channel

	/// <summary>SwizzleChannels. Default: All channels are set to identity</summary>
	SwizzleChannels() : r(Swizzle::Identity), g(Swizzle::Identity), b(Swizzle::Identity), a(Swizzle::Identity) { }

	/// <summary>SwizzleChannels</summary>
	/// <param name="r">Swizzle R channel</param>
	/// <param name="g">Swizzle G channel</param>
	/// <param name="b">Swizzle B channel</param>
	/// <param name="a">Swizzle A channel</param>
	SwizzleChannels(Swizzle r, Swizzle g, Swizzle b, Swizzle a) : r(r), g(g), b(b), a(a) { }
};

/// <summary>Structure describes the number of array levels and mip levels an image contains</summary>
struct ImageLayersSize
{
	uint16_t numArrayLevels;  //!< The number of array slices of the range
	uint16_t numMipLevels;   //!< The number of mipmap levels of the range

	/// <summary>Constructor. All arguments optional.</summary>
	/// <param name="numArrayLevels">The number of array levels represented by this range. (Default 1).</param>
	/// <param name="numMipLevels">The number of mipmap levels represented by this range. (Default 1).</param>
	ImageLayersSize(uint16_t numArrayLevels = 1, uint8_t numMipLevels = 1) :
		numArrayLevels(numArrayLevels), numMipLevels(numMipLevels) {}
};

/// <summary>Describes a single "layer" of an image: a single array layer of a single mip level, or the offset of a
/// layer range.</summary>
struct ImageSubresource
{
	ImageAspectFlags aspect; //!< The Aspect of the subresource (Color, Depth, Stencil, Depth&Stencil)
	uint16_t arrayLayerOffset; //!< The index of the array slice. In case of a range, the offset of the first layer.
	uint16_t mipLevelOffset;   //!< The index of the mipmap level. In case of a range, the offset of the first mipmap level.
	/// <summary>Constructor. All arguments optional.</summary>
	/// <param name="mipLevelOffset">The index of the array slice. In case of a range, the offset of the first layer.
	/// (Default 0)</param>
	/// <param name="arrayLayerOffset">The index of the mipmap level. In case of a range, the offset of the first
	/// mipmap level. (Default 0)</param>
	/// <param name="aspectFlags">The aspect(s) of the subresource (Color/Depth/Stencil/DepthStencil)</param>
	ImageSubresource(ImageAspectFlags aspectFlags = ImageAspectFlags::Color,
	                 uint16_t mipLevelOffset = 0, uint16_t arrayLayerOffset = 0) :
		aspect(aspectFlags), arrayLayerOffset(arrayLayerOffset), mipLevelOffset(mipLevelOffset) {}
};

/// <summary>Contains a 2d Integer size (width, height)</summary>
template<typename T>
struct GenericExtent2D
{
	T width; //!<Size along X axis
	T height; //!<Size along Y axis
	/// <summary>Constructor by width and height</summary>
	/// <param name="width">Horizontal size</param>
	/// <param name="height">Vertical size</param>
	GenericExtent2D(T width = 0, T height = 0) : width(width), height(height) {}
};

/// <summary>Contains a 3D Integer size (width, height, depth)</summary>
template<typename Txy, typename Tz>
struct GenericExtent3D : public GenericExtent2D<Txy>
{
	Tz depth; //!< Size along Z axis
	GenericExtent3D() {}

	/// <summary>Constructor. Defaults to (1,1,1)</summary>
	/// <param name="width">Horizontal size (default 1)</param>
	/// <param name="height">Vertical size (default 1)</param>
	/// <param name="depth">Depth size (default 1)</param>
	GenericExtent3D(Txy width, Txy height, Tz depth = 1) : GenericExtent2D<Txy>(width, height), depth(depth)
	{
	}

	/// <summary>Constructor from GenericExtent2D)</summary>
	/// <param name="extent2D">Vertical and horizontal size</param>
	/// <param name="depth">Depth size (default 1)</param>
	GenericExtent3D(const GenericExtent2D<Txy>& extent2D, Tz depth = 1) : GenericExtent2D<Txy>(extent2D), depth(depth)
	{
	}
};

/// <summary>The GenericOffset2D contains a 16-bit 2D offset (offsetX, offsetY)</summary>
template<typename T>
struct GenericOffset2D
{
	T x;//!< offset in x axis
	T y;//!< offset in y axis
	/// <summary>Constructor. Defaults to (0,0)</summary>
	/// <param name="offsetX">Offset in the X direction. (Default 0)</param>
	/// <param name="offsetY">Offset in the Y direction. (Default 0)</param>
	GenericOffset2D(T offsetX = 0, T offsetY = 0) : x(offsetX), y(offsetY)
	{ }

	/// <summary>Get the componentwise distance (Horizontal, Vertical) between this and another Offset2D.</summary>
	/// <param name="offset">The offset to which to get the distance</param>
	/// <returns>A 2d vector where the X is the distance along the X components, and the Y is the difference between the Y components</returns>
	glm::vec2 distanceTo(const GenericOffset2D& offset)const
	{
		return glm::vec2(offset.x - x, offset.y - y);
	}
	/// <summary>Sum this Offset with an Extent</summary>
	/// <param name="rhs">The right hand side of the additions</param>
	/// <returns>The result of this offset plus the extent rhs</returns>
	GenericOffset2D operator+(const GenericExtent2D<typename std::make_unsigned<T>::type>& rhs)const { return GenericOffset2D(*this) += rhs; }
	/// <summary>Add an Extent to this Offset</summary>
	/// <param name="rhs">The right hand side of the additions</param>
	/// <returns>This object, which now contains This offset plus the extent rhs</returns>
	GenericOffset2D& operator+=(const GenericExtent2D<typename std::make_unsigned<T>::type>& rhs) { x += rhs.width; y += rhs.height ; return *this; }
};

/// <summary>The GenericOffset3D contains the offsets in 3 dimension (offsetX, offsetY, offsetZ)</summary>
template<typename Txy, typename Tz>
struct GenericOffset3D : public GenericOffset2D<Txy>
{
	Tz z;//!< offset in z axis
	/// <summary>Constructor. Defaults to (0,0,0)</summary>
	/// <param name="offsetX">Offset in the X direction. (Default 0)</param>
	/// <param name="offsetY">Offset in the Y direction. (Default 0)</param>
	/// <param name="offsetZ">Offset in the Z direction. (Default 0)</param>
	GenericOffset3D(Txy offsetX = 0, Txy offsetY = 0, Tz offsetZ = 0) : GenericOffset2D<Txy>(offsetX, offsetY), z(offsetZ)
	{ }
	/// <summary>Sum this Offset with an Extent</summary>
	/// <param name="rhs">The right hand side of the additions</param>
	/// <returns>The result of this offset plus the extent rhs</returns>
	GenericOffset3D operator+(const GenericExtent3D<typename std::make_unsigned<Txy>::type, typename std::make_unsigned<Tz>::type>& rhs)const { return GenericOffset3D(*this) += rhs; }
	/// <summary>Add an Extent to this Offset</summary>
	/// <param name="rhs">The right hand side of the additions</param>
	/// <returns>This object, which now contains This offset plus the extent rhs</returns>
	GenericOffset3D& operator+=(const GenericExtent3D<typename std::make_unsigned<Txy>::type, typename std::make_unsigned<Tz>::type>& rhs) { GenericOffset2D<Txy>::x += rhs.width; GenericOffset2D<Txy>::y += rhs.height; z += rhs.height; return *this; }

	/// <summary>Constructor</summary>
	/// <param name="offsetXY">The 2D part of the offset</param>
	/// <param name="offsetZ">The depth of the offset. (Default 0)</param>
	GenericOffset3D(const GenericOffset2D<Txy>& offsetXY, Tz offsetZ = 0) : GenericOffset2D<Txy>(offsetXY), z(offsetZ)
	{ }
};

/// <summary> A 2D, integer Offset typically used for Images</summary>
typedef GenericOffset2D<int32_t> Offset2D;
/// <summary> A 3D, integer Offset typically used for 3D Images</summary>
typedef GenericOffset3D<int32_t, int32_t> Offset3D;

/// <summary> A 2D, integer Extent typically used for Images</summary>
typedef GenericExtent2D<uint32_t> Extent2D;
/// <summary> A 3D, integer Extent  typically used for 3D Images</summary>
typedef GenericExtent3D<uint32_t, uint32_t> Extent3D;

/// <summary>Represents a subresource range: A specified range of Array Layers and Mipmap levels of specific aspect of an
/// image</summary>
struct ImageSubresourceRange : public ImageLayersSize, public ImageSubresource
{
	/// <summary>Constructor</summary>
	ImageSubresourceRange() {}

	/// <summary>Constructor</summary>
	/// <param name="layersSize">Layers size</param>
	/// <param name="baseLayers">Base layers</param>
	ImageSubresourceRange(const ImageLayersSize& layersSize, const ImageSubresource& baseLayers) :
		ImageLayersSize(layersSize), ImageSubresource(baseLayers) {}
};

/// <summary>Represents a specific subresource layer: A specified Array Layer and Mipmap level of specific aspect of an
/// image</summary>
struct ImageSubresourceLayers : public ImageSubresource
{
	uint16_t numArrayLayers; //!< Number of array layers
	/// <summary>Constructor</summary>
	ImageSubresourceLayers() : numArrayLayers(1) {}
	/// <summary>Constructor</summary>
	/// <param name="baseLayers">Base layers</param>
	/// <param name="numArrayLayers">Number of array layers</param>
	ImageSubresourceLayers(ImageSubresource baseLayers, uint16_t numArrayLayers) :
		ImageSubresource(baseLayers), numArrayLayers(numArrayLayers) {}
};

/// <summary>Represents a specific 3-D range in an image (an orthogonal cuboid anywhere in the image)</summary>
struct ImageRange : public Extent3D, public Offset3D
{
	/// <summary>ImageRange</summary>
	ImageRange() {}

	/// <summary>ImageRange</summary>
	/// <param name="extents"></param>
	/// <param name="offset"></param>
	ImageRange(const Extent3D& extents, const Offset3D& offset) :
		Extent3D(extents), Offset3D(offset) {}
};

/// <summary>Represents an image resolve operation</summary>
struct ImageResolveRange
{
	Offset3D   srcOffset;//!< Source Region initial offset
	Offset3D   dstOffset;//!< Destination Region initial offset
	Extent3D   extent;//!< Size of the regions (as src must be equal to dst)
	ImageSubresourceLayers srcSubResource; //!< Source region subresource layers
	ImageSubresourceLayers dstSubResource; //!< Destination region subresource layers

	/// <summary>Constructor</summary>
	ImageResolveRange() {}

	/// <summary>Constructor</summary>
	/// <param name="srcOffset0">The source region's offset (bottom-left corner)</param>
	/// <param name="dstOffset0">The destination region's offset (bottom-left corner)</param>
	/// <param name="extent0">Size of both Source and Destination regions (as src must be equal to dst)</param>
	/// <param name="srcSubResource">Source region subresource layers</param>
	/// <param name="dstSubResource">Destination region subresource layers</param>
	ImageResolveRange(const Offset3D& srcOffset0, const Offset3D& dstOffset0, const Extent3D& extent0,
	                  const ImageSubresourceLayers& srcSubResource = ImageSubresourceLayers(),
	                  const ImageSubresourceLayers& dstSubResource = ImageSubresourceLayers())
	{
		srcOffset = srcOffset0;
		dstOffset = dstOffset0;
		extent = extent0;
		this->srcSubResource = srcSubResource;
		this->dstSubResource = dstSubResource;
	}
};

/// <summary>This class contains all the information of a Vertex Attribute's layout inside a block of memory,
/// typically a Vertex Buffer Object. This informations is normally the DataType of the attribute, the Offset (from
/// the beginning of the array) and the width (how many values of type DataType form an attribute).</summary>
struct VertexAttributeLayout
{

	DataType dataType; //!< Type of data of the vertex data
	uint16_t offset; //!< Offset, in bytes, of this vertex attribute
	uint8_t width; //!< Number of values per vertex

	/// <summary>VertexAttributeLayout</summary>
	VertexAttributeLayout() {}

	/// <summary>VertexAttributeLayout</summary>
	/// <param name="dataType"></param>
	/// <param name="width"></param>
	/// <param name="offset"></param>
	VertexAttributeLayout(DataType dataType, uint8_t width, uint16_t offset) :
		dataType(dataType), offset(offset), width(width) {}
};


/// <summary>Add blending configuration for a color attachment. Some API's only support one blending state for all
/// attachments, in which case the 1st such configuration will be used for all.</summary>
/// <remarks>--- Defaults --- Blend Enabled:false, Source blend Color factor: false, Destination blend Color
/// factor: Zero, Source blend Alpha factor: Zero, Destination blending Alpha factor :Zero, Blending operation
/// color: Add, Blending operation alpha: Add, Channel writing mask: All</remarks>
struct BlendingConfig
{
	bool        blendEnable;  //!< Enable blending
	BlendFactor  srcBlendColor;  //!< Source Blending color factor
	BlendFactor  dstBlendColor; //!< Destination blending color factor
	BlendFactor  srcBlendAlpha;  //!< Source blending alpha factor
	BlendFactor  dstBlendAlpha; //!< Destination blending alpha factor
	BlendOp    blendOpColor; //!< Blending operation color
	BlendOp    blendOpAlpha; //!< Blending operation alpha
	ColorChannelFlags channelWriteMask;//!<Channel writing mask

	/// <summary>Create a blending state. Separate color/alpha factors.</summary>
	/// <param name="blendEnable">Enable blending (default false)</param>
	/// <param name="srcBlendColor">Source Blending color factor (default:Zero)</param>
	/// <param name="dstBlendColor">Destination blending color factor (default:Zero)</param>
	/// <param name="srcBlendAlpha">Source blending alpha factor (default:Zero)</param>
	/// <param name="dstBlendAlpha">Destination blending alpha factor (default:Zero)</param>
	/// <param name="blendOpColor">Blending operation color (default:Add)</param>
	/// <param name="blendOpAlpha">Blending operation alpha (default:Add)</param>
	/// <param name="channelWriteMask">Channel writing mask (default:All)</param>
	BlendingConfig(
	  bool blendEnable = false, BlendFactor srcBlendColor = BlendFactor::One,
	  BlendFactor dstBlendColor = BlendFactor::Zero,
	  BlendFactor srcBlendAlpha = BlendFactor::One,
	  BlendFactor dstBlendAlpha = BlendFactor::Zero,
	  BlendOp blendOpColor = BlendOp::Add,
	  BlendOp blendOpAlpha = BlendOp::Add,
	  ColorChannelFlags channelWriteMask = ColorChannelFlags::All) :
		blendEnable(blendEnable), srcBlendColor(srcBlendColor), dstBlendColor(dstBlendColor),
		srcBlendAlpha(srcBlendAlpha), dstBlendAlpha(dstBlendAlpha), blendOpColor(blendOpColor),
		blendOpAlpha(blendOpAlpha), channelWriteMask(channelWriteMask) {}

	/// <summary>Create a blending state. Common color and alpha factors.</summary>
	/// <param name="blendEnable">Enable blending (default false)</param>
	/// <param name="srcBlendFactor">Source Blending factor</param>
	/// <param name="dstBlendFactor">Destination Blending factor</param>
	/// <param name="blendOpColorAlpha">Blending operation color & alpha (default:Add)</param>
	/// <param name="channelWriteMask">Channel writing mask (default:All)</param>
	BlendingConfig(bool blendEnable, BlendFactor srcBlendFactor,
	               BlendFactor dstBlendFactor, BlendOp blendOpColorAlpha,
	               ColorChannelFlags channelWriteMask = ColorChannelFlags::All) :
		blendEnable(blendEnable), srcBlendColor(srcBlendFactor), dstBlendColor(dstBlendFactor),
		srcBlendAlpha(srcBlendFactor), dstBlendAlpha(dstBlendFactor),
		blendOpColor(blendOpColorAlpha), blendOpAlpha(blendOpColorAlpha), channelWriteMask(channelWriteMask) {}
};

/// <summary>Pipeline Stencil state</summary>
struct StencilState
{
	StencilOp opDepthPass;//!< Action performed on samples that pass both the depth and stencil tests.
	StencilOp opDepthFail;//!< Action performed on samples that pass the stencil test and fail the depth test.
	StencilOp opStencilFail;//!< Action performed on samples that fail the stencil test.
	uint32_t compareMask;//!< Selects the bits of the unsigned Integer stencil values during in the stencil test.
	uint32_t writeMask;//!<  Selects the bits of the unsigned Integer stencil values updated by the stencil test in the stencil framebuffer attachment.
	uint32_t reference;//!< Integer reference value that is used in the unsigned stencil comparison.
	CompareOp compareOp;//!<  Comparison operator used in the stencil test.

	/// <summary>Constructor from all parameters</summary>
	/// <param name="depthPass">Action performed on samples that pass both the depth and stencil tests.</param>
	/// <param name="depthFail">Action performed on samples that pass the stencil test and fail the depth test.</param>
	/// <param name="stencilFail">Action performed on samples that fail the stencil test.</param>
	/// <param name="compareOp">Comparison operator used in the stencil test.</param>
	/// <param name="compareMask">Selects the bits of the unsigned Integer stencil values during in the stencil test.
	/// </param>
	/// <param name="writeMask">Selects the bits of the unsigned Integer stencil values updated by the stencil test in the
	/// stencil framebuffer attachment</param>
	/// <param name="reference">Integer reference value that is used in the unsigned stencil comparison.</param>
	StencilState(StencilOp depthPass = StencilOp::Keep,
	             StencilOp depthFail = StencilOp::Keep,
	             StencilOp stencilFail = StencilOp::Keep,
	             CompareOp compareOp = CompareOp::DefaultStencilFunc,
	             uint32_t compareMask = 0xff, uint32_t writeMask = 0xff, uint32_t reference = 0)
		: opDepthPass(depthPass), opDepthFail(depthFail),
		  opStencilFail(stencilFail), compareMask(compareMask),
		  writeMask(writeMask), reference(reference), compareOp(compareOp) {}
};


/// <summary>The DrawIndirectCmd struct. The structure contains the data for drawIndirect command buffer
/// </summary>
struct CmdDrawIndirect
{
	uint32_t  numVertices;  //!< Number of vertex to draw
	uint32_t  numInstances;  //!< Number of instance to draw
	uint32_t  firstVertex;  //!< First vertex in the buffer to begin
	uint32_t  firstInstance;  //!< First instance to begin
};

/// <summary>The DrawIndexedIndirectCmd struct. The structure contains the data for drawIndexedInsdirect command
/// buffer</summary>
struct CmdDrawIndexedIndirect
{
	uint32_t numIndices;//!<  The number of vertices to draw.
	uint32_t numInstances; //!< The number of instances to draw
	uint32_t firstIndex;//!< The base index within the index buffer.
	uint32_t vertexOffset;//!< The value added to the vertex index before indexing into the vertex buffer
	uint32_t firstInstance;//!< The instance ID of the first instance to draw.
};



/// <summary>Class wrapping an arithmetic type and providing bitwise operation for its bits</summary>
template<typename Storage_>
class Bitfield
{
public:
	/// <summary>Return true if a bit is set</summary>
	/// <param name="store">Bit storage</param>
	/// <param name="bit">Bit to check</param>
	/// <returns>True if the bit is set, otherwise false</returns>
	inline static bool isSet(Storage_ store, int8_t bit)
	{
		return (store & (1 << bit)) != 0;
	}

	/// <summary>Set a bit in the storage</summary>
	/// <param name="store">Storage</param>
	/// <param name="bit">Bit to set</param>
	inline static void set(Storage_ store, int8_t bit)
	{
		store |= (1 << bit);
	}

	/// <summary>Clear a bit fro the storage</summary>
	/// <param name="store">Storage</param>
	/// <param name="bit">Bit to clear</param>
	inline static void clear(Storage_ store, int8_t bit)
	{
		store &= (!(1 << bit));
	}

};
}
