/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\Texture.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         The PVRApi basic Texture implementation.
***********************************************************************************************************************/
#pragma once
#include "PVRAssets/Texture/PixelFormat.h"
#include "PVRCore/ForwardDecApiObjects.h"
#include "PVRApi/Bindables.h"

namespace pvr {
namespace api {
namespace impl {
class DescriptorSetGlesImpl;
}
/*!*********************************************************************************************************************
\brief Enumeration of texture Swizzle mask channels.
***********************************************************************************************************************/
namespace SwizzleMask {
enum Enum
{
	Unset,
	RedChannel = 1,
	Red = 1,
	R = 1,
	GreenChannel = 2,
	Green = 2,
	G = 2,
	BlueChannel = 3,
	Blue = 3,
	B = 3,
	AlphaChannel = 4,
	Alpha = 4,
	A = 4,
	Zero = 5,
	One = 6,
};
}// namespace SwizzleMask

/*!*********************************************************************************************************************
\brief Represents an image format, including pixel format(channels/bits per channel), datatype and colorspace.
***********************************************************************************************************************/
struct ImageDataFormat
{
	PixelFormat format;//!< pixel format
	VariableType::Enum dataType;//!< datatype
	ColorSpace::Enum colorSpace;//!< colorspace, e.g lRGB

	/*!*******************************************************************************************
	\param format The Pixel format
	\param dataType The datatype (e.g. UnsignedIntNormalized)
	\param colorSpace The colorspace e.g lRGB/sRGB
	**********************************************************************************************/
	ImageDataFormat(const PixelFormat& format = PixelFormat::RGBA_8888,
	                VariableType::Enum dataType = VariableType::UnsignedByteNorm,
	                ColorSpace::Enum colorSpace = ColorSpace::lRGB) :
		format(format), dataType(dataType), colorSpace(colorSpace) { }

	bool operator ==(const ImageDataFormat& rhs)const
	{
		return (format == rhs.format && dataType == rhs.dataType &&
		        colorSpace == rhs.colorSpace);
	}

	bool operator !=(const ImageDataFormat& rhs)const { return !(*this == rhs); }
};

/*!*******************************************************************************************
\brief ImageStorageFormat extends the ImageDataFormat with mipmaps and number of Samples.
**********************************************************************************************/
struct ImageStorageFormat : public ImageDataFormat
{
	int8 mipmapLevels;//< number of mip levels
	pvr::uint8 numSamples;//< number of samples

	/*!*******************************************************************************************
	\brief Constructor. Initialises to the provided values.
	\param format Pixel format
	\param mipmapLevels Number of mip levels
	\param colorSpace Color space (linear RGB or sRGB)
	\param dataType The DataType
	\param numSamples number of samples
	**********************************************************************************************/
	ImageStorageFormat(const PixelFormat& format = PixelFormat::RGBA_8888, int8 mipmapLevels = 1,
	                   ColorSpace::Enum colorSpace = ColorSpace::lRGB,
	                   VariableType::Enum dataType = VariableType::UnsignedByteNorm, pvr::uint8 numSamples = 1)
		: ImageDataFormat(format, dataType, colorSpace),
		  mipmapLevels(mipmapLevels), numSamples(numSamples) {}
};

/*!************************************************************************************************************
\brief Class used by texture update functions. Represents an area of the texture to be updated. Default value:
width=1, height=1, (depth=1), offsetx=0,offsety=0,offsetz=0, arrayslice=0, cubeface=0, miplevel=0
***************************************************************************************************************/
struct TextureArea
{
	uint32 width;  //< ! X-axis size (width) of the area of the texture to update. Default 1. At least 1.
	uint32 height; //< ! Y-axis size (height) of the area of the texture to update. Default 1. At least 1.
	union
	{
		uint32 depth;  //< ! Z-axis size (depth) of the area of the texture to update. Default 1. At least 1. IGNORED for 2D Textures.
		uint16 arraySize;      //<! Number of array slices of the area. IGNORED for non-array textures.
	};
	uint32 offsetx; //< ! x-coordinate of the start point of the area of the texture to update. Default 0.
	uint32 offsety; //< ! y-coordinate of the start point of the area of the texture to update. Default 0.
	union
	{
		uint32 offsetz; //< ! z-coordinate of the start point of the area of the texture to update. IGNORED for 2D Textures. Default 0.
		uint16 arrayIndex;     //<! Array index of the starting array slice of the area. IGNORED for non-array textures.
	};
	uint32 compressedSize; //<! Size of the actual data that will be provided for updating a compressed texture. IGNORED for uncompressed textures.
	uint8 cubeFace;        //<! Which face of the Cube texture to update. IGNORED for non-cube textures.
	uint8 mipLevel;        //<! Which mipmap level of the texture to update. Default 0;

