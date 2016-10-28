/*!*********************************************************************************************************************
\file       PVRNativeApi\ShaderUtils.h
\author     PowerVR by Imagination, Developer Technology Team
\copyright  Copyright (c) Imagination Technologies Limited.
\brief    Contains useful low level utils for shaders (loading, compiling) into low level Api object wrappers.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/IGraphicsContext.h"
#include "PVRAssets/Shader.h"

namespace pvr {
namespace utils {

/*!*********************************************************************************************************************
\brief Load a shader from binary.
\param context A Framework-wrapped handle to the API context (GL context, VK device etc) where the shader will be loaded to
\param shaderData A stream containing the shader binary data
\param shaderType The type (stage) of the shader (vertex, fragment...)
\param binaryFormat The binary format of the shader
\param[out] outShader The native Shader object will be saved here
\param contextCapabilities Optional A pointer to the API capabilities can be passed here
\return true on success
***********************************************************************************************************************/
bool loadShader(const native::HContext_& context, Stream& shaderData, types::ShaderType shaderType, types::ShaderBinaryFormat binaryFormat,
                native::HShader_& outShader, const ApiCapabilities* contextCapabilities = 0);


/*!*********************************************************************************************************************
\brief Load shader from shader source.
\param context A Framework-wrapped handle to the API context (GL context, VK device etc) where the shader will be loaded to
\param shaderSource A stream containing the shader source text data
\param shaderType The type (stage) of the shader (vertex, fragment...)
\param defines A number of preprocessor definitions that will be passed to the shader
\param numDefines The number of defines
\param[out] outShader The native Shader object will be saved here
\param contextCapabilities Optional A pointer to the API capabilities can be passed here
\return true on success
***********************************************************************************************************************/
bool loadShader(const native::HContext_& context, const Stream& shaderSource, types::ShaderType shaderType, const char* const* defines,
                uint32 numDefines, native::HShader_& outShader, const ApiCapabilities* contextCapabilities = 0);

}
}
