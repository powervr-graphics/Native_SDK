/******************************************************************************

 @File         OGLES2ProceduralTextures.cpp

 @Title        ProceduralTextures

 @Version

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Procedural texture example based on Steven Worley's  Cellular
               Texture Basis Functions.

******************************************************************************/

#include "PVRShell.h"
#include "OGLES3Tools.h"

#include "ProceduralTextures.h"

/******************************************************************************
 Defines
******************************************************************************/

#define VERTEX_ARRAY 0
#define TEXTURE_ARRAY 1

#define TEXTURE_SIZE   256

/******************************************************************************
 Enumerations
******************************************************************************/

enum eVisualisation
{
  FN0 = 0,
  FN1,
  FN2,
  FN3,
  FN1_MINUS_FN0,
  FN2_MINUS_FN1,
  SUM_FN0_FN1_FN2,
  NUM_VISUALISATIONS
};

const unsigned char c_pszColourSplineData[] =
{
	0,   0,   0,   255, 255, 255,  255,   0, 0,     0, 255,   0,
	255,   0,   0,   255, 255,   0,    0,   0, 0,   255, 255, 255,
	0, 255,   0,   255, 255, 255,    0, 255, 0,   255,   0, 255,
	0,   0, 255,   128,   0, 255,    0,   0, 0,   255, 128,   0,
};

/******************************************************************************
 Global strings
******************************************************************************/

// Visualisation modes
const char* c_szVisualisations[NUM_VISUALISATIONS] =
{
	"FN0", "FN1", "FN2", "FN3", "FN1_MINUS_FN0", "FN2_MINUS_FN1", "SUM_FN0_FN1_FN2"
};

const char* c_szVisualisationsDescriptions[NUM_VISUALISATIONS] =
{
	"FN0", "FN1", "FN2", "FN3", "FN1 - FN0", "FN2 - FN1", "FN0*a + FN1*b + FN2*c"
};

// Source and binary shaders
const char c_szFragShaderSrcFile[]	= "FragShader.fsh";
const char c_szFragShaderBinFile[]	= "FragShader.fsc";
const char c_szVertShaderSrcFile[]	= "VertShader.vsh";
const char c_szVertShaderBinFile[]	= "VertShader.vsc";

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES2ProceduralTextures : public PVRShell
{
public:
	// Print3D class used to display text
	CPVRTPrint3D m_Print3D;

	// Texture handle
	GLuint m_FnTexture;
	GLuint m_ui32ColourSplineTexture;

	unsigned int m_Width;
	unsigned int m_Height;

	ProceduralTextures* m_pProceduralTextures;
	unsigned int           m_uiGenerator;
	float                  m_Scalars[NUM_GENERATORS];

	GLuint m_uiVertShaderId;
	GLuint m_auiFragShaderId[NUM_VISUALISATIONS];
	GLuint m_auiShaderProgramId[NUM_VISUALISATIONS];
	GLint  m_aiColourSplineIndices[NUM_VISUALISATIONS];

	unsigned int m_uiVisualisation;
	bool         m_bDemoMode;

public:

	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	bool LoadShaders(CPVRTString* pErrorStr);
	bool GenerateFnTexture();

	bool HandleInput();
};

/*!****************************************************************************
 @Function		LoadShaders
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occurred
 @Description	Loads and compiles the shaders and links the shader programs
				required for this training course
******************************************************************************/
bool OGLES2ProceduralTextures::LoadShaders(CPVRTString* pErrorStr)
{
	const char* pszAttribs[] = { "inVertex", "inTexCoord" };
	const unsigned int uiNumAttribs = sizeof(pszAttribs) / sizeof(pszAttribs[0]);
	if (PVRTShaderLoadFromFile(c_szVertShaderBinFile, c_szVertShaderSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiVertShaderId, pErrorStr) != PVR_SUCCESS)
	{ return false; }

	for (unsigned int i = 0; i < NUM_VISUALISATIONS; i++)
	{
		if (PVRTShaderLoadFromFile(c_szFragShaderBinFile, c_szFragShaderSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_auiFragShaderId[i], pErrorStr, NULL, &c_szVisualisations[i], 1) != PVR_SUCCESS)
		{ return false; }

		if (PVRTCreateProgram(&m_auiShaderProgramId[i], m_uiVertShaderId, m_auiFragShaderId[i], pszAttribs, uiNumAttribs, pErrorStr) != PVR_SUCCESS)
		{ return false; }

		glUniform1i(glGetUniformLocation(m_auiShaderProgramId[i], "sTexture"), 0);
		glUniform1i(glGetUniformLocation(m_auiShaderProgramId[i], "sColourSpline"), 1);

		m_aiColourSplineIndices[i] = glGetUniformLocation(m_auiShaderProgramId[i], "uColourSplineIndex");
	}

	return true;
}

