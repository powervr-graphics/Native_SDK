#pragma once
#include "PVRApi/ApiObjects/PipelineLayout.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
namespace pvr {
namespace api {
namespace gles {

class PipelineLayoutGles_ : public impl::PipelineLayout_, public native::HPipelineLayout_
{
public:
	PipelineLayoutGles_(GraphicsContext& device) : PipelineLayout_(device) {}
	bool init(const PipelineLayoutCreateParam& createParam)
	{
		m_desc = createParam;
		return true;
	}
};
typedef RefCountedResource<PipelineLayoutGles_> PipelineLayoutGles;
}
}
}