	/*!************************************************************************************************************
	\brief Construct an empty texture area object
	**************************************************************************************************************/
	TextureArea() : width(1), height(1), depth(1),
		offsetx(0), offsety(0), offsetz(0),
		compressedSize(0), cubeFace(0), mipLevel(0)
	{
	}

	/*!************************************************************************************************************
	\brief Construct a texture area representing the most common case (zero-offset for a 2d uncompressed texture,
	        z-dimension 1)
	\param width The size of the textureArea along the x-direction, in texels  (default 1)
	\param height The size of the textureArea along the y-direction, in texels  (default 1)
	**************************************************************************************************************/
	TextureArea(uint32 width, uint32 height) : width(width), height(height), depth(1),
		offsetx(0), offsety(0), offsetz(0),
		compressedSize(0), cubeFace(0), mipLevel(0)
	{
	}

	/*!************************************************************************************************************
	\brief Sets the size of a compressed texture. For a compressed texture, the dimensions cannot be used directly.
	\param compressedSize The size, in bytes, of the texture.
	***************************************************************************************************************/
	void setCompressedSize(uint32 compressedSize) { this->compressedSize = compressedSize; }

	/*!************************************************************************************************************
	\brief Set the basic dimensions of the texture area (width/x,height/y,depth/z) in texels
	\param width The size of the textureArea along the x-direction, in texels 
	\param height The size of the textureArea along the y-direction, in texels 
	\param depth The size of the textureArea along the z-direction (default 1),in texels 
	***************************************************************************************************************/
	void setDimensions(uint32 width, uint32 height, uint32 depth = 1) { this->width = width; this->height = height; this->depth = depth; }
	
	/*!************************************************************************************************************
	\brief Set the basic dimensions of the texture area, in pixels (width/x,height/y,depth/z)
	\param size A 3d unsigned int vector containing the size of the area in each dimension, in texels
			(x=width, y=height, z=depth)
	***************************************************************************************************************/
	void setDimensions(const glm::uvec3& size = glm::uvec3(1, 1, 1)) { this->width = size.x; this->height = size.y; this->depth = size.z; }

	/*!************************************************************************************************************
	\brief Set the offset of the texture area is the distance along each direction from (0,0,0), in texels
	\param offsetx The distance along the x-axis of the leftmost extent of the area from the left
	\param offsety The distance along the y-axis of the topmost extent of the area from the top
	\param offsetz The distance along the z-axis of the foremost extent of the area from the front
	***************************************************************************************************************/
	void setOffset(uint32 offsetx, uint32 offsety, uint32 offsetz = 0) { this->offsetx = offsetx; this->offsety = offsety; this->offsetz = offsetz; }

	/*!************************************************************************************************************
	\brief Set the offset of the texture area is the distance along each direction from (0,0,0), in texels. 
			Initial value (initial value 0,0,0)
	\param offset The offset of the area from the start described as a vector of unsigned integers
	***************************************************************************************************************/
	void setOffset(const glm::uvec3& offset = glm::uvec3(0, 0, 0)) { this->width = offset.x; this->height = offset.y; this->depth = offset.z; }

	/*!************************************************************************************************************
	\brief Set the mipmap level that the area represents (initial value 0)
	\param mipLevel The mipmap level that the area represents
	***************************************************************************************************************/
	void setMipLevel(uint32 mipLevel) { this->mipLevel = (uint8)mipLevel; }

	/*!************************************************************************************************************
	\brief Set the array slice of an array texture that the TextureArea represents (initial value 0)
	\param arrayIndex The array slice that the area represents
	***************************************************************************************************************/
	void setArraySlice(uint32 arrayIndex) { this->arrayIndex = (uint16)arrayIndex; }

