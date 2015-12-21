/*!*********************************************************************************************************************
\file         PVRApi\OGLES\Shader.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementation of the IGraphicsContext::createShader methods. See IGraphicsContext.h.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRCore/IGraphicsContext.h"
#include "PVRApi/ApiObjects/Shader.h"
#include "PVRApi/OGLES/ShaderGles.h"
#include "PVRApi/OGLES/NativeObjectsGles.h"
#include "PVRApi/OGLES/OpenGLESBindings.h"
#include "PVRApi/ShaderUtils.h"

namespace pvr {

api::Shader IGraphicsContext::createShader(const Stream& shaderSrc, ShaderType::Enum type,  const char* const* defines, uint32 numDefines)
{
	api::ShaderGles vs;
	vs.construct(0);
	native::HShader vvs(vs);
	bool rslt = utils::loadShader(shaderSrc, type, defines, numDefines, vvs, &m_apiCapabilities);
	if (!rslt)
	{
		Log(Log.Error, "Failed to create VertexShader.");
		vs.release();
	}
	return vs;
}

api::Shader IGraphicsContext::createShader(Stream& shaderData, ShaderType::Enum type, assets::ShaderBinaryFormat::Enum binaryFormat)
{
	api::ShaderGles vs; vs.construct(0);
	native::HShader vvs(vs);
	bool rslt = utils::loadShader(shaderData, type, binaryFormat, vvs, &m_apiCapabilities);
	if (!rslt)
	{
		Log(Log.Error, "Failed to create VertexShader.");
		vs.release();
	}
	return vs;
}

}
//!\endcond