/*!*********************************************************************************************************************
\file         PVRNativeApi\OGLES\ExtensionLoaderGles.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Definitions for the function pointers of the glext:: class. See ExtensionLoaderGles.h.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRNativeApi/OGLES/ExtensionLoaderGles.h"
#include "PVRPlatformGlue/ExtensionLoader.h"
#include "PVRCore/Assert_.h"

PROC_EXT_glDiscardFramebufferEXT glext::DiscardFramebufferEXT = NULL;
PROC_EXT_glMultiDrawElementsEXT glext::MultiDrawElementsEXT = NULL;
PROC_EXT_glMultiDrawArraysEXT glext::MultiDrawArraysEXT = NULL;
PROC_EXT_glMapBufferOES glext::MapBufferOES = NULL;
PROC_EXT_glUnmapBufferOES glext::UnmapBufferOES = NULL;
PROC_EXT_glGetBufferPointervOES glext::GetBufferPointervOES = NULL;
PROC_EXT_glBindVertexArrayOES glext::BindVertexArrayOES = NULL;
PROC_EXT_glDeleteVertexArraysOES glext::DeleteVertexArraysOES = NULL;
PROC_EXT_glGenVertexArraysOES glext::GenVertexArraysOES = NULL;
PROC_EXT_glIsVertexArrayOES glext::IsVertexArrayOES = NULL;
PROC_EXT_glDeleteFencesNV glext::DeleteFencesNV = NULL;
PROC_EXT_glGenFencesNV glext::GenFencesNV = NULL;
PROC_EXT_glIsFenceNV glext::IsFenceNV = NULL;
PROC_EXT_glTestFenceNV glext::TestFenceNV = NULL;
PROC_EXT_glGetFenceivNV glext::GetFenceivNV = NULL;
PROC_EXT_glFinishFenceNV glext::FinishFenceNV = NULL;
PROC_EXT_glSetFenceNV glext::SetFenceNV = NULL;
#ifndef TARGET_OS_IPHONE
PROC_EXT_glEGLImageTargetRenderbufferStorageOES glext::EGLImageTargetRenderbufferStorageOES = NULL;
PROC_EXT_glEGLImageTargetTexture2DOES glext::EGLImageTargetTexture2DOES = NULL;
#endif
PROC_EXT_glBlendEquationSeparateOES glext::BlendEquationSeparateOES = NULL;
PROC_EXT_glBlendFuncSeparateOES glext::BlendFuncSeparateOES = NULL;
PROC_EXT_glBlendEquationOES glext::BlendEquationOES = NULL;
PROC_EXT_glQueryMatrixxOES glext::QueryMatrixxOES = NULL;

PROC_EXT_glCopyTextureLevelsAPPLE glext::CopyTextureLevelsAPPLE = NULL;
PROC_EXT_glRenderbufferStorageMultisampleAPPLE glext::RenderbufferStorageMultisampleAPPLE = NULL;
PROC_EXT_glResolveMultisampleFramebufferAPPLE glext::ResolveMultisampleFramebufferAPPLE = NULL;
PROC_EXT_glFenceSyncAPPLE glext::FenceSyncAPPLE = NULL;
PROC_EXT_glIsSyncAPPLE glext::IsSyncAPPLE = NULL;
PROC_EXT_glDeleteSyncAPPLE glext::DeleteSyncAPPLE = NULL;
PROC_EXT_glClientWaitSyncAPPLE glext::ClientWaitSyncAPPLE = NULL;
PROC_EXT_glWaitSyncAPPLE glext::WaitSyncAPPLE = NULL;
PROC_EXT_glGetInteger64vAPPLE glext::GetInteger64vAPPLE = NULL;
PROC_EXT_glGetSyncivAPPLE glext::GetSyncivAPPLE = NULL;
PROC_EXT_glMapBufferRangeEXT glext::MapBufferRangeEXT = NULL;
PROC_EXT_glFlushMappedBufferRangeEXT glext::FlushMappedBufferRangeEXT = NULL;
PROC_EXT_glRenderbufferStorageMultisampleEXT glext::RenderbufferStorageMultisampleEXT = NULL;
PROC_EXT_glFramebufferTexture2DMultisampleEXT glext::FramebufferTexture2DMultisampleEXT = NULL;
PROC_EXT_glGetGraphicsResetStatusEXT glext::GetGraphicsResetStatusEXT = NULL;
PROC_EXT_glReadnPixelsEXT glext::ReadnPixelsEXT = NULL;
PROC_EXT_glGetnUniformfvEXT glext::GetnUniformfvEXT = NULL;
PROC_EXT_glGetnUniformivEXT glext::GetnUniformivEXT = NULL;
PROC_EXT_glTexStorage1DEXT glext::TexStorage1DEXT = NULL;
PROC_EXT_glTexStorage2DEXT glext::TexStorage2DEXT = NULL;
PROC_EXT_glTexStorage3DEXT glext::TexStorage3DEXT = NULL;
PROC_EXT_glTextureStorage1DEXT glext::TextureStorage1DEXT = NULL;
PROC_EXT_glTextureStorage2DEXT glext::TextureStorage2DEXT = NULL;
PROC_EXT_glTextureStorage3DEXT glext::TextureStorage3DEXT = NULL;

PROC_EXT_glRenderbufferStorageMultisampleIMG glext::RenderbufferStorageMultisampleIMG = NULL;
PROC_EXT_glFramebufferTexture2DMultisampleIMG glext::FramebufferTexture2DMultisampleIMG = NULL;
PROC_EXT_glBlendEquationEXT glext::BlendEquationEXT = NULL;
PROC_EXT_glGetPerfMonitorGroupsAMD glext::GetPerfMonitorGroupsAMD = NULL;
PROC_EXT_glGetPerfMonitorCountersAMD glext::GetPerfMonitorCountersAMD = NULL;
PROC_EXT_glGetPerfMonitorGroupStringAMD glext::GetPerfMonitorGroupStringAMD = NULL;
PROC_EXT_glGetPerfMonitorCounterStringAMD glext::GetPerfMonitorCounterStringAMD = NULL;
PROC_EXT_glGetPerfMonitorCounterInfoAMD glext::GetPerfMonitorCounterInfoAMD = NULL;
PROC_EXT_glGenPerfMonitorsAMD glext::GenPerfMonitorsAMD = NULL;
PROC_EXT_glDeletePerfMonitorsAMD glext::DeletePerfMonitorsAMD = NULL;
PROC_EXT_glSelectPerfMonitorCountersAMD glext::SelectPerfMonitorCountersAMD = NULL;
PROC_EXT_glBeginPerfMonitorAMD glext::BeginPerfMonitorAMD = NULL;
PROC_EXT_glEndPerfMonitorAMD glext::EndPerfMonitorAMD = NULL;
PROC_EXT_glGetPerfMonitorCounterDataAMD glext::GetPerfMonitorCounterDataAMD = NULL;
PROC_EXT_glBlitFramebufferANGLE glext::BlitFramebufferANGLE = NULL;
PROC_EXT_glRenderbufferStorageMultisampleANGLE glext::RenderbufferStorageMultisampleANGLE = NULL;
PROC_EXT_glCoverageMaskNV glext::CoverageMaskNV = NULL;
PROC_EXT_glCoverageOperationNV glext::CoverageOperationNV = NULL;
PROC_EXT_glGetDriverControlsQCOM glext::GetDriverControlsQCOM = NULL;
PROC_EXT_glGetDriverControlStringQCOM glext::GetDriverControlStringQCOM = NULL;
PROC_EXT_glEnableDriverControlQCOM glext::EnableDriverControlQCOM = NULL;
PROC_EXT_glDisableDriverControlQCOM glext::DisableDriverControlQCOM = NULL;
PROC_EXT_glExtGetTexturesQCOM glext::ExtGetTexturesQCOM = NULL;
PROC_EXT_glExtGetBuffersQCOM glext::ExtGetBuffersQCOM = NULL;
PROC_EXT_glExtGetRenderbuffersQCOM glext::ExtGetRenderbuffersQCOM = NULL;
PROC_EXT_glExtGetFramebuffersQCOM glext::ExtGetFramebuffersQCOM = NULL;
PROC_EXT_glExtGetTexLevelParameterivQCOM glext::ExtGetTexLevelParameterivQCOM = NULL;
PROC_EXT_glExtTexObjectStateOverrideiQCOM glext::ExtTexObjectStateOverrideiQCOM = NULL;
PROC_EXT_glExtGetTexSubImageQCOM glext::ExtGetTexSubImageQCOM = NULL;
PROC_EXT_glExtGetBufferPointervQCOM glext::ExtGetBufferPointervQCOM = NULL;
PROC_EXT_glExtGetShadersQCOM glext::ExtGetShadersQCOM = NULL;
PROC_EXT_glExtGetProgramsQCOM glext::ExtGetProgramsQCOM = NULL;
PROC_EXT_glExtIsProgramBinaryQCOM glext::ExtIsProgramBinaryQCOM = NULL;
PROC_EXT_glExtGetProgramBinarySourceQCOM glext::ExtGetProgramBinarySourceQCOM = NULL;
PROC_EXT_glStartTilingQCOM glext::StartTilingQCOM = NULL;
PROC_EXT_glEndTilingQCOM glext::EndTilingQCOM = NULL;
PROC_EXT_glGetProgramBinaryOES glext::GetProgramBinaryOES = NULL;
PROC_EXT_glProgramBinaryOES glext::ProgramBinaryOES = NULL;
PROC_EXT_glTexImage3DOES glext::TexImage3DOES = NULL;
PROC_EXT_glTexSubImage3DOES glext::TexSubImage3DOES = NULL;
PROC_EXT_glCopyTexSubImage3DOES glext::CopyTexSubImage3DOES = NULL;
PROC_EXT_glCompressedTexImage3DOES glext::CompressedTexImage3DOES = NULL;
PROC_EXT_glCompressedTexSubImage3DOES glext::CompressedTexSubImage3DOES = NULL;
PROC_EXT_glFramebufferTexture3DOES glext::FramebufferTexture3DOES = NULL;
PROC_EXT_glDebugMessageControlKHR glext::DebugMessageControlKHR = NULL;
PROC_EXT_glDebugMessageInsertKHR glext::DebugMessageInsertKHR = NULL;
PROC_EXT_glDebugMessageCallbackKHR glext::DebugMessageCallbackKHR = NULL;
PROC_EXT_glGetDebugMessageLogKHR glext::GetDebugMessageLogKHR = NULL;
PROC_EXT_glPushDebugGroupKHR glext::PushDebugGroupKHR = NULL;
PROC_EXT_glPopDebugGroupKHR glext::PopDebugGroupKHR = NULL;
PROC_EXT_glObjectLabelKHR glext::ObjectLabelKHR = NULL;
PROC_EXT_glGetObjectLabelKHR glext::GetObjectLabelKHR = NULL;
PROC_EXT_glObjectPtrLabelKHR glext::ObjectPtrLabelKHR = NULL;
PROC_EXT_glGetObjectPtrLabelKHR glext::GetObjectPtrLabelKHR = NULL;
PROC_EXT_glGetPointervKHR glext::GetPointervKHR = NULL;
PROC_EXT_glDrawArraysInstancedANGLE glext::DrawArraysInstancedANGLE = NULL;
PROC_EXT_glDrawElementsInstancedANGLE glext::DrawElementsInstancedANGLE = NULL;
PROC_EXT_glVertexAttribDivisorANGLE glext::VertexAttribDivisorANGLE = NULL;
PROC_EXT_glGetTranslatedShaderSourceANGLE glext::GetTranslatedShaderSourceANGLE = NULL;
PROC_EXT_glLabelObjectEXT glext::LabelObjectEXT = NULL;
PROC_EXT_glGetObjectLabelEXT glext::GetObjectLabelEXT = NULL;
PROC_EXT_glInsertEventMarkerEXT glext::InsertEventMarkerEXT = NULL;
PROC_EXT_glPushGroupMarkerEXT glext::PushGroupMarkerEXT = NULL;
PROC_EXT_glPopGroupMarkerEXT glext::PopGroupMarkerEXT = NULL;
PROC_EXT_glGenQueriesEXT glext::GenQueriesEXT = NULL;
PROC_EXT_glDeleteQueriesEXT glext::DeleteQueriesEXT = NULL;
PROC_EXT_glIsQueryEXT glext::IsQueryEXT = NULL;
PROC_EXT_glBeginQueryEXT glext::BeginQueryEXT = NULL;
PROC_EXT_glEndQueryEXT glext::EndQueryEXT = NULL;
PROC_EXT_glGetQueryivEXT glext::GetQueryivEXT = NULL;
PROC_EXT_glGetQueryObjectuivEXT glext::GetQueryObjectuivEXT = NULL;
PROC_EXT_glUseProgramStagesEXT glext::UseProgramStagesEXT = NULL;
PROC_EXT_glActiveShaderProgramEXT glext::ActiveShaderProgramEXT = NULL;
PROC_EXT_glCreateShaderProgramvEXT glext::CreateShaderProgramvEXT = NULL;
PROC_EXT_glBindProgramPipelineEXT glext::BindProgramPipelineEXT = NULL;
PROC_EXT_glDeleteProgramPipelinesEXT glext::DeleteProgramPipelinesEXT = NULL;
PROC_EXT_glGenProgramPipelinesEXT glext::GenProgramPipelinesEXT = NULL;
PROC_EXT_glIsProgramPipelineEXT glext::IsProgramPipelineEXT = NULL;
PROC_EXT_glProgramParameteriEXT glext::ProgramParameteriEXT = NULL;
PROC_EXT_glGetProgramPipelineivEXT glext::GetProgramPipelineivEXT = NULL;
PROC_EXT_glProgramUniform1iEXT glext::ProgramUniform1iEXT = NULL;
PROC_EXT_glProgramUniform2iEXT glext::ProgramUniform2iEXT = NULL;
PROC_EXT_glProgramUniform3iEXT glext::ProgramUniform3iEXT = NULL;
PROC_EXT_glProgramUniform4iEXT glext::ProgramUniform4iEXT = NULL;
PROC_EXT_glProgramUniform1fEXT glext::ProgramUniform1fEXT = NULL;
PROC_EXT_glProgramUniform2fEXT glext::ProgramUniform2fEXT = NULL;
PROC_EXT_glProgramUniform3fEXT glext::ProgramUniform3fEXT = NULL;
PROC_EXT_glProgramUniform4fEXT glext::ProgramUniform4fEXT = NULL;
PROC_EXT_glProgramUniform1ivEXT glext::ProgramUniform1ivEXT = NULL;
PROC_EXT_glProgramUniform2ivEXT glext::ProgramUniform2ivEXT = NULL;
PROC_EXT_glProgramUniform3ivEXT glext::ProgramUniform3ivEXT = NULL;
PROC_EXT_glProgramUniform4ivEXT glext::ProgramUniform4ivEXT = NULL;
PROC_EXT_glProgramUniform1fvEXT glext::ProgramUniform1fvEXT = NULL;
PROC_EXT_glProgramUniform2fvEXT glext::ProgramUniform2fvEXT = NULL;
PROC_EXT_glProgramUniform3fvEXT glext::ProgramUniform3fvEXT = NULL;
PROC_EXT_glProgramUniform4fvEXT glext::ProgramUniform4fvEXT = NULL;
PROC_EXT_glProgramUniformMatrix2fvEXT glext::ProgramUniformMatrix2fvEXT = NULL;
PROC_EXT_glProgramUniformMatrix3fvEXT glext::ProgramUniformMatrix3fvEXT = NULL;
PROC_EXT_glProgramUniformMatrix4fvEXT glext::ProgramUniformMatrix4fvEXT = NULL;
PROC_EXT_glValidateProgramPipelineEXT glext::ValidateProgramPipelineEXT = NULL;
PROC_EXT_glGetProgramPipelineInfoLogEXT glext::GetProgramPipelineInfoLogEXT = NULL;
PROC_EXT_glProgramUniform1uiEXT glext::ProgramUniform1uiEXT = NULL;
PROC_EXT_glProgramUniform2uiEXT glext::ProgramUniform2uiEXT = NULL;
PROC_EXT_glProgramUniform3uiEXT glext::ProgramUniform3uiEXT = NULL;
PROC_EXT_glProgramUniform4uiEXT glext::ProgramUniform4uiEXT = NULL;
PROC_EXT_glProgramUniform1uivEXT glext::ProgramUniform1uivEXT = NULL;
PROC_EXT_glProgramUniform2uivEXT glext::ProgramUniform2uivEXT = NULL;
PROC_EXT_glProgramUniform3uivEXT glext::ProgramUniform3uivEXT = NULL;
PROC_EXT_glProgramUniform4uivEXT glext::ProgramUniform4uivEXT = NULL;
PROC_EXT_glProgramUniformMatrix2x3fvEXT glext::ProgramUniformMatrix2x3fvEXT = NULL;
PROC_EXT_glProgramUniformMatrix3x2fvEXT glext::ProgramUniformMatrix3x2fvEXT = NULL;
PROC_EXT_glProgramUniformMatrix2x4fvEXT glext::ProgramUniformMatrix2x4fvEXT = NULL;
PROC_EXT_glProgramUniformMatrix4x2fvEXT glext::ProgramUniformMatrix4x2fvEXT = NULL;
PROC_EXT_glProgramUniformMatrix3x4fvEXT glext::ProgramUniformMatrix3x4fvEXT = NULL;
PROC_EXT_glProgramUniformMatrix4x3fvEXT glext::ProgramUniformMatrix4x3fvEXT = NULL;

PROC_EXT_glAlphaFuncQCOM glext::AlphaFuncQCOM = NULL;
PROC_EXT_glReadBufferNV glext::ReadBufferNV = NULL;
PROC_EXT_glDrawBuffersNV glext::DrawBuffersNV = NULL;
PROC_EXT_glReadBufferIndexedEXT glext::ReadBufferIndexedEXT = NULL;
PROC_EXT_glDrawBuffersIndexedEXT glext::DrawBuffersIndexedEXT = NULL;
PROC_EXT_glGetIntegeri_vEXT glext::GetIntegeri_vEXT = NULL;
PROC_EXT_glDrawBuffersEXT glext::DrawBuffersEXT = NULL;
PROC_EXT_glBlendBarrierKHR glext::BlendBarrierKHR = NULL;
PROC_EXT_glTexStorage3DMultisampleOES glext::TexStorage3DMultisampleOES = NULL;
// GL_OVR_multiview
PROC_EXT_glFramebufferTextureMultiviewOVR glext::FramebufferTextureMultiviewOVR = NULL;

/* PLS2 */
PROC_EXT_glFramebufferPixelLocalStorageSize glext::FramebufferPixelLocalStorageSize = NULL;
PROC_EXT_glClearPixelLocalStorageui glext::ClearPixelLocalStorageui = NULL;
PROC_EXT_glGetFramebufferPixelLocalStorageSize glext::GetFramebufferPixelLocalStorageSize = NULL;

