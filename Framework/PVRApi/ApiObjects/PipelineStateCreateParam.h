/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\PipelineStateCreateParam.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains Creation parameters for a Graphics Pipeline object.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiObjects/PipelineConfigStateCreateParam.h"
#include "PVRApi/ApiObjects/ComputeStateCreateParam.h"
#include <map>
namespace pvr {
namespace api {
/*!****************************************************************************************************************
\brief This represents al the information needed to create a GraphicsPipeline. All items must have proper values
       for a pipeline to be successfully created, but all those for which it is possible  (except, for example, 
	   Shaders and Vertex Formats) will have defaults same as their default values OpenGL ES graphics API.
*******************************************************************************************************************/
struct GraphicsPipelineCreateParam
{
public:
	pipelineCreation::DepthStencilStateCreateParam   depthStencil;	//!< Depth and stencil buffer creation info
	pipelineCreation::ColorBlendStateCreateParam     colorBlend;	//!< Color blending and attachments info
	pipelineCreation::ViewportStateCreateParam       viewport;		//!< Viewport creation info
	pipelineCreation::RasterStateCreateParam         rasterizer;	//!< Rasterizer configuration creation info
	pipelineCreation::VertexInputCreateParam	     vertexInput;	//!< Vertex Input creation info
	pipelineCreation::InputAssemblerStateCreateParam inputAssembler;//!< Input Assembler creation info
	pipelineCreation::VertexShaderStageCreateParam   vertexShader;	//!< Vertex shader information
	pipelineCreation::FragmentShaderStageCreateParam fragmentShader;//!< Fragment shader information
	pipelineCreation::MultiSampleStateCreateParam	 multiSample;	//!< Multisampling information
	pvr::api::PipelineLayout                         pipelineLayout;//!< The pipeline layout
	GraphicsPipelineCreateParam() {}
};

/*!****************************************************************************************************************
\brief Compute pipeline create parameters.
*******************************************************************************************************************/
struct ComputePipelineCreateParam
{
public:
	pipelineCreation::ComputeShaderStageCreateParam computeShader;	//!< Compute shader information
	PipelineLayout pipelineLayout;									//!< Compute pipeline information
};
}
}
