/*!
\brief Contains structures, classes and enums used throughout the framework. Conceptually sits higher than Types.h
\file PVRCore/Base/ComplexTypes.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Log.h"
#include "PVRCore/Base/Types.h"
#include "../External/glm/glm.hpp"
namespace pvr {
namespace types {

/// <summary>Enumeration of texture Swizzle mask channels.</summary>
enum class Swizzle : uint8 //DIRECT VULKAN MAPPING - DO NOT REARRANGE
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
	uint16 numArrayLevels;  //!< The number of array slices of the range
	uint16 numMipLevels;   //!< The number of mipmap levels of the range

	/// <summary>Constructor. All arguments optional.</summary>
	/// <param name="numArrayLevels">The number of array levels represented by this range. (Default 1).</param>
	/// <param name="numMipLevels">The number of mipmap levels represented by this range. (Default 1).</param>
	ImageLayersSize(uint16 numArrayLevels = 1, uint8 numMipLevels = 1) :
		numArrayLevels(numArrayLevels), numMipLevels(numMipLevels) {}
};

/// <summary>Describes a single "layer" of an image: a single array layer of a single mip level, or the offset of a
/// layer range.</summary>
struct ImageSubresource
{
	uint16 arrayLayerOffset; //!< The index of the array slice. In case of a range, the offset of the first layer.
	uint16 mipLevelOffset;   //!< The index of the mipmap level. In case of a range, the offset of the first mipmap level.
	/// <summary>Constructor. All arguments optional.</summary>
	/// <param name="mipLevelOffset">The index of the array slice. In case of a range, the offset of the first layer.
	/// (Default 0)</param>
	/// <param name="arrayLayerOffset">The index of the mipmap level. In case of a range, the offset of the first
	/// mipmap level. (Default 0)</param>
	ImageSubresource(uint16 mipLevelOffset = 0, uint16 arrayLayerOffset = 0) :
		arrayLayerOffset(arrayLayerOffset), mipLevelOffset(mipLevelOffset) {}
};

/// <summary>Contains a 2d integer size (width, height)</summary>
template<typename T>
struct GenericExtent2D
{
	T width;
	T height;
	/// <summary>Constructor by width and height</summary>
	/// <param name="width">Horizontal size</param>
	/// <param name="height">Vertical size</param>
	GenericExtent2D(T width = 0, T height = 0) : width(width), height(height)
	{
		//PVR_ASSERTION(width < 65536 && height < 65536 && "Error - Max supported image extent must fit into a 16-bit unsigned integer");
	}
};

/// <summary>Contains a 3D integer size (width, height, depth)</summary>
template<typename Txy, typename Tz>
struct GenericExtent3D : public GenericExtent2D<Txy>
{
	Tz depth;
	GenericExtent3D() {}

	/// <summary>Constructor. Defaults to (1,1,1)</summary>
	/// <param name="width">Horizontal size (default 1)</param>
	/// <param name="height">Vertical size (default 1)</param>
	/// <param name="depth">Depth size (default 1)</param>
	GenericExtent3D(Txy width, Txy height, Tz depth = 1) : GenericExtent2D<Txy>(width, height), depth(depth)
	{
	}

	GenericExtent3D(const GenericExtent2D<Txy>& extent2D, Tz depth = 1) : GenericExtent2D<Txy>(extent2D), depth(depth)
	{
	}

};

/// <summary>The GenericOffset2D contains a 16-bit 2D offset (offsetX, offsetY)</summary>
template<typename T>
struct GenericOffset2D
{
	T offsetX;//!< offset in x axis
	T offsetY;//!< offset in y axis
	/// <summary>Constructor. Defaults to (0,0)</summary>
	/// <param name="offsetX">Offset in the X direction. (Default 0)</param>
	/// <param name="offsetY">Offset in the Y direction. (Default 0)</param>
	GenericOffset2D(T offsetX = 0, T offsetY = 0) : offsetX(offsetX), offsetY(offsetY)
	{ }

	glm::vec2 distanceTo(const GenericOffset2D& offset)const
	{
		return glm::ivec2(offset.offsetX - offsetX, offset.offsetY - offsetY);
	}

};

/// <summary>The GenericOffset3D contains the offsets in 3 dimension (offsetX, offsetY, offsetZ)</summary>
template<typename Txy, typename Tz>
struct GenericOffset3D : public GenericOffset2D<Txy>
{
	Tz offsetZ;//!< offset in z axis
	/// <summary>Constructor. Defaults to (0,0,0)</summary>
	/// <param name="offsetX">Offset in the X direction. (Default 0)</param>
	/// <param name="offsetY">Offset in the Y direction. (Default 0)</param>
	/// <param name="offsetZ">Offset in the Z direction. (Default 0)</param>
	GenericOffset3D(Txy offsetX = 0, Txy offsetY = 0, Tz offsetZ = 0) : GenericOffset2D<Txy>(offsetX, offsetY), offsetZ(offsetZ)
	{ }

	/// <summary>Constructor</summary>
	/// <param name="offsetXY">The 2D part of the offset</param>
	/// <param name="offsetZ">The depth of the offset. (Default 0)</param>
	GenericOffset3D(const GenericOffset2D<Txy>& offsetXY, Tz offsetZ = 0) : GenericOffset2D<Txy>(offsetXY), offsetZ(offsetZ)
	{ }
};

typedef GenericOffset2D<int16> Offset2D;
typedef GenericOffset3D<int16, int16> Offset3D;

typedef GenericExtent2D<uint16> Extent2D;
typedef GenericExtent3D<uint16, uint16> Extent3D;

/// <summary>Represents a single aspect of an image (Color, Depth, Stencil, Depth/Stencil)</summary>
struct ImageAspectRange
{
	types::ImageAspect aspect;

	/// <summary>Constructor. Defaults to Color aspect.</summary>
	/// <param name="aspect">The Image aspect (Default: Color)</param>
	ImageAspectRange(ImageAspect aspect = ImageAspect::Color) : aspect(aspect) {}
};

/// <summary>Represents a subresource range: A specified range of Array Layers and Mipmap levels of specific aspect of an
/// image</summary>
struct ImageSubresourceRange : public ImageLayersSize, public ImageSubresource, ImageAspectRange
{
	ImageSubresourceRange() {}
	ImageSubresourceRange(const ImageLayersSize& layersSize, const ImageSubresource& baseLayers,
	                      const ImageAspectRange& aspectRange = ImageAspectRange()) :
		ImageLayersSize(layersSize), ImageSubresource(baseLayers), ImageAspectRange(aspectRange) {}
};

/// <summary>Represents a specific subresource layer: A specified Array Layer and Mipmap level of specific aspect of an
/// image</summary>
struct ImageSubResourceLayers : public ImageSubresource, ImageAspectRange
{
	uint16 numArrayLayers;
	ImageSubResourceLayers() : numArrayLayers(1) {}
	ImageSubResourceLayers(ImageSubresource baseLayers, ImageAspectRange aspectRange, uint16 numArrayLayers) :
		ImageSubresource(baseLayers), ImageAspectRange(aspectRange), numArrayLayers(numArrayLayers) {}
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

/// <summary>Represents the geometric size (number of rows/columns/slices in x-y-z) and number of array and mipmap layers
/// of an image.</summary>
struct ImageAreaSize : public Extent3D, public ImageLayersSize
{
	ImageAreaSize() {}
	ImageAreaSize(const ImageLayersSize& layersSize, const Extent3D& extents) :
		Extent3D(extents), ImageLayersSize(layersSize) {}
};

/// <summary>Represents the geometric offset (distance from 0,0,0 in x-y-z) and offset of array and mipmap layers of an
/// image.</summary>
struct ImageAreaOffset : public ImageSubresource, public Offset3D
{
	ImageAreaOffset() {}
	ImageAreaOffset(const ImageSubresource& baseLayers, const Offset3D& offset) :
		ImageSubresource(baseLayers), Offset3D(offset) {}
};

/// <summary>Represents an exact range on an image: both a range of array and mipmap layers, and a geometric range
/// (offset and size in x-y-z)</summary>
struct ImageArea : public ImageAreaSize, ImageAreaOffset
{
	ImageArea() {}
	ImageArea(const ImageLayersSize& layerSize, const Extent3D& extents,
	          const ImageSubresource& baseLayers, const Offset3D& offset)
		: ImageAreaSize(layerSize, extents), ImageAreaOffset(baseLayers, offset)
	{
	}

	operator ImageSubresourceRange() const
	{
		return ImageSubresourceRange(*this, *this);
	}

};

/// <summary>Represents a blit operation through source and destination offsets</summary>
struct ImageBlitRange
{
	Offset3D        srcOffset[2];//!< Src Region end points (min, max)
	Offset3D        dstOffset[2];//!< Dst Region end points (min, max)
	ImageSubResourceLayers srcSubResource;
	ImageSubResourceLayers dstSubResource;

	ImageBlitRange() {}

	ImageBlitRange(const Offset3D& srcOffset0, const Offset3D& srcOffset1,
	               const Offset3D& dstOffset0, const Offset3D& dstOffset1,
	               const ImageSubResourceLayers& srcSubResource = ImageSubResourceLayers(),
	               const ImageSubResourceLayers& dstSubResource = ImageSubResourceLayers())
	{
		srcOffset[0] = srcOffset0, srcOffset[1] = srcOffset1;
		dstOffset[0] = dstOffset0, dstOffset[1] = dstOffset1;
		this->srcSubResource = srcSubResource;
		this->dstSubResource = dstSubResource;
	}

	ImageBlitRange(const Offset3D srcOffsets[2], const Offset3D dstOffsets[2],
	               const ImageSubResourceLayers& srcSubResource = ImageSubResourceLayers(),
	               const ImageSubResourceLayers& dstSubResource = ImageSubResourceLayers())
	{
		memcpy(srcOffset, srcOffsets, sizeof(Offset3D) * 2);
		memcpy(dstOffset, dstOffsets, sizeof(Offset3D) * 2);
		this->srcSubResource = srcSubResource;
		this->dstSubResource = dstSubResource;
	}
};

/// <summary>Contains the data for copying between a buffer and an image</summary>
struct BufferImageCopy
{
	uint32  bufferOffset;//!< Offset in bytes from the begining of the buffer object where the image data is copied from or to.

	/*! \brief control the addressing calculations of data in buffer memory. If either of bufferRowLength or bufferImageHeight is zero,
	that aspect of the buffer memory is considered to be tightly packed according to the imageExtent.*/
	uint32  bufferRowLength;
	/*! \brief control the addressing calculations of data in buffer memory. If either of bufferRowLength or bufferImageHeight is zero,
	that aspect of the buffer memory is considered to be tightly packed according to the imageExtent.*/
	uint32  bufferImageHeight;

	/*! \brief x, y, z offsets in texels of the sub-region of the source or destination image data.
	Images will only use corresponding coordinates (1D:x, 2D:x,y  3D: x,y,z)*/
	glm::uvec3  imageOffset;

	/*! \brief Size in texels of the image to copy in width, height and depth.
	Images will only use corresponding coordinates (1D:width, 2D:width,height  3D:width,height,depth)*/
	glm::uvec3  imageExtent;
	ImageSubResourceLayers  imageSubResource;

	/// <summary>Constructor. Default initialization for all values.</summary>
	BufferImageCopy() {}

	/// <summary>Constructor</summary>
	/// <param name="bufferOffset">The offset in the buffer from/to which to start the copy.</param>
	/// <param name="bufferRowLength">The number of bytes in the buffer that represent a row of the image</param>
	/// <param name="bufferImageHeight">The number of rows in the buffer that represent the height of the image
	/// </param>
	/// <param name="imageOffset">The offset in the image, in texels</param>
	/// <param name="imageExtent">The range in the image, in texels</param>
	/// <param name="imageSubResource">The (mip/array) level of the image to participate in the copy</param>
	BufferImageCopy(
	  uint32 bufferOffset, uint32 bufferRowLength, uint32 bufferImageHeight,
	  const glm::uvec3& imageOffset, const glm::uvec3& imageExtent,
	  const ImageSubResourceLayers& imageSubResource = ImageSubResourceLayers())
		: bufferOffset(bufferOffset), bufferRowLength(bufferRowLength), bufferImageHeight(bufferImageHeight),
		  imageOffset(imageOffset), imageExtent(imageExtent), imageSubResource(imageSubResource)
	{}
};


