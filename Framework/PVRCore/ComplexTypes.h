#pragma once
#include "PVRCore/Types.h"

namespace pvr {
namespace types {

/*!*********************************************************************************************************************
\brief Enumeration of texture Swizzle mask channels.
***********************************************************************************************************************/
namespace Swizzle {
enum Enum : uint8 //DIRECT VULKAN MAPPING - DO NOT REARRANGE
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

}// namespace SwizzleMask
struct SwizzleChannels
{
	uint8 r;
	uint8 g;
	uint8 b;
	uint8 a;
	SwizzleChannels() : r(Swizzle::Identity), g(Swizzle::Identity), b(Swizzle::Identity), a(Swizzle::Identity) { };
	SwizzleChannels(Swizzle::Enum r, Swizzle::Enum g, Swizzle::Enum b, Swizzle::Enum a) :
		r(r), g(g), b(b), a(a) { };
};

struct ImageLayersSize
{
	uint16 numArrayLevels;  //!< The number of array slices of the range
	uint16 numMipLevels;   //!< The number of mipmap levels of the range
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
	ImageSubresource(uint16 mipLevelOffset = 0, uint16 arrayLayerOffset = 0) :
		arrayLayerOffset(arrayLayerOffset), mipLevelOffset(mipLevelOffset) {}
};


struct ImageExtents
{
	uint16 width;
	uint16 height;
	uint16 depth;
	ImageExtents(uint32 width = 1, uint32 height = 1, uint32 depth = 1) :
		width((uint16)width), height((uint16)height), depth((uint16)depth)
	{
		PVR_ASSERTION(width < 65536 && height < 65536 && depth < 65536
		              && "Error - Max supported image extent must fit into a 16-bit unsigned integer");
	}
};
struct ImageOffset
{
	uint16 offsetX;
	uint16 offsetY;
	uint16 offsetZ;
	ImageOffset(uint32 offsetX = 0, uint32 offsetY = 0, uint32 offsetZ = 0) :
		offsetX((uint16)offsetX), offsetY((uint16)offsetX), offsetZ((uint16)offsetX)
	{
		PVR_ASSERTION(offsetX < 65536 && offsetY < 65536 && offsetZ < 65536
		              && "Error - Max supported image offset must fit into a 16-bit unsigned integer");
	}
};

struct ImageAspectRange
{
	ImageAspect::Bits aspect;
	ImageAspectRange(ImageAspect::Enum aspect = ImageAspect::Color) : aspect(aspect) {}
};

struct ImageSubresourceRange : public ImageLayersSize, public ImageSubresource, ImageAspectRange
{
	ImageSubresourceRange() {}
	ImageSubresourceRange(ImageLayersSize layersSize, ImageSubresource baseLayers):
		ImageLayersSize(layersSize), ImageSubresource(baseLayers) {}
};

struct ImageRange : public ImageExtents, public ImageOffset
{
	ImageRange() {}
	ImageRange(ImageExtents extents, ImageOffset offset) :
		ImageExtents(extents), ImageOffset(offset) {}
};
struct ImageAreaSize : public ImageExtents, public ImageLayersSize
{
	ImageAreaSize() {}
	ImageAreaSize(ImageLayersSize layersSize, ImageExtents extents) :
		ImageExtents(extents), ImageLayersSize(layersSize) {}
};
struct ImageAreaOffset: public ImageSubresource, public ImageOffset
{
	ImageAreaOffset() {}
	ImageAreaOffset(ImageSubresource baseLayers, ImageOffset offset) :
		ImageSubresource(baseLayers), ImageOffset(offset) {}
};
struct ImageArea : public ImageAreaSize, ImageAreaOffset
{
	ImageArea() {}
	ImageArea(ImageLayersSize layerSize, ImageExtents extents, ImageSubresource baseLayers, ImageOffset offset):
		ImageAreaSize(layerSize, extents), ImageAreaOffset(baseLayers, offset) {}
	operator ImageSubresourceRange() const { return ImageSubresourceRange(*this, *this);  }

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
		wrapModeV(types::SamplerWrap::Repeat), wrapModeW(types::SamplerWrap::Repeat), compareMode(types::ComparisonMode::None),
		anisotropyMaximum(0.0f), lodBias(0.0f), lodMinimum(0.0f), lodMaximum(100.0f), unnormalizedCoordinates(false),
		borderColor(types::BorderColor::TransparentBlack) {}

