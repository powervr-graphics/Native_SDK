/*!*********************************************************************************************************************
\file         PVRNativeApi\OGLES\ExtensionLoaderGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the glext:: class, with function pointers for all OpenGL ES 2/3 extensions. The function pointers are
              populated on start, but they must only be used when a context for which they are supported is bound.
***********************************************************************************************************************/
#pragma once
#include <cstring>
#include "ApiGlesExt.h"

/*!*********************************************************************************************************************
\brief    Contains function pointers for all OpenGL ES 2/3 extensions. The function pointers are populated when the glext::initGlext()
         is called (normally, PVR Shell calls this), but they must only be used when a context for which they are supported is
     bound.
***********************************************************************************************************************/
class glext
{
public:
	// GL_EXT_discard_framebuffer
	static PROC_EXT_glDiscardFramebufferEXT DiscardFramebufferEXT;

	// GL_EXT_multi_draw_arrays
	static PROC_EXT_glMultiDrawElementsEXT MultiDrawElementsEXT;
	static PROC_EXT_glMultiDrawArraysEXT MultiDrawArraysEXT;

	// GL_OES_mapbuffer
	static PROC_EXT_glMapBufferOES MapBufferOES;
	static PROC_EXT_glUnmapBufferOES UnmapBufferOES;
	static PROC_EXT_glGetBufferPointervOES GetBufferPointervOES;

	// GL_OES_vertex_array_object
	static PROC_EXT_glBindVertexArrayOES BindVertexArrayOES;
	static PROC_EXT_glDeleteVertexArraysOES DeleteVertexArraysOES;
	static PROC_EXT_glGenVertexArraysOES GenVertexArraysOES;
	static PROC_EXT_glIsVertexArrayOES IsVertexArrayOES;

	// GL_NV_fence
	static PROC_EXT_glDeleteFencesNV DeleteFencesNV;
	static PROC_EXT_glGenFencesNV GenFencesNV;
	static PROC_EXT_glIsFenceNV IsFenceNV;
	static PROC_EXT_glTestFenceNV TestFenceNV;
	static PROC_EXT_glGetFenceivNV GetFenceivNV;
	static PROC_EXT_glFinishFenceNV FinishFenceNV;
	static PROC_EXT_glSetFenceNV SetFenceNV;

#ifndef TARGET_OS_IPHONE
	// GL_OES_EGL_image and GL_OES_EGL_image_external
	static PROC_EXT_glEGLImageTargetRenderbufferStorageOES EGLImageTargetRenderbufferStorageOES;
	static PROC_EXT_glEGLImageTargetTexture2DOES EGLImageTargetTexture2DOES;
#endif


	// GL_OES_blend_equation_separate
	static PROC_EXT_glBlendEquationSeparateOES BlendEquationSeparateOES;

	// GL_OES_blend_func_separate
	static PROC_EXT_glBlendFuncSeparateOES BlendFuncSeparateOES;

	// GL_OES_blend_subtract
	static PROC_EXT_glBlendEquationOES BlendEquationOES;

	// GL_OES_query_matrix
	static PROC_EXT_glQueryMatrixxOES QueryMatrixxOES;

	// GL_APPLE_copy_texture_levels
	static PROC_EXT_glCopyTextureLevelsAPPLE CopyTextureLevelsAPPLE;


	// GL_APPLE_framebuffer_multisample
	static PROC_EXT_glRenderbufferStorageMultisampleAPPLE RenderbufferStorageMultisampleAPPLE;
	static PROC_EXT_glResolveMultisampleFramebufferAPPLE ResolveMultisampleFramebufferAPPLE;

	// GL_APPLE_sync
	static PROC_EXT_glFenceSyncAPPLE FenceSyncAPPLE;
	static PROC_EXT_glIsSyncAPPLE IsSyncAPPLE;
	static PROC_EXT_glDeleteSyncAPPLE DeleteSyncAPPLE;
	static PROC_EXT_glClientWaitSyncAPPLE ClientWaitSyncAPPLE;
	static PROC_EXT_glWaitSyncAPPLE WaitSyncAPPLE;
	static PROC_EXT_glGetInteger64vAPPLE GetInteger64vAPPLE;
	static PROC_EXT_glGetSyncivAPPLE GetSyncivAPPLE;

