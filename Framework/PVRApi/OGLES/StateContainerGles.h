<<<<<<< HEAD
/*!*********************************************************************************************************************
\file         PVRApi\OGLES\StateContainerGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Supporting class for the Graphics Pipeline object. Do not use directly.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
=======
/*!
\brief Supporting class for the Graphics Pipeline object. Do not use directly.
\file PVRApi/OGLES/StateContainerGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
>>>>>>> 1776432f... 4.3
#pragma once
#include "PVRApi/OGLES/PipelineConfigStatesGles.h"
#include "PVRNativeApi/OGLES/ApiErrorsGles.h"
namespace pvr {
namespace api {
namespace gles {

/// <summary>Supporting class for the Graphics Pipeline object. Do not use directly. Main responsibility: Contains
/// objects representing specific sub-states of the OpenGL ES pipeline, and controls their setting/unsetting.
/// </summary>
struct GraphicsStateContainer
{
public:
	typedef std::vector<impl::GraphicsPipelineImplState*> StateContainer;
	typedef StateContainer::iterator StateContainerIter;
	api::Shader vertexShader;
	api::Shader fragmentShader;
	api::Shader geometryShader;
	api::Shader tessControlShader;
	api::Shader tessEvalShader;
	PipelineLayout pipelineLayout;
	StateContainer states;
	VertexInputBindingMap vertexInputBindings;
	VertexAttributeMap    vertexAttributes;
	types::PrimitiveTopology    primitiveTopology;

	void clear()
	{
		*this = GraphicsStateContainer();
	}

	void addState(impl::GraphicsPipelineImplState* state) { states.push_back(state); }

	size_t numStates() const { return states.size(); }
	size_t numInputBindings() { return vertexInputBindings.size(); }


	bool hasVertexShader()const { return vertexShader.isValid(); }
	bool hasFragmentShader()const { return fragmentShader.isValid(); }
<<<<<<< HEAD

    /*!
       \brief Unset all the states.
       \param device
     */
	void unsetAll(pvr::IGraphicsContext& device)
	{
		for (StateContainer::iterator it = states.begin(); it != states.end(); ++it)
		{
			(*it)->unset(device);
			debugLogApiError("GraphicsStateContainerGles::unset");
		}
	}

    /*!
       \brief Set all the states.
       \param device
     */
=======

    /// <summary>Set all the states.</summary>
    /// <param name="device"></param>
>>>>>>> 1776432f... 4.3
	void setAll(pvr::IGraphicsContext& device)const
	{
		for (StateContainer::const_iterator it = states.begin(); it != states.end(); ++it)
		{
			(*it)->set(device);
			debugLogApiError("GraphicsStateContainerGles::unset");
		}
	}

<<<<<<< HEAD
    /*!
       \brief Get vertex input bindingInfo
       \param bindingId Input binding id
       \return Return a pointer to VertexInputBindingInfo or NULL if not found.
     */
=======
    /// <summary>Get vertex input bindingInfo</summary>
    /// <param name="bindingId">Input binding id</param>
    /// <returns>Return a pointer to VertexInputBindingInfo or NULL if not found.</returns>
>>>>>>> 1776432f... 4.3
	const VertexInputBindingInfo* getInputBindingInfo(pvr::uint16 bindingId)const
	{
		auto found = std::find_if(vertexInputBindings.begin(), vertexInputBindings.end(), VertexBindingInfoPred_BindingEqual(bindingId));
		if (found != vertexInputBindings.end()) { return &*found; }
		return NULL;
	}

<<<<<<< HEAD
    /*!
       \brief Get vertex attribute bindingInfo
       \param bindingId binding id
       \return Return a pointer to VertexAttributeInfoWithBinding or NULL if not found.
     */
=======
    /// <summary>Get vertex attribute bindingInfo</summary>
    /// <param name="bindingId">binding id</param>
    /// <returns>Return a pointer to VertexAttributeInfoWithBinding or NULL if not found.</returns>
>>>>>>> 1776432f... 4.3
	const VertexAttributeInfoWithBinding* getAttributesInfo(pvr::uint16 bindingId) const
	{
		auto found = std::find_if(vertexAttributes.begin(), vertexAttributes.end(), VertexAttributeInfoPred_BindingEquals(bindingId));
		if (found != vertexAttributes.end())
		{
			return &*found;
		}
		return NULL;
	}

<<<<<<< HEAD
    /*!
       \brief Get the number of attributes for a buffer binding.
       \param bindingId
       \return
     */
=======
    /// <summary>Get the number of attributes for a buffer binding.</summary>
    /// <param name="bindingId"></param>
>>>>>>> 1776432f... 4.3
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
    typedef StateContainer::iterator StateContainerIter;
	pvr::api::Shader computeShader;
	StateContainer states;
	PipelineLayout pipelineLayout;
	bool hasComputeShader()const { return computeShader.isValid(); }
};
}
}
}
//!\endcond