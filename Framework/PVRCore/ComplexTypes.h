#pragma once
#include "PVRCore/Types.h"
#include "../External/glm/glm.hpp"
namespace pvr {
namespace types {

/*!*********************************************************************************************************************
\brief Enumeration of texture Swizzle mask channels.
***********************************************************************************************************************/
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

/*!
   \brief The SwizzleChannels struct
 */
struct SwizzleChannels
{
	Swizzle r;//!< Swizzle R channel
	Swizzle g;//!< Swizzle G channel
	Swizzle b;//!< Swizzle B channel
	Swizzle a;//!< Swizzle A channel

	/*!
	   \brief SwizzleChannels. Default: All channels are set to identity
	 */
	SwizzleChannels() : r(Swizzle::Identity), g(Swizzle::Identity), b(Swizzle::Identity), a(Swizzle::Identity) { }

	/*!
	   \brief SwizzleChannels
	   \param r Swizzle R channel
	   \param g Swizzle G channel
	   \param b Swizzle B channel
	   \param a Swizzle A channel
	 */
	SwizzleChannels(Swizzle r, Swizzle g, Swizzle b, Swizzle a) : r(r), g(g), b(b), a(a) { }
};

/*!
   \brief The ImageLayersSize struct. Describes the number of array levels and mip levels an image contains
 */
struct ImageLayersSize
{
	uint16 numArrayLevels;  //!< The number of array slices of the range
	uint16 numMipLevels;   //!< The number of mipmap levels of the range
	/*!
	   \brief ImageLayersSize
	   \param numArrayLevels
	   \param numMipLevels
	 */
	ImageLayersSize(uint16 numArrayLevels = 1, uint8 numMipLevels = 1) :
		numArrayLevels(numArrayLevels), numMipLevels(numMipLevels) {}
};

/*!*******************************************************************************************
\brief Describes an area of an image, out of which a Subresource can be defined. This area is
a subarea of the image, containing a part of the image array slices, mip levels, or aspects.
**********************************************************************************************/
struct ImageSubresource
{
	uint16 arrayLayerOffset; //!< The index of the first array slice of the range
	uint16 mipLevelOffset;   //!< The index of the first mipmap levels of the range
	/*!
	   \brief ImageSubresource
	   \param mipLevelOffset
	   \param arrayLayerOffset
	 */
	ImageSubresource(uint16 mipLevelOffset = 0, uint16 arrayLayerOffset = 0) :
		arrayLayerOffset(arrayLayerOffset), mipLevelOffset(mipLevelOffset) {}
};

/*!
   \brief The Extent2D contains the extent in 2 dimension (width, height)
 */
struct Extent2D
{
	uint32 width;
	uint32 height;
	/*!
	   \brief Extent2D
	   \param width
	   \param height
	 */
	Extent2D(uint32 width, uint32 height) : width(width), height(height)
	{
		PVR_ASSERTION(width < 65536 && height < 65536 && "Error - Max supported image extent must fit into a 16-bit unsigned integer");
	}
};

/*!
   \brief The Extent3D contains the extent in 3 dimension (width, height, depth)
 */
struct Extent3D : public Extent2D
{
	uint16 depth;
	/*!
	   \brief Extent3D
	   \param width
	   \param height
	   \param depth
	 */
	Extent3D(uint32 width = 1, uint32 height = 1, uint32 depth = 1) :
		Extent2D(width, height), depth((uint16)depth)
	{
		PVR_ASSERTION(depth < 65536
		              && "Error - Max supported image extent must fit into a 16-bit unsigned integer");
	}
};

/*!
   \brief The Offset2D contains the offsets in 2 dimension (offsetX, offsetY)
 */
struct Offset2D
{
	uint16 offsetX;//!< offset in x axis
	uint16 offsetY;//!< offset in y axis
	/*!
	   \brief Offset2D
	   \param offsetX
	   \param offsetY
	 */
	Offset2D(uint16 offsetX = 0, uint16 offsetY = 0) : offsetX(offsetX), offsetY(offsetY)
	{ }
};

/*!
   \brief The Offset3D contains the offsets in 3 dimension (offsetX, offsetY, offsetZ)
 */
struct Offset3D : public Offset2D
{
	uint16 offsetZ;//!< offset in z axis
	/*!
	   \brief Offset3D
	   \param offsetX
	   \param offsetY
	   \param offsetZ
	 */
	Offset3D(uint16 offsetX = 0, uint16 offsetY = 0, uint16 offsetZ = 0) :
		Offset2D((uint16)offsetX, (uint16)offsetY), offsetZ((uint16)offsetZ)
	{ }