/// <summary>Object describing the state of a sampler. Also used to construct a full-blown Sampler object.</summary>
struct SamplerCreateParam
{
	/// <summary>Default constructor.</summary>
	/// <summary>Creates a Sampler with following configs. Can be edited after construction magnification filter: nearest,
	/// minifictation filter: nearest, mipmap filter: linear,wrap UVW: Repeat no comparison, no lod-bias, no
	/// anisotropy.</summary>
	SamplerCreateParam() :
		magnificationFilter(types::SamplerFilter::Linear), minificationFilter(types::SamplerFilter::Nearest),
		mipMappingFilter(types::SamplerFilter::Linear), wrapModeU(types::SamplerWrap::Repeat),
		wrapModeV(types::SamplerWrap::Repeat), wrapModeW(types::SamplerWrap::Repeat),
		compareMode(types::ComparisonMode::None), anisotropyMaximum(0.0f), lodBias(0.0f), lodMinimum(0.0f),
		lodMaximum(100.0f), unnormalizedCoordinates(false),
		borderColor(types::BorderColor::TransparentBlack)
	{}

	/// <summary>Constructor that sets the filters. Set the filtering mode explicitly. Wrap. Creates a Sampler with
	/// wrap:Repeat, no comparison, no lod bias, no anisotropy. Can be edited after construction.</summary>
	/// <param name="miniFilter">Minification filter: Nearest or Linear</param>
	/// <param name="magniFilter">Magnification filter: Nearest or Linear</param>
	/// <param name="mipMapFilter">Mipmap interpolation filter: Nearest or Linear</param>
	/// <param name="wrapModeU">The wrapping mode for the U coordinate</param>
	/// <param name="wrapModeV">The wrapping mode for the V coordinate</param>
	/// <param name="wrapModeW">The wrapping mode for the W coordinate</param>
	SamplerCreateParam(
	  types::SamplerFilter magniFilter, types::SamplerFilter miniFilter,
	  types::SamplerFilter mipMapFilter = types::SamplerFilter::Linear,
	  types::SamplerWrap wrapModeU = types::SamplerWrap::Repeat,
	  types::SamplerWrap wrapModeV = types::SamplerWrap::Repeat,
	  types::SamplerWrap wrapModeW = types::SamplerWrap::Repeat) :
		magnificationFilter(magniFilter), minificationFilter(miniFilter),
		mipMappingFilter(mipMapFilter), wrapModeU(wrapModeU), wrapModeV(wrapModeV),
		wrapModeW(wrapModeW), compareMode(types::ComparisonMode::None), anisotropyMaximum(0.0f),
		lodBias(0.0f), lodMinimum(0.0f), lodMaximum(100.0f), unnormalizedCoordinates(false),
		borderColor(types::BorderColor::TransparentBlack)
	{}

