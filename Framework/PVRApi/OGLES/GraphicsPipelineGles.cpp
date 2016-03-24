/*!*********************************************************************************************************************
\file         PVRApi\OGLES\GraphicsPipelineGles.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the OpenGL ES 2/3 implementation of the all-important pvr::api::GraphicsPipeline object.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/OGLES/StateContainerGles.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
#include "PVRNativeApi/ShaderUtils.h"
#include "PVRApi/OGLES/ShaderGles.h"
#include "PVRApi/OGLES/ContextGles.h"

namespace pvr {
namespace api {
namespace pipelineCreation {
void createStateObjects(const DepthStencilStateCreateParam& thisobject, gles::GraphicsStateContainer& storage, DepthStencilStateCreateParam* parent_param);
void createStateObjects(const ColorBlendStateCreateParam& thisobject, gles::GraphicsStateContainer& storage, ColorBlendStateCreateParam* parent_param);
void createStateObjects(const ViewportStateCreateParam& thisobject, gles::GraphicsStateContainer& storage, ViewportStateCreateParam* parent_param);
void createStateObjects(const RasterStateCreateParam& thisobject, gles::GraphicsStateContainer& storage, RasterStateCreateParam* parent_param);
void createStateObjects(const VertexInputCreateParam& thisobject, gles::GraphicsStateContainer& storage, VertexInputCreateParam* parent_param);
void createStateObjects(const InputAssemblerStateCreateParam& thisobject, gles::GraphicsStateContainer& storage, InputAssemblerStateCreateParam* parent_param);
void createStateObjects(const VertexShaderStageCreateParam& thisobject, gles::GraphicsStateContainer& storage, VertexShaderStageCreateParam* parent_param);
void createStateObjects(const FragmentShaderStageCreateParam& thisobject, gles::GraphicsStateContainer& storage, FragmentShaderStageCreateParam* parent_param);
}
namespace impl {
//!\cond NO_DOXYGEN
class PushPipeline;
class PopPipeline;
class ResetPipeline;
template<typename> class PackagedBindable;
template<typename, typename> class PackagedBindableWithParam;
class ParentableGraphicsPipeline_;
//!\endcond

//////IMPLEMENTATION INFO/////
/*The desired class hierarchy was
---- OUTSIDE INTERFACE ----
* ParentableGraphicsPipeline(PGP)			: GraphicsPipeline(GP)
-- Inside implementation --
* ParentableGraphicsPipelineGles(PGPGles)	: GraphicsPipelineGles(GPGles)
* GraphicsPipelineGles(GPGles)				: GraphicsPipeline(GP)
---------------------------
This would cause a diamond inheritance, with PGPGles inheriting twice from GP, once through PGP and once through GPGles.
To avoid this issue while maintaining the outside interface, the pImpl idiom is being used instead of the inheritance
chains commonly used for all other PVRApi objects. The same idiom (for the same reasons) is found in the CommandBuffer.
*/////////////////////////////

namespace {
struct PipelineStatePointerLess
{
	inline bool operator()(const PipelineState* lhs, const PipelineState* rhs) const
	{
		return static_cast<int32>(lhs->getStateType()) < static_cast<int32>(rhs->getStateType());
	}
};
struct PipelineStatePointerGreater
{
	inline bool operator()(const PipelineState* lhs, const PipelineState* rhs) const
	{
		return static_cast<int32>(lhs->getStateType()) > static_cast<int32>(rhs->getStateType());
	}
};
}


class GraphicsPipelineImplementationDetails : public native::HPipeline_
{
public:
	struct PipelineRelation
	{
		enum Enum
		{
			Unrelated,
			Identity,
			Null_Null,
			Null_NotNull,
			NotNull_Null,
			Father_Child,
			Child_Father,
			Siblings
		};
	};

	static PipelineRelation::Enum getRelation(GraphicsPipelineImplementationDetails* first, GraphicsPipelineImplementationDetails* second);

