/******************************************************************************

 @File         OGLES2AlphaBlend.cpp

 @Title        OGLES2 blending modes

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows different combinations of blending modes

******************************************************************************/

#include "PVRShell.h"
#include "OGLES2Tools.h"

/******************************************************************************
 constants
******************************************************************************/

// Index to bind the attributes to vertex shaders
const int VERTEX_ARRAY = 0;

/******************************************************************************
 Content file names
******************************************************************************/

// Source and binary shaders
const char c_szFragShaderSrcFile[]	= "FragShader.fsh";
const char c_szFragShaderBinFile[]	= "FragShader.fsc";
const char c_szVertShaderSrcFile[]	= "VertShader.vsh";
const char c_szVertShaderBinFile[]	= "VertShader.vsc";

// PVR texture files
const char c_szBgTexFile[]			= "Background.pvr";
const char c_szFgTexFile[]			= "Foreground.pvr";

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/

class OGLES2AlphaBlend : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// OpenGL handles for shaders, textures and VBOs
	GLuint m_uiVertShader;
	GLuint m_uiFragShader;
	GLuint m_uiTexBackground;
	GLuint m_uiTexForeground;
	GLuint m_uiVbo;

	// Group shader programs and their uniform locations together
	struct
	{
		GLuint uiId;
		GLuint uiLowerLeftLoc;
		GLuint uiScaleMatrixLoc;
	}
	m_ShaderProgram;

	// Screen rotation
	bool m_bRotateScreen;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	bool LoadTextures(CPVRTString* pErrorStr);
	bool LoadShaders(CPVRTString* pErrorStr);
	void LoadVbos();

	void DrawQuad(float x1, float y1, float x2, float y2, GLuint uiTexture);
};

/*!****************************************************************************
 @Function		LoadTextures
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course
******************************************************************************/
bool OGLES2AlphaBlend::LoadTextures(CPVRTString* const pErrorStr)
{
	if(PVRTTextureLoadFromPVR(c_szBgTexFile, &m_uiTexBackground) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Failed to load texture.";
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szFgTexFile, &m_uiTexForeground) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Failed to load texture.";
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return true;
}