/* Buffer Storage EXT */
PROC_EXT_glBufferStorageEXT glext::BufferStorageEXT = NULL;

/* GL_IMG_clear_texture */
PROC_EXT_glClearTexImageIMG glext::ClearTexImageIMG = NULL;
PROC_EXT_glClearTexSubImageIMG glext::ClearTexSubImageIMG = NULL;

/* GL_IMG_framebuffer_downsample */
PROC_EXT_glFramebufferTexture2DDownsampleIMG glext::FramebufferTexture2DDownsampleIMG = NULL;
PROC_EXT_glFramebufferTextureLayerDownsampleIMG glext::FramebufferTextureLayerDownsampleIMG = NULL;

/* GL_EXT_tessellation_shader */
PROC_EXT_glPatchParameteriEXT glext::PatchParameteriEXT = NULL;

namespace pvr {
namespace native {
static inline bool isExtensionSupported(const char* const extensionString, const char* const extension)
{
	if (!extensionString) {  return false; }

	// The recommended technique for querying OpenGL extensions;
	// from http://opengl.org/resources/features/OGLextensions/
	const char* start = extensionString;
	char* position, *terminator;

	// Extension names should not have spaces.
	position = (char*)strchr(extension, ' ');

	if (position || *extension == '\0') { return 0; }

	/* It takes a bit of care to be fool-proof about parsing the
	OpenGL extensions string. Don't be fooled by sub-strings, etc. */
	for (;;)
	{
		position = (char*)strstr((char*)start, extension);

		if (!position) { break; }

		terminator = position + strlen(extension);

		if (position == start || *(position - 1) == ' ')
		{
			if (*terminator == ' ' || *terminator == '\0')
			{
				return true;
			}
		}

		start = terminator;
	}

	return false;
}

}
}
void glext::initGlext()
{


#ifndef TARGET_OS_IPHONE
// GL_EXT_discard_framebuffer
	glext::DiscardFramebufferEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glDiscardFramebufferEXT>("glDiscardFramebufferEXT");
	// GL_EXT_multi_draw_arrays
	glext::MultiDrawElementsEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glMultiDrawElementsEXT>("glMultiDrawElementsEXT");
	glext::MultiDrawArraysEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glMultiDrawArraysEXT>("glMultiDrawArraysEXT");

	// GL_OES_mapbuffer
	glext::MapBufferOES = pvr::native::getExtensionProcAddress<PROC_EXT_glMapBufferOES>("glMapBufferOES");
	glext::UnmapBufferOES = pvr::native::getExtensionProcAddress<PROC_EXT_glUnmapBufferOES>("glUnmapBufferOES");
	glext::GetBufferPointervOES = pvr::native::getExtensionProcAddress<PROC_EXT_glGetBufferPointervOES>("glGetBufferPointervOES");

	// GL_OES_vertex_array_object
	glext::BindVertexArrayOES = pvr::native::getExtensionProcAddress<PROC_EXT_glBindVertexArrayOES>("glBindVertexArrayOES");
	glext::DeleteVertexArraysOES = pvr::native::getExtensionProcAddress<PROC_EXT_glDeleteVertexArraysOES>("glDeleteVertexArraysOES");
	glext::GenVertexArraysOES = pvr::native::getExtensionProcAddress<PROC_EXT_glGenVertexArraysOES>("glGenVertexArraysOES");
	glext::IsVertexArrayOES = pvr::native::getExtensionProcAddress<PROC_EXT_glIsVertexArrayOES>("glIsVertexArrayOES");

	// GL_NV_fence
	glext::DeleteFencesNV = pvr::native::getExtensionProcAddress<PROC_EXT_glDeleteFencesNV>("glDeleteFencesNV");
	glext::GenFencesNV = pvr::native::getExtensionProcAddress<PROC_EXT_glGenFencesNV>("glGenFencesNV");
	glext::IsFenceNV = pvr::native::getExtensionProcAddress<PROC_EXT_glIsFenceNV>("glIsFenceNV");
	glext::TestFenceNV = pvr::native::getExtensionProcAddress<PROC_EXT_glTestFenceNV>("glTestFenceNV");
	glext::GetFenceivNV = pvr::native::getExtensionProcAddress<PROC_EXT_glGetFenceivNV>("glGetFenceivNV");
	glext::FinishFenceNV = pvr::native::getExtensionProcAddress<PROC_EXT_glFinishFenceNV>("glFinishFenceNV");
	glext::SetFenceNV = pvr::native::getExtensionProcAddress<PROC_EXT_glSetFenceNV>("glSetFenceNV");

	// GL_OES_EGL_image and GL_OES_EGL_image_external
	glext::EGLImageTargetRenderbufferStorageOES =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glEGLImageTargetRenderbufferStorageOES>("glEGLImageTargetRenderbufferStorageOES");
	glext::EGLImageTargetTexture2DOES =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glEGLImageTargetTexture2DOES>("glEGLImageTargetTexture2DOES");

	// GL_OES_blend_equation_separate
	glext::BlendEquationSeparateOES =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glBlendEquationSeparateOES>("glBlendEquationSeparateOES");

	// GL_OES_blend_func_separate
	glext::BlendFuncSeparateOES = pvr::native::getExtensionProcAddress<PROC_EXT_glBlendFuncSeparateOES>("glBlendFuncSeparateOES");

	// GL_OES_blend_subtract
	glext::BlendEquationOES = pvr::native::getExtensionProcAddress<PROC_EXT_glBlendEquationOES>("glBlendEquationOES");


	// GL_OES_query_matrix
	glext::QueryMatrixxOES = pvr::native::getExtensionProcAddress<PROC_EXT_glQueryMatrixxOES>("glQueryMatrixxOES");

	// GL_APPLE_copy_texture_levels
	glext::CopyTextureLevelsAPPLE =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glCopyTextureLevelsAPPLE>("glCopyTextureLevelsAPPLE");


	// GL_APPLE_framebuffer_multisample
	glext::RenderbufferStorageMultisampleAPPLE =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glRenderbufferStorageMultisampleAPPLE>("glRenderbufferStorageMultisampleAPPLE");
	glext::ResolveMultisampleFramebufferAPPLE =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glResolveMultisampleFramebufferAPPLE>("glResolveMultisampleFramebufferAPPLE");

	// GL_APPLE_sync
	glext::FenceSyncAPPLE = pvr::native::getExtensionProcAddress<PROC_EXT_glFenceSyncAPPLE>("glFenceSyncAPPLE");
	glext::IsSyncAPPLE = pvr::native::getExtensionProcAddress<PROC_EXT_glIsSyncAPPLE>("glIsSyncAPPLE");
	glext::DeleteSyncAPPLE = pvr::native::getExtensionProcAddress<PROC_EXT_glDeleteSyncAPPLE>("glDeleteSyncAPPLE");
	glext::ClientWaitSyncAPPLE = pvr::native::getExtensionProcAddress<PROC_EXT_glClientWaitSyncAPPLE>("glClientWaitSyncAPPLE");
	glext::WaitSyncAPPLE = pvr::native::getExtensionProcAddress<PROC_EXT_glWaitSyncAPPLE>("glWaitSyncAPPLE");
	glext::GetInteger64vAPPLE = pvr::native::getExtensionProcAddress<PROC_EXT_glGetInteger64vAPPLE>("glGetInteger64vAPPLE");
	glext::GetSyncivAPPLE = pvr::native::getExtensionProcAddress<PROC_EXT_glGetSyncivAPPLE>("glGetSyncivAPPLE");

	// GL_EXT_map_buffer_range
	glext::MapBufferRangeEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glMapBufferRangeEXT>("glMapBufferRangeEXT");
	glext::FlushMappedBufferRangeEXT =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glFlushMappedBufferRangeEXT>("glFlushMappedBufferRangeEXT");

	// GL_EXT_multisampled_render_to_texture
	glext::RenderbufferStorageMultisampleEXT =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glRenderbufferStorageMultisampleEXT>("glRenderbufferStorageMultisampleEXT");
	glext::FramebufferTexture2DMultisampleEXT =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glFramebufferTexture2DMultisampleEXT>("glFramebufferTexture2DMultisampleEXT");

	// GL_EXT_robustness
	glext::GetGraphicsResetStatusEXT =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glGetGraphicsResetStatusEXT>("glGetGraphicsResetStatusEXT");
	glext::ReadnPixelsEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glReadnPixelsEXT>("glReadnPixelsEXT");
	glext::GetnUniformfvEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glGetnUniformfvEXT>("glGetnUniformfvEXT");
	glext::GetnUniformivEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glGetnUniformivEXT>("glGetnUniformivEXT");

	// GL_EXT_texture_storage
	glext::TexStorage1DEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glTexStorage1DEXT>("glTexStorage1DEXT");
	glext::TexStorage2DEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glTexStorage2DEXT>("glTexStorage2DEXT");
	glext::TexStorage3DEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glTexStorage3DEXT>("glTexStorage3DEXT");
	glext::TextureStorage1DEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glTextureStorage1DEXT>("glTextureStorage1DEXT");
	glext::TextureStorage2DEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glTextureStorage2DEXT>("glTextureStorage2DEXT");
	glext::TextureStorage3DEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glTextureStorage3DEXT>("glTextureStorage3DEXT");

	// GL_IMG_multisampled_render_to_texture
	glext::RenderbufferStorageMultisampleIMG =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glRenderbufferStorageMultisampleIMG>("glRenderbufferStorageMultisampleIMG");
	glext::FramebufferTexture2DMultisampleIMG =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glFramebufferTexture2DMultisampleIMG>("glFramebufferTexture2DMultisampleIMG");

	// GL_EXT_blend_minmax
	glext::BlendEquationEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glBlendEquationEXT>("glBlendEquationEXT");

	// GL_AMD_performance_monitor
	glext::GetPerfMonitorGroupsAMD =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glGetPerfMonitorGroupsAMD>("glGetPerfMonitorGroupsAMD");
	glext::GetPerfMonitorCountersAMD =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glGetPerfMonitorCountersAMD>("glGetPerfMonitorCountersAMD");
	glext::GetPerfMonitorGroupStringAMD =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glGetPerfMonitorGroupStringAMD>("glGetPerfMonitorGroupStringAMD");
	glext::GetPerfMonitorCounterStringAMD =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glGetPerfMonitorCounterStringAMD>("glGetPerfMonitorCounterStringAMD");
	glext::GetPerfMonitorCounterInfoAMD =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glGetPerfMonitorCounterInfoAMD>("glGetPerfMonitorCounterInfoAMD");
	glext::GenPerfMonitorsAMD = pvr::native::getExtensionProcAddress<PROC_EXT_glGenPerfMonitorsAMD>("glGenPerfMonitorsAMD");
	glext::DeletePerfMonitorsAMD = pvr::native::getExtensionProcAddress<PROC_EXT_glDeletePerfMonitorsAMD>("glDeletePerfMonitorsAMD");
	glext::SelectPerfMonitorCountersAMD =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glSelectPerfMonitorCountersAMD>("glSelectPerfMonitorCountersAMD");
	glext::BeginPerfMonitorAMD = pvr::native::getExtensionProcAddress<PROC_EXT_glBeginPerfMonitorAMD>("glBeginPerfMonitorAMD");
	glext::EndPerfMonitorAMD = pvr::native::getExtensionProcAddress<PROC_EXT_glEndPerfMonitorAMD>("glEndPerfMonitorAMD");
	glext::GetPerfMonitorCounterDataAMD =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glGetPerfMonitorCounterDataAMD>("glGetPerfMonitorCounterDataAMD");

	// GL_ANGLE_framebuffer_blit
	glext::BlitFramebufferANGLE = pvr::native::getExtensionProcAddress<PROC_EXT_glBlitFramebufferANGLE>("glBlitFramebufferANGLE");

	// GL_ANGLE_framebuffer_multisample
	glext::RenderbufferStorageMultisampleANGLE =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glRenderbufferStorageMultisampleANGLE>("glRenderbufferStorageMultisampleANGLE");

	// GL_NV_coverage_sample
	glext::CoverageMaskNV = pvr::native::getExtensionProcAddress<PROC_EXT_glCoverageMaskNV>("glCoverageMaskNV");
	glext::CoverageOperationNV = pvr::native::getExtensionProcAddress<PROC_EXT_glCoverageOperationNV>("glCoverageOperationNV");

	// GL_QCOM_driver_control
	glext::GetDriverControlsQCOM = pvr::native::getExtensionProcAddress<PROC_EXT_glGetDriverControlsQCOM>("glGetDriverControlsQCOM");
	glext::GetDriverControlStringQCOM =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glGetDriverControlStringQCOM>("glGetDriverControlStringQCOM");
	glext::EnableDriverControlQCOM =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glEnableDriverControlQCOM>("glEnableDriverControlQCOM");
	glext::DisableDriverControlQCOM =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glDisableDriverControlQCOM>("glDisableDriverControlQCOM");

	// GL_QCOM_extended_get
	glext::ExtGetTexturesQCOM = pvr::native::getExtensionProcAddress<PROC_EXT_glExtGetTexturesQCOM>("glExtGetTexturesQCOM");
	glext::ExtGetBuffersQCOM = pvr::native::getExtensionProcAddress<PROC_EXT_glExtGetBuffersQCOM>("glExtGetBuffersQCOM");
	glext::ExtGetRenderbuffersQCOM =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glExtGetRenderbuffersQCOM>("glExtGetRenderbuffersQCOM");
	glext::ExtGetFramebuffersQCOM =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glExtGetFramebuffersQCOM>("glExtGetFramebuffersQCOM");
	glext::ExtGetTexLevelParameterivQCOM =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glExtGetTexLevelParameterivQCOM>("glExtGetTexLevelParameterivQCOM");
	glext::ExtTexObjectStateOverrideiQCOM =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glExtTexObjectStateOverrideiQCOM>("glExtTexObjectStateOverrideiQCOM");
	glext::ExtGetTexSubImageQCOM = pvr::native::getExtensionProcAddress<PROC_EXT_glExtGetTexSubImageQCOM>("glExtGetTexSubImageQCOM");
	glext::ExtGetBufferPointervQCOM =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glExtGetBufferPointervQCOM>("glExtGetBufferPointervQCOM");

	// GL_QCOM_extended_get2
	glext::ExtGetShadersQCOM = pvr::native::getExtensionProcAddress<PROC_EXT_glExtGetShadersQCOM>("glExtGetShadersQCOM");
	glext::ExtGetProgramsQCOM = pvr::native::getExtensionProcAddress<PROC_EXT_glExtGetProgramsQCOM>("glExtGetProgramsQCOM");
	glext::ExtIsProgramBinaryQCOM =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glExtIsProgramBinaryQCOM>("glExtIsProgramBinaryQCOM");
	glext::ExtGetProgramBinarySourceQCOM =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glExtGetProgramBinarySourceQCOM>("glExtGetProgramBinarySourceQCOM");

	// GL_QCOM_tiled_rendering
	glext::StartTilingQCOM = pvr::native::getExtensionProcAddress<PROC_EXT_glStartTilingQCOM>("glStartTilingQCOM");
	glext::EndTilingQCOM = pvr::native::getExtensionProcAddress<PROC_EXT_glEndTilingQCOM>("glEndTilingQCOM");

	// GL_OES_get_program_binary
	glext::GetProgramBinaryOES = pvr::native::getExtensionProcAddress<PROC_EXT_glGetProgramBinaryOES>("glGetProgramBinaryOES");
	glext::ProgramBinaryOES = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramBinaryOES>("glProgramBinaryOES");

	// GL_OES_texture_3D
	glext::TexImage3DOES = pvr::native::getExtensionProcAddress<PROC_EXT_glTexImage3DOES>("glTexImage3DOES");
	glext::TexSubImage3DOES = pvr::native::getExtensionProcAddress<PROC_EXT_glTexSubImage3DOES>("glTexSubImage3DOES");
	glext::CopyTexSubImage3DOES = pvr::native::getExtensionProcAddress<PROC_EXT_glCopyTexSubImage3DOES>("glCopyTexSubImage3DOES");
	glext::CompressedTexImage3DOES =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glCompressedTexImage3DOES>("glCompressedTexImage3DOES");
	glext::CompressedTexSubImage3DOES =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glCompressedTexSubImage3DOES>("glCompressedTexSubImage3DOES");
	glext::FramebufferTexture3DOES =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glFramebufferTexture3DOES>("glFramebufferTexture3DOES");

	// GL_KHR_debug
	glext::DebugMessageControlKHR =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glDebugMessageControlKHR>("glDebugMessageControlKHR", "glDebugMessageControl");
	glext::DebugMessageInsertKHR = pvr::native::getExtensionProcAddress<PROC_EXT_glDebugMessageInsertKHR>("glDebugMessageInsertKHR",
	                               "glDebugMessageInsert");
	glext::DebugMessageCallbackKHR =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glDebugMessageCallbackKHR>("glDebugMessageCallbackKHR", "glDebugMessageCallback");
	glext::GetDebugMessageLogKHR = pvr::native::getExtensionProcAddress<PROC_EXT_glGetDebugMessageLogKHR>("glGetDebugMessageLogKHR",
	                               "glGetDebugMessageLog");
	glext::PushDebugGroupKHR = pvr::native::getExtensionProcAddress<PROC_EXT_glPushDebugGroupKHR>("glPushDebugGroupKHR",
	                           "glPushDebugGroup");
	glext::PopDebugGroupKHR = pvr::native::getExtensionProcAddress<PROC_EXT_glPopDebugGroupKHR>("glPopDebugGroupKHR",
	                          "glPopDebugGroup");
	glext::ObjectLabelKHR = pvr::native::getExtensionProcAddress<PROC_EXT_glObjectLabelKHR>("glObjectLabelKHR", "glObjectLabel");
	glext::GetObjectLabelKHR = pvr::native::getExtensionProcAddress<PROC_EXT_glGetObjectLabelKHR>("glGetObjectLabelKHR",
	                           "glGetObjectLabel");
	glext::ObjectPtrLabelKHR = pvr::native::getExtensionProcAddress<PROC_EXT_glObjectPtrLabelKHR>("glObjectPtrLabelKHR",
	                           "glObjectPtrLabel");
	glext::GetObjectPtrLabelKHR = pvr::native::getExtensionProcAddress<PROC_EXT_glGetObjectPtrLabelKHR>("glGetObjectPtrLabelKHR",
	                              "glGetObjectPtrLabel");
	glext::GetPointervKHR = pvr::native::getExtensionProcAddress<PROC_EXT_glGetPointervKHR>("glGetPointervKHR", "glGetPointerv");

	// GL_ANGLE_instanced_arrays
	glext::DrawArraysInstancedANGLE =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glDrawArraysInstancedANGLE>("glDrawArraysInstancedANGLE");
	glext::DrawElementsInstancedANGLE =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glDrawElementsInstancedANGLE>("glDrawElementsInstancedANGLE");
	glext::VertexAttribDivisorANGLE =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glVertexAttribDivisorANGLE>("glVertexAttribDivisorANGLE");

	// GL_ANGLE_texture_usage
	glext::GetTranslatedShaderSourceANGLE =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glGetTranslatedShaderSourceANGLE>("glGetTranslatedShaderSourceANGLE");

	// GL_EXT_debug_label
	glext::LabelObjectEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glLabelObjectEXT>("glLabelObjectEXT");
	glext::GetObjectLabelEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glGetObjectLabelEXT>("glGetObjectLabelEXT");

	// GL_EXT_debug_marker
	glext::InsertEventMarkerEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glInsertEventMarkerEXT>("glInsertEventMarkerEXT");
	glext::PushGroupMarkerEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glPushGroupMarkerEXT>("glPushGroupMarkerEXT");
	glext::PopGroupMarkerEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glPopGroupMarkerEXT>("glPopGroupMarkerEXT");

	// GL_EXT_occlusion_query_boolean
	glext::GenQueriesEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glGenQueriesEXT>("glGenQueriesEXT");
	glext::DeleteQueriesEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glDeleteQueriesEXT>("glDeleteQueriesEXT");
	glext::IsQueryEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glIsQueryEXT>("glIsQueryEXT");
	glext::BeginQueryEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glBeginQueryEXT>("glBeginQueryEXT");
	glext::EndQueryEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glEndQueryEXT>("glEndQueryEXT");
	glext::GetQueryivEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glGetQueryivEXT>("glGetQueryivEXT");
	glext::GetQueryObjectuivEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glGetQueryObjectuivEXT>("glGetQueryObjectuivEXT");

	// GL_EXT_separate_shader_objects
	glext::UseProgramStagesEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glUseProgramStagesEXT>("glUseProgramStagesEXT");
	glext::ActiveShaderProgramEXT =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glActiveShaderProgramEXT>("glActiveShaderProgramEXT");
	glext::CreateShaderProgramvEXT =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glCreateShaderProgramvEXT>("glCreateShaderProgramvEXT");
	glext::BindProgramPipelineEXT =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glBindProgramPipelineEXT>("glBindProgramPipelineEXT");
	glext::DeleteProgramPipelinesEXT =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glDeleteProgramPipelinesEXT>("glDeleteProgramPipelinesEXT");
	glext::GenProgramPipelinesEXT =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glGenProgramPipelinesEXT>("glGenProgramPipelinesEXT");
	glext::IsProgramPipelineEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glIsProgramPipelineEXT>("glIsProgramPipelineEXT");
	glext::ProgramParameteriEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramParameteriEXT>("glProgramParameteriEXT");
	glext::GetProgramPipelineivEXT =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glGetProgramPipelineivEXT>("glGetProgramPipelineivEXT");
	glext::ProgramUniform1iEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniform1iEXT>("glProgramUniform1iEXT");
	glext::ProgramUniform2iEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniform2iEXT>("glProgramUniform2iEXT");
	glext::ProgramUniform3iEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniform3iEXT>("glProgramUniform3iEXT");
	glext::ProgramUniform4iEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniform4iEXT>("glProgramUniform4iEXT");
	glext::ProgramUniform1fEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniform1fEXT>("glProgramUniform1fEXT");
	glext::ProgramUniform2fEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniform2fEXT>("glProgramUniform2fEXT");
	glext::ProgramUniform3fEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniform3fEXT>("glProgramUniform3fEXT");
	glext::ProgramUniform4fEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniform4fEXT>("glProgramUniform4fEXT");
	glext::ProgramUniform1ivEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniform1ivEXT>("glProgramUniform1ivEXT");
	glext::ProgramUniform2ivEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniform2ivEXT>("glProgramUniform2ivEXT");
	glext::ProgramUniform3ivEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniform3ivEXT>("glProgramUniform3ivEXT");
	glext::ProgramUniform4ivEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniform4ivEXT>("glProgramUniform4ivEXT");
	glext::ProgramUniform1fvEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniform1fvEXT>("glProgramUniform1fvEXT");
	glext::ProgramUniform2fvEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniform2fvEXT>("glProgramUniform2fvEXT");
	glext::ProgramUniform3fvEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniform3fvEXT>("glProgramUniform3fvEXT");
	glext::ProgramUniform4fvEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniform4fvEXT>("glProgramUniform4fvEXT");
	glext::ProgramUniformMatrix2fvEXT =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniformMatrix2fvEXT>("glProgramUniformMatrix2fvEXT");
	glext::ProgramUniformMatrix3fvEXT =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniformMatrix3fvEXT>("glProgramUniformMatrix3fvEXT");
	glext::ProgramUniformMatrix4fvEXT =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniformMatrix4fvEXT>("glProgramUniformMatrix4fvEXT");
	glext::ValidateProgramPipelineEXT =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glValidateProgramPipelineEXT>("glValidateProgramPipelineEXT");
	glext::GetProgramPipelineInfoLogEXT =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glGetProgramPipelineInfoLogEXT>("glGetProgramPipelineInfoLogEXT");
	glext::ProgramUniform1uiEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniform1uiEXT>("glProgramUniform1uiEXT");
	glext::ProgramUniform2uiEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniform2uiEXT>("glProgramUniform2uiEXT");
	glext::ProgramUniform3uiEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniform3uiEXT>("glProgramUniform3uiEXT");
	glext::ProgramUniform4uiEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniform4uiEXT>("glProgramUniform4uiEXT");
	glext::ProgramUniform1uivEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniform1uivEXT>("glProgramUniform1uivEXT");
	glext::ProgramUniform2uivEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniform2uivEXT>("glProgramUniform2uivEXT");
	glext::ProgramUniform3uivEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniform3uivEXT>("glProgramUniform3uivEXT");
	glext::ProgramUniform4uivEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniform4uivEXT>("glProgramUniform4uivEXT");
	glext::ProgramUniformMatrix2x3fvEXT =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniformMatrix2x3fvEXT>("glProgramUniformMatrix2x3fvEXT");
	glext::ProgramUniformMatrix3x2fvEXT =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniformMatrix3x2fvEXT>("glProgramUniformMatrix3x2fvEXT");
	glext::ProgramUniformMatrix2x4fvEXT =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniformMatrix2x4fvEXT>("glProgramUniformMatrix2x4fvEXT");
	glext::ProgramUniformMatrix4x2fvEXT =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniformMatrix4x2fvEXT>("glProgramUniformMatrix4x2fvEXT");
	glext::ProgramUniformMatrix3x4fvEXT =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniformMatrix3x4fvEXT>("glProgramUniformMatrix3x4fvEXT");
	glext::ProgramUniformMatrix4x3fvEXT =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glProgramUniformMatrix4x3fvEXT>("glProgramUniformMatrix4x3fvEXT");

	// GL_QCOM_alpha_test
	glext::AlphaFuncQCOM = pvr::native::getExtensionProcAddress<PROC_EXT_glAlphaFuncQCOM>("glAlphaFuncQCOM");

	// GL_NV_read_buffer
	glext::ReadBufferNV = pvr::native::getExtensionProcAddress<PROC_EXT_glReadBufferNV>("glReadBufferNV");

	// GL_NV_draw_buffers
	glext::DrawBuffersNV = pvr::native::getExtensionProcAddress<PROC_EXT_glDrawBuffersNV>("glDrawBuffersNV");

	// GL_EXT_multiview_draw_buffers
	glext::ReadBufferIndexedEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glReadBufferIndexedEXT>("glReadBufferIndexedEXT");
	glext::DrawBuffersIndexedEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glDrawBuffersIndexedEXT>("glDrawBuffersIndexedEXT");
	glext::GetIntegeri_vEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glGetIntegeri_vEXT>("glGetIntegeri_vEXT");

	// GL_EXT_draw_buffers
	glext::DrawBuffersEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glDrawBuffersEXT>("glDrawBuffersEXT");

	// GL_OES_texture_half_float
	// GL_OES_packed_depth_stencil
	// GL_OES_texture_float
	// GL_KHR_blend_equation_advanced
	glext::BlendBarrierKHR = pvr::native::getExtensionProcAddress<PROC_EXT_glBlendBarrierKHR>("glBlendBarrierKHR");

	// GL_OES_texture_storage_multisample_2d_array
	glext::TexStorage3DMultisampleOES =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glTexStorage3DMultisampleOES>("glTexStorage3DMultisampleOES");

	// GL_EXT_texture_cube_map_array
	glext::TexStorage3DMultisampleOES =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glTexStorage3DMultisampleOES>("glTexStorage3DMultisampleOES");

	// OVR_multiview
	glext::FramebufferTextureMultiviewOVR =
	  pvr::native::getExtensionProcAddress<PROC_EXT_glFramebufferTextureMultiviewOVR>("glFramebufferTextureMultiviewOVR");

	/* PLS2 */
	glext::FramebufferPixelLocalStorageSize = pvr::native::getExtensionProcAddress<PROC_EXT_glFramebufferPixelLocalStorageSize>("glFramebufferPixelLocalStorageSizeEXT");
	glext::ClearPixelLocalStorageui = pvr::native::getExtensionProcAddress<PROC_EXT_glClearPixelLocalStorageui>("glClearPixelLocalStorageuiEXT");
	glext::GetFramebufferPixelLocalStorageSize = pvr::native::getExtensionProcAddress<PROC_EXT_glGetFramebufferPixelLocalStorageSize>("glGetFramebufferPixelLocalStorageSizeEXT");

	/* Buffer Storage EXT */
	glext::BufferStorageEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glBufferStorageEXT>("glBufferStorageEXT");

	/* GL_IMG_clear_texture */
	glext::ClearTexImageIMG = pvr::native::getExtensionProcAddress<PROC_EXT_glClearTexImageIMG>("glClearTexImageIMG");
	glext::ClearTexSubImageIMG = pvr::native::getExtensionProcAddress<PROC_EXT_glClearTexSubImageIMG>("glClearTexSubImageIMG");

	/* GL_IMG_framebuffer_downsample */
	glext::FramebufferTexture2DDownsampleIMG = pvr::native::getExtensionProcAddress<PROC_EXT_glFramebufferTexture2DDownsampleIMG>("glFramebufferTexture2DDownsampleIMG");
	glext::FramebufferTextureLayerDownsampleIMG = pvr::native::getExtensionProcAddress<PROC_EXT_glFramebufferTextureLayerDownsampleIMG>("glFramebufferTextureLayerDownsampleIMG");

	glext::PatchParameteriEXT = pvr::native::getExtensionProcAddress<PROC_EXT_glPatchParameteriEXT>("glPatchParameteriEXT");
#else //TARGET_OS_IPHONE

// GL_EXT_discard_framebuffer
	glext::DiscardFramebufferEXT = &glDiscardFramebufferEXT;
//glext::GenVertexArraysOES = &glGenVertexArraysOES;
//	glext::IsVertexArrayOES = &glIsVertexArrayOES;

// GL_APPLE_copy_texture_levels
	glext::CopyTextureLevelsAPPLE = &glCopyTextureLevelsAPPLE;


// GL_APPLE_framebuffer_multisample
	glext::RenderbufferStorageMultisampleAPPLE = &glRenderbufferStorageMultisampleAPPLE;
	glext::ResolveMultisampleFramebufferAPPLE = &glResolveMultisampleFramebufferAPPLE;

// GL_APPLE_sync
	glext::FenceSyncAPPLE = &glFenceSyncAPPLE;
	glext::IsSyncAPPLE = &glIsSyncAPPLE;
	glext::DeleteSyncAPPLE = &glDeleteSyncAPPLE;
	glext::ClientWaitSyncAPPLE = &glClientWaitSyncAPPLE;
	glext::WaitSyncAPPLE = &glWaitSyncAPPLE;
	glext::GetInteger64vAPPLE = &glGetInteger64vAPPLE;
	glext::GetSyncivAPPLE = &glGetSyncivAPPLE;

// GL_EXT_map_buffer_range
	glext::MapBufferRangeEXT = &glMapBufferRangeEXT;
	glext::FlushMappedBufferRangeEXT = &glFlushMappedBufferRangeEXT;

// GL_EXT_multisampled_render_to_texture
	glext::RenderbufferStorageMultisampleEXT = &glRenderbufferStorageMultisample;


// GL_EXT_texture_storage
	glext::TexStorage2DEXT = &glTexStorage2DEXT;

// GL_EXT_debug_label
	glext::LabelObjectEXT = &glLabelObjectEXT;
	glext::GetObjectLabelEXT = &glGetObjectLabelEXT;

// GL_EXT_debug_marker
	glext::InsertEventMarkerEXT = &glInsertEventMarkerEXT;
	glext::PushGroupMarkerEXT = &glPushGroupMarkerEXT;
	glext::PopGroupMarkerEXT = &glPopGroupMarkerEXT;


// GL_EXT_separate_shader_objects
	glext::UseProgramStagesEXT = &glUseProgramStagesEXT;
	glext::ActiveShaderProgramEXT = &glActiveShaderProgramEXT;
//	glext::CreateShaderProgramvEXT = &glCreateShaderProgramvEXT;
	glext::BindProgramPipelineEXT = &glBindProgramPipelineEXT;
	glext::DeleteProgramPipelinesEXT = &glDeleteProgramPipelinesEXT;
	glext::GenProgramPipelinesEXT = &glGenProgramPipelinesEXT;
	glext::IsProgramPipelineEXT = &glIsProgramPipelineEXT;
	glext::ProgramParameteriEXT = &glProgramParameteriEXT;
	glext::GetProgramPipelineivEXT = &glGetProgramPipelineivEXT;
	glext::ProgramUniform1iEXT = &glProgramUniform1iEXT;
	glext::ProgramUniform2iEXT = &glProgramUniform2iEXT;
	glext::ProgramUniform3iEXT = &glProgramUniform3iEXT;
	glext::ProgramUniform4iEXT = &glProgramUniform4iEXT;
	glext::ProgramUniform1fEXT = &glProgramUniform1fEXT;
	glext::ProgramUniform2fEXT = &glProgramUniform2fEXT;
	glext::ProgramUniform3fEXT = &glProgramUniform3fEXT;
	glext::ProgramUniform4fEXT = &glProgramUniform4fEXT;
	glext::ProgramUniform1ivEXT = &glProgramUniform1ivEXT;
	glext::ProgramUniform2ivEXT = &glProgramUniform2ivEXT;
	glext::ProgramUniform3ivEXT = &glProgramUniform3ivEXT;
	glext::ProgramUniform4ivEXT = &glProgramUniform4ivEXT;
	glext::ProgramUniform1fvEXT = &glProgramUniform1fvEXT;
	glext::ProgramUniform2fvEXT = &glProgramUniform2fvEXT;
	glext::ProgramUniform3fvEXT = &glProgramUniform3fvEXT;
	glext::ProgramUniform4fvEXT = &glProgramUniform4fvEXT;
	glext::ProgramUniformMatrix2fvEXT = &glProgramUniformMatrix2fvEXT;
	glext::ProgramUniformMatrix3fvEXT = &glProgramUniformMatrix3fvEXT;
	glext::ProgramUniformMatrix4fvEXT = &glProgramUniformMatrix4fvEXT;
	glext::ValidateProgramPipelineEXT = &glValidateProgramPipelineEXT;
	glext::GetProgramPipelineInfoLogEXT = &glGetProgramPipelineInfoLogEXT;
	glext::ProgramUniform1uiEXT = &glProgramUniform1uiEXT;
	glext::ProgramUniform2uiEXT = &glProgramUniform2uiEXT;
	glext::ProgramUniform3uiEXT = &glProgramUniform3uiEXT;
	glext::ProgramUniform4uiEXT = &glProgramUniform4uiEXT;
	glext::ProgramUniform1uivEXT = &glProgramUniform1uivEXT;
	glext::ProgramUniform2uivEXT = &glProgramUniform2uivEXT;
	glext::ProgramUniform3uivEXT = &glProgramUniform3uivEXT;
	glext::ProgramUniform4uivEXT = &glProgramUniform4uivEXT;
	glext::ProgramUniformMatrix2x3fvEXT = &glProgramUniformMatrix2x3fvEXT;
	glext::ProgramUniformMatrix3x2fvEXT = &glProgramUniformMatrix3x2fvEXT;
	glext::ProgramUniformMatrix2x4fvEXT = &glProgramUniformMatrix2x4fvEXT;
	glext::ProgramUniformMatrix4x2fvEXT = &glProgramUniformMatrix4x2fvEXT;
	glext::ProgramUniformMatrix3x4fvEXT = &glProgramUniformMatrix3x4fvEXT;
	glext::ProgramUniformMatrix4x3fvEXT = &glProgramUniformMatrix4x3fvEXT;
#endif
}
//!\endcond
