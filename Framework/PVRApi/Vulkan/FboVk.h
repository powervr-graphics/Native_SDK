/*!
\brief Vulkan implementation of the PVRApi Fbo (Frame Buffer Object).
\file PVRApi/Vulkan/FboVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRApi/ApiObjects/Fbo.h"
#include "PVRApi/Vulkan/TextureVk.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
namespace pvr {
namespace api {
namespace vulkan {
/// <summary>Vulkan implementation of the Fbo (Framebuffer object) class.</summary>
class FboVk_ : public impl::Fbo_, public native::HFbo_
{
public:
	/// <summary>ctor. Construct a new FBO on the provided context.</summary>
	FboVk_(const GraphicsContext& context);

    /// <summary>Destructor.</summary>
    ~FboVk_();

    /// <summary>Initialize this FBO with the provided create param</summary>
    /// <param name="desc">Fbo create param</param>
    /// <returns>Return true on success</returns>
	bool init(const FboCreateParam& desc);

	/// <summary>Destroy this object, release its resources.</summary>
	void destroy();

};

/// <summary>Vulkan Default FBO (FBO pointing to the Back Buffer). This object is necessary for rendering anything to
/// the screen. Should be used through the Fbo object. If a GLES direct manipulation is required, use through the
/// DefaultFboVulkan Reference counted Framework object.</summary>
class DefaultFboVk_ : public FboVk_
{
public:
	/// <summary>Constructor. Construct a new default FBO on the provided context.</summary>
	DefaultFboVk_(const GraphicsContext& context);

	//\brief INTERNAL Vulkan: Return true if this is a default fbo (overload).
	bool isDefault() const { return true; }
};

//Vulkan Default FBO (FBO pointing to the Back Buffer). This object is necessary for rendering anything to the screen. Reference counted.
typedef RefCountedResource<DefaultFboVk_> DefaultFboVk;
typedef RefCountedResource<FboVk_> FboVk;
}// namespace vulkan
}// namespace api
}// namespace pvr

PVR_DECLARE_NATIVE_CAST(Fbo);
