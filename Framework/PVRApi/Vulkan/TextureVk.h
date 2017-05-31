/*!
\brief Contains Vulkan specific implementation of the Texture class. Use only if directly using Vulkan calls. Provides
the definitions allowing to move from the Framework object Texture2D to the underlying Vulkan Texture.
\file PVRApi/Vulkan/TextureVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"

namespace pvr {
namespace api {
namespace vulkan {
/// <summary>TextureStoreVk_ implementation that wraps the Vulkan Texture object</summary>
class TextureStoreVk_ : public native::HTexture_, public impl::TextureStore_
{
public:
	types::ImageLayout currentLayout;

	/// <summary>Return a reference to the format of the texture</summary>
	/// <returns>The reference to the ImageStorageFormat</returns>
	ImageStorageFormat&  getFormat() { return _format; }

	/// <summary>Set this texture format</summary>
	/// <param name="format">Texture format</param>
	void setFormat(const ImageStorageFormat& format) { _format = format; }

	/// <summary>Set the dimension of this texture</summary>
	/// <param name="extents">Texture dimension</param>
	void setDimensions(types::Extent3D extents)
	{
		assertion(extents.width > 0 && extents.height > 0 && extents.depth > 0);
		if (extents.height > 1 && extents.depth > 1) { _imageBaseType = types::ImageBaseType::Image3D; }
		else if (extents.height > 1) { _imageBaseType = types::ImageBaseType::Image2D; }
		else { _imageBaseType = types::ImageBaseType::Image1D; }
		_extents = extents;
	}

	/// <summary>Set this texture layer</summary>
	/// <returns>layerSize Texture layer size</returns>
	void setLayers(types::ImageLayersSize layersSize)
	{
		_layersSize = layersSize;
	}
	/// <summary>Set this texture layer</summary>
	/// <returns>layerSize Texture layer size</returns>
	const types::ImageLayersSize& getLayers() const { return _layersSize; }

	/// <summary>Set number of samples</summary>
	/// <param name="samplesCount"></param>
	void setNumSamples(types::SampleCount samplesCount) { _samplesCount = samplesCount; }

	/// <summary>Return number of samples</summary>
	types::SampleCount getNumSamples()const { return _samplesCount; }

	/// <summary>Default constructor</summary>
	TextureStoreVk_() : TextureStore_(), currentLayout(types::ImageLayout::Undefined) {}

	/// <summary>Constructor.</summary>
	/// <param name="context">The GraphicsContext where this Texture will belong</param>
	TextureStoreVk_(const GraphicsContext& context) : TextureStore_(context), currentLayout(types::ImageLayout::Undefined) {}

	/// <summary>Constructor. Use to wrap a preexisting, underlying texture object.</summary>
	/// <param name="context">The GraphicsContext where this Texture will belong</param>
	/// <param name="texture">An already existing texture object of the underlying API. The underlying object will be
	/// then owned by this TextureStore object, destroying it when done. If shared semantics are required, use the
	/// overload accepting a smart pointer object.</param>
	/// <remarks>NOTE: This object will take ownership of the passed texture object, destroying it in its destructor.
	/// </remarks>
	TextureStoreVk_(const GraphicsContext& context, const native::HTexture_& texture,
	                types::ImageBaseType imageBaseType, bool isCubeMap = false,
	                types::ImageLayout initialLayout = types::ImageLayout::Undefined) :
		HTexture_(texture), TextureStore_(context, isCubeMap, imageBaseType), currentLayout(initialLayout)
	{
	}

	/// <summary>Destructor. Will properly release all resources held by this object.</summary>
	virtual ~TextureStoreVk_();


	bool isAllocated_() const;
	void allocate2D_(const ImageStorageFormat& format, uint32 width, uint32 height, types::ImageUsageFlags usage, types::ImageLayout finalLayout);
	void allocate2DMS_(const ImageStorageFormat& format, uint32 width, uint32 height, types::ImageUsageFlags usage, types::ImageLayout finalLayout);
	void allocate2DArrayMS_(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 arraySize, types::ImageUsageFlags usage, types::ImageLayout finalLayout);
	void allocateTransient_(const ImageStorageFormat& format, uint32 width, uint32 height, types::ImageUsageFlags usage, types::ImageLayout finalLayout);
	void allocateStorage_(const ImageStorageFormat& format, uint32 width, uint32 height);
	void allocate2DCube_(const ImageStorageFormat& format, uint32 width, uint32 height, types::ImageUsageFlags usage, types::ImageLayout initialLayout);
	void allocate2DArray_(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 arraySize, types::ImageUsageFlags usage, types::ImageLayout finalLayout);
	void allocate3D_(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 depth, types::ImageUsageFlags usage, types::ImageLayout finalLayout);
	void update_(const void* data, const ImageDataFormat& format, const TextureArea& area);
};

typedef RefCountedResource<TextureStoreVk_> TextureStoreVk;
}
}
}

PVR_DECLARE_NATIVE_CAST(TextureStore);

namespace pvr {
namespace api {
namespace vulkan {
/// <summary>TextureViewVk_ implementation that wraps the Vulkan Texture View object</summary>
class TextureViewVk_ : public native::HImageView_, public impl::TextureView_
{
public:

	/// <summary>ctor.</summary>
	/// <param name="texture">Texture to create this view on.</param>
	/// <param name="range">This view range</param>
	/// <param name="swizzleChannels">Channels to be swizzled</param>
	TextureViewVk_(const TextureStoreVk& texture, const types::ImageSubresourceRange& range = types::ImageSubresourceRange(),
	               types::SwizzleChannels swizzleChannels = types::SwizzleChannels());

	/// <summary>ctor.</summary>
	/// <param name="texture">Texture to create this view on.</param>
	/// <param name="view">Native texture view object to be wrapped</param>
	TextureViewVk_(const TextureStoreVk& texture, const native::HImageView_& view) : TextureView_(texture, view) {}

	/// <summary>dtor</summary>
	~TextureViewVk_() { destroy();  }

	/// <summary>Destroy this textrue view object</summary>
	void destroy();
};

typedef RefCountedResource<TextureViewVk_> TextureViewVk;
}// namespace vulkan
}//namespace api

namespace native {
/// <summary>Get the Vulkan texture object underlying a PVRApi Texture object.</summary>
/// <returns>A smart pointer wrapper containing the Vulkan Texture</returns>
/// <remarks>The smart pointer returned by this function will keep alive the underlying Vulkan object even if all
/// other references to the texture (including the one that was passed to this function) are released.</remarks>
inline HTexture createNativeHandle(const api::TextureStore& texture)
{
	return static_cast<api::vulkan::TextureStoreVk>(texture);
}
}//namespace native
}
PVR_DECLARE_NATIVE_CAST(TextureView);

