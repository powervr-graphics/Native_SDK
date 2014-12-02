/******************************************************************************

 @File         OGLES/PVRTglesExt.cpp

 @Title        OGLES/PVRTglesExt

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  OGLES extensions.

******************************************************************************/
#include <string.h>
#include "PVRTContext.h"

#include "PVRTglesExt.h"

/****************************************************************************
** Macros
****************************************************************************/

/****************************************************************************
** Defines
****************************************************************************/

/****************************************************************************
** Structures
****************************************************************************/

/****************************************************************************
** Declarations
****************************************************************************/

/****************************************************************************
** Constants
****************************************************************************/

/****************************************************************************
** Globals
****************************************************************************/

/****************************************************************************
** Local code
****************************************************************************/

/****************************************************************************
** Class: CPVRTglesExt
****************************************************************************/

/*!***************************************************************************
 @Function			LoadExtensions
 @Description		Initialises IMG extensions
*****************************************************************************/
void CPVRTglesExt::LoadExtensions()
{
	glCurrentPaletteMatrixOES = 0;
	glLoadPaletteFromModelViewMatrixOES	= 0;
	glMatrixIndexPointerOES = 0;
	glWeightPointerOES = 0;
	glBlendEquationOES = 0;
	glBlendEquationSeparateOES  = 0;
	glClipPlanexIMG = 0;
	glClipPlanefIMG = 0;
	glVertexAttribPointerARB = 0;
	glEnableVertexAttribArrayARB = 0;
	glDisableVertexAttribArrayARB = 0;
	glProgramStringARB = 0;
	glBindProgramARB = 0;
	glDeleteProgramsARB = 0;
	glIsProgramARB = 0;
	glGenProgramsARB = 0;
	glVertexAttrib4fvARB = 0;
	glVertexAttrib4xvIMG = 0;
	glProgramLocalParameter4xIMG = 0;
	glProgramLocalParameter4xvIMG = 0;
	glProgramEnvParameter4xIMG = 0;
	glProgramEnvParameter4xvIMG = 0;
	glProgramEnvParameter4fARB = 0;
	glProgramEnvParameter4fvARB = 0;
	glProgramLocalParameter4fARB = 0;
	glProgramLocalParameter4fvARB = 0;
	glDrawTexiOES = 0;
	glDrawTexivOES = 0;
	glDrawTexsOES = 0;
	glDrawTexsvOES = 0;
	glDrawTexxOES = 0;
	glDrawTexxvOES = 0;
	glDrawTexfOES = 0;
	glDrawTexfvOES = 0;
	glGetTexStreamDeviceAttribivIMG = 0;
	glTexBindStreamIMG = 0;
	glGetTexStreamDeviceNameIMG = 0;
	glMultiDrawElementsEXT = 0;
	glMultiDrawArraysEXT = 0;
	glMapBufferOES = 0;
	glUnmapBufferOES = 0;
	glGetBufferPointervOES = 0;
	glIsRenderbufferOES = 0;
	glBindRenderbufferOES = 0;
	glDeleteRenderbuffersOES = 0;
	glGenRenderbuffersOES = 0;
	glRenderbufferStorageOES = 0;
	glGetRenderbufferParameterivOES = 0;
	glIsFramebufferOES = 0;
	glBindFramebufferOES = 0;
	glDeleteFramebuffersOES = 0;
	glGenFramebuffersOES = 0;
	glCheckFramebufferStatusOES = 0;
	glFramebufferTexture2DOES = 0;
	glFramebufferRenderbufferOES = 0;
	glGetFramebufferAttachmentParameterivOES = 0;
	glGenerateMipmapOES = 0;
	glPointSizePointerOES = 0;
    glQueryMatrixxOES = 0;
	glDiscardFramebufferEXT = 0;
	glBindVertexArrayOES = 0;
	glDeleteVertexArraysOES = 0;
	glGenVertexArraysOES = 0;
	glIsVertexArrayOES = 0;

	const GLubyte *pszGLExtensions;

	/* Retrieve GL extension string */
    pszGLExtensions = glGetString(GL_EXTENSIONS);

	/* GL_OES_matrix_palette */
	if (strstr((char *)pszGLExtensions, "GL_OES_matrix_palette"))
	{
		glCurrentPaletteMatrixOES			= (PFNGLCURRENTPALETTEMATRIXOES)PVRGetProcAddress(glCurrentPaletteMatrixOES);
		glLoadPaletteFromModelViewMatrixOES	= (PFNGLLOADPALETTEFROMMODELVIEWMATRIXOES)PVRGetProcAddress(glLoadPaletteFromModelViewMatrixOES);
		glMatrixIndexPointerOES				= (PFNGLMATRIXINDEXPOINTEROES)PVRGetProcAddress(glMatrixIndexPointerOES);
		glWeightPointerOES					= (PFNGLWEIGHTPOINTEROES)PVRGetProcAddress(glWeightPointerOES);
	}

	/* GL_OES_draw_texture */
	if (strstr((char *)pszGLExtensions, "GL_OES_draw_texture"))
	{
		glDrawTexiOES = (PFNGLDRAWTEXIOES)PVRGetProcAddress(glDrawTexiOES);
		glDrawTexivOES = (PFNGLDRAWTEXIVOES)PVRGetProcAddress(glDrawTexivOES);
		glDrawTexsOES = (PFNGLDRAWTEXSOES)PVRGetProcAddress(glDrawTexsOES);
		glDrawTexsvOES = (PFNGLDRAWTEXSVOES)PVRGetProcAddress(glDrawTexsvOES);
		glDrawTexxOES = (PFNGLDRAWTEXXOES)PVRGetProcAddress(glDrawTexxOES);
		glDrawTexxvOES = (PFNGLDRAWTEXXVOES)PVRGetProcAddress(glDrawTexxvOES);
		glDrawTexfOES = (PFNGLDRAWTEXFOES)PVRGetProcAddress(glDrawTexfOES);
		glDrawTexfvOES = (PFNGLDRAWTEXFVOES)PVRGetProcAddress(glDrawTexfvOES);
	}

	/* GL_EXT_multi_draw_arrays */
	if (strstr((char *)pszGLExtensions, "GL_OES_mapbuffer"))
	{
        glMapBufferOES = (PFNGLMAPBUFFEROES)PVRGetProcAddress(glMapBufferOES);
        glUnmapBufferOES = (PFNGLUNMAPBUFFEROES)PVRGetProcAddress(glUnmapBufferOES);
        glGetBufferPointervOES = (PFNGLGETBUFFERPOINTERVOES)PVRGetProcAddress(glGetBufferPointervOES);
	}

	    /* GL_OES_Framebuffer_object*/
	if (strstr((char *)pszGLExtensions, "GL_OES_framebuffer_object"))
	{
        glIsRenderbufferOES = (PFNGLISRENDERBUFFEROES)PVRGetProcAddress(glIsRenderbufferOES) ;
        glBindRenderbufferOES = (PFNGLBINDRENDERBUFFEROES)PVRGetProcAddress(glBindRenderbufferOES);
        glDeleteRenderbuffersOES = (PFNGLDELETERENDERBUFFERSOES)PVRGetProcAddress(glDeleteRenderbuffersOES);
        glGenRenderbuffersOES = (PFNGLGENRENDERBUFFERSOES)PVRGetProcAddress(glGenRenderbuffersOES);
        glRenderbufferStorageOES = (PFNGLRENDERBUFFERSTORAGEOES)PVRGetProcAddress(glRenderbufferStorageOES);
        glGetRenderbufferParameterivOES = (PFNGLGETRENDERBUFFERPARAMETERIVOES)PVRGetProcAddress(glGetRenderbufferParameterivOES);
        glIsFramebufferOES = (PFNGLISFRAMEBUFFEROES)PVRGetProcAddress(glIsFramebufferOES);
        glBindFramebufferOES = (PFNGLBINDFRAMEBUFFEROES)PVRGetProcAddress(glBindFramebufferOES);
        glDeleteFramebuffersOES = (PFNGLDELETEFRAMEBUFFERSOES)PVRGetProcAddress(glDeleteFramebuffersOES);
        glGenFramebuffersOES = (PFNGLGENFRAMEBUFFERSOES)PVRGetProcAddress(glGenFramebuffersOES);
        glCheckFramebufferStatusOES = (PFNGLCHECKFRAMEBUFFERSTATUSOES)PVRGetProcAddress(glCheckFramebufferStatusOES);
        glFramebufferTexture2DOES = (PFNGLFRAMEBUFFERTEXTURE2DOES)PVRGetProcAddress(glFramebufferTexture2DOES);
        glFramebufferRenderbufferOES = (PFNGLFRAMEBUFFERRENDERBUFFEROES)PVRGetProcAddress(glFramebufferRenderbufferOES);
        glGetFramebufferAttachmentParameterivOES = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVOES)PVRGetProcAddress(glGetFramebufferAttachmentParameterivOES);
        glGenerateMipmapOES = (PFNGLGENERATEMIPMAPOES)PVRGetProcAddress(glGenerateMipmapOES);
	}

	/* GL_OES_point_size_array */
	if (strstr((char *)pszGLExtensions, "GL_OES_point_size_array"))
	{
		glPointSizePointerOES = (PFNGLPOINTSIZEPOINTEROES)PVRGetProcAddress(glPointSizePointerOES);
	}

