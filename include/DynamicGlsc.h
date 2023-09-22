#pragma once
#include "DynamicEgl.h"
#include "pvr_openlib.h"

#ifdef GL_PROTOTYPES
#undef GL_PROTOTYPES
#endif
#define GL_NO_PROTOTYPES
#define EGL_NO_PROTOTYPES
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif
#include <utility>
#include <stdint.h>
#include <string>

// DETERMINE IF FUNCTIONS SHOULD BE PREPENDED WITH gl OR PUT IN gl:: NAMESPACE (DEFAULT IS NAMESPACE)
#ifndef DYNAMICGLES_NO_NAMESPACE
#define DYNAMICGLES_FUNCTION(name) GL_APIENTRY name
#else
#if TARGET_OS_IPHONE
#define DYNAMICGLES_FUNCTION(name) GL_APIENTRY name
#else
#define DYNAMICGLES_FUNCTION(name) GL_APIENTRY gl##name
#endif
#endif

#define GLSC2_GLUE_UNUSED( x ) ( (void)x )

// GLvoid is not defined in GLSC2.
typedef void             GLvoid;

// DYNAMICGLES_FUNCTION THE PLATFORM SPECIFIC LIBRARY NAME
namespace gl {
namespace internals {

#ifdef _WIN32
static const char* libName = "libGLESv2.dll";
#elif defined(__APPLE__)
static const char* libName = "libGLSCv2.dylib";
#else
static const char* libName = "libGLSCv2.so";
#endif

} // namespace internals
} // namespace gl

#include "GLSC2/glsc2.h"
#include "GLSC2/glsc2ext.h"

namespace gl {
namespace internals {
namespace Glsc2FuncName {
	enum OpenGLSC2FunctionName
	{
		// Functions originally from GLES2
		ActiveTexture,
		BindBuffer,
		BindFramebuffer,
		BindRenderbuffer,
		BindTexture,
		BlendColor,
		BlendEquation,
		BlendEquationSeparate,
		BlendFunc,
		BlendFuncSeparate,
		BufferData,
		BufferSubData,
		CheckFramebufferStatus,
		Clear,
		ClearColor,
		ClearDepthf,
		ClearStencil,
		ColorMask,
		CompressedTexSubImage2D,
		CreateProgram,
		CullFace,
		DepthFunc,
		DepthMask,
		DepthRangef,
		Disable,
		DisableVertexAttribArray,
		DrawArrays,
		DrawRangeElements,
		Enable,
		EnableVertexAttribArray,
		Finish,
		Flush,
		FramebufferRenderbuffer,
		FramebufferTexture2D,
		FrontFace,
		GenBuffers,
		GenerateMipmap,
		GenFramebuffers,
		GenRenderbuffers,
		GenTextures,
		GetAttribLocation,
		GetBooleanv,
		GetBufferParameteriv,
		GetError,
		GetFloatv,
		GetFramebufferAttachmentParameteriv,
		GetGraphicsResetStatus,
		GetIntegerv,
		GetProgramiv,
		GetRenderbufferParameteriv,
		GetString,
		GetTexParameterfv,
		GetTexParameteriv,
		GetnUniformfv,
		GetnUniformiv,
		GetUniformLocation,
		GetVertexAttribfv,
		GetVertexAttribiv,
		GetVertexAttribPointerv,
		Hint,
		IsEnabled,
		LineWidth,
		PixelStorei,
		PolygonOffset,
		ProgramBinary,
		ReadnPixels,
		RenderbufferStorage,
		SampleCoverage,
		Scissor,
		StencilFunc,
		StencilFuncSeparate,
		StencilMask,
		StencilMaskSeparate,
		StencilOp,
		StencilOpSeparate,
		TexStorage2D,
		TexParameterf,
		TexParameterfv,
		TexParameteri,
		TexParameteriv,
		TexSubImage2D,
		Uniform1f,
		Uniform1fv,
		Uniform1i,
		Uniform1iv,
		Uniform2f,
		Uniform2fv,
		Uniform2i,
		Uniform2iv,
		Uniform3f,
		Uniform3fv,
		Uniform3i,
		Uniform3iv,
		Uniform4f,
		Uniform4fv,
		Uniform4i,
		Uniform4iv,
		UniformMatrix2fv,
		UniformMatrix3fv,
		UniformMatrix4fv,
		UseProgram,
		VertexAttrib1f,
		VertexAttrib1fv,
		VertexAttrib2f,
		VertexAttrib2fv,
		VertexAttrib3f,
		VertexAttrib3fv,
		VertexAttrib4f,
		VertexAttrib4fv,
		VertexAttribPointer,
		Viewport,

