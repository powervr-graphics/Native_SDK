/*!*********************************************************************************************************************
\file         PVRNativeApi\OGLES\ShaderUtils.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementations of the shader utility functions
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRNativeApi/ShaderUtils.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"
#include "PVRNativeApi/ApiErrors.h"

namespace pvr {
namespace utils {

bool loadShader(const native::HContext_& context, const Stream& shaderSource, types::ShaderType shaderType, const char* const* defines, uint32 defineCount,
                pvr::native::HShader_& outShader, const ApiCapabilities* contextCapabilities)
{
	if (!shaderSource.isopen() && !shaderSource.open())
	{
		Log(Log.Error, "loadShader: Could not open the shaderSource stream.");
		return false;
	}
	if (outShader.handle)
	{
		pvr::Log(Log.Warning,
		         "loadShader: Generated shader passed to loadShader. Deleting reference to avoid leaking a preexisting shader object.");
		gl::DeleteShader(outShader);
		outShader = 0;
	}
	string shaderSrc;
	if (!shaderSource.readIntoString(shaderSrc)) { return false; };

	switch (shaderType)
	{
	case types::ShaderType::VertexShader:
		outShader = gl::CreateShader(GL_VERTEX_SHADER);
		break;
	case types::ShaderType::FragmentShader:
		outShader = gl::CreateShader(GL_FRAGMENT_SHADER);
		break;
	case types::ShaderType::ComputeShader:
#if defined(GL_COMPUTE_SHADER)
		if (!contextCapabilities || contextCapabilities->supports(ApiCapabilities::ComputeShader))
		{
			outShader = gl::CreateShader(GL_COMPUTE_SHADER);
		}
		else
#endif
		{
			Log("loadShader: Compute Shader not supported on this context");
			return false;
		}
		break;
	case types::ShaderType::GeometryShader:
#if defined(GL_GEOMETRY_SHADER_EXT)
		if (!contextCapabilities || contextCapabilities->supports(ApiCapabilities::GeometryShader))
		{
			outShader = gl::CreateShader(GL_GEOMETRY_SHADER_EXT);
		}
		else
#endif
		{
			Log("loadShader: Geometry Shader not supported on this context");
			return false;
		}

		break;
	case types::ShaderType::TessControlShader:
#if defined(GL_TESS_CONTROL_SHADER_EXT)
		if (!contextCapabilities || contextCapabilities->supports(ApiCapabilities::Tessellation))
		{
			outShader = gl::CreateShader(GL_TESS_CONTROL_SHADER_EXT);
		}
		else
#endif
		{
			Log("loadShader: Tessellation not supported on this context");
			return false;
		}
		break;
	case types::ShaderType::TessEvaluationShader:
#if defined(GL_TESS_EVALUATION_SHADER_EXT)
		if (!contextCapabilities || contextCapabilities->supports(ApiCapabilities::Tessellation))
		{
			outShader = gl::CreateShader(GL_TESS_EVALUATION_SHADER_EXT);
		}
		else
#endif
		{
			Log("loadShader: Tessellation not supported on this context");
			return false;
		}
		break;
	default:
		Log("loadShader: Unknown shader type requested.");
		return false;
	}
	string::size_type versBegin = shaderSrc.find("#version");
	string::size_type versEnd = 0;
	string sourceDataStr;
	if (versBegin != string::npos)
	{
		versEnd = shaderSrc.find("\n", versBegin);
		sourceDataStr.append(shaderSrc.begin() + versBegin, shaderSrc.begin() + versBegin + versEnd);
		sourceDataStr.append("\n");
	}
	else
	{
		versBegin = 0;
	}
	// insert the defines
	for (uint32 i = 0; i < defineCount; ++i)
	{
		sourceDataStr.append("#define ");
		sourceDataStr.append(defines[i]);
		sourceDataStr.append("\n");
	}
	sourceDataStr.append("\n");
	sourceDataStr.append(shaderSrc.begin() + versBegin + versEnd, shaderSrc.end());
	const char* pSource = sourceDataStr.c_str();
	gl::ShaderSource(outShader, 1, &pSource, NULL);
	gl::CompileShader(outShader);
	// error checking
	GLint glRslt;
	gl::GetShaderiv(outShader, GL_COMPILE_STATUS, &glRslt);
	if (!glRslt)
	{
		int infoLogLength, charsWritten;
		// get the length of the log
		gl::GetShaderiv(outShader, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::vector<char> pLog;
		pLog.resize(infoLogLength);
		gl::GetShaderInfoLog(outShader, infoLogLength, &charsWritten, pLog.data());
		const char* const typestring =
		  (shaderType == types::ShaderType::VertexShader ? "Vertex" :
		   shaderType == types::ShaderType::FragmentShader ? "Fragment" :
		   shaderType == types::ShaderType::ComputeShader ? "Compute" :
		   shaderType == types::ShaderType::TessControlShader ? "TessellationControl" :
		   shaderType == types::ShaderType::TessEvaluationShader ? "TessellationEvaluation" :
		   shaderType == types::ShaderType::RayShader ? "Ray" : "Unknown");


		Log(Log.Error, strings::createFormatted("Failed to compile %s shader.\n "
		                                        "==========Infolog:==========\n%s\n============================", typestring, pLog.data()).c_str());
		return false;
	}
	return true;
}

bool loadShader(const native::HContext_& context, Stream& shaderData, types::ShaderType shaderType,
                types::ShaderBinaryFormat binaryFormat, pvr::native::HShader_& outShader,
                const ApiCapabilities* contextCapabilities)
{
#if defined(TARGET_OS_IPHONE)
	assertion(false , "Target IPhone does not support Binary");
	return false;
#else

	if (binaryFormat != types::ShaderBinaryFormat::ImgSgx)
	{
		assertion(0 , "Non-SGX shader binaries not supported");
		Log("Attempted to load non-SGX shader binaries not supported");
		return false;
	}
	/* Create and compile the shader object */
	outShader = gl::CreateShader(GL_SGX_BINARY_IMG);
	string shaderBinaryData;
	shaderBinaryData.reserve(shaderData.getSize());
	size_t readLength = 0;
	shaderData.read(shaderData.getSize(), 1, &shaderBinaryData[0], readLength);
	gl::ShaderBinary(1, &outShader.handle, GL_SGX_BINARY_IMG, shaderBinaryData.c_str(), (GLint)shaderData.getSize());

	if (gl::GetError() != GL_NO_ERROR)
	{
		Log(Log.Error, "Failed to load binary shader\n");
		return false;
	}