	/*!
	   \brief Offset3D
	   \param offsetXY
	   \param offsetZ
	 */
	Offset3D(const Offset2D& offsetXY, uint32 offsetZ = 0) :
		Offset2D(offsetXY), offsetZ((uint16)offsetZ)
	{
		PVR_ASSERTION(offsetZ < std::numeric_limits<uint16>::max() && "Error - Max supported image offset must fit into a 16-bit unsigned integer");
	}
};

/*!
   \brief The ImageAspectRange struct
 */
struct ImageAspectRange
{
	types::ImageAspect aspect;

	/*!
	   \brief ImageAspectRange
	   \param aspect ImageAspectm, Default: Color
	 */
	ImageAspectRange(ImageAspect aspect = ImageAspect::Color) : aspect(aspect) {}
};

struct ImageSubresourceRange : public ImageLayersSize, public ImageSubresource, ImageAspectRange
{
	ImageSubresourceRange() {}
	ImageSubresourceRange(ImageLayersSize layersSize, ImageSubresource baseLayers):
		ImageLayersSize(layersSize), ImageSubresource(baseLayers) {}
};

struct ImageSubResourceLayers : public ImageSubresource, ImageAspectRange
{
	uint16 numArrayLayers;
	ImageSubResourceLayers() : numArrayLayers(1) {}
	ImageSubResourceLayers(ImageSubresource baseLayers, ImageAspectRange aspectRange, uint16 numArrayLayers) :
		ImageSubresource(baseLayers), ImageAspectRange(aspectRange), numArrayLayers(numArrayLayers) {}
};

/*!
   \brief The ImageRange struct. Contains the offset and extent in 3d
 */
struct ImageRange : public Extent3D, public Offset3D
{
	/*!
	   \brief ImageRange
	 */
	ImageRange() {}

	/*!
	   \brief ImageRange
	   \param extents
	   \param offset
	 */
	ImageRange(const Extent3D& extents, const Offset3D& offset) :
		Extent3D(extents), Offset3D(offset) {}
};

struct ImageAreaSize : public Extent3D, public ImageLayersSize
{
	ImageAreaSize() {}
	ImageAreaSize(ImageLayersSize layersSize, Extent3D extents) :
		Extent3D(extents), ImageLayersSize(layersSize) {}
};
struct ImageAreaOffset: public ImageSubresource, public Offset3D
{
	ImageAreaOffset() {}
	ImageAreaOffset(ImageSubresource baseLayers, Offset3D offset) :
		ImageSubresource(baseLayers), Offset3D(offset) {}
};
struct ImageArea : public ImageAreaSize, ImageAreaOffset
{
	ImageArea() {}
	ImageArea(ImageLayersSize layerSize, Extent3D extents, ImageSubresource baseLayers, Offset3D offset):
		ImageAreaSize(layerSize, extents), ImageAreaOffset(baseLayers, offset) {}
	operator ImageSubresourceRange() const
	{
		return ImageSubresourceRange(*this, *this);
	}

};

struct ImageBlitRange
{
	Offset3D             srcOffset[2];//!< Src Region end points (min, max)
	Offset3D             dstOffset[2];//!< Dst Region end points (min, max)
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

	ImageBlitRange(const Offset3D srcOffsets[2],  const Offset3D dstOffsets[2],
	               const ImageSubResourceLayers& srcSubResource = ImageSubResourceLayers(),
	               const ImageSubResourceLayers& dstSubResource = ImageSubResourceLayers())
	{
		memcpy(srcOffset, srcOffsets, sizeof(Offset3D) * 2);
		memcpy(dstOffset, dstOffsets, sizeof(Offset3D) * 2);
		this->srcSubResource = srcSubResource;
		this->dstSubResource = dstSubResource;
	}
};

/*!
   \brief The BufferImageCopy struct. Contains the data for copying between buffer and the image
 */
struct BufferImageCopy
{
	uint32  bufferOffset;//!< Offset in bytes from the begining of the buffer object where the image data is copied from or to.