#if !defined (TARGET_OS_IPHONE)
	/* GL_IMG_user_clip_plane */
	if (strstr((char *)pszGLExtensions, "GL_IMG_user_clip_plane"))
	{
		/* glClipPlanexIMG and glClipPlanefIMG */
		glClipPlanexIMG = (PFNGLCLIPPLANEXIMG)PVRGetProcAddress(glClipPlanexIMG);
		glClipPlanefIMG = (PFNGLCLIPPLANEFIMG)PVRGetProcAddress(glClipPlanefIMG);
	}

	/* GL_IMG_vertex_program */
	if (strstr((char *)pszGLExtensions, "GL_IMG_vertex_program"))
	{
		glVertexAttribPointerARB = (PFNGLVERTEXATTRIBPOINTERARB)PVRGetProcAddress(glVertexAttribPointerARB);
		glEnableVertexAttribArrayARB = (PFNGLENABLEVERTEXATTRIBARRAYARB)PVRGetProcAddress(glEnableVertexAttribArrayARB);
		glDisableVertexAttribArrayARB = (PFNGLDISABLEVERTEXATTRIBARRAYARB)PVRGetProcAddress(glDisableVertexAttribArrayARB);
		glProgramStringARB = (PFNGLPROGRAMSTRINGARB)PVRGetProcAddress(glProgramStringARB);
		glBindProgramARB = (PFNGLBINDPROGRAMARB)PVRGetProcAddress(glBindProgramARB);
		glDeleteProgramsARB = (PFNGLDELETEPROGRAMSARB)PVRGetProcAddress(glDeleteProgramsARB);
		glIsProgramARB = (PFNGLISPROGRAMARB)PVRGetProcAddress(glIsProgramARB);
		glGenProgramsARB = (PFNGLGENPROGRAMSARB)PVRGetProcAddress(glGenProgramsARB);
		glVertexAttrib4fvARB = (PFNGLVERTEXATTRIB4FVARB)PVRGetProcAddress(glVertexAttrib4fvARB);
		glVertexAttrib4xvIMG = (PFNGLVERTEXATTRIB4XVIMG)PVRGetProcAddress(glVertexAttrib4xvIMG);
		glProgramLocalParameter4xIMG = (PFNGLPROGRAMLOCALPARAMETER4XIMG)PVRGetProcAddress(glProgramLocalParameter4xIMG);
		glProgramLocalParameter4xvIMG = (PFNGLPROGRAMLOCALPARAMETER4XVIMG)PVRGetProcAddress(glProgramLocalParameter4xvIMG);
		glProgramEnvParameter4xIMG = (PFNGLPROGRAMENVPARAMETER4XIMG)PVRGetProcAddress(glProgramEnvParameter4xIMG);
		glProgramEnvParameter4xvIMG = (PFNGLPROGRAMENVPARAMETER4XVIMG)PVRGetProcAddress(glProgramEnvParameter4xvIMG);
		glProgramEnvParameter4fARB = (PFNGLPROGRAMENVPARAMETER4FARB)PVRGetProcAddress(glProgramEnvParameter4fARB);
		glProgramEnvParameter4fvARB = (PFNGLPROGRAMENVPARAMETER4FVARB)PVRGetProcAddress(glProgramEnvParameter4fvARB);
		glProgramLocalParameter4fARB = (PFNGLPROGRAMLOCALPARAMETER4FARB)PVRGetProcAddress(glProgramLocalParameter4fARB);
		glProgramLocalParameter4fvARB = (PFNGLPROGRAMLOCALPARAMETER4FVARB)PVRGetProcAddress(glProgramLocalParameter4fvARB);
	}

	/* IMG_texture_stream */
	if (strstr((char *)pszGLExtensions, "GL_IMG_texture_stream"))
	{
		glGetTexStreamDeviceAttribivIMG = (PFNGLGETTEXSTREAMDEVICEATTRIBIVIMG)PVRGetProcAddress(glGetTexStreamDeviceAttribivIMG);
		glTexBindStreamIMG = (PFNGLTEXBINDSTREAMIMG)PVRGetProcAddress(glTexBindStreamIMG);
		glGetTexStreamDeviceNameIMG = (PFNGLGETTEXSTREAMDEVICENAMEIMG)PVRGetProcAddress(glGetTexStreamDeviceNameIMG);
	}

	/* GL_EXT_multi_draw_arrays */
	if (strstr((char *)pszGLExtensions, "GL_EXT_multi_draw_arrays"))
	{
		glMultiDrawElementsEXT = (PFNGLMULTIDRAWELEMENTS)PVRGetProcAddress(glMultiDrawElementsEXT);
		glMultiDrawArraysEXT = (PFNGLMULTIDRAWARRAYS)PVRGetProcAddress(glMultiDrawArraysEXT);
	}

	/* GL_OES_query_matrix */
	if (strstr((char *)pszGLExtensions, "GL_OES_query_matrix"))
	{
		glQueryMatrixxOES = (PFNGLQUERYMATRIXXOES)PVRGetProcAddress(glQueryMatrixxOES);
	}

	/* GL_OES_blend_equation */
	if (strstr((char *)pszGLExtensions, "GL_OES_blend_subtract"))
	{
		glBlendEquationOES = (PFNGLBLENDEQUATIONOES)PVRGetProcAddress(glBlendEquationOES);
	}

	/* GL_OES_query_matrix */
	if (strstr((char *)pszGLExtensions, "GL_OES_blend_equation_separate"))
	{
		glBlendEquationSeparateOES = (PFNGLBLENDEQUATIONSEPARATEOES)PVRGetProcAddress(glBlendEquationSeparateOES);
	}

	/* GL_OES_vertex_array_object */
	if (strstr((char *)pszGLExtensions, "GL_OES_vertex_array_object"))
	{
        glBindVertexArrayOES = (PFNGLBINDVERTEXARRAYOES) PVRGetProcAddress(glBindVertexArrayOES);
        glDeleteVertexArraysOES = (PFNGLDELETEVERTEXARRAYSOES) PVRGetProcAddress(glDeleteVertexArraysOES);
        glGenVertexArraysOES = (PFNGLGENVERTEXARRAYSOES) PVRGetProcAddress(glGenVertexArraysOES);
		glIsVertexArrayOES = (PFNGLISVERTEXARRAYOES) PVRGetProcAddress(glIsVertexArrayOES);
	}
