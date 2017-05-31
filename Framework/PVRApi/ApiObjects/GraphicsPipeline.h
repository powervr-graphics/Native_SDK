/*!
\brief The Graphics pipeline is a cornerstone of PVRApi. It represents all state that is expected to be able to be
"baked" ahead of time - Shaders, blending, depth/stencil tests, vertex assembly etc.
\file PVRApi/ApiObjects/GraphicsPipeline.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiObjects/PipelineLayout.h"
#include "PVRApi/ApiObjects/PipelineConfig.h"
#include <vector>

namespace pvr {
namespace api {
<<<<<<< HEAD
/*!****************************************************************************************************************
\brief This represents al the information needed to create a GraphicsPipeline. All items must have proper values
       for a pipeline to be successfully created, but all those for which it is possible  (except, for example,
       Shaders and Vertex Formats) will have defaults same as their default values OpenGL ES graphics API.
*******************************************************************************************************************/
struct GraphicsPipelineCreateParam
{
public:
    pipelineCreation::DepthStencilStateCreateParam		depthStencil;		//!< Depth and stencil buffer creation info
    pipelineCreation::ColorBlendStateCreateParam		colorBlend;			//!< Color blending and attachments info
    pipelineCreation::ViewportStateCreateParam			viewport;			//!< Viewport creation info
    pipelineCreation::RasterStateCreateParam			rasterizer;			//!< Rasterizer configuration creation info
    pipelineCreation::VertexInputCreateParam			vertexInput;		//!< Vertex Input creation info
    pipelineCreation::InputAssemblerStateCreateParam	inputAssembler;		//!< Input Assembler creation info
    pipelineCreation::VertexShaderStageCreateParam		vertexShader;		//!< Vertex shader information
    pipelineCreation::FragmentShaderStageCreateParam	fragmentShader;		//!< Fragment shader information
    pipelineCreation::GeometryShaderStageCreateParam	geometryShader;     //!< Geometry shader information
    pipelineCreation::TesselationStageCreateParam		tesselationStates;	//!< Tesselation Control and evaluation shader information
    pipelineCreation::MultiSampleStateCreateParam		multiSample;		//!< Multisampling information
    pipelineCreation::DynamicStatesCreateParam			dynamicStates;		//!< Dynamic state Information
	pipelineCreation::OGLES2TextureUnitBindings			es2TextureBindings; //!< ES2 Shader Reflection. Use these 
    PipelineLayout										pipelineLayout;		//!< The pipeline layout
    RenderPass											renderPass;			//!< The Renderpass
    uint32												subPass;			//!< The subpass index
    GraphicsPipelineCreateParam(): subPass(0) {}
};

namespace impl {
class GraphicsPipelineImplBase
{
    friend class GraphicsPipeline_;
    friend class ParentableGraphicsPipeline_;
public:
	/*!
	   \brief Destructor
	 */
    virtual ~GraphicsPipelineImplBase() {}

	/*!
	   \brief Get information about a specific input binding
	   \param bindingId The binding id to return the info for.
	   \return Return binding info, else return NULL if not found
	 */
    virtual VertexInputBindingInfo const* getInputBindingInfo(pvr::uint16 bindingId)const = 0;
	/*!
	   \brief Get information about the vertex attributes of a specific binding point.
	   \param bindingId The binding id to return the info for.
	   \return Return attribute binding info, else return NULL if not found.
	 */
	virtual VertexAttributeInfoWithBinding const* getAttributesInfo(pvr::uint16 bindingId)const = 0;

	/*!
	   \brief Get the location of multiple uniforms at once. The underlying API must support Shader Reflection (OGLES yes, Vulkan no).
	   \param uniforms An array of \p numUniforms uniform variable names to retrieve the locations of
	   \param numUniforms Number of uniforms in the array \p uniforms
	   \param outLocation An array where the returned uniform locations will be stored. Location should contain enough memory for each uniform's location.
	   Any uniforms not found in the shader, or inactive, will get the location -1.
	 */
    virtual void getUniformLocation(const char8** uniforms, uint32 numUniforms, int32* outLocation)const = 0;

