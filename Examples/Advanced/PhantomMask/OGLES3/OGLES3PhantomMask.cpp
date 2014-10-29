/******************************************************************************

 @File         OGLES3PhantomMask.cpp

 @Title        Introducing the POD 3D file format

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows how to load POD files and play the animation with basic
               lighting

******************************************************************************/
#include <string.h>

#include "PVRShell.h"
#include "OGLES3Tools.h"

/******************************************************************************
 shader attributes
******************************************************************************/
// vertex attributes
enum EVertexAttrib {
	VERTEX_ARRAY, NORMAL_ARRAY, TEXCOORD_ARRAY, eNumAttribs };
const char* g_aszAttribNames[] = {
	"inVertex", "inNormal", "inTexCoord" };

// shader uniforms
enum ESHUniform {
	eSHMVPMatrix, eSHModel, ecAr, ecAg, ecAb, ecBr, ecBg, ecBb, ecC, eNumSHUniforms };

const char* g_aszSHUniformNames[] = {
	"MVPMatrix", "Model", "cAr", "cAg", "cAb", "cBr", "cBg", "cBb", "cC" };

enum EDifUniform {
	eDifMVPMatrix, eDifModel, eLightDir1, eLightDir2, eLightDir3, eLightDir4, eAmbient, eNumDifUniforms };

const char* g_aszDifUniformNames[] = {
	"MVPMatrix", "Model", "LightDir1", "LightDir2", "LightDir3", "LightDir4", "Ambient" };

/******************************************************************************
 Consts
******************************************************************************/
// Camera constants. Used for making the projection matrix
const float g_fCameraNear = 50.0f;
const float g_fCameraFar  = 5000.0f;

const float g_fDemoFrameRate = 1.0f / 30.0f;

// The camera to use from the pod file
const int g_ui32Camera = 0;

/******************************************************************************
 Content file names
******************************************************************************/

// Source and binary shaders
const char c_szFragShaderSrcFile[]	= "FragShader.fsh";
const char c_szFragShaderBinFile[]	= "FragShader.fsc";
const char c_szSHVertShaderSrcFile[]	= "SHVertShader.vsh";
const char c_szSHVertShaderBinFile[]	= "SHVertShader.vsc";
const char c_szDifVertShaderSrcFile[]	= "DiffuseVertShader.vsh";
const char c_szDifVertShaderBinFile[]	= "DiffuseVertShader.vsc";

// POD scene files
const char c_szSceneFile[]			= "PhantomMask.pod";

// PVR texture files
const char c_szMaskMainTexFile[]	= "MaskMain.pvr";
const char c_szRoomStillTexFile[]	= "RoomStill.pvr";

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES3PhantomMask : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// 3D Model
	CPVRTModelPOD	m_Scene;

	// OpenGL handles for shaders, textures and VBOs
	GLuint m_uiSHVertShader;
	GLuint m_uiDifVertShader;
	GLuint m_uiFragShader;
	GLuint* m_puiVbo;
	GLuint* m_puiIndexVbo;

	// Texture IDs
	GLuint m_ui32TexMask;
	GLuint m_ui32TexBackground;

	// The background
	CPVRTBackground m_Background;

	bool m_bEnableSH;

	// Group shader programs and their uniform locations together
	struct
	{
		GLuint uiId;
		GLuint auiLoc[eNumSHUniforms];
	}
	m_SHShaderProgram;

	struct
	{
		GLuint uiId;
		GLuint auiLoc[eNumDifUniforms];
	}
	m_DiffuseShaderProgram;

	// Variables to handle the animation in a time-based manner
	unsigned long	m_ulTimePrev;
	float			m_fFrame;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	bool LoadTextures(CPVRTString* pErrorStr);
	bool LoadShaders(CPVRTString* pErrorStr);
	bool LoadVbos(CPVRTString* pErrorStr);

	void DrawMesh(int i32NodeIndex);
	void ComputeAndSetSHIrradEnvMapConstants( float* pSHCoeffsRed, float* pSHCoeffsGreen, float* pSHCoeffsBlue );

	OGLES3PhantomMask() : m_puiVbo(0),
						  m_puiIndexVbo(0),
						  m_bEnableSH(true),
						  m_ulTimePrev(0),
						  m_fFrame(0.0f)
	{
	}
};