	types::SamplerFilter magnificationFilter; //!< Texture Magnification interpolation filter. Nearest or Linear. Default Nearest.
	types::SamplerFilter minificationFilter; //!< Texture Minification interpolation filter. Nearest or Linear. Default Nearest.
	types::SamplerFilter mipMappingFilter; //!< Texture Mipmap interpolation filter. Nearest, Linear or None (for no mipmaps). Default Linear.
	types::SamplerWrap wrapModeU; //!< Texture Wrap mode, U (x) direction (Repeat, Mirror, MirrorRepeat, Border, Clamp). Default Repeat.
	types::SamplerWrap wrapModeV; //!< Texture Wrap mode, V (y) direction (Repeat, Mirror, MirrorRepeat, Border, Clamp). Default Repeat.
	types::SamplerWrap wrapModeW; //!< Texture Wrap mode, W (z) direction (Repeat, Mirror, MirrorRepeat, Border, Clamp). Default Repeat.
	types::ComparisonMode compareMode; //!< Comparison mode for comparison samplers. Default None.
	float32 anisotropyMaximum; //!< Texture anisotropic filtering. Default 0.
	float32 lodBias;           //!< Texture level-of-detail bias (bias of mipmap to select). Default 0.
	float32 lodMinimum;        //!< Texture minimum level-of-detail (mipmap). Default 0.
	float32 lodMaximum;        //!< Texture maximum level-of-detail (mipmap). Default 0.
	bool            unnormalizedCoordinates; //!< Texture Coordinates are Un-Normalized
	types::BorderColor  borderColor; //!< Texture border color. Only used with Border wrap mode.
};

