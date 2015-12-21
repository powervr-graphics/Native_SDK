/*!*********************************************************************************************************************
\file         PVRApi\OGLES\GraphicsStateContainerGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Supporting class for the Graphics Pipeline object. Do not use directly.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiObjects/GraphicsStateCreateParam.h"
#include "PVRApi/ApiObjects/Shader.h"
#include "PVRApi/ApiObjects/PipelineState.h"
#include "PVRCore/Types.h"
#include "PVRApi/ApiErrors.h"
#include <map>
#include <vector>
namespace pvr {
namespace api {
namespace impl {

/*!****************************************************************************************************************
\brief Supporting class for the Graphics Pipeline object. Do not use directly. Main responsibility: Contains
       objects representing specific sub-states of the OpenGL ES pipeline, and controls their setting/unsetting.
******************************************************************************************************************/
struct GraphicsStateContainer
{
public:
	typedef std::map<uint16, VertexInputBindingInfo>
	VertexInputBindingMap;// map buffer binding -> VertexAttributes
	typedef std::map<uint16, std::vector<VertexAttributeInfo> >	VertexAttributeMap;

	typedef std::vector<GraphicsPipelineImplState*> StateContainer;
	api::Shader vertexShader;
	api::Shader fragmentShader;
	api::Shader geometryShader;
	PipelineLayout pipelineLayout;
	StateContainer states;
	VertexInputBindingMap vertexInputBindings;
	VertexAttributeMap    vertexAttributes;
	PrimitiveTopology::Enum    primitiveTopology;

	void addState(GraphicsPipelineImplState* state) { states.push_back(state); }

	size_t numStates() { return states.size(); }
	size_t numInputBindings() { return vertexInputBindings.size(); }

	//Get the number of attributes for a buffer binding.
	pvr::uint8 getNumAttributes(pvr::uint16 bindingId)const
	{
		auto found = vertexAttributes.find(bindingId);
		return (found != vertexAttributes.end() ? (uint8)found->second.size() : (uint8)0);
	}

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

	VertexInputBindingInfo const* getInputBindingInfo(pvr::uint16 bindingId)const
	{
		auto found = vertexInputBindings.find(bindingId);
		if (found != vertexInputBindings.end()) { return &found->second; }
		return NULL;
	}

	VertexAttributeInfo const* getAttributesInfo(pvr::uint16 bindId)const
	{
		auto found = vertexAttributes.find(bindId);
		if (found != vertexAttributes.end())
		{
			return found->second.data();
		}
		return NULL;
	}
};
}
}
}