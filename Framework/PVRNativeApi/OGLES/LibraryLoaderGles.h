/*!*********************************************************************************************************************
\file         PVRNativeApi\OGLES\LibraryLoaderGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        This file contains OpenGL ES bindings with function pointers. The PowerVR Framework uses them to allow unified
              access to OpenGL ES 2 and OpenGL ES 3 context throught the same functions.
              Function pointer loading is done with the initGl function which is normally called by the PVR Shell.
***********************************************************************************************************************/
#pragma once
#include "PVRNativeApi/OGLES/OpenGLESHeaders.h"
#if BUILD_API_MAX>3
#include "ApiGles31.h"
#elif (BUILD_API_MAX==3)
#include "ApiGles3.h"
#elif (BUILD_API_MAX==2)
#include "ApiGles2.h"
#endif

#include "PVRCore/NativeLibrary.h"
#include "PVRNativeApi/OGLES/ApiGlesExt.h"

#ifdef MemoryBarrier
#undef MemoryBarrier
#endif
/*!*********************************************************************************************************************
\brief This class contains function pointers to all OpenGL ES 2,3 and 3.1 functions. These function pointers will be populated
       on the initGl call, but should only be used when compatible OpenGL ES contexts are current. Use normally, using the gl
     class as a namespace. For example gl::BindBuffer(GL_UNIFORM_BUFFER, myBuffer);
***********************************************************************************************************************/
class gl
{
public:
	//Call this function once before using the OpenGL namespace. Do not call during static initialisation phase as it uses static global components.
	/*!*******************************************************************************************************************
	\brief Call once per application run to populate the function pointers. PVRShell calls this on context creation.
	*********************************************************************************************************************/
	static void initGl();
	/*!*******************************************************************************************************************
	\brief Call once per application run to release the OpenGL library. PVRShell calls this on exit.
	*********************************************************************************************************************/
	static void releaseGl();

