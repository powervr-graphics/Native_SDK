/******************************************************************************

 @File         OGLES/PVRTPrint3DAPI.cpp

 @Title        OGLES/PVRTPrint3DAPI

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     ANSI compatible

 @Description  Displays a text string using 3D polygons. Can be done in two ways:
               using a window defined by the user or writing straight on the
               screen.

******************************************************************************/

/****************************************************************************
** Includes
****************************************************************************/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "PVRTContext.h"
#include "PVRTFixedPoint.h"
#include "PVRTMatrix.h"
#include "PVRTPrint3D.h"
#include "PVRTglesExt.h"
#include "PVRTTexture.h"
#include "PVRTTextureAPI.h"
#include "PVRTMap.h"

/****************************************************************************
** Defines
****************************************************************************/
#define UNDEFINED_HANDLE 0xFAFAFAFA

/****************************************************************************
** Structures
****************************************************************************/

struct SPVRTPrint3DAPI
{
	GLuint			m_uFontTexture;

	struct SInstanceData
	{
		GLuint uTextureIMGLogo;
		GLuint uTexturePowerVRLogo;

		SInstanceData() : uTextureIMGLogo(UNDEFINED_HANDLE), uTexturePowerVRLogo(UNDEFINED_HANDLE)
		{
		}
	};

	SInstanceData*			m_pInstanceData;
	static SInstanceData	s_InstanceData;

	static bool				s_IsVGPSupported;
	static int				s_iRefCount;

	SPVRTPrint3DAPI() : m_pInstanceData(NULL) {}
	~SPVRTPrint3DAPI()
	{
		if(m_pInstanceData)
		{
			delete m_pInstanceData;
			m_pInstanceData = NULL;
		}
	}
};
bool SPVRTPrint3DAPI::s_IsVGPSupported			= false;
int SPVRTPrint3DAPI::s_iRefCount				= 0;
SPVRTPrint3DAPI::SInstanceData SPVRTPrint3DAPI::s_InstanceData;

const GLenum c_eMagTable[] =
{
	GL_NEAREST,
	GL_LINEAR,
};

const GLenum c_eMinTable[] =
{
	GL_NEAREST_MIPMAP_NEAREST,
	GL_LINEAR_MIPMAP_NEAREST,
	GL_NEAREST_MIPMAP_LINEAR,
	GL_LINEAR_MIPMAP_LINEAR,
	GL_NEAREST,
	GL_LINEAR,
};

/****************************************************************************
** Class: CPVRTPrint3D
****************************************************************************/
/*!***************************************************************************
 @Function			ReleaseTextures
 @Description		Deallocate the memory allocated in SetTextures(...)
*****************************************************************************/
void CPVRTPrint3D::ReleaseTextures()
{
#if !defined (DISABLE_PRINT3D)

	/* Only release textures if they've been allocated */
	if (!m_bTexturesSet) return;

	/* Release IndexBuffer */
	FREE(m_pwFacesFont);
	FREE(m_pPrint3dVtx);

	/* Delete textures */
	glDeleteTextures(1, &m_pAPI->m_uFontTexture);
	
	// Has local copy
	if(m_pAPI->m_pInstanceData)
	{
		glDeleteTextures(1, &m_pAPI->m_pInstanceData->uTextureIMGLogo);
		glDeleteTextures(1, &m_pAPI->m_pInstanceData->uTexturePowerVRLogo);
	}
	else
	{
		if(SPVRTPrint3DAPI::s_iRefCount != 0)
		{
			// Just decrease the reference count
			SPVRTPrint3DAPI::s_iRefCount--;
		}
		else
		{
			// Release the textures
			if(m_pAPI->s_InstanceData.uTextureIMGLogo != UNDEFINED_HANDLE) 
				glDeleteTextures(1, &m_pAPI->s_InstanceData.uTextureIMGLogo);

			m_pAPI->s_InstanceData.uTextureIMGLogo = UNDEFINED_HANDLE;

			// Release the textures
			if(m_pAPI->s_InstanceData.uTexturePowerVRLogo != UNDEFINED_HANDLE) 
				glDeleteTextures(1, &m_pAPI->s_InstanceData.uTexturePowerVRLogo);

			m_pAPI->s_InstanceData.uTexturePowerVRLogo = UNDEFINED_HANDLE;
		}
	}
	
	

	m_bTexturesSet = false;

	FREE(m_pVtxCache);

	APIRelease();

#endif
}

