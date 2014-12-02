/******************************************************************************

 @File         OGLES2Glass.cpp

 @Title        Glass

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Demonstrates dynamic reflection and refraction by rendering two
               halves of the scene to a single rectangular texture.

******************************************************************************/

#include "PVRShell.h"
#include "OGLES2Tools.h"
#include <limits.h>

/******************************************************************************
 Constants
******************************************************************************/

const GLsizei g_ParaboloidTexSize = 512;

// Camera constants used to generate the projection matrix
const float g_fCamNear	= 1.0f;
const float g_fCamFar	= 500.0f;
const float g_fCamFOV	= PVRT_PI * 0.5f;

// Vertex attributes
enum EVertexAttrib {
	VERTEX_ARRAY, NORMAL_ARRAY, TEXCOORD_ARRAY, eNumAttribs
};

const char* g_aszAttribNames[] = {
	"inVertex", "inNormal", "inTexCoords"
};

// Shader uniforms
enum EUniform {
	eMVPMatrix, eMVMatrix, eMMatrix, eInvVPMatrix, eLightDir, eEyePos, eNumUniforms
};

const char* g_aszUniformNames[] = {
	"MVPMatrix", "MVMatrix", "MMatrix", "InvVPMatrix", "LightDir", "EyePos"
};

// Effects
const int g_iNumShaderDefines = 3;
const int g_iNumEffects = 5;

const char* g_aaszEffectDefines[g_iNumEffects][g_iNumShaderDefines] = {
	{"REFLECT", "REFRACT", "CHROMATIC"}, {"REFLECT", "REFRACT"}, {"REFLECT"}, {"REFRACT", "CHROMATIC"}, {"REFRACT"}
};

const int g_aiNumEffectDefines[g_iNumEffects] = {
	3, 2, 1, 2, 1
};

const char* g_aszEffectNames[g_iNumEffects] = {
	 "Reflection + Chromatic Dispersion", "Reflection + Refraction", "Reflection", "Chromatic Dispersion", "Refraction"
};

/******************************************************************************
 Content file names
******************************************************************************/

// Source and binary shaders
const char c_szFragShaderSrcFile[]	= "DefaultFragShader.fsh";
const char c_szFragShaderBinFile[]	= "DefaultFragShader.fsc";
const char c_szVertShaderSrcFile[]	= "DefaultVertShader.vsh";
const char c_szVertShaderBinFile[]	= "DefaultVertShader.vsc";

const char c_szReflectionFragShaderSrcFile[]	= "EffectFragShader.fsh";
const char c_szReflectionFragShaderBinFile[]	= "EffectFragShader.fsc";
const char c_szReflectionVertShaderSrcFile[]	= "EffectVertShader.vsh";
const char c_szReflectionVertShaderBinFile[]	= "EffectVertShader.vsc";

const char c_szSkyboxFragShaderSrcFile[]	= "SkyboxFragShader.fsh";
const char c_szSkyboxFragShaderBinFile[]	= "SkyboxFragShader.fsc";
const char c_szSkyboxVertShaderSrcFile[]	= "SkyboxVertShader.vsh";
const char c_szSkyboxVertShaderBinFile[]	= "SkyboxVertShader.vsc";

const char c_szParaboloidVertShaderSrcFile[]	= "ParaboloidVertShader.vsh";
const char c_szParaboloidVertShaderBinFile[]	= "ParaboloidVertShader.vsc";

// PVR texture files
const char c_szBalloonTexFile[]	= "BalloonTex.pvr";
const char c_szCubeTexFile[]	= "SkyboxTex.pvr";