	/*!
	   \brief Get the location of the specified uniform. The underlying API must support Shader Reflection (OGLES yes, Vulkan no).
	   \param uniform The uniform variable name to retrieve the location of
	   \return The location of the uniform with variable name \p uniform. If not found or inactive, returns -1.
	 */
    virtual int32 getUniformLocation(const char8* uniform)const = 0;

	/*!
	   \brief Get the location of the specified vertex attribute. The underlying API must support Shader Reflection (OGLES yes, Vulkan no).
	   \param attribute The attribute variable name to retrieve the location of
	   \return The location of the attribute with variable name \p uniform. If not found or inactive, returns -1.
	 */
    virtual int32 getAttributeLocation(const char8* attribute)const = 0;

	/*!
	\brief Get the location of multiple vertex attributes at once. The underlying API must support Shader Reflection (OGLES yes, Vulkan no).
	\param attributes An array of \p numUAttributes vertex attribute variable names to retrieve the locations of
	\param numAttributes Number of attributes in the array \p uniforms
	\param outLocation An array where the returned attribute locations will be stored. Location should contain enough memory for each uniform's location.
	Any attributes not found in the shader, or inactive, will get the location -1.
	 */
    virtual void getAttributeLocation(const char8** attributes, uint32 numAttributes, int32* outLocation)const = 0;

	/*!
	   \brief Return the number of attributes for the specified binding point (VBO binding point).
	   \param bindingId A binding point
	   \return The number of attributes using the specified binding point
	 */
    virtual pvr::uint8 getNumAttributes(pvr::uint16 bindingId)const = 0;

	/*!
	   \brief Return the pipeline layout object that this GraphicsPipeline was created with
	   \return the pipeline layout object that this GraphicsPipeline was created with
	 */
    virtual const PipelineLayout& getPipelineLayout()const = 0;

	/*!
	   \brief Return the API-specific object (nothing for GLES, VkPipeline for Vulkan) underneath this GraphicsPipeline (if exists).
	   \return The API-specific object underneath this GraphicsPipeline (if exists).
	 */

    virtual const native::HPipeline_& getNativeObject() const = 0;

	/*!
	   \brief Return the API-specific object (nothing for GLES, VkPipeline for Vulkan) underneath this GraphicsPipeline (if exists).
	   \return The API-specific object underneath this GraphicsPipeline (if exists).
	 */
    virtual native::HPipeline_& getNativeObject() = 0;

	/*!
	   \brief Return the GraphicsPipelineCreateParam object that was used to create this graphics pipeline
	   \return the GraphicsPipelineCreateParam object that was used to create this graphics pipeline
	 */
    virtual const GraphicsPipelineCreateParam& getCreateParam()const = 0;
};

/*!****************************************************************************************************************
\brief API graphics pipeline wrapper. A GraphicsPipeline represents the configuration of almost the entire RenderState,
including vertex description, primitive assembly, Shader configuration, rasterization, blending etc. Access through
the Framework managed object GraphicsPipeline.
******************************************************************************************************************/
class GraphicsPipeline_
{
    friend class PopPipeline;
    friend class CommandBufferBase_;
    template<typename> friend class PackagedBindable;
    template<typename> friend struct ::pvr::RefCountEntryIntrusive;
    friend class ::pvr::IGraphicsContext;
public:
	virtual ~GraphicsPipeline_() { }

    /*!****************************************************************************************************************
    \brief Return pipeline vertex input binding info.
    \return VertexInputBindingInfo
    ******************************************************************************************************************/
    VertexInputBindingInfo const* getInputBindingInfo(pvr::uint16 bindingId)const
    {
        return pimpl->getInputBindingInfo(bindingId);
    }

    /*!****************************************************************************************************************
    \brief Return all the information on VertexAttributes of this pipeline.
    \return The information on VertexAttributes of this pipeline as a const pointer to VertexAttributeInfo.
    ******************************************************************************************************************/
    VertexAttributeInfoWithBinding const* getAttributesInfo(pvr::uint16 bindId)const
    {
        return pimpl->getAttributesInfo(bindId);
    }

