/*!
\brief Contains useful low level utils for shaders (loading, compiling) into low level Api object wrappers.
\file PVRNativeApi/Vulkan/ShaderUtilsVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Interfaces/IGraphicsContext.h"

namespace pvr {
namespace nativeVk {

/// <summary>Load shader from shader source.</summary>
/// <param name="context">A Framework-wrapped handle to the API context (GL context, VK device etc) where the
/// shader will be loaded to</param>
/// <param name="shaderSource">A stream containing the shader source text data</param>
/// <param name="shaderType">The type (stage) of the shader (vertex, fragment...)</param>
/// <param name="defines">A number of preprocessor definitions that will be passed to the shader</param>
/// <param name="numDefines">The number of defines</param>
/// <param name="outShader">The native Shader object will be saved here</param>
/// <param name="contextCapabilities">Optional A pointer to the API capabilities can be passed here</param>
/// <returns>true on success</returns>
bool loadShader(const native::HContext_& context, const Stream& shaderSource, types::ShaderType shaderType, const char* const* defines,
                uint32 numDefines, native::HShader_& outShader, const ApiCapabilities* contextCapabilities = 0);

}
}