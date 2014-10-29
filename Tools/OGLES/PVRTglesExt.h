/*!****************************************************************************

 @file         OGLES/PVRTglesExt.h
 @ingroup      API_OGLES
 @copyright    Copyright (c) Imagination Technologies Limited.
 @brief        GLES extensions.

******************************************************************************/

#ifndef _PVRTGLESEXT_H_
#define _PVRTGLESEXT_H_

/*!
 @addtogroup   API_OGLES
 @{
*/

/****************************************************************************
** Build options
****************************************************************************/
#if defined(__APPLE__)
#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE==1
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#else
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#endif

#else
#if !defined(EGL_NOT_PRESENT)
#include <EGL/egl.h>
#endif
#include <GLES/gl.h>
#include <GLES/glext.h>
#endif  // __APPLE__

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef GL_APIENTRY
#define GL_APIENTRY
#endif
#define GL_PVRTGLESEXT_VERSION 1

#if defined(__APPLE__) && defined (TARGET_OS_IPHONE)
#define PVRGetProcAddress(x) ::x
#else
#if defined(EGL_NOT_PRESENT)

#if defined(__PALMPDK__)
#include "SDL.h"

#define PVRGetProcAddress(x) SDL_GLES_GetProcAddress(#x)
#else
#define PVRGetProcAddress(x) NULL
#endif

#else
#define PVRGetProcAddress(x) eglGetProcAddress(#x)
#endif
#endif

/**************************************************************************
****************************** GL EXTENSIONS ******************************
**************************************************************************/

/*!************************************************************************
 @class CPVRTglesExt
 @brief A class for initialising and managing OGLES extensions
**************************************************************************/
class CPVRTglesExt
{

public:
	/*
	GL_OES_matrix_palette
	*/
	typedef void (GL_APIENTRY * PFNGLCURRENTPALETTEMATRIXOES) (GLuint matrixpaletteindex);
	typedef void (GL_APIENTRY * PFNGLLOADPALETTEFROMMODELVIEWMATRIXOES) (void);
	typedef void (GL_APIENTRY * PFNGLMATRIXINDEXPOINTEROES) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
	typedef void (GL_APIENTRY * PFNGLWEIGHTPOINTEROES) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);

	/* Function pointers */
	PFNGLCURRENTPALETTEMATRIXOES			glCurrentPaletteMatrixOES;
	PFNGLLOADPALETTEFROMMODELVIEWMATRIXOES	glLoadPaletteFromModelViewMatrixOES;
	PFNGLMATRIXINDEXPOINTEROES				glMatrixIndexPointerOES;
	PFNGLWEIGHTPOINTEROES					glWeightPointerOES;

	/*
	GL_IMG_texture_env_enhanced_fixed_function
	*/
	/* Tokens */
#if !defined(GL_IMG_texture_env_enhanced_fixed_function)
#define GL_MODULATE_COLOR_IMG						0x8C04
#define GL_RECIP_ADD_SIGNED_ALPHA_IMG				0x8C05
#define GL_TEXTURE_ALPHA_MODULATE_IMG				0x8C06
#define GL_FACTOR_ALPHA_MODULATE_IMG				0x8C07
#define GL_FRAGMENT_ALPHA_MODULATE_IMG				0x8C08
#define GL_ADD_BLEND_IMG							0x8C09
#ifndef GL_DOT3_RGBA
#define GL_DOT3_RGBA								0x86AF
#endif
#endif

	/*
	GL_IMG_texture_compression_pvrtc
	*/
	/* Tokens */
#if !defined(GL_IMG_texture_compression_pvrtc)
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG			0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG			0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG			0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG			0x8C03
#endif
	/*
	GL_IMG_texture_format_BGRA8888
	*/
	/* Tokens */
#if !defined(GL_IMG_texture_format_BGRA8888)
#define GL_BGRA										0x80E1
#endif
	/*
	GL_ARB_texture_env_combine
	*/
	/* Tokens */
