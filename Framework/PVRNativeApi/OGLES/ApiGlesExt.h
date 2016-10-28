/*!*********************************************************************************************************************
\file         PVRNativeApi\OGLES\ApiGlesExt.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains function pointer declarations for all OpenGL ES 2+ extension functions.
***********************************************************************************************************************/
#pragma once
#include "PVRNativeApi/OGLES/OpenGLESHeaders.h"

// GL_EXT_multi_draw_arrays
typedef void (PVR_APIENTRY* PROC_EXT_glMultiDrawElementsEXT)(GLenum mode, const GLsizei* count, GLenum type,
    const GLvoid** indices, GLsizei primcount);
typedef void (PVR_APIENTRY* PROC_EXT_glMultiDrawArraysEXT)(GLenum mode, const GLint* first, const GLsizei* count,
    GLsizei primcount);

// GL_EXT_discard_framebuffer
typedef void (PVR_APIENTRY* PROC_EXT_glDiscardFramebufferEXT)(GLenum target, GLsizei numAttachments, const GLenum* attachments);

//
typedef void* (PVR_APIENTRY* PROC_EXT_glMapBufferOES)(GLenum target, GLenum access);
typedef GLboolean(PVR_APIENTRY* PROC_EXT_glUnmapBufferOES)(GLenum target);
typedef void (PVR_APIENTRY* PROC_EXT_glGetBufferPointervOES)(GLenum target, GLenum pname, void** params);

// GL_OES_vertex_array_object
typedef void (PVR_APIENTRY* PROC_EXT_glBindVertexArrayOES)(GLuint vertexarray);
typedef void (PVR_APIENTRY* PROC_EXT_glDeleteVertexArraysOES)(GLsizei n, const GLuint* vertexarrays);
typedef void (PVR_APIENTRY* PROC_EXT_glGenVertexArraysOES)(GLsizei n, GLuint* vertexarrays);
typedef GLboolean(PVR_APIENTRY* PROC_EXT_glIsVertexArrayOES)(GLuint vertexarray);

// GL_NV_Fence
typedef void (PVR_APIENTRY* PROC_EXT_glDeleteFencesNV)(GLsizei, const GLuint*);
typedef void (PVR_APIENTRY* PROC_EXT_glGenFencesNV)(GLsizei, GLuint*);
typedef GLboolean(PVR_APIENTRY* PROC_EXT_glIsFenceNV)(GLuint);
typedef GLboolean(PVR_APIENTRY* PROC_EXT_glTestFenceNV)(GLuint);
typedef void (PVR_APIENTRY* PROC_EXT_glGetFenceivNV)(GLuint, GLenum, GLint*);
typedef void (PVR_APIENTRY* PROC_EXT_glFinishFenceNV)(GLuint);
typedef void (PVR_APIENTRY* PROC_EXT_glSetFenceNV)(GLuint, GLenum);

#ifndef TARGET_OS_IPHONE
// GL_OES_EGL_image
typedef void (PVR_APIENTRY* PROC_EXT_glEGLImageTargetTexture2DOES)(GLenum target, GLeglImageOES image);
typedef void (PVR_APIENTRY* PROC_EXT_glEGLImageTargetRenderbufferStorageOES)(GLenum target, GLeglImageOES image);
#endif
// multisampled_render_to_texture
typedef void (PVR_APIENTRY* PROC_EXT_glRenderbufferStorageMultisampleIMG)(GLenum , GLsizei , GLenum , GLsizei , GLsizei);
typedef void (PVR_APIENTRY* PROC_EXT_glFramebufferTexture2DMultisampleIMG)(GLenum  , GLenum , GLenum , GLuint , GLint , GLsizei);

// AMD_performance_monitor
typedef void (PVR_APIENTRY* PROC_EXT_glGetPerfMonitorGroupsAMD)(GLint* numGroups, GLsizei groupsSize, GLuint* groups);
typedef void (PVR_APIENTRY* PROC_EXT_glGetPerfMonitorCountersAMD)(GLuint group, GLint* numCounters, GLint* maxActiveCounters,
    GLsizei counterSize, GLuint* counters);
typedef void (PVR_APIENTRY* PROC_EXT_glGetPerfMonitorGroupStringAMD)(GLuint group, GLsizei bufSize, GLsizei* length,
    char* groupString);
