/*!
\brief Vulkan implementation of the all important GraphicsPipeline class
\file PVRApi/Vulkan/GraphicsPipelineVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiObjects/GraphicsPipeline.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRApi/Vulkan/ContextVk.h"
#include "PVRApi/Vulkan/PopulateVulkanCreateInfo.h"
namespace pvr {
namespace api {
namespace vulkan {

class GraphicsPipelineImplVk : public api::impl::GraphicsPipelineImplBase, public native::HPipeline_
{
public:
	virtual ~GraphicsPipelineImplVk();
	GraphicsPipelineImplVk(GraphicsContext context);

	bool init(const GraphicsPipelineCreateParam& desc, const ParentableGraphicsPipeline& parent);

	VertexInputBindingInfo const* getInputBindingInfo(pvr::uint16 bindingId)const;

	VertexAttributeInfoWithBinding const* getAttributesInfo(pvr::uint16 bindId)const;

	void destroy();

	void getUniformLocation(const char8** uniforms, uint32 numUniforms, int32* outLocation)const;

	int32 getUniformLocation(const char8* uniform)const;

	int32 getAttributeLocation(const char8* attribute)const;

	void getAttributeLocation(const char8** attributes, uint32 numAttributes, int32* outLocation)const;

	pvr::uint8 getNumAttributes(pvr::uint16 bindingId)const;

	const PipelineLayout& getPipelineLayout()const;

	const GraphicsPipelineCreateParam& getCreateParam()const;
protected:
	api::GraphicsPipelineCreateParam _createParam;
	GraphicsContext _context;
	native::HPipelineCache_ _pipeCache;
	GraphicsPipelineImplVk* _parent;
};


class ParentableGraphicsPipelineImplVk : public GraphicsPipelineImplVk
{
public:
	ParentableGraphicsPipelineImplVk(GraphicsContext context);
	bool init(const GraphicsPipelineCreateParam& desc,const api::ParentableGraphicsPipeline& parent);
private:
};
}
inline const native::HPipeline_& native_cast(const GraphicsPipeline& object) { return static_cast<const vulkan::GraphicsPipelineImplVk&>(object->getImpl()); }

inline native::HPipeline_& native_cast(GraphicsPipeline& object) { return static_cast<vulkan::GraphicsPipelineImplVk&>(object->getImpl()); }

inline const native::HPipeline_& native_cast(const ParentableGraphicsPipeline& object){ return static_cast<const vulkan::GraphicsPipelineImplVk&>(object->getImpl()); }

inline native::HPipeline_& native_cast(ParentableGraphicsPipeline& object) { return static_cast<vulkan::GraphicsPipelineImplVk&>(object->getImpl()); }

}
}
