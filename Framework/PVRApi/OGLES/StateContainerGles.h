/*!*********************************************************************************************************************
\file         PVRApi\OGLES\StateContainerGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Supporting class for the Graphics Pipeline object. Do not use directly.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/OGLES/PipelineConfigStatesGles.h"
#include "PVRNativeApi/ApiErrors.h"
namespace pvr {
namespace api {
namespace gles {

/*!****************************************************************************************************************
\brief Supporting class for the Graphics Pipeline object. Do not use directly. Main responsibility: Contains
       objects representing specific sub-states of the OpenGL ES pipeline, and controls their setting/unsetting.
******************************************************************************************************************/
struct GraphicsStateContainer
{
public:


	typedef std::vector<impl::GraphicsPipelineImplState*> StateContainer;
	api::Shader vertexShader;
	api::Shader fragmentShader;
	api::Shader geometryShader;
	PipelineLayout pipelineLayout;
	StateContainer states;
	VertexInputBindingMap vertexInputBindings;
	VertexAttributeMap    vertexAttributes;
	types::PrimitiveTopology::Enum    primitiveTopology;

	void clear()
	{
		*this = GraphicsStateContainer();
	}

	void addState(impl::GraphicsPipelineImplState* state) { states.push_back(state); }

	size_t numStates() { return states.size(); }
	size_t numInputBindings() { return vertexInputBindings.size(); }


	bool hasVertexShader() { return vertexShader.isValid(); }
	bool hasFragmentShader() { return fragmentShader.isValid(); }

	//Unset all the states.
	void unsetAll(pvr::IGraphicsContext& device)
	{
		for (StateContainer::iterator it = states.begin(); it != states.end(); ++it)
		{
			(*it)->unset(device);
			debugLogApiError("GraphicsStateContainerGles::unset");
		}
	}

	// Set all the states.
	void setAll(pvr::IGraphicsContext& device)
	{
		for (StateContainer::iterator it = states.begin(); it != states.end(); ++it)
		{
			(*it)->set(device);
			debugLogApiError("GraphicsStateContainerGles::unset");
		}
	}

	const VertexInputBindingInfo* getInputBindingInfo(pvr::uint16 bindingId)const
	{
		auto found = std::find_if(vertexInputBindings.begin(), vertexInputBindings.end(), VertexBindingInfoPred_BindingEqual(bindingId));
		if (found != vertexInputBindings.end()) { return &*found; }
		return NULL;
	}
	const VertexAttributeInfoWithBinding* getAttributesInfo(pvr::uint16 bindingId) const
	{
		auto found = std::find_if(vertexAttributes.begin(), vertexAttributes.end(), VertexAttributeInfoPred_BindingEquals(bindingId));
		if (found != vertexAttributes.end())
		{
			return &*found;
		}
		return NULL;
	}

	//Get the number of attributes for a buffer binding.
	pvr::uint8 getNumAttributes(pvr::uint16 bindingId)const
	{
		uint8 retval = 0;
		auto found = std::find_if(vertexAttributes.begin(), vertexAttributes.end(), VertexAttributeInfoPred_BindingEquals(bindingId));
		while (found != vertexAttributes.end() && found->binding == bindingId)
		{
			++found;
			++retval;
		}
		return retval;
	}
};

struct ComputeStateContainer
{
public:
	typedef std::vector<impl::ComputePipelineImplState*>StateContainer;

	pvr::api::Shader computeShader;
	StateContainer states;
	PipelineLayout pipelineLayout;
	bool hasComputeShader()const { return computeShader.isValid(); }
};
}
}
}
