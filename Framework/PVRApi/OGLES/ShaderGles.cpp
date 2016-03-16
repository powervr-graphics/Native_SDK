#include "PVRApi/OGLES/ShaderGles.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"
/*!*********************************************************************************************************************
\file         PVRApi\OGLES\ShaderGles.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementation of the IGraphicsContext::createShader methods. See IGraphicsContext.h.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN

namespace pvr {
namespace api {
namespace impl {
native::HShader_& Shader_::getNativeObject()
{
	return static_cast<gles::ShaderGles_&>(*this);
}
const native::HShader_& Shader_::getNativeObject() const
{
	return static_cast<const gles::ShaderGles_&>(*this);
}
}// namespace impl

namespace gles {
ShaderGles_::~ShaderGles_()
{
	if (m_context.isValid())
	{
		gl::DeleteShader(getNativeObject().handle);
	}
	else
	{
		Log(Log.Warning, "Tried to delete shader after context destruction");
	}
}
}
}// namespace api
}// namespace pvr
//!\endcond
