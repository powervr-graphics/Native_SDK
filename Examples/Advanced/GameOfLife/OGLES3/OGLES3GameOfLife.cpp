/******************************************************************************

 @File         OGLES3GameOfLife.cpp

 @Title        GameOfLife

 @Version

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Game of life implemented with OpenGL compute shaders.
			   Requires the PVRShell.

******************************************************************************/

/* Keyboard controls:
 *  Up         - Pause
 *  Down       - Regenerate Population
 *  Left/Right - Change Evolution Function (disabled in demo mode)
 */
#include "PVRShell.h"

#include "GameOfLife.h"

/******************************************************************************
 Content file names
******************************************************************************/

const char c_szFragShaderSrcFile[]	= "FragShader.fsh";
const char c_szVertShaderSrcFile[]	= "VertShader.vsh";

/******************************************************************************
 Defines
******************************************************************************/

// Index to bind the attributes to vertex shaders
#define VERTEX_ARRAY	0
#define TEXCOORD_ARRAY	1

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES3GameOfLife : public PVRShell
{
public:
	// Print3D class used to display text
	CPVRTPrint3D m_Print3D;

	// OGL Context
	SPVRTContext m_PVRTContext;


	// Texture names
	GLuint m_BackgroundTexture;

	// Vertex array object
	//GLuint 			m_VAO;

	// Vertex buffer object
	GLuint			m_uiVBO;

	unsigned int m_Width;
	unsigned int m_Height;
	float m_GolCellSizeTimes10;

	GLuint  m_VertShader;
	GLuint  m_FragShader;
	GLuint  m_ShaderProgramId;

	bool m_demoMode;
	eGOLMode m_Mode;
	GameOfLife m_GameOfLife;

public:
	OGLES3GameOfLife();

	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();


	bool LoadShaders(CPVRTString* pErrorStr);
	bool GenerateTextures();
	GLuint	GetPetriDishSize();

	GLuint GetGolWidth() { return static_cast<GLuint>(10 * m_Width / m_GolCellSizeTimes10); }
	GLuint GetGolHeight() { return static_cast<GLuint>(10 * m_Height / m_GolCellSizeTimes10); }

	void UpdateDemoMode();
	void HandleInput();
	bool UpdateLife();
};

/*!****************************************************************************
 @Function		OGLES3GameOfLife
 @Description	Constructor
******************************************************************************/
OGLES3GameOfLife::OGLES3GameOfLife()
	: m_GameOfLife(m_PVRTContext), m_GolCellSizeTimes10(10), m_demoMode(true)
{}

/*!****************************************************************************
 @Function		OGLES3GameOfLife
 @Description	Constructor
******************************************************************************/
unsigned int OGLES3GameOfLife::GetPetriDishSize()
{
	return PVRT_MAX(m_Height, m_Width) / 4;
}