		NUMBER_OF_OPENGLSC2_FUNCTIONS
	};
} // namespace Glsc2FuncName

// Preloads the OpenGL SC 2.0 function pointers the first time any OpenGL SC 2.0 function call is made
inline void* getSc20Function(gl::internals::Glsc2FuncName::OpenGLSC2FunctionName funcname)
{
	static void* FunctionTable[Glsc2FuncName::NUMBER_OF_OPENGLSC2_FUNCTIONS];

#if !TARGET_OS_IPHONE
	// Retrieve the OpenGL SC 2.0 functions pointers once
	if (!FunctionTable[0])
	{
		pvr::lib::LIBTYPE lib = pvr::lib::openlib(libName);
		if (!lib) { Log_Error("OpenGL SC Bindings: Failed to open library %s\n", libName); }
		else
		{
			Log_Info("OpenGL SC Bindings: Successfully loaded library %s for OpenGL SC 2.0\n", libName);
		}

		FunctionTable[Glsc2FuncName::ActiveTexture] = pvr::lib::getLibFunctionChecked<void*>(lib, "glActiveTexture");
//		FunctionTable[Glsc2FuncName::AttachShader] = pvr::lib::getLibFunctionChecked<void*>(lib, "glAttachShader");
//		FunctionTable[Glsc2FuncName::BindAttribLocation] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBindAttribLocation");
		FunctionTable[Glsc2FuncName::BindBuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBindBuffer");
		FunctionTable[Glsc2FuncName::BindFramebuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBindFramebuffer");
		FunctionTable[Glsc2FuncName::BindRenderbuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBindRenderbuffer");
		FunctionTable[Glsc2FuncName::BindTexture] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBindTexture");
		FunctionTable[Glsc2FuncName::BlendColor] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBlendColor");
		FunctionTable[Glsc2FuncName::BlendEquation] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBlendEquation");
		FunctionTable[Glsc2FuncName::BlendEquationSeparate] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBlendEquationSeparate");
		FunctionTable[Glsc2FuncName::BlendFunc] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBlendFunc");
		FunctionTable[Glsc2FuncName::BlendFuncSeparate] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBlendFuncSeparate");
		FunctionTable[Glsc2FuncName::BufferData] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBufferData");
		FunctionTable[Glsc2FuncName::BufferSubData] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBufferSubData");
		FunctionTable[Glsc2FuncName::CheckFramebufferStatus] = pvr::lib::getLibFunctionChecked<void*>(lib, "glCheckFramebufferStatus");
		FunctionTable[Glsc2FuncName::Clear] = pvr::lib::getLibFunctionChecked<void*>(lib, "glClear");
		FunctionTable[Glsc2FuncName::ClearColor] = pvr::lib::getLibFunctionChecked<void*>(lib, "glClearColor");
		FunctionTable[Glsc2FuncName::ClearDepthf] = pvr::lib::getLibFunctionChecked<void*>(lib, "glClearDepthf");
		FunctionTable[Glsc2FuncName::ClearStencil] = pvr::lib::getLibFunctionChecked<void*>(lib, "glClearStencil");
		FunctionTable[Glsc2FuncName::ColorMask] = pvr::lib::getLibFunctionChecked<void*>(lib, "glColorMask");
//		FunctionTable[Glsc2FuncName::CompileShader] = pvr::lib::getLibFunctionChecked<void*>(lib, "glCompileShader");
//		FunctionTable[Glsc2FuncName::CompressedTexImage2D] = pvr::lib::getLibFunctionChecked<void*>(lib, "glCompressedTexImage2D");
		FunctionTable[Glsc2FuncName::CompressedTexSubImage2D] = pvr::lib::getLibFunctionChecked<void*>(lib, "glCompressedTexSubImage2D");
//		FunctionTable[Glsc2FuncName::CopyTexImage2D] = pvr::lib::getLibFunctionChecked<void*>(lib, "glCopyTexImage2D");
//		FunctionTable[Glsc2FuncName::CopyTexSubImage2D] = pvr::lib::getLibFunctionChecked<void*>(lib, "glCopyTexSubImage2D");
		FunctionTable[Glsc2FuncName::CreateProgram] = pvr::lib::getLibFunctionChecked<void*>(lib, "glCreateProgram");
//		FunctionTable[Glsc2FuncName::CreateShader] = pvr::lib::getLibFunctionChecked<void*>(lib, "glCreateShader");
		FunctionTable[Glsc2FuncName::CullFace] = pvr::lib::getLibFunctionChecked<void*>(lib, "glCullFace");
//		FunctionTable[Glsc2FuncName::DeleteBuffers] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDeleteBuffers");
//		FunctionTable[Glsc2FuncName::DeleteFramebuffers] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDeleteFramebuffers");
//		FunctionTable[Glsc2FuncName::DeleteProgram] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDeleteProgram");
//		FunctionTable[Glsc2FuncName::DeleteRenderbuffers] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDeleteRenderbuffers");
//		FunctionTable[Glsc2FuncName::DeleteShader] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDeleteShader");
//		FunctionTable[Glsc2FuncName::DeleteTextures] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDeleteTextures");
		FunctionTable[Glsc2FuncName::DepthFunc] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDepthFunc");
		FunctionTable[Glsc2FuncName::DepthMask] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDepthMask");
		FunctionTable[Glsc2FuncName::DepthRangef] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDepthRangef");
//		FunctionTable[Glsc2FuncName::DetachShader] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDetachShader");
		FunctionTable[Glsc2FuncName::Disable] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDisable");
		FunctionTable[Glsc2FuncName::DisableVertexAttribArray] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDisableVertexAttribArray");
		FunctionTable[Glsc2FuncName::DrawArrays] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDrawArrays");
//		FunctionTable[Glsc2FuncName::DrawElements] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDrawElements");
		FunctionTable[Glsc2FuncName::Enable] = pvr::lib::getLibFunctionChecked<void*>(lib, "glEnable");
		FunctionTable[Glsc2FuncName::EnableVertexAttribArray] = pvr::lib::getLibFunctionChecked<void*>(lib, "glEnableVertexAttribArray");
		FunctionTable[Glsc2FuncName::Finish] = pvr::lib::getLibFunctionChecked<void*>(lib, "glFinish");
		FunctionTable[Glsc2FuncName::Flush] = pvr::lib::getLibFunctionChecked<void*>(lib, "glFlush");
		FunctionTable[Glsc2FuncName::FramebufferRenderbuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "glFramebufferRenderbuffer");
		FunctionTable[Glsc2FuncName::FramebufferTexture2D] = pvr::lib::getLibFunctionChecked<void*>(lib, "glFramebufferTexture2D");
		FunctionTable[Glsc2FuncName::FrontFace] = pvr::lib::getLibFunctionChecked<void*>(lib, "glFrontFace");
		FunctionTable[Glsc2FuncName::GenBuffers] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGenBuffers");
		FunctionTable[Glsc2FuncName::GenerateMipmap] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGenerateMipmap");
		FunctionTable[Glsc2FuncName::GenFramebuffers] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGenFramebuffers");
		FunctionTable[Glsc2FuncName::GenRenderbuffers] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGenRenderbuffers");
		FunctionTable[Glsc2FuncName::GenTextures] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGenTextures");
//		FunctionTable[Glsc2FuncName::GetActiveAttrib] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetActiveAttrib");
//		FunctionTable[Glsc2FuncName::GetActiveUniform] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetActiveUniform");
//		FunctionTable[Glsc2FuncName::GetAttachedShaders] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetAttachedShaders");
		FunctionTable[Glsc2FuncName::GetAttribLocation] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetAttribLocation");
		FunctionTable[Glsc2FuncName::GetBooleanv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetBooleanv");
		FunctionTable[Glsc2FuncName::GetBufferParameteriv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetBufferParameteriv");
		FunctionTable[Glsc2FuncName::GetError] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetError");
		FunctionTable[Glsc2FuncName::GetFloatv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetFloatv");
		FunctionTable[Glsc2FuncName::GetFramebufferAttachmentParameteriv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetFramebufferAttachmentParameteriv");
		FunctionTable[Glsc2FuncName::GetIntegerv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetIntegerv");
		FunctionTable[Glsc2FuncName::GetProgramiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetProgramiv");
//		FunctionTable[Glsc2FuncName::GetProgramInfoLog] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetProgramInfoLog");
		FunctionTable[Glsc2FuncName::GetRenderbufferParameteriv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetRenderbufferParameteriv");
//		FunctionTable[Glsc2FuncName::GetShaderiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetShaderiv");
//		FunctionTable[Glsc2FuncName::GetShaderInfoLog] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetShaderInfoLog");
//		FunctionTable[Glsc2FuncName::GetShaderPrecisionFormat] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetShaderPrecisionFormat");
//		FunctionTable[Glsc2FuncName::GetShaderSource] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetShaderSource");
		FunctionTable[Glsc2FuncName::GetString] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetString");
		FunctionTable[Glsc2FuncName::GetTexParameterfv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetTexParameterfv");
		FunctionTable[Glsc2FuncName::GetTexParameteriv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetTexParameteriv");
//		FunctionTable[Glsc2FuncName::GetUniformfv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetUniformfv");
//		FunctionTable[Glsc2FuncName::GetUniformiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetUniformiv");
		FunctionTable[Glsc2FuncName::GetUniformLocation] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetUniformLocation");
		FunctionTable[Glsc2FuncName::GetVertexAttribfv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetVertexAttribfv");
		FunctionTable[Glsc2FuncName::GetVertexAttribiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetVertexAttribiv");
		FunctionTable[Glsc2FuncName::GetVertexAttribPointerv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetVertexAttribPointerv");
		FunctionTable[Glsc2FuncName::Hint] = pvr::lib::getLibFunctionChecked<void*>(lib, "glHint");
//		FunctionTable[Glsc2FuncName::IsBuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "glIsBuffer");
		FunctionTable[Glsc2FuncName::IsEnabled] = pvr::lib::getLibFunctionChecked<void*>(lib, "glIsEnabled");
//		FunctionTable[Glsc2FuncName::IsFramebuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "glIsFramebuffer");
//		FunctionTable[Glsc2FuncName::IsProgram] = pvr::lib::getLibFunctionChecked<void*>(lib, "glIsProgram");
//		FunctionTable[Glsc2FuncName::IsRenderbuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "glIsRenderbuffer");
//		FunctionTable[Glsc2FuncName::IsShader] = pvr::lib::getLibFunctionChecked<void*>(lib, "glIsShader");
//		FunctionTable[Glsc2FuncName::IsTexture] = pvr::lib::getLibFunctionChecked<void*>(lib, "glIsTexture");
		FunctionTable[Glsc2FuncName::LineWidth] = pvr::lib::getLibFunctionChecked<void*>(lib, "glLineWidth");
//		FunctionTable[Glsc2FuncName::LinkProgram] = pvr::lib::getLibFunctionChecked<void*>(lib, "glLinkProgram");
		FunctionTable[Glsc2FuncName::PixelStorei] = pvr::lib::getLibFunctionChecked<void*>(lib, "glPixelStorei");
		FunctionTable[Glsc2FuncName::PolygonOffset] = pvr::lib::getLibFunctionChecked<void*>(lib, "glPolygonOffset");
//		FunctionTable[Glsc2FuncName::ReadPixels] = pvr::lib::getLibFunctionChecked<void*>(lib, "glReadPixels");
//		FunctionTable[Glsc2FuncName::ReleaseShaderCompiler] = pvr::lib::getLibFunctionChecked<void*>(lib, "glReleaseShaderCompiler");
		FunctionTable[Glsc2FuncName::RenderbufferStorage] = pvr::lib::getLibFunctionChecked<void*>(lib, "glRenderbufferStorage");
		FunctionTable[Glsc2FuncName::SampleCoverage] = pvr::lib::getLibFunctionChecked<void*>(lib, "glSampleCoverage");
		FunctionTable[Glsc2FuncName::Scissor] = pvr::lib::getLibFunctionChecked<void*>(lib, "glScissor");
//		FunctionTable[Glsc2FuncName::ShaderBinary] = pvr::lib::getLibFunctionChecked<void*>(lib, "glShaderBinary");
//		FunctionTable[Glsc2FuncName::ShaderSource] = pvr::lib::getLibFunctionChecked<void*>(lib, "glShaderSource");
		FunctionTable[Glsc2FuncName::StencilFunc] = pvr::lib::getLibFunctionChecked<void*>(lib, "glStencilFunc");
		FunctionTable[Glsc2FuncName::StencilFuncSeparate] = pvr::lib::getLibFunctionChecked<void*>(lib, "glStencilFuncSeparate");
		FunctionTable[Glsc2FuncName::StencilMask] = pvr::lib::getLibFunctionChecked<void*>(lib, "glStencilMask");
		FunctionTable[Glsc2FuncName::StencilMaskSeparate] = pvr::lib::getLibFunctionChecked<void*>(lib, "glStencilMaskSeparate");
		FunctionTable[Glsc2FuncName::StencilOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "glStencilOp");
		FunctionTable[Glsc2FuncName::StencilOpSeparate] = pvr::lib::getLibFunctionChecked<void*>(lib, "glStencilOpSeparate");
//		FunctionTable[Glsc2FuncName::TexImage2D] = pvr::lib::getLibFunctionChecked<void*>(lib, "glTexImage2D");
		FunctionTable[Glsc2FuncName::TexParameterf] = pvr::lib::getLibFunctionChecked<void*>(lib, "glTexParameterf");
		FunctionTable[Glsc2FuncName::TexParameterfv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glTexParameterfv");
		FunctionTable[Glsc2FuncName::TexParameteri] = pvr::lib::getLibFunctionChecked<void*>(lib, "glTexParameteri");
		FunctionTable[Glsc2FuncName::TexParameteriv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glTexParameteriv");
		FunctionTable[Glsc2FuncName::TexSubImage2D] = pvr::lib::getLibFunctionChecked<void*>(lib, "glTexSubImage2D");
		FunctionTable[Glsc2FuncName::Uniform1f] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform1f");
		FunctionTable[Glsc2FuncName::Uniform1fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform1fv");
		FunctionTable[Glsc2FuncName::Uniform1i] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform1i");
		FunctionTable[Glsc2FuncName::Uniform1iv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform1iv");
		FunctionTable[Glsc2FuncName::Uniform2f] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform2f");
		FunctionTable[Glsc2FuncName::Uniform2fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform2fv");
		FunctionTable[Glsc2FuncName::Uniform2i] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform2i");
		FunctionTable[Glsc2FuncName::Uniform2iv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform2iv");
		FunctionTable[Glsc2FuncName::Uniform3f] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform3f");
		FunctionTable[Glsc2FuncName::Uniform3fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform3fv");
		FunctionTable[Glsc2FuncName::Uniform3i] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform3i");
		FunctionTable[Glsc2FuncName::Uniform3iv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform3iv");
		FunctionTable[Glsc2FuncName::Uniform4f] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform4f");
		FunctionTable[Glsc2FuncName::Uniform4fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform4fv");
		FunctionTable[Glsc2FuncName::Uniform4i] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform4i");
		FunctionTable[Glsc2FuncName::Uniform4iv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform4iv");
		FunctionTable[Glsc2FuncName::UniformMatrix2fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniformMatrix2fv");
		FunctionTable[Glsc2FuncName::UniformMatrix3fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniformMatrix3fv");
		FunctionTable[Glsc2FuncName::UniformMatrix4fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniformMatrix4fv");
		FunctionTable[Glsc2FuncName::UseProgram] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUseProgram");
//		FunctionTable[Glsc2FuncName::ValidateProgram] = pvr::lib::getLibFunctionChecked<void*>(lib, "glValidateProgram");
		FunctionTable[Glsc2FuncName::VertexAttrib1f] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttrib1f");
		FunctionTable[Glsc2FuncName::VertexAttrib1fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttrib1fv");
		FunctionTable[Glsc2FuncName::VertexAttrib2f] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttrib2f");
		FunctionTable[Glsc2FuncName::VertexAttrib2fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttrib2fv");
		FunctionTable[Glsc2FuncName::VertexAttrib3f] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttrib3f");
		FunctionTable[Glsc2FuncName::VertexAttrib3fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttrib3fv");
		FunctionTable[Glsc2FuncName::VertexAttrib4f] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttrib4f");
		FunctionTable[Glsc2FuncName::VertexAttrib4fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttrib4fv");
		FunctionTable[Glsc2FuncName::VertexAttribPointer] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttribPointer");
		FunctionTable[Glsc2FuncName::Viewport] = pvr::lib::getLibFunctionChecked<void*>(lib, "glViewport");

		// GLSC functions not originally in base GLES2
		FunctionTable[Glsc2FuncName::TexStorage2D] = pvr::lib::getLibFunctionChecked<void*>(lib, "glTexStorage2D");
		FunctionTable[Glsc2FuncName::DrawRangeElements] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDrawRangeElements");
		FunctionTable[Glsc2FuncName::ProgramBinary] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramBinary");
		FunctionTable[Glsc2FuncName::ReadnPixels] = pvr::lib::getLibFunctionChecked<void*>(lib, "glReadnPixels");

		// TODO
		// GetGraphicsResetStatus
		// GetnUniformfv,
		// GetnUniformiv,
		// TexStorage2D
	}
#endif
	return FunctionTable[funcname];
}

} // namespace internals
} // namespace gl

// TODO: trim this list to SC only functions, and add those that aren't in base ES2
typedef void (GL_APIENTRYP PFNGLACTIVETEXTUREPROC) (GLenum texture);
typedef void (GL_APIENTRYP PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (GL_APIENTRYP PFNGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const GLchar *name);
typedef void (GL_APIENTRYP PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (GL_APIENTRYP PFNGLBINDFRAMEBUFFERPROC) (GLenum target, GLuint framebuffer);
typedef void (GL_APIENTRYP PFNGLBINDRENDERBUFFERPROC) (GLenum target, GLuint renderbuffer);
typedef void (GL_APIENTRYP PFNGLBINDTEXTUREPROC) (GLenum target, GLuint texture);
typedef void (GL_APIENTRYP PFNGLBLENDCOLORPROC) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (GL_APIENTRYP PFNGLBLENDEQUATIONPROC) (GLenum mode);
typedef void (GL_APIENTRYP PFNGLBLENDEQUATIONSEPARATEPROC) (GLenum modeRGB, GLenum modeAlpha);
typedef void (GL_APIENTRYP PFNGLBLENDFUNCPROC) (GLenum sfactor, GLenum dfactor);
typedef void (GL_APIENTRYP PFNGLBLENDFUNCSEPARATEPROC) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef void (GL_APIENTRYP PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void (GL_APIENTRYP PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
typedef GLenum(GL_APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSPROC) (GLenum target);
typedef void (GL_APIENTRYP PFNGLCLEARPROC) (GLbitfield mask);
typedef void (GL_APIENTRYP PFNGLCLEARCOLORPROC) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (GL_APIENTRYP PFNGLCLEARDEPTHFPROC) (GLfloat d);
typedef void (GL_APIENTRYP PFNGLCLEARSTENCILPROC) (GLint s);
typedef void (GL_APIENTRYP PFNGLCOLORMASKPROC) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
typedef void (GL_APIENTRYP PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
typedef void (GL_APIENTRYP PFNGLCOPYTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef void (GL_APIENTRYP PFNGLCOPYTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef GLuint(GL_APIENTRYP PFNGLCREATEPROGRAMPROC) (void);
typedef GLuint(GL_APIENTRYP PFNGLCREATESHADERPROC) (GLenum type);
typedef void (GL_APIENTRYP PFNGLCULLFACEPROC) (GLenum mode);
typedef void (GL_APIENTRYP PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint *buffers);
typedef void (GL_APIENTRYP PFNGLDELETEFRAMEBUFFERSPROC) (GLsizei n, const GLuint *framebuffers);
typedef void (GL_APIENTRYP PFNGLDELETEPROGRAMPROC) (GLuint program);
typedef void (GL_APIENTRYP PFNGLDELETERENDERBUFFERSPROC) (GLsizei n, const GLuint *renderbuffers);
typedef void (GL_APIENTRYP PFNGLDELETESHADERPROC) (GLuint shader);
typedef void (GL_APIENTRYP PFNGLDELETETEXTURESPROC) (GLsizei n, const GLuint *textures);
typedef void (GL_APIENTRYP PFNGLDEPTHFUNCPROC) (GLenum func);
typedef void (GL_APIENTRYP PFNGLDEPTHMASKPROC) (GLboolean flag);
typedef void (GL_APIENTRYP PFNGLDEPTHRANGEFPROC) (GLfloat n, GLfloat f);
typedef void (GL_APIENTRYP PFNGLDETACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (GL_APIENTRYP PFNGLDISABLEPROC) (GLenum cap);
typedef void (GL_APIENTRYP PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (GL_APIENTRYP PFNGLDRAWARRAYSPROC) (GLenum mode, GLint first, GLsizei count);
typedef void (GL_APIENTRYP PFNGLDRAWELEMENTSPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices);
typedef void (GL_APIENTRYP PFNGLENABLEPROC) (GLenum cap);
typedef void (GL_APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (GL_APIENTRYP PFNGLFINISHPROC) (void);
typedef void (GL_APIENTRYP PFNGLFLUSHPROC) (void);
typedef void (GL_APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFERPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (GL_APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (GL_APIENTRYP PFNGLFRONTFACEPROC) (GLenum mode);
typedef void (GL_APIENTRYP PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
typedef void (GL_APIENTRYP PFNGLGENERATEMIPMAPPROC) (GLenum target);
typedef void (GL_APIENTRYP PFNGLGENFRAMEBUFFERSPROC) (GLsizei n, GLuint *framebuffers);
typedef void (GL_APIENTRYP PFNGLGENRENDERBUFFERSPROC) (GLsizei n, GLuint *renderbuffers);
typedef void (GL_APIENTRYP PFNGLGENTEXTURESPROC) (GLsizei n, GLuint *textures);
typedef void (GL_APIENTRYP PFNGLGETACTIVEATTRIBPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
typedef void (GL_APIENTRYP PFNGLGETACTIVEUNIFORMPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
typedef void (GL_APIENTRYP PFNGLGETATTACHEDSHADERSPROC) (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
typedef GLint(GL_APIENTRYP PFNGLGETATTRIBLOCATIONPROC) (GLuint program, const GLchar *name);
typedef void (GL_APIENTRYP PFNGLGETBOOLEANVPROC) (GLenum pname, GLboolean *data);
typedef void (GL_APIENTRYP PFNGLGETBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef GLenum(GL_APIENTRYP PFNGLGETERRORPROC) (void);
typedef void (GL_APIENTRYP PFNGLGETFLOATVPROC) (GLenum pname, GLfloat *data);
typedef void (GL_APIENTRYP PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) (GLenum target, GLenum attachment, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETINTEGERVPROC) (GLenum pname, GLint *data);
typedef void (GL_APIENTRYP PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (GL_APIENTRYP PFNGLGETRENDERBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (GL_APIENTRYP PFNGLGETSHADERPRECISIONFORMATPROC) (GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
typedef void (GL_APIENTRYP PFNGLGETSHADERSOURCEPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
typedef const GLubyte *(GL_APIENTRYP PFNGLGETSTRINGPROC) (GLenum name);
typedef void (GL_APIENTRYP PFNGLGETTEXPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (GL_APIENTRYP PFNGLGETTEXPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETUNIFORMFVPROC) (GLuint program, GLint location, GLfloat *params);
typedef void (GL_APIENTRYP PFNGLGETUNIFORMIVPROC) (GLuint program, GLint location, GLint *params);
typedef GLint(GL_APIENTRYP PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar *name);
typedef void (GL_APIENTRYP PFNGLGETVERTEXATTRIBFVPROC) (GLuint index, GLenum pname, GLfloat *params);
typedef void (GL_APIENTRYP PFNGLGETVERTEXATTRIBIVPROC) (GLuint index, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETVERTEXATTRIBPOINTERVPROC) (GLuint index, GLenum pname, void **pointer);
typedef void (GL_APIENTRYP PFNGLHINTPROC) (GLenum target, GLenum mode);
typedef GLboolean(GL_APIENTRYP PFNGLISBUFFERPROC) (GLuint buffer);
typedef GLboolean(GL_APIENTRYP PFNGLISENABLEDPROC) (GLenum cap);
typedef GLboolean(GL_APIENTRYP PFNGLISFRAMEBUFFERPROC) (GLuint framebuffer);
typedef GLboolean(GL_APIENTRYP PFNGLISPROGRAMPROC) (GLuint program);
typedef GLboolean(GL_APIENTRYP PFNGLISRENDERBUFFERPROC) (GLuint renderbuffer);
typedef GLboolean(GL_APIENTRYP PFNGLISSHADERPROC) (GLuint shader);
typedef GLboolean(GL_APIENTRYP PFNGLISTEXTUREPROC) (GLuint texture);
typedef void (GL_APIENTRYP PFNGLLINEWIDTHPROC) (GLfloat width);
typedef void (GL_APIENTRYP PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void (GL_APIENTRYP PFNGLPIXELSTOREIPROC) (GLenum pname, GLint param);
typedef void (GL_APIENTRYP PFNGLPOLYGONOFFSETPROC) (GLfloat factor, GLfloat units);
typedef void (GL_APIENTRYP PFNGLREADPIXELSPROC) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
typedef void (GL_APIENTRYP PFNGLRELEASESHADERCOMPILERPROC) (void);
typedef void (GL_APIENTRYP PFNGLRENDERBUFFERSTORAGEPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (GL_APIENTRYP PFNGLSAMPLECOVERAGEPROC) (GLfloat value, GLboolean invert);
typedef void (GL_APIENTRYP PFNGLSCISSORPROC) (GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GL_APIENTRYP PFNGLSHADERBINARYPROC) (GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length);
typedef void (GL_APIENTRYP PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void (GL_APIENTRYP PFNGLSTENCILFUNCPROC) (GLenum func, GLint ref, GLuint mask);
typedef void (GL_APIENTRYP PFNGLSTENCILFUNCSEPARATEPROC) (GLenum face, GLenum func, GLint ref, GLuint mask);
typedef void (GL_APIENTRYP PFNGLSTENCILMASKPROC) (GLuint mask);
typedef void (GL_APIENTRYP PFNGLSTENCILMASKSEPARATEPROC) (GLenum face, GLuint mask);
typedef void (GL_APIENTRYP PFNGLSTENCILOPPROC) (GLenum fail, GLenum zfail, GLenum zpass);
typedef void (GL_APIENTRYP PFNGLSTENCILOPSEPARATEPROC) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
typedef void (GL_APIENTRYP PFNGLTEXIMAGE2DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void (GL_APIENTRYP PFNGLTEXPARAMETERFPROC) (GLenum target, GLenum pname, GLfloat param);
typedef void (GL_APIENTRYP PFNGLTEXPARAMETERFVPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (GL_APIENTRYP PFNGLTEXPARAMETERIPROC) (GLenum target, GLenum pname, GLint param);
typedef void (GL_APIENTRYP PFNGLTEXPARAMETERIVPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void (GL_APIENTRYP PFNGLTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
typedef void (GL_APIENTRYP PFNGLUNIFORM1FPROC) (GLint location, GLfloat v0);
typedef void (GL_APIENTRYP PFNGLUNIFORM1FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
typedef void (GL_APIENTRYP PFNGLUNIFORM1IVPROC) (GLint location, GLsizei count, const GLint *value);
typedef void (GL_APIENTRYP PFNGLUNIFORM2FPROC) (GLint location, GLfloat v0, GLfloat v1);
typedef void (GL_APIENTRYP PFNGLUNIFORM2FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLUNIFORM2IPROC) (GLint location, GLint v0, GLint v1);
typedef void (GL_APIENTRYP PFNGLUNIFORM2IVPROC) (GLint location, GLsizei count, const GLint *value);
typedef void (GL_APIENTRYP PFNGLUNIFORM3FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (GL_APIENTRYP PFNGLUNIFORM3FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLUNIFORM3IPROC) (GLint location, GLint v0, GLint v1, GLint v2);
typedef void (GL_APIENTRYP PFNGLUNIFORM3IVPROC) (GLint location, GLsizei count, const GLint *value);
typedef void (GL_APIENTRYP PFNGLUNIFORM4FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (GL_APIENTRYP PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLUNIFORM4IPROC) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void (GL_APIENTRYP PFNGLUNIFORM4IVPROC) (GLint location, GLsizei count, const GLint *value);
typedef void (GL_APIENTRYP PFNGLUNIFORMMATRIX2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLUNIFORMMATRIX3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLUSEPROGRAMPROC) (GLuint program);
typedef void (GL_APIENTRYP PFNGLVALIDATEPROGRAMPROC) (GLuint program);
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIB1FPROC) (GLuint index, GLfloat x);
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIB1FVPROC) (GLuint index, const GLfloat *v);
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIB2FPROC) (GLuint index, GLfloat x, GLfloat y);
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIB2FVPROC) (GLuint index, const GLfloat *v);
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIB3FPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIB3FVPROC) (GLuint index, const GLfloat *v);
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIB4FPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIB4FVPROC) (GLuint index, const GLfloat *v);
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void (GL_APIENTRYP PFNGLVIEWPORTPROC) (GLint x, GLint y, GLsizei width, GLsizei height);

// GLSC2 functions not originally in GLES2
typedef void (GL_APIENTRYP PFNGLTEXSTORAGE2DPROC) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (GL_APIENTRYP PFNGLDRAWRANGEELEMENTSPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
typedef void (GL_APIENTRYP PFNGLPROGRAMBINARYPROC) (GLuint program, GLenum binaryFormat, const void *binary, GLsizei length);
typedef void (GL_APIENTRYP PFNGLREADNPIXELSPROC) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);

#ifndef DYNAMICGLES_NO_NAMESPACE
namespace gl {
#elif TARGET_OS_IPHONE
namespace gl {
	namespace internals {
#endif

		inline void DYNAMICGLES_FUNCTION(ActiveTexture)(GLenum texture)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glActiveTexture) PFNGLACTIVETEXTUREPROC;
#endif
			PFNGLACTIVETEXTUREPROC _ActiveTexture = (PFNGLACTIVETEXTUREPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::ActiveTexture);
			return _ActiveTexture(texture);
		}


		inline void DYNAMICGLES_FUNCTION(BindBuffer)(GLenum target, GLuint buffer)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glBindBuffer) PFNGLBINDBUFFERPROC;
#endif
			PFNGLBINDBUFFERPROC _BindBuffer = (PFNGLBINDBUFFERPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::BindBuffer);
			return _BindBuffer(target, buffer);
		}
		inline void DYNAMICGLES_FUNCTION(BindFramebuffer)(GLenum target, GLuint framebuffer)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glBindFramebuffer) PFNGLBINDFRAMEBUFFERPROC;
#endif
			PFNGLBINDFRAMEBUFFERPROC _BindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::BindFramebuffer);
			return _BindFramebuffer(target, framebuffer);
		}
		inline void DYNAMICGLES_FUNCTION(BindRenderbuffer)(GLenum target, GLuint renderbuffer)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glBindRenderbuffer) PFNGLBINDRENDERBUFFERPROC;
#endif
			PFNGLBINDRENDERBUFFERPROC _BindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::BindRenderbuffer);
			return _BindRenderbuffer(target, renderbuffer);
		}
		inline void DYNAMICGLES_FUNCTION(BindTexture)(GLenum target, GLuint texture)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glBindTexture) PFNGLBINDTEXTUREPROC;
#endif
			PFNGLBINDTEXTUREPROC _BindTexture = (PFNGLBINDTEXTUREPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::BindTexture);
			return _BindTexture(target, texture);
		}
		inline void DYNAMICGLES_FUNCTION(BlendColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glBlendColor) PFNGLBLENDCOLORPROC;
#endif
			PFNGLBLENDCOLORPROC _BlendColor = (PFNGLBLENDCOLORPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::BlendColor);
			return _BlendColor(red, green, blue, alpha);
		}
		inline void DYNAMICGLES_FUNCTION(BlendEquation)(GLenum mode)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glBlendEquation) PFNGLBLENDEQUATIONPROC;
#endif
			PFNGLBLENDEQUATIONPROC _BlendEquation = (PFNGLBLENDEQUATIONPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::BlendEquation);
			return _BlendEquation(mode);
		}
		inline void DYNAMICGLES_FUNCTION(BlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glBlendEquationSeparate) PFNGLBLENDEQUATIONSEPARATEPROC;
#endif
			PFNGLBLENDEQUATIONSEPARATEPROC _BlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::BlendEquationSeparate);
			return _BlendEquationSeparate(modeRGB, modeAlpha);
		}
		inline void DYNAMICGLES_FUNCTION(BlendFunc)(GLenum sfactor, GLenum dfactor)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glBlendFunc) PFNGLBLENDFUNCPROC;
#endif
			PFNGLBLENDFUNCPROC _BlendFunc = (PFNGLBLENDFUNCPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::BlendFunc);
			return _BlendFunc(sfactor, dfactor);
		}
		inline void DYNAMICGLES_FUNCTION(BlendFuncSeparate)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glBlendFuncSeparate) PFNGLBLENDFUNCSEPARATEPROC;
#endif
			PFNGLBLENDFUNCSEPARATEPROC _BlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::BlendFuncSeparate);
			return _BlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
		}
		inline void DYNAMICGLES_FUNCTION(BufferData)(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glBufferData) PFNGLBUFFERDATAPROC;
#endif
			PFNGLBUFFERDATAPROC _BufferData = (PFNGLBUFFERDATAPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::BufferData);
			return _BufferData(target, size, data, usage);
		}
		inline void DYNAMICGLES_FUNCTION(BufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glBufferSubData) PFNGLBUFFERSUBDATAPROC;
#endif
			PFNGLBUFFERSUBDATAPROC _BufferSubData = (PFNGLBUFFERSUBDATAPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::BufferSubData);
			return _BufferSubData(target, offset, size, data);
		}
		inline GLenum DYNAMICGLES_FUNCTION(CheckFramebufferStatus)(GLenum target)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glCheckFramebufferStatus) PFNGLCHECKFRAMEBUFFERSTATUSPROC;
#endif
			PFNGLCHECKFRAMEBUFFERSTATUSPROC _CheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::CheckFramebufferStatus);
			return _CheckFramebufferStatus(target);
		}
		inline void DYNAMICGLES_FUNCTION(Clear)(GLbitfield mask)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glClear) PFNGLCLEARPROC;
#endif
			PFNGLCLEARPROC _Clear = (PFNGLCLEARPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::Clear);
			return _Clear(mask);
		}
		inline void DYNAMICGLES_FUNCTION(ClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glClearColor) PFNGLCLEARCOLORPROC;
#endif
			PFNGLCLEARCOLORPROC _ClearColor = (PFNGLCLEARCOLORPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::ClearColor);
			return _ClearColor(red, green, blue, alpha);
		}
		inline void DYNAMICGLES_FUNCTION(ClearDepthf)(GLfloat d)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glClearDepthf) PFNGLCLEARDEPTHFPROC;
#endif
			PFNGLCLEARDEPTHFPROC _ClearDepthf = (PFNGLCLEARDEPTHFPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::ClearDepthf);
			return _ClearDepthf(d);
		}
		inline void DYNAMICGLES_FUNCTION(ClearStencil)(GLint s)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glClearStencil) PFNGLCLEARSTENCILPROC;
#endif
			PFNGLCLEARSTENCILPROC _ClearStencil = (PFNGLCLEARSTENCILPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::ClearStencil);
			return _ClearStencil(s);
		}
		inline void DYNAMICGLES_FUNCTION(ColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glColorMask) PFNGLCOLORMASKPROC;
#endif
			PFNGLCOLORMASKPROC _ColorMask = (PFNGLCOLORMASKPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::ColorMask);
			return _ColorMask(red, green, blue, alpha);
		}

		inline void DYNAMICGLES_FUNCTION(CompressedTexSubImage2D)(
			GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glCompressedTexSubImage2D) PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC;
#endif
			PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC _CompressedTexSubImage2D = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::CompressedTexSubImage2D);
			return _CompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
		}

		inline GLenum DYNAMICGLES_FUNCTION(CreateProgram)()
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glCreateProgram) PFNGLCREATEPROGRAMPROC;
#endif
			PFNGLCREATEPROGRAMPROC _CreateProgram = (PFNGLCREATEPROGRAMPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::CreateProgram);
			return _CreateProgram();
		}

		inline void DYNAMICGLES_FUNCTION(CullFace)(GLenum mode)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glCullFace) PFNGLCULLFACEPROC;
#endif
			PFNGLCULLFACEPROC _CullFace = (PFNGLCULLFACEPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::CullFace);
			return _CullFace(mode);
		}
		inline void DYNAMICGLES_FUNCTION(DepthFunc)(GLenum func)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glDepthFunc) PFNGLDEPTHFUNCPROC;
#endif
			PFNGLDEPTHFUNCPROC _DepthFunc = (PFNGLDEPTHFUNCPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::DepthFunc);
			return _DepthFunc(func);
		}
		inline void DYNAMICGLES_FUNCTION(DepthMask)(GLboolean flag)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glDepthMask) PFNGLDEPTHMASKPROC;
#endif
			PFNGLDEPTHMASKPROC _DepthMask = (PFNGLDEPTHMASKPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::DepthMask);
			return _DepthMask(flag);
		}
		inline void DYNAMICGLES_FUNCTION(DepthRangef)(GLfloat n, GLfloat f)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glDepthRangef) PFNGLDEPTHRANGEFPROC;
#endif
			PFNGLDEPTHRANGEFPROC _DepthRangef = (PFNGLDEPTHRANGEFPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::DepthRangef);
			return _DepthRangef(n, f);
		}

		inline void DYNAMICGLES_FUNCTION(Disable)(GLenum cap)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glDisable) PFNGLDISABLEPROC;
#endif
			PFNGLDISABLEPROC _Disable = (PFNGLDISABLEPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::Disable);
			return _Disable(cap);
		}
		inline void DYNAMICGLES_FUNCTION(DisableVertexAttribArray)(GLuint index)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glDisableVertexAttribArray) PFNGLDISABLEVERTEXATTRIBARRAYPROC;
#endif
			PFNGLDISABLEVERTEXATTRIBARRAYPROC _DisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::DisableVertexAttribArray);
			return _DisableVertexAttribArray(index);
		}
		inline void DYNAMICGLES_FUNCTION(DrawArrays)(GLenum mode, GLint first, GLsizei count)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glDrawArrays) PFNGLDRAWARRAYSPROC;
#endif
			PFNGLDRAWARRAYSPROC _DrawArrays = (PFNGLDRAWARRAYSPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::DrawArrays);
			return _DrawArrays(mode, first, count);
		}

		inline void DYNAMICGLES_FUNCTION(Enable)(GLenum cap)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glEnable) PFNGLENABLEPROC;
#endif
			PFNGLENABLEPROC _Enable = (PFNGLENABLEPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::Enable);
			return _Enable(cap);
		}
		inline void DYNAMICGLES_FUNCTION(EnableVertexAttribArray)(GLuint index)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glEnableVertexAttribArray) PFNGLENABLEVERTEXATTRIBARRAYPROC;
#endif
			PFNGLENABLEVERTEXATTRIBARRAYPROC _EnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::EnableVertexAttribArray);
			return _EnableVertexAttribArray(index);
		}
		inline void DYNAMICGLES_FUNCTION(Finish)(void)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glFinish) PFNGLFINISHPROC;
#endif
			PFNGLFINISHPROC _Finish = (PFNGLFINISHPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::Finish);
			return _Finish();
		}
		inline void DYNAMICGLES_FUNCTION(Flush)(void)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glFlush) PFNGLFLUSHPROC;