    /*!****************************************************************************************************************
    \brief Get If uniforms are supported by the underlying API, get the shader locations of several uniform variables
            at once. If a uniform does not exist or is inactive, returns -1
    \param[in] uniforms An array of uniform variable names
    \param[in] numUniforms The number of uniforms in the array
    \param[out] outLocation An array where the locations will be saved. Writes -1 for inactive uniforms.
    ******************************************************************************************************************/
    void getUniformLocation(const char8** uniforms, uint32 numUniforms, int32* outLocation)
    {
        return pimpl->getUniformLocation(uniforms, numUniforms, outLocation);
    }

    /*!****************************************************************************************************************
    \brief Get If uniforms are supported by the underlying API, get the shader location of a uniform variables. If a
                uniform does not exist or is inactive, return -1
    \param uniform The name of a shader uniform variable name
    \return The location of the uniform, -1 if not found/inactive.
    ******************************************************************************************************************/
    int32 getUniformLocation(const char8* uniform) { return pimpl->getUniformLocation(uniform); }

    /*!****************************************************************************************************************
    \brief Get Get the shader locations of several uniform variables at once. If an attribute does not exist or is
            inactive, returns -1
    \brief attribute attributes name
    \return The shader attribute index of an attribute, -1 if nonexistent.
    ******************************************************************************************************************/
    int32 getAttributeLocation(const char8* attribute) { return pimpl->getAttributeLocation(attribute); }

    /*!****************************************************************************************************************
    \brief Get multiple attribute locations at once. If an attribute is inactive or does not exist, the location is
            set to -1
    \param[in] attributes The array of attributes names to get locations
    \param[in] numAttributes of attributes in the array
    \param[out] outLocation An array of sufficient size to write the locations to
    ******************************************************************************************************************/
    void getAttributeLocation(const char8** attributes, uint32 numAttributes, int32* outLocation)
    {
        return pimpl->getAttributeLocation(attributes, numAttributes, outLocation);
    }

    /*!****************************************************************************************************************
    \brief Get number of attributes of buffer binding.
    \param bindingId buffer binding id
    \return number of attributes
    ******************************************************************************************************************/
    pvr::uint8 getNumAttributes(pvr::uint16 bindingId)const { return pimpl->getNumAttributes(bindingId); }

    /*!****************************************************************************************************************
    \brief Return pipeline layout.
    \return const PipelineLayout&
    ******************************************************************************************************************/
    const PipelineLayout& getPipelineLayout()const
    {
        return pimpl->getPipelineLayout();
    }

	/*!
       \brief Return this native object handle (const)
	 */
    const native::HPipeline_& getNativeObject() const { return pimpl->getNativeObject(); }


	/*!
       \brief Return this native object handle
	 */
    native::HPipeline_& getNativeObject() { return pimpl->getNativeObject(); }

	/*!****************************************************************************************************************
	\brief	return pipeline create param used to create the child pipeline
	\return	const GraphicsPipelineCreateParam&
	*******************************************************************************************************************/
	const GraphicsPipelineCreateParam& getCreateParam()const { return pimpl->getCreateParam(); }

    // INTERNAL USE ONLY
    const GraphicsPipelineImplBase& getImpl()const { return *pimpl; }
    GraphicsPipelineImplBase& getImpl() { return *pimpl; }
protected:
    GraphicsPipeline_(std::auto_ptr<GraphicsPipelineImplBase>& pimpl) : pimpl(pimpl) {}
    std::auto_ptr<GraphicsPipelineImplBase> pimpl;
=======

/// <summary>This represents al the information needed to create a GraphicsPipeline. All items must have proper
/// values for a pipeline to be successfully created, but all those for which it is possible (except, for example,
/// Shaders and Vertex Formats) will have defaults same as their default values OpenGL ES graphics API.</summary>
struct GraphicsPipelineCreateParam
{
public:
	pipelineCreation::DepthStencilStateCreateParam    depthStencil;   //!< Depth and stencil buffer creation info
	pipelineCreation::ColorBlendStateCreateParam    colorBlend;     //!< Color blending and attachments info
	pipelineCreation::ViewportStateCreateParam      viewport;     //!< Viewport creation info
	pipelineCreation::RasterStateCreateParam      rasterizer;     //!< Rasterizer configuration creation info
	pipelineCreation::VertexInputCreateParam      vertexInput;    //!< Vertex Input creation info
	pipelineCreation::InputAssemblerStateCreateParam  inputAssembler;   //!< Input Assembler creation info
	pipelineCreation::VertexShaderStageCreateParam    vertexShader;   //!< Vertex shader information
	pipelineCreation::FragmentShaderStageCreateParam  fragmentShader;   //!< Fragment shader information
	pipelineCreation::GeometryShaderStageCreateParam  geometryShader;     //!< Geometry shader information
	pipelineCreation::TesselationStageCreateParam   tesselationStates;  //!< Tesselation Control and evaluation shader information
	pipelineCreation::MultiSampleStateCreateParam   multiSample;    //!< Multisampling information
	pipelineCreation::DynamicStatesCreateParam      dynamicStates;    //!< Dynamic state Information
	pipelineCreation::OGLES2TextureUnitBindings     es2TextureBindings; //!< ES2 Shader Reflection. Use these
	PipelineLayout                    pipelineLayout;   //!< The pipeline layout
	RenderPass                      renderPass;     //!< The Renderpass
	uint32                        subPass;      //!< The subpass index
	GraphicsPipelineCreateParam(): subPass(0) {}
};

namespace impl {
//!\cond NO_DOXYGEN
class GraphicsPipelineImplBase
{
	friend class GraphicsPipeline_;
	friend class ParentableGraphicsPipeline_;
public:
	virtual ~GraphicsPipelineImplBase() {}