/*!****************************************************************************
 @Function		LoadShaders
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads and compiles the shaders and links the shader programs
				required for this training course
******************************************************************************/
bool OGLES3GameOfLife::LoadShaders(CPVRTString* const pErrorStr)
{
	//	Load and compile the shaders from files.
	//	Binary shaders are tried first, source shaders
	//	are used as fallback.
	if (PVRTShaderLoadFromFile(0, c_szVertShaderSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_VertShader, pErrorStr, &m_PVRTContext) != PVR_SUCCESS)
	{ return false; }

	if (PVRTShaderLoadFromFile(0, c_szFragShaderSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_FragShader, pErrorStr, &m_PVRTContext) != PVR_SUCCESS)
	{ return false; }

	// Set up and link the shader program
	const char* aszAttribs[] = { "inVertex", "inTexCoord" };

	if (PVRTCreateProgram(&m_ShaderProgramId, m_VertShader, m_FragShader, aszAttribs, 2, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	// Set the sampler2D variable to the first texture unit
	glUniform1i(glGetUniformLocation(m_ShaderProgramId, "sTexture"), 0);

	return true;
}


/*!****************************************************************************
 @Function		GenerateTextures
 @Output		pErrorStr		A CPVRTString describing the error on failure
 @Return		bool			true if no error occured
 @Description	Generates the spotlight texture
******************************************************************************/
bool OGLES3GameOfLife::GenerateTextures()
{
	//We want to create a petri - dish effect. Keep in mind, this will only be used for graphical effect,\
	//drawn, so a low-res square texture is both faster and easier to construct.
	CPVRTArray<unsigned char>petriDish(GetPetriDishSize()*GetPetriDishSize());

	//THe center:
	PVRTfloat32 radius = GetPetriDishSize() * .5f;
	PVRTVec2 middle = PVRTVec2(radius, radius);


	// This will generate the petri dish texture
	for (unsigned int y = 0; y < GetPetriDishSize() ; y++)
	{
		for (unsigned int x = 0; x < GetPetriDishSize() ; x++)
		{
			PVRTVec2 r((float)x, (float)y); r -= middle;

			//Clamped between 0 and 255, keep everything that is slightly bigger than the actual radius 1.5
			petriDish[y * GetPetriDishSize() + x] = (unsigned char)(PVRT_MAX(0.f, PVRT_MIN(255.f, (1.2f - r.length() / radius) * 255.f)));
		}
	}

	glGenTextures(1, &m_BackgroundTexture);
	glBindTexture(GL_TEXTURE_2D, m_BackgroundTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, GetPetriDishSize(), GetPetriDishSize(), 0, GL_RED, GL_UNSIGNED_BYTE, &petriDish[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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
bool OGLES3GameOfLife::InitApplication()
{
	PVRShellSet(prefApiMajorVersion, 3);
	PVRShellSet(prefApiMinorVersion, 1);

	m_Mode = GOL_RANDOM;

	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	srand(PVRShellGetTime());

	//UNCOMMENT FOR VSYNC
	//PVRShellSet(prefSwapInterval, 0);

	m_Mode = GOL_RANDOM;

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
bool OGLES3GameOfLife::QuitApplication()
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
bool OGLES3GameOfLife::InitView()
{
	m_Width = PVRShellGet(prefWidth);
	m_Height = PVRShellGet(prefHeight);

	CPVRTString errorStr;

	if (GameOfLife::GetGlErrorString(errorStr))
	{
		PVRShellSet(prefExitMessage, ("Failed to InitView - GL error: " + errorStr).c_str());
		return false;
	}

	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	if (GameOfLife::GetGlErrorString(errorStr))
	{
		PVRShellSet(prefExitMessage, ("Failed to load extensions - GL error: " + errorStr).c_str());
		return false;
	}

	// Create a vertex array object.
	//glGenVertexArrays(1, &m_VAO);
	//glBindVertexArray(m_VAO);

	if (GameOfLife::GetGlErrorString(errorStr))
	{
		PVRShellSet(prefExitMessage, ("Failed to generate and bind VAO - GL error: " + errorStr).c_str());
		return false;
	}

	// Create a vertex buffer object
	glGenBuffers(1, &m_uiVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_uiVBO);

	if (GameOfLife::GetGlErrorString(errorStr))
	{
		PVRShellSet(prefExitMessage, ("Failed to generate and bind VBO - GL error: " + errorStr).c_str());
		return false;
	}

	// Initialise the vertex data (3 positions, 2 UVs)
	GLfloat pfVertexData[] = {	-1.0f, -1.0f, 0.0f,
	                            0.0f,  0.0f,
	                            1.0f, -1.0f, 0.0f,
	                            1.0f,  0.0f,
	                            -1.0f,  1.0f, 0.0f,
	                            0.0f,  1.0f,
	                            1.0f,  1.0f, 0.0f,
	                            1.0f,  1.0f
	                         };

	glBufferData(GL_ARRAY_BUFFER, sizeof(pfVertexData), pfVertexData, GL_STATIC_DRAW);

	if (GameOfLife::GetGlErrorString(errorStr))
	{
		PVRShellSet(prefExitMessage, ("Failed to set VBO data - GL error: " + errorStr).c_str());
		return false;
	}

	// Initialize Print3D textures
	if (m_Print3D.SetTextures(&m_PVRTContext, PVRShellGet(prefWidth), PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Could not initialize Print3D\n");
		return false;
	}

	if (GameOfLife::GetGlErrorString(errorStr))
	{
		PVRShellSet(prefExitMessage, ("GL error on Print3D - GL error: " + errorStr).c_str());
		return false;
	}

	if (!GenerateTextures())
	{
		PVRShellSet(prefExitMessage, ("GenerateTextures: " + errorStr).c_str());
		return false;
	}

	//	Load and compile the shaders & link programs
	if (!LoadShaders(&errorStr))
	{
		PVRShellSet(prefExitMessage, ("LoadShaders: " + errorStr).c_str());
		return false;
	}

	//	Initialize the OpenGL game of life
	if (!m_GameOfLife.Init(errorStr, GetGolWidth(), GetGolHeight()))
	{
		PVRShellSet(prefExitMessage, ("GameOfLife: " + errorStr).c_str());
		return false;
	}

	// Initialize for the first iteration
	if (!m_GameOfLife.SetMode(m_Mode, GetGolWidth(), GetGolHeight(), errorStr)) { errorStr = "ChangeMode failed:\n" + errorStr; return false; }

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES3GameOfLife::ReleaseView()
{
	// Release textures
	glDeleteTextures(1, &m_BackgroundTexture);

	glDeleteShader(m_VertShader);
	glDeleteShader(m_FragShader);
	glDeleteProgram(m_ShaderProgramId);

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

	m_GameOfLife.Release();

	// Release Vertex Array Object
	//glDeleteVertexArrays(1, &m_VAO);

	return true;
}

/*!****************************************************************************
 @Function		RenderScene
 @Return		bool		true if no error occured
 @Description	Main rendering loop function of the program. The shell will
				call this function every frame.
				wglSwapBuffers() will be performed by PVRShell automatically.
				PVRShell will also manage important OS events. The user has
				access to these events through an abstraction layer provided
				by PVRShell.
******************************************************************************/
bool OGLES3GameOfLife::RenderScene()
{
	CPVRTString errorStr;
	HandleInput();
	if (m_demoMode) { UpdateDemoMode(); }
	if (!m_GameOfLife.UpdateLife(errorStr))
	{
		PVRShellSet(prefExitMessage, ("ERROR: Updating life failed with message: " + errorStr).c_str());
		return false;
	}

	glBlendFunc(GL_ZERO, GL_SRC_COLOR);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use the shader program for the scene
	glUseProgram(m_ShaderProgramId);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_GameOfLife.GetCurrentInputTexture());

	// Enable vertex arributes
	//Bind the vertex and index buffers.
	glBindBuffer(GL_ARRAY_BUFFER, m_uiVBO);

	glEnableVertexAttribArray(VERTEX_ARRAY);
	glEnableVertexAttribArray(TEXCOORD_ARRAY);

	// Pass the vertex data
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
	glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));

	// Draw the quad
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


	glBindTexture(GL_TEXTURE_2D, m_BackgroundTexture);

	// Simply multiply the framebuffer contents with the spotlight texture to
	// create the petri dish / microscope effect
	glEnable(GL_BLEND);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisable(GL_BLEND);

	//Unbind the vertex and index buffers.
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Disable vertex arributes
	glDisableVertexAttribArray(VERTEX_ARRAY);
	glDisableVertexAttribArray(TEXCOORD_ARRAY);

	glBindTexture(GL_TEXTURE_2D, 0);

	//glBindVertexArray(0);

	m_Print3D.DisplayDefaultTitle("OpenGL ES 3.1 GameOfLife", m_GameOfLife.GetModeDescription(m_Mode), ePVRTPrint3DSDKLogo);

	char buffer2[256]; sprintf(buffer2, "Left / right - Speed (x1/%d)", m_GameOfLife.GetSimulationUpdatePeriod() + 1);
	char buffer3[256]; sprintf(buffer3, "Up / Down - Cell size (x%.2f)", m_GolCellSizeTimes10 / 10.f);

	m_Print3D.Print3D(0.5f, 90.0f, 0.4f, 0xFFA0A0A0, "Space         - Mode");
	m_Print3D.Print3D(0.5f, 92.5f, 0.4f, 0xFFA0A0A0, buffer2);
	m_Print3D.Print3D(0.5f, 95.0f, 0.4f, 0xFFA0A0A0, buffer3);
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		HandleInput
 @Description	Handles user input and updates live variables accordingly.
******************************************************************************/
void OGLES3GameOfLife::HandleInput()
{
	CPVRTString tempstr;
	bool toggleMode = PVRShellIsKeyPressed(PVRShellKeyNameACTION2);
	if (toggleMode)
	{
		// Toggle to next mode, wrap after last mode
		m_Mode = (eGOLMode)((m_Mode + 1) % GOL_NUM_MODES);
	}
	if (PVRShellIsKeyPressed(PVRShellKeyNameSELECT) ||	toggleMode)
	{
		m_GameOfLife.SetMode(m_Mode, GetGolWidth(), GetGolHeight(), tempstr);
	}
	if (PVRShellIsKeyPressed(PVRShellKeyNameACTION1))
	{
		m_demoMode = !m_demoMode;
	}
	if (PVRShellIsKeyPressed(PVRShellKeyNameLEFT))
	{
		m_GameOfLife.SetSimulationUpdatePeriod(m_GameOfLife.GetSimulationUpdatePeriod() + 1);
	}
	if (PVRShellIsKeyPressed(PVRShellKeyNameRIGHT))
	{
		m_GameOfLife.SetSimulationUpdatePeriod(PVRT_MAX(0, m_GameOfLife.GetSimulationUpdatePeriod() - 1));
	}
	if (PVRShellIsKeyPressed(PVRShellKeyNameUP))
	{
		m_GolCellSizeTimes10 = PVRT_MIN(100.f, (m_GolCellSizeTimes10 >= 10 ? m_GolCellSizeTimes10 + 10 : m_GolCellSizeTimes10 + 1));
		m_GameOfLife.SetMode(m_Mode, GetGolWidth(), GetGolHeight(), tempstr);
	}
	if (PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
	{
		m_GolCellSizeTimes10 = PVRT_MAX(1.f, (m_GolCellSizeTimes10 > 10 ?  m_GolCellSizeTimes10 - 10 : m_GolCellSizeTimes10 - 1));
		m_GameOfLife.SetMode(m_Mode, GetGolWidth(), GetGolHeight(), tempstr);
	}
}


/*!****************************************************************************
 @Function		UpdateDemoMode
 @Description	Modifies the simulation according to its predetermined script
******************************************************************************/
void OGLES3GameOfLife::UpdateDemoMode()
{
	static CPVRTString tmpstr;
	static const long timeToReset = 30000;
	static unsigned long time = PVRShellGetTime();
	unsigned long cur_time = PVRShellGetTime();

	// Increase the iteration counter every second
	if (cur_time >= time + timeToReset)
	{
		m_Mode = (eGOLMode)((m_Mode + 1) % GOL_NUM_MODES);
		time = cur_time;
		m_GameOfLife.SetMode(m_Mode, GetGolWidth(), GetGolHeight(), tmpstr);
	}

	return;
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
	return new OGLES3GameOfLife();
}

/******************************************************************************
 End of file (OGLES3GameOfLife.cpp)
******************************************************************************/

