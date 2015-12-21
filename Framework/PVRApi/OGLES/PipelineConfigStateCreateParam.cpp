/*!*********************************************************************************************************************
\file         PVRApi\OGLES\PipelineConfigStateCreateParam.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Definitions of the OpenGL ES implementation of several Pipeline State object creation params (see GraphicsPipeline).
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/ApiObjects/PipelineConfigStateCreateParam.h"
#include "PVRApi/ApiObjects/PipelineConfigStates.h"
#include "PVRApi/ApiObjects/ShaderProgramState.h"
#include "PVRApi/ApiObjects/ComputeStateCreateParam.h"
#include "PVRApi/OGLES/GraphicsStateContainerGles.h"
#include "PVRApi/ApiObjects/DescriptorTable.h"
namespace pvr {
namespace api {

namespace pipelineCreation {
void DepthStencilStateCreateParam::createStateObjects(impl::GraphicsStateContainer& storage,
        DepthStencilStateCreateParam* parent_param)const
{
	impl::GraphicsStateContainer& storageGles = static_cast<impl::GraphicsStateContainer&>(storage);

	if (!parent_param || parent_param->depthTest != depthTest) { storageGles.addState(new impl::DepthTestState(depthTest)); }
	/*
		-If the there is no parent and has depth test enabled then add the state. ELSE
		-If the child has depth test enabled AND
		          either the child has a different op than the parent OR
				  the parent has no depth test
				  then add the state

		If
		     1) This object has no depthtest
			 2) This object has the same function as its parent and the parent HAS depthest enabled
			 Don't add the state
	*/
	if ((!parent_param && depthTest) || (parent_param && (parent_param->depthCmpOp != depthCmpOp || !parent_param->depthTest)
	                                     && depthTest)) { storageGles.addState(new impl::DepthFuncState(depthCmpOp)); }
	if (!parent_param || parent_param->depthWrite != depthWrite) { storageGles.addState(new impl::DepthWriteState(depthWrite)); }
	if (!parent_param || parent_param->stencilTestEnable != stencilTestEnable) { storageGles.addState(new impl::StencilTestState(stencilTestEnable)); }
	if (!parent_param ||
	        (parent_param->opStencilFailFront != opStencilFailFront ||
	         parent_param->opDepthFailFront != opDepthFailFront ||
	         parent_param->opDepthPassFront != opDepthPassFront))
	{ storageGles.addState(new impl::StencilOpFrontState(opStencilFailFront, opDepthFailFront, opDepthPassFront)); }
	if (!parent_param ||
	        (parent_param->opStencilFailBack != opStencilFailBack ||
	         parent_param->opDepthFailBack != opDepthFailBack ||
	         parent_param->opDepthPassBack != opDepthPassBack))
	{ storageGles.addState(new impl::StencilOpBackState(opStencilFailBack, opDepthFailBack, opDepthPassBack)); }

	if (!parent_param || (parent_param->cmpOpStencilFront != cmpOpStencilFront))
	{
		storageGles.addState(new impl::StencilCompareOpFront(cmpOpStencilFront));
	}

	if (!parent_param || (parent_param->cmpOpStencilBack != cmpOpStencilBack))
	{
		storageGles.addState(new impl::StencilCompareOpBack(cmpOpStencilBack));
	}
}

