<<<<<<< HEAD
/*!*********************************************************************************************************************
\file         PVRApi/OGLES/TextureGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains OpenGL ES specific implementation of the Texture class. Use only if directly using OpenGL ES calls.
Provides the definitions allowing to move from the Framework object Texture2D to the underlying OpenGL ES Texture.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
=======
/*!
\brief Contains OpenGL ES specific implementation of the Texture class. Use only if directly using OpenGL ES calls.
Provides the definitions allowing to move from the Framework object Texture2D to the underlying OpenGL ES
Texture.
\file PVRApi/OGLES/TextureGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
>>>>>>> 1776432f... 4.3
#pragma once
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
#include "PVRNativeApi/OGLES/ConvertToApiTypes.h"
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRNativeApi/OGLES/ApiErrorsGles.h"

namespace pvr {

namespace api {
namespace gles {
class TextureStoreGles_ : public native::HTexture_, public impl::TextureStore_
{
public:
<<<<<<< HEAD
	/*!*******************************************************************************************
	\brief Return the basic dimensioning of the texture (1D/2D/3D).
	\return The TextureDimension
	**********************************************************************************************/
	types::ImageBaseType getDimensions() const;

	/*!*******************************************************************************************
	\brief Check if this texture is allocated.
	\return true if the texture is allocated. Otherwise, the texture is empty and must be constructed.
	**********************************************************************************************/
	bool isAllocated() const { return handle != 0; }

	/*!*******************************************************************************************
	\brief Constructor.
	\param context The GraphicsContext where this Texture will belong
	**********************************************************************************************/
	TextureStoreGles_(GraphicsContext& context) : HTexture_(0, 0), TextureStore_(context), m_sampler(0) {}

	/*!*******************************************************************************************
	\brief Constructor.
	\param context The GraphicsContext where this Texture will belong
	**********************************************************************************************/
	TextureStoreGles_() : HTexture_(0, 0), m_sampler(0) {}

	/*!*******************************************************************************************
	\brief Return a reference to the format of the texture
	\return The reference to the ImageStorageFormat
	**********************************************************************************************/
	ImageStorageFormat&  getFormat() { return format; }

	/*!*******************************************************************************************
	\brief Constructor. Use to wrap a preexisting, underlying texture object.
	\param context The GraphicsContext where this Texture will belong
	\param texture An already existing texture object of the underlying API. The underlying object
	will be then owned by this TextureStore object, destroying it when done. If shared
	semantics are required, use the overload accepting a smart pointer object.
	\description NOTE: This object will take ownership of the passed texture object, destroying it in its destructor.
	**********************************************************************************************/
	TextureStoreGles_(GraphicsContext& context, const native::HTexture_& texture) : HTexture_(texture), TextureStore_(context), m_sampler(0) {}

	/*!*******************************************************************************************
	\brief Destructor. Will properly release all resources held by this object.
	**********************************************************************************************/
	~TextureStoreGles_();


	/*!************************************************************************************************************
	\brief	Return the API's texture handle object.
	\param red    The swizzling that will be applied on the texel's "red" channel before that is returned to the shader.
	\param green    The swizzling that will be applied on the texel's "green" channel before that is returned to the shader.
	\param blue    The swizzling that will be applied on the texel's "blue" channel before that is returned to the shader.
	\param alpha    The swizzling that will be applied on the texel's "alpha" channel before that is returned to the shader.
	***************************************************************************************************************/
	void setSwizzle(types::Swizzle red = types::Swizzle::Identity, types::Swizzle green = types::Swizzle::Identity,
	                types::Swizzle blue = types::Swizzle::Identity, types::Swizzle alpha = types::Swizzle::Identity);

	/*!*******************************************************************************************
	\brief Set the dimension of this texture
	\param extents Texture dimension
	**********************************************************************************************/
	void setDimensions(types::Extent3D extents)
	{
		assertion(extents.width > 0 && extents.height > 0 && extents.depth > 0);
		if (extents.height > 1 && extents.depth > 1) { imageBaseType = types::ImageBaseType::Image3D; }
		else if (extents.height > 1) { imageBaseType = types::ImageBaseType::Image2D; }
		else { imageBaseType = types::ImageBaseType::Image1D; }
		this->extents = extents;
	}


	/*!*******************************************************************************************
	\brief Set this texture layer
	\return layerSize Texture layer size
	**********************************************************************************************/
	void setLayers(types::ImageLayersSize layersSize)
	{
		this->layersSize = layersSize;
	}
	mutable const impl::Sampler_* m_sampler;// for ES2
