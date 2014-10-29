/******************************************************************************

 @File         OGLImageConvolution.cpp

 @Title        ImageConvolution

 @Version

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows how to apply convolution kernels to images using Compute Shaders


******************************************************************************/

#include "CSImageConvolution.h"

/******************************************************************************
 Defines
******************************************************************************/

// Index to bind the attributes to vertex shaders
#define VERTEX_ARRAY	0
#define TEXCOORD_ARRAY	1

/******************************************************************************
 Content file names
******************************************************************************/

const char c_szFragShaderSrcFile[]	= "FragShader.fsh";
const char c_szVertShaderSrcFile[]	= "VertShader.vsh";

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES3ImageConvolution : public PVRShell
{
	//Text renderer
	CPVRTPrint3D m_Print3D;

	// OGL Extensions
	CPVRTgles3Ext m_Extensions;
	SPVRTContext m_PVRTContext;

	// Texture handle
	GLuint  m_uiVertShader;
	GLuint  m_uiFragShader;
	GLuint  m_uiShaderProgramId;
	GLuint  m_uiVBO;

	// Vertex array object
	GLuint 			m_VAO;

	bool m_bDemoMode;
	bool m_bBenchmarkKernel;

	CSImageConvolution m_ImageConvolution;

public:

	bool LoadShaders(CPVRTString* pErrorStr);

	void HandleInput();
	bool UpdateDemoMode();
	bool BenchmarkKernel(unsigned int times);

	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	OGLES3ImageConvolution() :	m_uiVertShader(0),
		m_uiFragShader(0),
		m_uiShaderProgramId(0),
		m_bDemoMode(false),
		m_bBenchmarkKernel(false),
		m_ImageConvolution(m_PVRTContext)
	{
	}
};