	const gles::GraphicsShaderProgramState& getShaderProgram();
	VertexInputBindingInfo const* getInputBindingInfo(pvr::uint16 bindingId)const;
	VertexAttributeInfo const* getAttributesInfo(pvr::uint16 bindId)const;

	GraphicsPipelineImplementationDetails(GraphicsContext& context) : m_context(context), m_initialized(false) { }
	~GraphicsPipelineImplementationDetails() { destroy(); }

	void destroy();
	int32 getUniformLocation(const char8* uniform);
	int32 getAttributeLocation(const char8* attribute);
	pvr::uint8 getNumAttributes(pvr::uint16 bindingId);
	const PipelineLayout& getPipelineLayout()const;

	gles::GraphicsStateContainer m_states;
	ParentableGraphicsPipeline_* m_parent;
	GraphicsContext m_context;
	Result::Enum init(GraphicsPipelineCreateParam& desc, ParentableGraphicsPipeline_* parent = NULL);

	Result::Enum createProgram();

	bool m_initialized;
	bool createPipeline();
	void setAll();

	/*!*********************************************************************************************************************
	\brief	Bind this pipeline for rendering. Switching to/from from a pipeline in the same hierarchy (parent, sibling) is
	very efficient, while switching to/from a null pipeline will in general require a large number of state changes.
	\param	context		The GraphicsContext to bind the pipeline to. A pipeline itself is effectively stateless and can be
	bound to any number of contexts
	***********************************************************************************************************************/
	void bind(IGraphicsContext& context);

	//Set all states that are different than the parent's
	void setFromParent();

	//Unset all states that are different than the parent's
	void unsetToParent();

};

GraphicsPipeline_::~GraphicsPipeline_() { destroy(); }

inline GraphicsPipelineImplementationDetails::PipelineRelation::Enum GraphicsPipelineImplementationDetails::getRelation(GraphicsPipelineImplementationDetails* lhs,
        GraphicsPipelineImplementationDetails* rhs)
{
	if (lhs)
	{
		if (rhs)
		{
			GraphicsPipelineImplementationDetails* first = lhs;
			GraphicsPipelineImplementationDetails* firstFather = lhs->m_parent ? lhs->m_parent->pimpl.get() : NULL;
			GraphicsPipelineImplementationDetails* second = rhs;
			GraphicsPipelineImplementationDetails* secondFather = rhs->m_parent ? rhs->m_parent->pimpl.get() : NULL;
			return first == second ? PipelineRelation::Identity :
			       firstFather == second ? PipelineRelation::Child_Father :
			       firstFather == secondFather ? firstFather == NULL ? PipelineRelation::Unrelated : PipelineRelation::Siblings :
			       first == secondFather ? PipelineRelation::Father_Child :
			       PipelineRelation::Unrelated;
		}
		else { return PipelineRelation::NotNull_Null; }
	}
	else { return rhs ? PipelineRelation::Null_NotNull : PipelineRelation::Null_Null; }
}

inline void GraphicsPipelineImplementationDetails::setAll()
{
	debugLogApiError("GraphicsPipeline::setAll entry");
	if (m_parent)
	{
		m_parent->pimpl->setAll();
	}
	setFromParent();
	debugLogApiError("GraphicsPipeline::setAll exit");
}


inline void GraphicsPipelineImplementationDetails::setFromParent() { m_states.setAll(*m_context); }

inline void GraphicsPipelineImplementationDetails::unsetToParent() { m_states.unsetAll(*m_context); }

inline void GraphicsPipelineImplementationDetails::destroy()
{
	gles::GraphicsStateContainer&	containerGles = m_states;
	containerGles.vertexShader.reset();
	containerGles.fragmentShader.reset();
	containerGles.vertexInputBindings.clear();
	for (gles::GraphicsStateContainer::StateContainer::iterator it = containerGles.states.begin(); it != containerGles.states.end(); ++it)
	{
		delete *it;
	}
	containerGles.states.clear();
	m_states.clear();
	m_parent = 0;
}

