/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/SamplerVk.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains Vulkan specific implementation of the Sampler class. Use only if directly using Vulkan calls.
			  Provides the definitions allowing to move from the Framework object Sampler to the underlying Vulkan Sampler.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiObjects/Sampler.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
namespace pvr {
namespace api {
namespace vulkan {
	/*!*********************************************************************************************************************
	\brief SamplerVk_ implementation that wraps the vulkan sampler
	***********************************************************************************************************************/
class SamplerVk_ : public impl::Sampler_, public native::HSampler_
{
public:
	/*!*********************************************************************************************************************
	\brief ctor. Construct this object
	\param context The Context to be construct from
	***********************************************************************************************************************/
	SamplerVk_(GraphicsContext& context) : Sampler_(context) {}
	
	/*!*********************************************************************************************************************
	\brief Initialize this object
	\param desc Sampler create parameters
	\return Return true on success
	***********************************************************************************************************************/
	bool init(const api::SamplerCreateParam& desc);
};
typedef RefCountedResource<SamplerVk_> SamplerVk;
}
}
}
PVR_DECLARE_NATIVE_CAST(Sampler);