	// GL_EXT_map_buffer_range
	static PROC_EXT_glMapBufferRangeEXT MapBufferRangeEXT;
	static PROC_EXT_glFlushMappedBufferRangeEXT FlushMappedBufferRangeEXT;

	// GL_EXT_multisampled_render_to_texture
	static PROC_EXT_glRenderbufferStorageMultisampleEXT RenderbufferStorageMultisampleEXT;
	static PROC_EXT_glFramebufferTexture2DMultisampleEXT FramebufferTexture2DMultisampleEXT;

	// GL_EXT_robustness
	static PROC_EXT_glGetGraphicsResetStatusEXT GetGraphicsResetStatusEXT;
	static PROC_EXT_glReadnPixelsEXT ReadnPixelsEXT;
	static PROC_EXT_glGetnUniformfvEXT GetnUniformfvEXT;
	static PROC_EXT_glGetnUniformivEXT GetnUniformivEXT;

	// GL_EXT_texture_storage
	static PROC_EXT_glTexStorage1DEXT TexStorage1DEXT;
	static PROC_EXT_glTexStorage2DEXT TexStorage2DEXT;
	static PROC_EXT_glTexStorage3DEXT TexStorage3DEXT;
	static PROC_EXT_glTextureStorage1DEXT TextureStorage1DEXT;
	static PROC_EXT_glTextureStorage2DEXT TextureStorage2DEXT;
	static PROC_EXT_glTextureStorage3DEXT TextureStorage3DEXT;

	// GL_IMG_multisampled_render_to_texture
	static PROC_EXT_glRenderbufferStorageMultisampleIMG RenderbufferStorageMultisampleIMG;
	static PROC_EXT_glFramebufferTexture2DMultisampleIMG FramebufferTexture2DMultisampleIMG;

	// GL_EXT_blend_minmax
	static PROC_EXT_glBlendEquationEXT BlendEquationEXT;

	// GL_AMD_performance_monitor
	static PROC_EXT_glGetPerfMonitorGroupsAMD GetPerfMonitorGroupsAMD;
	static PROC_EXT_glGetPerfMonitorCountersAMD GetPerfMonitorCountersAMD;
	static PROC_EXT_glGetPerfMonitorGroupStringAMD GetPerfMonitorGroupStringAMD;
	static PROC_EXT_glGetPerfMonitorCounterStringAMD GetPerfMonitorCounterStringAMD;
	static PROC_EXT_glGetPerfMonitorCounterInfoAMD GetPerfMonitorCounterInfoAMD;
	static PROC_EXT_glGenPerfMonitorsAMD GenPerfMonitorsAMD;
	static PROC_EXT_glDeletePerfMonitorsAMD DeletePerfMonitorsAMD;
	static PROC_EXT_glSelectPerfMonitorCountersAMD SelectPerfMonitorCountersAMD;
	static PROC_EXT_glBeginPerfMonitorAMD BeginPerfMonitorAMD;
	static PROC_EXT_glEndPerfMonitorAMD EndPerfMonitorAMD;
	static PROC_EXT_glGetPerfMonitorCounterDataAMD GetPerfMonitorCounterDataAMD;

	// GL_ANGLE_framebuffer_blit
	static PROC_EXT_glBlitFramebufferANGLE BlitFramebufferANGLE;

	// GL_ANGLE_framebuffer_multisample
	static PROC_EXT_glRenderbufferStorageMultisampleANGLE RenderbufferStorageMultisampleANGLE;

	// GL_NV_coverage_sample
	static PROC_EXT_glCoverageMaskNV CoverageMaskNV;
	static PROC_EXT_glCoverageOperationNV CoverageOperationNV;