#endif
	return true;
}

bool createShaderProgram(pvr::native::HShader_ pShaders[], uint32 count,
                         const char** const sAttribs, pvr::uint16* attribIndex, uint32 attribCount, pvr::native::HPipeline_& outShaderProg,
                         string* infolog, const ApiCapabilities* contextCapabilities)
{
	pvr::api::logApiError("createShaderProgram begin");
	if (!outShaderProg.handle)
	{
		outShaderProg = gl::CreateProgram();
	}
	for (uint32 i = 0; i < count; ++i)
	{
		pvr::api::logApiError("createShaderProgram begin AttachShader");
		gl::AttachShader(outShaderProg, pShaders[i].handle);
		pvr::api::logApiError("createShaderProgram end AttachShader");
	}
	if (sAttribs && attribCount)
	{
		for (uint32 i = 0; i < attribCount; ++i)
		{
			gl::BindAttribLocation(outShaderProg, attribIndex[i], sAttribs[i]);
		}
	}
	pvr::api::logApiError("createShaderProgram begin linkProgram");
	gl::LinkProgram(outShaderProg);
	pvr::api::logApiError("createShaderProgram end linkProgram");
	//check for link sucess
	GLint glStatus;
	gl::GetProgramiv(outShaderProg, GL_LINK_STATUS, &glStatus);
	if (!glStatus && infolog)
	{
		int32 infoLogLength, charWriten;
		gl::GetProgramiv(outShaderProg, GL_INFO_LOG_LENGTH, &infoLogLength);
		infolog->resize(infoLogLength);
		if (infoLogLength)
		{
			gl::GetProgramInfoLog(outShaderProg, infoLogLength, &charWriten, &(*infolog)[0]);
			Log(Log.Debug, infolog->c_str());
		}
		return false;
	}
	pvr::api::logApiError("createShaderProgram end");
	return true;
}

}
}
//!\endcond