// POD scene files
const char c_szBallFile[]		= "Ball.pod";
const char c_szBalloonFile[]	= "Balloon.pod";

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES2Glass : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D m_Print3D;

	// 3D Models
	CPVRTModelPOD m_Ball;
	CPVRTModelPOD m_Balloon;

	// Projection, view and model matrices
	PVRTMat4 m_mProjection, m_mView, m_mModels[2];

	// OpenGL handles for shaders, textures and VBOs
	GLuint m_uiDefaultVertShader;
	GLuint m_uiDefaultFragShader;
	GLuint m_uiSkyboxVertShader;
	GLuint m_uiSkyboxFragShader;
	GLuint m_uiParaboloidVertShader;
	GLuint m_auiEffectVertShaders[g_iNumEffects];
	GLuint m_auiEffectFragShaders[g_iNumEffects];

	GLuint m_uiCubeTex;
	GLuint m_uiBalloonTex;

	GLuint*	m_puiVbo;
	GLuint*	m_puiIndexVbo;
	GLuint*	m_puiBalloonVbo;
	GLuint*	m_puiBalloonIndexVbo;
	GLuint	m_uiSquareVbo;

	GLint m_iOriginalFramebuffer;

	GLuint m_uiParaboloidFramebuffer;
	GLuint m_uiParaboloidTexture;
	GLuint m_uiParaboloidDepthBuffer;

	// Group shader programs and their uniform locations together
	struct Program
	{
		GLuint uiId;
		GLuint auiLoc[eNumUniforms];
	}
	m_DefaultProgram, m_SkyboxProgram, m_ParaboloidProgram, m_aEffectPrograms[g_iNumEffects];

	// Current time in milliseconds
	unsigned long m_ulTime;

	// Rotation angle for the model
	float m_afAngles[2];

	int m_iEffect;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

private:
	bool LoadTextures(CPVRTString* pErrorStr);
	bool LoadShaders(CPVRTString* pErrorStr);
	void LoadVbos();
	bool LoadParaboloids(CPVRTString* pErrorStr);

	void UpdateScene();

	void DrawMesh(int i32NodeIndex, CPVRTModelPOD* pod, GLuint** ppuiVbos, GLuint** ppuiIbos, int i32NumAttributes);

	void DrawBalloons(Program* psProgram, PVRTMat4 mProjection, PVRTMat4 mView, PVRTMat4* pmModels, int iNum);
	void DrawSkybox();
	void DrawBall();
	void DrawIntoParaboloids(PVRTVec3 position);
};

