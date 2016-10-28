/*!*********************************************************************************************************************
\file         PVRApi\OGLES\PipelineConfigStateCreateParam.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Definitions of the OpenGL ES implementation of several Pipeline State object creation params (see GraphicsPipeline).
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/ApiObjects/PipelineConfig.h"
#include "PVRApi/OGLES/PipelineConfigStatesGles.h"
#include "PVRApi/OGLES/StateContainerGles.h"
#include "PVRApi/ApiObjects/DescriptorSet.h"
namespace pvr {
namespace api {
namespace pipelineCreation {

void createStateObjects(const DepthStencilStateCreateParam& thisobject,
                        gles::GraphicsStateContainer& storage,
                        const DepthStencilStateCreateParam* parent_param)
{
	gles::GraphicsStateContainer& storageGles = static_cast<gles::GraphicsStateContainer&>(storage);

	if (!parent_param || parent_param->isDepthTestEnable() != thisobject.isDepthTestEnable())
	{
		storageGles.addState(new gles::DepthTestState(thisobject.isDepthTestEnable()));
	}
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
	if ((!parent_param && thisobject.isDepthTestEnable()) || (parent_param && (parent_param->getDepthComapreOp() != thisobject.getDepthComapreOp() || !parent_param->isDepthTestEnable())
	    && thisobject.isDepthTestEnable())) { storageGles.addState(new gles::DepthFuncState(thisobject.getDepthComapreOp())); }

	const pipelineCreation::DepthStencilStateCreateParam::StencilState& parentStencilFront = parent_param->getStencilFront();
	const pipelineCreation::DepthStencilStateCreateParam::StencilState& parentStencilBack = parent_param->getStencilBack();
	const pipelineCreation::DepthStencilStateCreateParam::StencilState& thisStencilFront = thisobject.getStencilFront();
	const pipelineCreation::DepthStencilStateCreateParam::StencilState& thisStencilBack = thisobject.getStencilBack();

	if (!parent_param || parent_param->isDepthWriteEnable() != thisobject.isDepthWriteEnable()) { storageGles.addState(new gles::DepthWriteState(thisobject.isDepthWriteEnable())); }

	if (!parent_param || parent_param->isStencilTestEnable() != thisobject.isStencilTestEnable()) { storageGles.addState(new gles::StencilTestState(thisobject.isStencilTestEnable())); }

	if (!parent_param || (parentStencilFront.opStencilFail != thisStencilFront.opStencilFail ||
	                      parentStencilFront.opDepthFail != thisStencilFront.opDepthFail	 ||
	                      parentStencilFront.opDepthPass != thisStencilFront.opDepthPass))
	{
		storageGles.addState(new gles::StencilOpFrontState(thisStencilFront.opStencilFail, thisStencilFront.opDepthFail, thisStencilFront.opDepthPass));
	}

	if (!parent_param ||
	    (parentStencilBack.opStencilFail != thisStencilBack.opStencilFail ||
	     parentStencilBack.opDepthFail != thisStencilBack.opDepthFail ||
	     parentStencilBack.opDepthPass != thisStencilBack.opDepthPass))
	{
		storageGles.addState(new gles::StencilOpBackState(thisStencilBack.opStencilFail, thisStencilBack.opDepthFail, thisStencilBack.opDepthPass));
	}

	if (!parent_param || (parentStencilFront.compareOp != thisStencilFront.compareOp))
	{
		storageGles.addState(new gles::StencilCompareOpFront(thisStencilFront.compareOp));
	}

	if (!parent_param || (parentStencilBack.compareOp != thisStencilBack.compareOp))
	{
		storageGles.addState(new gles::StencilCompareOpBack(thisStencilFront.compareOp));
	}
}

void createStateObjects(const ColorBlendStateCreateParam& thisobject,
                        gles::GraphicsStateContainer& storage,
                        const ColorBlendStateCreateParam* parent_param)
{
	if (thisobject.getAttachmentStates().size())
	{
		if (!parent_param || parent_param->getAttachmentStates().size() == 0
		    || parent_param->getAttachmentStates()[0].blendEnable != thisobject.getAttachmentStates()[0].blendEnable)
		{
			storage.addState(new gles::BlendingEnableState(thisobject.getAttachmentStates()[0].blendEnable));
		}
		if (!parent_param || parent_param->getAttachmentStates().size() == 0 ||
		    parent_param->getAttachmentStates()[0].srcBlendColor != thisobject.getAttachmentStates()[0].srcBlendColor ||
		    parent_param->getAttachmentStates()[0].destBlendColor != thisobject.getAttachmentStates()[0].destBlendColor ||
		    parent_param->getAttachmentStates()[0].srcBlendAlpha != thisobject.getAttachmentStates()[0].srcBlendAlpha ||
		    parent_param->getAttachmentStates()[0].destBlendAlpha != thisobject.getAttachmentStates()[0].destBlendAlpha)
		{
			storage.addState(new gles::BlendFactorState(thisobject.getAttachmentStates()[0].srcBlendColor,
			                 thisobject.getAttachmentStates()[0].destBlendColor, thisobject.getAttachmentStates()[0].srcBlendAlpha,
			                 thisobject.getAttachmentStates()[0].destBlendAlpha));
		}
		if (!parent_param || parent_param->getAttachmentStates().size() == 0 ||
		    parent_param->getAttachmentStates()[0].channelWriteMask != thisobject.getAttachmentStates()[0].channelWriteMask)
		{
			storage.addState(new gles::ColorWriteMask(thisobject.getAttachmentStates()[0].channelWriteMask));
		}
	}

	if (thisobject.getAttachmentStates().size() > 1)
	{
		pvr::Log(pvr::Logger::Warning, "OpenGL doesn't support multiple color blend states.\n"
		         "using the first colorblend state");
	}
}

void createStateObjects(const ViewportStateCreateParam& thisobject,
                        gles::GraphicsStateContainer& storage,
                        const ViewportStateCreateParam* parent_param) {}

void createStateObjects(const RasterStateCreateParam& thisobject,
                        gles::GraphicsStateContainer& storage,
                        const RasterStateCreateParam* parent_param)
{
	if (!parent_param || parent_param->cullFace != thisobject.cullFace)
	{
		storage.addState(new gles::PolygonFrontFaceState(thisobject.cullFace));
	}
	if (!parent_param || parent_param->frontFaceWinding != thisobject.frontFaceWinding)
	{
		storage.addState(new gles::PolygonWindingOrderState(thisobject.frontFaceWinding));
	}
}

void createStateObjects(const VertexInputCreateParam& thisobject,
                        gles::GraphicsStateContainer& storage,
                        const VertexInputCreateParam* parent_param)
{
	if (thisobject.getInputBindings().size())
	{
		storage.vertexInputBindings = thisobject.getInputBindings();
	}
	else if (parent_param)
	{
		storage.vertexInputBindings = parent_param->getInputBindings();
	}

	if (thisobject.getAttributes().size())
	{
		storage.vertexAttributes = thisobject.getAttributes();
	}
	else if (parent_param)
	{
		storage.vertexAttributes = parent_param->getAttributes();
	}

}

void createStateObjects(const InputAssemblerStateCreateParam& thisobject,
                        gles::GraphicsStateContainer& storage,
                        const InputAssemblerStateCreateParam* parent_param)
{
	// - if the topology is explicitly set by the user
	// - else if the parent is null then use the default
	// - else use the parent topology
	if (thisobject.topology != types::PrimitiveTopology::None)
	{
		storage.primitiveTopology = thisobject.topology;
	}
	else if (parent_param == NULL)
	{
		storage.primitiveTopology = types::PrimitiveTopology::TriangleList;
		thisobject.topology = types::PrimitiveTopology::TriangleList;
	}
	else { thisobject.topology = storage.primitiveTopology = parent_param->topology; }
}

void createStateObjects(const VertexShaderStageCreateParam& thisobject,
                        gles::GraphicsStateContainer& storage,
                        const VertexShaderStageCreateParam* parent_param)
{
	storage.vertexShader = thisobject.getShader();
}

void createStateObjects(const FragmentShaderStageCreateParam& thisobject,
                        gles::GraphicsStateContainer& storage,
                        const FragmentShaderStageCreateParam* parent_param)
{
	storage.fragmentShader = thisobject.getShader();
}

void createStateObjects(const GeometryShaderStageCreateParam& thisobject,
                        gles::GraphicsStateContainer& storage,
                        const GeometryShaderStageCreateParam* parent_param)
{
	storage.geometryShader = thisobject.getShader();
}

void createStateObjects(const TesselationStageCreateParam& thisobject,
                        gles::GraphicsStateContainer& storage,
                        const TesselationStageCreateParam* parent_param)
{
	storage.tessControlShader = thisobject.getControlShader();
	storage.tessEvalShader = thisobject.getEvaluationShader();
	if (storage.tessControlShader.isValid() || storage.tessEvalShader.isValid())
	{
		storage.addState(new gles::TessPatchControlPoints(thisobject.getNumPatchControlPoints()));
	}
}

void createStateObjects(const ComputeShaderStageCreateParam& thisobject,
                        gles::ComputeStateContainer& storage)
{
	storage.computeShader = thisobject.getShader();
}

}
}
}
//!\endcond
