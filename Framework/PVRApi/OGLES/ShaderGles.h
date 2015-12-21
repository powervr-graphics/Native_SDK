/*!*********************************************************************************************************************
\file         PVRApi\OGLES\ShaderGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains OpenGL ES specific implementation of the Shader class. Use only if directly using OpenGL ES calls.
              Provides the definitions allowing to move from the Framework object Texture2D to the underlying OpenGL ES Shader.
***********************************************************************************************************************/

#pragma once
#include "PVRApi/ApiObjects/Shader.h"
#include "PVRApi/OGLES/NativeObjectsGles.h"

namespace pvr {
namespace api {
namespace impl {
/*!*********************************************************************************************************************
\param  gles shader wrapper
***********************************************************************************************************************/
class ShaderGlesImpl : public native::HShader_, public ShaderImpl
{
public:

	/*!*********************************************************************************************************************
	\brief ctor. Construct with namtoe shader handle.
	***********************************************************************************************************************/
	ShaderGlesImpl(const native::HShader_& shader)
	{
		this->handle = shader.handle;
	}

	/*!*********************************************************************************************************************
	\brief dtor.
	***********************************************************************************************************************/
	virtual ~ShaderGlesImpl() {}
};
}

typedef RefCountedResource<impl::ShaderGlesImpl> ShaderGles;
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
inline RefCountedResource<HShader_> getNativeHandle(const RefCountedResource<api::impl::ShaderImpl>& Shader)
{
	return static_cast<RefCountedResource<native::HShader_>/**/>(static_cast<RefCountedResource<api::impl::ShaderGlesImpl>/**/>(Shader));
}

/*!*********************************************************************************************************************
\brief Get the OpenGL ES Shader object underlying a PVRApi Buffer object.
\return A OpenGL ES Shader. For immediate use only.
\description The object returned by this function will only be kept alive as long as there are other references to it. If all
        other references to it are released or out of scope, the Shader returned by this function will be deleted and invalid.
***********************************************************************************************************************/
inline HShader_::NativeType useNativeHandle(const RefCountedResource<api::impl::ShaderImpl>& Shader)
{
	return static_cast<const api::impl::ShaderGlesImpl&>(*Shader).handle;
}

}
}