#endif

#if defined(GL_EXT_discard_framebuffer)
	/* GL_EXT_discard_framebuffer */
	if (strstr((char *)pszGLExtensions, "GL_EXT_discard_framebuffer"))
	{
        glDiscardFramebufferEXT = (PFNGLDISCARDFRAMEBUFFEREXT) PVRGetProcAddress(glDiscardFramebufferEXT);
	}
#endif
}

/*!***********************************************************************
@Function			IsGLExtensionSupported
@Input				extension extension to query for
@Returns			True if the extension is supported
@Description		Queries for support of an extension
*************************************************************************/
bool CPVRTglesExt::IsGLExtensionSupported(const char * const extension)
{
	// The recommended technique for querying OpenGL extensions;
	// from http://opengl.org/resources/features/OGLextensions/
    const GLubyte *extensions = NULL;
    const GLubyte *start;
    GLubyte *where, *terminator;

    /* Extension names should not have spaces. */
    where = (GLubyte *) strchr(extension, ' ');
    if (where || *extension == '\0')
        return 0;

    extensions = glGetString(GL_EXTENSIONS);

    /* It takes a bit of care to be fool-proof about parsing the
    OpenGL extensions string. Don't be fooled by sub-strings, etc. */
    start = extensions;
    for (;;) {
        where = (GLubyte *) strstr((const char *) start, extension);
        if (!where)
            break;
        terminator = where + strlen(extension);
        if (where == start || *(where - 1) == ' ')
            if (*terminator == ' ' || *terminator == '\0')
                return true;
        start = terminator;
    }
    return false;
}

/*****************************************************************************
 End of file (PVRTglesExt.cpp)
*****************************************************************************/

