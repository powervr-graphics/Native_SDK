/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\GraphicsPipeline.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         The Graphics pipeline is a cornerstone of PVRApi. It represents all state that is expected to be able to be
              "baked" ahead of time - Shaders, blending, depth/stencil tests, vertex assembly etc.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiObjects/PipelineLayout.h"
#include "PVRApi/ApiObjects/PipelineConfig.h"
#include <vector>

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
	pipelineCreation::DepthStencilStateCreateParam		depthStencil;	//!< Depth and stencil buffer creation info
	pipelineCreation::ColorBlendStateCreateParam		colorBlend;	//!< Color blending and attachments info
	pipelineCreation::ViewportStateCreateParam			viewport;		//!< Viewport creation info
	pipelineCreation::RasterStateCreateParam			rasterizer;	//!< Rasterizer configuration creation info
	pipelineCreation::VertexInputCreateParam			vertexInput;	//!< Vertex Input creation info
	pipelineCreation::InputAssemblerStateCreateParam	inputAssembler;//!< Input Assembler creation info
	pipelineCreation::VertexShaderStageCreateParam		vertexShader;	//!< Vertex shader information
	pipelineCreation::FragmentShaderStageCreateParam	fragmentShader;//!< Fragment shader information
	pipelineCreation::GeometryShaderStageCreateParam	geometryShader;
	pipelineCreation::TessControlShaderStageCreateParam tessControlShader; //<! Tesselation Control Shader information
	pipelineCreation::TessEvalShaderStageCreateParam	tessEvalShader; //<! Tesselation Evaluation Shader information
	pipelineCreation::MultiSampleStateCreateParam		multiSample;	//!< Multisampling information
	pipelineCreation::DynamicStatesCreateParam			dynamicStates;
	pvr::api::PipelineLayout							pipelineLayout;//!< The pipeline layout
	pvr::api::RenderPass								renderPass;	//!< The Renderpass
	uint32												subPass;		//!< The subpass index

	GraphicsPipelineCreateParam(): subPass(0) {}
};

namespace impl {
class GraphicsPipelineImplementationDetails;
/*!****************************************************************************************************************
\brief API graphics pipeline wrapper. A GraphicsPipeline represents the configuration of almost the entire RenderState,
including vertex description, primitive assembly, Shader configuration, rasterization, blending etc. Access through
the Framework managed object GraphicsPipeline.
******************************************************************************************************************/
class GraphicsPipeline_
{
	friend class PopPipeline;
	friend class CommandBufferBase_;
	friend class GraphicsPipelineImplementationDetails;
	template<typename> friend class PackagedBindable;
	template<typename> friend struct ::pvr::RefCountEntryIntrusive;
	friend class ::pvr::IGraphicsContext;
public:
	/*!****************************************************************************************************************
	\brief Return pipeline vertex input binding info.
	\return VertexInputBindingInfo
	******************************************************************************************************************/
	VertexInputBindingInfo const* getInputBindingInfo(pvr::uint16 bindingId)const;

	/*!****************************************************************************************************************
	\brief Return all the information on VertexAttributes of this pipeline.
	\return The information on VertexAttributes of this pipeline as a const pointer to VertexAttributeInfo.
	******************************************************************************************************************/
	VertexAttributeInfoWithBinding const* getAttributesInfo(pvr::uint16 bindId)const;

	/*!****************************************************************************************************************
	\brief Destructor. Destroys all resources held by this pipeline.
	******************************************************************************************************************/
	virtual ~GraphicsPipeline_();

	/*!****************************************************************************************************************
	\brief Destroy this pipeline. Releases all resources held by the pipeline.
	******************************************************************************************************************/
	void destroy();

	/*!****************************************************************************************************************
	\brief Get If uniforms are supported by the underlying API, get the shader locations of several uniform variables
			at once. If a uniform does not exist or is inactive, returns -1
	\param[in] uniforms An array of uniform variable names
	\param[in] numUniforms The number of uniforms in the array
	\param[out] outLocation An array where the locations will be saved. Writes -1 for inactive uniforms.
	******************************************************************************************************************/
	void getUniformLocation(const char8** uniforms, uint32 numUniforms, int32* outLocation)
	{
		for (uint32 i = 0; i < numUniforms; ++i) { outLocation[i] = getUniformLocation(uniforms[i]); }
	}

