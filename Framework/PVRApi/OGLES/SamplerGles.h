/*!*********************************************************************************************************************
\file         PVRApi\OGLES\SamplerGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         OpenGL ES 2+ implementation of the Sampler class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#pragma once
#include "PVRApi/ApiObjects/Sampler.h"
#include "PVRApi/OGLES/ContextGles.h"

namespace pvr {
namespace api {
namespace gles {
class SamplerGles_ : public impl::Sampler_, public native::HSampler_
{
public:
	SamplerCreateParam m_desc;//< required for ES2
	mutable bool m_initialized;
	SamplerGles_(GraphicsContext& device) : Sampler_(device), m_initialized(false) {}
	void bind(IGraphicsContext& context, uint32 index) const;
	bool init(const SamplerCreateParam& _desc);
	void destroy();
	~SamplerGles_()
	{
		if (m_context.isValid())
		{
			destroy();
		}
		else
		{
			reportDestroyedAfterContext("Sampler");
		}

	}
};
typedef RefCountedResource<gles::SamplerGles_> SamplerGles;
}
}
}
//!\endcond