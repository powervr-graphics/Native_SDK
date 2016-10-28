/*!*********************************************************************************************************************
\file         PVRApi\OGLES\ShaderGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains OpenGL ES specific implementation of the Shader class. Use only if directly using OpenGL ES calls.
              Provides the definitions allowing to move from the Framework object Texture2D to the underlying OpenGL ES Shader.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#pragma once
#include "PVRApi/ApiObjects/Shader.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"

namespace pvr {
namespace api {
namespace gles {
/*!*********************************************************************************************************************
\param  gles shader wrapper
***********************************************************************************************************************/
class ShaderGles_ : public native::HShader_, public impl::Shader_
{
public:

	/*!*********************************************************************************************************************
	\brief ctor. Construct with namtoe shader handle.
	***********************************************************************************************************************/
	ShaderGles_(GraphicsContext& context, const native::HShader_& shader) : impl::Shader_(context)
	{
		this->handle = shader.handle;
	}

	/*!*********************************************************************************************************************
	\brief dtor.
	***********************************************************************************************************************/
	virtual ~ShaderGles_();
};
typedef RefCountedResource<ShaderGles_> ShaderGles;
}
}
namespace native {
/*!*********************************************************************************************************************
\brief Get the OpenGL ES Shader object underlying a PVRApi Shader object.
\return A smart pointer wrapper containing the OpenGL ES Shader.
\description The smart pointer returned by this function works normally with the reference counting, and shares it with the
             rest of the references to this object, keeping the underlying OpenGL ES object alive even if all other
             references to it (including the one that was passed to this function) are released. Release when done using it to
			 avoid leaking the object.
***********************************************************************************************************************/
inline HShader createNativeHandle(const RefCountedResource<api::impl::Shader_>& Shader)
{
	return static_cast<RefCountedResource<native::HShader_>/**/>(static_cast<RefCountedResource<api::gles::ShaderGles_>/**/>(Shader));
}

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