/// <summary>This class contains all the information of a Vertex Attribute's layout inside a block of memory,
/// typically a Vertex Buffer Object. This informations is normally the DataType of the attribute, the Offset (from
/// the beginning of the array) and the width (how many values of type DataType form an attribute).</summary>
struct VertexAttributeLayout
{
	// Type of data stored, should this be an enum or should it be an int to allow for users to do their own data types
	types::DataType dataType;
	uint16 offset; // Should be 16 bit?
	uint8 width; // Number of values per vertex

	/// <summary>VertexAttributeLayout</summary>
	VertexAttributeLayout() {}

	/// <summary>VertexAttributeLayout</summary>
	/// <param name="dataType"></param>
	/// <param name="width"></param>
	/// <param name="offset"></param>
	VertexAttributeLayout(types::DataType dataType, uint8 width, uint16 offset) :
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
	types::BlendFactor  srcBlendColor;  //!< Source Blending color factor
	types::BlendFactor  destBlendColor; //!< Destination blending color factor
	types::BlendFactor  srcBlendAlpha;  //!< Source blending alpha factor
	types::BlendFactor  destBlendAlpha; //!< Destination blending alpha factor
	types::BlendOp    blendOpColor; //!< Blending operation color
	types::BlendOp    blendOpAlpha; //!< Blending operation alpha
	types::ColorChannel channelWriteMask;//!<Channel writing mask

	/// <summary>Create a blending state. Separate color/alpha factors.</summary>
	/// <param name="blendEnable">Enable blending (default false)</param>
	/// <param name="srcBlendColor">Source Blending color factor (default:Zero)</param>
	/// <param name="destBlendColor">Destination blending color factor (default:Zero)</param>
	/// <param name="srcBlendAlpha">Source blending alpha factor (default:Zero)</param>
	/// <param name="destBlendAlpha">Destination blending alpha factor (default:Zero)</param>
	/// <param name="blendOpColor">Blending operation color (default:Add)</param>
	/// <param name="blendOpAlpha">Blending operation alpha (default:Add)</param>
	/// <param name="channelWriteMask">Channel writing mask (default:All)</param>
	BlendingConfig(
	  bool blendEnable = false, types::BlendFactor srcBlendColor = types::BlendFactor::One,
	  types::BlendFactor destBlendColor = types::BlendFactor::Zero,
	  types::BlendFactor srcBlendAlpha = types::BlendFactor::One,
	  types::BlendFactor destBlendAlpha = types::BlendFactor::Zero,
	  types::BlendOp blendOpColor = types::BlendOp::Add,
	  types::BlendOp blendOpAlpha = types::BlendOp::Add,
	  types::ColorChannel channelWriteMask = types::ColorChannel::All) :
		blendEnable(blendEnable), srcBlendColor(srcBlendColor), destBlendColor(destBlendColor),
		srcBlendAlpha(srcBlendAlpha), destBlendAlpha(destBlendAlpha), blendOpColor(blendOpColor),
		blendOpAlpha(blendOpAlpha), channelWriteMask(channelWriteMask) {}

	/// <summary>Create a blending state. Common color and alpha factors.</summary>
	/// <param name="blendEnable">Enable blending (default false)</param>
	/// <param name="srcBlendFactor">Source Blending factor</param>
	/// <param name="dstBlendFactor">Destination Blending factor</param>
	/// <param name="blendOpColorAlpha">Blending operation color & alpha (default:Add)</param>
	/// <param name="channelWriteMask">Channel writing mask (default:All)</param>
	BlendingConfig(bool blendEnable, types::BlendFactor srcBlendFactor, types::BlendFactor dstBlendFactor,
	               types::BlendOp blendOpColorAlpha, types::ColorChannel channelWriteMask = types::ColorChannel::All) :
		blendEnable(blendEnable), srcBlendColor(srcBlendFactor), destBlendColor(dstBlendFactor),
		srcBlendAlpha(srcBlendFactor), destBlendAlpha(dstBlendFactor),
		blendOpColor(blendOpColorAlpha), blendOpAlpha(blendOpColorAlpha), channelWriteMask(channelWriteMask) {}
};

/// <summary>Pipeline Stencil state</summary>
struct StencilState
{
	types::StencilOp opDepthPass;//!< Action performed on samples that pass both the depth and stencil tests.
	types::StencilOp opDepthFail;//!< Action performed on samples that pass the stencil test and fail the depth test.
	types::StencilOp opStencilFail;//!< Action performed on samples that fail the stencil test.
	uint32 compareMask;//!< Selects the bits of the unsigned integer stencil values during in the stencil test.
	uint32 writeMask;//!<  Selects the bits of the unsigned integer stencil values updated by the stencil test in the stencil framebuffer attachment.
	uint32 reference;//!< Integer reference value that is used in the unsigned stencil comparison.
	types::ComparisonMode compareOp;//!<  Comparison operator used in the stencil test.