typedef void (PVR_APIENTRY* PROC_EXT_glGetPerfMonitorCounterStringAMD)(GLuint group, GLuint counter, GLsizei bufSize,
    GLsizei* length, char* counterString);
typedef void (PVR_APIENTRY* PROC_EXT_glGetPerfMonitorCounterInfoAMD)(GLuint group, GLuint counter, GLenum pname, GLvoid* data);
typedef void (PVR_APIENTRY* PROC_EXT_glGenPerfMonitorsAMD)(GLsizei n, GLuint* monitors);
typedef void (PVR_APIENTRY* PROC_EXT_glDeletePerfMonitorsAMD)(GLsizei n, GLuint* monitors);
typedef void (PVR_APIENTRY* PROC_EXT_glSelectPerfMonitorCountersAMD)(GLuint monitor, GLboolean enable, GLuint group,
    GLint numCounters, GLuint* countersList);
typedef void (PVR_APIENTRY* PROC_EXT_glBeginPerfMonitorAMD)(GLuint monitor);
typedef void (PVR_APIENTRY* PROC_EXT_glEndPerfMonitorAMD)(GLuint monitor);
typedef void (PVR_APIENTRY* PROC_EXT_glGetPerfMonitorCounterDataAMD)(GLuint monitor, GLenum pname, GLsizei dataSize, GLuint* data,
    GLint* bytesWritten);

// GL_ANGLE_framebuffer_blit
typedef void (PVR_APIENTRY* PROC_EXT_glBlitFramebufferANGLE)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0,
    GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);

// GL_ANGLE_framebuffer_multisample
typedef void (PVR_APIENTRY* PROC_EXT_glRenderbufferStorageMultisampleANGLE)(GLenum target, GLsizei samples, GLenum internalformat,
    GLsizei width, GLsizei height);

// GL_APPLE_framebuffer_multisample
typedef void (PVR_APIENTRY* PROC_EXT_glRenderbufferStorageMultisampleAPPLE)(GLenum target, GLsizei samples, GLenum internalformat,
    GLsizei width, GLsizei height);
typedef void (PVR_APIENTRY* PROC_EXT_glResolveMultisampleFramebufferAPPLE)(void);

// GL_NV_coverage_sample
typedef void (PVR_APIENTRY* PROC_EXT_glCoverageMaskNV)(GLboolean mask);
typedef void (PVR_APIENTRY* PROC_EXT_glCoverageOperationNV)(GLenum operation);

// GL_QCOM_driver_control
typedef void (PVR_APIENTRY* PROC_EXT_glGetDriverControlsQCOM)(GLint* num, GLsizei size, GLuint* driverControls);
typedef void (PVR_APIENTRY* PROC_EXT_glGetDriverControlStringQCOM)(GLuint driverControl, GLsizei bufSize, GLsizei* length,
    char* driverControlString);
typedef void (PVR_APIENTRY* PROC_EXT_glEnableDriverControlQCOM)(GLuint driverControl);
typedef void (PVR_APIENTRY* PROC_EXT_glDisableDriverControlQCOM)(GLuint driverControl);

// GL_QCOM_extended_get
typedef void (PVR_APIENTRY* PROC_EXT_glExtGetTexturesQCOM)(GLuint* textures, GLint maxTextures, GLint* numTextures);
typedef void (PVR_APIENTRY* PROC_EXT_glExtGetBuffersQCOM)(GLuint* buffers, GLint maxBuffers, GLint* numBuffers);
typedef void (PVR_APIENTRY* PROC_EXT_glExtGetRenderbuffersQCOM)(GLuint* renderbuffers, GLint maxRenderbuffers,
    GLint* numRenderbuffers);
typedef void (PVR_APIENTRY* PROC_EXT_glExtGetFramebuffersQCOM)(GLuint* framebuffers, GLint maxFramebuffers,
    GLint* numFramebuffers);
typedef void (PVR_APIENTRY* PROC_EXT_glExtGetTexLevelParameterivQCOM)(GLuint texture, GLenum face, GLint level, GLenum pname,
    GLint* params);
