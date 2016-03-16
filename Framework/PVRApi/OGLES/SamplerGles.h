#pragma once
#include "PVRApi/ApiObjects/Sampler.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
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
	void destroy_();
};
typedef RefCountedResource<gles::SamplerGles_> SamplerGles;
}
}
}
