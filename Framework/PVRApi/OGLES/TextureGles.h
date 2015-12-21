/*!*********************************************************************************************************************
\file         PVRApi/OGLES/TextureGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains OpenGL ES specific implementation of the Texture class. Use only if directly using OpenGL ES calls.
Provides the definitions allowing to move from the Framework object Texture2D to the underlying OpenGL ES Texture.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRApi/OGLES/NativeObjectsGles.h"
namespace pvr {
namespace native {
/*!*********************************************************************************************************************
\brief Get the OpenGL ES texture object underlying a PVRApi Texture object.
\return A smart pointer wrapper containing the OpenGL ES Texture
\description The smart pointer returned by this function will keep alive the underlying OpenGL ES object even if all other
references to the texture (including the one that was passed to this function) are released.
***********************************************************************************************************************/
inline RefCountedResource<HTexture_> getNativeHandle(const RefCountedResource<api::impl::TextureStoreImpl>& texture)
{
	return texture->getNativeHandle();
}

/*!*********************************************************************************************************************
\brief Get the OpenGL ES texture underlying a PVRApi Texture object.
\return A OpenGL ES texture. For immediate use only.
\description The texture name returned by this function will only be valid as long as there are other references to it. If all
other references to it are out of scope or released, the name will be deleted.
***********************************************************************************************************************/
inline HTexture_::NativeType& useNativeHandle(RefCountedResource<api::impl::TextureStoreImpl>& texture)
{
	return texture->getNativeHandle()->handle;
}

/*!*********************************************************************************************************************
\brief Get the OpenGL ES object underlying a PVRApi Texture object.
\return A OpenGL ES texture. For immediate use only.
\description The texture name returned by this function will only be valid as long as there are other references to it. If all
other references to it are out of scope or released, the name will be deleted.
***********************************************************************************************************************/
inline const HTexture_::NativeType& useNativeHandle(const RefCountedResource<api::impl::TextureStoreImpl>& texture)
{
	return texture->getNativeHandle()->handle;
}

/*!*********************************************************************************************************************
\brief Get the OpenGL ES object underlying a PVRApi Texture object.
\return A OpenGL ES texture. For immediate use only.
\description The texture name returned by this function will only be valid as long as there are other references to it. If all
other references to it are out of scope or released, the name will be deleted.
***********************************************************************************************************************/
inline const HTexture_::TargetNativeType& useNativeHandleTarget(const RefCountedResource<api::impl::TextureStoreImpl>& texture)
{
	return texture->getNativeHandle()->target;
}


/*!*********************************************************************************************************************
\brief Get the OpenGL ES texture underlying a PVRApi Texture object.
\return A OpenGL ES texture. For immediate use only.
\description The texture name returned by this function will only be valid as long as there are other references to it. If all
other references to it are out of scope or released, the name will be deleted.
***********************************************************************************************************************/
inline HTexture_::TargetNativeType& useNativeHandleTarget(RefCountedResource<api::impl::TextureStoreImpl>& texture)
{
	return texture->getNativeHandle()->target;
}

}
}