#define GL_COMBINE									0x8570
#define GL_COMBINE_RGB								0x8571
#define GL_COMBINE_ALPHA							0x8572
#define GL_RGB_SCALE								0x8573
#define GL_ADD_SIGNED								0x8574
#define GL_INTERPOLATE								0x8575
#define GL_CONSTANT									0x8576
#define GL_PRIMARY_COLOR							0x8577
#define GL_PREVIOUS									0x8578
#define GL_SOURCE0_RGB								0x8580
#define GL_SOURCE1_RGB								0x8581
#define GL_SOURCE2_RGB								0x8582
#define GL_SOURCE0_ALPHA							0x8588
#define GL_SOURCE1_ALPHA							0x8589
#define GL_SOURCE2_ALPHA							0x858A
#define GL_OPERAND0_RGB								0x8590
#define GL_OPERAND1_RGB								0x8591
#define GL_OPERAND2_RGB								0x8592
#define GL_OPERAND0_ALPHA							0x8598
#define GL_OPERAND1_ALPHA							0x8599
#define GL_OPERAND2_ALPHA							0x859A
#define GL_SUBTRACT									0x84E7


	/*
	GL_ARB_texture_env_dot3
	*/
	/* Tokens */
#define GL_DOT3_RGB									0x86AE
#ifndef GL_DOT3_RGBA
#define GL_DOT3_RGBA								0x86AF
#endif


	/*
	GL_IMG_user_clip_plane
	*/
	/* Tokens */
#if !defined(GL_IMG_user_clip_plane)
#define GL_MAX_CLIP_PLANES							0x0D32
#define GL_CLIP_PLANE0								0x3000
#define GL_CLIP_PLANE1								0x3001
#define GL_CLIP_PLANE2								0x3002
#define GL_CLIP_PLANE3								0x3003
#define GL_CLIP_PLANE4								0x3004
#define GL_CLIP_PLANE5								0x3005
#endif

	/* Typedefs */
	typedef void (APIENTRY * PFNGLCLIPPLANEFIMG) (GLenum p, GLfloat eqn[4]);
	typedef void (APIENTRY * PFNGLCLIPPLANEXIMG) (GLenum p, GLfixed eqn[4]);


	/* Function pointers */
	PFNGLCLIPPLANEFIMG	glClipPlanefIMG;
	PFNGLCLIPPLANEXIMG	glClipPlanexIMG;


	/*
	GL_IMG_vertex_program
	*/
	/* Tokens */
#if !defined(GL_IMG_vertex_program)
#define GL_VERTEX_PROGRAM_ARB						0x8620
#define GL_PROGRAM_STRING_ARB						0x8628
#define GL_PROGRAM_ERROR_STRING_ARB					0x8874
#define GL_MAX_PROGRAM_ATTRIBS_ARB					0x88AD
#define GL_MAX_PROGRAM_PARAMETERS_ARB				0x88A9
#define GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB			0x88B4
#define GL_MAX_PROGRAM_ENV_PARAMETERS_ARB			0x88B5
#define GL_MAX_VERTEX_ATTRIBS_ARB					0x8869
#define GL_MAX_PROGRAM_MATRICES_ARB					0x862F
#define GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB		0x862E
#define GL_MATRIX0_ARB								0x88C0
#define GL_MATRIX1_ARB								0x88C1
#define GL_MATRIX2_ARB								0x88C2
#define GL_MATRIX3_ARB								0x88C3
#define GL_MATRIX4_ARB								0x88C4
#define GL_MATRIX5_ARB								0x88C5
#define GL_MATRIX6_ARB								0x88C6
#define GL_MATRIX7_ARB								0x88C7
#define GL_PROGRAM_FORMAT_BINARY_IMG				0x8C0B
#define GL_UNSIGNED_BYTE_NORM_IMG					0x8C0C
#endif

	/* IMG_texture_stream */