	virtual VertexInputBindingInfo const* getInputBindingInfo(pvr::uint16 bindingId)const = 0;

	virtual VertexAttributeInfoWithBinding const* getAttributesInfo(pvr::uint16 bindingId)const = 0;

	virtual void getUniformLocation(const char8** uniforms, uint32 numUniforms, int32* outLocation)const = 0;

	virtual int32 getUniformLocation(const char8* uniform)const = 0;

	virtual int32 getAttributeLocation(const char8* attribute)const = 0;

	virtual void getAttributeLocation(const char8** attributes, uint32 numAttributes, int32* outLocation)const = 0;

	virtual pvr::uint8 getNumAttributes(pvr::uint16 bindingId)const = 0;

	virtual const PipelineLayout& getPipelineLayout()const = 0;

	virtual const GraphicsPipelineCreateParam& getCreateParam()const = 0;
};
//!\endcond

/// <summary>API graphics pipeline wrapper. A GraphicsPipeline represents the configuration of almost the entire
/// RenderState, including vertex description, primitive assembly, Shader configuration, rasterization, blending
/// etc. Access through the Framework managed object GraphicsPipeline.</summary>
class GraphicsPipeline_
{
	friend class PopPipeline;
	friend class CommandBufferBase_;
	template<typename> friend class PackagedBindable;
	template<typename> friend struct ::pvr::RefCountEntryIntrusive;
	friend class ::pvr::IGraphicsContext;
public:
	virtual ~GraphicsPipeline_() { }

	/// <summary>Return pipeline vertex input binding info.</summary>
	/// <returns>VertexInputBindingInfo</returns>
	VertexInputBindingInfo const* getInputBindingInfo(pvr::uint16 bindingId)const
	{
		return pimpl->getInputBindingInfo(bindingId);
	}

	/// <summary>Return all the information on VertexAttributes of this pipeline.</summary>
	/// <returns>The information on VertexAttributes of this pipeline as a const pointer to VertexAttributeInfo.
	/// </returns>
	VertexAttributeInfoWithBinding const* getAttributesInfo(pvr::uint16 bindId)const
	{
		return pimpl->getAttributesInfo(bindId);
	}

	/// <summary>Get If uniforms are supported by the underlying API, get the shader locations of several uniform
	/// variables at once. If a uniform does not exist or is inactive, returns -1</summary>
	/// <param name="uniforms">An array of uniform variable names</param>
	/// <param name="numUniforms">The number of uniforms in the array</param>
	/// <param name="outLocation">An array where the locations will be saved. Writes -1 for inactive uniforms.
	/// </param>
	void getUniformLocation(const char8** uniforms, uint32 numUniforms, int32* outLocation)
	{
		return pimpl->getUniformLocation(uniforms, numUniforms, outLocation);
	}