/*!****************************************************************************
 @Function		LoadTextures
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course.
				For a more detailed explanation, see Texturing and
				IntroducingPVRTools.
******************************************************************************/
bool OGLES3PhantomMask::LoadTextures(CPVRTString* pErrorStr)
{
	if(PVRTTextureLoadFromPVR(c_szRoomStillTexFile, &m_ui32TexBackground) != PVR_SUCCESS)
	{
        *pErrorStr = "ERROR: Failed to load texture for Background.\n";
		return false;
	}

	if(PVRTTextureLoadFromPVR(c_szMaskMainTexFile, &m_ui32TexMask) != PVR_SUCCESS)
	{
        *pErrorStr = "ERROR: Failed to load texture for Mask.\n";
		return false;
	}

	return true;
}

/*!****************************************************************************
 @Function		LoadShaders
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads and compiles the shaders and links the shader programs
				required for this training course
******************************************************************************/
bool OGLES3PhantomMask::LoadShaders(CPVRTString* pErrorStr)
{
	int i;

	// Load the common frag shader
	if (PVRTShaderLoadFromFile(
			c_szFragShaderBinFile, c_szFragShaderSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_uiFragShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	// Load the vertex shader and create the program
	if(PVRTShaderLoadFromFile(
			c_szSHVertShaderBinFile, c_szSHVertShaderSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiSHVertShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	if(PVRTCreateProgram(
			&m_SHShaderProgram.uiId, m_uiSHVertShader, m_uiFragShader, g_aszAttribNames, 3, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	// Store the location of uniforms for later use
	for(i = 0; i < eNumSHUniforms; ++i)
		m_SHShaderProgram.auiLoc[i] = glGetUniformLocation(m_SHShaderProgram.uiId, g_aszSHUniformNames[i]);

	// Set the sampler2D variable to the first texture unit
	glUniform1i(glGetUniformLocation(m_SHShaderProgram.uiId, "sTexture"), 0);

	// Setup shader constants

	// Setup some base constants
	bool light0 = false;
	bool light1 = false;
	bool envlight = true;

	// SH Data Sets
	// Not all are used
	float SHCoeffsLight1Red[9] = {0.83409595f, -1.4446964f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, -0.93254757f, 0.00000000f, -1.6152197f};
	float SHCoeffsLight1Green[9] = {0.83409595f, -1.4446964f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, -0.93254757f, 0.00000000f, -1.6152197f};
	float SHCoeffsLight1Blue[9] = {0.83409595f, -1.4446964f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, -0.93254757f, 0.00000000f, -1.6152197f};

	float SHCoeffsLight2Red[9] = {0.83409595f, -1.2120811f, -0.24892779f, -0.74568230f, -1.3989232f, -0.46699628f, -0.84948879f, 0.28729999f, -0.70663643f};
	float SHCoeffsLight2Green[9] = {0.83409595f, -1.2120811f, -0.24892779f, -0.74568230f, -1.3989232f, -0.46699628f, -0.84948879f, 0.28729999f, -0.70663643f};
	float SHCoeffsLight2Blue[9] = {0.83409595f, -1.2120811f, -0.24892779f, -0.74568230f, -1.3989232f, -0.46699628f, -0.84948879f, 0.28729999f, -0.70663643f};

	float SHCoeffsLightEnvRed[9] = {1.2961891f, -0.42659417f, -0.10065936f, -8.4035477e-005f, -0.00021227333f, 0.10019236f, 0.011847760f, 0.00016783635f, -0.10584830f};
	float SHCoeffsLightEnvGreen[9] = {1.2506844f, -0.12775756f, 0.33325988f, -8.7283181e-005f, -0.00015105936f, -0.025249202f, -0.048718069f, 0.00026852929f, -0.28519103f};
	float SHCoeffsLightEnvBlue[9] = {1.6430428f, 0.098693930f, 0.071262904f, 0.00044371662f, 0.00027166531f, 0.056100018f, -0.23762819f, -0.00015725456f, -0.49318397f};

	float SHCoeffsLightSideRed[9] = {	0.83409595f, 0.00000000f, 0.00000000f, -1.4446964f, 0.00000000f, 0.00000000f, -0.93254757f, 0.00000000f, 1.6152197f};
	float SHCoeffsLightSideGreen[9] = {	0.83409595f, 0.00000000f, 0.00000000f, -1.4446964f, 0.00000000f, 0.00000000f, -0.93254757f, 0.00000000f, 1.6152197f};
	float SHCoeffsLightSideBlue[9] = {	0.83409595f, 0.00000000f, 0.00000000f, -1.4446964f, 0.00000000f, 0.00000000f, -0.93254757f, 0.00000000f, 1.6152197f};

	float SHCoeffsLightEnvGraceCrossRed[9] = {10.153550f, -5.0607910f, -4.3494077f, 3.7619650f, -1.4272760f, 3.3470039f, -2.0500889f, -7.1480651f, 2.7244451f};
	float SHCoeffsLightEnvGraceCrossGreen[9] = {5.6218147f, -4.4867749f, -2.3315217f, 0.71724868f, -0.65607071f, 2.8644383f, -1.2423282f, -2.7321301f, -0.70176142f};
	float SHCoeffsLightEnvGraceCrossBlue[9] = {6.9620109f, -7.7706318f, -3.4473803f, -0.12024292f, -1.5760463f, 6.0764866f, -1.9274533f, -1.7631743f, -3.9185245f};

	float	SHCoeffsLightSummedRed[9];
	float	SHCoeffsLightSummedGreen[9];
	float	SHCoeffsLightSummedBlue[9];

	float LIGHT1WEIGHT,LIGHT2WEIGHT,LIGHTENVWEIGHT,LIGHTSIDEWEIGHT,LIGHTGRACECROSSWEIGHT;

	// SH Weights
	LIGHT1WEIGHT=0.0f;
	LIGHT2WEIGHT=0.0f;
	LIGHTENVWEIGHT=0.0f;
	LIGHTSIDEWEIGHT=0.0f;
	LIGHTGRACECROSSWEIGHT=0.0f;

	// Set weights based on scene info

	if(light0 && light1 && envlight)
	{
		LIGHT1WEIGHT=0.3f;
		LIGHT2WEIGHT=0.3f;
		LIGHTENVWEIGHT=1.0f;
	}
	else if(!light0 && !light1 && envlight)
	{
		LIGHTENVWEIGHT=1.0f;
	}

	// Calculate the final SH coefs using the different lights and weights
	for(i = 0; i < 9; ++i)
	{
		SHCoeffsLightSummedRed[i]   = LIGHT1WEIGHT * SHCoeffsLight1Red[i]
										+ LIGHT2WEIGHT  * SHCoeffsLight2Red[i]
										+ LIGHTENVWEIGHT* SHCoeffsLightEnvRed[i]
										+ LIGHTSIDEWEIGHT * SHCoeffsLightSideRed[i]
										+ LIGHTGRACECROSSWEIGHT * SHCoeffsLightEnvGraceCrossRed[i];

		SHCoeffsLightSummedGreen[i] = LIGHT1WEIGHT * SHCoeffsLight1Green[i]
										+ LIGHT2WEIGHT * SHCoeffsLight2Green[i]
										+ LIGHTENVWEIGHT * SHCoeffsLightEnvGreen[i]
										+ LIGHTSIDEWEIGHT * SHCoeffsLightSideGreen[i]
										+ LIGHTGRACECROSSWEIGHT * SHCoeffsLightEnvGraceCrossGreen[i];

		SHCoeffsLightSummedBlue[i]  = LIGHT1WEIGHT * SHCoeffsLight1Blue[i]
										+ LIGHT2WEIGHT * SHCoeffsLight2Blue[i]
										+ LIGHTENVWEIGHT * SHCoeffsLightEnvBlue[i]
										+ LIGHTSIDEWEIGHT * SHCoeffsLightSideBlue[i]
										+ LIGHTGRACECROSSWEIGHT * SHCoeffsLightEnvGraceCrossBlue[i];
	}

	ComputeAndSetSHIrradEnvMapConstants(SHCoeffsLightSummedRed, SHCoeffsLightSummedGreen, SHCoeffsLightSummedBlue);

	// Setup the shaders we're going to use for Vertex lighting

	// Load the vertex shader and create the program
	if(PVRTShaderLoadFromFile(
			c_szDifVertShaderBinFile, c_szDifVertShaderSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiDifVertShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	if(PVRTCreateProgram(
			&m_DiffuseShaderProgram.uiId, m_uiDifVertShader, m_uiFragShader, g_aszAttribNames, 3, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	// Store the location of uniforms for later use
	for(i = 0; i < eNumDifUniforms; ++i)
		m_DiffuseShaderProgram.auiLoc[i] = glGetUniformLocation(m_DiffuseShaderProgram.uiId, g_aszDifUniformNames[i]);

	// Setup constants

	// Light direction 1 : TOP
	glUniform3fv(m_DiffuseShaderProgram.auiLoc[eLightDir1], 1, PVRTVec3(0.0f,0.5f,0.0f).ptr());

	// Light direction 2 : BOTTOM
	glUniform3fv(m_DiffuseShaderProgram.auiLoc[eLightDir2], 1, PVRTVec3(0.0f,-0.5f,0.0f).ptr());

	// Light direction 3 : LEFT
	glUniform3fv(m_DiffuseShaderProgram.auiLoc[eLightDir3], 1, PVRTVec3(-0.5f,0.0f,0.0f).ptr());

	// Light direction 4 : RIGHT
	glUniform3fv(m_DiffuseShaderProgram.auiLoc[eLightDir4], 1, PVRTVec3(0.5f,0.0f,0.0f).ptr());

	// Ambient Light
	glUniform4fv(m_DiffuseShaderProgram.auiLoc[eAmbient], 1, PVRTVec4(0.05f,0.05f,0.05f,0.05f).ptr());

	// Set the sampler2D variable to the first texture unit
	glUniform1i(glGetUniformLocation(m_DiffuseShaderProgram.uiId, "sTexture"), 0);
	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
bool OGLES3PhantomMask::LoadVbos(CPVRTString* pErrorStr)
{
	if(!m_Scene.pMesh[0].pInterleaved)
	{
		*pErrorStr = "ERROR: IntroducingPOD requires the pod data to be interleaved. Please re-export with the interleaved option enabled.";
		return false;
	}

	if (!m_puiVbo)      m_puiVbo = new GLuint[m_Scene.nNumMesh];
	if (!m_puiIndexVbo) m_puiIndexVbo = new GLuint[m_Scene.nNumMesh];

	/*
		Load vertex data of all meshes in the scene into VBOs

		The meshes have been exported with the "Interleave Vectors" option,
		so all data is interleaved in the buffer at pMesh->pInterleaved.
		Interleaving data improves the memory access pattern and cache efficiency,
		thus it can be read faster by the hardware.
	*/
	glGenBuffers(m_Scene.nNumMesh, m_puiVbo);
	for (unsigned int i = 0; i < m_Scene.nNumMesh; ++i)
	{
		// Load vertex data into buffer object
		SPODMesh& Mesh = m_Scene.pMesh[i];
		unsigned int uiSize = Mesh.nNumVertex * Mesh.sVertex.nStride;
		glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[i]);
		glBufferData(GL_ARRAY_BUFFER, uiSize, Mesh.pInterleaved, GL_STATIC_DRAW);

		// Load index data into buffer object if available
		m_puiIndexVbo[i] = 0;
		if (Mesh.sFaces.pData)
		{
			glGenBuffers(1, &m_puiIndexVbo[i]);
			uiSize = PVRTModelPODCountIndices(Mesh) * sizeof(GLshort);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, Mesh.sFaces.pData, GL_STATIC_DRAW);
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

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
bool OGLES3PhantomMask::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	// Load the scene
	if(m_Scene.ReadFromFile(c_szSceneFile) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Couldn't load the .pod file\n");
		return false;
	}

	// The cameras are stored in the file. We check it contains at least one.
	if(m_Scene.nNumCamera == 0)
	{
		PVRShellSet(prefExitMessage, "ERROR: The scene does not contain a camera. Please add one and re-export.\n");
		return false;
	}

	// Initialise variables used for the animation
	m_ulTimePrev = PVRShellGetTime();

	return true;
}

/*!****************************************************************************
 @Function		QuitApplication
 @Return		bool		true if no error occurred
 @Description	Code in QuitApplication() will be called by PVRShell once per
				run, just before exiting the program.
				If the rendering context is lost, QuitApplication() will
				not be called.
******************************************************************************/
bool OGLES3PhantomMask::QuitApplication()
{
	// Free the memory allocated for the scene
	m_Scene.Destroy();

	delete[] m_puiVbo;
	delete[] m_puiIndexVbo;

    return true;
}

/*!****************************************************************************
 @Function		InitView
 @Return		bool		true if no error occurred
 @Description	Code in InitView() will be called by PVRShell upon
				initialization or after a change in the rendering context.
				Used to initialize variables that are dependant on the rendering
				context (e.g. textures, vertex buffers, etc.)
******************************************************************************/
bool OGLES3PhantomMask::InitView()
{
	CPVRTString ErrorStr;

	// Initialise VBO data
	if(!LoadVbos(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Load textures
	if(!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Load and compile the shaders & link programs
	if(!LoadShaders(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Initialise Print3D
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	// Initialise the background
	if(m_Background.Init(0, bRotate, &ErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Use a nice bright blue as clear colour
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occurred
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES3PhantomMask::ReleaseView()
{
	// Release all Textures
	glDeleteTextures(1, &m_ui32TexMask);
	glDeleteTextures(1, &m_ui32TexBackground);

	// Delete program and shader objects
	glDeleteProgram(m_SHShaderProgram.uiId);
	glDeleteProgram(m_DiffuseShaderProgram.uiId);

	glDeleteShader(m_uiSHVertShader);
	glDeleteShader(m_uiDifVertShader);
	glDeleteShader(m_uiFragShader);

	// Delete buffer objects
	glDeleteBuffers(m_Scene.nNumMesh, m_puiVbo);
	glDeleteBuffers(m_Scene.nNumMesh, m_puiIndexVbo);

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

    m_Background.Destroy();

	return true;
}

/*!****************************************************************************
 @Function		RenderScene
 @Return		bool		true if no error occurred
 @Description	Main rendering loop function of the program. The shell will
				call this function every frame.
				eglSwapBuffers() will be performed by PVRShell automatically.
				PVRShell will also manage important OS events.
				Will also manage relevant OS events. The user has access to
				these events through an abstraction layer provided by PVRShell.
******************************************************************************/
bool OGLES3PhantomMask::RenderScene()
{
	if(PVRShellIsKeyPressed(PVRShellKeyNameACTION1))
		m_bEnableSH = !m_bEnableSH;

	// Clear the colour and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw the background
	m_Background.Draw(m_ui32TexBackground);

	// Enable culling
	glEnable(GL_CULL_FACE);

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

	// Use shader program
	GLuint ProgramID, MVPLoc, ModelLoc;

	if(m_bEnableSH)
	{
		ProgramID = m_SHShaderProgram.uiId;
		MVPLoc	  = m_SHShaderProgram.auiLoc[eSHMVPMatrix];
		ModelLoc  = m_SHShaderProgram.auiLoc[eSHModel];
	}
	else
	{
		ProgramID = m_DiffuseShaderProgram.uiId;
		MVPLoc	  = m_DiffuseShaderProgram.auiLoc[eDifMVPMatrix];
		ModelLoc  = m_DiffuseShaderProgram.auiLoc[eDifModel];
	}

	glUseProgram(ProgramID);

	/*
		Calculates the frame number to animate in a time-based manner.
		Uses the shell function PVRShellGetTime() to get the time in milliseconds.
	*/
	unsigned long ulTime = PVRShellGetTime();

	if(ulTime > m_ulTimePrev)
	{
		unsigned long ulDeltaTime = ulTime - m_ulTimePrev;
		m_fFrame += (float)ulDeltaTime * g_fDemoFrameRate;

		if(m_fFrame > m_Scene.nNumFrame - 1)
			m_fFrame = 0;

		// Sets the scene animation to this frame
		m_Scene.SetFrame(m_fFrame);
	}

	m_ulTimePrev = ulTime;

	/*
		Set up the view and projection matrices from the camera
	*/
	PVRTMat4 mView, mProjection;
	PVRTVec3	vFrom, vTo(0.0f), vUp(0.0f, 1.0f, 0.0f);
	float fFOV;

	// Setup the camera
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	// Camera nodes are after the mesh and light nodes in the array
	int i32CamID = m_Scene.pNode[m_Scene.nNumMeshNode + m_Scene.nNumLight + g_ui32Camera].nIdx;

	// Get the camera position, target and field of view (fov)
	if(m_Scene.pCamera[i32CamID].nIdxTarget != -1) // Does the camera have a target?
		fFOV = m_Scene.GetCameraPos( vFrom, vTo, g_ui32Camera); // vTo is taken from the target node
	else
		fFOV = m_Scene.GetCamera( vFrom, vTo, vUp, g_ui32Camera); // vTo is calculated from the rotation

	fFOV *= bRotate ? (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight) : (float)PVRShellGet(prefHeight)/(float)PVRShellGet(prefWidth);

	// We can build the model view matrix from the camera position, target and an up vector.
	// For this we usePVRTMat4LookAtRH()
	mView = PVRTMat4::LookAtRH(vFrom, vTo, vUp);

	// Calculate the projection matrix
	mProjection = PVRTMat4::PerspectiveFovRH(fFOV, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), g_fCameraNear, g_fCameraFar, PVRTMat4::OGL, bRotate);

	SPODNode& Node = m_Scene.pNode[0];

	// Get the node model matrix
	PVRTMat4 mWorld;
	mWorld = m_Scene.GetWorldMatrix(Node);

	// Set the model inverse transpose matrix
	PVRTMat3 mMat3 = PVRTMat3(mWorld);

	if(m_bEnableSH)
		mMat3 *= PVRTMat3::RotationY(-1.047197f);

	glUniformMatrix3fv(ModelLoc, 1, GL_FALSE, mMat3.f);

	// Pass the model-view-projection matrix (MVP) to the shader to transform the vertices
	PVRTMat4 mModelView, mMVP;
	mModelView = mView * mWorld;
	mMVP = mProjection * mModelView;
	glUniformMatrix4fv(MVPLoc, 1, GL_FALSE, mMVP.f);

	glBindTexture(GL_TEXTURE_2D, m_ui32TexMask);
	DrawMesh(Node.nIdx);

	// Print text on screen

	if(m_bEnableSH)
	{
		// Base
		m_Print3D.DisplayDefaultTitle("PhantomMask", "Spherical Harmonics Lighting", ePVRTPrint3DSDKLogo);
	}
	else
	{
		// Base
		m_Print3D.DisplayDefaultTitle("PhantomMask", "Vertex Lighting", ePVRTPrint3DSDKLogo);
	}

	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			i32NodeIndex		Node index of the mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the material prepared.
******************************************************************************/
void OGLES3PhantomMask::DrawMesh(int i32MeshIndex)
{
	SPODMesh* pMesh = &m_Scene.pMesh[i32MeshIndex];

	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[i32MeshIndex]);
	// bind the index buffer, won't hurt if the handle is 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[i32MeshIndex]);

	// Enable the vertex attribute arrays
	glEnableVertexAttribArray(VERTEX_ARRAY);
	glEnableVertexAttribArray(NORMAL_ARRAY);
	glEnableVertexAttribArray(TEXCOORD_ARRAY);

	// Set the vertex attribute offsets
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, pMesh->sVertex.nStride, pMesh->sVertex.pData);
	glVertexAttribPointer(NORMAL_ARRAY, 3, GL_FLOAT, GL_FALSE, pMesh->sNormals.nStride, pMesh->sNormals.pData);
	glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, pMesh->psUVW[0].nStride, pMesh->psUVW[0].pData);

	/*
		The geometry can be exported in 4 ways:
		- Indexed Triangle list
		- Non-Indexed Triangle list
		- Indexed Triangle strips
		- Non-Indexed Triangle strips
	*/
	if(pMesh->nNumStrips == 0)
	{
		if(m_puiIndexVbo[i32MeshIndex])
		{
			// Indexed Triangle list
			glDrawElements(GL_TRIANGLES, pMesh->nNumFaces*3, GL_UNSIGNED_SHORT, 0);
		}
		else
		{
			// Non-Indexed Triangle list
			glDrawArrays(GL_TRIANGLES, 0, pMesh->nNumFaces*3);
		}
	}
	else
	{
		int offset = 0;

		for(int i = 0; i < (int)pMesh->nNumStrips; ++i)
		{
			if(m_puiIndexVbo[i32MeshIndex])
			{
				// Indexed Triangle strips
				glDrawElements(GL_TRIANGLE_STRIP, pMesh->pnStripLength[i]+2, GL_UNSIGNED_SHORT, (void*)(offset * sizeof(GLushort)));
			}
			else
			{
				// Non-Indexed Triangle strips
				glDrawArrays(GL_TRIANGLE_STRIP, offset, pMesh->pnStripLength[i]+2);
			}
			offset += pMesh->pnStripLength[i]+2;
		}
	}

	// Safely disable the vertex attribute arrays
	glDisableVertexAttribArray(VERTEX_ARRAY);
	glDisableVertexAttribArray(NORMAL_ARRAY);
	glDisableVertexAttribArray(TEXCOORD_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/*******************************************************************************
 @Function ComputeAndSetSHIrradEnvMapConstants
 @Description Function to pre-calculate and setup the Spherical Harmonics Constants
 *******************************************************************************/
void OGLES3PhantomMask::ComputeAndSetSHIrradEnvMapConstants( float* pSHCoeffsRed, float* pSHCoeffsGreen, float* pSHCoeffsBlue )
{
    float* fLight[3] = { pSHCoeffsRed, pSHCoeffsGreen, pSHCoeffsBlue };

    // Lighting environment coefficients
    float vCoefficients[3][4];
	int iChannel;

    // These constants are described in the article by Peter-Pike Sloan titled
    // "Efficient Evaluation of Irradiance Environment Maps" in the book
    // "ShaderX 2 - Shader Programming Tips and Tricks" by Wolfgang F. Engel.
    static const float s_fSqrtPI = 1.772453850905516027298167483341f;
    const float fC0 = 1.0f/(2.0f*s_fSqrtPI);
    const float fC1 = (float)1.7320508075688772935274463415059f / (3.0f * s_fSqrtPI);
    const float fC2 = (float)3.8729833462074168851792653997824f / (8.0f * s_fSqrtPI);
    const float fC3 = (float)2.2360679774997896964091736687313f / (16.0f* s_fSqrtPI);
    const float fC4 = 0.5f*fC2;

    for(iChannel = 0; iChannel < 3; ++iChannel)
    {
        vCoefficients[iChannel][0] = -fC1 * fLight[iChannel][3];
        vCoefficients[iChannel][1] = -fC1 * fLight[iChannel][1];
        vCoefficients[iChannel][2] =  fC1 * fLight[iChannel][2];
        vCoefficients[iChannel][3] =  fC0 * fLight[iChannel][0] - fC3*fLight[iChannel][6];
    }

	glUniform4fv(m_SHShaderProgram.auiLoc[ecAr], 1, vCoefficients[0]);
	glUniform4fv(m_SHShaderProgram.auiLoc[ecAg], 1, vCoefficients[1]);
	glUniform4fv(m_SHShaderProgram.auiLoc[ecAb], 1, vCoefficients[2]);

	for(iChannel = 0; iChannel < 3; ++iChannel)
    {
        vCoefficients[iChannel][0] = fC2 * fLight[iChannel][4];
        vCoefficients[iChannel][1] = -fC2 * fLight[iChannel][5];
        vCoefficients[iChannel][2] = 3.0f * fC3 * fLight[iChannel][6];
        vCoefficients[iChannel][3] = -fC2 * fLight[iChannel][7];
    }

	glUniform4fv(m_SHShaderProgram.auiLoc[ecBr], 1, vCoefficients[0]);
	glUniform4fv(m_SHShaderProgram.auiLoc[ecBg], 1, vCoefficients[1]);
	glUniform4fv(m_SHShaderProgram.auiLoc[ecBb], 1, vCoefficients[2]);

    vCoefficients[0][0] = fC4 * fLight[0][8];
    vCoefficients[0][1] = fC4 * fLight[1][8];
    vCoefficients[0][2] = fC4 * fLight[2][8];

	glUniform3fv(m_SHShaderProgram.auiLoc[ecC], 1, vCoefficients[0]);
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
	return new OGLES3PhantomMask();
}

/******************************************************************************
 End of file (OGLES3PhantomMask.cpp)
******************************************************************************/

