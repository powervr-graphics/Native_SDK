/******************************************************************************

 @File         OGLES3ParticleSystem.cpp

 @Title        ParticleSystem

 @Version

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Particle animation system using Compute Shaders. Requires the PVRShell.

******************************************************************************/

#include "ParticleSystemGPU.h"

#include "PVRShell.h"

/******************************************************************************
 Content file names
******************************************************************************/

// Asset files
const char c_szParticleTexFile[] = "ParticleGradient.pvr";
const char c_szSphereModelFile[] = "sphere.pod";

// Source and binary shaders
const char c_szFragShaderSrcFile[]	= "FragShader.fsh";
const char c_szFragShaderBinFile[]	= "FragShader.fsc";
const char c_szVertShaderSrcFile[]	= "VertShader.vsh";
const char c_szVertShaderBinFile[]	= "VertShader.vsc";
const char c_szFloorFragShaderSrcFile[]	= "FloorFragShader.fsh";
const char c_szFloorFragShaderBinFile[]	= "FloorFragShader.fsc";
const char c_szFloorVertShaderSrcFile[]	= "FloorVertShader.vsh";
const char c_szFloorVertShaderBinFile[]	= "FloorVertShader.vsc";
const char c_szParticleShaderFragSrcFile[]	= "ParticleFragShader.fsh";
const char c_szParticleShaderFragBinFile[]	= "ParticleFragShader.fsc";
const char c_szParticleShaderVertSrcFile[]	= "ParticleVertShader.vsh";
const char c_szParticleShaderVertBinFile[]	= "ParticleVertShader.vsc";

/******************************************************************************
 Defines
******************************************************************************/

// Maximum number of m_Particles
const unsigned int g_ui32MinNoParticles = 1024;
const unsigned int g_ui32MaxNoParticles = 131072 * 64;
const unsigned int g_ui32InitialNoParticles = 32768;
const float g_fCameraNear = 1.0f;
const float g_fCameraFar = 100.0f;

const PVRTVec3 g_caLightPosition(0.0f, 10.0f, 0.0f);

// Index to bind the attributes to vertex shaders
#define POSITION_ARRAY	0
#define LIFESPAN_ARRAY	1

#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY  2

#define BINDING_INDEX_0 0

/******************************************************************************
 Structure definitions
******************************************************************************/

const Sphere _spheresMem[] =
{
	{ PVRTVec3(-20.0f,  4.0f,  0.0f),  4.0f },
	{ PVRTVec3(0.0f,  4.0f, 20.0f),  4.5f },
	{ PVRTVec3(20.0f,  4.0f,  0.0f),  5.0f },
	{ PVRTVec3(0.0f,  4.0f, -18.0f),  4.0f },

	{ PVRTVec3(-10.0f,  2.0f, -11.0f),  2.0f },
	{ PVRTVec3(-11.0f,  1.5f,  10.0f),  1.5f },
	{ PVRTVec3(10.0f,  3.0f,  11.0f),  3.0f },
	{ PVRTVec3(10.0f,  2.0f, -10.0f),  2.0f },

};

const CPVRTArray<Sphere> g_Spheres(_spheresMem, 8);

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES3ParticleSystem : public PVRShell
{
public:
	// Print3D class used to display text
	CPVRTPrint3D m_Print3D;

	// OGLES3 Extensions
	SPVRTContext m_PVRTContext;

	// Shader ids
	GLuint m_uiSimpleVertShader;
	GLuint m_uiSimpleFragShader;
	GLuint m_uiFloorVertShader;
	GLuint m_uiFloorFragShader;
	GLuint m_uiParticleVertShader;
	GLuint m_uiParticleFragShader;

	// Texture names
	GLuint m_ui32ParticleTexName;

	CPVRTModelPOD m_Scene;
	GLuint m_uiVbo;
	GLuint m_uiIbo;

	// View matrix
	PVRTMat4 m_mView, m_mProjection, m_mProjectionShad, m_mViewProjection;
	PVRTMat4 m_mLightView, m_mBiasMatrix;

	ParticleSystemGPU * m_ParticleSystemGPU;

	bool m_bPointSize;

	struct
	{
		GLuint uiId;
		GLint iPositionArrayLoc;
		GLint iLifespanArrayLoc;
		GLint iModelViewProjectionMatrixLoc;
	}
	m_ParticleShaderProgram;

	struct
	{
		GLuint uiId;
		GLint iModelViewMatrixLoc;
		GLint iModelViewITMatrixLoc;
		GLint iModelViewProjectionMatrixLoc;
		GLint iLightPosition;
	}
	m_SimpleShaderProgram;

	struct
	{
		GLuint uiId;
		GLint iModelViewMatrixLoc;
		GLint iModelViewITMatrixLoc;
		GLint iModelViewProjectionMatrixLoc;
		GLint iLightPosition;
	} m_FloorShaderProgram;

	GLuint m_uiFloorVBO;
	GLuint m_uiParticleVBO;
public:
	OGLES3ParticleSystem();

	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	bool CreateBuffers(CPVRTString* const pErrorStr);
	bool LoadShaders(CPVRTString* const pErrorStr);
	bool LoadTextures(CPVRTString* const pErrorStr);

	void HandleInput();

	void RenderFloor();
	void RenderSphere(const PVRTVec3& position, const PVRTMat4& proj, const PVRTMat4& view, float radius);
	void RenderParticles(const PVRTMat4& proj, const PVRTMat4& view);

	void UpdateParticles();

	bool SetCollisionSpheres(const Sphere* pSpheres, unsigned int uiNumSpheres);
};