	//!< control the addressing calculations of data in buffer memory. If either of bufferRowLength or bufferImageHeight is zero,
	//! that aspect of the buffer memory is considered to be tightly packed according to the imageExtent.
	uint32  bufferRowLength;
	uint32  bufferImageHeight;

	glm::uvec3  imageOffset; //!< x, y, z offsets in texels of the sub-region of the source or destination image data.
	glm::uvec3  imageExtent; /*! Size in texels of the image to copy in width, height and depth.
  1D images use only x and width. 2D images use x, y, width and height.
  3D images use x, y, z, width, height and depth.*/
	ImageSubResourceLayers  imageSubResource;

	/*!
	   \brief BufferImageCopy
	 */
	BufferImageCopy() {}

	/*!
	   \brief BufferImageCopy
	   \param bufferOffset
	   \param bufferRowLength
	   \param bufferImageHeight
	   \param imageOffset
	   \param imageExtent
	   \param imageSubResource
	 */
	BufferImageCopy(
	  uint32 bufferOffset, uint32 bufferRowLength, uint32 bufferImageHeight,
	  const glm::uvec3& imageOffset, const glm::uvec3& imageExtent,
	  const ImageSubResourceLayers& imageSubResource = ImageSubResourceLayers())
		: bufferOffset(bufferOffset), bufferRowLength(bufferRowLength), bufferImageHeight(bufferImageHeight),
		  imageOffset(imageOffset), imageExtent(imageExtent), imageSubResource(imageSubResource)
	{}
};


/*!*********************************************************************************************************************
\brief Object describing the state of a sampler. Also used to construct a full-blown Sampler object.
***********************************************************************************************************************/
struct SamplerCreateParam
{
	/*!*********************************************************************************************************************
	\brief Default constructor.
	\brief Creates a Sampler with following configs. Can be edited after construction
	magnification filter: nearest, minifictation filter: nearest, mipmap filter: linear,wrap UVW: Repeat
	no comparison, no lod-bias, no anisotropy.
	***********************************************************************************************************************/
	SamplerCreateParam() :
		magnificationFilter(types::SamplerFilter::Linear), minificationFilter(types::SamplerFilter::Nearest),
		mipMappingFilter(types::SamplerFilter::Linear), wrapModeU(types::SamplerWrap::Repeat),
		wrapModeV(types::SamplerWrap::Repeat), wrapModeW(types::SamplerWrap::Repeat),
		compareMode(types::ComparisonMode::None), anisotropyMaximum(0.0f), lodBias(0.0f), lodMinimum(0.0f),
		lodMaximum(100.0f), unnormalizedCoordinates(false),
		borderColor(types::BorderColor::TransparentBlack)
	{}

	/*!*********************************************************************************************************************
	\brief Constructor that sets the filters. Set the filtering mode explicitly. Wrap.
	Creates a Sampler with wrap:Repeat, no comparison, no lod bias, no anisotropy. Can be edited after construction.
	\param[in] miniFilter  Minification filter: Nearest or Linear
	\param[in] magniFilter  Magnification filter: Nearest or Linear
	\param[in] mipMapFilter Mipmap interpolation filter: Nearest or Linear
	***********************************************************************************************************************/
	SamplerCreateParam(
	  types::SamplerFilter magniFilter, types::SamplerFilter miniFilter,
	  types::SamplerFilter mipMapFilter = types::SamplerFilter::Linear) :
		magnificationFilter(magniFilter), minificationFilter(miniFilter),
		mipMappingFilter(mipMapFilter), wrapModeU(types::SamplerWrap::Repeat), wrapModeV(types::SamplerWrap::Repeat),
		wrapModeW(types::SamplerWrap::Repeat), compareMode(types::ComparisonMode::None), anisotropyMaximum(0.0f),
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

/*!*********************************************************************************************
\brief       This class contains all the information of a Vertex Attribute's layout inside a
block of memory, typically a Vertex Buffer Object. This informations is normally
the DataType of the attribute, the Offset (from the beginning of the array) and the
width (how many values of type DataType form an attribute).
***********************************************************************************************/
struct VertexAttributeLayout
{
	// Type of data stored, should this be an enum or should it be an int to allow for users to do their own data types
	types::DataType dataType;
	uint16 offset; // Should be 16 bit?
	uint8 width; // Number of values per vertex

