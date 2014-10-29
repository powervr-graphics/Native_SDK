/******************************************************************************

 @File         OGLESAlphaBlend.cpp

 @Title        OGLES blending modes

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows the different combinations of blending modes

******************************************************************************/

#include "PVRShell.h"
#include "OGLESTools.h"

/******************************************************************************
 Content file names
******************************************************************************/

// PVR texture files
const char c_szBackgroundTexFile[]	= "Background.pvr";
const char c_szForegroundTexFile[]	= "Foreground.pvr";

/******************************************************************************
 Const
******************************************************************************/
const float fBlockWidth  = 0.31f;
const float fBlockHeight = 0.284f;

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLESAlphaBlend : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D m_Print3D;

	// Texture handle
	GLuint	m_ui32TexBackground, m_ui32TexForeground;

	// Vertex Buffer Object (VBO) handle
	GLuint	m_ui32Vbo;
	GLuint	m_ui32IndexVbo;

	// The background
	CPVRTBackground m_Background;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	void DrawQuad(float x1, float y1);
};


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
bool OGLESAlphaBlend::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

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
bool OGLESAlphaBlend::QuitApplication()
{
    return true;
}

/*!****************************************************************************
 @Function		InitView
 @Return		bool		true if no error occured
 @Description	Code in InitView() will be called by PVRShell upon
				initialization or after a change in the rendering context.
				Used to initialize variables that are dependant on the rendering
				context (e.g. textures, vertex buffers, etc.)
******************************************************************************/
bool OGLESAlphaBlend::InitView()
{
	// Initialize Print3D
    bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight),bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	// Initialize Background
	if(m_Background.Init(0, bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Background\n");
		return false;
	}

	// Enables texturing
	glEnable(GL_TEXTURE_2D);

	// Loads the textures. For a detailed explanation see the Texturing training course
	if(PVRTTextureLoadFromPVR(c_szBackgroundTexFile, &m_ui32TexBackground) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot load the background texture\n");
		return false;
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szForegroundTexFile, &m_ui32TexForeground) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot load the foreground texture\n");
		return false;
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Set the clear colour
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	// Create VBO for the quad from our data


	// Interleaved vertex data
	float afVertices[] = {-fBlockWidth, -fBlockHeight, 0, // Position
							0, 0,							 // UV
		                    fBlockWidth, -fBlockHeight, 0,
							1, 0,
							-fBlockWidth, fBlockHeight, 0,
							0, 1,
							fBlockWidth, fBlockHeight, 0,
							1,1};

	// Draws an indexed triangle list
	unsigned short ui16Faces[] = {0,1,2, 2,1,3};

	glGenBuffers(1, &m_ui32Vbo);
	glGenBuffers(1, &m_ui32IndexVbo);

	unsigned int uiSize = 4 * (sizeof(float) * 5); // 4 vertices * stride (5 verttypes per vertex (3 pos + 2 uv))

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);

	// Set the buffer's data
	glBufferData(GL_ARRAY_BUFFER, uiSize, afVertices, GL_STATIC_DRAW);

	uiSize = 6 * sizeof(unsigned short);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ui32IndexVbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, ui16Faces, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLESAlphaBlend::ReleaseView()
{
	// Frees the texture
	glDeleteTextures(1, &m_ui32TexForeground);
	glDeleteTextures(1, &m_ui32TexBackground);

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

	return true;
}

