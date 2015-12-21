/*!*********************************************************************************************************************
\file         PVRApi\OGLES\ShaderProgramState.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementation of methods for the GraphicsShaderProgramState class. See ShaderProgramState.h.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/ApiObjects/ShaderProgramState.h"
#include "PVRApi/ApiErrors.h"
#include "PVRApi/OGLES/NativeObjectsGles.h"
#include "PVRApi/OGLES/OpenGLESBindings.h"

namespace pvr {
namespace api {
namespace impl {
void GraphicsShaderProgramState::bind()
{
	gl::UseProgram(m_shaderProgram->handle);
	debugLogApiError("GraphicsShaderProgramState::bind exit");
}
void GraphicsShaderProgramState::reset(pvr::IGraphicsContext& device)
{
	gl::UseProgram(0);
	debugLogApiError("GraphicsShaderProgramState::reset exit");
}
void GraphicsShaderProgramState::destroy()
{
	gl::DeleteProgram(m_shaderProgram->handle); m_shaderProgram.release(); m_isValid = false;
	debugLogApiError("GraphicsShaderProgramState::destoy exit");
}
void GraphicsShaderProgramState::generate()
{
	if (!m_shaderProgram.isValid()) { m_shaderProgram.construct(0); }
	m_shaderProgram->handle = gl::CreateProgram(); m_isValid = true;
	debugLogApiError("GraphicsShaderProgramState::generate exit");
}

bool GraphicsShaderProgramState::saveProgramBinary(Stream& outFile)
{
#if (!defined(BUILD_API_MAX)||(BUILD_API_MAX>=30))
	// validate the program
	GLint linked;
	gl::GetProgramiv(m_shaderProgram->handle, GL_LINK_STATUS, &linked);
	if (!linked) { return false; }

	// get the length of the shader binary program in memory.
	GLsizei length = 0;
	gl::GetProgramiv(m_shaderProgram->handle, GL_PROGRAM_BINARY_LENGTH, &length);

	// No binary?
	if (length == 0) { return false; }

	std::vector<byte> shaderBinary;
	shaderBinary.resize(length);

	GLenum binaryFmt = 0;
	GLsizei lengthWritten = 0;
	gl::GetProgramBinary(m_shaderProgram->handle, length, &lengthWritten, &binaryFmt, &shaderBinary[0]);

	// save failed?
	if (!lengthWritten) { return false; }

	// save the binary format
	size_t fileWrittenLen = 0;
	bool rslt = outFile.write(sizeof(GLenum), 1, (void*)&binaryFmt, fileWrittenLen);

	// File failed
	if (!rslt)  { return false; }

	// save the program
	rslt = outFile.write(length, 1, &shaderBinary[0], fileWrittenLen);

	return rslt;
#else
	PVR_ASSERT(0
	           && "ShaderUtils::saveProgramBinary Underlying API OpenGL ES 2 does not support Program Binaries");
	Log(Log.Error,
	    "ShaderUtils::saveProgramBinary Underlying API OpenGL ES 2 does not support Program Binaries");
	return Result::UnsupportedRequest;
#endif
}


/////////////////////////////// COMPUTE SHADER ///////////////////////////////
void ComputeShaderProgramState::generate()
{
	if (!m_shaderProgram.isValid()) { m_shaderProgram.construct(0); }
	m_shaderProgram->handle = gl::CreateProgram(); m_isValid = true;
	debugLogApiError("ComputeShaderProgramState::generate exit");
}
void ComputeShaderProgramState::bind()
{
	gl::UseProgram(m_shaderProgram->handle);
	debugLogApiError("ComputeShaderProgramState::bind exit");
}
void ComputeShaderProgramState::reset(pvr::IGraphicsContext& device)
{
	gl::UseProgram(0);
	debugLogApiError("ComputeShaderProgramState::reset exit");
}
}
}//namespace api
}//namespace pvr
 //!\endcond