/*!****************************************************************************
 @Function		LoadShaders
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads and compiles the shaders and links the shader programs
				required for this training course
******************************************************************************/
bool OGLES3ImageConvolution::LoadShaders(CPVRTString* pErrorStr)
{
	//	Load and compile the shaders from files.
	if (PVRTShaderLoadFromFile(NULL, c_szVertShaderSrcFile, GL_VERTEX_SHADER, 0, &m_uiVertShader, pErrorStr) != PVR_SUCCESS)
	{
		*pErrorStr = "Vertex Shader : " + *pErrorStr; return false;
	}

	if (PVRTShaderLoadFromFile(NULL, c_szFragShaderSrcFile, GL_FRAGMENT_SHADER, 0, &m_uiFragShader, pErrorStr) != PVR_SUCCESS)
	{
		*pErrorStr = "Fragment Shader : " + *pErrorStr; return false;
	}

	// Set up and link the shader program
	const char* aszAttribs[] = { "inVertex", "inTexCoord" };
	if (PVRTCreateProgram(&m_uiShaderProgramId, m_uiVertShader, m_uiFragShader, aszAttribs, 2, pErrorStr) != PVR_SUCCESS)
	{
		*pErrorStr = "Compute Shader : " + *pErrorStr; return false;
	}

	// Set the sampler2D variable to the first texture unit
	glUniform1i(glGetUniformLocation(m_uiShaderProgramId, "sTexture"), 0);

	return true;
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
bool OGLES3ImageConvolution::InitApplication()
{
	PVRShellSet(prefApiMajorVersion, 3);
	PVRShellSet(prefApiMinorVersion, 1);

	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	m_bDemoMode = true;
	m_bBenchmarkKernel = false;

	PVRShellSet(prefSwapInterval, 0);

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
bool OGLES3ImageConvolution::QuitApplication()
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
bool OGLES3ImageConvolution::InitView()
{
	CPVRTString ErrorString;
	m_Extensions.LoadExtensions();
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	// Create a vertex array object.
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	// Create a vertex buffer
	glGenBuffers(1, &m_uiVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_uiVBO);

	// Vertex data - 2 Position (x/y) and 2 UVs
	const float afVertexData[] = {  -1.0f, -1.0f,  0.0f, 0.0f,
	                                1.0f, -1.0f,  1.0f, 0.0f,
	                                -1.0f,  1.0f,  0.0f, 1.0f,
	                                1.0f,  1.0f,  1.0f, 1.0f,
	                             };

	glBufferData(GL_ARRAY_BUFFER, sizeof(afVertexData), afVertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Initialize Print3D textures
	if (m_Print3D.SetTextures(&m_PVRTContext, PVRShellGet(prefWidth), PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D");
		return false;
	}

	// Load the shader required to render the processed image
	if (!LoadShaders(&ErrorString))
	{
		PVRShellSet(prefExitMessage, ErrorString.c_str());
		return false;
	}

	if (!m_ImageConvolution.init(ErrorString))
	{
		PVRShellSet(prefExitMessage, ErrorString.c_str());
		return false;
	}
	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES3ImageConvolution::ReleaseView()
{
	glDeleteShader(m_uiVertShader);
	glDeleteShader(m_uiFragShader);
	glDeleteProgram(m_uiShaderProgramId);

	m_ImageConvolution.release();
	m_Print3D.ReleaseTextures();

	//Delete the vertex buffer
	glDeleteBuffers(1, &m_uiVBO);

	// Release Vertex Array Object
	glDeleteVertexArrays(1, &m_VAO);

	return true;
}

/*!****************************************************************************
 @Function		RenderScene
 @Return		bool		true if no error occured
 @Description	Main rendering loop function of the program. The shell will
				call this function every frame.
				wglSwapBuffers() will be performed by PVRShell automatically.
				PVRShell will also manage important OS events.
				Will also manage relevent OS events. The user has access to
				these events through an abstraction layer provided by PVRShell.
******************************************************************************/
bool OGLES3ImageConvolution::RenderScene()
{
	if (m_bDemoMode)
	{ UpdateDemoMode(); }

	if (m_bBenchmarkKernel)
	{
		BenchmarkKernel(KERNEL_BENCHMARK_ITERATIONS);
		m_bBenchmarkKernel = false;
	}

	HandleInput();

	//Bind our original image as a texture
	//(i.e. start over)
	m_ImageConvolution.resetTextures();

	// Apply the current compute shader filter to our base image
	if (!m_ImageConvolution.executeCurrentFilter())
	{ return false; }

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Bind the processed image as a texture, and render it as a quad.
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_ImageConvolution.getCurrentInputTexture());

	// Use the shader program for the scene
	glUseProgram(m_uiShaderProgramId);

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_uiVBO);

	// Enable vertex arributes
	glEnableVertexAttribArray(VERTEX_ARRAY);
	glEnableVertexAttribArray(TEXCOORD_ARRAY);

	glVertexAttribPointer(VERTEX_ARRAY, 2, GL_FLOAT, GL_FALSE, 16, (void*)0);
	glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, 16, (void*)8);

	// Draw the quad
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// Disable vertex arributes
	glDisableVertexAttribArray(VERTEX_ARRAY);
	glDisableVertexAttribArray(TEXCOORD_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	static char pszBuffer[256];
	m_ImageConvolution.imprintCurrentFilterDescription(pszBuffer);

	m_Print3D.DisplayDefaultTitle("OpenGL ES 3.1 Image Convolution", pszBuffer, ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}


/*!****************************************************************************
 @Function		HandleInput
 @Description	Updates internal state and keeps track of the user input.
******************************************************************************/
void OGLES3ImageConvolution::HandleInput()
{
	if (PVRShellIsKeyPressed(PVRShellKeyNameDOWN) || PVRShellIsKeyPressed(PVRShellKeyNameLEFT))
	{
		m_ImageConvolution.previousFilter();
	}

	if (PVRShellIsKeyPressed(PVRShellKeyNameUP) || PVRShellIsKeyPressed(PVRShellKeyNameRIGHT) || PVRShellIsKeyPressed(PVRShellKeyNameSELECT))
	{
		m_ImageConvolution.nextFilter();
	}

	if (PVRShellIsKeyPressed(PVRShellKeyNameACTION1))
	{
		m_bDemoMode = !m_bDemoMode;
	}
}

/*!****************************************************************************
 @Function		UpdateDemoMode
 @Description	Switches internal modes every few seconds.
******************************************************************************/
bool OGLES3ImageConvolution::UpdateDemoMode()
{
	static unsigned long time = PVRShellGetTime() + 3000;
	unsigned long cur_time = PVRShellGetTime();

	// Change filter every e seconds.
	if (cur_time >= time + 3000)
	{
		time = cur_time;

		m_ImageConvolution.nextFilter();
	}

	return true;
}

/*!****************************************************************************
 @Function		BenchmarkKernel
 @Description	Runs a very quick benchmark of the currently set kernel
******************************************************************************/
bool OGLES3ImageConvolution::BenchmarkKernel(unsigned int times)
{
	unsigned long start_time = PVRShellGetTime();

	for (unsigned int i = 0; i < times; i++)
		if (!m_ImageConvolution.executeCurrentFilter())
		{ return false; }

	unsigned long end_time = PVRShellGetTime();

	float delta = (end_time - start_time) / (float)times;

	PVRShellOutputDebug("ImageConvolution benchmark -> Avg. execution time in ms: %f\n", delta);

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
	return new OGLES3ImageConvolution();
}

/******************************************************************************
 End of file (OGLImageConvolution.cpp)
******************************************************************************/

