/*!*********************************************************************************************************************
\file         PVRApi\OGLES\ComputePipelineGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        OpenGL ES Implementation of the ComputePipeline class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#pragma once
#include "PVRApi/ApiObjects/ComputePipeline.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRApi/OGLES/StateContainerGles.h"
#include "PVRApi/OGLES/ShaderGles.h"
namespace pvr {
namespace api {
namespace pipelineCreation {
void createStateObjects(const ComputeShaderStageCreateParam& thisobject, gles::ComputeStateContainer& storage);
}
namespace gles {

class ComputePipelineImplGles : public impl::ComputePipelineImplBase, public native::HPipeline_
{
public:
	ComputePipelineImplGles(GraphicsContext context) :
		m_initialized(false), m_owner(NULL), m_context(context) {}

	~ComputePipelineImplGles() { destroy(); }

	// ComputePipelineImplBase interface
	void getUniformLocation(const char8** uniforms, uint32 numUniforms, int32* outLocation);

	bool init(const ComputePipelineCreateParam& desc, impl::ComputePipeline_ * owner);

	void bind();

	void setAll();

	native::HPipeline_& getNativeObject();

	const native::HPipeline_& getNativeObject() const;

	inline ComputeShaderProgramState getShaderProgram()const;

	/*!*********************************************************************************************************************
	\brief Get the PipelineLayout of this pipeline.
	\return The PipelineLayout of this pipeline.
	***********************************************************************************************************************/
	const PipelineLayout& getPipelineLayout()const;

	/*!*********************************************************************************************************************
	\brief If free standing uniforms are supported by the underlying API, get the location of a uniform.
	If a uniform is not found (does not exist or is not active in the shader program), -1 is returned.
	\param uniform uniform name
	\return The location of a uniform variable
	***********************************************************************************************************************/
	int32 getUniformLocation(const char8* uniform);

	void destroy();

	Result createProgram();

	GraphicsContext m_context;

	ComputeStateContainer m_states;
	impl::ComputePipeline_* m_owner;
	bool m_initialized;

};
}
}
}
//!\endcond