	// GL_QCOM_driver_control
	static PROC_EXT_glGetDriverControlsQCOM GetDriverControlsQCOM;
	static PROC_EXT_glGetDriverControlStringQCOM GetDriverControlStringQCOM;
	static PROC_EXT_glEnableDriverControlQCOM EnableDriverControlQCOM;
	static PROC_EXT_glDisableDriverControlQCOM DisableDriverControlQCOM;

	// GL_QCOM_extended_get
	static PROC_EXT_glExtGetTexturesQCOM ExtGetTexturesQCOM;
	static PROC_EXT_glExtGetBuffersQCOM ExtGetBuffersQCOM;
	static PROC_EXT_glExtGetRenderbuffersQCOM ExtGetRenderbuffersQCOM;
	static PROC_EXT_glExtGetFramebuffersQCOM ExtGetFramebuffersQCOM;
	static PROC_EXT_glExtGetTexLevelParameterivQCOM ExtGetTexLevelParameterivQCOM;
	static PROC_EXT_glExtTexObjectStateOverrideiQCOM ExtTexObjectStateOverrideiQCOM;
	static PROC_EXT_glExtGetTexSubImageQCOM ExtGetTexSubImageQCOM;
	static PROC_EXT_glExtGetBufferPointervQCOM ExtGetBufferPointervQCOM;

	// GL_QCOM_extended_get2
	static PROC_EXT_glExtGetShadersQCOM ExtGetShadersQCOM;
	static PROC_EXT_glExtGetProgramsQCOM ExtGetProgramsQCOM;
	static PROC_EXT_glExtIsProgramBinaryQCOM ExtIsProgramBinaryQCOM;
	static PROC_EXT_glExtGetProgramBinarySourceQCOM ExtGetProgramBinarySourceQCOM;

	// GL_QCOM_tiled_rendering
	static PROC_EXT_glStartTilingQCOM StartTilingQCOM;
	static PROC_EXT_glEndTilingQCOM EndTilingQCOM;

	// GL_OES_get_program_binary
	static PROC_EXT_glGetProgramBinaryOES GetProgramBinaryOES;
	static PROC_EXT_glProgramBinaryOES ProgramBinaryOES;

	// GL_OES_texture_3D
	static PROC_EXT_glTexImage3DOES TexImage3DOES;
	static PROC_EXT_glTexSubImage3DOES TexSubImage3DOES;
	static PROC_EXT_glCopyTexSubImage3DOES CopyTexSubImage3DOES;
	static PROC_EXT_glCompressedTexImage3DOES CompressedTexImage3DOES;
	static PROC_EXT_glCompressedTexSubImage3DOES CompressedTexSubImage3DOES;
	static PROC_EXT_glFramebufferTexture3DOES FramebufferTexture3DOES;

	// GL_KHR_debug
	static PROC_EXT_glDebugMessageControlKHR DebugMessageControlKHR;
	static PROC_EXT_glDebugMessageInsertKHR DebugMessageInsertKHR;
	static PROC_EXT_glDebugMessageCallbackKHR DebugMessageCallbackKHR;
	static PROC_EXT_glGetDebugMessageLogKHR GetDebugMessageLogKHR;
	static PROC_EXT_glPushDebugGroupKHR PushDebugGroupKHR;
	static PROC_EXT_glPopDebugGroupKHR PopDebugGroupKHR;
	static PROC_EXT_glObjectLabelKHR ObjectLabelKHR;
	static PROC_EXT_glGetObjectLabelKHR GetObjectLabelKHR;
	static PROC_EXT_glObjectPtrLabelKHR ObjectPtrLabelKHR;
	static PROC_EXT_glGetObjectPtrLabelKHR GetObjectPtrLabelKHR;
	static PROC_EXT_glGetPointervKHR GetPointervKHR;

	// GL_ANGLE_instanced_arrays
	static PROC_EXT_glDrawArraysInstancedANGLE DrawArraysInstancedANGLE;
	static PROC_EXT_glDrawElementsInstancedANGLE DrawElementsInstancedANGLE;
	static PROC_EXT_glVertexAttribDivisorANGLE VertexAttribDivisorANGLE;