	/*!*********************************************************************************************************************
	\brief Constructor that sets the filters. Set the filtering mode explicitly. Wrap.
	Creates a Sampler with wrap:Repeat, no comparison, no lod bias, no anisotropy. Can be edited after construction.
	\param[in] miniFilter  Minification filter: Nearest or Linear
	\param[in] magniFilter  Magnification filter: Nearest or Linear
	\param[in] mipMapFilter Mipmap interpolation filter: Nearest or Linear
	***********************************************************************************************************************/
	SamplerCreateParam(types::SamplerFilter::Enum magniFilter, types::SamplerFilter::Enum miniFilter,
	                   types::SamplerFilter::Enum mipMapFilter = types::SamplerFilter::Linear) :
		magnificationFilter(magniFilter), minificationFilter(miniFilter),
		mipMappingFilter(mipMapFilter), wrapModeU(types::SamplerWrap::Repeat), wrapModeV(types::SamplerWrap::Repeat),
		wrapModeW(types::SamplerWrap::Repeat), compareMode(types::ComparisonMode::None), anisotropyMaximum(0.0f),
		lodBias(0.0f), lodMinimum(0.0f), lodMaximum(100.0f), unnormalizedCoordinates(false),
		borderColor(types::BorderColor::TransparentBlack)
	{}

	types::SamplerFilter::Enum magnificationFilter; //!< Texture Magnification interpolation filter. Nearest or Linear. Default Nearest.
	types::SamplerFilter::Enum minificationFilter; //!< Texture Minification interpolation filter. Nearest or Linear. Default Nearest.
	types::SamplerFilter::Enum mipMappingFilter; //!< Texture Mipmap interpolation filter. Nearest, Linear or None (for no mipmaps). Default Linear.
	types::SamplerWrap::Enum wrapModeU; //!< Texture Wrap mode, U (x) direction (Repeat, Mirror, MirrorRepeat, Border, Clamp). Default Repeat.
	types::SamplerWrap::Enum wrapModeV; //!< Texture Wrap mode, V (y) direction (Repeat, Mirror, MirrorRepeat, Border, Clamp). Default Repeat.
	types::SamplerWrap::Enum wrapModeW; //!< Texture Wrap mode, W (z) direction (Repeat, Mirror, MirrorRepeat, Border, Clamp). Default Repeat.
	types::ComparisonMode::Enum compareMode; //!< Comparison mode for comparison samplers. Default None.
	float32 anisotropyMaximum; //!< Texture anisotropic filtering. Default 0.
	float32 lodBias;           //!< Texture level-of-detail bias (bias of mipmap to select). Default 0.
	float32 lodMinimum;        //!< Texture minimum level-of-detail (mipmap). Default 0.
	float32 lodMaximum;        //!< Texture maximum level-of-detail (mipmap). Default 0.
	bool unnormalizedCoordinates; //!< Texture Coordinates are Un-Normalized
	types::BorderColor::Enum borderColor; //!< Texture border color. Only used with Border wrap mode.
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
	types::DataType::Enum dataType;
	uint16 offset; // Should be 16 bit?
	uint8 width; // Number of values per vertex
	VertexAttributeLayout() {}
	VertexAttributeLayout(types::DataType::Enum dataType, uint8 width, uint16 offset) :
		dataType(dataType), offset(offset), width(width) {}
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
}