	/// <summary>ctor</summary>
	/// <param name="depthPass">Action performed on samples that pass both the depth and stencil tests.</param>
	/// <param name="depthFail">Action performed on samples that pass the stencil test and fail the depth test.</param>
	/// <param name="stencilFail">Action performed on samples that fail the stencil test.</param>
	/// <param name="compareOp">Comparison operator used in the stencil test.</param>
	/// <param name="compareMask">Selects the bits of the unsigned integer stencil values during in the stencil test.
	/// </param>
	/// <param name="writeMask">Selects the bits of the unsigned integer stencil values updated by the stencil test in the
	/// stencil framebuffer attachment</param>
	/// <param name="reference">Integer reference value that is used in the unsigned stencil comparison.</param>
	StencilState(types::StencilOp depthPass = types::StencilOp::Keep,
	             types::StencilOp depthFail = types::StencilOp::Keep,
	             types::StencilOp stencilFail = types::StencilOp::Keep,
	             types::ComparisonMode compareOp = types::ComparisonMode::DefaultStencilFunc,
	             uint32 compareMask = 0xff, uint32 writeMask = 0xff, uint32 reference = 0)
		: opDepthPass(depthPass), opDepthFail(depthFail),
		  opStencilFail(stencilFail), compareMask(compareMask),
		  writeMask(writeMask), reference(reference), compareOp(compareOp) {}
};


/// <summary>The DrawIndirectCmd struct. The structure contains the data for drawIndirect command buffer
/// </summary>
struct CmdDrawIndirect
{
	uint32  vertexCount;  //!< Number of vertex to draw
	uint32  instanceCount;  //!< Number of instance to draw
	uint32  firstVertex;  //!< First vertex in the buffer to begin
	uint32  firstInstance;  //!< First instance to begin
};

/// <summary>The DrawIndexedIndirectCmd struct. The structure contains the data for drawIndexedInsdirect command
/// buffer</summary>
struct CmdDrawIndexedIndirect
{
	uint32 indexCount;//!<  The number of vertices to draw.
	uint32 instanceCount; //!< The number of instances to draw
	uint32 firstIndex;//!< The base index within the index buffer.
	uint32 vertexOffset;//!< The value added to the vertex index before indexing into the vertex buffer
	uint32 firstInstance;//!< The instance ID of the first instance to draw.
};

/// <summary>Base class for a descriptor type binding</summary>
struct DescriptorTypeBinding
{
	// the binding id for the descriptor type
	pvr::uint16 _bindingId;
	// the type of the descriptor
	types::DescriptorType _descType;

	/// <summary>ctor</summary>
	/// <param name="bindingId">The binding id.</param>
	/// <param name="descType">The type of the descriptor.</param>
	DescriptorTypeBinding(pvr::uint16 bindingId, types::DescriptorType descType) :
		_bindingId(bindingId), _descType(descType) {}

	/// <summary>ctor</summary>
	DescriptorTypeBinding() : _bindingId(static_cast<pvr::uint16>(pvr::types::DescriptorBindingDefaults::BindingId)),
		_descType(pvr::types::DescriptorBindingDefaults::Type) {}

	DescriptorTypeBinding operator=(const DescriptorTypeBinding& other)
	{
		// check for self-assignment
		if (&other == this)
		{
			return *this;
		}
		_bindingId = other._bindingId;
		_descType = other._descType;
		return *this;
	}

	/// <summary>Returns whether the descriptor type binding is valid</summary>
	/// <returns>True for valid, false for invalid</returns>
	bool isValid() const
	{
		return (int16)_bindingId != (int16)pvr::types::DescriptorBindingDefaults::BindingId && (int16)_descType != (int16)pvr::types::DescriptorBindingDefaults::Type;
	}
};

/// <summary>A binding layout for a descriptor</summary>
struct DescriptorBindingLayout : public DescriptorTypeBinding
{
	// the shader stage(s) the descriptor type will be used in
	types::ShaderStageFlags _shaderStage;
	// the array size for the descriptor type
	uint16 _arraySize;

	/// <summary>ctor</summary>
	/// <param name="bindingId">The binding id.</param>
	/// <param name="arraySize">The array size for the descriptor type.</param>
	/// <param name="descType">The type of the descriptor.</param>
	/// <param name="shaderStage">The stages the descriptor will be used in.</param>
	DescriptorBindingLayout(pvr::uint16 bindingId, pvr::int16 arraySize, types::DescriptorType descType, types::ShaderStageFlags shaderStage) :
		DescriptorTypeBinding(bindingId, descType), _shaderStage(shaderStage), _arraySize(arraySize) {}

	/// <summary>ctor</summary>
	DescriptorBindingLayout() :
		DescriptorTypeBinding(), _shaderStage(types::DescriptorBindingDefaults::ShaderStage), _arraySize((uint16)types::DescriptorBindingDefaults::ArraySize) {}

	DescriptorBindingLayout operator=(const DescriptorBindingLayout& other)
	{
		// check for self-assignment
		if (&other == this)
		{
			return *this;
		}
		DescriptorTypeBinding::operator=(other);
		_shaderStage = other._shaderStage;
		_arraySize = other._arraySize;
		return *this;
	}