#if !defined(GL_IMG_texture_stream)
#define GL_TEXTURE_STREAM_IMG						0x8C0D
#define GL_TEXTURE_NUM_STREAM_DEVICES_IMG			0x8C0E
#define GL_TEXTURE_STREAM_DEVICE_WIDTH_IMG			0x8C0F
#define GL_TEXTURE_STREAM_DEVICE_HEIGHT_IMG			0x8C10
#define GL_TEXTURE_STREAM_DEVICE_FORMAT_IMG			0x8C11
#define GL_TEXTURE_STREAM_DEVICE_NUM_BUFFERS_IMG	0x8C12
#endif

	/* Typedefs */
	typedef void (APIENTRY * PFNGLVERTEXATTRIBPOINTERARB) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
	typedef void (APIENTRY * PFNGLENABLEVERTEXATTRIBARRAYARB) (GLuint index);
	typedef void (APIENTRY * PFNGLDISABLEVERTEXATTRIBARRAYARB) (GLuint index);
	typedef void (APIENTRY * PFNGLPROGRAMSTRINGARB) (GLenum target, GLenum format, GLsizei len, const GLvoid *string);
	typedef void (APIENTRY * PFNGLBINDPROGRAMARB) (GLenum target, GLuint program);
	typedef void (APIENTRY * PFNGLDELETEPROGRAMSARB) (GLsizei n, const GLuint *programs);
	typedef void (APIENTRY * PFNGLGENPROGRAMSARB) (GLsizei n, GLuint *programs);
	typedef void (APIENTRY * PFNGLPROGRAMENVPARAMETER4FARB) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	typedef void (APIENTRY * PFNGLPROGRAMENVPARAMETER4FVARB) (GLenum target, GLuint index, const GLfloat *params);
	typedef void (APIENTRY * PFNGLPROGRAMLOCALPARAMETER4FARB) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	typedef void (APIENTRY * PFNGLPROGRAMLOCALPARAMETER4FVARB) (GLenum target, GLuint index, const GLfloat *params);
	typedef GLboolean (APIENTRY * PFNGLISPROGRAMARB) (GLuint program);
	typedef void (APIENTRY * PFNGLVERTEXATTRIB4FVARB) (GLuint index, const float *v);
	typedef void (APIENTRY * PFNGLVERTEXATTRIB4XVIMG) (GLuint index, const GLfixed *v);
	typedef void (APIENTRY * PFNGLPROGRAMLOCALPARAMETER4XIMG) (GLenum target, GLuint index, GLfixed x, GLfixed y, GLfixed z, GLfixed w);
	typedef void (APIENTRY * PFNGLPROGRAMLOCALPARAMETER4XVIMG) (GLenum target, GLuint index, const GLfixed *params);
	typedef void (APIENTRY * PFNGLPROGRAMENVPARAMETER4XIMG) (GLenum target, GLuint index, GLfixed x, GLfixed y, GLfixed z, GLfixed w);
	typedef void (APIENTRY * PFNGLPROGRAMENVPARAMETER4XVIMG) (GLenum target, GLuint index, const GLfixed *params);
	typedef void (APIENTRY * PFNGLDRAWTEXIOES) (GLint x, GLint y, GLint z, GLint width, GLint height);
	typedef void (APIENTRY * PFNGLDRAWTEXIVOES) (const GLint* coords);
	typedef void (APIENTRY * PFNGLDRAWTEXSOES) (GLshort x, GLshort y, GLshort z, GLshort width, GLshort height);
	typedef void (APIENTRY * PFNGLDRAWTEXSVOES) (const GLshort* coords);
	typedef void (APIENTRY * PFNGLDRAWTEXXOES) (GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height);
	typedef void (APIENTRY * PFNGLDRAWTEXXVOES) (const GLfixed* coords);
	typedef void (APIENTRY * PFNGLDRAWTEXFOES) (GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height);
	typedef void (APIENTRY * PFNGLDRAWTEXFVOES) (const GLfloat* coords);
	/* IMG_texture_stream */
	typedef void (APIENTRY * PFNGLGETTEXSTREAMDEVICEATTRIBIVIMG)(GLint device, GLenum pname, GLint *params);
	typedef void (APIENTRY * PFNGLTEXBINDSTREAMIMG)(GLint device, GLint deviceoffset);
	typedef const GLubyte * (APIENTRY * PFNGLGETTEXSTREAMDEVICENAMEIMG)(GLint device);
	/* GL_EXT_multi_draw_arrays */
	typedef void (GL_APIENTRY * PFNGLMULTIDRAWELEMENTS) (GLenum mode, GLsizei *count, GLenum type, const GLvoid **indices, GLsizei primcount); // glvoid
	typedef void (GL_APIENTRY * PFNGLMULTIDRAWARRAYS) (GLenum mode, GLint *first, GLsizei *count, GLsizei primcount); // glvoid

	/* GL_OES_mapbuffer */
	typedef void* (GL_APIENTRY *PFNGLMAPBUFFEROES)(GLenum target, GLenum access);
	typedef GLboolean (GL_APIENTRY *PFNGLUNMAPBUFFEROES)(GLenum target);
	typedef void (GL_APIENTRY *PFNGLGETBUFFERPOINTERVOES)(GLenum target, GLenum pname, void** params);

	/* GL_OES_Framebuffer_object*/

	typedef GLboolean (GL_APIENTRY *PFNGLISRENDERBUFFEROES)(GLuint renderbuffer);
	typedef void (GL_APIENTRY *PFNGLBINDRENDERBUFFEROES)(GLenum target, GLuint renderbuffer);
	typedef void (GL_APIENTRY *PFNGLDELETERENDERBUFFERSOES)(GLsizei n, const GLuint *renderbuffers);
	typedef void (GL_APIENTRY *PFNGLGENRENDERBUFFERSOES)(GLsizei n, GLuint *renderbuffers);
	typedef void (GL_APIENTRY *PFNGLRENDERBUFFERSTORAGEOES)(GLenum target, GLenum internalformat,GLsizei width, GLsizei height);
	typedef void (GL_APIENTRY *PFNGLGETRENDERBUFFERPARAMETERIVOES)(GLenum target, GLenum pname, GLint* params);
	typedef GLboolean (GL_APIENTRY *PFNGLISFRAMEBUFFEROES)(GLuint framebuffer);
	typedef void (GL_APIENTRY *PFNGLBINDFRAMEBUFFEROES)(GLenum target, GLuint framebuffer);
	typedef void (GL_APIENTRY *PFNGLDELETEFRAMEBUFFERSOES)(GLsizei n, const GLuint *framebuffers);
	typedef void (GL_APIENTRY *PFNGLGENFRAMEBUFFERSOES)(GLsizei n, GLuint *framebuffers);
	typedef GLenum (GL_APIENTRY *PFNGLCHECKFRAMEBUFFERSTATUSOES)(GLenum target);
	typedef void (GL_APIENTRY *PFNGLFRAMEBUFFERTEXTURE2DOES)(GLenum target, GLenum attachment,GLenum textarget, GLuint texture,GLint level);
	typedef void (GL_APIENTRY *PFNGLFRAMEBUFFERRENDERBUFFEROES)(GLenum target, GLenum attachment,GLenum renderbuffertarget, GLuint renderbuffer);
	typedef void (GL_APIENTRY *PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVOES)(GLenum target, GLenum attachment,GLenum pname, GLint *params);
	typedef void (GL_APIENTRY *PFNGLGENERATEMIPMAPOES)(GLenum target);

    /* GL_OES_point_size_array */
    typedef void (GL_APIENTRY *PFNGLPOINTSIZEPOINTEROES)(GLenum type, GLsizei stride, const void *ptr );

    typedef GLbitfield (GL_APIENTRY *PFNGLQUERYMATRIXXOES)( GLfixed mantissa[16],GLint exponent[16] );

    /* GL_OES_blend_subtract */

    typedef GLvoid (GL_APIENTRY *PFNGLBLENDEQUATIONOES)( GLenum mode );

    /* GL_blend_equation_separate */
    typedef GLvoid (GL_APIENTRY *PFNGLBLENDEQUATIONSEPARATEOES)(GLenum modeRGB, GLenum modeAlpha);

	/* GL_EXT_discard_framebuffer */
	typedef void (GL_APIENTRY * PFNGLDISCARDFRAMEBUFFEREXT)(GLenum target, GLsizei numAttachments, const GLenum *attachments);

	/* GL_OES_vertex_array_object */
	typedef void (GL_APIENTRY * PFNGLBINDVERTEXARRAYOES) (GLuint vertexarray);
	typedef void (GL_APIENTRY * PFNGLDELETEVERTEXARRAYSOES) (GLsizei n, const GLuint *vertexarrays);
	typedef void (GL_APIENTRY * PFNGLGENVERTEXARRAYSOES) (GLsizei n, GLuint *vertexarrays);
	typedef GLboolean (GL_APIENTRY * PFNGLISVERTEXARRAYOES) (GLuint vertexarray);

	/* Function pointers */
	PFNGLVERTEXATTRIBPOINTERARB			glVertexAttribPointerARB;
	PFNGLENABLEVERTEXATTRIBARRAYARB		glEnableVertexAttribArrayARB;
	PFNGLDISABLEVERTEXATTRIBARRAYARB	glDisableVertexAttribArrayARB;
	PFNGLPROGRAMSTRINGARB				glProgramStringARB;
	PFNGLBINDPROGRAMARB					glBindProgramARB;
	PFNGLDELETEPROGRAMSARB				glDeleteProgramsARB;
	PFNGLGENPROGRAMSARB					glGenProgramsARB;
	PFNGLISPROGRAMARB					glIsProgramARB;
	PFNGLPROGRAMENVPARAMETER4FARB		glProgramEnvParameter4fARB;
	PFNGLPROGRAMENVPARAMETER4FVARB		glProgramEnvParameter4fvARB;
	PFNGLPROGRAMLOCALPARAMETER4FARB		glProgramLocalParameter4fARB;
	PFNGLPROGRAMLOCALPARAMETER4FVARB	glProgramLocalParameter4fvARB;
	PFNGLVERTEXATTRIB4FVARB				glVertexAttrib4fvARB;
	PFNGLVERTEXATTRIB4XVIMG				glVertexAttrib4xvIMG;
	PFNGLPROGRAMLOCALPARAMETER4XIMG		glProgramLocalParameter4xIMG;
	PFNGLPROGRAMLOCALPARAMETER4XVIMG	glProgramLocalParameter4xvIMG;
	PFNGLPROGRAMENVPARAMETER4XIMG		glProgramEnvParameter4xIMG;
	PFNGLPROGRAMENVPARAMETER4XVIMG		glProgramEnvParameter4xvIMG;
	PFNGLDRAWTEXIOES					glDrawTexiOES;
	PFNGLDRAWTEXIVOES					glDrawTexivOES;
	PFNGLDRAWTEXSOES					glDrawTexsOES;
	PFNGLDRAWTEXSVOES					glDrawTexsvOES;
	PFNGLDRAWTEXXOES					glDrawTexxOES;
	PFNGLDRAWTEXXVOES					glDrawTexxvOES;
	PFNGLDRAWTEXFOES					glDrawTexfOES;
	PFNGLDRAWTEXFVOES					glDrawTexfvOES;
	/* IMG_texture_stream */
	PFNGLGETTEXSTREAMDEVICEATTRIBIVIMG	glGetTexStreamDeviceAttribivIMG;
	PFNGLTEXBINDSTREAMIMG				glTexBindStreamIMG;
	PFNGLGETTEXSTREAMDEVICENAMEIMG		glGetTexStreamDeviceNameIMG;
	/* GL_EXT_multi_draw_arrays */
	PFNGLMULTIDRAWELEMENTS				glMultiDrawElementsEXT;
	PFNGLMULTIDRAWARRAYS				glMultiDrawArraysEXT;

	/* GL_EXT_multi_draw_arrays */
	PFNGLMAPBUFFEROES                   glMapBufferOES;
	PFNGLUNMAPBUFFEROES                 glUnmapBufferOES;
	PFNGLGETBUFFERPOINTERVOES           glGetBufferPointervOES;


	/* GL_OES_Framebuffer_object*/

	PFNGLISRENDERBUFFEROES glIsRenderbufferOES;
	PFNGLBINDRENDERBUFFEROES glBindRenderbufferOES;
	PFNGLDELETERENDERBUFFERSOES glDeleteRenderbuffersOES;
	PFNGLGENRENDERBUFFERSOES glGenRenderbuffersOES;
	PFNGLRENDERBUFFERSTORAGEOES glRenderbufferStorageOES;
	PFNGLGETRENDERBUFFERPARAMETERIVOES glGetRenderbufferParameterivOES;
	PFNGLISFRAMEBUFFEROES glIsFramebufferOES;
	PFNGLBINDFRAMEBUFFEROES glBindFramebufferOES;
	PFNGLDELETEFRAMEBUFFERSOES glDeleteFramebuffersOES;
	PFNGLGENFRAMEBUFFERSOES glGenFramebuffersOES;
	PFNGLCHECKFRAMEBUFFERSTATUSOES glCheckFramebufferStatusOES;
	PFNGLFRAMEBUFFERTEXTURE2DOES glFramebufferTexture2DOES;
	PFNGLFRAMEBUFFERRENDERBUFFEROES glFramebufferRenderbufferOES;
	PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVOES glGetFramebufferAttachmentParameterivOES;
	PFNGLGENERATEMIPMAPOES glGenerateMipmapOES;

    /* GL_OES_point_size_array */
    PFNGLPOINTSIZEPOINTEROES glPointSizePointerOES;

    PFNGLQUERYMATRIXXOES    glQueryMatrixxOES;

    PFNGLBLENDEQUATIONOES             glBlendEquationOES;
    PFNGLBLENDEQUATIONSEPARATEOES     glBlendEquationSeparateOES;


	/* GL_EXT_discard_framebuffer */
	PFNGLDISCARDFRAMEBUFFEREXT			glDiscardFramebufferEXT;

	/* GL_OES_vertex_array_object */
	#define GL_VERTEX_ARRAY_BINDING_OES 0x85B5
	PFNGLBINDVERTEXARRAYOES glBindVertexArrayOES;
	PFNGLDELETEVERTEXARRAYSOES glDeleteVertexArraysOES;
	PFNGLGENVERTEXARRAYSOES glGenVertexArraysOES;
	PFNGLISVERTEXARRAYOES glIsVertexArrayOES;

	/**************************************************************************
	****************************** EGL EXTENSIONS *****************************
	**************************************************************************/

	/*
	EGL_IMG_power_management
	*/
	/* Tokens */
#define EGL_CONTEXT_LOST_IMG						0x300E


public:
	/*!***********************************************************************
	@brief		Initialises IMG extensions
	*************************************************************************/
	void LoadExtensions();

	/*!***********************************************************************
	@brief      		Queries for support of an extension
	@param[in]			extension extension to query for
	@return 			True if the extension is supported
	*************************************************************************/
	static bool IsGLExtensionSupported(const char * const extension);
	// The recommended technique for querying OpenGL extensions;
	// from http://opengl.org/resources/features/OGLextensions/
};


/****************************************************************************
** Functions
****************************************************************************/

/*! @} */

#endif /* _PVRTGLESEXT_H_ */

/*****************************************************************************
End of file (PVRTglesExt.h)
*****************************************************************************/