#endif
			PFNGLFLUSHPROC _Flush = (PFNGLFLUSHPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::Flush);
			return _Flush();
		}
		inline void DYNAMICGLES_FUNCTION(FramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glFramebufferRenderbuffer) PFNGLFRAMEBUFFERRENDERBUFFERPROC;
#endif
			PFNGLFRAMEBUFFERRENDERBUFFERPROC _FramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::FramebufferRenderbuffer);
			return _FramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
		}
		inline void DYNAMICGLES_FUNCTION(FramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glFramebufferTexture2D) PFNGLFRAMEBUFFERTEXTURE2DPROC;
#endif
			PFNGLFRAMEBUFFERTEXTURE2DPROC _FramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::FramebufferTexture2D);
			return _FramebufferTexture2D(target, attachment, textarget, texture, level);
		}
		inline void DYNAMICGLES_FUNCTION(FrontFace)(GLenum mode)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glFrontFace) PFNGLFRONTFACEPROC;
#endif
			PFNGLFRONTFACEPROC _FrontFace = (PFNGLFRONTFACEPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::FrontFace);
			return _FrontFace(mode);
		}
		inline void DYNAMICGLES_FUNCTION(GenBuffers)(GLsizei n, GLuint* buffers)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glGenBuffers) PFNGLGENBUFFERSPROC;
