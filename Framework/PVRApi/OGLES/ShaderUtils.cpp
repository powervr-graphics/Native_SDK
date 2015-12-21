/*!*********************************************************************************************************************
\file         PVRApi\OGLES\ShaderUtils.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementations of the shader utility functions
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/ShaderUtils.h"
#include "PVRApi/OGLES/NativeObjectsGles.h"
#include "PVRApi/OGLES/OpenGLESBindings.h"
#include "PVRApi/ApiErrors.h"

namespace pvr {
namespace utils {

bool loadShader(const Stream& shaderSource, ShaderType::Enum shaderType, const char* const* defines, uint32 defineCount,
                        pvr::native::HShader& outShader, const ApiCapabilities* contextCapabilities)
{
	if (!shaderSource.isopen() && !shaderSource.open())
	{
		Log(Log.Error, "loadShader: Could not open the shaderSource stream.");
		return false;
	}
	if (!outShader.isValid())
	{
		Log(Log.Warning,
		    "loadShader: Unconstructed shader passed to loadShader. Constructing new object. Note that this handle is the only reference to it even if"
			"copies of that object have been attempted");
		outShader.construct(0);
	}
	if (outShader->handle)
	{
		pvr::Log(Log.Warning,
		         "loadShader: Generated shader passed to loadShader. Deleting reference to avoid leaking a preexisting shader object.");
		gl::DeleteShader(outShader->handle);
		outShader->handle = 0;
	}
	string shaderSrc;
	if (!shaderSource.readIntoString(shaderSrc)) { return false; };

	switch (shaderType)
	{
	case ShaderType::VertexShader:
		outShader->handle = gl::CreateShader(GL_VERTEX_SHADER);
		break;
	case ShaderType::FragmentShader:
		outShader->handle = gl::CreateShader(GL_FRAGMENT_SHADER);
		break;
	case ShaderType::ComputeShader:
#if !defined(BUILD_API_MAX)||BUILD_API_MAX >= 31
		if (!contextCapabilities || contextCapabilities->supports(ApiCapabilities::ComputeShader))
		{
			outShader->handle = gl::CreateShader(GL_COMPUTE_SHADER);
		}
		else
#endif
		{
			Log("loadShader: Compute Shader not supported on this context");
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
	gl::ShaderSource(outShader->handle, 1, &pSource, NULL);
	gl::CompileShader(outShader->handle);
	// error checking
	GLint glRslt;
	gl::GetShaderiv(outShader->handle, GL_COMPILE_STATUS, &glRslt);
	if (!glRslt)
	{
		int infoLogLength, charsWritten;
		// get the length of the log
		gl::GetShaderiv(outShader->handle, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::vector<char> pLog;
		pLog.resize(infoLogLength);
		gl::GetShaderInfoLog(outShader->handle, infoLogLength, &charsWritten, pLog.data());
		const char* const typestring =
		    (shaderType == ShaderType::VertexShader ? "Vertex" :
		     shaderType == ShaderType::FragmentShader ? "Fragment" :
		     shaderType == ShaderType::ComputeShader ? "Compute" :
		     shaderType == ShaderType::FrameShader ? "Frame" :
		     shaderType == ShaderType::RayShader ? "Ray" : "Unknown");


		Log(Log.Error, strings::createFormatted("Failed to compile %s shader.\n "
		                                        "==========Infolog:==========\n%s\n============================", typestring, pLog.data()).c_str());
		return false;
	}
	return true;
}

bool loadShader(Stream& shaderData, ShaderType::Enum shaderType,
                        assets::ShaderBinaryFormat::Enum binaryFormat, pvr::native::HShader& outShader,
                        const ApiCapabilities* contextCapabilities)
{
#if defined(TARGET_OS_IPHONE)
	PVR_ASSERT(false && "Target IPhone does not support Binary");
	return Result::UnsupportedRequest;
#else

	if (binaryFormat != assets::ShaderBinaryFormat::ImgSgx)
	{
		PVR_ASSERT(0 && "Non-SGX shader binaries not supported");
		Log("Attempted to load non-SGX shader binaries not supported");
		return false;
	}
	/* Create and compile the shader object */
	outShader->handle = gl::CreateShader(GL_SGX_BINARY_IMG);
	string shaderBinaryData;
	shaderBinaryData.reserve(shaderData.getSize());
	size_t readLength = 0;
	shaderData.read(shaderData.getSize(), 1, &shaderBinaryData[0], readLength);
	gl::ShaderBinary(1, &outShader->handle, GL_SGX_BINARY_IMG, shaderBinaryData.c_str(), (GLint)shaderData.getSize());

	if (gl::GetError() != GL_NO_ERROR)
	{
		Log(Log.Error, "Failed to load binary shader\n");
		return false;
	}
#endif
	return true;
}

bool createShaderProgram(pvr::native::HShader_ pShaders[], uint32 count,
                                 const char** const sAttribs, pvr::uint16* attribIndex, uint32 attribCount, pvr::native::HShaderProgram& shaderProg,
                                 string* infolog, const ApiCapabilities* contextCapabilities)
{
	pvr::api::logApiError("createShaderProgram begin");
	if (!shaderProg.isValid())
	{
		shaderProg.construct();
		shaderProg->handle = gl::CreateProgram();
	}
	for (uint32 i = 0; i < count; ++i)
	{
		pvr::api::logApiError("createShaderProgram begin AttachShader");
		gl::AttachShader(shaderProg->handle, pShaders[i].handle);
		pvr::api::logApiError("createShaderProgram end AttachShader");
	}
	if (sAttribs && attribCount)
	{
		for (uint32 i = 0; i < attribCount; ++i)
		{
			gl::BindAttribLocation(shaderProg->handle, attribIndex[i], sAttribs[i]);
		}
	}
	pvr::api::logApiError("createShaderProgram begin linkProgram");
	gl::LinkProgram(shaderProg->handle);
	pvr::api::logApiError("createShaderProgram end linkProgram");
	//check for link sucess
	GLint glStatus;
	gl::GetProgramiv(shaderProg->handle, GL_LINK_STATUS, &glStatus);
	if (!glStatus && infolog)
	{
		int32 infoLogLength, charWriten;
		gl::GetProgramiv(shaderProg->handle, GL_INFO_LOG_LENGTH, &infoLogLength);
		infolog->resize(infoLogLength);
		if (infoLogLength)
		{
			gl::GetProgramInfoLog(shaderProg->handle, infoLogLength, &charWriten, &(*infolog)[0]);
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