/*!****************************************************************************
 @Function		LoadShaders
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads and compiles the shaders and links the shader programs
				required for this training course
******************************************************************************/
bool OGLES2AlphaBlend::LoadShaders(CPVRTString* pErrorStr)
{
	/*
		Load and compile the shaders from files.
		Binary shaders are tried first, source shaders
		are used as fallback.
	*/
	if (PVRTShaderLoadFromFile(
			c_szVertShaderBinFile, c_szVertShaderSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiVertShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	if (PVRTShaderLoadFromFile(
			c_szFragShaderBinFile, c_szFragShaderSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_uiFragShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	/*
		Set up and link the shader program
	*/
	const char* aszAttribs[] = { "inVertex" };
	if (PVRTCreateProgram(&m_ShaderProgram.uiId, m_uiVertShader, m_uiFragShader, aszAttribs, 1, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}
	// Set the sampler2D variable to the first texture unit
	glUniform1i(glGetUniformLocation(m_ShaderProgram.uiId, "sTexture"), 0);

	// Store the location of uniforms for later use
	m_ShaderProgram.uiLowerLeftLoc = glGetUniformLocation(m_ShaderProgram.uiId, "LowerLeft");
	m_ShaderProgram.uiScaleMatrixLoc = glGetUniformLocation(m_ShaderProgram.uiId, "ScaleMatrix");

	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLES2AlphaBlend::LoadVbos()
{
	static float afQuadVerts[] = { 0, 0,  1, 0,  0, 1,  1, 1 };

	glGenBuffers(1, &m_uiVbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_uiVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(afQuadVerts), afQuadVerts, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
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
bool OGLES2AlphaBlend::InitApplication()
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
bool OGLES2AlphaBlend::QuitApplication()
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
bool OGLES2AlphaBlend::InitView()
{
	CPVRTString ErrorStr;

	/*
		Initialize VBO data
	*/
	LoadVbos();

	/*
		Load textures
	*/
	if (!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	/*
		Load and compile the shaders & link programs
	*/
	if (!LoadShaders(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	/*
		Get screen rotation state
	*/
	m_bRotateScreen = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	/*
		Initialize Print3D
	*/
	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), m_bRotateScreen) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	// Disable the depth test
	glDisable(GL_DEPTH_TEST);

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
bool OGLES2AlphaBlend::ReleaseView()
{
	// Delete textures
	glDeleteTextures(1, &m_uiTexForeground);
	glDeleteTextures(1, &m_uiTexBackground);

	// Delete program and shader objects
	glDeleteProgram(m_ShaderProgram.uiId);

	glDeleteShader(m_uiVertShader);
	glDeleteShader(m_uiFragShader);

	// Delete buffer objects
	glDeleteBuffers(1, &m_uiVbo);

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
bool OGLES2AlphaBlend::RenderScene()
{
	// Do our clear
	glClear(GL_COLOR_BUFFER_BIT);

	// Use the loaded shader program
	glUseProgram(m_ShaderProgram.uiId);

	// Draws the background
	glDisable(GL_BLEND);
	DrawQuad(-1, -1, +1, +1, m_uiTexBackground);

	/*
		Prepares to draw the different blend modes, activate blending.
		Now we can use glBlendFunc() to specify the blending mode wanted.
	*/
	glEnable(GL_BLEND);

	// Prepares the variables used to divide the screen in NUM_BLEND x NUM_BLEND rectangles
	float fX1 = -1;
	float fX2 = +1;
	float fY1 = -1;
	float fY2 = +.85f;
	float fPosX = fX1;
	float fPosY = fY1;
	float fMarginX = .25f;
	float fMarginY = .25f;
	float fBlockWidth = ((fX2-fX1) - fMarginX * 3.0f) * 0.5f;
	float fBlockHeight= ((fY2-fY1) - fMarginY * 3.0f) * 0.5f;

	//Position and draw the first quad (Transparency)
	fPosY = fY2 - fBlockHeight - fMarginY;
	fPosX += fMarginX;

	//Set up the blend function for this quad
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	DrawQuad(fPosX, fPosY, fPosX + fBlockWidth, fPosY + fBlockHeight, m_uiTexForeground);

	//Draw the text for this quad to the screen.
	m_Print3D.Print3D(18,12, 0.6f, 0xff00ffff, "Transparency");
	m_Print3D.Print3D(7,16, 0.6f, 0xff00ffff, "(SRC_ALPHA, 1 - SRC_ALPHA)");

	//Position and draw the second quad (Additive)
	fPosX += fMarginX + fBlockWidth;
	glBlendFunc(GL_ONE, GL_ONE);
	DrawQuad(fPosX, fPosY, fPosX + fBlockWidth, fPosY + fBlockHeight, m_uiTexForeground);

	m_Print3D.Print3D(66,12, 0.6f, 0xff00ffff, "Additive");
	m_Print3D.Print3D(64,16, 0.6f, 0xff00ffff, "(ONE, ONE)");

	//Position and draw the third quad (Modulate)
	fPosX = fX1 + fMarginX;
	fPosY -= fMarginY + fBlockHeight;

	glBlendFunc(GL_DST_COLOR, GL_ZERO);
	DrawQuad(fPosX, fPosY, fPosX + fBlockWidth, fPosY + fBlockHeight, m_uiTexForeground);

	m_Print3D.Print3D(22,52, 0.6f, 0xff00ffff, "Modulate");
	m_Print3D.Print3D(14,56, 0.6f, 0xff00ffff, "(DST_COLOR, ZERO)");

	//Position and draw the fourth quad (Modulate X 2)
	fPosX += fMarginX + fBlockWidth;
	glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
	DrawQuad(fPosX, fPosY, fPosX + fBlockWidth, fPosY + fBlockHeight, m_uiTexForeground);

	m_Print3D.Print3D(64,52, 0.6f, 0xff00ffff, "Modulate X2");
	m_Print3D.Print3D(53,56, 0.6f, 0xff00ffff, "(DST_COLOR, SRC_COLOR)");

	/* Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools */
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
 @Input			uiTexture	OpenGL ES texture handle to use
 @Description	Draws a given texture on a quad on the screen.
******************************************************************************/
void OGLES2AlphaBlend::DrawQuad(float x1, float y1, float x2, float y2, GLuint uiTexture)
{
	// Set the uniforms for rectangle position and width
	if (m_bRotateScreen)
	{
		glUniform2f(m_ShaderProgram.uiLowerLeftLoc, -y1, x1);
		float afMatrix[4] = { 0, x2 - x1, y1 - y2, 0 };
		glUniformMatrix2fv(m_ShaderProgram.uiScaleMatrixLoc, 1, GL_FALSE, afMatrix);
	}
	else
	{
		glUniform2f(m_ShaderProgram.uiLowerLeftLoc, x1, y1);
		float afMatrix[4] = { x2 - x1, 0, 0, y2 - y1 };
		glUniformMatrix2fv(m_ShaderProgram.uiScaleMatrixLoc, 1, GL_FALSE, afMatrix);
	}

	// Use the given texture
	glBindTexture(GL_TEXTURE_2D, uiTexture);

	// Bind the vertex buffer object
	glBindBuffer(GL_ARRAY_BUFFER, m_uiVbo);
	glEnableVertexAttribArray(VERTEX_ARRAY);
	glVertexAttribPointer(VERTEX_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Draws a short triangle strip
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
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
	return new OGLES2AlphaBlend();
}

/******************************************************************************
 End of file (OGLES2AlphaBlend.cpp)
******************************************************************************/

