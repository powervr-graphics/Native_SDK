/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/TextureVk.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains Vulkan specific implementation of the Texture class. Use only if directly using Vulkan calls.
			  Provides the definitions allowing to move from the Framework object Texture2D to the underlying Vulkan Texture.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"

namespace pvr {
namespace api {
namespace vulkan {
/*!*********************************************************************************************************************
\brief TextureStoreVk_ implementation that wraps the Vulkan Texture object
***********************************************************************************************************************/
class TextureStoreVk_ : public native::HTexture_, public impl::TextureStore_
{
public:
	types::TextureDimension::Enum dimension;
	types::ImageExtents extents;
	types::ImageLayersSize layersSize;

	/*!*******************************************************************************************
	\brief Return the basic dimensioning of the texture (1D/2D/3D).
	\return The TextureDimension
	**********************************************************************************************/
	types::TextureDimension::Enum getDimension() const { return dimension; }

	/*!*******************************************************************************************
	\brief Return a reference to the format of the texture
	\return The reference to the ImageStorageFormat
	**********************************************************************************************/
	ImageStorageFormat&  getFormat() { return format; }
	
	/*!*******************************************************************************************
	\brief Return a const reference to the format of the texture
	\return The const reference to the ImageStorageFormat
	**********************************************************************************************/
	const ImageStorageFormat&  getFormat() const { return format; }

	/*!*******************************************************************************************
	\brief Set the dimension of this texture
	\param extents Texture dimension
	**********************************************************************************************/
	void setDimensions(types::ImageExtents extents) { this->extents = extents; }
	
	/*!*******************************************************************************************
	\brief Set this texture layer
	\return layerSize Texture layer size
	**********************************************************************************************/
	void setLayers(types::ImageLayersSize layersSize) { this->layersSize = layersSize; }

	/*!*******************************************************************************************
	\brief Check if this texture is allocated.
	\return true if the texture is allocated. Otherwise, the texture is empty and must be constructed.
	**********************************************************************************************/
	bool isAllocated() const { return (image != VK_NULL_HANDLE) && (memory != VK_NULL_HANDLE); }

	/*!*******************************************************************************************
	\brief Constructor.
	\param context The GraphicsContext where this Texture will belong
	**********************************************************************************************/
	TextureStoreVk_(GraphicsContext& context) : TextureStore_(context) {}

	/*!*******************************************************************************************
	\brief Constructor. Use to wrap a preexisting, underlying texture object.
	\param context The GraphicsContext where this Texture will belong
	\param texture An already existing texture object of the underlying API. The underlying object
	will be then owned by this TextureStore object, destroying it when done. If shared
	semantics are required, use the overload accepting a smart pointer object.
	\description NOTE: This object will take ownership of the passed texture object, destroying it in its destructor.
	**********************************************************************************************/
	TextureStoreVk_(GraphicsContext& context, const native::HTexture_& texture, types::TextureDimension::Enum textureDim) :
		HTexture_(texture), TextureStore_(context), dimension(textureDim) {}

	/*!*******************************************************************************************
	\brief Destructor. Will properly release all resources held by this object.
	**********************************************************************************************/
	~TextureStoreVk_() {}
};

typedef RefCountedResource<TextureStoreVk_> TextureStoreVk;
}
}
}

PVR_DECLARE_NATIVE_CAST(TextureStore);

namespace pvr {
namespace api {
namespace vulkan {
/*!*********************************************************************************************************************
\brief TextureViewVk_ implementation that wraps the Vulkan Texture View object
***********************************************************************************************************************/	
class TextureViewVk_ : public native::HImageView_, public impl::TextureView_
{
public:

	/*!*********************************************************************************************************************
	\brief ctor.
	\param texture	Texture to create this view on.
	\param range	This view range
	\param swizzleChannels Channels to be swizzled 
	***********************************************************************************************************************/	
	TextureViewVk_(const TextureStoreVk& texture, const types::ImageSubresourceRange& range = types::ImageSubresourceRange(),
				   types::SwizzleChannels swizzleChannels = types::SwizzleChannels());
		
	/*!*********************************************************************************************************************
	\brief ctor.
	\param texture	Texture to create this view on.
	\param view	Native texture view object to be wrapped
	***********************************************************************************************************************/			
	TextureViewVk_(const TextureStoreVk& texture, const native::HImageView_& view) : TextureView_(texture, view) {}
	
	/*!*********************************************************************************************************************
	\brief dtor
	***********************************************************************************************************************/	
	~TextureViewVk_(){	destroy();	}
	
	/*!*********************************************************************************************************************
	\brief Destroy this textrue view object
	***********************************************************************************************************************/	
	void destroy();
};

typedef RefCountedResource<TextureViewVk_> TextureViewVk;
}

}

namespace native {
/*!*********************************************************************************************************************
\brief Get the Vulkan texture object underlying a PVRApi Texture object.
\return A smart pointer wrapper containing the Vulkan Texture
\description The smart pointer returned by this function will keep alive the underlying Vulkan object even if all other
references to the texture (including the one that was passed to this function) are released.
***********************************************************************************************************************/
inline HTexture createNativeHandle(const api::TextureStore& texture)
{
	return static_cast<api::vulkan::TextureStoreVk>(texture);
}
}//namespace native
}
PVR_DECLARE_NATIVE_CAST(TextureView);