static gles::GraphicsShaderProgramState dummy_state;

inline const gles::GraphicsShaderProgramState& GraphicsPipelineImplementationDetails::getShaderProgram()
{
	gles::GraphicsStateContainer& containerGles = m_states;
	if (!containerGles.numStates() || containerGles.states[0]->getStateType() !=
	        GraphicsStateType::ShaderProgram)
	{
		if (m_parent)
		{
			return m_parent->pimpl->getShaderProgram();
		}
		else
		{
			return dummy_state;
		}
	}
	return *(static_cast<gles::GraphicsShaderProgramState*>(containerGles.states[0]));
}

const native::HPipeline_& GraphicsPipeline_::getNativeObject() const { return pimpl->getShaderProgram().getNativeObject(); }

native::HPipeline_& GraphicsPipeline_::getNativeObject() { return pimpl->getShaderProgram().getNativeObject(); }

void GraphicsPipeline_::destroy() { return pimpl->destroy(); }

GraphicsPipeline_::GraphicsPipeline_(GraphicsContext& context)
{
	pimpl.reset(new GraphicsPipelineImplementationDetails(context));
}

void GraphicsPipeline_::bind(IGraphicsContext& context)
{
	gles::GraphicsStateContainer& containerGles = static_cast<gles::GraphicsStateContainer&>(pimpl->m_states);
	platform::ContextGles::RenderStatesTracker& currentStates = static_cast<pvr::platform::ContextGles&>
	        (context).getCurrentRenderStates();

	currentStates.primitiveTopology = containerGles.primitiveTopology;
	if (!static_cast<pvr::platform::ContextGles&>(context).isLastBoundPipelineGraphics())
	{
		pimpl->setAll();
	}
	else
	{
		switch (pimpl->getRelation(context.getBoundGraphicsPipeline_()->pimpl.get(), pimpl.get()))
		{
		/////////////// Trivial cases: ///////////////
		//No-op: null pipes
		case GraphicsPipelineImplementationDetails::PipelineRelation::Null_Null:
		//Binding the same pipe
		case GraphicsPipelineImplementationDetails::PipelineRelation::Identity:
			return;
		//Null pipeline is being bound, so need to unset the currently bound pipeline. No state to set since new is null
		case GraphicsPipelineImplementationDetails::PipelineRelation::NotNull_Null:
			assertion(0 , "This Should Not Have Happened - Is this a custom framework?"
			          "A Null pipeline should not be bound.");
			break;
		//Null pipeline was bound, not null new. This will happen for example on program start. Just set the state of new pipeline
		case GraphicsPipelineImplementationDetails::PipelineRelation::Null_NotNull:
			pimpl->setAll();
			break;

		/////////////// Non-trivial cases: ///////////////

		//WORST CASE SCENARIO: Pipelines are unrelated. Unset ALL the state of the new pipeline, set all the state of the new one.
		//AVOID THIS CASE! In order to avoid this kind of case, create pipelines as derivatives, or correlate() them after creation.
		case GraphicsPipelineImplementationDetails::PipelineRelation::Unrelated:
			pimpl->setAll();
			break;

		//"NORMAL" scenarios
		//Father was bound, binding child. Since parent was bound, only the child state needs to be set (diffed...)
		case GraphicsPipelineImplementationDetails::PipelineRelation::Father_Child:
			pimpl->setFromParent();
			break;
		//Child was bound, binding father. Need to revert the child state to the parent's.
		case GraphicsPipelineImplementationDetails::PipelineRelation::Child_Father:
			context.getBoundGraphicsPipeline_()->pimpl->unsetToParent();
			break;
		//Child was bound, binding father. Need to revert the child state to the parent's.
		case GraphicsPipelineImplementationDetails::PipelineRelation::Siblings:
			context.getBoundGraphicsPipeline_()->pimpl->unsetToParent();
			pimpl->setFromParent();
			break;
		}
	}
	static_cast<platform::ContextGles&>(context).onBind(this);
}