	// GL_ANGLE_texture_usage
	static PROC_EXT_glGetTranslatedShaderSourceANGLE GetTranslatedShaderSourceANGLE;

	// GL_EXT_debug_label
	static PROC_EXT_glLabelObjectEXT LabelObjectEXT;
	static PROC_EXT_glGetObjectLabelEXT GetObjectLabelEXT;

	// GL_EXT_debug_marker
	static PROC_EXT_glInsertEventMarkerEXT InsertEventMarkerEXT;
	static PROC_EXT_glPushGroupMarkerEXT PushGroupMarkerEXT;
	static PROC_EXT_glPopGroupMarkerEXT PopGroupMarkerEXT;

	// GL_EXT_occlusion_query_boolean
	static PROC_EXT_glGenQueriesEXT GenQueriesEXT;
	static PROC_EXT_glDeleteQueriesEXT DeleteQueriesEXT;
	static PROC_EXT_glIsQueryEXT IsQueryEXT;
	static PROC_EXT_glBeginQueryEXT BeginQueryEXT;
	static PROC_EXT_glEndQueryEXT EndQueryEXT;
	static PROC_EXT_glGetQueryivEXT GetQueryivEXT;
	static PROC_EXT_glGetQueryObjectuivEXT GetQueryObjectuivEXT;

	// GL_EXT_separate_shader_objects
	static PROC_EXT_glUseProgramStagesEXT UseProgramStagesEXT;
	static PROC_EXT_glActiveShaderProgramEXT ActiveShaderProgramEXT;
	static PROC_EXT_glCreateShaderProgramvEXT CreateShaderProgramvEXT;
	static PROC_EXT_glBindProgramPipelineEXT BindProgramPipelineEXT;
	static PROC_EXT_glDeleteProgramPipelinesEXT DeleteProgramPipelinesEXT;
	static PROC_EXT_glGenProgramPipelinesEXT GenProgramPipelinesEXT;
	static PROC_EXT_glIsProgramPipelineEXT IsProgramPipelineEXT;
	static PROC_EXT_glProgramParameteriEXT ProgramParameteriEXT;
	static PROC_EXT_glGetProgramPipelineivEXT GetProgramPipelineivEXT;
	static PROC_EXT_glProgramUniform1iEXT ProgramUniform1iEXT;
	static PROC_EXT_glProgramUniform2iEXT ProgramUniform2iEXT;
	static PROC_EXT_glProgramUniform3iEXT ProgramUniform3iEXT;
	static PROC_EXT_glProgramUniform4iEXT ProgramUniform4iEXT;
	static PROC_EXT_glProgramUniform1fEXT ProgramUniform1fEXT;
	static PROC_EXT_glProgramUniform2fEXT ProgramUniform2fEXT;
	static PROC_EXT_glProgramUniform3fEXT ProgramUniform3fEXT;
	static PROC_EXT_glProgramUniform4fEXT ProgramUniform4fEXT;
	static PROC_EXT_glProgramUniform1ivEXT ProgramUniform1ivEXT;
	static PROC_EXT_glProgramUniform2ivEXT ProgramUniform2ivEXT;
	static PROC_EXT_glProgramUniform3ivEXT ProgramUniform3ivEXT;
	static PROC_EXT_glProgramUniform4ivEXT ProgramUniform4ivEXT;
	static PROC_EXT_glProgramUniform1fvEXT ProgramUniform1fvEXT;
	static PROC_EXT_glProgramUniform2fvEXT ProgramUniform2fvEXT;
	static PROC_EXT_glProgramUniform3fvEXT ProgramUniform3fvEXT;
	static PROC_EXT_glProgramUniform4fvEXT ProgramUniform4fvEXT;
	static PROC_EXT_glProgramUniformMatrix2fvEXT ProgramUniformMatrix2fvEXT;
	static PROC_EXT_glProgramUniformMatrix3fvEXT ProgramUniformMatrix3fvEXT;
	static PROC_EXT_glProgramUniformMatrix4fvEXT ProgramUniformMatrix4fvEXT;
	static PROC_EXT_glValidateProgramPipelineEXT ValidateProgramPipelineEXT;
	static PROC_EXT_glGetProgramPipelineInfoLogEXT GetProgramPipelineInfoLogEXT;
	static PROC_EXT_glProgramUniform1uiEXT ProgramUniform1uiEXT;
	static PROC_EXT_glProgramUniform2uiEXT ProgramUniform2uiEXT;
	static PROC_EXT_glProgramUniform3uiEXT ProgramUniform3uiEXT;
	static PROC_EXT_glProgramUniform4uiEXT ProgramUniform4uiEXT;
	static PROC_EXT_glProgramUniform1uivEXT ProgramUniform1uivEXT;
	static PROC_EXT_glProgramUniform2uivEXT ProgramUniform2uivEXT;
	static PROC_EXT_glProgramUniform3uivEXT ProgramUniform3uivEXT;
	static PROC_EXT_glProgramUniform4uivEXT ProgramUniform4uivEXT;
	static PROC_EXT_glProgramUniformMatrix2x3fvEXT ProgramUniformMatrix2x3fvEXT;
	static PROC_EXT_glProgramUniformMatrix3x2fvEXT ProgramUniformMatrix3x2fvEXT;
	static PROC_EXT_glProgramUniformMatrix2x4fvEXT ProgramUniformMatrix2x4fvEXT;
	static PROC_EXT_glProgramUniformMatrix4x2fvEXT ProgramUniformMatrix4x2fvEXT;
	static PROC_EXT_glProgramUniformMatrix3x4fvEXT ProgramUniformMatrix3x4fvEXT;
	static PROC_EXT_glProgramUniformMatrix4x3fvEXT ProgramUniformMatrix4x3fvEXT;

