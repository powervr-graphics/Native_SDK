/*!
\brief Implementation of the IGraphicsContext::createShader methods. See IGraphicsContext.h.
\file PVRApi/OGLES/ShaderGles.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRApi/OGLES/ShaderGles.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"
namespace pvr {
namespace api {

namespace gles {
ShaderGles_::~ShaderGles_()
{
	if (_context.isValid())
	{
		gl::DeleteShader(handle);
	}
	else
	{
		Log(Log.Warning, "Tried to delete shader after context destruction");
	}
}
}
}// namespace api
}// namespace pvr