#endif
			PFNGLGENBUFFERSPROC _GenBuffers = (PFNGLGENBUFFERSPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::GenBuffers);
			return _GenBuffers(n, buffers);
		}
		inline void DYNAMICGLES_FUNCTION(GenerateMipmap)(GLenum target)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glGenerateMipmap) PFNGLGENERATEMIPMAPPROC;
#endif
			PFNGLGENERATEMIPMAPPROC _GenerateMipmap = (PFNGLGENERATEMIPMAPPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::GenerateMipmap);
			return _GenerateMipmap(target);
		}
		inline void DYNAMICGLES_FUNCTION(GenFramebuffers)(GLsizei n, GLuint* framebuffers)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glGenFramebuffers) PFNGLGENFRAMEBUFFERSPROC;
#endif
			PFNGLGENFRAMEBUFFERSPROC _GenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::GenFramebuffers);
			return _GenFramebuffers(n, framebuffers);
		}
		inline void DYNAMICGLES_FUNCTION(GenRenderbuffers)(GLsizei n, GLuint* renderbuffers)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glGenRenderbuffers) PFNGLGENRENDERBUFFERSPROC;
#endif
			PFNGLGENRENDERBUFFERSPROC _GenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::GenRenderbuffers);
			return _GenRenderbuffers(n, renderbuffers);
		}
		inline void DYNAMICGLES_FUNCTION(GenTextures)(GLsizei n, GLuint* textures)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glGenTextures) PFNGLGENTEXTURESPROC;
