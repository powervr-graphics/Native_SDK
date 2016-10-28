/*!*********************************************************************************************************************te
\file         PVRApi\OGLES\GraphicsPipelineGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains the OpenGL ES 2/3 implementation of the all-important pvr::api::GraphicsPipeline object.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#pragma once
#include "PVRApi/ApiObjects/GraphicsPipeline.h"
#include "PVRApi/OGLES/StateContainerGles.h"
#include "PVRApi/OGLES/ContextGles.h"
namespace pvr {
namespace api {
namespace gles {

class GraphicsPipelineImplGles : public impl::GraphicsPipelineImplBase,  public native::HPipeline_
{
public:
	GraphicsPipelineImplGles(GraphicsContext context) :
		m_initialized(false), m_parent(NULL), m_owner(NULL), m_context(context) {}

	const GraphicsShaderProgramState& getShaderProgram()const;

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

	bool init(const GraphicsPipelineCreateParam& desc, impl::ParentableGraphicsPipeline_* parent, impl::GraphicsPipeline_* owner);

	const native::HPipeline_& getNativeObject() const;

	native::HPipeline_& getNativeObject();

	Result createProgram();

	bool m_initialized;

	bool createPipeline();

	void setAll();

	//Set all states that are different than the parent's
	void setFromParent();

	//Unset all states that are different than the parent's
	void unsetToParent();

	const GraphicsPipelineCreateParam& getCreateParam()const;

	void bind()
	{
		gles::GraphicsStateContainer& containerGles = static_cast<gles::GraphicsStateContainer&>(m_states);
		platform::ContextGles::RenderStatesTracker& currentStates =
		  static_cast<platform::ContextGles&>(*m_context).getCurrentRenderStates();

		currentStates.primitiveTopology = containerGles.primitiveTopology;
		setAll();
		static_cast<platform::ContextGles&>(*m_context).onBind(m_owner);
	}

	GraphicsStateContainer					m_states;
	impl::ParentableGraphicsPipeline_*		m_parent;
	impl::GraphicsPipeline_*				m_owner;
	GraphicsContext							m_context;
	GraphicsPipelineCreateParam				m_createParam;
};


class ParentableGraphicsPipelineImplGles : public GraphicsPipelineImplGles
{
public:
	ParentableGraphicsPipelineImplGles(GraphicsContext& context) : GraphicsPipelineImplGles(context) {}
	bool init(const GraphicsPipelineCreateParam& desc, impl::ParentableGraphicsPipeline_* owner);

};

}
}
}
//!\endcond