	/*!************************************************************************************************************
	\brief Set the Cube face of a Cube Texture that the area represents (initial value CubeFacePositiveX)
	\param cubeFace The cube face that the area represents
	***************************************************************************************************************/
	void setCubeFace(CubeFace::Enum cubeFace) { this->cubeFace = (uint8)cubeFace; }
};

/*!*******************************************************************************************
\brief Describes a Compressed format. Compressed formats provide less information than the
uncompressed format, as they can only be accessed "black box".
**********************************************************************************************/
struct CompressedImageDataFormat
{
	CompressedPixelFormat::Enum format;//!< compressed format
};

/*!*******************************************************************************************
\brief Describes a Compressed format. Compressed formats provide less information than the
uncompressed format, as they can only be accessed "black box".
**********************************************************************************************/
struct ImageStorageFormatCompressed : public CompressedImageDataFormat
{
	int8 mipmapLevels;//!< number of mip levels
};

/*!*******************************************************************************************
\brief Enumeration of the "aspect" (or "semantics") of an image: Color, Depth, Stencil.
**********************************************************************************************/
namespace ImageAspect {
enum Enum
{
	Color,
	Depth,
	Stencil
};
}

/*!*******************************************************************************************
\brief Describes an area of an image, out of which a SubResource can be defined. This area is
       a subarea of the image, containing a part of the image array slices, mip levels, or aspects.
**********************************************************************************************/
struct ImageSubResourceRange
{
	pvr::uint16 baseArraySize; //!< The index of the first array slice of the range
	pvr::uint16 numArraySize;  //!< The number of array slices of the range
	pvr::uint8 baseMipLevel;   //!< The index of the first mipmap levels οf the range
	pvr::uint8 numMipLevels;   //!< The number of mipmap levels of the range
	ImageAspect::Enum aspect;  //!< The aspect of the range
};

class BindDescriptorSets;
namespace impl {
class TextureViewImpl;

/*!*******************************************************************************************
\brief The class powering any texture. Wraps the underlying API Texture object and represents
the storage bits of any texture. Is normally used through a TextureView object.
Wrapped and accessed in a TextureStore Reference Counted Framework object.
**********************************************************************************************/
class TextureStoreImpl
{
	friend class TextureViewImpl;
	friend class ::pvr::api::impl::DescriptorSetImpl;
	friend class ::pvr::api::BindDescriptorSets;
	template<typename, typename> friend class ::pvr::api::impl::PackagedBindableWithParam;
	TextureStoreImpl& operator=(const TextureStoreImpl&);//deleted
	TextureStoreImpl(const TextureStoreImpl&);//deleted
protected:
	native::HTexture m_texture;
	GraphicsContext context;
	bool destroy();
public:
	ImageStorageFormat format;

	/*!*******************************************************************************************
	\brief Returns a const native handle to the texture. Calling this function will require including
	       the API-specific headers.
	\return A const HTexture (thin smart pointer wrapper) over the underlying API object
	**********************************************************************************************/
	const native::HTexture& getNativeHandle() const { return m_texture; }

	/*!*******************************************************************************************
	\brief Returns a native handle to the texture. Calling this function will require including
	       the API-specific headers.
	\return An HTexture (thin smart pointer wrapper) over the underlying API object
	**********************************************************************************************/
	native::HTexture& getNativeHandle() { return m_texture; }


	/*!*******************************************************************************************
	\brief Return the basic dimensioning of the texture (1D/2D/3D).
	\return The TextureDimension
	**********************************************************************************************/
	TextureDimension::Enum getDimensions() const;

	/*!*******************************************************************************************
	\brief Check if this texture is allocated.
	\return true if the texture is allocated. Otherwise, the texture is empty and must be constructed.
	**********************************************************************************************/
	bool isAllocated() const;

	/*!*******************************************************************************************
	\brief Get the storage format of this texture.
	\return The storage format of this texture.
	**********************************************************************************************/
	const ImageStorageFormat& getFormat() const { return format; }

	/*!*******************************************************************************************
	\brief Constructor.
	\param context The GraphicsContext where this Texture will belong
	**********************************************************************************************/
	TextureStoreImpl(GraphicsContext& context);

