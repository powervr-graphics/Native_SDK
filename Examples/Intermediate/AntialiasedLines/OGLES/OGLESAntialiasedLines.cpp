/******************************************************************************

 @File         OGLESAntialiasedLines.cpp

 @Title        Antialiased Lines

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows how to render to a pBuffer surface and bind that to a
               texture.

******************************************************************************/

/****************************************************************************
 ** Includes                                                               **
 ****************************************************************************/

#include "PVRShell.h"
#include "OGLESTools.h"
#include <math.h>
#include <stddef.h>

/****************************************************************************
 ** Constants                                                              **
 ****************************************************************************/
const int c_iNumLines = 29;
const float c_fLineArc = 13.5f;
const float c_fLineWidth = 7.0f;
// 2D vertex with color
struct SVertex
{
	PVRTVec2 vPosition;
	GLuint uiColor;
};

const unsigned int SVertexColourOffset = (unsigned int)sizeof(PVRTVec2);

// 2D vertex with texcoords and color
struct STexVertex
{
	PVRTVec2 vPosition;
	PVRTVec2 vTexcoord;
	GLuint uiColor;
};

const unsigned int STexVertexTexCoordOffset = (unsigned int)sizeof(PVRTVec2);
const unsigned int STexVertexColourOffset = STexVertexTexCoordOffset+(unsigned int)sizeof(PVRTVec2);

/****************************************************************************
** Class: OGLESAntialiasedLines
****************************************************************************/
class OGLESAntialiasedLines : public PVRShell
{
	// Texture ID
	GLuint m_uiTexture;

	// Vertex and index buffers
	GLuint m_uiVbos[3];

	int m_iWidth, m_iHeight;

	// Print3D
	CPVRTPrint3D  m_Print3D;

	// Method to tessellate antialiased line
	void TessellateLine(const PVRTVec2& vPointA, const PVRTVec2& vPointB,
		float fWidth, GLuint uiColor, STexVertex* pVertexArray,
		GLushort u16StartIndex, GLushort* pu16IndexArray);

public:

	// PVRShell functions
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();
};

void OGLESAntialiasedLines::TessellateLine(const PVRTVec2& vPointA, const PVRTVec2& vPointB,
	float fWidth, GLuint uiColor, STexVertex* pVertexArray,
	GLushort u16StartIndex, GLushort* pu16IndexArray)
{
	// Calculate the normalised tangent and normal for the line,
	// multiplied by the line width
	PVRTVec2 vDiff = vPointA - vPointB;
	PVRTVec2 vTangent = vDiff * fWidth / vDiff.length();
	PVRTVec2 vNormal = vTangent.rotated90();

    /*
	We write eight vertices to the vertex array. The rectangles (0,1,2,3) and
	(4,5,6,7) represent the round line caps. The rectangle (2,3,4,5) is the
	main line segment.

	  -t                 +t
	0---->2--  ...  --4<----6
	^\    |\_         |\    ^ +n
	| \   |  \        | \   |
	A  \  |    ...    |  \  B
	|   \ |        \_ |   \ |
	v    \|          \|    \v -n
	1---->3--  ...  --5<----7
	  cap      line     cap

	Note that for this example the caps are entirely between the end points
	(A and B in the diagram above). Even vertices are displaced along the
	positive normal (+n), odd vertices are displaced along the negative normal.
	The pairs (2, 3) and (4, 5) are shifted inwards along the line tangent.

	To achieve the antialiasing we use a special texture where on the U-axis
	there is an opaque segment from 0 to 0.5. We then use -0.25 as texcoord
	for the even vertices and 0.75 for the odd vertices. This is necessary so
	it still looks ok when the 2 texel wide mip level is used. The GL_REPEAT
	texture wrap mode makes sure the transparent part on the right side of the
	texture is repeated to the left of the line/opaque part.
	Texture filtering and blending then results in smooth lines.

	The method breaks down when the line geometry gets less than 2 pixels
	wide (actual line width < 1). In this case we should clamp the line
	width to 1 and use the actual line width as an alpha factor, so very thin
	lines will smoothly fade out.

	texcoords:-1/2 -1/4 0          3/4  1
                |   |   |           |   |
	miplevel 2: |   0   |   1       0   |
	miplevel 1: | 0   0 | 1   1   0   0 |
	miplevel 0: |0 0 0 0|1 1 1 1 0 0 0 0|
	*/

	pVertexArray[0].vPosition = vPointA + vNormal;
	pVertexArray[0].vTexcoord = PVRTVec2(-0.25f, 0.245f);
	pVertexArray[1].vPosition = vPointA - vNormal;
	pVertexArray[1].vTexcoord = PVRTVec2(0.75f, 0.245f);

	pVertexArray[2].vPosition = vPointA + vNormal - vTangent;
	pVertexArray[2].vTexcoord = PVRTVec2(-0.25f, 0.75f);
	pVertexArray[3].vPosition = vPointA - vNormal - vTangent;
	pVertexArray[3].vTexcoord = PVRTVec2(0.75f, 0.75f);

	pVertexArray[4].vPosition = vPointB + vNormal + vTangent;
	pVertexArray[4].vTexcoord = PVRTVec2(-0.25f, 0.75f);
	pVertexArray[5].vPosition = vPointB - vNormal + vTangent;
	pVertexArray[5].vTexcoord = PVRTVec2(0.75f, 0.75f);

	pVertexArray[6].vPosition = vPointB + vNormal;
	pVertexArray[6].vTexcoord = PVRTVec2(-0.25f, 0.245f);
	pVertexArray[7].vPosition = vPointB - vNormal;
	pVertexArray[7].vTexcoord = PVRTVec2(0.75f, 0.245f);

	// The color is constant for each line, but we write it to the vertex
	// array so we can render multiple lines in one draw call
	int i;
	for (i = 0; i < 8; ++i)
	{
		pVertexArray[i].uiColor = uiColor;
	}

	// Write indices to index buffer
	GLushort au16Indices[18] = { 0, 1, 2,  2, 1, 3,  2, 3, 4,  4, 3, 5,  4, 5, 6,  6, 5, 7 };
	for (i = 0; i < 18; ++i)
	{
		pu16IndexArray[i] = au16Indices[i] + u16StartIndex;
	}
}

