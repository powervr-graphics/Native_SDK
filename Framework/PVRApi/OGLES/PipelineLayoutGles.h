/*!*********************************************************************************************************************
\file         PVRApi\OGLES\PipelineLayoutGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Definitions of the OpenGL ES implementation of the RenderPass.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
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
//!\endcond