/*!*********************************************************************************************************************
\file         PVRApi\OGLES\GraphicsPipeline.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the OpenGL ES 2/3 implementation of the all-important pvr::api::GraphicsPipeline object.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/ApiObjects/GraphicsPipeline.h"
#include "PVRApi/OGLES/NativeObjectsGles.h"
#include "PVRApi/OGLES/OpenGLESBindings.h"
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRApi/OGLES/ShaderGles.h"
#include "PVRApi/OGLES/GraphicsStateContainerGles.h"
#include "PVRApi/ShaderUtils.h"

namespace pvr {
namespace api {
namespace impl {

GraphicsPipelineImpl::GraphicsPipelineImpl(GraphicsContext& context) :
	m_context(context), m_initialised(false)
{
	m_states = new GraphicsStateContainer;
}

void GraphicsPipelineImpl::bind(IGraphicsContext& context)
{
	GraphicsStateContainer& containerGles = static_cast<GraphicsStateContainer&>(*m_states);
	platform::ContextGles::RenderStatesTracker& currentStates = static_cast<pvr::platform::ContextGles&>
	    (context).getCurrentRenderStates();

	currentStates.primitiveTopology = containerGles.primitiveTopology;
	if (!static_cast<pvr::platform::ContextGles&>(context).isLastBoundPipelineGraphics())
	{
		this->setAll();
	}
	else
	{
		switch (GraphicsPipelineImpl::getRelation(context.getBoundGraphicsPipelineImpl(), this))
		{
		/////////////// Trivial cases: ///////////////
		//No-op: null pipes
		case GraphicsPipelineImpl::PipelineRelation::Null_Null:
		//Binding the same pipe
		case GraphicsPipelineImpl::PipelineRelation::Identity:
			return;
		//Null pipeline is being bound, so need to unset the currently bound pipeline. No state to set since new is null
		case GraphicsPipelineImpl::PipelineRelation::NotNull_Null:
			PVR_ASSERT(0 && "This Should Not Have Happened - Is this a custom framework?"
			           "A Null pipeline should not be bound.");
			break;
		//Null pipeline was bound, not null new. This will happen for example on program start. Just set the state of new pipeline
		case GraphicsPipelineImpl::PipelineRelation::Null_NotNull:
			this->setAll();
			break;

		/////////////// Non-trivial cases: ///////////////

		//WORST CASE SCENARIO: Pipelines are unrelated. Unset ALL the state of the new pipeline, set all the state of the new one.
		//AVOID THIS CASE! In order to avoid this kind of case, create pipelines as derivatives, or correlate() them after creation.
		case GraphicsPipelineImpl::PipelineRelation::Unrelated:
			this->setAll();
			break;

		//"NORMAL" scenarios
		//Father was bound, binding child. Since parent was bound, only the child state needs to be set (diffed...)
		case GraphicsPipelineImpl::PipelineRelation::Father_Child:
			this->setFromParent();
			break;
		//Child was bound, binding father. Need to revert the child state to the parent's.
		case GraphicsPipelineImpl::PipelineRelation::Child_Father:
			context.getBoundGraphicsPipelineImpl()->unsetToParent();
			break;
		//Child was bound, binding father. Need to revert the child state to the parent's.
		case GraphicsPipelineImpl::PipelineRelation::Siblings:
			context.getBoundGraphicsPipelineImpl()->unsetToParent();
			this->setFromParent();
			break;
		}
	}
	static_cast<pvr::platform::ContextGles&>(context).onBind(this);
}

Result::Enum GraphicsPipelineImpl::init(GraphicsPipelineCreateParam& desc,
                                        ParentableGraphicsPipelineImpl* parent)
{
	if (m_initialised) { return Result::AlreadyInitialised; }
	m_parent = parent;
	m_states->pipelineLayout = desc.pipelineLayout;
	if (!m_states->pipelineLayout.isValid() && (parent && !parent->getPipelineLayout().isValid()))
	{
		pvr::Log(pvr::Logger::Error, "Invalid Pipeline Layout");
		return Result::NotInitialised;
	}
	if (!parent && !desc.colorBlend.attachmentStates.size())
	{
		desc.colorBlend.addAttachmentState(0, pipelineCreation::ColorBlendAttachmentState());
	}
	desc.colorBlend.createStateObjects(*m_states, (m_parent ? &m_parent->m_createParams->colorBlend : NULL));
	desc.depthStencil.createStateObjects(*m_states, (m_parent ? &m_parent->m_createParams->depthStencil : NULL));
	desc.fragmentShader.createStateObjects(*m_states, (m_parent ? &m_parent->m_createParams->fragmentShader : NULL));
	desc.vertexShader.createStateObjects(*m_states, (m_parent ? &m_parent->m_createParams->vertexShader : NULL));
	desc.inputAssembler.createStateObjects(*m_states, (m_parent ? &m_parent->m_createParams->inputAssembler : NULL));
	desc.rasterizer.createStateObjects(*m_states, (m_parent ? &m_parent->m_createParams->rasterizer : NULL));
	desc.vertexInput.createStateObjects(*m_states, (m_parent ? &m_parent->m_createParams->vertexInput : NULL));
	desc.viewport.createStateObjects(*m_states, (m_parent ? &m_parent->m_createParams->viewport : NULL));

	if (!m_states->hasVertexShader() || !m_states->hasFragmentShader())
	{
		if ((m_parent != NULL) && (!m_parent->m_states->hasVertexShader() ||
		                           !m_parent->m_states->hasFragmentShader()))
		{
			return Result::AlreadyInitialised;
		}
	}

	Result::Enum retval = Result::Success;
	if (m_states->hasVertexShader() && m_states->hasFragmentShader())
	{
		retval = createProgram();
	}
	else if (!this->m_parent)
	{
		pvr::Log(Log.Debug, "GraphicsPipeline:: Shaders were invalid");
		retval = Result::InvalidData;
	}
	if (retval != pvr::Result::Success)
	{
		pvr::Log(Log.Debug, "GraphicsPipeline:: Program creation unsuccessful.");
		return retval;
	}
	//Invariant: no duplicates created.
	GraphicsStateContainer& containerGles = *m_states;
	std::sort(containerGles.states.begin(), containerGles.states.end(), PipelineStatePointerLess());

	PipelineStatePointerLess	compLess;
	PipelineStatePointerGreater compGreater;
	uint32 counterChild = 0, counterParent = 0;

	while (m_parent != NULL && containerGles.states.size() > counterChild
	       && m_parent->m_states->states.size()
	       > counterParent)
	{
		if (compLess(containerGles.states[counterChild],
		             m_parent->m_states->states[counterParent]))
		{
			counterChild++;
		}
		else if (compGreater(containerGles.states[counterChild],
		                     m_parent->m_states->states[counterParent]))
		{
			counterParent++;
		}
		else
		{
			containerGles.states[counterChild]->m_parent =
			  m_parent->m_states->states[counterParent];
			counterChild++;
			counterParent++;
		}
	}
	return retval;
}

GraphicsShaderProgramState GraphicsPipelineImpl::getShaderProgram()
{
	GraphicsStateContainer& containerGles = static_cast<GraphicsStateContainer&>(*m_states);
	if (!containerGles.numStates() || containerGles.states[0]->getStateType() !=
	    GraphicsStateType::ShaderProgram)
	{
		if (m_parent)
		{
			return m_parent->getShaderProgram();
		}
		else
		{
			return GraphicsShaderProgramState();
		}
	}
	return *(static_cast<GraphicsShaderProgramState*>(containerGles.states[0]));
}

int32 GraphicsPipelineImpl::getAttributeLocation(const char8* attribute)
{
	GLint prog;
	gl::GetIntegerv(GL_CURRENT_PROGRAM, &prog);
	gl::UseProgram(getShaderProgram().getNativeHandle()->handle);
	int32 ret = gl::GetAttribLocation(getShaderProgram().getNativeHandle()->handle, attribute);
	gl::UseProgram(prog);
	return ret;
}

Result::Enum GraphicsPipelineImpl::createProgram()
{
	GraphicsShaderProgramState* program = new GraphicsShaderProgramState();
	GraphicsStateContainer& containerGles = static_cast<GraphicsStateContainer&>(*m_states);
	bool hasGeomShader = containerGles.geometryShader.isValid();

	std::vector<native::HShader_> shaders;
	shaders.push_back(native::useNativeHandle(containerGles.vertexShader));
	shaders.push_back(native::useNativeHandle(containerGles.fragmentShader));
	if (hasGeomShader)
	{
		shaders.push_back(native::useNativeHandle(containerGles.geometryShader));
	}

	std::vector<const char*> attribNames;
	std::vector<uint16> attribIndex;

	// retrive the attribute names and index
	for (GraphicsStateContainer::VertexAttributeMap::iterator it = containerGles.vertexAttributes.begin();
	     it != containerGles.vertexAttributes.end(); ++it)
	{
		for (pvr::uint32 i = 0; i < it->second.size(); ++i)
		{
			if (it->second[i].attribName.size())
			{
				attribNames.push_back(it->second[i].attribName.c_str());
				attribIndex.push_back(it->second[i].index);
			}
		}
	}

	std::string errorStr;
	const char** attribs = (attribNames.size() ? &attribNames[0] : NULL);
	if (!pvr::utils::createShaderProgram(&shaders[0], (uint32)shaders.size(), attribs, attribIndex.data(),
	                                     (uint32)attribIndex.size(),
	                                     program->getNativeHandle(), &errorStr, &m_context->getApiCapabilities()))
	{
		Log(Log.Critical, "Linking failed. Shader infolog: %s", errorStr.c_str());
		return Result::InvalidData;
	}
	containerGles.states.push_back(program);
	return Result::Success;
}

int32 GraphicsPipelineImpl::getUniformLocation(const char8* uniform)
{
	GLint prog;
	gl::GetIntegerv(GL_CURRENT_PROGRAM, &prog);
	gl::UseProgram(getShaderProgram().getNativeHandle()->handle);
	int32 ret = gl::GetUniformLocation(getShaderProgram().getNativeHandle()->handle, uniform);
	if (ret == -1)
	{
		Log(Log.Debug, "GraphicsPipeline::getUniformLocation for uniform [%s] returned -1: Uniform was not active", uniform);
	}
	gl::UseProgram(prog);
	return ret;
}

pvr::uint8 GraphicsPipelineImpl::getNumAttributes(pvr::uint16 bindingId)const
{
	return m_states->getNumAttributes(bindingId);
}

VertexInputBindingInfo const* GraphicsPipelineImpl::getInputBindingInfo(pvr::uint16 bindingId)const
{
	return m_states->getInputBindingInfo(bindingId);
}

VertexAttributeInfo const* GraphicsPipelineImpl::getAttributesInfo(pvr::uint16 bindId)const
{
	return m_states->getAttributesInfo(bindId);
}

void GraphicsPipelineImpl::setFromParent() { m_states->setAll(*m_context); }

void GraphicsPipelineImpl::unsetToParent() { m_states->unsetAll(*m_context); }

void GraphicsPipelineImpl::destroy()
{
	GraphicsStateContainer&	containerGles = *m_states;
	containerGles.vertexShader.release();
	containerGles.fragmentShader.release();
	containerGles.vertexInputBindings.clear();
	for (GraphicsStateContainer::StateContainer::iterator it = containerGles.states.begin(); it != containerGles.states.end(); ++it)
	{
		delete *it;
	}
	containerGles.states.clear();
	delete m_states;
	m_states = NULL;
	m_parent = 0;
}

const pvr::api::PipelineLayout& GraphicsPipelineImpl::getPipelineLayout() const
{
	// return the pipeline layout / else return the valid parent's pipeline layout else NULL object
	if (m_states->pipelineLayout.isNull() && m_parent)
	{
		return m_parent->getPipelineLayout();
	}
	PVR_ASSERT(!m_states->pipelineLayout.isNull() && "invalid pipeline layout");
	return m_states->pipelineLayout;
}

Result::Enum ParentableGraphicsPipelineImpl::init(const GraphicsPipelineCreateParam& desc)
{
	m_createParams.reset(new GraphicsPipelineCreateParam(desc));
	return GraphicsPipelineImpl::init(*m_createParams.get());
}

}
}
}
//!\endcond