OGLES3ParticleSystem::OGLES3ParticleSystem(): m_ParticleSystemGPU(0)
{
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
bool OGLES3ParticleSystem::CreateBuffers(CPVRTString* const pErrorStr)
{
	glGenBuffers(1, &m_uiVbo);

	// Load vertex data into buffer object
	SPODMesh& Mesh = m_Scene.pMesh[0];
	glBindBuffer(GL_ARRAY_BUFFER, m_uiVbo);
	glBufferData(GL_ARRAY_BUFFER, Mesh.nNumVertex * Mesh.sVertex.nStride, Mesh.pInterleaved, GL_STATIC_DRAW);

	// Load index data into buffer object
	glGenBuffers(1, &m_uiIbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiIbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, PVRTModelPODCountIndices(Mesh) * sizeof(GLshort), Mesh.sFaces.pData, GL_STATIC_DRAW);


	glGenBuffers(1, &m_uiFloorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_uiFloorVBO);

	PVRTVec2 minCorner(-40, -40);
	PVRTVec2 maxCorner(40,  40);

	//Initialise the vertex buffer data for the floor - 3*Position data, 3* normal data and 2* tex coords data
	const float afVertexBufferData[] = {minCorner.x, 0.0f, minCorner.y,
	                                    0.0f, 1.0f, 0.0f,
	                                    0.0f, 0.0f,
	                                    maxCorner.x, 0.0f, minCorner.y,
	                                    0.0f, 1.0f, 0.0f,
	                                    1.0f, 0.0f,
	                                    minCorner.x, 0.0f, maxCorner.y,
	                                    0.0f, 1.0f, 0.0f,
	                                    0.0f, 1.0f,
	                                    maxCorner.x, 0.0f, maxCorner.y,
	                                    0.0f, 1.0f, 0.0f,
	                                    1.0f, 1.0f
	                                   };


	glBufferData(GL_ARRAY_BUFFER, sizeof(afVertexBufferData), afVertexBufferData, GL_STATIC_DRAW);

	//Setup a VBO to use for the particles. The particle class will assign / populate it.
	glGenBuffers(1, &m_uiParticleVBO);


	return true;
}

/*!****************************************************************************
 @Function		LoadTextures
 @Output		pErrorStr		A CPVRTString describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course
******************************************************************************/
bool OGLES3ParticleSystem::LoadTextures(CPVRTString* const pErrorStr)
{
	//	Load textures.
	if (PVRTTextureLoadFromPVR(c_szParticleTexFile, &m_ui32ParticleTexName) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Cannot load particle gradient texture.\n";
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return true;
}

/*!****************************************************************************
 @Function		LoadShaders
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads and compiles the shaders and links the shader programs
				required for this training course
******************************************************************************/
bool OGLES3ParticleSystem::LoadShaders(CPVRTString* pErrorStr)
{
	/*
	 *  Simple Shader
	 */
	const char* aszSimpleAttribs[] = { "inVertex", "inNormal" };
	const unsigned int numSimpleAttribs = sizeof(aszSimpleAttribs) / sizeof(aszSimpleAttribs[0]);

	if (PVRTShaderLoadFromFile(c_szVertShaderBinFile, c_szVertShaderSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiSimpleVertShader, pErrorStr, &m_PVRTContext) != PVR_SUCCESS)
	{ *pErrorStr = "Vertex shader : " + (*pErrorStr); return false;}


	if (PVRTShaderLoadFromFile(c_szFragShaderBinFile, c_szFragShaderSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_uiSimpleFragShader, pErrorStr, &m_PVRTContext) != PVR_SUCCESS)
	{ *pErrorStr = "Fragment shader : " + (*pErrorStr); return false;}

	if (PVRTCreateProgram(&m_SimpleShaderProgram.uiId, m_uiSimpleVertShader, m_uiSimpleFragShader, aszSimpleAttribs, numSimpleAttribs, pErrorStr) != PVR_SUCCESS)
	 { *pErrorStr = "Program linking : " + (*pErrorStr); return false;}

	m_SimpleShaderProgram.iModelViewMatrixLoc = glGetUniformLocation(m_SimpleShaderProgram.uiId, "uModelViewMatrix");
	m_SimpleShaderProgram.iModelViewITMatrixLoc = glGetUniformLocation(m_SimpleShaderProgram.uiId, "uModelViewITMatrix");
	m_SimpleShaderProgram.iModelViewProjectionMatrixLoc	= glGetUniformLocation(m_SimpleShaderProgram.uiId, "uModelViewProjectionMatrix");
	m_SimpleShaderProgram.iLightPosition = glGetUniformLocation(m_SimpleShaderProgram.uiId, "uLightPosition");

	/*
		Floor Shader
	*/
	const char* aszFloorAttribs[] = { "inVertex", "inNormal", "inTexCoords" };
	const unsigned int numFloorAttribs = sizeof(aszFloorAttribs) / sizeof(aszFloorAttribs[0]);

	if (PVRTShaderLoadFromFile(c_szFloorVertShaderBinFile, c_szFloorVertShaderSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiFloorVertShader, pErrorStr, &m_PVRTContext) != PVR_SUCCESS)
	{ (*pErrorStr) = "Floor Vertex shader : " + (*pErrorStr); return false;}

	if (PVRTShaderLoadFromFile(c_szFloorFragShaderBinFile, c_szFloorFragShaderSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_uiFloorFragShader, pErrorStr, &m_PVRTContext) != PVR_SUCCESS)
	{ (*pErrorStr) = "Floor Fragment shader : " + (*pErrorStr); return false;}

	if (PVRTCreateProgram(&m_FloorShaderProgram.uiId, m_uiFloorVertShader, m_uiFloorFragShader, aszFloorAttribs, numFloorAttribs, pErrorStr) != PVR_SUCCESS)
	{ (*pErrorStr) = "Floor Program linking : " + (*pErrorStr); return false;}

	m_FloorShaderProgram.iModelViewMatrixLoc = glGetUniformLocation(m_FloorShaderProgram.uiId, "uModelViewMatrix");
	m_FloorShaderProgram.iModelViewITMatrixLoc = glGetUniformLocation(m_FloorShaderProgram.uiId, "uModelViewITMatrix");
	m_FloorShaderProgram.iModelViewProjectionMatrixLoc	= glGetUniformLocation(m_FloorShaderProgram.uiId, "uModelViewProjectionMatrix");
	m_FloorShaderProgram.iLightPosition = glGetUniformLocation(m_FloorShaderProgram.uiId, "uLightPosition");

	/*
	 *  Particle Shader
	 */
	const char* aszParticleAttribs[] = { "inPosition", "inLifespan" };
	const unsigned int numParticleAttribs = sizeof(aszParticleAttribs) / sizeof(aszParticleAttribs[0]);

	if (PVRTShaderLoadFromFile(c_szParticleShaderVertBinFile, c_szParticleShaderVertSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiParticleVertShader, pErrorStr, &m_PVRTContext) != PVR_SUCCESS)
	{ *pErrorStr = "Particle Vertex shader : " + (*pErrorStr); return false;}

	if (PVRTShaderLoadFromFile(c_szParticleShaderFragBinFile, c_szParticleShaderFragSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_uiParticleFragShader, pErrorStr, &m_PVRTContext) != PVR_SUCCESS)
	{ *pErrorStr = "Particle Fragment shader : " + (*pErrorStr); return false;}

	if (PVRTCreateProgram(&m_ParticleShaderProgram.uiId, m_uiParticleVertShader, m_uiParticleFragShader, aszParticleAttribs, numParticleAttribs, pErrorStr) != PVR_SUCCESS)
	{ *pErrorStr = "Particle Program linking : " + (*pErrorStr); return false;}

	m_ParticleShaderProgram.iModelViewProjectionMatrixLoc = glGetUniformLocation(m_ParticleShaderProgram.uiId, "uModelViewProjectionMatrix");
	glUniform1i(glGetUniformLocation(m_ParticleShaderProgram.uiId, "sTexture"), 0);

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
bool OGLES3ParticleSystem::InitApplication()
{
	PVRShellSet(prefSwapInterval, 0);
	PVRShellSet(prefApiMajorVersion, 3);
	PVRShellSet(prefApiMinorVersion, 1);

	m_bPointSize = true;

	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	// Load the scene
	if (m_Scene.ReadFromFile(c_szSphereModelFile) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Couldn't load the sphere.pod file\n");
		return false;
	}

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
bool OGLES3ParticleSystem::QuitApplication()
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
bool OGLES3ParticleSystem::InitView()
{

	CPVRTString ErrorStr;
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);


	// Initialize Print3D textures
	if (m_Print3D.SetTextures(&m_PVRTContext, PVRShellGet(prefWidth), PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	//	Create the Buffers
	if (!CreateBuffers(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	//	Load and compile the shaders & link programs
	if (!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	//	Load and compile the shaders & link programs
	if (!LoadShaders(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Create view matrices
	PVRTVec3 vFrom = PVRTVec3(0.0f, 80.0f, 0.0f);
	m_mLightView   = PVRTMat4::LookAtRH(vFrom, PVRTVec3(0.0f, 0.0f, 0.0f), PVRTVec3(0.0f, 0.0f, -1.0f));


	// Creates the projection matrix.
	m_mProjection     = PVRTMat4::PerspectiveFovRH(PVRT_PI / 3.0f, (float)PVRShellGet(prefWidth) / (float)PVRShellGet(prefHeight), g_fCameraNear, g_fCameraFar, PVRTMat4::OGL, bRotate);
	m_mProjectionShad = PVRTMat4::PerspectiveFovRH(PVRT_PI / 3.0f, 1.0f, g_fCameraNear, g_fCameraFar, PVRTMat4::OGL, bRotate);

	// Create a bias matrix
	m_mBiasMatrix = PVRTMat4(0.5f, 0.0f, 0.0f, 0.0f,
	                         0.0f, 0.5f, 0.0f, 0.0f,
	                         0.0f, 0.0f, 0.5f, 0.0f,
	                         0.5f, 0.5f, 0.5f, 1.0f);

	m_ParticleSystemGPU = new ParticleSystemGPU(m_PVRTContext);

	if (!m_ParticleSystemGPU->Init(ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	m_ParticleSystemGPU->SetParticleVbo(m_uiParticleVBO);
	m_ParticleSystemGPU->SetNumberOfParticles(g_ui32InitialNoParticles);
	m_ParticleSystemGPU->SetCollisionSpheres(g_Spheres);
	m_ParticleSystemGPU->SetGravity(PVRTVec3(0.f, -9.81f, 0.f));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES3ParticleSystem::ReleaseView()
{
	// Release textures
	glDeleteTextures(1, &m_ui32ParticleTexName);

	// Release shaders
	glDeleteShader(m_uiParticleFragShader);
	glDeleteShader(m_uiParticleVertShader);
	glDeleteShader(m_uiSimpleFragShader);
	glDeleteShader(m_uiSimpleVertShader);

	glDeleteProgram(m_ParticleShaderProgram.uiId);
	glDeleteProgram(m_SimpleShaderProgram.uiId);

	glDeleteBuffers(1, &m_uiVbo);
	glDeleteBuffers(1, &m_uiIbo);
	glDeleteBuffers(1, &m_uiParticleVBO);
	
	delete m_ParticleSystemGPU;

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
bool OGLES3ParticleSystem::RenderScene()
{
	const char * err=0;
	HandleInput();

	UpdateParticles();

	float time_delta = PVRShellGetTime() / 10000.0f;
	PVRTVec3 vFrom = PVRTVec3(sinf(time_delta) * 50.0f, 30.0f, cosf(time_delta) * 50.0f);
	m_mView = PVRTMat4::LookAtRH(vFrom, PVRTVec3(0.0f, 5.0f, 0.0f), PVRTVec3(0.0f, 1.0f, 0.0f));
	m_mViewProjection = m_mProjection * m_mView;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear colour and depth buffers
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// Enables depth testing
	glEnable(GL_DEPTH_TEST);
	
	// Render floor
	RenderFloor();

	for (unsigned int i = 0; i < g_Spheres.GetSize(); i++)
	{
		RenderSphere(g_Spheres[i].vPosition, m_mProjection, m_mView, g_Spheres[i].fRadius);
	}

	// Render particles
	RenderParticles(m_mProjection, m_mView);

	// Display info text.

	char lower_buffer[64];
	unsigned int numParticles = m_ParticleSystemGPU->GetNumberOfParticles();
	sprintf(lower_buffer, "No. of Particles: %d", numParticles);

	m_Print3D.DisplayDefaultTitle("OpenGL ES 3.1 Compute Particle System", NULL, ePVRTPrint3DSDKLogo);
	m_Print3D.Print3D(2.0f, 90.0f, 1.0f, 0xFFFFFFFF, "No. of Particles: %d", numParticles);
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		RenderSphere
 @Description	Renders a sphere at the specified position.
******************************************************************************/
void OGLES3ParticleSystem::RenderSphere(const PVRTVec3& position, const PVRTMat4& proj, const PVRTMat4& view, float radius)
{
	glUseProgram(m_SimpleShaderProgram.uiId);

	PVRTMat4 mModel = PVRTMat4::Translation(position) * PVRTMat4::Scale(radius, radius, radius);
	PVRTMat4 mModelView = view * mModel;
	PVRTMat4 mModelViewProj = proj * mModelView;
	PVRTMat3 mModelViewIT(mModelView.inverse().transpose());
	glUniformMatrix4fv(m_SimpleShaderProgram.iModelViewProjectionMatrixLoc, 1, GL_FALSE, mModelViewProj.f);
	glUniformMatrix4fv(m_SimpleShaderProgram.iModelViewMatrixLoc, 1, GL_FALSE, mModelView.f);
	glUniformMatrix3fv(m_SimpleShaderProgram.iModelViewITMatrixLoc, 1, GL_FALSE, mModelViewIT.f);

	PVRTVec3 vLightPosition = view * PVRTVec4(g_caLightPosition, 1.0f);
	glUniform3fv(m_SimpleShaderProgram.iLightPosition, 1, &vLightPosition.x);

	// Enable vertex arributes
	glEnableVertexAttribArray(VERTEX_ARRAY);
	glEnableVertexAttribArray(NORMAL_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, m_uiVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiIbo);

	SPODMesh* pMesh = &m_Scene.pMesh[0];
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, pMesh->sVertex.nStride, pMesh->sVertex.pData);
	glVertexAttribPointer(NORMAL_ARRAY, 3, GL_FLOAT, GL_FALSE, pMesh->sNormals.nStride, pMesh->sNormals.pData);

	// Indexed Triangle list
	glDrawElements(GL_TRIANGLES, pMesh->nNumFaces * 3, GL_UNSIGNED_SHORT, 0);

	// Safely disable the vertex attribute arrays
	glDisableVertexAttribArray(VERTEX_ARRAY);
	glDisableVertexAttribArray(NORMAL_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


/*!****************************************************************************
 @Function		RenderFloor
 @Description	Renders the floor as a quad.
******************************************************************************/
void OGLES3ParticleSystem::RenderFloor()
{
	PVRTMat4 mTextureMatrix, mViewInv;
	mViewInv = m_mView.inverse();;

	// We need to calculate the texture projection matrix. This matrix takes the pixels from world space to previously rendered light projection space
	// where we can look up values from our saved depth buffer. The matrix is constructed from the light view and projection matrices as used for the previous render and
	// then multiplied by the inverse of the current view matrix.
	mTextureMatrix = m_mBiasMatrix * m_mProjectionShad *  m_mLightView * mViewInv;

	glUseProgram(m_FloorShaderProgram.uiId);

	PVRTMat3 mViewIT(m_mView.inverse().transpose());
	glUniformMatrix4fv(m_FloorShaderProgram.iModelViewProjectionMatrixLoc, 1, GL_FALSE, m_mViewProjection.f);
	glUniformMatrix4fv(m_FloorShaderProgram.iModelViewMatrixLoc, 1, GL_FALSE, m_mView.f);
	glUniformMatrix3fv(m_FloorShaderProgram.iModelViewITMatrixLoc, 1, GL_FALSE, mViewIT.f);

	PVRTVec3 vLightPosition = m_mView * PVRTVec4(g_caLightPosition, 1.0f);
	glUniform3fv(m_FloorShaderProgram.iLightPosition, 1, &vLightPosition.x);

	glBindBuffer(GL_ARRAY_BUFFER, m_uiFloorVBO);

	// Enable vertex arributes
	glEnableVertexAttribArray(VERTEX_ARRAY);
	glEnableVertexAttribArray(NORMAL_ARRAY);
	glEnableVertexAttribArray(TEXCOORD_ARRAY);

	glVertexAttribPointer(VERTEX_ARRAY,   3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glVertexAttribPointer(NORMAL_ARRAY,   3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(TEXCOORD_ARRAY, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

	// Draw the quad
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// Safely disable the vertex attribute arrays
	glDisableVertexAttribArray(VERTEX_ARRAY);
	glDisableVertexAttribArray(NORMAL_ARRAY);
	glDisableVertexAttribArray(TEXCOORD_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

/*!****************************************************************************
 @Function		RenderParticles
 @Description	Renders the particles.
******************************************************************************/
void OGLES3ParticleSystem::RenderParticles(const PVRTMat4& proj, const PVRTMat4& view)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_DEPTH_TEST);
	glUseProgram(m_ParticleShaderProgram.uiId);

	glBindBuffer(GL_ARRAY_BUFFER, m_uiParticleVBO);

	glVertexAttribPointer(POSITION_ARRAY, 3, GL_FLOAT, false, sizeof(Particle), 0);
	glVertexAttribPointer(LIFESPAN_ARRAY, 1, GL_FLOAT, false, sizeof(Particle), (void*)(sizeof(float) * 7));

	glEnableVertexAttribArray(POSITION_ARRAY);
	glEnableVertexAttribArray(LIFESPAN_ARRAY);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_ui32ParticleTexName);

	PVRTMat4 mModelViewProj = proj * view;
	glUniformMatrix4fv(m_ParticleShaderProgram.iModelViewProjectionMatrixLoc, 1, GL_FALSE, mModelViewProj.f);
	glDrawArrays(GL_POINTS, 0, m_ParticleSystemGPU->GetNumberOfParticles());

	// Safely disable the vertex attribute arrays
	glDisableVertexAttribArray(POSITION_ARRAY);
	glDisableVertexAttribArray(LIFESPAN_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	glDisable(GL_BLEND);

}


/*!****************************************************************************
 @Function		UpdateParticles
 @Description	Updates particle positions and attributes, e.g. lifespan.
******************************************************************************/
void OGLES3ParticleSystem::UpdateParticles()
{
	static long lastTime = PVRShellGetTime();
	long nowTime = PVRShellGetTime();
	long elapsedTime = nowTime - lastTime;
	float step = (float)elapsedTime;
	lastTime = nowTime;

	static float rot_angle = 0.0f;
	rot_angle += step / 500.0f;
	float el_angle = (sinf(rot_angle / 4.0f) + 1.0f) * 0.2f + 0.2f;

	PVRTMat4 rot = PVRTMat4::RotationY(rot_angle);
	PVRTMat4 skew = PVRTMat4::RotationZ(el_angle);

	Emitter sEmitter;
	sEmitter.mTransformation = rot * skew;
	sEmitter.fHeight = 1.3f;
	sEmitter.fRadius = 1.0f;


	m_ParticleSystemGPU->SetEmitter(sEmitter);
	m_ParticleSystemGPU->Update(step);
}


/*!****************************************************************************
 @Function		HandleInput
 @Description	Handles user input and updates live variables accordingly.
******************************************************************************/
void OGLES3ParticleSystem::HandleInput()
{
	// Keyboard input (cursor to change Reflection Flag)
	if (PVRShellIsKeyPressed(PVRShellKeyNameLEFT))
	{
		unsigned int numParticles = m_ParticleSystemGPU->GetNumberOfParticles();
		if (numParticles / 2 >= g_ui32MinNoParticles)
			if (!m_ParticleSystemGPU->SetNumberOfParticles(numParticles / 2))
			{ PVRShellOutputDebug("Error: Failed decreasing number of particles to %d\n", numParticles / 2); }
	}

	if (PVRShellIsKeyPressed(PVRShellKeyNameRIGHT))
	{
		unsigned int numParticles = m_ParticleSystemGPU->GetNumberOfParticles();
		if (numParticles * 2 <= g_ui32MaxNoParticles)
			if (!m_ParticleSystemGPU->SetNumberOfParticles(numParticles * 2))
			{ PVRShellOutputDebug("Error: Failed increasing number of particles to %d\n", numParticles / 2); }
	}
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
	return new OGLES3ParticleSystem();
}

/******************************************************************************
 End of file (OGLES3ParticleSystem.cpp)
******************************************************************************/