	/// <summary>Returns whether the descriptor type binding layout is valid</summary>
	/// <returns>True for valid, false for invalid</returns>
	bool isValid() const
	{
		return DescriptorTypeBinding::isValid() &&
		       _shaderStage != types::DescriptorBindingDefaults::ShaderStage && _arraySize != (uint16)types::DescriptorBindingDefaults::ArraySize;
	}
};

/// <summary>An item binding for a descriptor</summary>
template<class _Binding>
struct DescriptorItemBinding : public DescriptorTypeBinding
{
	_Binding _binding;
	pvr::int16 _arrayIndex;

	/// <summary>ctor</summary>
	/// <param name="bindingId">The binding id.</param>
	/// <param name="arrayIndex">Array index the the descriptor type will be bound to.</param>
	/// <param name="descType">The type of the descriptor.</param>
	/// <param name="item">The descriptor item.</param>
	DescriptorItemBinding(pvr::int16 bindingId, pvr::int16 arrayIndex, types::DescriptorType descType, const _Binding& item) :
		DescriptorTypeBinding(bindingId, descType), _binding(item), _arrayIndex(arrayIndex) {}

	/// <summary>ctor</summary>
	DescriptorItemBinding() :
		DescriptorTypeBinding(), _arrayIndex(pvr::types::DescriptorBindingDefaults::ArraySize) {}

	DescriptorItemBinding<_Binding> operator=(const DescriptorItemBinding<_Binding>& other)
	{
		// check for self-assignment
		if (&other == this)
		{
			return *this;
		}
		DescriptorTypeBinding::operator=(other);
		_binding = other._binding;
		_arrayIndex = other._arrayIndex;
		return *this;
	}

	/// <summary>Returns whether the descriptor type binding is valid</summary>
	/// <returns>True for valid, false for invalid</returns>
	bool isValid() const
	{
		return DescriptorTypeBinding::isValid() && _arrayIndex != types::DescriptorBindingDefaults::ArraySize;
	}
};

enum { BindingOverflow = 16 };

/// <summary>A store for a set of descriptor item bindings</summary>
template<class DescriptorTypeBinding>
class DescriptorBindingStore
{
public:

	/// <summary>ctor</summary>
	DescriptorBindingStore() : _numItems(0) {}

	/// <summary>Clears the binding store</summary>
	void clear()
	{
		_numItems = 0;
		_overflowStore.clear();

		for (pvr::uint16 i = 0; i < BindingOverflow; i++)
		{
			_limitedStore[i] = DescriptorTypeBinding();
		}
	}

	/// <summary>Returns the number of items held in the store</summary>
	/// <returns>The item count</returns>
	pvr::uint16 itemCount() const
	{
		return _numItems;
	}

	/// <summary>Returns the list of descriptor bindings</summary>
	/// <returns>The list of descriptor type bindings</returns>
	const DescriptorTypeBinding* descriptorBindings() const
	{
		if (isOverflowed())
		{
			return _overflowStore.data();
		}
		else
		{
			return _limitedStore;
		}
	}

protected:
	// the limited capacity store
	DescriptorTypeBinding _limitedStore[BindingOverflow];
	// the overflow store
	std::vector<DescriptorTypeBinding> _overflowStore;
	// the number of items held in the store
	uint16 _numItems;

	/// <summary>Adds a particular item to the store at the index specified</summary>
	/// <param name="index">The index in the store to use.</param>
	/// <param name="item">The descriptor item to add into the store.</param>
	void addToIndex(uint16 index, const DescriptorTypeBinding& item)
	{
		// if already using the overflow store then add into the overflow store
		if (isOverflowed())
		{
			// add the the end of the store
			_overflowStore.push_back(item);
			sortLastItem(_overflowStore.data(), _numItems + 1, index);
		}
		else
		{
			// add the the end of the store
			_limitedStore[_numItems] = item;
			sortLastItem(_limitedStore, _numItems + 1, index);
		}
		_numItems++;

		// if the limited capacity store is full then move into the overflow store
		if (mustOverflow()) { moveToOverflow(); }
	}

	/// <summary>Retrieves a particular descriptor using the store index</summary>
	/// <param name="index">The index in the store to use.</param>
	/// <returns>The descriptor type bindings</returns>
	const DescriptorTypeBinding& retrieveDescriptorByIndex(uint16 index) const
	{
		if (isOverflowed()) { return _overflowStore[index]; }
		else { return _limitedStore[index]; }
	}

	/// <summary>(non const) retrieves a particular descriptor using the store index</summary>
	/// <param name="index">The index in the store to use.</param>
	/// <returns>The descriptor type bindings</returns>
	DescriptorTypeBinding& retrieveDescriptorByIndex(uint16 index)
	{
		if (isOverflowed()) { return _overflowStore[index]; }
		else { return _limitedStore[index]; }
	}

	/// <summary>Sorts the store</summary>
	/// <param name="items">The items to sort.</param>
	/// <param name="_numItems">The number of items being sorted.</param>
	/// <param name="newposition">The newly added item.</param>
	void sortLastItem(DescriptorTypeBinding* items, uint16 _numItems, uint16 newposition)
	{
		if (newposition != _numItems - 1)
		{
			auto newItem = items[_numItems - 1];
			for (pvr::uint32 i = _numItems - 1; i > newposition; i--)
			{
				items[i] = items[i - 1];
			}
			items[newposition] = newItem;
		}
	}