/*!****************************************************************************
 @Function		InitApplication
 @Return		bool		true if no error occurred
 @Description	Code in InitApplication() will be called by PVRShell once per
				run, before the rendering context is created.
				Used to initialize variables that are not dependent on it
				(e.g. external modules, loading meshes, etc.)
				If the rendering context is lost, InitApplication() will
				not be called again.
******************************************************************************/
bool OGLES2ProceduralTextures::InitApplication()
{
	PVRShellSet(prefApiMajorVersion, 3);
	PVRShellSet(prefApiMinorVersion, 1);

	m_FnTexture = 0;
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	m_Width = TEXTURE_SIZE;
	m_Height = TEXTURE_SIZE;

	m_uiGenerator = EUCLID;
	m_uiVisualisation = FN1_MINUS_FN0;
	m_bDemoMode = true;

	m_Scalars[EUCLID] = 0.26448223f;
	m_Scalars[CHESSBOARD] = 0.284799f;
	m_Scalars[MANHATTAN] = 0.134101f;

	m_pProceduralTextures = new ProceduralTextures();

	srand(PVRShellGetTime());

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
bool OGLES2ProceduralTextures::QuitApplication()
{
	delete m_pProceduralTextures;
	return true;
}

/*!****************************************************************************
 @Function		InitView
 @Return		bool		true if no error occured
 @Description	Code in InitView() will be called by PVRShell upon
				initialization or after a change in the rendering context.
				Used to initialize variables that are dependent on the rendering
				context (e.g. textures, vertex buffers, etc.)
******************************************************************************/
bool OGLES2ProceduralTextures::InitView()
{
	CPVRTString ErrorStr;
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	if (!LoadShaders(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Initialize Print3D textures
	if (m_Print3D.SetTextures(NULL, PVRShellGet(prefWidth), PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "Error: Cannot initialise Print3D.\n");
		return false;
	}

	if (!m_pProceduralTextures->Init(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	if (!GenerateFnTexture())
	{
		PVRShellSet(prefExitMessage, "Error: Failed to generate texture.\n");
		return false;
	}

	glGenTextures(1, &m_ui32ColourSplineTexture);
	glBindTexture(GL_TEXTURE_2D, m_ui32ColourSplineTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4, 4, 0, GL_RGB, GL_UNSIGNED_BYTE, c_pszColourSplineData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES2ProceduralTextures::ReleaseView()
{
	if (m_pProceduralTextures) { m_pProceduralTextures->Release(); }
	// Release texture
	glDeleteTextures(1, &m_FnTexture);
	glDeleteTextures(1, &m_ui32ColourSplineTexture);

	// Release shaders
	glDeleteShader(m_uiVertShaderId);
	for (unsigned int i = 0; i < NUM_VISUALISATIONS; i++)
	{
		glDeleteShader(m_auiFragShaderId[i]);
		glDeleteProgram(m_auiShaderProgramId[i]);
	}

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
bool OGLES2ProceduralTextures::RenderScene()
{
	if (!HandleInput())
	{ return false; }

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_ui32ColourSplineTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_FnTexture);

	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	glUseProgram(m_auiShaderProgramId[m_uiVisualisation]);

	const float fIndex = m_uiGenerator / (float)(NUM_GENERATORS - 1);
	glUniform1f(m_aiColourSplineIndices[m_uiGenerator], fIndex);

	glEnableVertexAttribArray(VERTEX_ARRAY);
	glEnableVertexAttribArray(TEXTURE_ARRAY);

	// Pass the vertex data
	GLfloat pfVertices[] = { -1.0f, -1.0f, 0.0f,   1.0f, -1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   -1.0f, 1.0f, 0.0f };
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, 0, pfVertices);

	// Pass the texture coordinates data
	GLfloat pfTexCoord[] = { 0.0f, 0.0f,   1.0f, 0.0f,   1.0f, 1.0f,   0.0f, 1.0f };
	glVertexAttribPointer(TEXTURE_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, pfTexCoord);

	unsigned short indices[] = { 0, 1, 3, 1, 2, 3 };
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

	glDisableVertexAttribArray(VERTEX_ARRAY);
	glDisableVertexAttribArray(TEXTURE_ARRAY);

	glUseProgram(0);

	m_Print3D.DisplayDefaultTitle("OpenGL ES Compute Shader Procedural Textures", NULL, ePVRTPrint3DSDKLogo);

	m_Print3D.Print3D(1.0f, 80.0f, 1.0f, 0xFFFFFFFF, "Metric: %s", m_pProceduralTextures->GetModeDescription((eGenerator)m_uiGenerator));
	m_Print3D.Print3D(1.0f, 90.0f, 1.0f, 0xFFFFFFFF, "Evaluator: %s", c_szVisualisationsDescriptions[m_uiVisualisation]);
	m_Print3D.Flush();

	return true;
}


/*!****************************************************************************
 @Function		HandleInput
 @Description	Handles user input and updates live variables accordingly.
******************************************************************************/
bool OGLES2ProceduralTextures::HandleInput()
{
	static unsigned long prevTime = PVRShellGetTime();
	unsigned long curTime = PVRShellGetTime();
	unsigned long deltaTime = curTime - prevTime;

	if (m_bDemoMode && deltaTime > 2500)
	{
		prevTime = curTime;
		m_uiVisualisation++;
		if (m_uiVisualisation == NUM_VISUALISATIONS)
		{
			m_uiVisualisation = 0;
			m_uiGenerator = (m_uiGenerator + 1) % NUM_GENERATORS;
		}

		return GenerateFnTexture();
	}
	else if (PVRShellIsKeyPressed(PVRShellKeyNameRIGHT) || (m_bDemoMode && deltaTime > 5000))
	{
		m_uiGenerator = (m_uiGenerator + 1) % NUM_GENERATORS;
		return GenerateFnTexture();

	}
	else if (PVRShellIsKeyPressed(PVRShellKeyNameLEFT))
	{
		if (m_uiGenerator > 0)
		{ m_uiGenerator = (m_uiGenerator - 1) % NUM_GENERATORS; }
		else
		{ m_uiGenerator = NUM_GENERATORS - 1; }
		return GenerateFnTexture();
	}
	else if (PVRShellIsKeyPressed(PVRShellKeyNameUP))
	{
		m_uiVisualisation = (m_uiVisualisation + 1) % NUM_VISUALISATIONS;
	}
	else if (PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
	{
		if (m_uiVisualisation > 0)
		{ m_uiVisualisation--; }
		else
		{ m_uiVisualisation = NUM_VISUALISATIONS - 1; }
	}
	else if (PVRShellIsKeyPressed(PVRShellKeyNameACTION1))
	{
		m_Scalars[m_uiGenerator] *= 0.95f;
		GenerateFnTexture();
	}
	else if (PVRShellIsKeyPressed(PVRShellKeyNameACTION2))
	{
		m_Scalars[m_uiGenerator] *= 1.05f;
		GenerateFnTexture();
	}

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occurred
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES2ProceduralTextures::GenerateFnTexture()
{
	if (!m_FnTexture)
	{
		glGenTextures(1, &m_FnTexture);
		glBindTexture(GL_TEXTURE_2D, m_FnTexture);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, m_Width, m_Height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	m_pProceduralTextures->GenerateIntoTexture((eGenerator)m_uiGenerator, m_FnTexture, static_cast<float>(m_Width), static_cast<float>(m_Height), m_Scalars[m_uiGenerator]);
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
	return new OGLES2ProceduralTextures();
}

/******************************************************************************
 End of file (OGLES2ProceduralTextures.cpp)
******************************************************************************/

