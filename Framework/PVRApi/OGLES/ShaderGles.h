/*!
\brief Contains OpenGL ES specific implementation of the Shader class. Use only if directly using OpenGL ES calls.
Provides the definitions allowing to move from the Framework object Texture2D to the underlying OpenGL ES
Shader.
\file PVRApi/OGLES/ShaderGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiObjects/Shader.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"

namespace pvr {
namespace api {
namespace gles {
/// <summary>OpenGL ES implementation of a shader</summary>
class ShaderGles_ : public native::HShader_, public impl::Shader_
{
public:

	/// <summary>ctor. Construct with namtoe shader handle.</summary>
	ShaderGles_(const GraphicsContext& context, const native::HShader_& shader) : impl::Shader_(context)
	{
		this->handle = shader.handle;
	}

	/// <summary>dtor.</summary>
	virtual ~ShaderGles_();
};
typedef RefCountedResource<ShaderGles_> ShaderGles;
}
}
}

PVR_DECLARE_NATIVE_CAST(Shader);
