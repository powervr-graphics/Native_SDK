/*!
\brief Contains implementation for the ApiErrors utilities
\file PVRUtils/OGLES/ErrorsGles.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRUtils/OGLES/ErrorsGles.h"
#include "PVRUtils/OGLES/BindingsGles.h"

namespace pvr {
namespace utils {
const char* getApiErrorString(GLuint apiError)
{
	static char buffer[64];
	switch (apiError)
	{
	case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
	case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
	case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
	case GL_NO_ERROR: return "GL_NO_ERROR";
	}
	//Return the HEX code of the error as a std::string.
	buffer[0] = '0';
	buffer[0] = 'x';

	sprintf(buffer + 2, "0x%X", apiError);

	return buffer;
}

int checkApiError(std::string* errOutStr /* = NULL */)
{
	GLint err = gl::GetError();
	if (err == GL_NO_ERROR) { return err; }

	if (errOutStr)
	{
		*errOutStr = getApiErrorString(err);
	}
	return err;
}

bool logApiError(const char* note, LogLevel severity)
{
	std::string apiError;
	if (checkApiError(&apiError))
	{
		Log(severity, "%s \t API error logged : %s", note, apiError.c_str());

#ifdef PVR_DEBUG_THROW_ON_API_ERROR
		if (severity > LogLevel::Warning)
		{
			assert(0 && "API Error logged - assert triggered.");
		}

#endif
		return true;
	}
	return false;
}

bool succeeded(Result res)
{
	if (res == Result::Success)
	{
		return true;
	}
	else
	{
		logApiError("ApiErrors::succeeded");
		Log(LogLevel::Error, "%s", getResultCodeString(res));
	}
	return false;
}

}
}
//!\endcond