#endif
			PFNGLGENTEXTURESPROC _GenTextures = (PFNGLGENTEXTURESPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::GenTextures);
			return _GenTextures(n, textures);
		}

		inline GLint DYNAMICGLES_FUNCTION(GetAttribLocation)(GLuint program, const GLchar* name)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glGetAttribLocation) PFNGLGETATTRIBLOCATIONPROC;
#endif
			PFNGLGETATTRIBLOCATIONPROC _GetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::GetAttribLocation);
			return _GetAttribLocation(program, name);
		}
		inline void DYNAMICGLES_FUNCTION(GetBooleanv)(GLenum pname, GLboolean* data)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glGetBooleanv) PFNGLGETBOOLEANVPROC;
#endif
			PFNGLGETBOOLEANVPROC _GetBooleanv = (PFNGLGETBOOLEANVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::GetBooleanv);
			return _GetBooleanv(pname, data);
		}
		inline void DYNAMICGLES_FUNCTION(GetBufferParameteriv)(GLenum target, GLenum pname, GLint* params)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glGetBufferParameteriv) PFNGLGETBUFFERPARAMETERIVPROC;
#endif
			PFNGLGETBUFFERPARAMETERIVPROC _GetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::GetBufferParameteriv);
			return _GetBufferParameteriv(target, pname, params);
		}
		inline GLenum DYNAMICGLES_FUNCTION(GetError)(void)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glGetError) PFNGLGETERRORPROC;
