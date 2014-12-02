/******************************************************************************

 @File         OGLESIntroducingPVRTools.cpp

 @Title        Introducing the PVRTools

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows how to use the tools to load textures and display text

******************************************************************************/
#include "PVRShell.h"
#include "OGLESTools.h"

/******************************************************************************
 Content file names
******************************************************************************/

// PVR texture files
const char c_szImageTexFile[] = "Image.pvr";

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLESIntroducingPVRTools : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// Texture handle
	GLuint	m_ui32Texture;

	// Vertex Buffer Object (VBO) handle
	GLuint	m_ui32Vbo;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();
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
bool OGLESIntroducingPVRTools::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*) PVRShellGet(prefReadPath));

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
bool OGLESIntroducingPVRTools::QuitApplication()
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
bool OGLESIntroducingPVRTools::InitView()
{
	// Is the screen rotated?
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	/*
		Initialize the textures used by Print3D.
		To properly display text, Print3D needs to know the viewport dimensions
		and whether the text should be rotated. We get the dimensions using the
		shell function PVRShellGet(prefWidth/prefHeight). We can also get the
		rotate parameter by checking prefIsRotated and prefFullScreen.
	*/
	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	// Set the clear colour
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	// Enable texturing
	glEnable(GL_TEXTURE_2D);

	/*
		Loads the texture using the tool function PVRTTextureLoadFromPointer.
		The first parameter is a pointer to the data in memory and the
		second parameter returns the resulting texture handle.
		We could also use PVRTTextureLoadFromPVR() to load the texture from an
		external .PVR file.
	*/
	if(PVRTTextureLoadFromPVR(c_szImageTexFile, &m_ui32Texture) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot load the texture\n");
		return false;
	}

	// The texture we loaded contains mipmap levels so we can interpolate between them
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Calculate the projection matrix
	if (bRotate)
	{
		PVRTMat4 mRotate = PVRTMat4::RotationZ(-90.0f * (PVRT_PIf / 180.0f));

		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(mRotate.f);
	}

	// Create VBO for the triangle from our data

	// Interleaved vertex data
	float afVertices[] = {	-0.4f,-0.4f,0.0f, // Pos
							0.0f,0.0f,		  // UVs
							0.4f,-0.4f,0.0f,
							1.0f,0.0f,
							0.0f,0.4f,0.0f,
							0.5f,1.0f};

	glGenBuffers(1, &m_ui32Vbo);

	unsigned int uiSize = 3 * (sizeof(float) * 5); // 3 vertices * stride (5 verttypes per vertex)

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);

	// Set the buffer's data
	glBufferData(GL_ARRAY_BUFFER, uiSize, afVertices, GL_STATIC_DRAW);

	// Unbind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Enable culling
	glEnable(GL_CULL_FACE);
	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLESIntroducingPVRTools::ReleaseView()
{
	// Frees the texture
	glDeleteTextures(1, &m_ui32Texture);

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
bool OGLESIntroducingPVRTools::RenderScene()
{
	// Clears the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Binds the loaded texture
	glBindTexture(GL_TEXTURE_2D, m_ui32Texture);

	// bind the VBO for the triangle
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);

	/*
		Draw a triangle.
		Please refer to the training course IntroducingPVRShell for a detailed explanation.
	*/

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT,sizeof(float) * 5, 0);

	// Pass the texture coordinates data
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2,GL_FLOAT,sizeof(float) * 5, (unsigned char*) (sizeof(float) * 3));

	// Draws a non-indexed triangle array
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// unbind the vertex buffer as we don't need it bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/*
		Display some text.
		Print3D() function allows to draw text anywhere on the screen using any color.
		Param 1: Position of the text along X (from 0 to 100 scale independent)
		Param 2: Position of the text along Y (from 0 to 100 scale independent)
		Param 3: Scale of the text
		Param 4: Colour of the text
		Param 5: Formated string (uses the same sintax as printf)
	*/
	m_Print3D.Print3D(8.0f, 30.0f, 1.5f, PVRTRGBA(64, 64, 170, 255), "example");

	/*
		DisplayDefaultTitle() writes a title and description text on the top left of the screen.
		It can also display the PVR logo (ePVRTPrint3DLogoPVR), the IMG logo (ePVRTPrint3DLogoIMG) or both (ePVRTPrint3DLogoPVR | ePVRTPrint3DLogoIMG).
		Set this last parameter to NULL not to display the logos.
	*/
	m_Print3D.DisplayDefaultTitle("IntroducingPVRTools", "Description", ePVRTPrint3DSDKLogo);

	// Tell Print3D to do all the pending text rendering now
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
	return new OGLESIntroducingPVRTools();
}

/******************************************************************************
 End of file (OGLESIntroducingPVRTools.cpp)
******************************************************************************/