	/// <summary>Determines whether the store has been moved to the overflow store</summary>
	/// <returns>True if the store has been moved to the overflow store</returns>
	bool isOverflowed() const { return _numItems >= BindingOverflow; }

private:
	bool mustOverflow()
	{
		return (_numItems == BindingOverflow);
	}

	/// <summary>Copies the contents of the limited store to the overflow store</summary>
	void moveToOverflow()
	{
		_overflowStore.reserve(2 * BindingOverflow);
		for (pvr::uint32 i = 0; i < _numItems; i++)
		{
			_overflowStore.push_back(_limitedStore[i]);
		}
	}
};

/// <summary>A store for a set of descriptor layout bindings</summary>
class DescriptorLayoutBindingStore : public DescriptorBindingStore<DescriptorBindingLayout>
{
public:
	/// <summary>ctor</summary>
	DescriptorLayoutBindingStore() : DescriptorBindingStore() {}

	/// <summary>adds a new descriptor binding layout</summary>
	/// <param name="item">The item to add to the store.</param>
	void add(const DescriptorBindingLayout& item)
	{
		// only add valid items
		assertion(item.isValid(), "Descriptor binding layout must be valid.");

		// retrieve the next index to use
		uint16 index = retrieveDescriptorIndex(item._bindingId);
		bool addItem = true;

		// if the binding layout at the same binding id already exists in the store then replace it with the new layout
		if (index != itemCount())
		{
			auto& desc = this->retrieveDescriptorByIndex(index);
			if (desc._bindingId == item._bindingId)
			{
				desc = item;
				addItem = false;
			}
		}

		// if we're adding a new layout at a new binding id then add it to the store
		if (addItem)
		{
			this->addToIndex(index, item);
		}
	}

	/// <summary>Determines whether the store has already got an item with the binding id specified</summary>
	/// <param name="bindingId">The binding id to find.</param>
	/// <returns>True if the store has already includes an item with the same binding id</returns>
	bool hasBinding(uint16 bindingId)
	{
		return this->retrieveDescriptorByIndex(retrieveDescriptorIndex(bindingId))._bindingId == bindingId;
	}

	/// <summary>Determines whether the store has already got an item with the binding id specified</summary>
	/// <param name="bindingId">The binding id to find.</param>
	/// <returns>True if the store has already includes an item with the same binding id</returns>
	const bool hasBinding(uint16 bindingId) const
	{
		return this->retrieveDescriptorByIndex(retrieveDescriptorIndex(bindingId))._bindingId == bindingId;
	}

	/// <summary>Retrieves a descriptor using the binding id specified</summary>
	/// <param name="bindingId">The binding id to find.</param>
	/// <returns>The item with the binding id specified</returns>
	const DescriptorTypeBinding& retrieveDescriptor(uint16 bindingId) const
	{
		return this->retrieveDescriptorByIndex(retrieveDescriptorIndex(bindingId));
	}

	/// <summary>Retrieves a descriptor using the binding id specified</summary>
	/// <param name="bindingId">The binding id to find.</param>
	/// <returns>The item with the binding id specified</returns>
	DescriptorTypeBinding& retrieveDescriptor(uint16 bindingId)
	{
		return this->retrieveDescriptorByIndex(retrieveDescriptorIndex(bindingId));
	}

	/// <summary>equality operator</summary>
	/// <param name="rhs">The item to compare to.</param>
	/// <returns>True if this and rhs are equal</returns>
	bool operator==(const DescriptorBindingStore<DescriptorBindingLayout>& rhs) const
	{
		if (itemCount() != rhs.itemCount()) { return false; }
		auto str = descriptorBindings();
		auto rhsstr = rhs.descriptorBindings();

		for (unsigned int i = 0; i < _numItems; i++)
		{
			if (str[i]._arraySize != rhsstr[i]._arraySize ||
			    str[i]._descType != rhsstr[i]._descType ||
			    str[i]._shaderStage != rhsstr[i]._shaderStage)
			{
				return false;
			}
		}
		return true;
	}
protected:
	/// <summary>retrieves the index of descriptor binding layout with the specified binding id or returns the
	/// number of items if one cannot be found with the binding id </summary >
	/// <param name="bindingId">The binding id to use for retrieving the descriptor.</param>
	/// <returns>The next index to use</returns>
	uint16 retrieveDescriptorIndex(uint16 bindingId) const
	{
		struct tmpcmp
		{
			bool operator()(const DescriptorTypeBinding& lhs, uint16 rhs)
			{
				return lhs._bindingId < rhs;
			}
		};
		// returns the index of the first instance of binding id or the number of items in the list
		if (isOverflowed())
		{
			return uint16(std::lower_bound(_overflowStore.begin(), _overflowStore.end(), bindingId, tmpcmp()) - _overflowStore.begin());
		}
		else
		{
			return uint16(std::lower_bound(_limitedStore, _limitedStore + _numItems, bindingId, tmpcmp()) - _limitedStore);
		}
	}
};

/// <summary>A store for a set of descriptor item bindings</summary>
template<class _BindingType>
class DescriptorUpdateBindingStore : public DescriptorBindingStore<DescriptorItemBinding<_BindingType>>
{
public:
	/// <summary>ctor</summary>
	DescriptorUpdateBindingStore() : DescriptorBindingStore<DescriptorItemBinding<_BindingType>>() {}

