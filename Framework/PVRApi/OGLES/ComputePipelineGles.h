/*!
\brief OpenGL ES Implementation of the ComputePipeline class.
\file PVRApi/OGLES/ComputePipelineGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiObjects/ComputePipeline.h"
#include "PVRApi/OGLES/StateContainerGles.h"
#include "PVRApi/OGLES/ContextGles.h"
namespace pvr {
namespace api {
namespace gles {

class ComputePipelineImplGles : public impl::ComputePipelineImplBase
{
public:
	ComputePipelineImplGles(GraphicsContext context) :
		_initialized(false), _owner(NULL), _context(context) {}

	const ComputeShaderProgramState& getShaderProgram()const;
	ComputeShaderProgramState& getShaderProgram();

	void getUniformLocation(const char8** uniforms, uint32 numUniforms, int32* outLocation) const;

	~ComputePipelineImplGles() { destroy(); }

	void destroy();

	int32 getUniformLocation(const char8* uniform)const;

	const PipelineLayout& getPipelineLayout()const;

	bool init(const ComputePipelineCreateParam& desc, ComputePipeline& owner);

	Result createProgram();

	bool _initialized;

	void setAll();

	const ComputePipelineCreateParam& getCreateParam()const;

	void bind()
	{
		pvr::platform::ContextGles& contextES = native_cast(*_context);
		if (!contextES.isLastBoundPipelineCompute() || contextES.getBoundComputePipeline() != _owner)
		{
			setAll();
			contextES.onBind(_owner);
		}
	}

	ComputeStateContainer         _states;
	impl::ComputePipeline_*         _owner;
	GraphicsContext             _context;
	ComputePipelineCreateParam        _createParam;

};
}

inline const native::HPipeline_& native_cast(const impl::ComputePipeline_& object)
{
	return native_cast(static_cast<const gles::ComputePipelineImplGles&>(object.getImpl()).getShaderProgram());
}
inline native::HPipeline_& native_cast(impl::ComputePipeline_& object)
{
	return native_cast(static_cast<gles::ComputePipelineImplGles&>(object.getImpl()).getShaderProgram());
}
inline const native::HPipeline_& native_cast(const ComputePipeline& object)
{
	return native_cast(*object);
}
inline native::HPipeline_& native_cast(ComputePipeline& object)
{
	return native_cast(*object);
}
}
}
