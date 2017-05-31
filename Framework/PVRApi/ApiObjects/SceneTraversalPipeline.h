/*!
\brief The SceneTraversal pipeline represents all state that is expected to be able to be
"baked" ahead of time - Shaders, renderpass, subpass etc.
\file PVRApi/ApiObjects/SceneTraversalPipeline.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiObjects/PipelineLayout.h"
#include "PVRApi/ApiObjects/PipelineConfig.h"

namespace pvr {
namespace api {
/// <summary>This represents al the information needed to create a SceneTraversalPipeline. All items must have proper values
/// for a pipeline to be successfully created, but all those for which it is possible(except, for example,
/// Shaders) will have defaults same as their default values OpenGL ES graphics API.</summary>

struct SceneTraversalPipelineCreateParam
{
public:
	pipelineCreation::SceneTraversalShaderStageCreateParam	frameShader;	//!< frame shader information
	pvr::api::PipelineLayout							pipelineLayout;		//!< The pipeline layout

	SceneTraversalPipelineCreateParam() {}
};

namespace impl {
class SceneTraversalPipelineImplBase
{
	friend class SceneTraversalPipeline_;
	friend class ParentableSceneTraversalPipeline_;
public:
	/// <summary>Destructor</summary>
	virtual ~SceneTraversalPipelineImplBase() {}

	/// <summary>Get information about a specific input binding</summary>
	/// <param name="bindingId">The binding id to return the info for.</param>
	/// <returns>Return binding info, else return NULL if not found</returns>
	virtual VertexInputBindingInfo const* getInputBindingInfo(pvr::uint16 bindingId)const = 0;

	/// <summary>Get information about the vertex attributes of a specific binding point.</summary>
	/// <param name="bindingId">The binding id to return the info for.</param>
	/// <returns>Return attribute binding info, else return NULL if not found.</returns>
	virtual VertexAttributeInfoWithBinding const* getAttributesInfo(pvr::uint16 bindingId)const = 0;

	/// <summary>Get the location of multiple uniforms at once. The underlying API must support Shader Reflection
	/// (OGLES yes, Vulkan no).</summary>
	/// <param name="uniforms">An array of <paramref name="numUniforms"/>uniform variable names to retrieve the
	/// locations of</param>
	/// <param name="numUniforms">Number of uniforms in the array <paramref name="uniforms"/></param>
	/// <param name="outLocation">An array where the returned uniform locations will be stored. Location should
	/// contain enough memory for each uniform's location. Any uniforms not found in the shader, or inactive, will get
	/// the location -1.</param>
	virtual void getUniformLocation(const char8** uniforms, uint32 numUniforms, int32* outLocation)const = 0;

	/// <summary>Get the location of the specified uniform. The underlying API must support Shader Reflection (OGLES
	/// yes, Vulkan no).</summary>
	/// <param name="uniform">The uniform variable name to retrieve the location of</param>
	/// <returns>The location of the uniform with variable name <paramref name="uniform."/>If not found or inactive,
	/// returns -1.</returns>
	virtual int32 getUniformLocation(const char8* uniform)const = 0;

	/// <summary>Get the location of the specified vertex attribute. The underlying API must support Shader Reflection
	/// (OGLES yes, Vulkan no).</summary>
	/// <param name="attribute">The attribute variable name to retrieve the location of</param>
	/// <returns>The location of the attribute with variable name <paramref name="uniform."/>If not found or inactive,
	/// returns -1.</returns>
	virtual int32 getAttributeLocation(const char8* attribute)const = 0;

	/// <summary>Get the location of multiple vertex attributes at once. The underlying API must support Shader
	/// Reflection (OGLES yes, Vulkan no).</summary>
	/// <param name="attributes">An array of <paramref name="numUAttributes"/>vertex attribute variable names to
	/// retrieve the locations of</param>
	/// <param name="numAttributes">Number of attributes in the array <paramref name="uniforms"/></param>
	/// <param name="outLocation">An array where the returned attribute locations will be stored. Location should
	/// contain enough memory for each uniform's location. Any attributes not found in the shader, or inactive, will
	/// get the location -1.</param>
	virtual void getAttributeLocation(const char8** attributes, uint32 numAttributes, int32* outLocation)const = 0;

	/// <summary>Return the number of attributes for the specified binding point (VBO binding point).</summary>
	/// <param name="bindingId">A binding point</param>
	/// <returns>The number of attributes using the specified binding point</returns>
	virtual pvr::uint8 getNumAttributes(pvr::uint16 bindingId)const = 0;


	/// <summary>Return the number of attributes for the specified binding point (VBO binding point).</summary>
	/// <param name="bindingId">A binding point</param>
	/// <returns>The number of attributes using the specified binding point</returns>

	/// <summary>Return pipeline layout.</summary>
	/// <returns>const PipelineLayout&</returns>
	virtual const PipelineLayout& getPipelineLayout()const = 0;

	/// <summary>Return this native object handle (const)</summary>
	virtual const native::HPipeline_& getNativeObject() const = 0;

	/// <summary>Return this native object handle</summary>
	virtual native::HPipeline_& getNativeObject() = 0;

	/// <summary>return pipeline create param used to create the child pipeline</summary>
	/// <returns>const SceneTraversalPipelineCreateParam&</returns>
	virtual const SceneTraversalPipelineCreateParam& getCreateParam()const = 0;
};

/// <summary>API SceneTraversal pipeline wrapper. A SceneTraversalPipeline represents the configuration for ray initialisation.
/// Access through the Framework managed object SceneTraversalPipeline.</summary>
class SceneTraversalPipeline_
{
	friend class PopPipeline;
	friend class CommandBufferBase_;
	template<typename> friend class PackagedBindable;
	template<typename> friend struct ::pvr::RefCountEntryIntrusive;
	friend class ::pvr::IGraphicsContext;
public:
	virtual ~SceneTraversalPipeline_() { }

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

	/// <summary>Return this native object handle (const)</summary>
	const native::HPipeline_& getNativeObject() const { return pimpl->getNativeObject(); }


	/// <summary>Return this native object handle</summary>
	native::HPipeline_& getNativeObject() { return pimpl->getNativeObject(); }

	/// <summary>return pipeline create param used to create the child pipeline</summary>
	/// <returns>const SceneTraversalPipelineCreateParam&</returns>
	const SceneTraversalPipelineCreateParam& getCreateParam()const { return pimpl->getCreateParam(); }

	// INTERNAL USE ONLY
	const SceneTraversalPipelineImplBase& getImpl()const { return *pimpl; }
	SceneTraversalPipelineImplBase& getImpl() { return *pimpl; }
protected:
	SceneTraversalPipeline_(std::auto_ptr<SceneTraversalPipelineImplBase>& pimpl) : pimpl(pimpl) {}
	std::auto_ptr<SceneTraversalPipelineImplBase> pimpl;
};

/// <summary>API SceneTraversal pipeline wrapper. A SceneTraversalPipeline represents the configuration for ray initialisation.
/// Access through the Framework managed object SceneTraversalPipeline. A ParentableSceneTraversalPipeline is a pipeline
/// that is suitable to function as the "Parent" of another pipeline, helping to create efficient Pipeline
/// Hierarchies.</summary>
/// <remarks>ParentableSceneTraversalPipelines can and should be used to make switching between different pipelines more efficient. In effect, a
/// ParentableSceneTraversalPipeline allows the user to create another (non-parentable pipeline) as a "diff" of the
/// state between the Parentable pipeline and itself, making the transition between them very efficient.
/// </remarks>
class ParentableSceneTraversalPipeline_ : public SceneTraversalPipeline_
{
	friend class SceneTraversalPipeline_;
	friend class ::pvr::IGraphicsContext;
public:
	/// <summary>INTERNAL. Use context->createSceneTraversalPipeline().</summary>
	/// <param name="pimpl">INTERNAL</param>
	ParentableSceneTraversalPipeline_(std::auto_ptr<SceneTraversalPipelineImplBase> pimpl) : SceneTraversalPipeline_(pimpl) {}
};
}
}
}