	/// <summary>Adds a descriptor item binding to the store</summary >
	/// <param name="item">The descriptor binding to add to the store.</param>
	void add(const DescriptorItemBinding<_BindingType>& item)
	{
		// only add valid items
		assertion(item.isValid(), "Descriptor binding update must be valid.");

		// retrieve the next index
		uint16 index = retrieveDescriptorIndex(item._bindingId, item._arrayIndex);
		bool addItem = true;

		// if the binding already exists in the store
		if (index != this->itemCount())
		{
			auto& desc = this->retrieveDescriptorByIndex(index);
			if (desc._bindingId == item._bindingId && desc._arrayIndex == item._arrayIndex)
			{
				desc = item;
				addItem = false;
			}
		}

		if (addItem)
		{
			this->addToIndex(index, item);
		}
	}

	/// <summary>Retrieves a descriptor using the binding id specified</summary>
	/// <param name="bindingId">The binding id to find.</param>
	/// <returns>The item with the binding id specified</returns>
	const DescriptorItemBinding<_BindingType>& retrieveDescriptor(uint16 bindingId, uint16 arayIndex) const
	{
		return this->retrieveDescriptorByIndex(retrieveDescriptorIndex(bindingId, arayIndex));
	}

	/// <summary>Retrieves a descriptor using the binding id specified</summary>
	/// <param name="bindingId">The binding id to find.</param>
	/// <returns>The item with the binding id specified</returns>
	DescriptorItemBinding<_BindingType>& retrieveDescriptor(uint16 bindingId, uint16 arayIndex)
	{
		return this->retrieveDescriptorByIndex(retrieveDescriptorIndex(bindingId, arayIndex));
	}

	/// <summary>equality operator</summary>
	/// <param name="rhs">The item to compare to.</param>
	/// <returns>True if this and rhs are equal</returns>
	bool operator==(const DescriptorBindingStore<DescriptorItemBinding<_BindingType>>& rhs) const
	{
		if (this->itemCount() != rhs.itemCount()) { return false; }
		auto str = this->descriptorBindings();
		auto rhsstr = rhs.descriptorBindings();

		for (unsigned int i = 0; i < this->itemCount(); i++)
		{
			if (str[i]._arraySize != rhsstr[i]._arraySize ||
			    str[i]._descType != rhsstr[i]._descType ||
			    str[i]._shaderStage != rhsstr[i]._shaderStage)
			{
				return false;
			}
		}
		return true;
	}

protected:
	/// <summary>retrieves the index of descriptor binding with the specified binding id or returns the
	/// number of items if one cannot be found with the binding id </summary >
	/// <param name="bindingId">The binding id to use for retrieving the descriptor.</param>
	/// <param name="arayIndex">The array id to retireve the descriptor index of.</param>
	/// <returns>The next index to use</returns>
	uint16 retrieveDescriptorIndex(uint16 bindingId, uint16 arayIndex) const
	{
		struct DescriptorIndexBinding
		{
			uint16 bindingId;
			uint16 arayIndex;
		};

		struct tmpcmp
		{
			bool operator()(const DescriptorItemBinding<_BindingType>& lhs, DescriptorIndexBinding rhs)
			{
				if (lhs._bindingId < rhs.bindingId)
				{
					return true;
				}

				if (lhs._bindingId == rhs.bindingId)
				{
					if (lhs._arrayIndex < rhs.arayIndex)
					{
						return true;
					}
				}

				return false;
			}
		};

		DescriptorIndexBinding indexBinding{ bindingId, arayIndex };

		// returns the index of the first instance of binding id or the number of items in the list
		if (this->isOverflowed())
		{
			return uint16(std::lower_bound(this->_overflowStore.begin(), this->_overflowStore.end(), indexBinding, tmpcmp()) - this->_overflowStore.begin());
		}
		else
		{
			return uint16(std::lower_bound(this->_limitedStore, this->_limitedStore + this->itemCount(), indexBinding, tmpcmp()) - this->_limitedStore);
		}
	}
};

struct PushConstantRange
{
	types::ShaderStageFlags stage;
	uint32 offset;
	uint32 size;

	PushConstantRange() : offset(0), size(0) {}
	PushConstantRange(types::ShaderStageFlags stage, uint32 offset, uint32 size) :
		stage(stage), offset(offset), size(size) {}

};

}//namespace types

/// <summary>Class wrapping an arithmetic type and providing bitwise operation for its bits</summary>
template<typename Storage_>
class Bitfield
{
public:
	/// <summary>Return true if a bit is set</summary>
	/// <param name="store">Bit storage</param>
	/// <param name="bit">Bit to check</param>
	inline static bool isSet(Storage_ store, int8 bit)
	{
		return (store & (1 << bit)) != 0;
	}

	/// <summary>Set a bit in the storage</summary>
	/// <param name="store">Storage</param>
	/// <param name="bit">Bit to set</param>
	inline static void set(Storage_ store, int8 bit)
	{
		store |= (1 << bit);
	}

	/// <summary>Clear a bit fro the storage</summary>
	/// <param name="store">Storage</param>
	/// <param name="bit">Bit to clear</param>
	inline static void clear(Storage_ store, int8 bit)
	{
		store &= (!(1 << bit));
	}

};
}