Result::Enum GraphicsPipeline_::init(const GraphicsPipelineCreateParam& desc, ParentableGraphicsPipeline_* parent)
{
	if (pimpl->m_initialized) { return Result::AlreadyInitialized; }
	pimpl->m_parent = parent;
	gles::GraphicsStateContainer& states = pimpl->m_states;
	states.pipelineLayout = desc.pipelineLayout;
	if (!states.pipelineLayout.isValid() && (parent && !parent->getPipelineLayout().isValid()))
	{
		pvr::Log(pvr::Logger::Error, "Invalid Pipeline Layout");
		return Result::NotInitialized;
	}
	if (desc.colorBlend.getAttachmentStates().size() == 0)
	{
		pvr::Log("Pipeline must have atleast one color attachment state");
		return Result::NotInitialized;
	}
	pipelineCreation::createStateObjects(desc.colorBlend, states, (parent ? &parent->m_createParams->colorBlend : NULL));
	pipelineCreation::createStateObjects(desc.depthStencil, states, (parent ? &parent->m_createParams->depthStencil : NULL));
	pipelineCreation::createStateObjects(desc.fragmentShader, states, (parent ? &parent->m_createParams->fragmentShader : NULL));
	pipelineCreation::createStateObjects(desc.vertexShader, states, (parent ? &parent->m_createParams->vertexShader : NULL));
	pipelineCreation::createStateObjects(desc.inputAssembler, states, (parent ? &parent->m_createParams->inputAssembler : NULL));
	pipelineCreation::createStateObjects(desc.rasterizer, states, (parent ? &parent->m_createParams->rasterizer : NULL));
	pipelineCreation::createStateObjects(desc.vertexInput, states, (parent ? &parent->m_createParams->vertexInput : NULL));
	pipelineCreation::createStateObjects(desc.viewport, states, (parent ? &parent->m_createParams->viewport : NULL));

	if (!states.hasVertexShader() || !states.hasFragmentShader())
	{
		if ((parent != NULL) && (!parent->pimpl->m_states.hasVertexShader() ||
		                         !parent->pimpl->m_states.hasFragmentShader()))
		{
			return Result::AlreadyInitialized;
		}
	}

	Result::Enum retval = Result::Success;
	if (states.hasVertexShader() && states.hasFragmentShader())
	{
		retval = createProgram();
	}
	else if (!parent)
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
	gles::GraphicsStateContainer& containerGles = states;
	std::sort(containerGles.states.begin(), containerGles.states.end(), PipelineStatePointerLess());

	PipelineStatePointerLess	compLess;
	PipelineStatePointerGreater compGreater;
	uint32 counterChild = 0, counterParent = 0;

	while (parent != NULL && containerGles.states.size() > counterChild
	        && parent->pimpl->m_states.states.size() > counterParent)
	{
		if (compLess(containerGles.states[counterChild],
		             parent->pimpl->m_states.states[counterParent]))
		{
			counterChild++;
		}
		else if (compGreater(containerGles.states[counterChild],
		                     parent->pimpl->m_states.states[counterParent]))
		{
			counterParent++;
		}
		else
		{
			containerGles.states[counterChild]->m_parent =
			    parent->pimpl->m_states.states[counterParent];
			counterChild++;
			counterParent++;
		}
	}
	return retval;
}

int32 GraphicsPipeline_::getAttributeLocation(const char8* attribute)
{
	GLint prog;
	gl::GetIntegerv(GL_CURRENT_PROGRAM, &prog);
	gl::UseProgram(getNativeObject());
	int32 ret = gl::GetAttribLocation(getNativeObject(), attribute);
	gl::UseProgram(prog);
	debugLogApiError(strings::createFormatted("ERROR GET ATTRIBUTE LOC [%s] [%d]", attribute, prog).c_str());
	return ret;
}