#endif
			PFNGLGETERRORPROC _GetError = (PFNGLGETERRORPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::GetError);
			return _GetError();
		}
		inline void DYNAMICGLES_FUNCTION(GetFloatv)(GLenum pname, GLfloat* data)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glGetFloatv) PFNGLGETFLOATVPROC;
#endif
			PFNGLGETFLOATVPROC _GetFloatv = (PFNGLGETFLOATVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::GetFloatv);
			return _GetFloatv(pname, data);
		}
		inline void DYNAMICGLES_FUNCTION(GetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint* params)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glGetFramebufferAttachmentParameteriv) PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC;
#endif
			PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC _GetFramebufferAttachmentParameteriv =
				(PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::GetFramebufferAttachmentParameteriv);
			return _GetFramebufferAttachmentParameteriv(target, attachment, pname, params);
		}
		inline void DYNAMICGLES_FUNCTION(GetIntegerv)(GLenum pname, GLint* data)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glGetIntegerv) PFNGLGETINTEGERVPROC;
#endif
			PFNGLGETINTEGERVPROC _GetIntegerv = (PFNGLGETINTEGERVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::GetIntegerv);
			return _GetIntegerv(pname, data);
		}
		inline void DYNAMICGLES_FUNCTION(GetProgramiv)(GLuint program, GLenum pname, GLint* params)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glGetProgramiv) PFNGLGETPROGRAMIVPROC;
#endif
			PFNGLGETPROGRAMIVPROC _GetProgramiv = (PFNGLGETPROGRAMIVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::GetProgramiv);
			return _GetProgramiv(program, pname, params);
		}

		inline void DYNAMICGLES_FUNCTION(GetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint* params)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glGetRenderbufferParameteriv) PFNGLGETRENDERBUFFERPARAMETERIVPROC;
#endif
			PFNGLGETRENDERBUFFERPARAMETERIVPROC _GetRenderbufferParameteriv =
				(PFNGLGETRENDERBUFFERPARAMETERIVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::GetRenderbufferParameteriv);
			return _GetRenderbufferParameteriv(target, pname, params);
		}

		inline const GLubyte* DYNAMICGLES_FUNCTION(GetString)(GLenum name)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glGetString) PFNGLGETSTRINGPROC;
#endif
			PFNGLGETSTRINGPROC _GetString = (PFNGLGETSTRINGPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::GetString);
			return _GetString(name);
		}
		inline void DYNAMICGLES_FUNCTION(GetTexParameterfv)(GLenum target, GLenum pname, GLfloat* params)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glGetTexParameterfv) PFNGLGETTEXPARAMETERFVPROC;
#endif
			PFNGLGETTEXPARAMETERFVPROC _GetTexParameterfv = (PFNGLGETTEXPARAMETERFVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::GetTexParameterfv);
			return _GetTexParameterfv(target, pname, params);
		}
		inline void DYNAMICGLES_FUNCTION(GetTexParameteriv)(GLenum target, GLenum pname, GLint* params)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glGetTexParameteriv) PFNGLGETTEXPARAMETERIVPROC;
#endif
			PFNGLGETTEXPARAMETERIVPROC _GetTexParameteriv = (PFNGLGETTEXPARAMETERIVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::GetTexParameteriv);
			return _GetTexParameteriv(target, pname, params);
		}

		inline GLint DYNAMICGLES_FUNCTION(GetUniformLocation)(GLuint program, const GLchar* name)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glGetUniformLocation) PFNGLGETUNIFORMLOCATIONPROC;
#endif
			PFNGLGETUNIFORMLOCATIONPROC _GetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::GetUniformLocation);
			return _GetUniformLocation(program, name);
		}
		inline void DYNAMICGLES_FUNCTION(GetVertexAttribfv)(GLuint index, GLenum pname, GLfloat* params)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glGetVertexAttribfv) PFNGLGETVERTEXATTRIBFVPROC;
#endif
			PFNGLGETVERTEXATTRIBFVPROC _GetVertexAttribfv = (PFNGLGETVERTEXATTRIBFVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::GetVertexAttribfv);
			return _GetVertexAttribfv(index, pname, params);
		}
		inline void DYNAMICGLES_FUNCTION(GetVertexAttribiv)(GLuint index, GLenum pname, GLint* params)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glGetVertexAttribiv) PFNGLGETVERTEXATTRIBIVPROC;
#endif
			PFNGLGETVERTEXATTRIBIVPROC _GetVertexAttribiv = (PFNGLGETVERTEXATTRIBIVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::GetVertexAttribiv);
			return _GetVertexAttribiv(index, pname, params);
		}
		inline void DYNAMICGLES_FUNCTION(GetVertexAttribPointerv)(GLuint index, GLenum pname, void** pointer)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glGetVertexAttribPointerv) PFNGLGETVERTEXATTRIBPOINTERVPROC;
#endif
			PFNGLGETVERTEXATTRIBPOINTERVPROC _GetVertexAttribPointerv = (PFNGLGETVERTEXATTRIBPOINTERVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::GetVertexAttribPointerv);
			return _GetVertexAttribPointerv(index, pname, pointer);
		}
		inline void DYNAMICGLES_FUNCTION(Hint)(GLenum target, GLenum mode)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glHint) PFNGLHINTPROC;
#endif
			PFNGLHINTPROC _Hint = (PFNGLHINTPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::Hint);
			return _Hint(target, mode);
		}

		inline GLboolean DYNAMICGLES_FUNCTION(IsEnabled)(GLenum cap)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glIsEnabled) PFNGLISENABLEDPROC;
#endif
			PFNGLISENABLEDPROC _IsEnabled = (PFNGLISENABLEDPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::IsEnabled);
			return _IsEnabled(cap);
		}
		inline void DYNAMICGLES_FUNCTION(LineWidth)(GLfloat width)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glLineWidth) PFNGLLINEWIDTHPROC;
#endif
			PFNGLLINEWIDTHPROC _LineWidth = (PFNGLLINEWIDTHPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::LineWidth);
			return _LineWidth(width);
		}
		inline void DYNAMICGLES_FUNCTION(PixelStorei)(GLenum pname, GLint param)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glPixelStorei) PFNGLPIXELSTOREIPROC;
#endif
			PFNGLPIXELSTOREIPROC _PixelStorei = (PFNGLPIXELSTOREIPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::PixelStorei);
			return _PixelStorei(pname, param);
		}
		inline void DYNAMICGLES_FUNCTION(PolygonOffset)(GLfloat factor, GLfloat units)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glPolygonOffset) PFNGLPOLYGONOFFSETPROC;
#endif
			PFNGLPOLYGONOFFSETPROC _PolygonOffset = (PFNGLPOLYGONOFFSETPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::PolygonOffset);
			return _PolygonOffset(factor, units);
		}
		inline void DYNAMICGLES_FUNCTION(RenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glRenderbufferStorage) PFNGLRENDERBUFFERSTORAGEPROC;
#endif
			PFNGLRENDERBUFFERSTORAGEPROC _RenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::RenderbufferStorage);
			return _RenderbufferStorage(target, internalformat, width, height);
		}
		inline void DYNAMICGLES_FUNCTION(SampleCoverage)(GLfloat value, GLboolean invert)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glSampleCoverage) PFNGLSAMPLECOVERAGEPROC;
#endif
			PFNGLSAMPLECOVERAGEPROC _SampleCoverage = (PFNGLSAMPLECOVERAGEPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::SampleCoverage);
			return _SampleCoverage(value, invert);
		}
		inline void DYNAMICGLES_FUNCTION(Scissor)(GLint x, GLint y, GLsizei width, GLsizei height)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glScissor) PFNGLSCISSORPROC;
#endif
			PFNGLSCISSORPROC _Scissor = (PFNGLSCISSORPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::Scissor);
			return _Scissor(x, y, width, height);
		}
		inline void DYNAMICGLES_FUNCTION(StencilFunc)(GLenum func, GLint ref, GLuint mask)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glStencilFunc) PFNGLSTENCILFUNCPROC;
#endif
			PFNGLSTENCILFUNCPROC _StencilFunc = (PFNGLSTENCILFUNCPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::StencilFunc);
			return _StencilFunc(func, ref, mask);
		}
		inline void DYNAMICGLES_FUNCTION(StencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glStencilFuncSeparate) PFNGLSTENCILFUNCSEPARATEPROC;
#endif
			PFNGLSTENCILFUNCSEPARATEPROC _StencilFuncSeparate = (PFNGLSTENCILFUNCSEPARATEPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::StencilFuncSeparate);
			return _StencilFuncSeparate(face, func, ref, mask);
		}
		inline void DYNAMICGLES_FUNCTION(StencilMask)(GLuint mask)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glStencilMask) PFNGLSTENCILMASKPROC;
#endif
			PFNGLSTENCILMASKPROC _StencilMask = (PFNGLSTENCILMASKPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::StencilMask);
			return _StencilMask(mask);
		}
		inline void DYNAMICGLES_FUNCTION(StencilMaskSeparate)(GLenum face, GLuint mask)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glStencilMaskSeparate) PFNGLSTENCILMASKSEPARATEPROC;