typedef void (PVR_APIENTRY* PROC_EXT_glExtTexObjectStateOverrideiQCOM)(GLenum target, GLenum pname, GLint param);
typedef void (PVR_APIENTRY* PROC_EXT_glExtGetTexSubImageQCOM)(GLenum target, GLint level, GLint xoffset, GLint yoffset,
    GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLvoid* texels);
typedef void (PVR_APIENTRY* PROC_EXT_glExtGetBufferPointervQCOM)(GLenum target, GLvoid** params);

// GL_QCOM_extended_get2
typedef void (PVR_APIENTRY* PROC_EXT_glExtGetShadersQCOM)(GLuint* shaders, GLint maxShaders, GLint* numShaders);
typedef void (PVR_APIENTRY* PROC_EXT_glExtGetProgramsQCOM)(GLuint* programs, GLint maxPrograms, GLint* numPrograms);
typedef GLboolean(PVR_APIENTRY* PROC_EXT_glExtIsProgramBinaryQCOM)(GLuint program);
typedef void (PVR_APIENTRY* PROC_EXT_glExtGetProgramBinarySourceQCOM)(GLuint program, GLenum shadertype, char* source,
    GLint* length);

// GL_QCOM_tiled_rendering
typedef void (PVR_APIENTRY* PROC_EXT_glStartTilingQCOM)(GLuint x, GLuint y, GLuint width, GLuint height, GLbitfield preserveMask);
typedef void (PVR_APIENTRY* PROC_EXT_glEndTilingQCOM)(GLbitfield preserveMask);

// GL_OES_get_program_binary
typedef void (PVR_APIENTRY* PROC_EXT_glGetProgramBinaryOES)(GLuint program, GLsizei bufSize, GLsizei* length,
    GLenum* binaryFormat, GLvoid* binary);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramBinaryOES)(GLuint program, GLenum binaryFormat, const GLvoid* binary, GLint length);

// GL_OES_texture_3D
typedef void (PVR_APIENTRY* PROC_EXT_glTexImage3DOES)(GLenum target, GLint level, GLenum internalformat, GLsizei width,
    GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
typedef void (PVR_APIENTRY* PROC_EXT_glTexSubImage3DOES)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
    GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels);
typedef void (PVR_APIENTRY* PROC_EXT_glCopyTexSubImage3DOES)(GLenum target, GLint level, GLint xoffset, GLint yoffset,
    GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (PVR_APIENTRY* PROC_EXT_glCompressedTexImage3DOES)(GLenum target, GLint level, GLenum internalformat, GLsizei width,
    GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data);
typedef void (PVR_APIENTRY* PROC_EXT_glCompressedTexSubImage3DOES)(GLenum target, GLint level, GLint xoffset, GLint yoffset,
    GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data);
typedef void (PVR_APIENTRY* PROC_EXT_glFramebufferTexture3DOES)(GLenum target, GLenum attachment, GLenum textarget,
    GLuint texture, GLint level, GLint zoffset);

// GL_OES_blend_equation_separate
typedef void (PVR_APIENTRY* PROC_EXT_glBlendEquationSeparateOES)(GLenum modeRGB, GLenum modeAlpha);

// GL_OES_blend_func_separate
typedef void (PVR_APIENTRY* PROC_EXT_glBlendFuncSeparateOES)(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);

// GL_OES_blend_subtract
typedef void (PVR_APIENTRY* PROC_EXT_glBlendEquationOES)(GLenum mode);

// GL_OES_query_matrix
typedef GLbitfield(PVR_APIENTRY* PROC_EXT_glQueryMatrixxOES)(GLfixed mantissa[16], GLint exponent[16]);

// GL_APPLE_copy_texture_levels
typedef void (PVR_APIENTRY* PROC_EXT_glCopyTextureLevelsAPPLE)(GLuint destinationTexture, GLuint sourceTexture,
    GLint sourceBaseLevel, GLsizei sourceLevelCount);


// GL_APPLE_framebuffer_multisample
typedef void (PVR_APIENTRY* PROC_EXT_glRenderbufferStorageMultisampleAPPLE)(GLenum target, GLsizei samples, GLenum internalformat,
    GLsizei width, GLsizei height);
typedef void (PVR_APIENTRY* PROC_EXT_glResolveMultisampleFramebufferAPPLE)(void);

// GL_APPLE_sync
typedef GLsync(PVR_APIENTRY* PROC_EXT_glFenceSyncAPPLE)(GLenum condition, GLbitfield flags);
typedef GLboolean(PVR_APIENTRY* PROC_EXT_glIsSyncAPPLE)(GLsync sync);
typedef void (PVR_APIENTRY* PROC_EXT_glDeleteSyncAPPLE)(GLsync sync);
typedef GLenum(PVR_APIENTRY* PROC_EXT_glClientWaitSyncAPPLE)(GLsync sync, GLbitfield flags, GLuint64 timeout);
typedef void (PVR_APIENTRY* PROC_EXT_glWaitSyncAPPLE)(GLsync sync, GLbitfield flags, GLuint64 timeout);
typedef void (PVR_APIENTRY* PROC_EXT_glGetInteger64vAPPLE)(GLenum pname, GLint64* params);
typedef void (PVR_APIENTRY* PROC_EXT_glGetSyncivAPPLE)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length,
    GLint* values);

