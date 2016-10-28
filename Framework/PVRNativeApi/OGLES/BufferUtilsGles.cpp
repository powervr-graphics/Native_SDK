/*!*********************************************************************************************************************
\file         PVRNativeApi\OGLES\ShaderUtils.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Implementations of the shader utility functions
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRNativeApi/BufferUtils.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"
namespace pvr {
namespace apiUtils {
bool createBuffer(IPlatformContext& context, types::BufferBindingUse usage,
                  pvr::uint32 size, bool memHostVisible, native::HBuffer_& outBuffer)
{
	gl::GenBuffers(1, &outBuffer.handle);
	gl::BufferData(outBuffer.handle, size, NULL, (memHostVisible ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
	return true;
}
}
}
//!\endcond