	// Es2 functions
	static PROC_ES2_glActiveTexture ActiveTexture;
	static PROC_ES2_glAttachShader AttachShader;
	static PROC_ES2_glBindAttribLocation BindAttribLocation;
	static PROC_ES2_glBindBuffer BindBuffer;
	static PROC_ES2_glBindFramebuffer BindFramebuffer;
	static PROC_ES2_glBindRenderbuffer BindRenderbuffer;
	static PROC_ES2_glBindTexture BindTexture;
	static PROC_ES2_glBlendColor BlendColor;
	static PROC_ES2_glBlendEquation BlendEquation;
	static PROC_ES2_glBlendEquationSeparate BlendEquationSeparate;
	static PROC_ES2_glBlendFunc BlendFunc;
	static PROC_ES2_glBlendFuncSeparate BlendFuncSeparate;
	static PROC_ES2_glBufferData BufferData;
	static PROC_ES2_glBufferSubData BufferSubData;
	static PROC_ES2_glCheckFramebufferStatus CheckFramebufferStatus;
	static PROC_ES2_glClear Clear;
	static PROC_ES2_glClearColor ClearColor;
	static PROC_ES2_glClearDepthf ClearDepthf;
	static PROC_ES2_glClearStencil ClearStencil;
	static PROC_ES2_glColorMask ColorMask;
	static PROC_ES2_glCompileShader CompileShader;
	static PROC_ES2_glCompressedTexImage2D CompressedTexImage2D;
	static PROC_ES2_glCompressedTexSubImage2D CompressedTexSubImage2D;
	static PROC_ES2_glCopyTexImage2D CopyTexImage2D;
	static PROC_ES2_glCopyTexSubImage2D CopyTexSubImage2D;
	static PROC_ES2_glCreateProgram CreateProgram;
	static PROC_ES2_glCreateShader CreateShader;
	static PROC_ES2_glCullFace CullFace;
	static PROC_ES2_glDeleteBuffers DeleteBuffers;
	static PROC_ES2_glDeleteFramebuffers DeleteFramebuffers;
	static PROC_ES2_glDeleteTextures DeleteTextures;
	static PROC_ES2_glDeleteProgram DeleteProgram;
	static PROC_ES2_glDeleteRenderbuffers DeleteRenderbuffers;
	static PROC_ES2_glDeleteShader DeleteShader;
	static PROC_ES2_glDetachShader DetachShader;
	static PROC_ES2_glDepthFunc DepthFunc;
	static PROC_ES2_glDepthMask DepthMask;
	static PROC_ES2_glDepthRangef DepthRangef;
	static PROC_ES2_glDisable Disable;
	static PROC_ES2_glDisableVertexAttribArray DisableVertexAttribArray;
	static PROC_ES2_glDrawArrays DrawArrays;
	static PROC_ES2_glDrawElements DrawElements;
	static PROC_ES2_glEnable Enable;
	static PROC_ES2_glEnableVertexAttribArray EnableVertexAttribArray;
	static PROC_ES2_glFinish Finish;
	static PROC_ES2_glFlush Flush;
	static PROC_ES2_glFramebufferRenderbuffer FramebufferRenderbuffer;
	static PROC_ES2_glFramebufferTexture2D FramebufferTexture2D;
	static PROC_ES2_glFrontFace FrontFace;
	static PROC_ES2_glGenBuffers GenBuffers;
	static PROC_ES2_glGenerateMipmap GenerateMipmap;
	static PROC_ES2_glGenFramebuffers GenFramebuffers;
	static PROC_ES2_glGenRenderbuffers GenRenderbuffers;
	static PROC_ES2_glGenTextures GenTextures;
	static PROC_ES2_glGetActiveAttrib GetActiveAttrib;
	static PROC_ES2_glGetActiveUniform GetActiveUniform;
	static PROC_ES2_glGetAttachedShaders GetAttachedShaders;
	static PROC_ES2_glGetAttribLocation GetAttribLocation;
	static PROC_ES2_glGetBooleanv GetBooleanv;
	static PROC_ES2_glGetBufferParameteriv GetBufferParameteriv;
	static PROC_ES2_glGetError GetError;
	static PROC_ES2_glGetFloatv GetFloatv;
	static PROC_ES2_glGetFramebufferAttachmentParameteriv GetFramebufferAttachmentParameteriv;
	static PROC_ES2_glGetIntegerv GetIntegerv;
	static PROC_ES2_glGetProgramiv GetProgramiv;
	static PROC_ES2_glGetProgramInfoLog GetProgramInfoLog;
	static PROC_ES2_glGetRenderbufferParameteriv GetRenderbufferParameteriv;
	static PROC_ES2_glGetShaderiv GetShaderiv;
	static PROC_ES2_glGetShaderInfoLog GetShaderInfoLog;
	static PROC_ES2_glGetShaderPrecisionFormat GetShaderPrecisionFormat;
	static PROC_ES2_glGetShaderSource GetShaderSource;
	static PROC_ES2_glGetString GetString;
	static PROC_ES2_glGetTexParameterfv GetTexParameterfv;
	static PROC_ES2_glGetTexParameteriv GetTexParameteriv;
	static PROC_ES2_glGetUniformfv GetUniformfv;
	static PROC_ES2_glGetUniformiv GetUniformiv;
	static PROC_ES2_glGetUniformLocation GetUniformLocation;
	static PROC_ES2_glGetVertexAttribfv GetVertexAttribfv;
	static PROC_ES2_glGetVertexAttribiv GetVertexAttribiv;
	static PROC_ES2_glGetVertexAttribPointerv GetVertexAttribPointerv;
	static PROC_ES2_glHint Hint;
	static PROC_ES2_glIsBuffer IsBuffer;
	static PROC_ES2_glIsEnabled IsEnabled;
	static PROC_ES2_glIsFramebuffer IsFramebuffer;
	static PROC_ES2_glIsProgram IsProgram;
	static PROC_ES2_glIsRenderbuffer IsRenderbuffer;
	static PROC_ES2_glIsShader IsShader;
	static PROC_ES2_glIsTexture IsTexture;
	static PROC_ES2_glLineWidth LineWidth;
	static PROC_ES2_glLinkProgram LinkProgram;
	static PROC_ES2_glPixelStorei PixelStorei;
	static PROC_ES2_glPolygonOffset PolygonOffset;
	static PROC_ES2_glReadPixels ReadPixels;
	static PROC_ES2_glReleaseShaderCompiler ReleaseShaderCompiler;
	static PROC_ES2_glRenderbufferStorage RenderbufferStorage;
	static PROC_ES2_glSampleCoverage SampleCoverage;
	static PROC_ES2_glScissor Scissor;
	static PROC_ES2_glShaderBinary ShaderBinary;
	static PROC_ES2_glShaderSource ShaderSource;
	static PROC_ES2_glStencilFunc StencilFunc;
	static PROC_ES2_glStencilFuncSeparate StencilFuncSeparate;
	static PROC_ES2_glStencilMask StencilMask;
	static PROC_ES2_glStencilMaskSeparate StencilMaskSeparate;
	static PROC_ES2_glStencilOp StencilOp;
	static PROC_ES2_glStencilOpSeparate StencilOpSeparate;
	static PROC_ES2_glTexImage2D TexImage2D;
	static PROC_ES2_glTexParameterf TexParameterf;
	static PROC_ES2_glTexParameterfv TexParameterfv;
	static PROC_ES2_glTexParameteri TexParameteri;
	static PROC_ES2_glTexParameteriv TexParameteriv;
	static PROC_ES2_glTexSubImage2D TexSubImage2D;
	static PROC_ES2_glUniform1f Uniform1f;
	static PROC_ES2_glUniform1fv Uniform1fv;
	static PROC_ES2_glUniform1i Uniform1i;
	static PROC_ES2_glUniform1iv Uniform1iv;
	static PROC_ES2_glUniform2f Uniform2f;
	static PROC_ES2_glUniform2fv Uniform2fv;
	static PROC_ES2_glUniform2i Uniform2i;
	static PROC_ES2_glUniform2iv Uniform2iv;
	static PROC_ES2_glUniform3f Uniform3f;
	static PROC_ES2_glUniform3fv Uniform3fv;
	static PROC_ES2_glUniform3i Uniform3i;
	static PROC_ES2_glUniform3iv Uniform3iv;
	static PROC_ES2_glUniform4f Uniform4f;
	static PROC_ES2_glUniform4fv Uniform4fv;
	static PROC_ES2_glUniform4i Uniform4i;
	static PROC_ES2_glUniform4iv Uniform4iv;
	static PROC_ES2_glUniformMatrix2fv UniformMatrix2fv;
	static PROC_ES2_glUniformMatrix3fv UniformMatrix3fv;
	static PROC_ES2_glUniformMatrix4fv UniformMatrix4fv;
	static PROC_ES2_glUseProgram UseProgram;
	static PROC_ES2_glValidateProgram ValidateProgram;
	static PROC_ES2_glVertexAttrib1f VertexAttrib1f;
	static PROC_ES2_glVertexAttrib1fv VertexAttrib1fv;
	static PROC_ES2_glVertexAttrib2f VertexAttrib2f;
	static PROC_ES2_glVertexAttrib2fv VertexAttrib2fv;
	static PROC_ES2_glVertexAttrib3f VertexAttrib3f;
	static PROC_ES2_glVertexAttrib3fv VertexAttrib3fv;
	static PROC_ES2_glVertexAttrib4f VertexAttrib4f;
	static PROC_ES2_glVertexAttrib4fv VertexAttrib4fv;
	static PROC_ES2_glVertexAttribPointer VertexAttribPointer;
	static PROC_ES2_glViewport Viewport;

#if (BUILD_API_MAX >= 30)
	// Es3 functions
	static PROC_ES3_glReadBuffer ReadBuffer;
	static PROC_ES3_glDrawRangeElements DrawRangeElements;
	static PROC_ES3_glTexImage3D TexImage3D;
	static PROC_ES3_glTexSubImage3D TexSubImage3D;
	static PROC_ES3_glCopyTexSubImage3D CopyTexSubImage3D;
	static PROC_ES3_glCompressedTexImage3D CompressedTexImage3D;
	static PROC_ES3_glCompressedTexSubImage3D CompressedTexSubImage3D;
	static PROC_ES3_glGenQueries GenQueries;
	static PROC_ES3_glDeleteQueries DeleteQueries;
	static PROC_ES3_glIsQuery IsQuery;
	static PROC_ES3_glBeginQuery BeginQuery;
	static PROC_ES3_glEndQuery EndQuery;
	static PROC_ES3_glGetQueryiv GetQueryiv;
	static PROC_ES3_glGetQueryObjectuiv GetQueryObjectuiv;
	static PROC_ES3_glUnmapBuffer UnmapBuffer;
	static PROC_ES3_glGetBufferPointerv GetBufferPointerv;
	static PROC_ES3_glDrawBuffers DrawBuffers;
	static PROC_ES3_glUniformMatrix2x3fv UniformMatrix2x3fv;
	static PROC_ES3_glUniformMatrix3x2fv UniformMatrix3x2fv;
	static PROC_ES3_glUniformMatrix2x4fv UniformMatrix2x4fv;
	static PROC_ES3_glUniformMatrix4x2fv UniformMatrix4x2fv;
	static PROC_ES3_glUniformMatrix3x4fv UniformMatrix3x4fv;
	static PROC_ES3_glUniformMatrix4x3fv UniformMatrix4x3fv;
	static PROC_ES3_glBlitFramebuffer BlitFramebuffer;
	static PROC_ES3_glRenderbufferStorageMultisample RenderbufferStorageMultisample;
	static PROC_ES3_glFramebufferTextureLayer FramebufferTextureLayer;
	static PROC_ES3_glMapBufferRange MapBufferRange;
	static PROC_ES3_glFlushMappedBufferRange FlushMappedBufferRange;
	static PROC_ES3_glBindVertexArray BindVertexArray;
	static PROC_ES3_glDeleteVertexArrays DeleteVertexArrays;
	static PROC_ES3_glGenVertexArrays GenVertexArrays;
	static PROC_ES3_glIsVertexArray IsVertexArray;
	static PROC_ES3_glGetIntegeri_v GetIntegeri_v;
	static PROC_ES3_glBeginTransformFeedback BeginTransformFeedback;
	static PROC_ES3_glEndTransformFeedback EndTransformFeedback;
	static PROC_ES3_glBindBufferRange BindBufferRange;
	static PROC_ES3_glBindBufferBase BindBufferBase;
	static PROC_ES3_glTransformFeedbackVaryings TransformFeedbackVaryings;
	static PROC_ES3_glGetTransformFeedbackVarying GetTransformFeedbackVarying;
	static PROC_ES3_glVertexAttribIPointer VertexAttribIPointer;
	static PROC_ES3_glGetVertexAttribIiv GetVertexAttribIiv;
	static PROC_ES3_glGetVertexAttribIuiv GetVertexAttribIuiv;
	static PROC_ES3_glVertexAttribI4i VertexAttribI4i;
	static PROC_ES3_glVertexAttribI4ui VertexAttribI4ui;
	static PROC_ES3_glVertexAttribI4iv VertexAttribI4iv;
	static PROC_ES3_glVertexAttribI4uiv VertexAttribI4uiv;
	static PROC_ES3_glGetUniformuiv GetUniformuiv;
	static PROC_ES3_glGetFragDataLocation GetFragDataLocation;
	static PROC_ES3_glUniform1ui Uniform1ui;
	static PROC_ES3_glUniform2ui Uniform2ui;
	static PROC_ES3_glUniform3ui Uniform3ui;
	static PROC_ES3_glUniform4ui Uniform4ui;
	static PROC_ES3_glUniform1uiv Uniform1uiv;
	static PROC_ES3_glUniform2uiv Uniform2uiv;
	static PROC_ES3_glUniform3uiv Uniform3uiv;
	static PROC_ES3_glUniform4uiv Uniform4uiv;
	static PROC_ES3_glClearBufferiv ClearBufferiv;
	static PROC_ES3_glClearBufferuiv ClearBufferuiv;
	static PROC_ES3_glClearBufferfv ClearBufferfv;
	static PROC_ES3_glClearBufferfi ClearBufferfi;
	static PROC_ES3_glGetStringi GetStringi;
	static PROC_ES3_glCopyBufferSubData CopyBufferSubData;
	static PROC_ES3_glGetUniformIndices GetUniformIndices;
	static PROC_ES3_glGetActiveUniformsiv GetActiveUniformsiv;
	static PROC_ES3_glGetUniformBlockIndex GetUniformBlockIndex;
	static PROC_ES3_glGetActiveUniformBlockiv GetActiveUniformBlockiv;
	static PROC_ES3_glGetActiveUniformBlockName GetActiveUniformBlockName;
	static PROC_ES3_glUniformBlockBinding UniformBlockBinding;
	static PROC_ES3_glDrawArraysInstanced DrawArraysInstanced;
	static PROC_ES3_glDrawElementsInstanced DrawElementsInstanced;
	static PROC_ES3_glFenceSync FenceSync;
	static PROC_ES3_glIsSync IsSync;
	static PROC_ES3_glDeleteSync DeleteSync;
	static PROC_ES3_glClientWaitSync ClientWaitSync;
	static PROC_ES3_glWaitSync WaitSync;
	static PROC_ES3_glGetInteger64v GetInteger64v;
	static PROC_ES3_glGetSynciv GetSynciv;
	static PROC_ES3_glGetInteger64i_v GetInteger64i_v;
	static PROC_ES3_glGetBufferParameteri64v GetBufferParameteri64v;
	static PROC_ES3_glGenSamplers GenSamplers;
	static PROC_ES3_glDeleteSamplers DeleteSamplers;
	static PROC_ES3_glIsSampler IsSampler;
	static PROC_ES3_glBindSampler BindSampler;
	static PROC_ES3_glSamplerParameteri SamplerParameteri;
	static PROC_ES3_glSamplerParameteriv SamplerParameteriv;
	static PROC_ES3_glSamplerParameterf SamplerParameterf;
	static PROC_ES3_glSamplerParameterfv SamplerParameterfv;
	static PROC_ES3_glGetSamplerParameteriv GetSamplerParameteriv;
	static PROC_ES3_glGetSamplerParameterfv GetSamplerParameterfv;
	static PROC_ES3_glVertexAttribDivisor VertexAttribDivisor;
	static PROC_ES3_glBindTransformFeedback BindTransformFeedback;
	static PROC_ES3_glDeleteTransformFeedbacks DeleteTransformFeedbacks;
	static PROC_ES3_glGenTransformFeedbacks GenTransformFeedbacks;
	static PROC_ES3_glIsTransformFeedback IsTransformFeedback;
	static PROC_ES3_glPauseTransformFeedback PauseTransformFeedback;
	static PROC_ES3_glResumeTransformFeedback ResumeTransformFeedback;
	static PROC_ES3_glGetProgramBinary GetProgramBinary;
	static PROC_ES3_glProgramBinary ProgramBinary;
	static PROC_ES3_glProgramParameteri ProgramParameteri;
	static PROC_ES3_glInvalidateFramebuffer InvalidateFramebuffer;
	static PROC_ES3_glInvalidateSubFramebuffer InvalidateSubFramebuffer;
	static PROC_ES3_glTexStorage2D TexStorage2D;
	static PROC_ES3_glTexStorage3D TexStorage3D;
	static PROC_ES3_glGetInternalformativ GetInternalformativ;
#endif

#if BUILD_API_MAX>=3
	// Es3.1 functions
	static PROC_ES31_glDispatchCompute DispatchCompute;
	static PROC_ES31_glDispatchComputeIndirect DispatchComputeIndirect;
	static PROC_ES31_glBindImageTexture BindImageTexture;
	static PROC_ES31_glMemoryBarrier MemoryBarrier;
	static PROC_ES31_glMemoryBarrierByRegion MemoryBarrierByRegion;
	static PROC_ES31_glBindProgramPipeline BindProgramPipeline;
	static PROC_ES31_glDeleteProgramPipelines DeleteProgramPipelines;
	static PROC_ES31_glGenProgramPipelines GenProgramPipelines;
	static PROC_ES31_glIsProgramPipeline IsProgramPipeline;
	static PROC_ES31_glGetProgramPipelineiv GetProgramPipelineiv;
	static PROC_ES31_glValidateProgramPipeline ValidateProgramPipeline;
	static PROC_ES31_glGetProgramPipelineInfoLog GetProgramPipelineInfoLog;
	static PROC_ES31_glUseProgramStages UseProgramStages;
	static PROC_ES31_glActiveShaderProgram ActiveShaderProgram;
	static PROC_ES31_glProgramUniform1i ProgramUniform1i;
	static PROC_ES31_glProgramUniform2i ProgramUniform2i;
	static PROC_ES31_glProgramUniform3i ProgramUniform3i;
	static PROC_ES31_glProgramUniform4i ProgramUniform4i;
	static PROC_ES31_glProgramUniform1ui ProgramUniform1ui;
	static PROC_ES31_glProgramUniform2ui ProgramUniform2ui;
	static PROC_ES31_glProgramUniform3ui ProgramUniform3ui;
	static PROC_ES31_glProgramUniform4ui ProgramUniform4ui;
	static PROC_ES31_glProgramUniform1f ProgramUniform1f;
	static PROC_ES31_glProgramUniform2f ProgramUniform2f;
	static PROC_ES31_glProgramUniform3f ProgramUniform3f;
	static PROC_ES31_glProgramUniform4f ProgramUniform4f;
	static PROC_ES31_glProgramUniform1iv ProgramUniform1iv;
	static PROC_ES31_glProgramUniform2iv ProgramUniform2iv;
	static PROC_ES31_glProgramUniform3iv ProgramUniform3iv;
	static PROC_ES31_glProgramUniform4iv ProgramUniform4iv;
	static PROC_ES31_glProgramUniform1uiv ProgramUniform1uiv;
	static PROC_ES31_glProgramUniform2uiv ProgramUniform2uiv;
	static PROC_ES31_glProgramUniform3uiv ProgramUniform3uiv;
	static PROC_ES31_glProgramUniform4uiv ProgramUniform4uiv;
	static PROC_ES31_glProgramUniform1fv ProgramUniform1fv;
	static PROC_ES31_glProgramUniform2fv ProgramUniform2fv;
	static PROC_ES31_glProgramUniform3fv ProgramUniform3fv;
	static PROC_ES31_glProgramUniform4fv ProgramUniform4fv;
	static PROC_ES31_glProgramUniformMatrix2fv ProgramUniformMatrix2fv;
	static PROC_ES31_glProgramUniformMatrix3fv ProgramUniformMatrix3fv;
	static PROC_ES31_glProgramUniformMatrix4fv ProgramUniformMatrix4fv;
	static PROC_ES31_glProgramUniformMatrix2x3fv ProgramUniformMatrix2x3fv;
	static PROC_ES31_glProgramUniformMatrix3x2fv ProgramUniformMatrix3x2fv;
	static PROC_ES31_glProgramUniformMatrix2x4fv ProgramUniformMatrix2x4fv;
	static PROC_ES31_glProgramUniformMatrix4x2fv ProgramUniformMatrix4x2fv;
	static PROC_ES31_glProgramUniformMatrix3x4fv ProgramUniformMatrix3x4fv;
	static PROC_ES31_glProgramUniformMatrix4x3fv ProgramUniformMatrix4x3fv;
	static PROC_ES31_glCreateShaderProgramv CreateShaderProgramv;
	static PROC_ES31_glDrawArraysIndirect DrawArraysIndirect;
	static PROC_ES31_glDrawElementsIndirect DrawElementsIndirect;
	static PROC_ES31_glTexStorage2DMultisample TexStorage2DMultisample;
	static PROC_ES31_glSampleMaski SampleMaski;
	static PROC_ES31_glBindVertexBuffer BindVertexBuffer;
	static PROC_ES31_glVertexAttribFormat VertexAttribFormat;
	static PROC_ES31_glVertexAttribIFormat VertexAttribIFormat;
	static PROC_ES31_glVertexAttribBinding VertexAttribBinding;
	static PROC_ES31_glVertexBindingDivisor VertexBindingDivisor;
	static PROC_ES31_glFramebufferParameteri FramebufferParameteri;
	static PROC_ES31_glGetFramebufferParameteriv GetFramebufferParameteriv;
	static PROC_ES31_glGetProgramInterfaceiv GetProgramInterfaceiv;
	static PROC_ES31_glGetProgramResourceIndex GetProgramResourceIndex;
	static PROC_ES31_glGetProgramResourceName GetProgramResourceName;
	static PROC_ES31_glGetProgramResourceiv GetProgramResourceiv;
	static PROC_ES31_glGetProgramResourceLocation GetProgramResourceLocation;
	static PROC_ES31_glGetBooleani_v GetBooleani_v;
	static PROC_ES31_glGetMultisamplefv GetMultisamplefv;
	static PROC_ES31_glGetTexLevelParameteriv GetTexLevelParameteriv;
	static PROC_ES31_glGetTexLevelParameterfv GetTexLevelParameterfv;
#endif
};
