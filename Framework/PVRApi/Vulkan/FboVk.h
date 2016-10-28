/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/FboVk.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Vulkan implementation of the PVRApi Fbo (Frame Buffer Object).
***********************************************************************************************************************/
//!\cond NO_DOXYGEN

#pragma once
#include "PVRApi/ApiObjects/Fbo.h"
#include "PVRApi/Vulkan/TextureVk.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
namespace pvr {
namespace api {
namespace vulkan {
/*!*********************************************************************************************************************
\brief Vulkan implementation of the Fbo (Framebuffer object) class.
***********************************************************************************************************************/
class FboVk_ : public impl::Fbo_, public native::HFbo_
{
public:
	/*!*********************************************************************************************************************
	\brief  ctor. Construct a new FBO on the provided context.
	***********************************************************************************************************************/
	FboVk_(GraphicsContext& context);

    /*!*
        \brief Destructor.
     */
    ~FboVk_();

    /*!*********************************************************************************************************************
	\brief Initialize this FBO with the provided create param
	\param desc Fbo create param
	\return Return true on success
	***********************************************************************************************************************/
	bool init(const FboCreateParam& desc);

	/*!*
	\brief Destroy this object, release its resources.
	 */
	void destroy();

};

/*!*********************************************************************************************************************
\brief Vulkan Default FBO (FBO pointing to the Back Buffer). This object is necessary for rendering anything to the screen.
       Should be used through the Fbo object. If a GLES direct manipulation is required, use through the DefaultFboVulkan Reference
	   counted Framework object.
***********************************************************************************************************************/
class DefaultFboVk_ : public FboVk_
{
public:
	/*!*********************************************************************************************************************
	\brief  Constructor. Construct a new default FBO on the provided context.
	***********************************************************************************************************************/
	DefaultFboVk_(GraphicsContext& context);

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
//!\endcond