/*!****************************************************************************
 @Function		RenderScene
 @Return		bool		true if no error occured
 @Description	Main rendering loop function of the program. The shell will
				call this function every frame.
				eglSwapBuffers() will be performed by PVRShell automatically.
				PVRShell will also manage important OS events.
				Will also manage relevent OS events. The user has access to
				these events through an abstraction layer provided by PVRShell.
******************************************************************************/
bool OGLESAlphaBlend::RenderScene()
{
	// Clears the color buffer.
	glClear(GL_COLOR_BUFFER_BIT);

	// Disable z-buffer test
	glDisable(GL_DEPTH_TEST);

	// Draws the background
	glDisable(GL_BLEND);

	// Use PVRTools to draw a background image
	m_Background.Draw(m_ui32TexBackground);

	/*
		Prepares to draw the different blend modes, activate blending.
		Now we can use glBlendFunc() to specify the blending mode wanted.
	*/
	glEnable(GL_BLEND);

	// Position and draw the first quad (Transparency)

	// Set up the blend function for this quad
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	DrawQuad(-0.435f, 0.33f);

	// Draw the text for this quad to the screen.
	m_Print3D.Print3D(18,12, 0.6f, PVRTRGBA(255, 255, 0, 255), "Transparency");
	m_Print3D.Print3D(7,16, 0.6f, PVRTRGBA(255, 255, 0, 255), "(SRC_ALPHA, 1 - SRC_ALPHA)");

	// Position and draw the second quad (Additive)

	glBlendFunc(GL_ONE, GL_ONE);
	DrawQuad(0.435f, 0.33f);

	m_Print3D.Print3D(66,12, 0.6f, PVRTRGBA(255, 255, 0, 255), "Additive");
	m_Print3D.Print3D(64,16, 0.6f, PVRTRGBA(255, 255, 0, 255), "(ONE, ONE)");

	// Position and draw the third quad (Modulate)

	glBlendFunc(GL_DST_COLOR, GL_ZERO);
	DrawQuad(-0.435f, -0.48f);

	m_Print3D.Print3D(22,52, 0.6f, PVRTRGBA(255, 255, 0, 255), "Modulate");
	m_Print3D.Print3D(14,56, 0.6f, PVRTRGBA(255, 255, 0, 255), "(DST_COLOR, ZERO)");

	// Position and draw the fourth quad (Modulate X 2)

	glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
	DrawQuad(0.435f, -0.48f);

	m_Print3D.Print3D(64,52, 0.6f, PVRTRGBA(255, 255, 0, 255), "Modulate X2");
	m_Print3D.Print3D(53,56, 0.6f, PVRTRGBA(255, 255, 0, 255), "(DST_COLOR, SRC_COLOR)");

	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("AlphaBlend", "", ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();
	return true;
}

/*!****************************************************************************
 @Function		DrawQuad
 @Input			x1			left coordinate of the rectangle to draw on the screen (between -1 and 1)
 @Input			y1			top coordinate of the rectangle to draw on the screen (between -1 and 1)
 @Input			x2			right coordinate of the rectangle to draw on the screen (between -1 and 1)
 @Input			y2			bottom coordinate of the rectangle to draw on the screen (between -1 and 1)
 @Input			fWrapCount	Number of times to tile the texture on the rectangle
 @Input			uiTexture	OpenGL ES texture handle to use
 @Description	Draws a given texture on a quad on the screen.
******************************************************************************/
void OGLESAlphaBlend::DrawQuad(float x1, float y1)
{
	// bind the VBO for the quad
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ui32IndexVbo);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	/*
		Sets the projection and model view matrices to identity.
		So the screen coordinates go from (-1,-1) to (+1,+1).
	*/
	glMatrixMode(GL_PROJECTION);

	PVRTMat4 mMat;
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);
	mMat = PVRTMat4::RotationZ( bRotate ? -90.0f * PVRT_PIf / 180.0f : 0 );

	glLoadMatrixf(mMat.f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Use the given texture
	glBindTexture(GL_TEXTURE_2D, m_ui32TexForeground);

	/*
		Draw a quad.
		Please refer to the training course IntroducingPVRShell for a detailed explanation.
	*/

	glPushMatrix();
	glTranslatef(x1, y1, 0);

	// Pass the vertex data
	glVertexPointer(3,GL_FLOAT,sizeof(float) * 5, 0);

	// Pass the texture coordinates data
	glTexCoordPointer(2,GL_FLOAT, sizeof(float) * 5, (unsigned char*) (sizeof(float) * 3));

	// Draws an indexed triangle list
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

	glPopMatrix();

	// unbind the VBO for the quad
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
	return new OGLESAlphaBlend();
}

/******************************************************************************
 End of file (OGLESAlphaBlend.cpp)
******************************************************************************/

