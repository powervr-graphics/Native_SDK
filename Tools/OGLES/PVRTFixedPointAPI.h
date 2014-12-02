/*!****************************************************************************

 @file         OGLES/PVRTFixedPointAPI.h
 @ingroup      API_OGLES
 @copyright    Copyright (c) Imagination Technologies Limited.
 @brief        Set of macros and functions to make OpenGL ES Lite profile easier
               to program. Behaviour is affected by the following macro:
               PVRT_FIXED_POINT_ENABLE

******************************************************************************/
#ifndef _PVRTFIXEDPOINTAPI_H_
#define _PVRTFIXEDPOINTAPI_H_


#if defined(OGLESLITE) || defined(PVRTFIXEDPOINTENABLE)
#error OGLESLITE and PVRTFIXEDPOINTENABLE have been replaced by PVRT_FIXED_POINT_ENABLE. Please update your code.
#endif

#include "PVRTContext.h"


/*!
 @addtogroup   API_OGLES
 @{
*/

/*!****************************************************************************
    Set of macros and functions to make OpenGL ES Lite profile easier to program.
    OpenGL ES function macros to abstract the profile used (Common or Common-Lite)
    Defines a set of shims of the form myglXXX which can allow on-the-fly conversion
    from a floating point format to fixed-point OpenGL ES entrypoints.
    If the flag PVRT_FIXED_POINT_ENABLE is defined, inputs to the function are converted to fixed
    point and the the OpenGL ES Lite version of the function is then called
    If the flag PVRT_FIXED_POINT_ENABLE is not defined, the macros directly default to their
    Common profile counterpart.
*******************************************************************************/

#ifndef PVRT_FIXED_POINT_ENABLE

	#define VERTTYPE GLfloat
	#define VERTTYPEENUM GL_FLOAT

	#define myglFog				glFogf
	#define myglFogv			glFogfv

	#define myglLightv			glLightfv
	#define myglLight			glLightf

	#define myglLightModelv		glLightModelfv
	#define myglLightModel		glLightModelf

	#define myglAlphaFunc		glAlphaFunc

	#define myglMaterialv		glMaterialfv
	#define myglMaterial		glMaterialf

	#define myglTexParameter	glTexParameterf
	#define myglTexEnv			glTexEnvf

	#define myglOrtho			glOrthof
	#define myglFrustum			glFrustumf

	#define myglTranslate		glTranslatef
	#define myglScale			glScalef
	#define myglRotate			glRotatef

	#define myglColor4			glColor4f

	#define myglClearColor		glClearColor

	#define myglClearDepth		glClearDepthf

	#define myglMultMatrix		glMultMatrixf

	#define myglNormal3			glNormal3f

	#define myglLoadMatrix		glLoadMatrixf

	#define myglPolygonOffset	glPolygonOffset

	#define myglPointSize		glPointSize

	/* GL_IMG_VERTEX_PROGRAM extensions */
	#define myglProgramLocalParameter4v		glProgramLocalParameter4fvARB
	#define myglProgramLocalParameter4		glProgramLocalParameter4fARB
	#define myglProgramEnvParameter4v		glProgramEnvParameter4fvARB
	#define myglProgramEnvParameter4		glProgramEnvParameter4fARB
	#define myglVertexAttrib4v				glVertexAttrib4fvARB

	#define myglClipPlane					glClipPlanef

	#define myglPointParameter				glPointParameterf

	#define myglPointParameterv				glPointParameterfv


#else

	#define VERTTYPE GLfixed
	#define VERTTYPEENUM GL_FIXED

	#define myglFog				glFogx
	#define myglFogv			glFogxv

	#define myglLight			glLightx
	#define myglLightv			glLightxv

	#define myglLightModel		glLightModelx
	#define myglLightModelv		glLightModelxv

	#define myglAlphaFunc		glAlphaFuncx

	#define myglMaterial		glMaterialx
	#define myglMaterialv		glMaterialxv

	#define myglTexParameter	glTexParameterx
	#define myglTexEnv			glTexEnvx

	#define myglOrtho			glOrthox
	#define myglFrustum			glFrustumx

	#define myglTranslate		glTranslatex
	#define myglScale			glScalex
	#define myglRotate			glRotatex

	#define myglColor4			glColor4x

	#define myglClearColor		glClearColorx

	#define myglClearDepth		glClearDepthx

	#define myglMultMatrix		glMultMatrixx

	#define myglNormal3			glNormal3x

	#define myglLoadMatrix		glLoadMatrixx

	#define myglPolygonOffset	glPolygonOffsetx

	#define myglPointSize		glPointSizex

	/* GL_IMG_VERTEX_PROGRAM extensions */
	#define myglProgramLocalParameter4v	glProgramLocalParameter4xvIMG
	#define myglProgramLocalParameter4	glProgramLocalParameter4xIMG
	#define myglProgramEnvParameter4v	glProgramEnvParameter4xvIMG
	#define myglProgramEnvParameter4	glProgramEnvParameter4xIMG
	#define myglVertexAttrib4v			glVertexAttrib4xvIMG

	#define myglClipPlane		glClipPlanex

	#define myglPointParameter	glPointParameterx
	#define myglPointParameterv	glPointParameterxv

#endif

/*! @} */

#endif /* _PVRTFIXEDPOINTAPI_H_ */

/*****************************************************************************
 End of file (PVRTFixedPoint.h)
*****************************************************************************/