#endif
			PFNGLSTENCILMASKSEPARATEPROC _StencilMaskSeparate = (PFNGLSTENCILMASKSEPARATEPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::StencilMaskSeparate);
			return _StencilMaskSeparate(face, mask);
		}
		inline void DYNAMICGLES_FUNCTION(StencilOp)(GLenum fail, GLenum zfail, GLenum zpass)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glStencilOp) PFNGLSTENCILOPPROC;
#endif
			PFNGLSTENCILOPPROC _StencilOp = (PFNGLSTENCILOPPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::StencilOp);
			return _StencilOp(fail, zfail, zpass);
		}
		inline void DYNAMICGLES_FUNCTION(StencilOpSeparate)(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glStencilOpSeparate) PFNGLSTENCILOPSEPARATEPROC;
#endif
			PFNGLSTENCILOPSEPARATEPROC _StencilOpSeparate = (PFNGLSTENCILOPSEPARATEPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::StencilOpSeparate);
			return _StencilOpSeparate(face, sfail, dpfail, dppass);
		}
		inline void DYNAMICGLES_FUNCTION(TexParameterf)(GLenum target, GLenum pname, GLfloat param)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glTexParameterf) PFNGLTEXPARAMETERFPROC;
#endif
			PFNGLTEXPARAMETERFPROC _TexParameterf = (PFNGLTEXPARAMETERFPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::TexParameterf);
			return _TexParameterf(target, pname, param);
		}
		inline void DYNAMICGLES_FUNCTION(TexParameterfv)(GLenum target, GLenum pname, const GLfloat* params)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glTexParameterfv) PFNGLTEXPARAMETERFVPROC;
#endif
			PFNGLTEXPARAMETERFVPROC _TexParameterfv = (PFNGLTEXPARAMETERFVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::TexParameterfv);
			return _TexParameterfv(target, pname, params);
		}
		inline void DYNAMICGLES_FUNCTION(TexParameteri)(GLenum target, GLenum pname, GLint param)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glTexParameteri) PFNGLTEXPARAMETERIPROC;
#endif
			PFNGLTEXPARAMETERIPROC _TexParameteri = (PFNGLTEXPARAMETERIPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::TexParameteri);
			return _TexParameteri(target, pname, param);
		}
		inline void DYNAMICGLES_FUNCTION(TexParameteriv)(GLenum target, GLenum pname, const GLint* params)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glTexParameteriv) PFNGLTEXPARAMETERIVPROC;
#endif
			PFNGLTEXPARAMETERIVPROC _TexParameteriv = (PFNGLTEXPARAMETERIVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::TexParameteriv);
			return _TexParameteriv(target, pname, params);
		}
		inline void DYNAMICGLES_FUNCTION(TexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glTexSubImage2D) PFNGLTEXSUBIMAGE2DPROC;
#endif
			PFNGLTEXSUBIMAGE2DPROC _TexSubImage2D = (PFNGLTEXSUBIMAGE2DPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::TexSubImage2D);
			return _TexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
		}
		inline void DYNAMICGLES_FUNCTION(Uniform1f)(GLint location, GLfloat v0)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glUniform1f) PFNGLUNIFORM1FPROC;
#endif
			PFNGLUNIFORM1FPROC _Uniform1f = (PFNGLUNIFORM1FPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::Uniform1f);
			return _Uniform1f(location, v0);
		}
		inline void DYNAMICGLES_FUNCTION(Uniform1fv)(GLint location, GLsizei count, const GLfloat* value)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glUniform1fv) PFNGLUNIFORM1FVPROC;
#endif
			PFNGLUNIFORM1FVPROC _Uniform1fv = (PFNGLUNIFORM1FVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::Uniform1fv);
			return _Uniform1fv(location, count, value);
		}
		inline void DYNAMICGLES_FUNCTION(Uniform1i)(GLint location, GLint v0)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glUniform1i) PFNGLUNIFORM1IPROC;
#endif
			PFNGLUNIFORM1IPROC _Uniform1i = (PFNGLUNIFORM1IPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::Uniform1i);
			return _Uniform1i(location, v0);
		}
		inline void DYNAMICGLES_FUNCTION(Uniform1iv)(GLint location, GLsizei count, const GLint* value)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glUniform1iv) PFNGLUNIFORM1IVPROC;
#endif
			PFNGLUNIFORM1IVPROC _Uniform1iv = (PFNGLUNIFORM1IVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::Uniform1iv);
			return _Uniform1iv(location, count, value);
		}
		inline void DYNAMICGLES_FUNCTION(Uniform2f)(GLint location, GLfloat v0, GLfloat v1)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glUniform2f) PFNGLUNIFORM2FPROC;
#endif
			PFNGLUNIFORM2FPROC _Uniform2f = (PFNGLUNIFORM2FPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::Uniform2f);
			return _Uniform2f(location, v0, v1);
		}
		inline void DYNAMICGLES_FUNCTION(Uniform2fv)(GLint location, GLsizei count, const GLfloat* value)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glUniform2fv) PFNGLUNIFORM2FVPROC;
#endif
			PFNGLUNIFORM2FVPROC _Uniform2fv = (PFNGLUNIFORM2FVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::Uniform2fv);
			return _Uniform2fv(location, count, value);
		}
		inline void DYNAMICGLES_FUNCTION(Uniform2i)(GLint location, GLint v0, GLint v1)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glUniform2i) PFNGLUNIFORM2IPROC;
#endif
			PFNGLUNIFORM2IPROC _Uniform2i = (PFNGLUNIFORM2IPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::Uniform2i);
			return _Uniform2i(location, v0, v1);
		}
		inline void DYNAMICGLES_FUNCTION(Uniform2iv)(GLint location, GLsizei count, const GLint* value)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glUniform2iv) PFNGLUNIFORM2IVPROC;
#endif
			PFNGLUNIFORM2IVPROC _Uniform2iv = (PFNGLUNIFORM2IVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::Uniform2iv);
			return _Uniform2iv(location, count, value);
		}
		inline void DYNAMICGLES_FUNCTION(Uniform3f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glUniform3f) PFNGLUNIFORM3FPROC;
#endif
			PFNGLUNIFORM3FPROC _Uniform3f = (PFNGLUNIFORM3FPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::Uniform3f);
			return _Uniform3f(location, v0, v1, v2);
		}
		inline void DYNAMICGLES_FUNCTION(Uniform3fv)(GLint location, GLsizei count, const GLfloat* value)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glUniform3fv) PFNGLUNIFORM3FVPROC;
#endif
			PFNGLUNIFORM3FVPROC _Uniform3fv = (PFNGLUNIFORM3FVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::Uniform3fv);
			return _Uniform3fv(location, count, value);
		}
		inline void DYNAMICGLES_FUNCTION(Uniform3i)(GLint location, GLint v0, GLint v1, GLint v2)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glUniform3i) PFNGLUNIFORM3IPROC;
#endif
			PFNGLUNIFORM3IPROC _Uniform3i = (PFNGLUNIFORM3IPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::Uniform3i);
			return _Uniform3i(location, v0, v1, v2);
		}
		inline void DYNAMICGLES_FUNCTION(Uniform3iv)(GLint location, GLsizei count, const GLint* value)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glUniform3iv) PFNGLUNIFORM3IVPROC;
#endif
			PFNGLUNIFORM3IVPROC _Uniform3iv = (PFNGLUNIFORM3IVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::Uniform3iv);
			return _Uniform3iv(location, count, value);
		}
		inline void DYNAMICGLES_FUNCTION(Uniform4f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glUniform4f) PFNGLUNIFORM4FPROC;
#endif
			PFNGLUNIFORM4FPROC _Uniform4f = (PFNGLUNIFORM4FPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::Uniform4f);
			return _Uniform4f(location, v0, v1, v2, v3);
		}
		inline void DYNAMICGLES_FUNCTION(Uniform4fv)(GLint location, GLsizei count, const GLfloat* value)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glUniform4fv) PFNGLUNIFORM4FVPROC;
#endif
			PFNGLUNIFORM4FVPROC _Uniform4fv = (PFNGLUNIFORM4FVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::Uniform4fv);
			return _Uniform4fv(location, count, value);
		}
		inline void DYNAMICGLES_FUNCTION(Uniform4i)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glUniform4i) PFNGLUNIFORM4IPROC;
#endif
			PFNGLUNIFORM4IPROC _Uniform4i = (PFNGLUNIFORM4IPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::Uniform4i);
			return _Uniform4i(location, v0, v1, v2, v3);
		}
		inline void DYNAMICGLES_FUNCTION(Uniform4iv)(GLint location, GLsizei count, const GLint* value)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glUniform4iv) PFNGLUNIFORM4IVPROC;
#endif
			PFNGLUNIFORM4IVPROC _Uniform4iv = (PFNGLUNIFORM4IVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::Uniform4iv);
			return _Uniform4iv(location, count, value);
		}
		inline void DYNAMICGLES_FUNCTION(UniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glUniformMatrix2fv) PFNGLUNIFORMMATRIX2FVPROC;
#endif
			PFNGLUNIFORMMATRIX2FVPROC _UniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::UniformMatrix2fv);
			return _UniformMatrix2fv(location, count, transpose, value);
		}
		inline void DYNAMICGLES_FUNCTION(UniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glUniformMatrix3fv) PFNGLUNIFORMMATRIX3FVPROC;
