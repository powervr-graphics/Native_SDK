/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\GraphicsPipeline.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         The Graphics pipeline is a cornerstone of PVRApi. It represents all state that is expected to be able to be
              "baked" ahead of time - Shaders, blending, depth/stencil tests, vertex assembly etc.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/IGraphicsContext.h"
#include "PVRApi/ApiObjects/PipelineState.h"
#include "PVRApi/ApiObjects/PipelineStateCreateParam.h"
#include "PVRApi/ApiObjects/ShaderProgramState.h"
#include "PVRApi/ApiErrors.h"
#include <vector>

namespace pvr {
namespace api {

namespace impl {
//!\cond NO_DOXYGEN
class PushPipeline;
class PopPipeline;
class ResetPipeline;
template<typename> class PackagedBindable;
template<typename, typename> class PackagedBindableWithParam;
class ParentableGraphicsPipelineImpl;
//!\endcond

/*!****************************************************************************************************************
\brief API graphics pipeline wrapper. A GraphicsPipeline represents the configuration of almost the entire RenderState,
including vertex description, primitive assembly, Shader configuration, rasterization, blending etc. Access through
the Framework managed object GraphicsPipeline.
******************************************************************************************************************/
class GraphicsPipelineImpl
{
	template<typename any> friend class ::pvr::api::impl::PackagedBindable;
	friend class ::pvr::IGraphicsContext;
	friend class ::pvr::api::impl::PushPipeline;
	friend class ::pvr::api::impl::PopPipeline;
	friend class ::pvr::api::impl::ResetPipeline;
public:
	typedef int isBindable; //!< SFINAE type trait: Required for CommandBuffer submission

	/*!****************************************************************************************************************
	\brief Return pipeline shaderprogram.
	\return GraphicsShaderProgramState
	******************************************************************************************************************/
	GraphicsShaderProgramState getShaderProgram();

	/*!****************************************************************************************************************
	\brief Return pipeline vertex input binding info.
	\return VertexInputBindingInfo
	******************************************************************************************************************/
	VertexInputBindingInfo const* getInputBindingInfo(pvr::uint16 bindingId)const;

	/*!****************************************************************************************************************
	\brief Return all the information on VertexAttributes of this pipeline.
	\return The information on VertexAttributes of this pipeline as a const pointer to VertexAttributeInfo.
	******************************************************************************************************************/
	VertexAttributeInfo const* getAttributesInfo(pvr::uint16 bindId)const;


	/*!****************************************************************************************************************
	\brief Destructor. Destroys all resources held by this pipeline.
	******************************************************************************************************************/
	virtual ~GraphicsPipelineImpl() { destroy(); }

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

protected:
	GraphicsStateContainer* m_states;
	ParentableGraphicsPipelineImpl* m_parent;
	GraphicsContext m_context;
	Result::Enum init(GraphicsPipelineCreateParam& desc,
	ParentableGraphicsPipelineImpl* parent = NULL);

	Result::Enum createProgram();
	GraphicsPipelineImpl(GraphicsContext& device);
private:
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

	static PipelineRelation::Enum getRelation(GraphicsPipelineImpl* first, GraphicsPipelineImpl* second);

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

	bool m_initialised;
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
class ParentableGraphicsPipelineImpl : public GraphicsPipelineImpl
{
	friend class GraphicsPipelineImpl;
	friend class ::pvr::IGraphicsContext;
	std::auto_ptr<GraphicsPipelineCreateParam> m_createParams;
public:
	/*!****************************************************************************************************************
	\brief  Construct this on device.
	\param device
	******************************************************************************************************************/
	ParentableGraphicsPipelineImpl(GraphicsContext& device) :
		GraphicsPipelineImpl(device) {}

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
	\return	pvr::api::GraphicsPipelineCreateParam
	*******************************************************************************************************************/
	GraphicsPipelineCreateParam getCreateParam() { return *m_createParams.get(); }

	/*!****************************************************************************************************************
	\brief Get pipeline create info.
	\return GraphicsPipelineCreateParam*
	******************************************************************************************************************/
	GraphicsPipelineCreateParam* getDescriptors()
	{
		return m_createParams.get();
	}
};

inline GraphicsPipelineImpl::PipelineRelation::Enum GraphicsPipelineImpl::getRelation(
    GraphicsPipelineImpl* first, GraphicsPipelineImpl* second)
{
	if (first)
	{
		if (second)
		{
			return first == second ? PipelineRelation::Identity :
			       first->m_parent == second ? PipelineRelation::Child_Father :
			       first->m_parent == second->m_parent ? first->m_parent == NULL ? PipelineRelation::Unrelated : PipelineRelation::Siblings :
			       first == second->m_parent ? PipelineRelation::Father_Child :
			       PipelineRelation::Unrelated;
		}
		else { return PipelineRelation::NotNull_Null; }
	}
	else { return second ? PipelineRelation::Null_NotNull : PipelineRelation::Null_Null; }
}

inline void GraphicsPipelineImpl::setAll()
{
	debugLogApiError("GraphicsPipeline::setAll entry");
	if (m_parent)
	{
		m_parent->setAll();
	}
	setFromParent();
	debugLogApiError("GraphicsPipeline::setAll exit");
}

}
}
}
