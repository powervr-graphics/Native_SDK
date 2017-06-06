/*!
\brief Vulkan implementation of the all important ComputePipeline class
\file PVRApi/Vulkan/ComputePipelineVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiObjects/ComputePipeline.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRApi/Vulkan/ContextVk.h"
#include "PVRApi/Vulkan/PopulateVulkanCreateInfo.h"
namespace pvr {
namespace api {
namespace vulkan {

class ComputePipelineImplVk : public impl::ComputePipelineImplBase, public native::HPipeline_
{
public:
	virtual ~ComputePipelineImplVk();
	ComputePipelineImplVk(GraphicsContext context);

	bool init(const ComputePipelineCreateParam& desc);

	void destroy();

	void getUniformLocation(const char8** uniforms, uint32 numUniforms, int32* outLocation)const;

	int32 getUniformLocation(const char8* uniform)const;

	const PipelineLayout& getPipelineLayout() const { return _createParam.pipelineLayout; }

	const ComputePipelineCreateParam& getCreateParam()const;
protected:
	api::ComputePipelineCreateParam _createParam;
	GraphicsContext _context;
	native::HPipelineCache_ _pipeCache;
};
}
inline const ::pvr::native::HPipeline_& native_cast(const ::pvr::api::ComputePipeline& object) { return static_cast<const vulkan::ComputePipelineImplVk&>(object->getImpl()); }
inline ::pvr::native::HPipeline_& native_cast(::pvr::api::ComputePipeline& object) { return static_cast<vulkan::ComputePipelineImplVk&>(object->getImpl()); }

}
}