=======
	/// <summary>Return the basic dimensioning of the texture (1D/2D/3D).</summary>
	/// <returns>The TextureDimension</returns>
	types::ImageBaseType getDimensions() const;

	/// <summary>Constructor.</summary>
	/// <param name="context">The GraphicsContext where this Texture will belong</param>
	TextureStoreGles_(const GraphicsContext& context) : HTexture_(0, 0), TextureStore_(context), _sampler(0) {}

	/// <summary>Constructor.</summary>
	/// <param name="context">The GraphicsContext where this Texture will belong</param>
	TextureStoreGles_() : HTexture_(0, 0), _sampler(0) {}

	/// <summary>Return a reference to the format of the texture</summary>
	/// <returns>The reference to the ImageStorageFormat</returns>
	ImageStorageFormat&  getFormat() { return _format; }

	/// <summary>Constructor. Use to wrap a preexisting, underlying texture object.</summary>
	/// <param name="context">The GraphicsContext where this Texture will belong</param>
	/// <param name="texture">An already existing texture object of the underlying API. The underlying object will be
	/// then owned by this TextureStore object, destroying it when done. If shared semantics are required, use the
	/// overload accepting a smart pointer object.</param>
	/// <remarks>NOTE: This object will take ownership of the passed texture object, destroying it in its destructor.
	/// </remarks>
	TextureStoreGles_(const GraphicsContext& context, const native::HTexture_& texture) : HTexture_(texture), TextureStore_(context), _sampler(0) {}

	/// <summary>Destructor. Will properly release all resources held by this object.</summary>
	~TextureStoreGles_();


	/// <summary>Return the API's texture handle object.</summary>
	/// <param name="red">The swizzling that will be applied on the texel's "red" channel before that is returned to
	/// the shader.</param>
	/// <param name="green">The swizzling that will be applied on the texel's "green" channel before that is returned
	/// to the shader.</param>
	/// <param name="blue">The swizzling that will be applied on the texel's "blue" channel before that is returned to
	/// the shader.</param>
	/// <param name="alpha">The swizzling that will be applied on the texel's "alpha" channel before that is returned
	/// to the shader.</param>
	void setSwizzle(types::Swizzle red = types::Swizzle::Identity, types::Swizzle green = types::Swizzle::Identity,
	                types::Swizzle blue = types::Swizzle::Identity, types::Swizzle alpha = types::Swizzle::Identity);

	/// <summary>Set the dimension of this texture</summary>
	/// <param name="extents">Texture dimension</param>
	void setDimensions(const types::Extent3D& extents)
	{
		assertion(extents.width > 0 && extents.height > 0 && extents.depth > 0);
		if (extents.height > 1 && extents.depth > 1) { _imageBaseType = types::ImageBaseType::Image3D; }
		else if (extents.height > 1) { _imageBaseType = types::ImageBaseType::Image2D; }
		else { _imageBaseType = types::ImageBaseType::Image1D; }
		this->_extents = extents;
	}


	/// <summary>Set this texture layer</summary>
	/// <returns>layerSize Texture layer size</returns>
	void setLayers(types::ImageLayersSize layersSize)
	{
		this->_layersSize = layersSize;
	}

	void bind(IGraphicsContext& context, uint16 bindIndex)const
	{
		platform::ContextGles& contextEs = native_cast(context);
		platform::ContextGles::RenderStatesTracker& renderStates = contextEs.getCurrentRenderStates();
		if (renderStates.texSamplerBindings[bindIndex].lastBoundTex == this)
		{
			return;
		}
		if (renderStates.lastBoundTexBindIndex != bindIndex)
		{
			gl::ActiveTexture(GL_TEXTURE0 + bindIndex);
		}
		gl::BindTexture(target, handle);
		debugLogApiError(strings::createFormatted("TextureStoreGles_::bind TARGET%x HANDLE%x",
		                 target, handle).c_str());
		contextEs.onBind(*this, bindIndex);
	}


	void bindImage(IGraphicsContext& context, uint16 imageUnit) const
	{
#if defined(GL_READ_WRITE)
        if(gl::BindImageTexture == NULL)
        {
            Log("glBindImageTexture not supported on this platform");
            return;
        }
		platform::ContextGles& contextEs = native_cast(context);
		platform::ContextGles::RenderStatesTracker& renderStates = contextEs.getCurrentRenderStates();
		if (renderStates.imageBindings[imageUnit] == this) { return; }
		uint32 imageFormat;
		nativeGles::ConvertToGles::getOpenGLStorageFormat(_format, imageFormat);
		gl::BindImageTexture(imageUnit, handle, 0, GL_FALSE, 0, GL_READ_WRITE, imageFormat);
		debugLogApiError(strings::createFormatted("TextureStoreGles_::bind TARGET%x HANDLE%x",
		                 target, handle).c_str());
		contextEs.onBindImage(*this, imageUnit);
#endif
	}
	mutable const impl::Sampler_* _sampler;// for ES2