// GL_EXT_map_buffer_range
typedef void* (PVR_APIENTRY* PROC_EXT_glMapBufferRangeEXT)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef void (PVR_APIENTRY* PROC_EXT_glFlushMappedBufferRangeEXT)(GLenum target, GLintptr offset, GLsizeiptr length);

// GL_EXT_multisampled_render_to_texture
typedef void (PVR_APIENTRY* PROC_EXT_glRenderbufferStorageMultisampleEXT)(GLenum target, GLsizei samples, GLenum internalformat,
    GLsizei width, GLsizei height);
typedef void (PVR_APIENTRY* PROC_EXT_glFramebufferTexture2DMultisampleEXT)(GLenum target, GLenum attachment, GLenum textarget,
    GLuint texture, GLint level, GLsizei samples);

// GL_EXT_robustness
typedef GLenum(PVR_APIENTRY* PROC_EXT_glGetGraphicsResetStatusEXT)(void);
typedef void (PVR_APIENTRY* PROC_EXT_glReadnPixelsEXT)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format,
    GLenum type, GLsizei bufSize, void* data);
typedef void (PVR_APIENTRY* PROC_EXT_glGetnUniformfvEXT)(GLuint program, GLint location, GLsizei bufSize, float* params);
typedef void (PVR_APIENTRY* PROC_EXT_glGetnUniformivEXT)(GLuint program, GLint location, GLsizei bufSize, GLint* params);

// GL_EXT_texture_storage
typedef void (PVR_APIENTRY* PROC_EXT_glTexStorage1DEXT)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
typedef void (PVR_APIENTRY* PROC_EXT_glTexStorage2DEXT)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width,
    GLsizei height);
typedef void (PVR_APIENTRY* PROC_EXT_glTexStorage3DEXT)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width,
    GLsizei height, GLsizei depth);
typedef void (PVR_APIENTRY* PROC_EXT_glTextureStorage1DEXT)(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat,
    GLsizei width);
typedef void (PVR_APIENTRY* PROC_EXT_glTextureStorage2DEXT)(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat,
    GLsizei width, GLsizei height);
typedef void (PVR_APIENTRY* PROC_EXT_glTextureStorage3DEXT)(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat,
    GLsizei width, GLsizei height, GLsizei depth);

// GL_IMG_multisampled_render_to_texture
typedef void (PVR_APIENTRY* PROC_EXT_glRenderbufferStorageMultisampleIMG)(GLenum target, GLsizei samples, GLenum internalformat,
    GLsizei width, GLsizei height);
typedef void (PVR_APIENTRY* PROC_EXT_glFramebufferTexture2DMultisampleIMG)(GLenum target, GLenum attachment, GLenum textarget,
    GLuint texture, GLint level, GLsizei samples);

// GL_KHR_debug
#if !defined(GL_KHR_debug)
typedef void (GL_APIENTRYP GLDEBUGPROCKHR)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
    const GLchar* message, const void* userParam);
#endif

typedef void (PVR_APIENTRY* PROC_EXT_glDebugMessageControlKHR)(GLenum source, GLenum type, GLenum severity, GLsizei count,
    const GLuint* ids, GLboolean enabled);