void ColorBlendStateCreateParam::createStateObjects(impl::GraphicsStateContainer& storage,
        ColorBlendStateCreateParam* parent_param)const
{
	if (attachmentStates.size()){
		if (!parent_param || parent_param->attachmentStates.size() == 0
			|| parent_param->attachmentStates[0].blendEnable != attachmentStates[0].blendEnable)
		{
			storage.addState(new impl::BlendingEnableState(attachmentStates[0].blendEnable));
		}
		if (!parent_param || parent_param->attachmentStates.size() == 0 ||
			parent_param->attachmentStates[0].srcBlendColor != attachmentStates[0].srcBlendColor ||
			parent_param->attachmentStates[0].destBlendColor != attachmentStates[0].destBlendColor ||
			parent_param->attachmentStates[0].srcBlendAlpha != attachmentStates[0].srcBlendAlpha ||
			parent_param->attachmentStates[0].destBlendAlpha != attachmentStates[0].destBlendAlpha)
		{
			storage.addState(new impl::BlendFactorState(attachmentStates[0].srcBlendColor,
				attachmentStates[0].destBlendColor, attachmentStates[0].srcBlendAlpha,
				attachmentStates[0].destBlendAlpha));
			//storage.addState()
		}
		if (!parent_param || parent_param->attachmentStates.size() == 0 ||
			parent_param->attachmentStates[0].channelWriteMask != attachmentStates[0].channelWriteMask)
		{
			storage.addState(new impl::ColorWriteMask(attachmentStates[0].channelWriteMask));
			//storage.addState()
		}
	}
	
	if (attachmentStates.size() > 1)
	{
		pvr::Log(pvr::Logger::Warning, "OpenGL doesn't support multiple color blend states.\n"
		         "using the first colorblend state");
	}
}

void ViewportStateCreateParam::createStateObjects(impl::GraphicsStateContainer& storage,
        ViewportStateCreateParam* parent_param)const {}

void RasterStateCreateParam::createStateObjects(impl::GraphicsStateContainer& storage, RasterStateCreateParam* parent_param)const
{
	if (!parent_param || parent_param->cullFace != cullFace)
	{
		storage.addState(new impl::PolygonFrontFaceState(cullFace));
	}
	if (!parent_param || parent_param->cullMode != cullMode)
	{
		storage.addState(new impl::PolygonWindingOrderState(cullMode));
	}
}

void VertexInputCreateParam::createStateObjects(impl::GraphicsStateContainer& storage, VertexInputCreateParam* parent_param) const
{
//	PVR_ASSERT(inputBindings.size() || parent_param && "invalid vertex input state");
//	PVR_ASSERT(attributes.size() || parent_param && "invalid vertex input state");
	if (inputBindings.size())
	{
		storage.vertexInputBindings = inputBindings;
	}
	else if (parent_param)
	{
		storage.vertexInputBindings = parent_param->inputBindings;
	}

	if (attributes.size())
	{
		storage.vertexAttributes = attributes;
	}
	else if (parent_param)
	{
		storage.vertexAttributes = parent_param->attributes;
	}

}

void InputAssemblerStateCreateParam::createStateObjects(impl::GraphicsStateContainer& storage,
        InputAssemblerStateCreateParam* parent_param) const
{
	// - if the topology is explicitly set by the user
	// - else if the parent is null then use the default
	// - else use the parent topology
	if (topology != PrimitiveTopology::None)
	{
		storage.primitiveTopology = topology;
	}
	else if (parent_param == NULL)
	{
		storage.primitiveTopology = PrimitiveTopology::TriangleList;
		topology = PrimitiveTopology::TriangleList;
	}
	else { topology = storage.primitiveTopology = parent_param->topology; }
}

void VertexShaderStageCreateParam::createStateObjects(impl::GraphicsStateContainer& storage,
        VertexShaderStageCreateParam* parent_param) const
{
	storage.vertexShader = shader;
}

void FragmentShaderStageCreateParam::createStateObjects(impl::GraphicsStateContainer& storage,
        FragmentShaderStageCreateParam* parent_param) const
{
	storage.fragmentShader = shader;
}

void ComputeShaderStageCreateParam::createStateObjects(ComputeStateContainer& storage,
        ComputeShaderStageCreateParam* parent_param)const
{
//	impl::GraphicsStateContainerGles& storageGles = static_cast<impl::GraphicsStateContainerGles&>(storage);
//	storageGles.computeShader = m_shader;
	storage.computeShader = m_shader;
}

}
}
}
//!\endcond 