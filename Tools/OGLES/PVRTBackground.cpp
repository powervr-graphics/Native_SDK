/******************************************************************************

 @File         OGLES/PVRTBackground.cpp

 @Title        OGLES/PVRTBackground

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     ANSI compatible

 @Description  Function to draw a background texture.

******************************************************************************/
#include <string.h>
#include "../PVRTFixedPoint.h"
#include "../PVRTBackground.h"

/****************************************************************************
** Structures
****************************************************************************/

/*! The struct to include various API variables */
struct SPVRTBackgroundAPI
{
	// Vertex Buffer Object (VBO) handle
	GLuint	m_ui32Vbo;

	GLsizei m_i32Stride;
	unsigned char* m_pVertexOffset;
	unsigned char* m_pTextureOffset;
};

/****************************************************************************
** Class: CPVRTBackground
****************************************************************************/

/*****************************************************************************
 @Function			Background
 @Description		Initialises the background
*****************************************************************************/
CPVRTBackground::CPVRTBackground(void)
{
	m_bInit = false;
	m_pAPI  = 0;
}

/*!***************************************************************************
 @Function		~CPVRTBackground
 @Description	destructor
*****************************************************************************/
CPVRTBackground::~CPVRTBackground(void)
{
	delete m_pAPI;
	m_pAPI = 0;
}

/*!***************************************************************************
 @Function		Destroy
 @Description	Destroys the background and releases API specific resources
*****************************************************************************/
void CPVRTBackground::Destroy()
{
	m_bInit = false;

	delete m_pAPI;
	m_pAPI = 0;
}

/*!***************************************************************************
 @Function		Init
 @Input			pContext	A pointer to a PVRTContext
 @Input			bRotate		true to rotate texture 90 degrees.
 @Input			pszError	An option string for returning errors
 @Return 		PVR_SUCCESS on success
 @Description	Initialises the background
*****************************************************************************/
EPVRTError CPVRTBackground::Init(const SPVRTContext * const /*pContext*/, bool bRotate, CPVRTString *pszError)
{
	Destroy();

	m_pAPI = new SPVRTBackgroundAPI;

	if(!m_pAPI)
	{
		if(pszError)
			*pszError = "Error: Insufficient memory to allocate SPVRTBackgroundAPI.";

		return PVR_FAIL;
	}

	// The vertex data for non-rotated
	VERTTYPE afVertexData[20] = {f2vt(-1.0f), f2vt(-1.0f), f2vt(1.0f),  // Position
								 f2vt( 0.0f), f2vt( 0.0f),				// Texture coordinates
								 f2vt( 1.0f), f2vt(-1.0f), f2vt(1.0f),
								 f2vt( 1.0f), f2vt( 0.0f),
								 f2vt(-1.0f), f2vt( 1.0f), f2vt(1.0f),
								 f2vt( 0.0f), f2vt( 1.0f),
								 f2vt( 1.0f), f2vt( 1.0f), f2vt(1.0f),
								 f2vt( 1.0f), f2vt( 1.0f)};

	// The vertex data for rotated
	VERTTYPE afVertexDataRotated[20] = {f2vt(-1.0f), f2vt( 1.0f), f2vt(1.0f),
										f2vt( 1.0f), f2vt( 1.0f),
										f2vt(-1.0f), f2vt(-1.0f), f2vt(1.0f),
										f2vt( 0.0f), f2vt( 1.0f),
										f2vt( 1.0f), f2vt( 1.0f), f2vt(1.0f),
										f2vt( 1.0f), f2vt( 0.0f),
										f2vt( 1.0f), f2vt(-1.0f), f2vt(1.0f),
										f2vt( 0.0f), f2vt( 0.0f)};

	glGenBuffers(1, &m_pAPI->m_ui32Vbo);

	unsigned int uiSize = 4 * (sizeof(VERTTYPE) * 5); // 4 vertices * stride (5 verttypes per vertex (3 pos + 2 uv))

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_pAPI->m_ui32Vbo);

	// Set the buffer's data
	glBufferData(GL_ARRAY_BUFFER, uiSize, bRotate ? afVertexDataRotated : afVertexData, GL_STATIC_DRAW);

	// Setup the vertex and texture data pointers for conveniece
	m_pAPI->m_pVertexOffset  = 0;
	m_pAPI->m_pTextureOffset = (unsigned char*) (sizeof(VERTTYPE) * 3);

	// Setup the stride variable
	m_pAPI->m_i32Stride = sizeof(VERTTYPE) * 5;

	// All initialised
	m_bInit = true;

	// Unbind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return PVR_SUCCESS;
}


/*!***************************************************************************
 @Function		Draw
 @Input			ui32Texture	Texture to use
 @Return 		PVR_SUCCESS on success
 @Description	Draws a texture on a quad covering the whole screen.
*****************************************************************************/
EPVRTError CPVRTBackground::Draw(const GLuint ui32Texture)
{
	if(!m_bInit)
		return PVR_FAIL;

	glActiveTexture(GL_TEXTURE0);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ui32Texture);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	// Store matrices and set them to Identity
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisableClientState(GL_COLOR_ARRAY);

	// Set state
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_pAPI->m_ui32Vbo);

	// set pointers
	glVertexPointer(3  ,VERTTYPEENUM,m_pAPI->m_i32Stride, m_pAPI->m_pVertexOffset);
	glTexCoordPointer(2,VERTTYPEENUM,m_pAPI->m_i32Stride, m_pAPI->m_pTextureOffset);

	// Render geometry
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);

	// Disable client states
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	// Unbind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Recover matrices
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	return PVR_SUCCESS;
}

/*****************************************************************************
 End of file (PVRTBackground.cpp)
*****************************************************************************/