/*!****************************************************************************
 @Function		InitApplication
 @Return		bool		true if no error occured
 @Description	Code in InitApplication() will be called by PVRShell once per
				run, before the rendering context is created.
				Used to initialize variables that are not dependant on it
				(e.g. external modules, loading meshes, etc.)
				If the rendering context is lost, InitApplication() will
				not be called again.
******************************************************************************/
bool OGLESAntialiasedLines::InitApplication()
{
	CPVRTResourceFile::SetReadPath((const char*) PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));
	return true;
}

/*!****************************************************************************
 @Function		QuitApplication
 @Return		bool		true if no error occured
 @Description	Code in QuitApplication() will be called by PVRShell once per
				run, just before exiting the program.
				If the rendering context is lost, QuitApplication() will
				not be called.
******************************************************************************/
bool OGLESAntialiasedLines::QuitApplication()
{
    return true;
}

/*!****************************************************************************
 @Function		InitView
 @Return		bool		true if no error occurred
 @Description	Code in InitView() will be called by PVRShell upon
				initialization or after a change in the rendering context.
				Used to initialize variables that are dependent on the rendering
				context (e.g. textures, vertex buffers, etc.)
******************************************************************************/
bool OGLESAntialiasedLines::InitView()
{
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);
	m_iWidth  = PVRShellGet(prefWidth);
	m_iHeight = PVRShellGet(prefHeight);

	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	// Initialise Print3D
	if(m_Print3D.SetTextures(0, m_iWidth, m_iHeight, bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D.\n");
		return false;
	}

	// Initialise the texture
	if (PVRTTextureLoadFromPVR("LineRound.pvr", &m_uiTexture) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load texture.\n");
		return false;
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Initialise geometry
	SVertex *paVertices = new SVertex[c_iNumLines * 2];       // 2 vertices per GL_LINE
	STexVertex *paTexVertices = new STexVertex[c_iNumLines * 8]; // 8 vertices per AA line (includes caps)
	GLushort *paui16Indices = new GLushort[c_iNumLines * 18];  // 18 indices per AA line (6 triangles)

	if(!paVertices || !paTexVertices || !paui16Indices)
	{
		delete[] paVertices;
		delete[] paTexVertices;
		delete[] paui16Indices;
		PVRShellSet(prefExitMessage, "ERROR: Failed to allocate line vertices and indices.\n");
		return false;
	}

	srand(0);
	float fAngleStep = PVRT_TWO_PI / c_iNumLines;
	float fSize = PVRT_MIN(m_iWidth, m_iHeight) * 0.4f;
	for (int i = 0; i < c_iNumLines; ++i)
	{
		// Place the line vertices on a circle
		paVertices[i*2].vPosition.x = fSize * PVRTSIN(fAngleStep * (i + c_fLineArc));
		paVertices[i*2].vPosition.y = fSize * PVRTCOS(fAngleStep * (i + c_fLineArc));
		paVertices[i*2+1].vPosition.x = fSize * PVRTSIN(fAngleStep * i);
		paVertices[i*2+1].vPosition.y = fSize * PVRTCOS(fAngleStep * i);

		// Pick a random RGB color
		paVertices[i*2].uiColor = (0xFF << 24) +
			((rand() & 0xFF) << 16) + ((rand() & 0xFF) << 8) + (rand() & 0xFF);
		paVertices[i*2+1].uiColor = paVertices[i*2].uiColor;

		// Tessellate the antialiased line
		TessellateLine(paVertices[i*2].vPosition, paVertices[i*2+1].vPosition,
			c_fLineWidth, paVertices[i*2].uiColor,
			&paTexVertices[i * 8], i * 8, &paui16Indices[i * 18]);
	}

	// We use 3 VBOs for clarity:
	// 0: AA line vertex data
	// 1: AA line index data
	// 2: GL_LINES vertex data
	glGenBuffers(3, m_uiVbos);

	// Bind the VBOs and fill them with data
	glBindBuffer(GL_ARRAY_BUFFER, m_uiVbos[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(*paTexVertices) * c_iNumLines * 8, paTexVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiVbos[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*paui16Indices) * c_iNumLines * 18, paui16Indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_uiVbos[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(*paVertices) * c_iNumLines * 2, paVertices, GL_STATIC_DRAW);

	// Unbind buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Set projection to use pixel coordinates
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrthof(0, (float)PVRShellGet(prefWidth), (float)PVRShellGet(prefHeight), 0, 0, 1);

	delete[] paVertices;
	delete[] paTexVertices;
	delete[] paui16Indices;

	// Setup our render states
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
	return true;
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns        : Nothing
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool OGLESAntialiasedLines::ReleaseView()
{
	// Release the Print3D textures and windows
	m_Print3D.ReleaseTextures();

	// Delete the textures and buffers we created
	glDeleteTextures(1, &m_uiTexture);
	glDeleteBuffers(3, m_uiVbos);

	return true;
}

/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occurred
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool OGLESAntialiasedLines::RenderScene()
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	// setup render states
	glBindTexture(GL_TEXTURE_2D, m_uiTexture);
	glDisable(GL_DEPTH_TEST);

	// translate to centre, animate rotation and scale
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(m_iWidth * 0.5f, m_iHeight * 0.5f, 0);

	unsigned long ulTime = PVRShellGetTime() % 36000;
	glRotatef(ulTime * 0.01f, 0, 0, 1);
	float fScale = PVRTSIN(PVRT_PI * (ulTime / 9000.f)) * 0.5f + 0.6f;
	glScalef(fScale, fScale, 1);

	if ((ulTime / 2250) & 1)
	{
		// render aliased lines
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, m_uiVbos[2]);
		glVertexPointer(2, GL_FLOAT, sizeof(SVertex), 0);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(SVertex), (GLvoid*)SVertexColourOffset);

		glLineWidth(c_fLineWidth * fScale);
		glDrawArrays(GL_LINES, 0, c_iNumLines * 2);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		m_Print3D.DisplayDefaultTitle("Antialiased Lines", "GL_LINES (aliased)", ePVRTPrint3DSDKLogo);
	}
	else
	{
		// Render antialiased lines with blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, m_uiVbos[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiVbos[1]);
		glVertexPointer(2, GL_FLOAT, sizeof(STexVertex), 0);
		glTexCoordPointer(2, GL_FLOAT, sizeof(STexVertex), (GLvoid*)STexVertexTexCoordOffset);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(STexVertex), (GLvoid*)STexVertexColourOffset);

		glDrawElements(GL_TRIANGLES, c_iNumLines * 18, GL_UNSIGNED_SHORT, 0);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glDisable(GL_BLEND);

		m_Print3D.DisplayDefaultTitle("Antialiased Lines", "Textured rectangles (antialiased)", ePVRTPrint3DSDKLogo);
	}

	// Flush all Print3D commands
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		NewDemo
 @Return		PVRShell*		The demo supplied by the user
 @Description	This function must be implemented by the user of the shell.
				The user should return its PVRShell object defining the
				behaviour of the application.
******************************************************************************/
PVRShell* NewDemo()
{
	return new OGLESAntialiasedLines();
}

/*****************************************************************************
 End of file (OGLESAntialiasedLines.cpp)
*****************************************************************************/