	/*!*******************************************************************************************
	\brief Constructor. Use to wrap a preexisting, underlying texture object.
	\param context The GraphicsContext where this Texture will belong
	\param texture An already existing texture object of the underlying API. The underlying object 
	       will be then owned by this TextureStore object, destroying it when done. If shared
		   semantics are required, use the overload accepting a smart pointer object.
	\description NOTE: This object will take ownership of the passed texture object, destroying it in its destructor.
	**********************************************************************************************/
	TextureStoreImpl(GraphicsContext& context, const native::HTexture_& texture);

	/*!*******************************************************************************************
	\brief Constructor. Use to wrap a preexisting, underlying texture object. 
	\param context The GraphicsContext where this Texture will belong
	\param texture An already existing smart pointer to texture object of the underlying API. The
			underlying object  can be shared. 
	**********************************************************************************************/
	TextureStoreImpl(GraphicsContext& context, const native::HTexture& texture)
		: m_texture(texture), context(context) { }

	/*!*******************************************************************************************
	\brief Destructor. Will properly release all resources held by this object.
	**********************************************************************************************/
	~TextureStoreImpl();

	/*!************************************************************************************************************
	\brief	Return the API's texture handle object.
	\param red    The swizzling that will be applied on the texel's "red" channel before that is returned to the shader.
	\param green    The swizzling that will be applied on the texel's "green" channel before that is returned to the shader.
	\param blue    The swizzling that will be applied on the texel's "blue" channel before that is returned to the shader.
	\param alpha    The swizzling that will be applied on the texel's "alpha" channel before that is returned to the shader.
	***************************************************************************************************************/
	void setSwizzle(SwizzleMask::Enum red = SwizzleMask::Unset, SwizzleMask::Enum green = SwizzleMask::Unset,
	                SwizzleMask::Enum blue = SwizzleMask::Unset, SwizzleMask::Enum alpha = SwizzleMask::Unset);


};


/*!************************************************************************************************************
\brief Base texture view class.
***************************************************************************************************************/
class TextureViewImpl
{
	friend class ::pvr::api::impl::DescriptorSetImpl;
	friend class ::pvr::api::impl::DescriptorSetGlesImpl;
	template<typename, typename> friend class ::pvr::api::impl::PackagedBindableWithParam;
	friend class ::pvr::api::impl::SamplerImpl;
protected:
	TextureStore resource;//!<Texture view implementations access the underlying texture through this
	mutable const SamplerImpl* m_sampler;// for ES2
	/*!************************************************************************************************************
	\brief DescriptorSets use this function to bind the texture to a texture unit.
	***************************************************************************************************************/
	void bind(IGraphicsContext& context, uint16 bindIdx)const;
public:
	/*!************************************************************************************************************
	\brief Get the dimensionality of this texture.
	***************************************************************************************************************/
	TextureDimension::Enum getTextureType()const { return resource->getDimensions(); }

	virtual ~TextureViewImpl() {}

	/*!************************************************************************************************************
	\brief Create a TextureView by wrapping an existing API texture and taking ownership of it.
	\param context The GraphicsContext on which to create the Texture
	\param texture An already created Native Texture object. The object constructed will take ownership of it.
	***************************************************************************************************************/
	TextureViewImpl(GraphicsContext& context, const native::HTexture_& texture);

	/*!************************************************************************************************************
	\brief Create a new TextureView object. It will be unallocated and contain no actual underlying Texture until
	allocated.
	***************************************************************************************************************/
	TextureViewImpl(GraphicsContext& context);
	void setSwizzle(SwizzleMask::Enum red = SwizzleMask::Unset, SwizzleMask::Enum green = SwizzleMask::Unset,
	                SwizzleMask::Enum blue = SwizzleMask::Unset, SwizzleMask::Enum alpha = SwizzleMask::Unset)
	{
		resource->setSwizzle(red, green, blue, alpha);
	}

	/*!************************************************************************************************************
	\brief Initialise the texture with the specified parameters. Only valid once.
	***************************************************************************************************************/
	void allocate2D(const ImageStorageFormat& format, uint32 width, uint32 height);

