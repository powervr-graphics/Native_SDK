/*!*********************************************************************************************************************
\file         PVRNativeApi\OGLES\NativeObjectsGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         This file contains the OpenGL ES definitions of the (normally only forward-declared in app code) native handles to
              the underlying OpenGL ES objects. The objects are always wrapped in a thin struct to allow forward-declaring.
***********************************************************************************************************************/
#pragma once
//Headers
#include "PVRNativeApi/OGLES/OpenGLESHeaders.h"
#include "PVRCore/Rectangle.h"
//Handles
namespace pvr {
namespace native {
//!\cond NO_DOXYGEN
struct GLdummy { };
//!\endcond

#define DECLARE_NATIVE_TYPE(_framework_type_, _native_type_) \
struct _framework_type_\
{\
  typedef _native_type_ NativeType;\
  NativeType handle;\
  _framework_type_() : handle(_native_type_()) {} \
  _framework_type_(NativeType handle) : handle(handle) {} \
  operator const NativeType&() const { return handle; }\
  operator NativeType&() { return handle; }\
  const NativeType& operator*() const { return handle; }\
  NativeType& operator*() { return handle; }\
};
#define DECLARE_DUMMY_TYPE(_framework_type_) struct _framework_type_{ typedef void NativeType; };

DECLARE_NATIVE_TYPE(HFbo_, GLuint);
DECLARE_NATIVE_TYPE(HSampler_, GLuint);
DECLARE_NATIVE_TYPE(HBuffer_, GLuint);
DECLARE_NATIVE_TYPE(HShader_, GLuint);
DECLARE_NATIVE_TYPE(HPipeline_, GLuint);

DECLARE_DUMMY_TYPE(HContext_);
DECLARE_DUMMY_TYPE(HCommandPool_);
DECLARE_DUMMY_TYPE(HCommandBuffer_);
DECLARE_DUMMY_TYPE(HDescriptorSet_);
DECLARE_DUMMY_TYPE(HColorAttachmentView_);
DECLARE_DUMMY_TYPE(HDescriptorSetLayout_);
DECLARE_DUMMY_TYPE(HPipelineLayout_);
DECLARE_DUMMY_TYPE(HDescriptorPool_);
DECLARE_DUMMY_TYPE(HBufferView_);
DECLARE_DUMMY_TYPE(HImageView_);
#undef DECLARE_NATIVE_TYPE
#undef DECLARE_DUMMY_TYPE

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
\struct HFbo_
\brief Handle to OpenGL ES Frame-Buffer-Object.
***********************************************************************************************************************/
/*!*********************************************************************************************************************
\struct HSampler_
\brief Handle to an OpenGL ES sampler. Can be retrieved with the getNativeObject() method of a PVRApi Sampler
***********************************************************************************************************************/
/*!*********************************************************************************************************************
\struct HBuffer_
\brief Handle to an OpenGL ES buffer. Can be retrieved with the getNativeObject() method of a PVRApi Buffer
***********************************************************************************************************************/
/*!*********************************************************************************************************************
\struct HShader_
\brief Handle to an OpenGL ES shader.
**********************************************************************************************************************/
/*!*********************************************************************************************************************
\struct HPipeline_
\brief Handle to an OpenGL ESs compiled shader program.
***********************************************************************************************************************/
/*!*********************************************************************************************************************
\struct HColorAttachmentView_
\brief Handle to an OpenGL ES ColorAttachmentView. Dummy class for OpenGL ES implementation.
***********************************************************************************************************************/
/*!*********************************************************************************************************************
\struct HColorAttachmentView_
\brief Handle to a Descriptor Set Layout. Dummy class, as the OpenGL ES API does not have an
    explicit Color Attachment View object.
***********************************************************************************************************************/
/*!*********************************************************************************************************************
\struct HDescriptorSet_
\brief Handle to a Descriptor Set. Dummy class, as the OpenGL ES API does not have a Descriptor Set object.
***********************************************************************************************************************/
}
}