	/*!
	   \brief VertexAttributeLayout
	 */
	VertexAttributeLayout() {}

	/*!
	   \brief VertexAttributeLayout
	   \param dataType
	   \param width
	   \param offset
	 */
	VertexAttributeLayout(types::DataType dataType, uint8 width, uint16 offset) :
		dataType(dataType), offset(offset), width(width) {}
};


/*!*********************************************************************************************************************
\brief Add blending configuration for a color attachment. Some API's only support one blending state for all attachments,
in which case the 1st such configuration will be used for all.
\description --- Defaults ---
Blend Enabled:false,    Source blend Color factor: false,    Destination blend Color factor: Zero,
Source blend Alpha factor: Zero,    Destination blending Alpha factor :Zero,
Blending operation color: Add,    Blending operation alpha: Add,     Channel writing mask: All
***********************************************************************************************************************/
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

	/*!*********************************************************************************************************************
	\brief Create a blending state. Separate color/alpha factors.
	\param blendEnable Enable blending (default false)
	\param srcBlendColor Source Blending color factor (default:Zero)
	\param destBlendColor Destination blending color factor (default:Zero)
	\param srcBlendAlpha Source blending alpha factor (default:Zero)
	\param destBlendAlpha Destination blending alpha factor (default:Zero)
	\param blendOpColor Blending operation color (default:Add)
	\param blendOpAlpha Blending operation alpha (default:Add)
	\param channelWriteMask  Channel writing mask (default:All)
	***********************************************************************************************************************/
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

	/*!*********************************************************************************************************************
	\brief Create a blending state. Common color and alpha factors.
	\param blendEnable Enable blending (default false)
	\param srcBlendFactor Source Blending factor
	\param dstBlendFactor Destination Blending factor
	\param blendOpColorAlpha Blending operation color & alpha (default:Add)
	\param channelWriteMask  Channel writing mask (default:All)
	***********************************************************************************************************************/
	BlendingConfig(bool blendEnable, types::BlendFactor srcBlendFactor, types::BlendFactor dstBlendFactor,
	               types::BlendOp blendOpColorAlpha, types::ColorChannel channelWriteMask = types::ColorChannel::All) :
		blendEnable(blendEnable), srcBlendColor(srcBlendFactor), destBlendColor(dstBlendFactor),
		srcBlendAlpha(srcBlendFactor), destBlendAlpha(dstBlendFactor),
		blendOpColor(blendOpColorAlpha), blendOpAlpha(blendOpColorAlpha), channelWriteMask(channelWriteMask) {}
};

/*!*********************************************************************************************************************
\brief Pipeline Stencil state
***********************************************************************************************************************/
struct StencilState
{
	types::StencilOp opDepthPass;//!< Action performed on samples that pass both the depth and stencil tests.
	types::StencilOp opDepthFail;//!< Action performed on samples that pass the stencil test and fail the depth test.
	types::StencilOp opStencilFail;//!< Action performed on samples that fail the stencil test.
	uint32 compareMask;//!< Selects the bits of the unsigned integer stencil values during in the stencil test.
	uint32 writeMask;//!<  Selects the bits of the unsigned integer stencil values updated by the stencil test in the stencil framebuffer attachment.
	uint32 reference;//!< Integer reference value that is used in the unsigned stencil comparison.
	types::ComparisonMode compareOp;//!<  Comparison operator used in the stencil test.

