<<<<<<< HEAD
/*!*********************************************************************************************************************
\file         PVRApi\OGLES\ShaderGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains OpenGL ES specific implementation of the Shader class. Use only if directly using OpenGL ES calls.
              Provides the definitions allowing to move from the Framework object Texture2D to the underlying OpenGL ES Shader.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
=======
/*!
\brief Contains OpenGL ES specific implementation of the Shader class. Use only if directly using OpenGL ES calls.
Provides the definitions allowing to move from the Framework object Texture2D to the underlying OpenGL ES
Shader.
\file PVRApi/OGLES/ShaderGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
>>>>>>> 1776432f... 4.3
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

<<<<<<< HEAD
}
namespace utils {
/*!*********************************************************************************************************************
\brief Create a native shader program from an array of native shader handles.
\param pShaders array of shaders
\param count number shaders in the array
\param attribs array of attributes
\param attribIndex array of attributeIndices
\param attribCount Number of attributes in the attributes array
\param outShaderProg Output, the shader program
\param infolog OPTIONAL Output, the infolog of the shader
\param contextCapabilities OPTIONAL can be used to pass specific context capabilities
\return true on success
***********************************************************************************************************************/
bool createShaderProgram(native::HShader_ pShaders[], uint32 count, const char** const attribs, pvr::uint16* attribIndex, uint32 attribCount, native::HPipeline_& outShaderProg,
                         string* infolog, const ApiCapabilities* contextCapabilities = 0);
}
}
//!\endcond
=======
PVR_DECLARE_NATIVE_CAST(Shader);
>>>>>>> 1776432f... 4.3