	/*!****************************************************************************************************************
	\brief Get If uniforms are supported by the underlying API, get the shader location of a uniform variables. If a
				uniform does not exist or is inactive, return -1
	\param uniform The name of a shader uniform variable name
	\return The location of the uniform, -1 if not found/inactive.
	******************************************************************************************************************/
	int32 getUniformLocation(const char8* uniform);

	/*!****************************************************************************************************************
	\brief Get Get the shader locations of several uniform variables at once. If an attribute does not exist or is
			inactive, returns -1
	\brief attribute attributes name
	\return The shader attribute index of an attribute, -1 if nonexistent.
	******************************************************************************************************************/
	int32 getAttributeLocation(const char8* attribute);

	/*!****************************************************************************************************************
	\brief Get multiple attribute locations at once. If an attribute is inactive or does not exist, the location is
			set to -1
	\param[in] attributes The array of attributes names to get locations
	\param[in] numAttributes of attributes in the array
	\param[out] outLocation An array of sufficient size to write the locations to
	******************************************************************************************************************/
	void getAttributeLocation(const char8** attributes, uint32 numAttributes, int32* outLocation)
	{
		for (uint32 i = 0; i < numAttributes; ++i) { outLocation[i] = getAttributeLocation(attributes[i]); }
	}

	/*!****************************************************************************************************************
	\brief Get number of attributes of buffer binding.
	\param bindingId buffer binding id
	\return number of attributes
	******************************************************************************************************************/
	pvr::uint8 getNumAttributes(pvr::uint16 bindingId)const;

	/*!****************************************************************************************************************
	\brief Return pipeline layout.
	\return const PipelineLayout&
	******************************************************************************************************************/
	const PipelineLayout& getPipelineLayout()const;

	const native::HPipeline_& getNativeObject() const;
	native::HPipeline_& getNativeObject();
protected:
	Result::Enum init(const GraphicsPipelineCreateParam& desc, ParentableGraphicsPipeline_* parent = NULL);

	Result::Enum createProgram();
	GraphicsPipeline_(GraphicsContext& device);
	std::auto_ptr<GraphicsPipelineImplementationDetails> pimpl;
	void bind(IGraphicsContext& context);
};

/*!****************************************************************************************************************
\brief API graphics pipeline wrapper. A GraphicsPipeline represents the configuration of almost the entire RenderState,
including vertex description, primitive assembly, Shader configuration, rasterization, blending etc. Access through
the Framework managed object GraphicsPipeline.
A ParentableGraphicsPipeline is a pipeline that is suitable to function as the "Parent" of another pipeline, helping
to create efficient Pipeline Hierarchies.
\description ParentableGraphicsPipelines can and should be used to make switching between different pipelines more
efficient. In effect, a ParentableGraphicsPipeline allows the user to create another (non-parentable pipeline) as
a "diff" of the state between the Parentable pipeline and itself, making the transition between them very efficient.
******************************************************************************************************************/
class ParentableGraphicsPipeline_ : public GraphicsPipeline_
{
	friend class GraphicsPipeline_;
	friend class ::pvr::IGraphicsContext;
	std::auto_ptr<GraphicsPipelineCreateParam> m_createParams;
public:
	/*!****************************************************************************************************************
	\brief  Construct this on device.
	\param device
	******************************************************************************************************************/
	ParentableGraphicsPipeline_(GraphicsContext& device) :
		GraphicsPipeline_(device) {}

	/*!****************************************************************************************************************
	\brief Initialize this with create param.
	\param desc
	******************************************************************************************************************/
	Result::Enum init(const GraphicsPipelineCreateParam& desc);

	/*!****************************************************************************************************************
	\brief	return pipeline create param used to create the child pipeline
	\return	const GraphicsPipelineCreateParam&
	*******************************************************************************************************************/
	const GraphicsPipelineCreateParam& getCreateParam()const { return *m_createParams.get(); }

	/*!****************************************************************************************************************
	\brief	return pipeline create param used to create the child pipeline
	\return	The createParam used to create this pipeline
	*******************************************************************************************************************/
	GraphicsPipelineCreateParam getCreateParam() { return *m_createParams.get(); }

};

}
inline native::HPipeline_& native_cast(pvr::api::impl::GraphicsPipeline_& object) { return object.getNativeObject(); }
inline const native::HPipeline_& native_cast(const pvr::api::impl::GraphicsPipeline_& object) { return object.getNativeObject(); }
}
}
