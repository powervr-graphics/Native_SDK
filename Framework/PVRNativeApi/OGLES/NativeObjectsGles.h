/*!
\brief This file contains the OpenGL ES definitions of the (normally only forward-declared in app code) native
handles to the underlying OpenGL ES objects. The objects are always wrapped in a thin struct to allow
forward-declaring.
\file PVRNativeApi/OGLES/NativeObjectsGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
//Headers
#include "PVRNativeApi/OGLES/OpenGLESHeaders.h"
#include "PVRCore/Math/Rectangle.h"
//Handles
namespace pvr {
namespace native {
//!\cond NO_DOXYGEN
struct GLdummy { };

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
DECLARE_NATIVE_TYPE(HFence_, GLsync);

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
DECLARE_DUMMY_TYPE(HSemaphore_);
DECLARE_DUMMY_TYPE(HEvent_);
#undef DECLARE_NATIVE_TYPE
#undef DECLARE_DUMMY_TYPE

/// <summary>Handle to an OpenGL ES texture.</summary>
struct HTexture_
{
	typedef GLuint NativeType; NativeType handle;
	typedef GLenum TargetNativeType; TargetNativeType target;

	/// <summary>Construct with empty OpenGL ES Texture and target</summary>
	HTexture_() : handle(0), target(0) {}

	/// <summary>Construct with OpenGL ES Texture and OpenGL ES target.</summary>
	/// <param name="handle">An OpenGL ES texture name</param>
	/// <param name="target">The GLenum target that this texture should be bound to (e.g. GL_TEXTURE_2D)</param>
	HTexture_(NativeType handle, TargetNativeType target) : handle(handle), target(target) {}
	operator NativeType() { return handle; }
	NativeType operator*() { return handle; }
};
//!\endcond

#define PVR_DECLARE_NATIVE_CAST(_naked_name_) \
namespace pvr { namespace api { \
inline const gles::_naked_name_##Gles_& native_cast(const pvr::api::impl::_naked_name_##_& object) { return static_cast<const pvr::api::gles::_naked_name_##Gles_&>(object); } \
inline const gles::_naked_name_##Gles_* native_cast(const pvr::api::_naked_name_& object) { return &native_cast(*object); } \
inline const gles::_naked_name_##Gles_* native_cast(const pvr::api::impl::_naked_name_##_* object) { return &native_cast(*object); } \
inline gles::_naked_name_##Gles_& native_cast(pvr::api::impl::_naked_name_##_& object) { return static_cast<pvr::api::gles::_naked_name_##Gles_&>(object); } \
inline gles::_naked_name_##Gles_* native_cast(pvr::api::_naked_name_& object) { return &native_cast(*object); } \
inline gles::_naked_name_##Gles_* native_cast(pvr::api::impl::_naked_name_##_* object) { return &native_cast(*object); } \
} }

/*!  \struct HFbo_ */
/// <summary>Handle to OpenGL ES Frame-Buffer-Object.</summary>
/*!  \struct HSampler_ */
/// <summary>Handle to an OpenGL ES sampler. Can be retrieved with the native_cast function on a PVRApi Sampler
/// </summary>
/*!  \struct HBuffer_ */
/// <summary>Handle to an OpenGL ES buffer. Can be retrieved with the native_cast function on a PVRApi Buffer
/// </summary>
/*!  \struct HShader_ */
/// <summary>Handle to an OpenGL ES shader.</summary>
/*!  \struct HPipeline_ */
/// <summary>Handle to an OpenGL ESs compiled shader program.</summary>
/*!  \struct HColorAttachmentView_ */
/// <summary>Handle to an OpenGL ES ColorAttachmentView. Dummy class for OpenGL ES implementation.</summary>
/*!  \struct HColorAttachmentView_ */
/// <summary>Handle to a Descriptor Set Layout. Dummy class, as the OpenGL ES API does not have an explicit Color
/// Attachment View object.</summary>
/*!  \struct HDescriptorSet_ */
/// <summary>Handle to a Descriptor Set. Dummy class, as the OpenGL ES API does not have a Descriptor Set object.
/// </summary>
}
}