	/*!*********************************************************************************************************************
	\brief ctor
	\param depthPass Action performed on samples that pass both the depth and stencil tests.
	\param depthFail Action performed on samples that pass the stencil test and fail the depth test.
	\param stencilFail Action performed on samples that fail the stencil test.
	\param compareOp Comparison operator used in the stencil test.
	\param compareMask Selects the bits of the unsigned integer stencil values during in the stencil test.
	\param  writeMask Selects the bits of the unsigned integer stencil values updated by the stencil test in the stencil framebuffer attachment
	\param  reference Integer reference value that is used in the unsigned stencil comparison.
	***********************************************************************************************************************/
	StencilState(types::StencilOp depthPass = types::StencilOp::Keep,
	             types::StencilOp depthFail = types::StencilOp::Keep,
	             types::StencilOp stencilFail = types::StencilOp::Keep,
	             types::ComparisonMode compareOp = types::ComparisonMode::DefaultDepthFunc,
	             uint32 compareMask = 0xff, uint32 writeMask = 0xff, uint32 reference = 0)
		: opDepthPass(depthPass), opDepthFail(depthFail),
		  opStencilFail(stencilFail), compareMask(compareMask),
		  writeMask(writeMask), reference(reference), compareOp(compareOp) {}
};


/*!************************************************************************************************************************
\brief The DrawIndirectCmd struct. The structure contains the data for drawIndirect command buffer
***************************************************************************************************************************/
struct CmdDrawIndirect
{
	uint32  vertexCount;  //!< Number of vertex to draw
	uint32  instanceCount;  //!< Number of instance to draw
	uint32  firstVertex;  //!< First vertex in the buffer to begin
	uint32  firstInstance;  //!< First instance to begin
};

/*!************************************************************************************************************************
\brief The DrawIndexedIndirectCmd struct. The structure contains the data for drawIndexedInsdirect command buffer
***************************************************************************************************************************/
struct CmdDrawIndexedIndirect
{
	uint32 indexCount;//!<  The number of vertices to draw.
	uint32 instanceCount; //!< The number of instances to draw
	uint32 firstIndex;//!< The base index within the index buffer.
	uint32 vertexOffset;//!< The value added to the vertex index before indexing into the vertex buffer
	uint32 firstInstance;//!< The instance ID of the first instance to draw.
};


struct DescriptorBindingLayout
{
	types::ShaderStageFlags shaderStage; //!< shader stage
	pvr::int8 arraySize;//!< array size
	types::DescriptorType descType;//!< descriptor type

	DescriptorBindingLayout(pvr::int8 arraySize, types::DescriptorType descType, types::ShaderStageFlags shaderStage) :
		shaderStage(shaderStage), arraySize(arraySize), descType(descType) {}

	DescriptorBindingLayout() :
		shaderStage(pvr::types::DescriptorBindingDefaults::ShaderStage),
		arraySize(pvr::types::DescriptorBindingDefaults::ArraySize),
		descType(pvr::types::DescriptorBindingDefaults::Type) {}

	/*!
	   \brief Return true if this is a valid descriptor binding layout
	 */
	bool isValid() const
	{
		return !(descType == pvr::types::DescriptorBindingDefaults::Type ||
		         shaderStage == pvr::types::DescriptorBindingDefaults::ShaderStage ||
		         arraySize == pvr::types::DescriptorBindingDefaults::ArraySize);
	}
};

template<class _Binding>
struct DescriptorBinding
{
	_Binding binding;
	pvr::int8 bindingId;
	pvr::int8 arrayIndex;
	types::DescriptorType descType;

	DescriptorBinding(pvr::int8 bindingId, pvr::int8 index, types::DescriptorType descType, const _Binding& obj) :
		binding(obj), bindingId(bindingId), arrayIndex(index), descType(descType) {}

	DescriptorBinding() : bindingId(pvr::types::DescriptorBindingDefaults::BindingId),
		arrayIndex(pvr::types::DescriptorBindingDefaults::ArraySize),
		descType(pvr::types::DescriptorBindingDefaults::Type) {}

	bool isValid() const
	{
		return !(bindingId == pvr::types::DescriptorBindingDefaults::BindingId ||
		         descType == pvr::types::DescriptorBindingDefaults::Type ||
		         arrayIndex == pvr::types::DescriptorBindingDefaults::ArraySize);
	}
};

}//namespace types

/*!*********************************************************************************************************************
\brief  Class wrapping an arithmetic type and providing bitwise operation for its bits
***********************************************************************************************************************/
template<typename Storage_>
class Bitfield
{
public:
	/*!
	   \brief Return true if a bit is set
	   \param store Bit storage
	   \param bit Bit to check
	   \return
	 */
	inline static bool isSet(Storage_ store, int8 bit)
	{
		return (store & (1 << bit)) != 0;
	}

	/*!
	   \brief Set a bit in the storage
	   \param store Storage
	   \param bit Bit to set
	 */
	inline static void set(Storage_ store, int8 bit)
	{
		store |= (1 << bit);
	}

	/*!
	   \brief Clear a bit fro the storage
	   \param store Storage
	   \param bit Bit to clear
	 */
	inline static void clear(Storage_ store, int8 bit)
	{
		store &= (!(1 << bit));
	}

};

}