/*!****************************************************************************
 @Function		LoadTextures
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course
******************************************************************************/
bool OGLES2Glass::LoadTextures(CPVRTString* const pErrorStr)
{
	if(PVRTTextureLoadFromPVR(c_szCubeTexFile, &m_uiCubeTex) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Failed to load texture.";
		return false;
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if(PVRTTextureLoadFromPVR(c_szBalloonTexFile, &m_uiBalloonTex) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Failed to load texture.";
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
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
bool OGLES2Glass::LoadShaders(CPVRTString* pErrorStr)
{
	/*
		Load and compile the shaders from files.
		Binary shaders are tried first, source shaders
		are used as fallback.
	*/
	if (PVRTShaderLoadFromFile(
			c_szVertShaderBinFile, c_szVertShaderSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiDefaultVertShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	if (PVRTShaderLoadFromFile(
			c_szFragShaderBinFile, c_szFragShaderSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_uiDefaultFragShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	/*
		Set up and link the shader program
	*/
	if (PVRTCreateProgram(&m_DefaultProgram.uiId, m_uiDefaultVertShader, m_uiDefaultFragShader, g_aszAttribNames, 3, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	// Store the location of uniforms for later use
	for (int i = 0; i < eNumUniforms; ++i)
	{
		m_DefaultProgram.auiLoc[i] = glGetUniformLocation(m_DefaultProgram.uiId, g_aszUniformNames[i]);
	}



	/*
		Load and compile the shaders from files.
		Binary shaders are tried first, source shaders
		are used as fallback.
	*/
	if (PVRTShaderLoadFromFile(
			c_szSkyboxVertShaderBinFile, c_szSkyboxVertShaderSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiSkyboxVertShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	if (PVRTShaderLoadFromFile(
			c_szSkyboxFragShaderBinFile, c_szSkyboxFragShaderSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_uiSkyboxFragShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	/*
		Set up and link the shader program
	*/
	if (PVRTCreateProgram(&m_SkyboxProgram.uiId, m_uiSkyboxVertShader, m_uiSkyboxFragShader, g_aszAttribNames, 1, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	// Store the location of uniforms for later use
	for (int i = 0; i < eNumUniforms; ++i)
	{
		m_SkyboxProgram.auiLoc[i] = glGetUniformLocation(m_SkyboxProgram.uiId, g_aszUniformNames[i]);
	}



	/*
		Load and compile the shaders from files.
		Binary shaders are tried first, source shaders
		are used as fallback.
	*/
	if (PVRTShaderLoadFromFile(
			c_szParaboloidVertShaderBinFile, c_szParaboloidVertShaderSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiParaboloidVertShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	/*
		Set up and link the shader program
	*/
	if (PVRTCreateProgram(&m_ParaboloidProgram.uiId, m_uiParaboloidVertShader, m_uiDefaultFragShader, g_aszAttribNames, 3, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	// Store the location of uniforms for later use
	for (int i = 0; i < eNumUniforms; ++i)
	{
		m_ParaboloidProgram.auiLoc[i] = glGetUniformLocation(m_ParaboloidProgram.uiId, g_aszUniformNames[i]);
	}



	for (int i = 0; i < g_iNumEffects; ++i) {
		/*
			Load and compile the shaders from files.
			Binary shaders are tried first, source shaders
			are used as fallback.
		*/
		if (PVRTShaderLoadFromFile(
				c_szReflectionVertShaderBinFile, c_szReflectionVertShaderSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_auiEffectVertShaders[i], pErrorStr, 0, g_aaszEffectDefines[i], g_aiNumEffectDefines[i]) != PVR_SUCCESS)
		{
			return false;
		}

		if (PVRTShaderLoadFromFile(
				c_szReflectionFragShaderBinFile, c_szReflectionFragShaderSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_auiEffectFragShaders[i], pErrorStr, 0, g_aaszEffectDefines[i], g_aiNumEffectDefines[i]) != PVR_SUCCESS)
		{
			return false;
		}

		/*
			Set up and link the shader program
		*/
		if (PVRTCreateProgram(&m_aEffectPrograms[i].uiId, m_auiEffectVertShaders[i], m_auiEffectFragShaders[i], g_aszAttribNames, 2, pErrorStr) != PVR_SUCCESS)
		{
			PVRShellSet(prefExitMessage, pErrorStr->c_str());
			return false;
		}

		// Store the location of uniforms for later use
		for (int j = 0; j < eNumUniforms; ++j)
		{
			m_aEffectPrograms[i].auiLoc[j] = glGetUniformLocation(m_aEffectPrograms[i].uiId, g_aszUniformNames[j]);
		}
	}

	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLES2Glass::LoadVbos()
{
	if (!m_puiVbo)      m_puiVbo = new GLuint[m_Ball.nNumMesh];
	if (!m_puiIndexVbo) m_puiIndexVbo = new GLuint[m_Ball.nNumMesh];

	/*
		Load vertex data of all meshes in the scene into VBOs

		The meshes have been exported with the "Interleave Vectors" option,
		so all data is interleaved in the buffer at pMesh->pInterleaved.
		Interleaving data improves the memory access pattern and cache efficiency,
		thus it can be read faster by the hardware.
	*/
	glGenBuffers(m_Ball.nNumMesh, m_puiVbo);
	for (unsigned int i = 0; i < m_Ball.nNumMesh; ++i)
	{
		// Load vertex data into buffer object
		SPODMesh& Mesh = m_Ball.pMesh[i];
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

	if (!m_puiBalloonVbo)      m_puiBalloonVbo = new GLuint[m_Balloon.nNumMesh];
	if (!m_puiBalloonIndexVbo) m_puiBalloonIndexVbo = new GLuint[m_Balloon.nNumMesh];

	/*
		Load vertex data of all meshes in the scene into VBOs

		The meshes have been exported with the "Interleave Vectors" option,
		so all data is interleaved in the buffer at pMesh->pInterleaved.
		Interleaving data improves the memory access pattern and cache efficiency,
		thus it can be read faster by the hardware.
	*/
	glGenBuffers(m_Balloon.nNumMesh, m_puiBalloonVbo);
	for (unsigned int i = 0; i < m_Balloon.nNumMesh; ++i)
	{
		// Load vertex data into buffer object
		SPODMesh& Mesh = m_Balloon.pMesh[i];
		unsigned int uiSize = Mesh.nNumVertex * Mesh.sVertex.nStride;
		glBindBuffer(GL_ARRAY_BUFFER, m_puiBalloonVbo[i]);
		glBufferData(GL_ARRAY_BUFFER, uiSize, Mesh.pInterleaved, GL_STATIC_DRAW);

		// Load index data into buffer object if available
		m_puiBalloonIndexVbo[i] = 0;
		if (Mesh.sFaces.pData)
		{
			glGenBuffers(1, &m_puiBalloonIndexVbo[i]);
			uiSize = PVRTModelPODCountIndices(Mesh) * sizeof(GLshort);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiBalloonIndexVbo[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, Mesh.sFaces.pData, GL_STATIC_DRAW);
		}
	}

	static GLfloat fQuadVertices[] =
	{
		-1, 1, 0.9999f,
		-1, -1, 0.9999f,
		1, 1, 0.9999f,
		1, 1, 0.9999f,
		-1, -1, 0.9999f,
		1, -1, 0.9999f
	};

	glGenBuffers(1, &m_uiSquareVbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_uiSquareVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 6, fQuadVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/*!****************************************************************************
 @Function		LoadParaboloids
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Creates the required framebuffers and textures to render into.
******************************************************************************/
bool OGLES2Glass::LoadParaboloids(CPVRTString* pErrorStr)
{
	// Generate a framebuffer, a 2D texture and a renderbuffer
	glGenFramebuffers(1, &m_uiParaboloidFramebuffer);
	glGenTextures(1, &m_uiParaboloidTexture);
	glGenRenderbuffers(1, &m_uiParaboloidDepthBuffer);
	
	// Bind and set up the 2D texture
	glBindTexture(GL_TEXTURE_2D, m_uiParaboloidTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_ParaboloidTexSize * 2, g_ParaboloidTexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Bind and set up the renderbuffer as a depth buffer
	glBindRenderbuffer(GL_RENDERBUFFER, m_uiParaboloidDepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, g_ParaboloidTexSize * 2, g_ParaboloidTexSize);

	// Bind the 2D texture and the renderbuffer to the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_uiParaboloidFramebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uiParaboloidTexture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_uiParaboloidDepthBuffer);

	// Check for errors
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to create framebuffer.\n");
		return false;
	}

	// Unbind the current framebuffer and texture
	glBindFramebuffer(GL_FRAMEBUFFER, m_iOriginalFramebuffer);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

/*!****************************************************************************
 @Function		InitApplication
 @Return		bool		true if no error occurred
 @Description	Code in InitApplication() will be called by PVRShell once per
				run, before the rendering context is created.
				Used to initialize variables that are not dependant on it
				(e.g. external modules, loading meshes, etc.)
				If the rendering context is lost, InitApplication() will
				not be called again.
******************************************************************************/
bool OGLES2Glass::InitApplication()
{
	m_puiVbo = 0;
	m_puiIndexVbo = 0;
	m_puiBalloonVbo = 0;
	m_puiBalloonIndexVbo = 0;

	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	// Load the mask
	if (m_Ball.ReadFromFile(c_szBallFile) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Couldn't load the .pod file\n");
		return false;
	}

	// Load the balloon
	if (m_Balloon.ReadFromFile(c_szBalloonFile) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Couldn't load the .pod file\n");
		return false;
	}

	m_ulTime = ULONG_MAX;

	m_afAngles[0] = 0.0f;
	m_afAngles[1] = 0.0f;

	m_iEffect = 0;

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
bool OGLES2Glass::QuitApplication()
{
	// Free the memory allocated for the scene
	m_Ball.Destroy();
	m_Balloon.Destroy();

	delete [] m_puiVbo;
	delete [] m_puiIndexVbo;
	delete [] m_puiBalloonVbo;
	delete [] m_puiBalloonIndexVbo;

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
bool OGLES2Glass::InitView()
{
	CPVRTString ErrorStr;
	// Store the original FBO handle
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_iOriginalFramebuffer);
	/*
		Initialize VBO data
	*/
	LoadVbos();

	if (!LoadParaboloids(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

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

	// Is the screen rotated?
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);
	/*
		Initialize Print3D
	*/
	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	// Set the sampler2D uniforms to corresponding texture units
	glUseProgram(m_DefaultProgram.uiId);
	glUniform1i(glGetUniformLocation(m_DefaultProgram.uiId, "s2DMap"), 0);
	
	glUseProgram(m_SkyboxProgram.uiId);
	glUniform1i(glGetUniformLocation(m_SkyboxProgram.uiId, "sSkybox"), 0);

	glUseProgram(m_ParaboloidProgram.uiId);
	glUniform1i(glGetUniformLocation(m_ParaboloidProgram.uiId, "s2DMap"), 0);
	glUniform1f(glGetUniformLocation(m_ParaboloidProgram.uiId, "Near"), g_fCamNear);
	glUniform1f(glGetUniformLocation(m_ParaboloidProgram.uiId, "Far"), g_fCamFar);

	for (int i = 0; i < g_iNumEffects; ++i) {
		glUseProgram(m_aEffectPrograms[i].uiId);
		glUniform1i(glGetUniformLocation(m_aEffectPrograms[i].uiId, "sParaboloids"), 0);
		glUniform1i(glGetUniformLocation(m_aEffectPrograms[i].uiId, "sSkybox"), 1);
	}

	

	/*
		Calculate the projection and view matrices
	*/
	m_mProjection = PVRTMat4::PerspectiveFovRH(g_fCamFOV, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), g_fCamNear, g_fCamFar, PVRTMat4::OGL, bRotate);

	/*
		Set OpenGL ES render states needed for this training course
	*/
	// Enable backface culling and depth test
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);

	// Use a nice bright blue as clear colour
	glClearColor(0.6f, 0.8f, 1.0f, 0.0f);

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES2Glass::ReleaseView()
{
	// Delete textures
	glDeleteTextures(1, &m_uiCubeTex);
	glDeleteTextures(1, &m_uiBalloonTex);
	glDeleteTextures(1, &m_uiParaboloidTexture);

	// Delete program objects
	glDeleteProgram(m_DefaultProgram.uiId);
	glDeleteProgram(m_SkyboxProgram.uiId);
	glDeleteProgram(m_ParaboloidProgram.uiId);
	for (int i = 0; i < g_iNumEffects; ++i)
	{
		glDeleteProgram(m_aEffectPrograms[i].uiId);
	}

	// Delete shader objects
	glDeleteShader(m_uiDefaultVertShader);
	glDeleteShader(m_uiDefaultFragShader);
	glDeleteShader(m_uiSkyboxVertShader);
	glDeleteShader(m_uiSkyboxFragShader);
	glDeleteShader(m_uiParaboloidVertShader);
	for (int i = 0; i < g_iNumEffects; ++i)
	{
		glDeleteShader(m_auiEffectVertShaders[i]);
		glDeleteShader(m_auiEffectFragShaders[i]);
	}

	// Delete buffer objects
	glDeleteBuffers(m_Ball.nNumMesh, m_puiVbo);
	glDeleteBuffers(m_Ball.nNumMesh, m_puiIndexVbo);
	glDeleteBuffers(m_Balloon.nNumMesh, m_puiBalloonVbo);
	glDeleteBuffers(m_Balloon.nNumMesh, m_puiBalloonIndexVbo);
	glDeleteBuffers(1, &m_uiSquareVbo);

	// Delete renderbuffers
	glDeleteRenderbuffers(1, &m_uiParaboloidDepthBuffer);

	// Delete framebuffers
	glDeleteFramebuffers(1, &m_uiParaboloidFramebuffer);

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
bool OGLES2Glass::RenderScene()
{
	if (PVRShellIsKeyPressed(PVRShellKeyNameLEFT))	m_iEffect -= 1;
	if (PVRShellIsKeyPressed(PVRShellKeyNameRIGHT))	m_iEffect += 1;
	m_iEffect = (m_iEffect + g_iNumEffects) % g_iNumEffects;

	UpdateScene();

	DrawIntoParaboloids(PVRTVec3(0, 0, 0));

	// Clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw the ball
	DrawBall();

	// Draw the balloons
	DrawBalloons(&m_DefaultProgram, m_mProjection, m_mView, m_mModels, 2);

	// Draw the skybox
	DrawSkybox();

	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("Glass", g_aszEffectNames[m_iEffect], ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		UpdateScene
 @Description	Moves the scene.
******************************************************************************/
void OGLES2Glass::UpdateScene() {
	// Fetch current time and make sure the previous time isn't greater
	unsigned long ulCurrentTime = PVRShellGetTime();
	if (ulCurrentTime < m_ulTime) m_ulTime = ulCurrentTime;

	// Calculate the time difference
	unsigned long ulTimeDifference = ulCurrentTime - m_ulTime;

	// Store the current time for the next frame
	m_ulTime = ulCurrentTime;

	m_afAngles[0] += ulTimeDifference * 0.0002f;
	m_afAngles[1] -= ulTimeDifference * 0.00008f;

	float fRise = sin(m_afAngles[0] * 3.0f);

	// Rotate the camera
	m_mView = PVRTMat4::LookAtRH(PVRTVec3(0, 0, -10), PVRTVec3(0, 0, 0), PVRTVec3(0, 1, 0)) * PVRTMat4::RotationY(m_afAngles[0] * 0.2f);

	// Rotate the balloon model matrices
	m_mModels[0] = PVRTMat4::RotationY(m_afAngles[0]) * PVRTMat4::Translation(120.0f, fRise * 20.0f, 0.0f) * PVRTMat4::Scale(3.0f, 3.0f, 3.0f);
	m_mModels[1] = PVRTMat4::RotationY(m_afAngles[1]) * PVRTMat4::Translation(-180.0f, -fRise * 20.0f, 0.0f) * PVRTMat4::Scale(3.0f, 3.0f, 3.0f);
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			i32NodeIndex		Node index of the mesh to draw
				pod					POD containing the node to draw
				ppuiVbos			VBO to bind to
				ppuiIbos			IBO to bind to
				i32NumAttributes	Number of vertex attributes to activate
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the meterial prepared.
******************************************************************************/
void OGLES2Glass::DrawMesh(int i32NodeIndex, CPVRTModelPOD* pod, GLuint** ppuiVbos, GLuint** ppuiIbos, int i32NumAttributes)
{
	int i32MeshIndex = pod->pNode[i32NodeIndex].nIdx;
	SPODMesh* pMesh = &pod->pMesh[i32MeshIndex];

	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, (*ppuiVbos)[i32MeshIndex]);
	// bind the index buffer, won't hurt if the handle is 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*ppuiIbos)[i32MeshIndex]);

	// Enable the vertex attribute arrays
	for (int i = 0; i < i32NumAttributes; ++i) glEnableVertexAttribArray(i);

	// Set the vertex attribute offsets
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, pMesh->sVertex.nStride, pMesh->sVertex.pData);
	glVertexAttribPointer(NORMAL_ARRAY, 3, GL_FLOAT, GL_FALSE, pMesh->sNormals.nStride, pMesh->sNormals.pData);
	if (pMesh->psUVW) glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, pMesh->psUVW[0].nStride, pMesh->psUVW[0].pData);

	/*
		The geometry can be exported in 4 ways:
		- Indexed Triangle list
		- Non-Indexed Triangle list
		- Indexed Triangle strips
		- Non-Indexed Triangle strips
	*/
	if(pMesh->nNumStrips == 0)
	{
		if((*ppuiIbos)[i32MeshIndex])
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
		for(int i = 0; i < (int)pMesh->nNumStrips; ++i)
		{
			int offset = 0;
			if((*ppuiIbos)[i32MeshIndex])
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
	for (int i = 0; i < i32NumAttributes; ++i) glDisableVertexAttribArray(i);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/*!****************************************************************************
 @Function		DrawBalloons
 @Input			psProgram			Program to use
				mProjection			Projection matrix to use
				mView				View matrix to use
				pmModels			A pointer to an array of model matrices
				iNum				Number of balloons to draw
 @Description	Draws balloons.
******************************************************************************/
void OGLES2Glass::DrawBalloons(Program* psProgram, PVRTMat4 mProjection, PVRTMat4 mView, PVRTMat4* pmModels, int iNum) {
	// Use shader program
	glUseProgram(psProgram->uiId);

	// Bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uiBalloonTex);

	PVRTMat4 mModelView, mMVP;

	for (int i = 0; i < iNum; ++i)
	{
		mModelView = mView * pmModels[i];
		mMVP =  mProjection * mModelView;
	
		glUniformMatrix4fv(psProgram->auiLoc[eMVMatrix], 1, GL_FALSE, mModelView.ptr());
		glUniformMatrix4fv(psProgram->auiLoc[eMVPMatrix], 1, GL_FALSE, mMVP.ptr());

		// Calculate and set the model space light direction
		PVRTVec3 vLightDir = pmModels[i].inverse() * PVRTVec4(19, 22, -50, 0);
		vLightDir = vLightDir.normalize();
		glUniform3fv(psProgram->auiLoc[eLightDir], 1, vLightDir.ptr());

		// Calculate and set the model space eye position
		PVRTVec3 vEyePos = mModelView.inverse() * PVRTVec4(0.0f, 0.0f, 0.0f, 1.0f);
		glUniform3fv(psProgram->auiLoc[eEyePos], 1, vEyePos.ptr());

		// Now that the uniforms are set, call another function to actually draw the mesh.
		DrawMesh(0, &m_Balloon, &m_puiBalloonVbo, &m_puiBalloonIndexVbo, 3);
	}
}

/*!****************************************************************************
 @Function		DrawSkybox
 @Description	Draws the skybox onto the screen.
******************************************************************************/
void OGLES2Glass::DrawSkybox()
{
	glUseProgram(m_SkyboxProgram.uiId);

	PVRTMat4 mVP = m_mProjection * m_mView;
	PVRTMat4 mInvVP = mVP.inverseEx();

	glUniformMatrix4fv(m_SkyboxProgram.auiLoc[eInvVPMatrix], 1, GL_FALSE, mInvVP.ptr());

	PVRTVec3 vEyePos = m_mView.inverse() * PVRTVec4(0, 0, 0, 1);

	glUniform3fv(m_SkyboxProgram.auiLoc[eEyePos], 1, vEyePos.ptr());

	glBindBuffer(GL_ARRAY_BUFFER, m_uiSquareVbo);

	glEnableVertexAttribArray(VERTEX_ARRAY);
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_uiCubeTex);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/*!****************************************************************************
 @Function		DrawBall
 @Description	Draws the reflective and refractive ball onto the screen.
******************************************************************************/
void OGLES2Glass::DrawBall() {
	// Set model view projection matrix
	PVRTMat4 mModel, mModelView, mMVP;

	mModel = PVRTMat4::Scale(6.0f, 6.0f, 6.0f);
	mModelView = m_mView * mModel;
	mMVP = m_mProjection * mModelView;

	// Use shader program
	glUseProgram(m_aEffectPrograms[m_iEffect].uiId);

	// Bind textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uiParaboloidTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_uiCubeTex);

	glUniformMatrix4fv(m_aEffectPrograms[m_iEffect].auiLoc[eMVPMatrix], 1, GL_FALSE, mMVP.ptr());

	// Set model matrix
	PVRTMat3 Model3x3 = PVRTMat3(mModel);
	glUniformMatrix3fv(m_aEffectPrograms[m_iEffect].auiLoc[eMMatrix], 1, GL_FALSE, Model3x3.ptr());

	// Set eye position in model space
	PVRTVec4 vEyePosModel;
	vEyePosModel = mModelView.inverse() * PVRTVec4(0, 0, 0, 1);
	glUniform3fv(m_aEffectPrograms[m_iEffect].auiLoc[eEyePos], 1, &vEyePosModel.x);

	// Now that the uniforms are set, call another function to actually draw the mesh
	DrawMesh(0, &m_Ball, &m_puiVbo, &m_puiIndexVbo, 2);
}

/*!****************************************************************************
 @Function		DrawIntoParaboloids
 @Description	Draws the scene from the position of the ball into the two
				paraboloid textures.
******************************************************************************/
void OGLES2Glass::DrawIntoParaboloids(PVRTVec3 position)
{
	// Bind and clear the paraboloid framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_uiParaboloidFramebuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set the viewport to the left
	glViewport(0, 0, g_ParaboloidTexSize, g_ParaboloidTexSize);

	// Create the first view matrix and make it flip the X coordinate
	PVRTMat4 mView = PVRTMat4::LookAtRH(position, position + PVRTVec3(0, 0, 1), PVRTVec3(0, 1, 0));
	mView = PVRTMat4::Scale(-1.0f, 1.0f, 1.0f) * mView;

	// Switch to front face culling due to flipped winding order
	glCullFace(GL_FRONT);

	// Draw the balloons
	DrawBalloons(&m_ParaboloidProgram, PVRTMat4::Identity(), mView, m_mModels, 2);

	// Switch back to back face culling
	glCullFace(GL_BACK);

	// Shift the viewport to the right
	glViewport(g_ParaboloidTexSize, 0, g_ParaboloidTexSize, g_ParaboloidTexSize);

	// Create the second view matrix
	mView = PVRTMat4::LookAtRH(position, position - PVRTVec3(0, 0, 1), PVRTVec3(0, 1, 0));

	// Draw the balloons
	DrawBalloons(&m_ParaboloidProgram, PVRTMat4::Identity(), mView, m_mModels, 2);

	// Bind back the original framebuffer and reset the viewport
	glBindFramebuffer(GL_FRAMEBUFFER, m_iOriginalFramebuffer);
	glViewport(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));
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
	return new OGLES2Glass;
}

/******************************************************************************
 End of file (OGLES2Glass.cpp)
******************************************************************************/