/*!***************************************************************************
 @Function			Flush
 @Description		Flushes all the print text commands
*****************************************************************************/
int CPVRTPrint3D::Flush()
{
#if !defined (DISABLE_PRINT3D)

	int		nTris, nVtx, nVtxBase, nTrisTot;

	_ASSERT((m_nVtxCache % 4) == 0);
	_ASSERT(m_nVtxCache <= m_nVtxCacheMax);

	/* Save render states */
	APIRenderStates(0);

	/* Set font texture */
	glBindTexture(GL_TEXTURE_2D, m_pAPI->m_uFontTexture);

	unsigned int uiIndex = m_eFilterMethod[eFilterProc_Min] + (m_eFilterMethod[eFilterProc_Mip]*2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, c_eMagTable[m_eFilterMethod[eFilterProc_Mag]]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, c_eMinTable[uiIndex]);

	/* Set blending mode */
	glEnable(GL_BLEND);

	nTrisTot = m_nVtxCache >> 1;

	/*
		Render the text then. Might need several submissions.
	*/
	nVtxBase = 0;
	while(m_nVtxCache)
	{
		nVtx	= PVRT_MIN(m_nVtxCache, 0xFFFC);
		nTris	= nVtx >> 1;

		_ASSERT(nTris <= (PVRTPRINT3D_MAX_RENDERABLE_LETTERS*2));
		_ASSERT((nVtx % 4) == 0);

		/* Draw triangles */
		glVertexPointer(3,		VERTTYPEENUM,		sizeof(SPVRTPrint3DAPIVertex), &m_pVtxCache[nVtxBase].sx);
		glColorPointer(4,		GL_UNSIGNED_BYTE,	sizeof(SPVRTPrint3DAPIVertex), &m_pVtxCache[nVtxBase].color);
		glTexCoordPointer(2,	VERTTYPEENUM,		sizeof(SPVRTPrint3DAPIVertex), &m_pVtxCache[nVtxBase].tu);
		glDrawElements(GL_TRIANGLES, nTris * 3, GL_UNSIGNED_SHORT, m_pwFacesFont);
		if (glGetError())
		{
			_RPT0(_CRT_WARN,"glDrawElements(GL_TRIANGLES, (VertexCount/2)*3, GL_UNSIGNED_SHORT, m_pFacesFont); failed\n");
		}

		nVtxBase		+= nVtx;
		m_nVtxCache	-= nVtx;
	}

	/* Draw a logo if requested */
#if !defined(FORCE_NO_LOGO)
	// User selected logos
	if(m_uLogoToDisplay & ePVRTPrint3DLogoPowerVR && m_uLogoToDisplay & ePVRTPrint3DLogoIMG)
	{
		APIDrawLogo(ePVRTPrint3DLogoIMG, eBottom | eRight);	// IMG to the right
		APIDrawLogo(ePVRTPrint3DLogoPowerVR, eBottom | eLeft);	// PVR to the left
	}
	else if(m_uLogoToDisplay & ePVRTPrint3DLogoPowerVR)
	{
		APIDrawLogo(ePVRTPrint3DLogoPowerVR, eBottom | eRight);	// logo to the right
	}
	else if(m_uLogoToDisplay & ePVRTPrint3DLogoIMG)
	{
		APIDrawLogo(ePVRTPrint3DLogoIMG, eBottom | eRight);	// logo to the right
	}
#endif

	/* Restore render states */
	APIRenderStates(1);

	return nTrisTot;

#else
	return 0;
#endif
}

/*************************************************************
*					 PRIVATE FUNCTIONS						 *
**************************************************************/

