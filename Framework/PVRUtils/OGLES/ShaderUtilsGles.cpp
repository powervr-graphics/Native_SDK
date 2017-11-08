/*!
\brief Implementations of the shader utility functions
\file PVRUtils/OGLES/ShaderUtilsGles.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRCore/Stream.h"
#include "PVRCore/StringFunctions.h"
#include "PVRUtils/OGLES/ShaderUtilsGles.h"
#include "PVRUtils/OGLES/BindingsGles.h"
#include "PVRUtils/OGLES/ErrorsGles.h"

namespace pvr {
namespace utils {

bool loadShader(const Stream& shaderSource, ShaderType shaderType, const char* const* defines, uint32_t defineCount,
                GLuint& outShader)
{
	logApiError("loadShader: Error on entry!");
	if (!shaderSource.isopen() && !shaderSource.open())
	{
		Log(LogLevel::Error, "loadShader: Could not open the shaderSource stream.");
		return false;
	}
	if (outShader)
	{
		Log(LogLevel::Warning,
		    "loadShader: Generated shader passed to loadShader. Deleting reference to avoid leaking a preexisting shader object.");
		gl::DeleteShader(outShader);
		outShader = 0;
	}
	std::string shaderSrc;
	if (!shaderSource.readIntoString(shaderSrc)) { return false; };

	switch (shaderType)
	{
	case ShaderType::VertexShader:
		outShader = gl::CreateShader(GL_VERTEX_SHADER);
		break;
	case ShaderType::FragmentShader:
		outShader = gl::CreateShader(GL_FRAGMENT_SHADER);
		break;
	case ShaderType::ComputeShader:
#if defined(GL_COMPUTE_SHADER)
		outShader = gl::CreateShader(GL_COMPUTE_SHADER);
#else
		Log("loadShader: Compute Shader not supported on this context");
		return false;
#endif
		break;
	case ShaderType::GeometryShader:
#if defined(GL_GEOMETRY_SHADER_EXT)
		outShader = gl::CreateShader(GL_GEOMETRY_SHADER_EXT);
#else
		Log("loadShader: Geometry Shader not supported on this context");
		return false;
#endif

		break;
	case ShaderType::TessControlShader:
#if defined(GL_TESS_CONTROL_SHADER_EXT)
		outShader = gl::CreateShader(GL_TESS_CONTROL_SHADER_EXT);
#else
		Log("loadShader: Tessellation not supported on this context");
		return false;
#endif
		break;
	case ShaderType::TessEvaluationShader:
#if defined(GL_TESS_EVALUATION_SHADER_EXT)
		outShader = gl::CreateShader(GL_TESS_EVALUATION_SHADER_EXT);
#else
		Log("loadShader: Tessellation not supported on this context");
		return false;
#endif
		break;
	default:
		Log("loadShader: Unknown shader type requested.");
		return false;
	}
	std::string::size_type versBegin = shaderSrc.find("#version");
	std::string::size_type versEnd = 0;
	std::string sourceDataStr;
	if (versBegin != std::string::npos)
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
	for (uint32_t i = 0; i < defineCount; ++i)
	{
		sourceDataStr.append("#define ");
		sourceDataStr.append(defines[i]);
		sourceDataStr.append("\n");
	}
	sourceDataStr.append("\n");
	sourceDataStr.append(shaderSrc.begin() + versBegin + versEnd, shaderSrc.end());
	const char* pSource = sourceDataStr.c_str();
	gl::ShaderSource(outShader, 1, &pSource, NULL);
	logApiError("CreateShader::glShaderSource error");
	gl::CompileShader(outShader);

	logApiError("CreateShader::glCompile error");

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
		  (shaderType == ShaderType::VertexShader ? "Vertex" :
		   shaderType == ShaderType::FragmentShader ? "Fragment" :
		   shaderType == ShaderType::ComputeShader ? "Compute" :
		   shaderType == ShaderType::TessControlShader ? "TessellationControl" :
		   shaderType == ShaderType::TessEvaluationShader ? "TessellationEvaluation" : "Unknown");

		Log(LogLevel::Error, strings::createFormatted("Failed to compile %s shader.\n "
		    "==========Infolog:==========\n%s\n============================", typestring, pLog.data()).c_str());
		return false;
	}
	logApiError("CreateShader::exit");
	return true;
}

bool createShaderProgram(GLuint pShaders[], uint32_t count,
                         const char** const sAttribs, const uint16_t* attribIndex, uint32_t attribCount,
                         GLuint& outShaderProg, std::string* infologptr)
{
	pvr::utils::logApiError("createShaderProgram begin");
	if (!outShaderProg)
	{
		outShaderProg = gl::CreateProgram();
	}
	for (uint32_t i = 0; i < count; ++i)
	{
		pvr::utils::logApiError("createShaderProgram begin AttachShader");
		gl::AttachShader(outShaderProg, pShaders[i]);
		pvr::utils::logApiError("createShaderProgram end AttachShader");
	}
	if (sAttribs && attribCount)
	{
		for (uint32_t i = 0; i < attribCount; ++i)
		{
			gl::BindAttribLocation(outShaderProg, attribIndex[i], sAttribs[i]);
		}
	}
	pvr::utils::logApiError("createShaderProgram begin linkProgram");
	gl::LinkProgram(outShaderProg);
	pvr::utils::logApiError("createShaderProgram end linkProgram");
	//check for link sucess
	GLint glStatus;

	gl::GetProgramiv(outShaderProg, GL_LINK_STATUS, &glStatus);
	std::string default_infolog;
	std::string& infolog = infologptr ? *infologptr : default_infolog;
	if (!glStatus)
	{
		int32_t infoLogLength, charWriten;
		gl::GetProgramiv(outShaderProg, GL_INFO_LOG_LENGTH, &infoLogLength);
		infolog.resize(infoLogLength);
		if (infoLogLength)
		{
			gl::GetProgramInfoLog(outShaderProg, infoLogLength, &charWriten, &infolog[0]);
			Log(LogLevel::Debug, infolog.c_str());
		}
		return false;
	}
	pvr::utils::logApiError("createShaderProgram end");
	return true;
}
}
}
//!\endcond