	/// <summary>Get If uniforms are supported by the underlying API, get the shader location of a uniform variables.
	/// If a uniform does not exist or is inactive, return -1</summary>
	/// <param name="uniform">The name of a shader uniform variable name</param>
	/// <returns>The location of the uniform, -1 if not found/inactive.</returns>
	int32 getUniformLocation(const char8* uniform) { return pimpl->getUniformLocation(uniform); }

	/// <summary>Get Get the shader locations of several uniform variables at once. If an attribute does not exist or
	/// is inactive, returns -1</summary>
	/// <summary>attribute attributes name</summary>
	/// <returns>The shader attribute index of an attribute, -1 if nonexistent.</returns>
	int32 getAttributeLocation(const char8* attribute) { return pimpl->getAttributeLocation(attribute); }

	/// <summary>Get multiple attribute locations at once. If an attribute is inactive or does not exist, the location
	/// is set to -1</summary>
	/// <param name="attributes">The array of attributes names to get locations</param>
	/// <param name="numAttributes">of attributes in the array</param>
	/// <param name="outLocation">An array of sufficient size to write the locations to</param>
	void getAttributeLocation(const char8** attributes, uint32 numAttributes, int32* outLocation)
	{
		return pimpl->getAttributeLocation(attributes, numAttributes, outLocation);
	}

	/// <summary>Get number of attributes of buffer binding.</summary>
	/// <param name="bindingId">buffer binding id</param>
	/// <returns>number of attributes</returns>
	pvr::uint8 getNumAttributes(pvr::uint16 bindingId)const { return pimpl->getNumAttributes(bindingId); }

	/// <summary>Return pipeline layout.</summary>
	/// <returns>const PipelineLayout&</returns>
	const PipelineLayout& getPipelineLayout()const
	{
		return pimpl->getPipelineLayout();
	}

	/// <summary>return pipeline create param used to create the child pipeline</summary>
	/// <returns>const GraphicsPipelineCreateParam&</returns>
	const GraphicsPipelineCreateParam& getCreateParam()const { return pimpl->getCreateParam(); }

	// INTERNAL USE ONLY
	const GraphicsPipelineImplBase& getImpl()const { return *pimpl; }
	GraphicsPipelineImplBase& getImpl() { return *pimpl; }
protected:
	GraphicsPipeline_(std::auto_ptr<GraphicsPipelineImplBase>& pimpl) : pimpl(pimpl) {}
	std::auto_ptr<GraphicsPipelineImplBase> pimpl;
>>>>>>> 1776432f... 4.3
};

/// <summary>API graphics pipeline wrapper. A GraphicsPipeline represents the configuration of almost the entire
/// RenderState, including vertex description, primitive assembly, Shader configuration, rasterization, blending
/// etc. Access through the Framework managed object GraphicsPipeline. A ParentableGraphicsPipeline is a pipeline
/// that is suitable to function as the "Parent" of another pipeline, helping to create efficient Pipeline
/// Hierarchies.</summary>
/// <remarks>ParentableGraphicsPipelines can and should be used to make switching between different pipelines more efficient. In effect, a
/// ParentableGraphicsPipeline allows the user to create another (non-parentable pipeline) as a "diff" of the
/// state between the Parentable pipeline and itself, making the transition between them very efficient.
/// </remarks>
class ParentableGraphicsPipeline_ : public GraphicsPipeline_
{
<<<<<<< HEAD
    friend class GraphicsPipeline_;
    friend class ::pvr::IGraphicsContext;
public:
    /*!****************************************************************************************************************
	\brief  INTERNAL. Use context->createGraphicsPipeline().
	\param pimpl INTERNAL
    ******************************************************************************************************************/
    ParentableGraphicsPipeline_(std::auto_ptr<GraphicsPipelineImplBase> pimpl) : GraphicsPipeline_(pimpl) {}
=======
	friend class GraphicsPipeline_;
	friend class ::pvr::IGraphicsContext;
public:
	/// <summary>INTERNAL. Use context->createGraphicsPipeline().</summary>
	/// <param name="pimpl">INTERNAL</param>
	ParentableGraphicsPipeline_(std::auto_ptr<GraphicsPipelineImplBase> pimpl) : GraphicsPipeline_(pimpl) {}
>>>>>>> 1776432f... 4.3
};
}
}
}