/*!***************************************************************************
 @Function			APIInit
 @Description		Initialization and texture upload. Should be called only once
					for a given context.
*****************************************************************************/
bool CPVRTPrint3D::APIInit(const SPVRTContext	* const /*pContext*/, bool bMakeCopy)
{
	m_pAPI = new SPVRTPrint3DAPI;
	if(!m_pAPI)
		return false;

	if(bMakeCopy)
		m_pAPI->m_pInstanceData = new SPVRTPrint3DAPI::SInstanceData();

	SPVRTPrint3DAPI::s_IsVGPSupported = CPVRTglesExt::IsGLExtensionSupported("GL_IMG_vertex_program");

	return true;
}

/*!***************************************************************************
 @Function			APIRelease
 @Description		Deinitialization.
*****************************************************************************/
void CPVRTPrint3D::APIRelease()
{
	delete m_pAPI;
	m_pAPI = 0;
}

/*!***************************************************************************
 @Function			APIUpLoadIcons
 @Description		Initialization and texture upload. Should be called only once
					for a given context.
*****************************************************************************/
bool CPVRTPrint3D::APIUpLoadIcons(const PVRTuint8 * const pIMG, const PVRTuint8 * const pPowerVR)
{
	SPVRTPrint3DAPI::SInstanceData& Data = (m_pAPI->m_pInstanceData ? *m_pAPI->m_pInstanceData : SPVRTPrint3DAPI::s_InstanceData);

	// Load Icon textures
	if(Data.uTextureIMGLogo == UNDEFINED_HANDLE)		// Static, so might already be initialized.
		if(PVRTTextureLoadFromPointer((unsigned char*)pIMG, &Data.uTextureIMGLogo) != PVR_SUCCESS)
			return false;

	if(Data.uTexturePowerVRLogo == UNDEFINED_HANDLE)		// Static, so might already be initialized.
		if(PVRTTextureLoadFromPointer((unsigned char*)pPowerVR, &Data.uTexturePowerVRLogo) != PVR_SUCCESS)
			return false;

	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

/*!***************************************************************************
@Function		APIUpLoadTexture
@Input			pSource
@Output			header
@Return			bool	true if successful.
@Description	Loads and uploads the font texture from a PVR file.
*****************************************************************************/
bool CPVRTPrint3D::APIUpLoadTexture(const PVRTuint8* pSource, const PVRTextureHeaderV3* header, CPVRTMap<PVRTuint32, CPVRTMap<PVRTuint32, MetaDataBlock> >& MetaDataMap)
{
	if(PVRTTextureLoadFromPointer(pSource, &m_pAPI->m_uFontTexture, header, true, 0U, NULL, &MetaDataMap) != PVR_SUCCESS)
		return false;

	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

/*!***************************************************************************
 @Function			APIRenderStates
 @Description		Stores, writes and restores Render States
*****************************************************************************/
void CPVRTPrint3D::APIRenderStates(int nAction)
{
	PVRTMat4 mxOrtho;
	float fW, fH;
	// Saving or restoring states ?
	switch (nAction)
	{
	case 0:
		/******************************
		** SET PRINT3D RENDER STATES **
		******************************/
		// Set matrix with viewport dimensions
		fW = m_fScreenScale[0]*640.0f;
		fH = m_fScreenScale[1]*480.0f;

		mxOrtho = PVRTMat4::Ortho(0.0f, 0.0f, fW, -fH, -1.0f, 1.0f, PVRTMat4::OGL, m_bRotate);
		if(m_bRotate)
		{
			PVRTMat4 mxTrans = PVRTMat4::Translation(-fH,fW,0.0f);
			mxOrtho = mxOrtho * mxTrans;
		}

		// Set matrix modes
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		if(m_bUsingProjection)
			myglLoadMatrix(m_mProj.f);
		else
			myglLoadMatrix(mxOrtho.f);

		// Apply ModelView matrix (probably identity).
		glMultMatrixf(m_mModelView.f);

		// Reset
		m_bUsingProjection = false;
		PVRTMatrixIdentity(m_mModelView);

		// Disable lighting
		glDisable(GL_LIGHTING);

		// Culling
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);
		glCullFace(GL_FRONT);

		// Set client states
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

		glClientActiveTexture(GL_TEXTURE0);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		// texture
		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		myglTexEnv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

		// Blending mode
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Disable fog
		glDisable(GL_FOG);

		// Set Z compare properties
		glDisable(GL_DEPTH_TEST);

		// Disable vertex program
		if(m_pAPI->s_IsVGPSupported)
			glDisable(GL_VERTEX_PROGRAM_ARB);

#if defined(GL_OES_VERSION_1_1) || defined(GL_VERSION_ES_CM_1_1) || defined(GL_VERSION_ES_CL_1_1)
		// unbind the VBO for the mesh
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
		break;

	case 1:
		// Restore render states
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		// Restore matrix mode & matrix
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		glBindTexture(GL_TEXTURE_2D, 0);
		break;
	}
}

/****************************************************************************
** Local code
****************************************************************************/

/*!***************************************************************************
 @Function			APIDrawLogo
 @Description		
*****************************************************************************/
void CPVRTPrint3D::APIDrawLogo(const EPVRTPrint3DLogo uLogoToDisplay, const int ePos)
{
	GLuint	tex = 0;
	float fScale = 1.0f;
	SPVRTPrint3DAPI::SInstanceData& Data = (m_pAPI->m_pInstanceData ? *m_pAPI->m_pInstanceData : SPVRTPrint3DAPI::s_InstanceData);
	
	//If the logo isn't valid, return.
	switch(uLogoToDisplay)
	{
	case ePVRTPrint3DLogoIMG: 
		tex = Data.uTextureIMGLogo;
		break;
	case ePVRTPrint3DLogoPowerVR: 
		tex = Data.uTexturePowerVRLogo;
		break;
	default:
		return; // Logo not recognised
	}

	const float fLogoXSizeHalf = (128.0f / m_ui32ScreenDim[0]);
	const float fLogoYSizeHalf = (64.0f / m_ui32ScreenDim[1]);

	const float fLogoXShift = 0.035f / fScale;
	const float fLogoYShift = 0.035f / fScale;

	const float fLogoSizeXHalfShifted = fLogoXSizeHalf + fLogoXShift;
	const float fLogoSizeYHalfShifted = fLogoYSizeHalf + fLogoYShift;

	static float Vertices[] =
		{
			-fLogoXSizeHalf, fLogoYSizeHalf , 0.5f,
			-fLogoXSizeHalf, -fLogoYSizeHalf, 0.5f,
			fLogoXSizeHalf , fLogoYSizeHalf , 0.5f,
			fLogoXSizeHalf , -fLogoYSizeHalf, 0.5f
		};

	static float Colours[] = {
			f2vt(1.0f), f2vt(1.0f), f2vt(1.0f), f2vt(1.0f),
			f2vt(1.0f), f2vt(1.0f), f2vt(1.0f), f2vt(1.0f),
			f2vt(1.0f), f2vt(1.0f), f2vt(1.0f), f2vt(1.0f),
	 		f2vt(1.0f), f2vt(1.0f), f2vt(1.0f), f2vt(1.0f)
		};

	static float UVs[] = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};

	VERTTYPE *pVertices = ( (VERTTYPE*)&Vertices );
	VERTTYPE *pColours  = ( (VERTTYPE*)&Colours );
	VERTTYPE *pUV       = ( (VERTTYPE*)&UVs );

	// Matrices
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if(m_bRotate)
		glRotatef(90, 0, 0, 1);

	int nXPos = (ePos & eLeft) ? -1 : 1;
	int nYPos = (ePos & eTop) ? 1 : -1;

	glTranslatef(nXPos - (fLogoSizeXHalfShifted * fScale * nXPos), nYPos - (fLogoSizeYHalfShifted * fScale * nYPos), 0.0f);
	glScalef(fScale, fScale, 1);

	// Render states
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);

	glDisable(GL_DEPTH_TEST);

	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Vertices
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,VERTTYPEENUM,0,pVertices);

	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4,VERTTYPEENUM,0,pColours);

	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2,VERTTYPEENUM,0,pUV);

	glDrawArrays(GL_TRIANGLE_STRIP,0,4);

	glDisableClientState(GL_VERTEX_ARRAY);

	glDisableClientState(GL_COLOR_ARRAY);

	glClientActiveTexture(GL_TEXTURE0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);

	// Restore render states
	glDisable (GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

/*****************************************************************************
 End of file (PVRTPrint3DAPI.cpp)
*****************************************************************************/

