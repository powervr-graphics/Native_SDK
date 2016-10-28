/*!*********************************************************************************************************************
\file         PVRNativeApi\OGLES\ApiGles31.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains function pointer declarations for all OpenGL ES 3.1 functions (minus ES 3/2 functions).
***********************************************************************************************************************/
#pragma once
#include "PVRNativeApi/OGLES/ApiGles3.h"
typedef void (PVR_APIENTRY* PROC_ES31_glDispatchCompute)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
typedef void (PVR_APIENTRY* PROC_ES31_glDispatchComputeIndirect)(GLintptr indirect);
typedef void (PVR_APIENTRY* PROC_ES31_glDrawArraysIndirect)(GLenum mode, const void* indirect);
typedef void (PVR_APIENTRY* PROC_ES31_glDrawElementsIndirect)(GLenum mode, GLenum type, const void* indirect);
typedef void (PVR_APIENTRY* PROC_ES31_glFramebufferParameteri)(GLenum target, GLenum pname, GLint param);
typedef void (PVR_APIENTRY* PROC_ES31_glGetFramebufferParameteriv)(GLenum target, GLenum pname, GLint* params);
typedef void (PVR_APIENTRY* PROC_ES31_glGetProgramInterfaceiv)(GLuint program, GLenum programInterface, GLenum pname, GLint* params);
typedef GLuint(PVR_APIENTRY* PROC_ES31_glGetProgramResourceIndex)(GLuint program, GLenum programInterface, const GLchar* name);
typedef void (PVR_APIENTRY* PROC_ES31_glGetProgramResourceName)(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei* length, GLchar* name);
typedef void (PVR_APIENTRY* PROC_ES31_glGetProgramResourceiv)(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum* props, GLsizei bufSize, GLsizei* length, GLint* params);
typedef GLint(PVR_APIENTRY* PROC_ES31_glGetProgramResourceLocation)(GLuint program, GLenum programInterface, const GLchar* name);
typedef void (PVR_APIENTRY* PROC_ES31_glUseProgramStages)(GLuint pipeline, GLbitfield stages, GLuint program);
typedef void (PVR_APIENTRY* PROC_ES31_glActiveShaderProgram)(GLuint pipeline, GLuint program);
typedef GLuint(PVR_APIENTRY* PROC_ES31_glCreateShaderProgramv)(GLenum type, GLsizei count, const GLchar* const* strings);
typedef void (PVR_APIENTRY* PROC_ES31_glBindProgramPipeline)(GLuint pipeline);
typedef void (PVR_APIENTRY* PROC_ES31_glDeleteProgramPipelines)(GLsizei n, const GLuint* pipelines);
typedef void (PVR_APIENTRY* PROC_ES31_glGenProgramPipelines)(GLsizei n, GLuint* pipelines);
typedef GLboolean(PVR_APIENTRY* PROC_ES31_glIsProgramPipeline)(GLuint pipeline);
typedef void (PVR_APIENTRY* PROC_ES31_glGetProgramPipelineiv)(GLuint pipeline, GLenum pname, GLint* params);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniform1i)(GLuint program, GLint location, GLint v0);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniform2i)(GLuint program, GLint location, GLint v0, GLint v1);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniform3i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniform4i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniform1ui)(GLuint program, GLint location, GLuint v0);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniform2ui)(GLuint program, GLint location, GLuint v0, GLuint v1);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniform3ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniform4ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniform1f)(GLuint program, GLint location, GLfloat v0);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniform2f)(GLuint program, GLint location, GLfloat v0, GLfloat v1);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniform3f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniform4f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniform1iv)(GLuint program, GLint location, GLsizei count, const GLint* value);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniform2iv)(GLuint program, GLint location, GLsizei count, const GLint* value);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniform3iv)(GLuint program, GLint location, GLsizei count, const GLint* value);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniform4iv)(GLuint program, GLint location, GLsizei count, const GLint* value);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniform1uiv)(GLuint program, GLint location, GLsizei count, const GLuint* value);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniform2uiv)(GLuint program, GLint location, GLsizei count, const GLuint* value);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniform3uiv)(GLuint program, GLint location, GLsizei count, const GLuint* value);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniform4uiv)(GLuint program, GLint location, GLsizei count, const GLuint* value);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniform1fv)(GLuint program, GLint location, GLsizei count, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniform2fv)(GLuint program, GLint location, GLsizei count, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniform3fv)(GLuint program, GLint location, GLsizei count, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniform4fv)(GLuint program, GLint location, GLsizei count, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniformMatrix2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniformMatrix3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniformMatrix4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniformMatrix2x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniformMatrix3x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniformMatrix2x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniformMatrix4x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniformMatrix3x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_ES31_glProgramUniformMatrix4x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (PVR_APIENTRY* PROC_ES31_glValidateProgramPipeline)(GLuint pipeline);
typedef void (PVR_APIENTRY* PROC_ES31_glGetProgramPipelineInfoLog)(GLuint pipeline, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void (PVR_APIENTRY* PROC_ES31_glBindImageTexture)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
typedef void (PVR_APIENTRY* PROC_ES31_glGetBooleani_v)(GLenum target, GLuint index, GLboolean* data);
typedef void (PVR_APIENTRY* PROC_ES31_glMemoryBarrier)(GLbitfield barriers);
typedef void (PVR_APIENTRY* PROC_ES31_glMemoryBarrierByRegion)(GLbitfield barriers);
typedef void (PVR_APIENTRY* PROC_ES31_glTexStorage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef void (PVR_APIENTRY* PROC_ES31_glGetMultisamplefv)(GLenum pname, GLuint index, GLfloat* val);
typedef void (PVR_APIENTRY* PROC_ES31_glSampleMaski)(GLuint maskNumber, GLbitfield mask);
typedef void (PVR_APIENTRY* PROC_ES31_glGetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint* params);
typedef void (PVR_APIENTRY* PROC_ES31_glGetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat* params);
typedef void (PVR_APIENTRY* PROC_ES31_glBindVertexBuffer)(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
typedef void (PVR_APIENTRY* PROC_ES31_glVertexAttribFormat)(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
typedef void (PVR_APIENTRY* PROC_ES31_glVertexAttribIFormat)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
typedef void (PVR_APIENTRY* PROC_ES31_glVertexAttribBinding)(GLuint attribindex, GLuint bindingindex);
typedef void (PVR_APIENTRY* PROC_ES31_glVertexBindingDivisor)(GLuint bindingindex, GLuint divisor);