typedef void (PVR_APIENTRY* PROC_EXT_glDebugMessageInsertKHR)(GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar* buf);
typedef void (PVR_APIENTRY* PROC_EXT_glDebugMessageCallbackKHR)(GLDEBUGPROCKHR callback, const void* userParam);
typedef GLuint(PVR_APIENTRY* PROC_EXT_glGetDebugMessageLogKHR)(GLuint count, GLsizei bufsize, GLenum* sources, GLenum* types,
    GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog);
typedef void (PVR_APIENTRY* PROC_EXT_glPushDebugGroupKHR)(GLenum source, GLuint id, GLsizei length, const GLchar* message);
typedef void (PVR_APIENTRY* PROC_EXT_glPopDebugGroupKHR)(void);
typedef void (PVR_APIENTRY* PROC_EXT_glObjectLabelKHR)(GLenum identifier, GLuint name, GLsizei length, const GLchar* label);
typedef void (PVR_APIENTRY* PROC_EXT_glGetObjectLabelKHR)(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei* length,
    GLchar* label);
typedef void (PVR_APIENTRY* PROC_EXT_glObjectPtrLabelKHR)(const void* ptr, GLsizei length, const GLchar* label);
typedef void (PVR_APIENTRY* PROC_EXT_glGetObjectPtrLabelKHR)(const void* ptr, GLsizei bufSize, GLsizei* length, GLchar* label);
typedef void (PVR_APIENTRY* PROC_EXT_glGetPointervKHR)(GLenum pname, void** params);

// GL_ANGLE_instanced_arrays
typedef void (PVR_APIENTRY* PROC_EXT_glDrawArraysInstancedANGLE)(GLenum mode, GLint first, GLsizei count, GLsizei primcount);
typedef void (PVR_APIENTRY* PROC_EXT_glDrawElementsInstancedANGLE)(GLenum mode, GLsizei count, GLenum type, const void* indices,
    GLsizei primcount);
typedef void (PVR_APIENTRY* PROC_EXT_glVertexAttribDivisorANGLE)(GLuint index, GLuint divisor);

// GL_ANGLE_texture_usage
typedef void (PVR_APIENTRY* PROC_EXT_glGetTranslatedShaderSourceANGLE)(GLuint shader, GLsizei bufsize, GLsizei* length,
    GLchar* source);

// GL_APPLE_copy_texture_levels
typedef void (PVR_APIENTRY* PROC_EXT_glCopyTextureLevelsAPPLE)(GLuint destinationTexture, GLuint sourceTexture,
    GLint sourceBaseLevel, GLsizei sourceLevelCount);

// GL_EXT_debug_label
typedef void (PVR_APIENTRY* PROC_EXT_glLabelObjectEXT)(GLenum type, GLuint object, GLsizei length, const GLchar* label);
typedef void (PVR_APIENTRY* PROC_EXT_glGetObjectLabelEXT)(GLenum type, GLuint object, GLsizei bufSize, GLsizei* length,
    GLchar* label);

// GL_EXT_debug_marker
typedef void (PVR_APIENTRY* PROC_EXT_glInsertEventMarkerEXT)(GLsizei length, const GLchar* marker);
typedef void (PVR_APIENTRY* PROC_EXT_glPushGroupMarkerEXT)(GLsizei length, const GLchar* marker);
typedef void (PVR_APIENTRY* PROC_EXT_glPopGroupMarkerEXT)(void);

// GL_EXT_occlusion_query_boolean
typedef void (PVR_APIENTRY* PROC_EXT_glGenQueriesEXT)(GLsizei n, GLuint* ids);
typedef void (PVR_APIENTRY* PROC_EXT_glDeleteQueriesEXT)(GLsizei n, const GLuint* ids);
typedef GLboolean(PVR_APIENTRY* PROC_EXT_glIsQueryEXT)(GLuint id);
typedef void (PVR_APIENTRY* PROC_EXT_glBeginQueryEXT)(GLenum target, GLuint id);
typedef void (PVR_APIENTRY* PROC_EXT_glEndQueryEXT)(GLenum target);
typedef void (PVR_APIENTRY* PROC_EXT_glGetQueryivEXT)(GLenum target, GLenum pname, GLint* params);
typedef void (PVR_APIENTRY* PROC_EXT_glGetQueryObjectuivEXT)(GLuint id, GLenum pname, GLuint* params);