private:
	bool isAllocated_() const { return handle != 0; }
	void allocate2D_(const ImageStorageFormat& format, uint32 width, uint32 height, types::ImageUsageFlags usage, types::ImageLayout newLayout)
	{
		allocate_(format, width, height, 1, 1, false, usage, false);
	}

	void allocate2DMS_(const ImageStorageFormat& format, uint32 width, uint32 height, types::ImageUsageFlags usage, types::ImageLayout newLayout)
	{
		allocate_(format, width, height, 1, 1, false, usage, false);
	}
	void allocate2DArrayMS_(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 arraySize, types::ImageUsageFlags usage, types::ImageLayout newLayout)
	{
		allocate_(format, width, height, 1, arraySize, false, usage, false);
	}

	void allocateTransient_(const ImageStorageFormat& format, uint32 width, uint32 height, types::ImageUsageFlags usage, types::ImageLayout newLayout)
	{
		allocate_(format, width, height, 1, 1, false, usage, true);
	}
	void allocateStorage_(const ImageStorageFormat& format, uint32 width, uint32 height);
	void allocate2DCube_(const ImageStorageFormat& format, uint32 width, uint32 height, types::ImageUsageFlags usage, types::ImageLayout initialLayout)
	{
		allocate_(format, width, height, 1, 1, true, usage, false);
	}
	void allocate2DArray_(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 arraySize, types::ImageUsageFlags usage, types::ImageLayout initialLayout)
	{
		allocate_(format, width, height, 1, arraySize, false, usage, false);
	}

	void allocate3D_(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 depth, types::ImageUsageFlags usage, types::ImageLayout initialLayout)
	{
		allocate_(format, width, height, depth, false, false, usage, false);
	}

	void allocate_(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 depth, uint32 arraySize, bool isCube,
	               types::ImageUsageFlags usage, bool transient);
	void update_(const void* data, const ImageDataFormat& format, const TextureArea& area);
>>>>>>> 1776432f... 4.3
};

typedef RefCountedResource<gles::TextureStoreGles_> TextureStoreGles;

class TextureViewGles_ : public native::HImageView_, public impl::TextureView_
{
public:
	TextureViewGles_(const TextureStoreGles& texture,
	                 const types::ImageSubresourceRange& range = types::ImageSubresourceRange(),
	                 const types::SwizzleChannels swizzleChannels = types::SwizzleChannels()) :
		impl::TextureView_(texture), _subResourceRange(range), _swizzleChannels(swizzleChannels)
	{
	}

	const types::ImageSubresourceRange& getSubResourceRange()const {  return _subResourceRange; }
	const types::SwizzleChannels getSwizzleChannel()const { return _swizzleChannels; }

<<<<<<< HEAD
	const types::ImageSubresourceRange& getSubResourceRange()const {	return m_subResourceRange;	}
	const types::SwizzleChannels getSwizzleChannel()const { return m_swizzleChannels; }
=======
>>>>>>> 1776432f... 4.3
private:
	types::ImageSubresourceRange _subResourceRange;
	types::SwizzleChannels _swizzleChannels;
};
typedef RefCountedResource<gles::TextureViewGles_> TextureViewGles;

}
}
}
<<<<<<< HEAD
//!\endcond
=======
PVR_DECLARE_NATIVE_CAST(TextureStore);
PVR_DECLARE_NATIVE_CAST(TextureView);
>>>>>>> 1776432f... 4.3