	// GL_QCOM_alpha_test
	static PROC_EXT_glAlphaFuncQCOM AlphaFuncQCOM;

	// GL_NV_read_buffer
	static PROC_EXT_glReadBufferNV ReadBufferNV;

	// GL_NV_draw_buffers
	static PROC_EXT_glDrawBuffersNV DrawBuffersNV;

	// GL_EXT_multiview_draw_buffers
	static PROC_EXT_glReadBufferIndexedEXT ReadBufferIndexedEXT;
	static PROC_EXT_glDrawBuffersIndexedEXT DrawBuffersIndexedEXT;
	static PROC_EXT_glGetIntegeri_vEXT GetIntegeri_vEXT;

	// GL_EXT_draw_buffers
	static PROC_EXT_glDrawBuffersEXT DrawBuffersEXT;

	// GL_KHR_blend_equation_advanced
	static PROC_EXT_glBlendBarrierKHR BlendBarrierKHR;

	// GL_OES_texture_storage_multisample_2d_array
	static PROC_EXT_glTexStorage3DMultisampleOES TexStorage3DMultisampleOES;

	// GL_OVR_multiview
	static PROC_EXT_glFramebufferTextureMultiviewOVR FramebufferTextureMultiviewOVR;

	static PROC_EXT_glFramebufferPixelLocalStorageSize FramebufferPixelLocalStorageSize;
	static PROC_EXT_glClearPixelLocalStorageui ClearPixelLocalStorageui;
	static PROC_EXT_glGetFramebufferPixelLocalStorageSize GetFramebufferPixelLocalStorageSize;

	static PROC_EXT_glBufferStorageEXT BufferStorageEXT;

	static PROC_EXT_glClearTexImageIMG ClearTexImageIMG;
	static PROC_EXT_glClearTexSubImageIMG ClearTexSubImageIMG;

	static PROC_EXT_glFramebufferTexture2DDownsampleIMG FramebufferTexture2DDownsampleIMG;
	static PROC_EXT_glFramebufferTextureLayerDownsampleIMG FramebufferTextureLayerDownsampleIMG;

	// GL_EXT_tessellation_shader
	static PROC_EXT_glPatchParameteriEXT PatchParameteriEXT;

	static void isExtensionSupported(const char* extension);

	static void initGlext();
};
