/*!
\brief Contains the OpenGL ES 2/3 implementation of the all-important pvr::api::GraphicsPipeline object.
\file PVRApi/OGLES/GraphicsPipelineGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiObjects/GraphicsPipeline.h"
#include "PVRApi/OGLES/StateContainerGles.h"
#include "PVRApi/OGLES/ContextGles.h"
namespace pvr {
namespace api {
namespace gles {

class GraphicsPipelineImplGles : public impl::GraphicsPipelineImplBase
{
public:
	GraphicsPipelineImplGles(GraphicsContext context) :
		_initialized(false), _parent(NULL), _owner(NULL), _context(context) {}

	const GraphicsShaderProgramState& getShaderProgram()const;
	GraphicsShaderProgramState& getShaderProgram();

	VertexInputBindingInfo const* getInputBindingInfo(pvr::uint16 bindingId)const;

	VertexAttributeInfoWithBinding const* getAttributesInfo(pvr::uint16 bindId)const;

	void getUniformLocation(const char8** uniforms, uint32 numUniforms, int32* outLocation)const;

	~GraphicsPipelineImplGles() { destroy(); }

	void destroy();

	int32 getUniformLocation(const char8* uniform)const;

	int32 getAttributeLocation(const char8* attribute)const;

	void getAttributeLocation(const char8** attributes, uint32 numAttributes, int32* outLocation)const;

	pvr::uint8 getNumAttributes(pvr::uint16 bindingId)const;

	const PipelineLayout& getPipelineLayout()const;

	bool init(const GraphicsPipelineCreateParam& desc, ParentableGraphicsPipeline& parent, GraphicsPipeline& owner);

	Result createProgram();

	bool _initialized;

	void setAll();

	//Set all states that are different than the parent's
	void setFromParent();

	const GraphicsPipelineCreateParam& getCreateParam()const;

	void bind()
	{
		gles::GraphicsStateContainer& containerGles = static_cast<gles::GraphicsStateContainer&>(_states);
		platform::ContextGles::RenderStatesTracker& currentStates = native_cast(*_context).getCurrentRenderStates();
		if (_context->getLastPipelineBindingPoint() == pvr::types::PipelineBindPoint::Graphics)
		{
			if (_context->getBoundGraphicsPipeline() != NULL && &_context->getBoundGraphicsPipeline()->getImpl() == this)
			{
				return;
			}
		}
		currentStates.primitiveTopology = containerGles.primitiveTopology;
		setAll();
		native_cast(*_context).onBind(_owner);
	}

	GraphicsStateContainer          _states;
	impl::ParentableGraphicsPipeline_*    _parent;
	impl::GraphicsPipeline_*        _owner;
	GraphicsContext             _context;
	GraphicsPipelineCreateParam       _createParam;

protected:
	bool initBase(const GraphicsPipelineCreateParam& desc, gles::GraphicsStateContainer& states, ParentableGraphicsPipeline& parent);
};


class ParentableGraphicsPipelineImplGles : public GraphicsPipelineImplGles
{
public:
	ParentableGraphicsPipelineImplGles(GraphicsContext context) : GraphicsPipelineImplGles(context) {}
	bool init(const GraphicsPipelineCreateParam& desc, ParentableGraphicsPipeline& owner);
};

}
inline const native::HPipeline_& native_cast(const impl::GraphicsPipeline_& object)
{
	return native_cast(static_cast<const gles::GraphicsPipelineImplGles&>(object.getImpl()).getShaderProgram());
}
inline native::HPipeline_& native_cast(impl::GraphicsPipeline_& object)
{
	return native_cast(static_cast<gles::GraphicsPipelineImplGles&>(object.getImpl()).getShaderProgram());
}
inline const native::HPipeline_& native_cast(const impl::ParentableGraphicsPipeline_& object)
{
	return native_cast(static_cast<const gles::GraphicsPipelineImplGles&>(object.getImpl()).getShaderProgram());
}
inline native::HPipeline_& native_cast(impl::ParentableGraphicsPipeline_& object)
{
	return native_cast(static_cast<gles::GraphicsPipelineImplGles&>(object.getImpl()).getShaderProgram());
}
inline const native::HPipeline_& native_cast(const GraphicsPipeline& object)
{
	return native_cast(*object);
}
inline native::HPipeline_& native_cast(GraphicsPipeline& object)
{
	return native_cast(*object);
}
inline const native::HPipeline_& native_cast(const ParentableGraphicsPipeline& object)
{
	return native_cast(*object);
}
inline native::HPipeline_& native_cast(ParentableGraphicsPipeline& object)
{
	return native_cast(*object);
}


}
}