#endif
			PFNGLUNIFORMMATRIX3FVPROC _UniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::UniformMatrix3fv);
			return _UniformMatrix3fv(location, count, transpose, value);
		}
		inline void DYNAMICGLES_FUNCTION(UniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glUniformMatrix4fv) PFNGLUNIFORMMATRIX4FVPROC;
#endif
			PFNGLUNIFORMMATRIX4FVPROC _UniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::UniformMatrix4fv);
			return _UniformMatrix4fv(location, count, transpose, value);
		}
		inline void DYNAMICGLES_FUNCTION(UseProgram)(GLuint program)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glUseProgram) PFNGLUSEPROGRAMPROC;
#endif
			PFNGLUSEPROGRAMPROC _UseProgram = (PFNGLUSEPROGRAMPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::UseProgram);
			return _UseProgram(program);
		}
		inline void DYNAMICGLES_FUNCTION(VertexAttrib1f)(GLuint index, GLfloat x)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glVertexAttrib1f) PFNGLVERTEXATTRIB1FPROC;
#endif
			PFNGLVERTEXATTRIB1FPROC _VertexAttrib1f = (PFNGLVERTEXATTRIB1FPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::VertexAttrib1f);
			return _VertexAttrib1f(index, x);
		}
		inline void DYNAMICGLES_FUNCTION(VertexAttrib1fv)(GLuint index, const GLfloat* v)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glVertexAttrib1fv) PFNGLVERTEXATTRIB1FVPROC;
#endif
			PFNGLVERTEXATTRIB1FVPROC _VertexAttrib1fv = (PFNGLVERTEXATTRIB1FVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::VertexAttrib1fv);
			return _VertexAttrib1fv(index, v);
		}
		inline void DYNAMICGLES_FUNCTION(VertexAttrib2f)(GLuint index, GLfloat x, GLfloat y)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glVertexAttrib2f) PFNGLVERTEXATTRIB2FPROC;
#endif
			PFNGLVERTEXATTRIB2FPROC _VertexAttrib2f = (PFNGLVERTEXATTRIB2FPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::VertexAttrib2f);
			return _VertexAttrib2f(index, x, y);
		}
		inline void DYNAMICGLES_FUNCTION(VertexAttrib2fv)(GLuint index, const GLfloat* v)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glVertexAttrib2fv) PFNGLVERTEXATTRIB2FVPROC;
#endif
			PFNGLVERTEXATTRIB2FVPROC _VertexAttrib2fv = (PFNGLVERTEXATTRIB2FVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::VertexAttrib2fv);
			return _VertexAttrib2fv(index, v);
		}
		inline void DYNAMICGLES_FUNCTION(VertexAttrib3f)(GLuint index, GLfloat x, GLfloat y, GLfloat z)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glVertexAttrib3f) PFNGLVERTEXATTRIB3FPROC;
#endif
			PFNGLVERTEXATTRIB3FPROC _VertexAttrib3f = (PFNGLVERTEXATTRIB3FPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::VertexAttrib3f);
			return _VertexAttrib3f(index, x, y, z);
		}
		inline void DYNAMICGLES_FUNCTION(VertexAttrib3fv)(GLuint index, const GLfloat* v)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glVertexAttrib3fv) PFNGLVERTEXATTRIB3FVPROC;
#endif
			PFNGLVERTEXATTRIB3FVPROC _VertexAttrib3fv = (PFNGLVERTEXATTRIB3FVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::VertexAttrib3fv);
			return _VertexAttrib3fv(index, v);
		}
		inline void DYNAMICGLES_FUNCTION(VertexAttrib4f)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glVertexAttrib4f) PFNGLVERTEXATTRIB4FPROC;
#endif
			PFNGLVERTEXATTRIB4FPROC _VertexAttrib4f = (PFNGLVERTEXATTRIB4FPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::VertexAttrib4f);
			return _VertexAttrib4f(index, x, y, z, w);
		}
		inline void DYNAMICGLES_FUNCTION(VertexAttrib4fv)(GLuint index, const GLfloat* v)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glVertexAttrib4fv) PFNGLVERTEXATTRIB4FVPROC;
#endif
			PFNGLVERTEXATTRIB4FVPROC _VertexAttrib4fv = (PFNGLVERTEXATTRIB4FVPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::VertexAttrib4fv);
			return _VertexAttrib4fv(index, v);
		}
		inline void DYNAMICGLES_FUNCTION(VertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glVertexAttribPointer) PFNGLVERTEXATTRIBPOINTERPROC;
#endif
			PFNGLVERTEXATTRIBPOINTERPROC _VertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::VertexAttribPointer);
			return _VertexAttribPointer(index, size, type, normalized, stride, pointer);
		}
		inline void DYNAMICGLES_FUNCTION(Viewport)(GLint x, GLint y, GLsizei width, GLsizei height)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glViewport) PFNGLVIEWPORTPROC;
#endif
			PFNGLVIEWPORTPROC _Viewport = (PFNGLVIEWPORTPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::Viewport);
			return _Viewport(x, y, width, height);
		}

		// GLSC2 Functions not originally in GLES2
		inline void DYNAMICGLES_FUNCTION(TexStorage2D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glTexStorage2D) PFNGLTEXSTORAGE2DPROC;
#endif
			PFNGLTEXSTORAGE2DPROC _TexStorage2D = (PFNGLTEXSTORAGE2DPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::TexStorage2D);
			return _TexStorage2D(target, levels, internalformat, width, height);
		}

		inline void DYNAMICGLES_FUNCTION(DrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glDrawRangeElements) PFNGLDRAWRANGEELEMENTSPROC;
#endif
			PFNGLDRAWRANGEELEMENTSPROC _DrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::DrawRangeElements);
			return _DrawRangeElements( mode,  start,  end,  count,  type, indices);
		}

		inline void DYNAMICGLES_FUNCTION(ProgramBinary)(GLuint program, GLenum binaryFormat, const void *binary, GLsizei length)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glProgramBinary) PFNGLPROGRAMBINARYPROC;
#endif
			PFNGLPROGRAMBINARYPROC _ProgramBinary = (PFNGLPROGRAMBINARYPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::ProgramBinary);
			return _ProgramBinary(program,  binaryFormat, binary, length);
		}

		inline void DYNAMICGLES_FUNCTION(ReadnPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data)
		{
#if TARGET_OS_IPHONE
			typedef decltype(&glReadnPixels) PFNGLREADNPIXELSPROC;
#endif
			PFNGLREADNPIXELSPROC _ReadnPixels = (PFNGLREADNPIXELSPROC)gl::internals::getSc20Function(gl::internals::Glsc2FuncName::ReadnPixels);
			return _ReadnPixels(x, y, width, height, format, type, bufSize, data);
		}

		// Empty functions for convenience when using GLES code with GLSC
		inline void DYNAMICGLES_FUNCTION(DeleteBuffers)(GLsizei n, const GLuint* buffers) { GLSC2_GLUE_UNUSED(n); GLSC2_GLUE_UNUSED(buffers); }
		inline void DYNAMICGLES_FUNCTION(DeleteFramebuffers)(GLsizei n, const GLuint *framebuffers) { GLSC2_GLUE_UNUSED(n); GLSC2_GLUE_UNUSED(framebuffers); }
		inline void DYNAMICGLES_FUNCTION(DeleteProgram)(GLuint program) { GLSC2_GLUE_UNUSED(program); }
		inline void DYNAMICGLES_FUNCTION(DeleteRenderbuffers)(GLsizei n, const GLuint *renderbuffers) { GLSC2_GLUE_UNUSED(n);  GLSC2_GLUE_UNUSED(renderbuffers); }
		inline void DYNAMICGLES_FUNCTION(DeleteShader)(GLuint shader) { GLSC2_GLUE_UNUSED(shader); }
		inline void DYNAMICGLES_FUNCTION(DeleteTextures)(GLsizei n, const GLuint *textures) { GLSC2_GLUE_UNUSED(n); GLSC2_GLUE_UNUSED(textures); }

		// Conversions for convenience when using GLES code with GLSC
		inline void DYNAMICGLES_FUNCTION(TexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *data)
		{
			//GLSC2_GLUE_UNUSED(internalFormat);
			GLSC2_GLUE_UNUSED(border);

			// Hack for ModernDashSC
			if (internalformat == GL_RGBA)
				internalformat = GL_RGBA8;
			//else if (internalformat == GL_RGB)
			//	internalformat = GL_RGB8;

			TexStorage2D(target, 1, internalformat, width, height);
			TexSubImage2D(target, level, 0, 0, width, height, format, type, data);
		}

		inline void DYNAMICGLES_FUNCTION(CompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data)
		{
			GLSC2_GLUE_UNUSED(border);
			CompressedTexSubImage2D(target, level, 0, 0, width, height, internalformat, imageSize, data);
		}

		// This doesn't work. Count isn't the correct thing to use for the "end" parameter.
		//inline void DYNAMICGLES_FUNCTION(DrawElements)(GLenum mode, GLsizei count, GLenum type, const void *indices)
		//{
		//	DrawRangeElements(mode, 0, count, count, type, indices);
		//}
		
		inline void DYNAMICGLES_FUNCTION(ReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *data)
		{
			// Calculate buffer size from dimensions, format, and type information.
			const GLuint numChannels = (format == GL_RGBA) ? 4 : 3;
			const GLuint typeSize = (type == GL_UNSIGNED_BYTE) ? 1 : 2;
			const GLsizei bufSize = width * height * numChannels * typeSize;
			ReadnPixels(x, y, width, height, format, type, bufSize, data);
		}
		
		// Handle extensions
		inline bool isGlExtensionSupported(const char* extensionName)
		{
			return false;
		}

#ifndef DYNAMICGLES_NO_NAMESPACE
	}
#elif TARGET_OS_IPHONE
	}
}
#endif
