/******************************************************************************

 @File         OGLES3Bloom.cpp

 @Title        Bloom

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows how to do a bloom effect

******************************************************************************/

#include "PVRShell.h"
#include "OGLES3Tools.h"

/******************************************************************************
 Defines
******************************************************************************/
// Index to bind the attributes to vertex shaders
#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY	2

#define AXIS_ALIGNED_QUAD_VERTEX_ARRAY   0
#define AXIS_ALIGNED_QUAD_TEXCOORD_ARRAY 1

/******************************************************************************
 Consts
******************************************************************************/
// Camera constants. Used for making the projection matrix
const float g_fCameraNear = 60.0f;
const float g_fCameraFar  = 1000.0f;
const float g_fCameraFOV  = PVRT_PI / 6;

/******************************************************************************
 Content file names
******************************************************************************/

// Source and binary shaders
const char c_szFragShaderSrcFile[]	= "FragShader.fsh";
const char c_szFragShaderBinFile[]	= "FragShader.fsc";
const char c_szVertShaderSrcFile[]	= "VertShader.vsh";
const char c_szVertShaderBinFile[]	= "VertShader.vsc";
const char c_szPreBloomFragShaderSrcFile[]	= "PreBloomFragShader.fsh";
const char c_szPreBloomFragShaderBinFile[]	= "PreBloomFragShader.fsc";
const char c_szPreBloomVertShaderSrcFile[]	= "PreBloomVertShader.vsh";
const char c_szPreBloomVertShaderBinFile[]	= "PreBloomVertShader.vsc";
const char c_szPostBloomFragShaderSrcFile[]	= "PostBloomFragShader.fsh";
const char c_szPostBloomFragShaderBinFile[]	= "PostBloomFragShader.fsc";
const char c_szPostBloomVertShaderSrcFile[]	= "PostBloomVertShader.vsh";
const char c_szPostBloomVertShaderBinFile[]	= "PostBloomVertShader.vsc";
const char c_szBlurFragSrcFile[]	= "BlurFragShader.fsh";
const char c_szBlurFragBinFile[]	= "BlurFragShader.fsc";
const char c_szBlurVertSrcFile[]	= "BlurVertShader.vsh";
const char c_szBlurVertBinFile[]	= "BlurVertShader.vsc";

// PVR texture files
const char c_szBaseTexFile[]			= "BaseTex.pvr";
const char c_szBloomMappingTexFile[]	= "bloom_mapping.pvr";

// POD scene files
const char c_szSceneFile[]			= "Mask.pod";

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES3Bloom : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// 3D Model
	CPVRTModelPOD	m_Scene;

	// OpenGL handles for shaders, textures and VBOs
	GLuint m_uiVertShader;
	GLuint m_uiFragShader;
	GLuint m_uiPreBloomVertShader;
	GLuint m_uiPreBloomFragShader;
	GLuint m_uiPostBloomVertShader;
	GLuint m_uiPostBloomFragShader;
	GLuint m_uiBlurFragShader;
	GLuint m_uiBlurVertShader;

	GLuint* m_puiVbo;
	GLuint* m_puiIndexVbo;

	GLint  m_i32OriginalFbo;
	GLuint m_uiBlurFramebufferObjects[2];
	GLuint m_uiBlurTextures[2];
	GLuint m_uiDepthBuffer;

	GLuint m_uiBaseTex;
	GLuint m_uiBloomMappingTexture;

	int m_i32TexSize;

	bool m_bApplyBloom;
	unsigned int m_ui32BlurPasses;
	float m_fRotation;
	float m_fTexelOffset;
	float m_fBloomIntensity;

	// Group shader programs and their uniform locations together
	struct
	{
		GLuint uiId;
		GLuint uiMVPMatrixLoc;
		GLuint uiLightDirLoc;
	}
	m_ShaderProgram;

	struct
	{
		GLuint uiId;
		GLuint uiTexelOffsetX;
		GLuint uiTexelOffsetY;
	}
	m_BlurShaderProgram;

	struct
	{
		GLuint uiId;
		GLuint uiMVPMatrixLoc;
		GLuint uiLightDirLoc;
		GLuint uiBloomIntensity;
	}
	m_PreBloomShaderProgram;

	struct
	{
		GLuint uiId;
	}
	m_PostBloomShaderProgram;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	bool LoadTextures(CPVRTString* pErrorStr);
	bool LoadShaders(CPVRTString* pErrorStr);
	bool LoadVbos(CPVRTString* pErrorStr);

	void HandleInput();
	void DrawMesh(int i32NodeIndex);
    void DrawAxisAlignedQuad(PVRTVec2 afLowerLeft, PVRTVec2 afUpperRight);
	void DrawAxisAlignedQuad(PVRTVec2 afLowerLeft, PVRTVec2 afLowerLeftUV,
										  PVRTVec2 afUpperRight, PVRTVec2 afUpperRightUV);
};

