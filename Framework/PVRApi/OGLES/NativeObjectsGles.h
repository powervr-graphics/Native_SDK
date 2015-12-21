/*!*********************************************************************************************************************
\file         PVRApi\OGLES\NativeObjectsGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         This file contains the OpenGL ES definitions of the (normally only forward-declared in app code) native handles to
              the underlying OpenGL ES objects. The objects are always wrapped in a thin struct to allow forward-declaring.
***********************************************************************************************************************/
#pragma once
//Headers
#include "PVRApi/OGLES/OpenGLESHeaders.h"
#include "PVRCore/Rectangle.h"
//Handles
namespace pvr {
namespace native {
//!\cond NO_DOXYGEN
struct GLdummy { };
//!\endcond

/*!*********************************************************************************************************************
\brief Handle to OpenGL ES Frame-Buffer-Object.
***********************************************************************************************************************/
struct HFbo_
{
	typedef GLuint NativeType; GLuint handle;

	/*!*********************************************************************************************************************
	\brief Construct with default(0) OpenGL ES FBO.
	***********************************************************************************************************************/
	HFbo_() : handle(0) {}

	/*!*********************************************************************************************************************
	\brief Construct with OpenGL Framebuffer name
	\param handle An OpenGL Framebuffer name
	***********************************************************************************************************************/
	HFbo_(GLuint handle) : handle(handle) {}
	operator NativeType() { return handle; }
	NativeType operator*() { return handle; }
};

/*!*********************************************************************************************************************
\brief Handle to an OpenGL ES texture.
***********************************************************************************************************************/
struct HTexture_
{
	typedef GLuint NativeType; NativeType handle;
	typedef GLenum TargetNativeType; TargetNativeType target;

	/*!*********************************************************************************************************************
	\brief Construct with empty OpenGL ES Texture and target
	***********************************************************************************************************************/
	HTexture_() : handle(0), target(0) {}

	/*!*********************************************************************************************************************
	\brief Construct with OpenGL ES Texture and OpenGL ES target.
	\param handle An OpenGL ES texture name
	\param target The GLenum target that this texture should be bound to (e.g. GL_TEXTURE_2D)
	***********************************************************************************************************************/
	HTexture_(NativeType handle, TargetNativeType target) : handle(handle), target(target) {}
	operator NativeType() { return handle; }
	NativeType operator*() { return handle; }
};

/*!*********************************************************************************************************************
\brief Handle to an OpenGL ES sampler.
***********************************************************************************************************************/
struct HSampler_
{
	typedef GLuint NativeType; NativeType handle;

	/*!*********************************************************************************************************************
	\brief Construct with empty handle.
	***********************************************************************************************************************/
	HSampler_() : handle(0) {}

	/*!*********************************************************************************************************************
	\brief Construct with OpenGL Sampler name
	\param handle An OpenGL Sampler name
	***********************************************************************************************************************/
	HSampler_(GLuint handle) : handle(handle) {}
	operator NativeType() { return handle; }
	NativeType operator*() { return handle; }
};

/*!*********************************************************************************************************************
\brief Handle to an OpenGL ES buffer.
***********************************************************************************************************************/
struct HBuffer_
{
	typedef GLuint NativeType; GLuint handle;

	/*!*********************************************************************************************************************
	\brief Construct with empty handle.
	***********************************************************************************************************************/
	HBuffer_() : handle(0) {}

	/*!*********************************************************************************************************************
	\brief Construct with OpenGL Buffer name
	\param handle An OpenGL Buffer name
	***********************************************************************************************************************/
	HBuffer_(GLuint handle) : handle(handle) {}
	operator NativeType() { return handle; }
	NativeType operator*() { return handle; }
};

/*!*********************************************************************************************************************
\brief Handle to an OpenGL ES shader.
***********************************************************************************************************************/
struct HShader_
{
	typedef GLuint NativeType; GLuint handle;

	/*!*********************************************************************************************************************
	\brief Construct with default handle.
	***********************************************************************************************************************/
	HShader_() : handle(0) {}

	/*!*********************************************************************************************************************
	\brief Construct with OpenGL Shader name
	\param handle An OpenGL Shader name
	***********************************************************************************************************************/
	HShader_(GLuint handle) : handle(handle) {}
	operator NativeType() { return handle; }
	NativeType operator*() { return handle; }
};

/*!*********************************************************************************************************************
\brief Handle to an OpenGL ESs compiled shader program.
***********************************************************************************************************************/
struct HShaderProgram_
{
	typedef GLuint NativeType; GLuint handle;

	/*!*********************************************************************************************************************
	\brief Construct with default handle.
	***********************************************************************************************************************/
	HShaderProgram_() : handle(0) {}

	/*!*********************************************************************************************************************
	\brief Construct with OpenGL ShaderProgram name
	\param handle An OpenGL ShaderProgram name
	***********************************************************************************************************************/
	HShaderProgram_(GLuint handle) : handle(handle) {}
	operator NativeType() { return handle; }
	NativeType operator*() { return handle; }
};

/*!*********************************************************************************************************************
\brief Handle to an OpenGL ES ColorAttachmentView. Dummy class (for OpenGL ES implementation).
***********************************************************************************************************************/
struct HColorAttachmentView_
{
	typedef GLdummy NativeType; NativeType handle;

	/*!*********************************************************************************************************************
	\brief Construct empty. Dummy function (for OpenGL ES implementation).
	***********************************************************************************************************************/
	HColorAttachmentView_() {}

	/*!*********************************************************************************************************************
	\brief Construct with OpenGL name. Dummy function (for OpenGL ES implementation).
	\param handle Dummy param. Unused
	***********************************************************************************************************************/
	HColorAttachmentView_(NativeType handle) {}
	operator NativeType() { return handle; }
	NativeType operator*() { return handle; }
};

/*!*********************************************************************************************************************
\brief Dummy OpenGL ES implementation of a Descriptor Set Layout
***********************************************************************************************************************/
struct HDescriptorSetLayout_
{
	typedef GLdummy NativeType; NativeType handle; HDescriptorSetLayout_() {}
};

/*!*********************************************************************************************************************
\brief Dummy OpenGL ES implementation of a Descriptor Set
***********************************************************************************************************************/
struct HDescriptorSet_
{
	typedef GLdummy NativeType; NativeType handle; HDescriptorSet_() {}
};



}
}