Result::Enum GraphicsPipeline_::createProgram()
{
	gles::GraphicsShaderProgramState* program = new gles::GraphicsShaderProgramState();
	gles::GraphicsStateContainer& containerGles = static_cast<gles::GraphicsStateContainer&>(pimpl->m_states);
	bool hasGeomShader = containerGles.geometryShader.isValid();

	std::vector<native::HShader_> shaders;
	shaders.push_back(containerGles.vertexShader->getNativeObject());
	shaders.push_back(containerGles.fragmentShader->getNativeObject());
	if (hasGeomShader)
	{
		shaders.push_back(containerGles.geometryShader->getNativeObject());
	}

	std::vector<const char*> attribNames;
	std::vector<uint16> attribIndex;

	// retrive the attribute names and index
	for (auto it = containerGles.vertexAttributes.begin(); it != containerGles.vertexAttributes.end(); ++it)
	{
		attribNames.push_back(it->attribName.c_str());
		attribIndex.push_back(it->index);
	}
	std::vector<int> vec;
	std::string errorStr;
	const char** attribs = (attribNames.size() ? &attribNames[0] : NULL);
	if (!pvr::utils::createShaderProgram(&shaders[0], (uint32)shaders.size(), attribs, attribIndex.data(),
	                                     (uint32)attribIndex.size(),
	                                     program->getNativeObject(), &errorStr, &pimpl->m_context->getApiCapabilities()))
	{
		Log(Log.Critical, "Linking failed. Shader infolog: %s", errorStr.c_str());
		return Result::InvalidData;
	}
	containerGles.states.push_back(program);
	return Result::Success;
}

int32 GraphicsPipeline_::getUniformLocation(const char8* uniform)
{
	GLuint prog = getNativeObject();
	GLint lastBoundProg;
	gl::GetIntegerv(GL_CURRENT_PROGRAM, &lastBoundProg);
	if (lastBoundProg != prog) {	gl::UseProgram(prog);	}
	int32 ret = gl::GetUniformLocation(getNativeObject(), uniform);
	if (ret == -1)
	{
		Log(Log.Debug, "GraphicsPipeline::getUniformLocation [%s] for program [%d]  returned -1: Uniform was not active", uniform, prog);
	}
	if (lastBoundProg != prog)	{	gl::UseProgram(lastBoundProg);	}
	debugLogApiError(strings::createFormatted("GraphicsPipeline_::getUniformLocation [%s] for program [%d] ", uniform, prog).c_str());
	return ret;
}

pvr::uint8 GraphicsPipeline_::getNumAttributes(pvr::uint16 bindingId)const
{
	return pimpl->m_states.getNumAttributes(bindingId);
}

const VertexInputBindingInfo* GraphicsPipeline_::getInputBindingInfo(pvr::uint16 bindingId)const
{
	return pimpl->m_states.getInputBindingInfo(bindingId);
}

const VertexAttributeInfoWithBinding* GraphicsPipeline_::getAttributesInfo(pvr::uint16 bindId)const
{
	return pimpl->m_states.getAttributesInfo(bindId);
}

const pvr::api::PipelineLayout& GraphicsPipeline_::getPipelineLayout() const
{
	// return the pipeline layout / else return the valid parent's pipeline layout else NULL object
	if (pimpl->m_states.pipelineLayout.isNull() && pimpl->m_parent)
	{
		return pimpl->m_parent->getPipelineLayout();
	}
	assertion(!pimpl->m_states.pipelineLayout.isNull() , "invalid pipeline layout");
	return pimpl->m_states.pipelineLayout;
}

Result::Enum ParentableGraphicsPipeline_::init(const GraphicsPipelineCreateParam& desc)
{
	m_createParams.reset(new GraphicsPipelineCreateParam(desc));
	return GraphicsPipeline_::init(*m_createParams.get());
}

}
}
}
//!\endcond