/*!****************************************************************************
 @Function		LoadTextures
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course
******************************************************************************/
bool OGLES3Bloom::LoadTextures(CPVRTString* pErrorStr)
{
    /*
		Loads the textures.
		For a more detailed explanation, see Texturing and IntroducingPVRTools
	*/

	if(PVRTTextureLoadFromPVR(c_szBaseTexFile, &m_uiBaseTex) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Failed to load texture.";
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	if (PVRTTextureLoadFromPVR(c_szBloomMappingTexFile, &m_uiBloomMappingTexture) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Failed to load texture: bloom_mapping.pvr .";
		return false;
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create the color textures
	glGenTextures(2, m_uiBlurTextures);

	for (unsigned int i=0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, m_uiBlurTextures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_i32TexSize, m_i32TexSize, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	GLenum status = glGetError();
	if (status != GL_NO_ERROR)
	{
		*pErrorStr += "Error: Could not create renderbuffer textures.";
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
bool OGLES3Bloom::LoadShaders(CPVRTString* pErrorStr)
{
	/*
		Load and compile the shaders from files.
		Binary shaders are tried first, source shaders
		are used as fallback.
	*/
	if(PVRTShaderLoadFromFile(
			c_szVertShaderBinFile, c_szVertShaderSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiVertShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	if (PVRTShaderLoadFromFile(
			c_szFragShaderBinFile, c_szFragShaderSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_uiFragShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	// Set up and link the shader program
	const char* aszAttribs[] = { "inVertex", "inNormal", "inTexCoord" };

	if(PVRTCreateProgram(
			&m_ShaderProgram.uiId, m_uiVertShader, m_uiFragShader, aszAttribs, 3, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	// Set the sampler2D variable to the first texture unit
	glUniform1i(glGetUniformLocation(m_ShaderProgram.uiId, "sTexture"), 0);

	// Store the location of uniforms for later use
	m_ShaderProgram.uiMVPMatrixLoc	= glGetUniformLocation(m_ShaderProgram.uiId, "MVPMatrix");
	m_ShaderProgram.uiLightDirLoc	= glGetUniformLocation(m_ShaderProgram.uiId, "LightDirection");


	/*
	    Load shaders required for bloom post processing effect
	*/

	/*
	    Pre-Bloom shader program
    */

	if(PVRTShaderLoadFromFile(
		c_szPreBloomVertShaderBinFile, c_szPreBloomVertShaderSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiPreBloomVertShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	if(PVRTShaderLoadFromFile(
		c_szPreBloomFragShaderBinFile, c_szPreBloomFragShaderSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_uiPreBloomFragShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	// Set up and link the shader program
	const char* aszPreBloomAttribs[] = { "inVertex", "inNormal", "inTexCoord" };

	if(PVRTCreateProgram(
		&m_PreBloomShaderProgram.uiId, m_uiPreBloomVertShader, m_uiPreBloomFragShader, aszPreBloomAttribs, 3, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	// Set the sampler2D variables to the first and second texture units
	glUniform1i(glGetUniformLocation(m_PreBloomShaderProgram.uiId, "sBloomMapping"), 0);

	// Store the location of uniforms for later use
	m_PreBloomShaderProgram.uiMVPMatrixLoc = glGetUniformLocation(m_PreBloomShaderProgram.uiId, "MVPMatrix");
	m_PreBloomShaderProgram.uiLightDirLoc  = glGetUniformLocation(m_PreBloomShaderProgram.uiId, "LightDirection");
	m_PreBloomShaderProgram.uiBloomIntensity  = glGetUniformLocation(m_PreBloomShaderProgram.uiId, "fBloomIntensity");

	/*
	    Post-Bloom shader program
    */

	if(PVRTShaderLoadFromFile(
		c_szPostBloomVertShaderBinFile, c_szPostBloomVertShaderSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiPostBloomVertShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	if(PVRTShaderLoadFromFile(
		c_szPostBloomFragShaderBinFile, c_szPostBloomFragShaderSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_uiPostBloomFragShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	// Set up and link the shader program
	const char* aszPostBloomAttribs[] = { "inVertex", "inTexCoord" };

	if(PVRTCreateProgram(
		&m_PostBloomShaderProgram.uiId, m_uiPostBloomVertShader, m_uiPostBloomFragShader, aszPostBloomAttribs, 2, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	// Set the sampler2D variables to the first and second texture units
	glUniform1i(glGetUniformLocation(m_PostBloomShaderProgram.uiId, "sTexture"), 0);

	/*
	    Blur shader program
    */

	if (PVRTShaderLoadFromFile(
		c_szBlurFragBinFile, c_szBlurFragSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_uiBlurFragShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	if (PVRTShaderLoadFromFile(
		c_szBlurVertBinFile, c_szBlurVertSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiBlurVertShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	// Set up and link the shader program
	const char* aszBlurAttribs[] = { "inVertex", "inTexCoord" };

	if(PVRTCreateProgram(
		&m_BlurShaderProgram.uiId, m_uiBlurVertShader, m_uiBlurFragShader, aszBlurAttribs, 2, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	// Set the sampler2D variables to the first and second texture units
	glUniform1i(glGetUniformLocation(m_BlurShaderProgram.uiId, "sTexture"), 0);

	m_BlurShaderProgram.uiTexelOffsetX = glGetUniformLocation(m_BlurShaderProgram.uiId, "TexelOffsetX");
	m_BlurShaderProgram.uiTexelOffsetY = glGetUniformLocation(m_BlurShaderProgram.uiId, "TexelOffsetY");

	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
bool OGLES3Bloom::LoadVbos(CPVRTString* pErrorStr)
{
	if(!m_Scene.pMesh[0].pInterleaved)
	{
		*pErrorStr = "ERROR: Bloom requires the pod data to be interleaved. Please re-export with the interleaved option enabled.";
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
 @Return		bool		true if no error occured
 @Description	Code in InitApplication() will be called by PVRShell once per
				run, before the rendering context is created.
				Used to initialize variables that are not dependant on it
				(e.g. external modules, loading meshes, etc.)
				If the rendering context is lost, InitApplication() will
				not be called again.
******************************************************************************/
bool OGLES3Bloom::InitApplication()
{
	// Apply bloom per default
	m_bApplyBloom = true;
	// Initial number of blur passes, can be changed during runtime
	m_ui32BlurPasses = 1;

	m_fRotation = 0.0f;

	// Blur render target size (power-of-two)
	m_i32TexSize = 128;

	// Texel offset for blur filter kernle
	m_fTexelOffset = 1.0f / (float)m_i32TexSize;

	// Altered weights for the faster filter kernel
	float w1 = 0.0555555f;
	float w2 = 0.2777777f;
	float intraTexelOffset = (w1 / (w1 + w2)) * m_fTexelOffset;
	m_fTexelOffset += intraTexelOffset;

	// Intensity multiplier for the bloom effect
	m_fBloomIntensity = 0.5f;

	m_puiVbo = 0;
	m_puiIndexVbo = 0;

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
bool OGLES3Bloom::QuitApplication()
{
	// Free the memory allocated for the scene
	m_Scene.Destroy();

	delete[] m_puiVbo;
	delete[] m_puiIndexVbo;

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
bool OGLES3Bloom::InitView()
{
	CPVRTString ErrorStr;

	/*
		Initialize VBO data
	*/
	if(!LoadVbos(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	/*
		Load textures
	*/
	if(!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	/*
		Load and compile the shaders & link programs
	*/
	if(!LoadShaders(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	/*
		Initialize Print3D
	*/
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	/*
		Get the currently bound frame buffer object. On most platforms this just gives 0.
	 */
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_i32OriginalFbo);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// create a new depth render buffer
	glGenRenderbuffers(1, &m_uiDepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_uiDepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_i32TexSize, m_i32TexSize);

	/*
		Set OpenGL ES render states needed for this training course
	*/
	glGenFramebuffers(2, m_uiBlurFramebufferObjects);

	for (unsigned int i=0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_uiBlurFramebufferObjects[i]);
		
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		// The first render target needs a depth buffer, as we have to draw "blooming" 3d objects into it		
		if (i==0)
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_uiDepthBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uiBlurTextures[i], 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			PVRShellSet(prefExitMessage, "ERROR: Frame buffer not set up correctly\n");
			return false;
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_i32OriginalFbo);

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES3Bloom::ReleaseView()
{
	// Deletes the textures
	glDeleteTextures(2, m_uiBlurTextures);
	glDeleteTextures(1, &m_uiBaseTex);
	glDeleteTextures(1, &m_uiBloomMappingTexture);
	glDeleteRenderbuffers(1, &m_uiDepthBuffer);

	// Delete program and shader objects
	glDeleteProgram(m_ShaderProgram.uiId);
	glDeleteProgram(m_PreBloomShaderProgram.uiId);
	glDeleteProgram(m_PostBloomShaderProgram.uiId);
	glDeleteProgram(m_BlurShaderProgram.uiId);

	glDeleteShader(m_uiVertShader);
	glDeleteShader(m_uiFragShader);
	glDeleteShader(m_uiPreBloomVertShader);
	glDeleteShader(m_uiPreBloomFragShader);
	glDeleteShader(m_uiPostBloomVertShader);
	glDeleteShader(m_uiPostBloomFragShader);
	glDeleteShader(m_uiBlurFragShader);
	glDeleteShader(m_uiBlurVertShader);

	// Delete buffer objects
	glDeleteBuffers(m_Scene.nNumMesh, m_puiVbo);
	glDeleteBuffers(m_Scene.nNumMesh, m_puiIndexVbo);

	glDeleteFramebuffers(2, m_uiBlurFramebufferObjects);

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
bool OGLES3Bloom::RenderScene()
{
	HandleInput();

	// Calculate the mask and light rotation based on the passed time
	static unsigned long ulPreviousTime = PVRShellGetTime();
	unsigned long ulNowTime = PVRShellGetTime();
	m_fRotation += PVRT_PI * (ulNowTime - ulPreviousTime) * 0.0002f;
	ulPreviousTime = ulNowTime;
	if (m_fRotation > (PVRT_PI * 2.0f))
		m_fRotation -= PVRT_PI * 2.0f;
	
	// Calculate the model, view and projection matrix
	float fModelAngleY = m_fRotation;
	float fLightAngleY = -m_fRotation;

	PVRTMat4 mWorld = PVRTMat4::RotationY(fModelAngleY);
	PVRTMat4 mLight = PVRTMat4::RotationY(fLightAngleY);
	PVRTMat4 mView = PVRTMat4::LookAtRH(PVRTVec3(0, 0, 150), PVRTVec3(0, 0, 0), PVRTVec3(0, 1, 0));

	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);
	PVRTMat4 mProjection = PVRTMat4::PerspectiveFovRH(g_fCameraFOV, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), g_fCameraNear, g_fCameraFar, PVRTMat4::OGL, bRotate);
	PVRTMat4 mMVP = mProjection * mView * mWorld;

	// Simple rotating directional light in model-space
	PVRTVec4 vMsLightPos = mWorld.inverse() * mLight * PVRTVec4(0.5f, -1, -0.5f, 0).normalize();

	glBindFramebuffer(GL_FRAMEBUFFER, m_i32OriginalFbo);
	glClearColor(0.075f, 0.1f, 0.125f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use simple shader program to render the mask
	glUseProgram(m_ShaderProgram.uiId);
	glUniformMatrix4fv(m_ShaderProgram.uiMVPMatrixLoc, 1, GL_FALSE, mMVP.f);
	glUniform3fv(m_ShaderProgram.uiLightDirLoc, 1, &vMsLightPos.x);

	// Draw the mesh
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uiBaseTex);
	DrawMesh(0);

	if (m_bApplyBloom)
	{
		// First render the objects which shall have the bloom effect to a texture
		glBindFramebuffer(GL_FRAMEBUFFER, m_uiBlurFramebufferObjects[0]);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glViewport(0, 0, m_i32TexSize, m_i32TexSize);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(m_PreBloomShaderProgram.uiId);
		glUniformMatrix4fv(m_PreBloomShaderProgram.uiMVPMatrixLoc, 1, GL_FALSE, mMVP.f);
		glUniform3fv(m_PreBloomShaderProgram.uiLightDirLoc, 1, &vMsLightPos.x);
		glUniform1f(m_PreBloomShaderProgram.uiBloomIntensity, m_fBloomIntensity);

		// Draw the mesh
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_uiBloomMappingTexture);
		DrawMesh(0);

		//Invalidate the depth attachment we don't need to avoid unnecessary copying to system memory
		const GLenum attachment = GL_DEPTH_ATTACHMENT;
		glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attachment);

		/*
		  Blur the generated image n-times
		*/
		for (unsigned int i=0; i < m_ui32BlurPasses; i++)
		{
			/*
			 Apply horizontal blur
			*/
			glBindFramebuffer(GL_FRAMEBUFFER, m_uiBlurFramebufferObjects[1]);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_uiBlurTextures[0]);

			// Use the shader program for the scene
			glUseProgram(m_BlurShaderProgram.uiId);
			glUniform1f(m_BlurShaderProgram.uiTexelOffsetX, m_fTexelOffset);
			glUniform1f(m_BlurShaderProgram.uiTexelOffsetY, 0.0f);

			DrawAxisAlignedQuad(PVRTVec2(-1, -1), PVRTVec2(1, 1));

			//No attachments we can invalidate here, as only color was used which is necessary.

			/*
			   Apply vertical blur
			*/
			glBindFramebuffer(GL_FRAMEBUFFER, m_uiBlurFramebufferObjects[0]);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_uiBlurTextures[1]);

			// Use the shader program for the scene
			glUseProgram(m_BlurShaderProgram.uiId);
			glUniform1f(m_BlurShaderProgram.uiTexelOffsetX, 0.0f);
			glUniform1f(m_BlurShaderProgram.uiTexelOffsetY, m_fTexelOffset);

			DrawAxisAlignedQuad(PVRTVec2(-1, -1), PVRTVec2(1, 1));

			//Invalidate the depth attachment we don't need to avoid unnecessary copying to system memory
			const GLenum attachment = GL_DEPTH_ATTACHMENT;
			glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attachment);
		}

		/*
		  Draw scene with bloom
		*/
		glBindFramebuffer(GL_FRAMEBUFFER, m_i32OriginalFbo);
		glViewport(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_uiBlurTextures[0]);

		// Use the shader program for the scene
		glUseProgram(m_PostBloomShaderProgram.uiId);

		/*
		    The following section will draw a quad on the screen
			where the post processing pixel shader shall be executed.
			Try to minimize the area by only drawing where the actual
			post processing should happen, as this is a very costly operation.
		*/
		if (PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen)) {
			DrawAxisAlignedQuad(PVRTVec2(-0.875f, -0.5f), PVRTVec2(0.0625f, 0.25f),
					            PVRTVec2(0.8755f, 0.5f), PVRTVec2(0.9375f, 0.75f));
		}
		else
		{
			DrawAxisAlignedQuad(PVRTVec2(-0.5f, -0.875f), PVRTVec2(0.25f, 0.0625f),
				                PVRTVec2(0.5f, 0.875f), PVRTVec2(0.75f, 0.9375f));
		}

		glDisable(GL_BLEND);
	}

	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("Bloom", NULL, ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		HandleInput
 @Description	Handles user input and updates live variables accordingly.
******************************************************************************/
void OGLES3Bloom::HandleInput()
{
	// Keyboard input (cursor to change Reflection Flag)
	if (PVRShellIsKeyPressed(PVRShellKeyNameUP) || PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
	{
		m_bApplyBloom = !m_bApplyBloom;
	}

	if (PVRShellIsKeyPressed(PVRShellKeyNameLEFT) && (m_fBloomIntensity > 0.0f))
		m_fBloomIntensity -= 0.1f;

	if (PVRShellIsKeyPressed(PVRShellKeyNameRIGHT))
		m_fBloomIntensity += 0.1f;
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			i32NodeIndex		Node index of the mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the meterial prepared.
******************************************************************************/
void OGLES3Bloom::DrawMesh(int i32NodeIndex)
{
	int i32MeshIndex = m_Scene.pNode[i32NodeIndex].nIdx;
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
				glDrawElements(GL_TRIANGLE_STRIP, pMesh->pnStripLength[i]+2, GL_UNSIGNED_SHORT, (void*) (offset * sizeof(GLshort)));
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

/*!****************************************************************************
 @Function		DrawFullScreenQuad
 @Input			afLowerLeft		Lower left corner of the quad in normalized device coordinates
                afUpperRight    Upper right corner of the quad in normalized device coordinates
 @Description	Draws a textured fullscreen quad
******************************************************************************/
void OGLES3Bloom::DrawAxisAlignedQuad(PVRTVec2 afLowerLeft,
									  PVRTVec2 afUpperRight)
{
	glDisable(GL_DEPTH_TEST);

	// Enable vertex arributes
	glEnableVertexAttribArray(AXIS_ALIGNED_QUAD_VERTEX_ARRAY);
	glEnableVertexAttribArray(AXIS_ALIGNED_QUAD_TEXCOORD_ARRAY);

	const float afVertexData[] = { afLowerLeft.x, afLowerLeft.y,  afUpperRight.x, afLowerLeft.y,
		                           afLowerLeft.x, afUpperRight.y,  afUpperRight.x, afUpperRight.y };
	glVertexAttribPointer(VERTEX_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, afVertexData);

	const float afTexCoordData[] = { 0, 0,  1, 0,  0, 1,  1, 1 };
	glVertexAttribPointer(NORMAL_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, afTexCoordData);

	// Draw the quad
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// Disable vertex arributes
	glDisableVertexAttribArray(AXIS_ALIGNED_QUAD_VERTEX_ARRAY);
	glDisableVertexAttribArray(AXIS_ALIGNED_QUAD_TEXCOORD_ARRAY);

	// Disable blending
	glEnable(GL_DEPTH_TEST);
}


/*!****************************************************************************
 @Function		DrawFullScreenQuad
 @Input			afLowerLeft		Lower left corner of the quad in normalized device coordinates
				afLowerLeftUV   UV coordinates of lower left corner
                afUpperRight    Upper right corner of the quad in normalized device coordinates
				afUpperRightUV	UV coordinates of upper right corner
 @Description	Draws a textured fullscreen quad
******************************************************************************/
void OGLES3Bloom::DrawAxisAlignedQuad(PVRTVec2 afLowerLeft, PVRTVec2 afLowerLeftUV,
									  PVRTVec2 afUpperRight, PVRTVec2 afUpperRightUV)
{
	glDisable(GL_DEPTH_TEST);

	// Enable vertex arributes
	glEnableVertexAttribArray(AXIS_ALIGNED_QUAD_VERTEX_ARRAY);
	glEnableVertexAttribArray(AXIS_ALIGNED_QUAD_TEXCOORD_ARRAY);

	const float afVertexData[] = { afLowerLeft.x, afLowerLeft.y,  afUpperRight.x, afLowerLeft.y,
		                           afLowerLeft.x, afUpperRight.y,  afUpperRight.x, afUpperRight.y };
	glVertexAttribPointer(VERTEX_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, afVertexData);

	//const float afTexCoordData[] = { afLowerLeftUV.x, afLowerLeftUV.y,  afUpperRightUV.x, afLowerLeftUV.y,
	//								 afLowerLeftUV.x, afUpperRightUV.y,  afUpperRightUV.x, afUpperRightUV.y };
	float afTexCoordData[8];
	afTexCoordData[0] = afLowerLeftUV.x;
	afTexCoordData[1] = afLowerLeftUV.y;
	afTexCoordData[2] = afUpperRightUV.x;
	afTexCoordData[3] = afLowerLeftUV.y;
	afTexCoordData[4] = afLowerLeftUV.x;
	afTexCoordData[5] = afUpperRightUV.y;
	afTexCoordData[6] = afUpperRightUV.x;
	afTexCoordData[7] = afUpperRightUV.y;

	glVertexAttribPointer(NORMAL_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, afTexCoordData);

	// Draw the quad
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// Disable vertex arributes
	glDisableVertexAttribArray(AXIS_ALIGNED_QUAD_VERTEX_ARRAY);
	glDisableVertexAttribArray(AXIS_ALIGNED_QUAD_TEXCOORD_ARRAY);

	// Disable blending
	glEnable(GL_DEPTH_TEST);
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
	return new OGLES3Bloom();
}

/******************************************************************************
 End of file (OGLES3Bloom.cpp)
******************************************************************************/