// GL_EXT_separate_shader_objects
typedef void (PVR_APIENTRY* PROC_EXT_glUseProgramStagesEXT)(GLuint pipeline, GLbitfield stages, GLuint program);
typedef void (PVR_APIENTRY* PROC_EXT_glActiveShaderProgramEXT)(GLuint pipeline, GLuint program);
typedef GLuint(PVR_APIENTRY* PROC_EXT_glCreateShaderProgramvEXT)(GLenum type, GLsizei count, const GLchar** strings);
typedef void (PVR_APIENTRY* PROC_EXT_glBindProgramPipelineEXT)(GLuint pipeline);
typedef void (PVR_APIENTRY* PROC_EXT_glDeleteProgramPipelinesEXT)(GLsizei n, const GLuint* pipelines);
typedef void (PVR_APIENTRY* PROC_EXT_glGenProgramPipelinesEXT)(GLsizei n, GLuint* pipelines);
typedef GLboolean(PVR_APIENTRY* PROC_EXT_glIsProgramPipelineEXT)(GLuint pipeline);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramParameteriEXT)(GLuint program, GLenum pname, GLint value);
typedef void (PVR_APIENTRY* PROC_EXT_glGetProgramPipelineivEXT)(GLuint pipeline, GLenum pname, GLint* params);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniform1iEXT)(GLuint program, GLint location, GLint x);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniform2iEXT)(GLuint program, GLint location, GLint x, GLint y);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniform3iEXT)(GLuint program, GLint location, GLint x, GLint y, GLint z);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniform4iEXT)(GLuint program, GLint location, GLint x, GLint y, GLint z, GLint w);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniform1fEXT)(GLuint program, GLint location, GLfloat x);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniform2fEXT)(GLuint program, GLint location, GLfloat x, GLfloat y);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniform3fEXT)(GLuint program, GLint location, GLfloat x, GLfloat y, GLfloat z);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniform4fEXT)(GLuint program, GLint location, GLfloat x, GLfloat y, GLfloat z,
    GLfloat w);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniform1ivEXT)(GLuint program, GLint location, GLsizei count, const GLint* value);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniform2ivEXT)(GLuint program, GLint location, GLsizei count, const GLint* value);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniform3ivEXT)(GLuint program, GLint location, GLsizei count, const GLint* value);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniform4ivEXT)(GLuint program, GLint location, GLsizei count, const GLint* value);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniform1fvEXT)(GLuint program, GLint location, GLsizei count, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniform2fvEXT)(GLuint program, GLint location, GLsizei count, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniform3fvEXT)(GLuint program, GLint location, GLsizei count, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniform4fvEXT)(GLuint program, GLint location, GLsizei count, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniformMatrix2fvEXT)(GLuint program, GLint location, GLsizei count,
    GLboolean transpose, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniformMatrix3fvEXT)(GLuint program, GLint location, GLsizei count,
    GLboolean transpose, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniformMatrix4fvEXT)(GLuint program, GLint location, GLsizei count,
    GLboolean transpose, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_EXT_glValidateProgramPipelineEXT)(GLuint pipeline);
typedef void (PVR_APIENTRY* PROC_EXT_glGetProgramPipelineInfoLogEXT)(GLuint pipeline, GLsizei bufSize, GLsizei* length,
    GLchar* infoLog);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniform1uiEXT)(GLuint program, GLint location, GLuint v0);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniform2uiEXT)(GLuint program, GLint location, GLuint v0, GLuint v1);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniform3uiEXT)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniform4uiEXT)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2,
    GLuint v3);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniform1uivEXT)(GLuint program, GLint location, GLsizei count, const GLuint* value);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniform2uivEXT)(GLuint program, GLint location, GLsizei count, const GLuint* value);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniform3uivEXT)(GLuint program, GLint location, GLsizei count, const GLuint* value);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniform4uivEXT)(GLuint program, GLint location, GLsizei count, const GLuint* value);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniformMatrix2x3fvEXT)(GLuint program, GLint location, GLsizei count,
    GLboolean transpose, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniformMatrix3x2fvEXT)(GLuint program, GLint location, GLsizei count,
    GLboolean transpose, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniformMatrix2x4fvEXT)(GLuint program, GLint location, GLsizei count,
    GLboolean transpose, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniformMatrix4x2fvEXT)(GLuint program, GLint location, GLsizei count,
    GLboolean transpose, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniformMatrix3x4fvEXT)(GLuint program, GLint location, GLsizei count,
    GLboolean transpose, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_EXT_glProgramUniformMatrix4x3fvEXT)(GLuint program, GLint location, GLsizei count,
    GLboolean transpose, const GLfloat* value);