	/*!************************************************************************************************************
	\brief Initialise the texture with the specified parameters. Only valid once.
	***************************************************************************************************************/
	void allocate2DCube(const ImageStorageFormat& format, uint32 width, uint32 height);

	/*!************************************************************************************************************
	\brief Initialise the texture with the specified parameters. Only valid once.
	***************************************************************************************************************/
	void allocate2DArray(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 arraySize);

	/*!************************************************************************************************************
	\brief Initialise the texture with the specified parameters. Only valid once.
	***************************************************************************************************************/
	void allocate3D(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 depth);

	/*!************************************************************************************************************
	\brief Update the data of the texture. DOES NOT WORK WITH COMPRESSED TEXTURES.
	\param data Pointer to the memory which will be copied by the texture
	\param format The format of "data" pointer
	\param area A TextureArea object describing the area of the texture that will be updated by this call
	***************************************************************************************************************/
	void update(const void* data, const ImageDataFormat& format, const TextureArea& area);

	/*!************************************************************************************************************
	\brief Get the underlying TextureStore object.
	***************************************************************************************************************/
	const TextureStore& getResource() const { return resource; }

	/*!************************************************************************************************************
	\brief Query if this object contains a valid reference to an actual Texture.
	***************************************************************************************************************/
	bool isAllocated()	{ return (resource.isValid() && resource->isAllocated()); }

};
}//namespace impl


/*!****************************************************************************************************************
\brief	Get the display's color, depth and or stencil format from a DisplayAttributes object.
\param[in]	attribute The DisplayAttributes to query. Normally from shell.getDisplayAttributes()
\param[out]	outColorFmt The Color Format that this DisplayAttributes object has
\param[out]	outDepthStencilFmt The DepthStencilFormat that this DisplayAttributes object has
\description This function is usually called on an object found with Shell::getDisplayAttributes(), in order to
determine the formats that are necessary for an On Screen FBO.
*******************************************************************************************************************/
inline void getDisplayFormat(const system::DisplayAttributes& attribute,
                             api::ImageDataFormat* outColorFmt,
                             api::ImageDataFormat* outDepthStencilFmt)
{
	if (outColorFmt)
	{
		outColorFmt->format = PixelFormat(attribute.redBits ? 'r' : 0, attribute.greenBits ? 'g' : 0,
		                                  attribute.blueBits ? 'b' : 0, attribute.alphaBits ? 'a' : 0,
		                                  (uint8)attribute.redBits, (uint8)attribute.greenBits,
		                                  (uint8)attribute.blueBits, (uint8)attribute.alphaBits);
	}
	if (outDepthStencilFmt)
	{
		outDepthStencilFmt->format = PixelFormat('d', (attribute.stencilBPP ? 's' : 0), (uint8)0, (uint8)0,
		                                (uint8)attribute.depthBPP, (uint8)attribute.stencilBPP, (uint8)0, (uint8)0);
	}
}

/*!****************************************************************************************************************
\brief	    Get the color format from a Displaydisplay's color format.
\param[in]	attribute The DisplayAttributes to query. Normally from shell.getDisplayAttributes()
\return     outColorFmt The Color Format that this DisplayAttributes object has
\description This function is usually called on an object found with Shell::getDisplayAttributes(), in order to
determine the formats that are necessary for an On Screen FBO.
*******************************************************************************************************************/
inline api::ImageDataFormat getDisplayColorFormat(const system::DisplayAttributes& attribute)
{
	api::ImageDataFormat outFmt;
	getDisplayFormat(attribute, &outFmt, NULL);
	return outFmt;
}

/*!****************************************************************************************************************
\brief	    Get the depth and/or stencil format from a Displaydisplay's color format.
\param[in]	attribute The DisplayAttributes to query. Normally from shell.getDisplayAttributes()
\return     outDepthStencilFmt The DepthStencilFormat that this DisplayAttributes object has
\description This function is usually called on an object found with Shell::getDisplayAttributes(), in order to
determine the formats that are necessary for an On Screen FBO.
*******************************************************************************************************************/
inline api::ImageDataFormat getDisplayDepthStencilFormat(const system::DisplayAttributes& attribute)
{
	api::ImageDataFormat outFmt;
	getDisplayFormat(attribute, NULL, &outFmt);
	return outFmt;
}
}//namespace api
}//namespace pvr


