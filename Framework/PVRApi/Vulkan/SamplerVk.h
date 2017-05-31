<<<<<<< HEAD
/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/SamplerVk.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains Vulkan specific implementation of the Sampler class. Use only if directly using Vulkan calls.
			  Provides the definitions allowing to move from the Framework object Sampler to the underlying Vulkan Sampler.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
=======
/*!
\brief Contains Vulkan specific implementation of the Sampler class. Use only if directly using Vulkan calls. Provides
the definitions allowing to move from the Framework object Sampler to the underlying Vulkan Sampler.
\file PVRApi/Vulkan/SamplerVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
>>>>>>> 1776432f... 4.3

#pragma once
#include "PVRApi/ApiObjects/Sampler.h"
#include "PVRApi/Vulkan/ContextVk.h"

namespace pvr {
namespace api {
namespace vulkan {
/// <summary>SamplerVk_ implementation that wraps the vulkan sampler</summary>
class SamplerVk_ : public impl::Sampler_, public native::HSampler_
{
public:
	/// <summary>ctor. Construct this object</summary>
	/// <param name="context">The Context to be construct from</param>
	SamplerVk_(const GraphicsContext& context) : Sampler_(context) {}

	/// <summary>Initialize this object</summary>
	/// <param name="desc">Sampler create parameters</param>
	/// <returns>Return true on success</returns>
	bool init(const api::SamplerCreateParam& desc);

	/// <summary>Releases all resources held by this object</summary>
	void destroy();

<<<<<<< HEAD
	/*!
		\brief destructor
	*/
	~SamplerVk_()
	{
#ifdef DEBUG
		if (this->m_context.isValid())
=======
	/// <summary>destructor</summary>
	~SamplerVk_()
	{
#ifdef DEBUG
		if (this->_context.isValid())
>>>>>>> 1776432f... 4.3
		{
			destroy();
		}
		else
		{
			reportDestroyedAfterContext("Sampler");
		}
#else
		destroy();
#endif
	}
};
typedef RefCountedResource<SamplerVk_> SamplerVk;
}
}
}
PVR_DECLARE_NATIVE_CAST(Sampler);

<<<<<<< HEAD
//!\endcond
=======
>>>>>>> 1776432f... 4.3