// GL_QCOM_alpha_test
typedef void (PVR_APIENTRY* PROC_EXT_glAlphaFuncQCOM)(GLenum func, GLclampf ref);

// GL_NV_read_buffer
typedef void (PVR_APIENTRY* PROC_EXT_glReadBufferNV)(GLenum mode);

// GL_NV_draw_buffers
typedef void (PVR_APIENTRY* PROC_EXT_glDrawBuffersNV)(GLsizei n, const GLenum* bufs);

// GL_EXT_multiview_draw_buffers
typedef void (PVR_APIENTRY* PROC_EXT_glReadBufferIndexedEXT)(GLenum src, GLint index);
typedef void (PVR_APIENTRY* PROC_EXT_glDrawBuffersIndexedEXT)(GLint n, const GLenum* location, const GLint* indices);
typedef void (PVR_APIENTRY* PROC_EXT_glGetIntegeri_vEXT)(GLenum target, GLuint index, GLint* data);

//EXT_draw_buffers:
typedef void (PVR_APIENTRY* PROC_EXT_glDrawBuffersEXT)(GLsizei n, const GLenum* bufs);

//EXT_blend_minmax:
typedef void (PVR_APIENTRY* PROC_EXT_glBlendEquationEXT)(GLenum mode);

// GL_KHR_blend_equation_advanced
typedef void (PVR_APIENTRY* PROC_EXT_glBlendBarrierKHR)(void);

// GL_OES_texture_storage_multisample_2d_array
typedef void (GL_APIENTRYP PROC_EXT_glTexStorage3DMultisampleOES)(GLenum target, GLsizei samples, GLenum internalformat,
    GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);

// GL_OVR_multiview
typedef void(GL_APIENTRYP PROC_EXT_glFramebufferTextureMultiviewOVR)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint baseViewIndex, GLsizei numViews);

/* PLS2 */
typedef void (PVR_APIENTRY* PROC_EXT_glFramebufferPixelLocalStorageSize)(GLuint target, GLsizei storageSize);
typedef void (PVR_APIENTRY* PROC_EXT_glClearPixelLocalStorageui)(GLsizei offset, GLsizei n, const GLuint* values);
typedef void (PVR_APIENTRY* PROC_EXT_glGetFramebufferPixelLocalStorageSize)(GLuint target);

/* Buffer Storage EXT */
typedef void (PVR_APIENTRY* PROC_EXT_glBufferStorageEXT)(GLenum target, GLsizei size, const void* data, GLbitfield flags);

/* GL_IMG_clear_texture */
typedef void (PVR_APIENTRY* PROC_EXT_glClearTexImageIMG)(GLuint texture, GLint level,
    GLenum format, GLenum type,
    const GLvoid* data);
typedef void (PVR_APIENTRY* PROC_EXT_glClearTexSubImageIMG)(GLuint texture, GLint level,
    GLint xoffset, GLint yoffset, GLint zoffset,
    GLsizei width, GLsizei height, GLsizei depth,
    GLenum format, GLenum type,
    const GLvoid* data);

/* GL_IMG_framebuffer_downsample */
typedef void (PVR_APIENTRY* PROC_EXT_glFramebufferTexture2DDownsampleIMG)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLuint xscale, GLuint yscale);
typedef void (PVR_APIENTRY* PROC_EXT_glFramebufferTextureLayerDownsampleIMG)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer, GLuint xscale, GLuint yscale);

/*GL_EXT_tessellation_shader*/
typedef void (PVR_APIENTRY* PROC_EXT_glPatchParameteriEXT)(GLenum pname, GLint val);
