/******************************************************************************

 @File         OGLES2RenderToTexture.cpp

 @Title        RenderToTexture

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows how to render to a pBuffer surface or FBO and bind that to a
               texture.

******************************************************************************/

#include "PVRShell.h"
#include "OGLES2Tools.h"

/******************************************************************************
 Content file names
******************************************************************************/

// Source and binary shaders
const char c_szFragShaderSrcFile[]	= "FragShader.fsh";
const char c_szFragShaderBinFile[]	= "FragShader.fsc";
const char c_szVertShaderSrcFile[]	= "VertShader.vsh";
const char c_szVertShaderBinFile[]	= "VertShader.vsc";

// Scene
/*
	The .pod file was exported from 3DSMax using PVRGeoPOD.
*/
const char c_szSceneFile[] = "RenderToTexture.pod";
const char c_szMaskTex[]   = "YellowWood.pvr";

/******************************************************************************
 Consts
******************************************************************************/
// Camera constants. Used for making the projection matrix
const float g_fCameraNear = 4.0f;
const float g_fCameraFar  = 500.0f;

const char* g_pszDesc[] = {
	"Using FBOs",						// eMultisampleExtension_None
	"Using multisampled FBOs [IMG]",	// eMultisampleExtension_IMG
	"Using multisampled FBOs [EXT]",	// eMultisampleExtension_EXT
};

/******************************************************************************
 shader attributes
******************************************************************************/
// vertex attributes
enum EVertexAttrib {
	eVERTEX_ARRAY, eNORMAL_ARRAY, eTEXCOORD_ARRAY, eNumAttribs };
const char* g_aszAttribNames[] = {
	"inVertex", "inNormal", "inTexCoord" };

// shader uniforms
enum EUniform {
	eMVPMatrix, eLightDirection, eNumUniforms };
const char* g_aszUniformNames[] = {
	"MVPMatrix", "LightDirection" };

/*****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES2RenderToTexture : public PVRShell
{
	enum EMultisampleExtension
	{
		eMultisampleExtension_None,
		eMultisampleExtension_IMG,
		eMultisampleExtension_EXT
	};
	
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// Vertex Buffer Object (VBO) handles
	GLuint*	m_puiVbo;
	GLuint*	m_puiIndexVbo;

	// 3D Model
	CPVRTModelPOD	m_Scene;

	// Projection and Model View matrices
	PVRTMat4 m_mProjection, m_mView;
	PVRTMat4 m_mR2TProjection, m_mR2TView;

	// Texture IDs used by the app
	GLuint	m_uiTextureID;
	GLuint	m_uiTextureToRenderTo;
	GLuint	m_uiTextureToRenderToMultisampled;

	// Handles for our shaders
	GLuint	m_uiVertShader;
	GLuint	m_uiFragShader;

	// Group shader programs and their uniform locations together
	struct
	{
		GLuint uiId;
		GLuint auiLoc[eNumUniforms];
	}
	m_ShaderProgram;

	// Variables used for the animation
	float	m_fAngleY;

	// App Variables
	int		m_i32TexSize;

	// Render contexts, etc
	GLint	  m_i32OriginalFbo;

	// Handle for our FBO and the depth buffer that it requires
	GLuint m_uFBO;
	GLuint m_uDepthBuffer;

	// Handle for our multisampled FBO and the depth buffer that it requires
	GLuint m_uFBOMultisampled;
	GLuint m_uDepthBufferMultisampled;

	// Extensions
	CPVRTgles2Ext m_Extensions;

	// Discard the frame buffer attachments
	bool m_bDiscard;
	bool m_bMultisampledSupported;
	bool m_bUseMultisampled;
	EMultisampleExtension m_eMultisampleMode;

public:
	OGLES2RenderToTexture() : m_puiVbo(0),
							m_puiIndexVbo(0),
							m_uiTextureID(0),
							m_uiTextureToRenderTo(0),
							m_uiTextureToRenderToMultisampled(0),
							m_fAngleY(0),
							m_i32OriginalFbo(0),
							m_uFBOMultisampled(0),
						    m_uDepthBufferMultisampled(0),
							m_bUseMultisampled(false)
	{
	}

	// PVRShell functions
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	bool LoadVbos(CPVRTString* pErrorStr);
	void DrawMesh(unsigned int ui32MeshID);
	bool LoadTextures(CPVRTString* pErrorStr);
	bool LoadShaders(CPVRTString* pErrorStr);
	bool CreateFBO();
	bool CreateMultisampledFBO();
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
bool OGLES2RenderToTexture::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	/*
		Loads the scene from the .pod file into a CPVRTModelPOD object.
		We could also export the scene as a header file and
		load it with ReadFromMemory().
	*/

	if(m_Scene.ReadFromFile(c_szSceneFile) != PVR_SUCCESS)
	{
		CPVRTString ErrorStr = "ERROR: Couldn't load '" + CPVRTString(c_szSceneFile) + "'.";
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
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
bool OGLES2RenderToTexture::QuitApplication()
{
	// Frees the memory allocated for the scene
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
bool OGLES2RenderToTexture::InitView()
{
	m_Extensions.LoadExtensions();

	CPVRTString ErrorStr;
	/*
		Initialise Print3D
	*/
    bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	//	Initialize VBO data
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
	if (!LoadShaders(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Create normal FBO
	if(!CreateFBO())
		return false;

	// Create a multisampled FBO if the required extension is supported
	m_eMultisampleMode       = eMultisampleExtension_None;
	m_bMultisampledSupported = false;
	m_bMultisampledSupported |= CPVRTgles2Ext::IsGLExtensionSupported("GL_EXT_multisampled_render_to_texture");
	m_bMultisampledSupported |= CPVRTgles2Ext::IsGLExtensionSupported("GL_IMG_multisampled_render_to_texture");
	if (m_bMultisampledSupported)
	{
		m_bUseMultisampled = m_bMultisampledSupported = CreateMultisampledFBO();
	}

	// Check to see if the GL_EXT_discard_framebuffer extension is supported
	m_bDiscard = CPVRTgles2Ext::IsGLExtensionSupported("GL_EXT_discard_framebuffer");
	if(m_bDiscard)
	{
		m_bDiscard = m_Extensions.glDiscardFramebufferEXT != 0;
	}

	// Setup some render states

	// Enable the depth test
	glEnable(GL_DEPTH_TEST);

	// Enable culling
	glEnable(GL_CULL_FACE);

	// Setup view and projection matrices used for when rendering to the texture

	// Calculate the view matrix
	m_mR2TView = PVRTMat4::LookAtRH(PVRTVec3(0, 0, 60), PVRTVec3(0, 0, 0), PVRTVec3(0, 1, 0));

	// Calculate the projection matrix
	// Note: As we'll be rendering to a texture we don't need to take the screen rotation into account
	m_mR2TProjection = PVRTMat4::PerspectiveFovRH(1, 1, g_fCameraNear, g_fCameraFar, PVRTMat4::OGL, false);

	// Setup view and projection matrices used for when rendering the main scene

	// Caculate the view matrix
	m_mView = PVRTMat4::LookAtRH(PVRTVec3(0, 0, 125), PVRTVec3(0, 0, 0), PVRTVec3(0, 1, 0));

	// Calculate the projection matrix
	m_mProjection = PVRTMat4::PerspectiveFovRH(PVRT_PI/6, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), g_fCameraNear, g_fCameraFar, PVRTMat4::OGL, bRotate);

	return true;
}

/*!****************************************************************************
 @Function		LoadTextures
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course
******************************************************************************/
bool OGLES2RenderToTexture::LoadTextures(CPVRTString* pErrorStr)
{
	// Load the texture that our mask will have
	if(PVRTTextureLoadFromPVR(c_szMaskTex, &m_uiTextureID) != PVR_SUCCESS)
	{
		*pErrorStr += "Failed to open ";
		*pErrorStr += c_szMaskTex;
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
bool OGLES2RenderToTexture::LoadShaders(CPVRTString* pErrorStr)
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

	if (PVRTCreateProgram(&m_ShaderProgram.uiId, m_uiVertShader, m_uiFragShader, g_aszAttribNames, eNumAttribs, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	// Store the location of uniforms for later use
	for (int i = 0; i < eNumUniforms; ++i)
	{
		m_ShaderProgram.auiLoc[i] = glGetUniformLocation(m_ShaderProgram.uiId, g_aszUniformNames[i]);
	}

	// Set the sampler2D variable to the first texture unit
	glUniform1i(glGetUniformLocation(m_ShaderProgram.uiId, "sTexture"), 0);

	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
bool OGLES2RenderToTexture::LoadVbos(CPVRTString* pErrorStr)
{
	if(m_Scene.nNumMesh == 0) // If there are no VBO to create return
		return true;

	if(!m_Scene.pMesh[0].pInterleaved)
	{
		*pErrorStr = "ERROR: RenderToTexture requires the pod data to be interleaved. Please re-export with the interleaved option enabled.";
		return false;
	}

	if(!m_puiVbo)
		m_puiVbo = new GLuint[m_Scene.nNumMesh];

	if(!m_puiIndexVbo)
		m_puiIndexVbo = new GLuint[m_Scene.nNumMesh];

	/*
		Load vertex data of all meshes in the scene into VBOs

		The meshes have been exported with the "Interleave Vectors" option,
		so all data is interleaved in the buffer at pMesh->pInterleaved.
		Interleaving data improves the memory access pattern and cache efficiency,
		thus it can be read faster by the hardware.
	*/

	glGenBuffers(m_Scene.nNumMesh, m_puiVbo);

	for(unsigned int i = 0; i < m_Scene.nNumMesh; ++i)
	{
		// Load vertex data into buffer object
		SPODMesh& Mesh = m_Scene.pMesh[i];
		unsigned int uiSize = Mesh.nNumVertex * Mesh.sVertex.nStride;

		glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[i]);
		glBufferData(GL_ARRAY_BUFFER, uiSize, Mesh.pInterleaved, GL_STATIC_DRAW);

		// Load index data into buffer object if available
		m_puiIndexVbo[i] = 0;

		if(Mesh.sFaces.pData)
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
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES2RenderToTexture::ReleaseView()
{
	// Delete the texture
	glDeleteTextures(1, &m_uiTextureID);
	glDeleteTextures(1, &m_uiTextureToRenderTo);
	glDeleteTextures(1, &m_uiTextureToRenderToMultisampled);

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

	// Delete program and shader objects
	glDeleteProgram(m_ShaderProgram.uiId);
	glDeleteShader(m_uiVertShader);
	glDeleteShader(m_uiFragShader);

	// Delete frame buffer objects
	glDeleteFramebuffers(1, &m_uFBO);
	glDeleteFramebuffers(1, &m_uFBOMultisampled);

	// Delete our depth buffer
	glDeleteRenderbuffers(1, &m_uDepthBuffer);
	glDeleteRenderbuffers(1, &m_uDepthBufferMultisampled);

	return true;
}

/*!****************************************************************************
 @Function		CreateFBO
 @Return		bool		true if no error occured
 @Description	Attempts to create our FBO.
******************************************************************************/
bool OGLES2RenderToTexture::CreateFBO()
{
	// Find the largest square power of two texture that fits into the viewport
	m_i32TexSize = 1;
	int iSize = PVRT_MIN(PVRShellGet(prefWidth), PVRShellGet(prefHeight));
	while (m_i32TexSize * 2 < iSize) m_i32TexSize *= 2;

	// Get the currently bound frame buffer object. On most platforms this just gives 0.
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_i32OriginalFbo);

	// Generate and bind a render buffer which will become a depth buffer shared between our two FBOs
	glGenRenderbuffers(1, &m_uDepthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_uDepthBuffer);

	/*
		Currently it is unknown to GL that we want our new render buffer to be a depth buffer.
		glRenderbufferStorage will fix this and in this case will allocate a depth buffer
		m_i32TexSize by m_i32TexSize.
	*/

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_i32TexSize, m_i32TexSize);

	// Create a texture for rendering to
	glGenTextures(1, &m_uiTextureToRenderTo);
	glBindTexture(GL_TEXTURE_2D, m_uiTextureToRenderTo);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_i32TexSize, m_i32TexSize, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 0);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create the object that will allow us to render to the aforementioned texture
	glGenFramebuffers(1, &m_uFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_uFBO);

	// Attach the texture to the FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uiTextureToRenderTo, 0);

	// Attach the depth buffer we created earlier to our FBO.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_uDepthBuffer);

	// Check that our FBO creation was successful
	GLuint uStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if(uStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to initialise FBO");
		return false;
	}

	// Clear the colour and depth buffers for the FBO / PBuffer surface
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Unbind the frame buffer object so rendering returns back to the backbuffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_i32OriginalFbo);

	return true;
}

/*!****************************************************************************
 @Function		CreateMultisampledFBO
 @Return		bool		true if no error occured
 @Description	Attempts to create a multisampled FBO.
******************************************************************************/
bool OGLES2RenderToTexture::CreateMultisampledFBO()
{
	// Figure out if the platform supports either EXT or IMG extension
	m_eMultisampleMode = eMultisampleExtension_IMG;
	if(m_Extensions.glFramebufferTexture2DMultisampleEXT && m_Extensions.glRenderbufferStorageMultisampleEXT)
	{
		m_eMultisampleMode = eMultisampleExtension_EXT;
	}
	
	// Query the max amount of samples that are supported, we are going to use the max
	GLint samples;
	if(m_eMultisampleMode == eMultisampleExtension_EXT)
		glGetIntegerv(GL_MAX_SAMPLES_EXT, &samples);
	else
		glGetIntegerv(GL_MAX_SAMPLES_IMG, &samples);

	// Generate and bind a render buffer which will become a multisampled depth buffer shared between our two FBOs
	glGenRenderbuffers(1, &m_uDepthBufferMultisampled);
	glBindRenderbuffer(GL_RENDERBUFFER, m_uDepthBufferMultisampled);
	if(m_eMultisampleMode == eMultisampleExtension_EXT)
		m_Extensions.glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT16, m_i32TexSize, m_i32TexSize);
	else
		m_Extensions.glRenderbufferStorageMultisampleIMG(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT16, m_i32TexSize, m_i32TexSize);
	
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// Create a texture for rendering to
	glGenTextures(1, &m_uiTextureToRenderToMultisampled);
	glBindTexture(GL_TEXTURE_2D, m_uiTextureToRenderToMultisampled);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_i32TexSize, m_i32TexSize, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Create the object that will allow us to render to the aforementioned texture
	glGenFramebuffers(1, &m_uFBOMultisampled);
	glBindFramebuffer(GL_FRAMEBUFFER, m_uFBOMultisampled);

	// Attach the depth buffer we created earlier to our FBO.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_uDepthBufferMultisampled);

	// Attach the texture to the FBO
	if(m_eMultisampleMode == eMultisampleExtension_EXT)
		m_Extensions.glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uiTextureToRenderToMultisampled, 0, samples);
	else
		m_Extensions.glFramebufferTexture2DMultisampleIMG(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uiTextureToRenderToMultisampled, 0, samples);

	// Check that our FBO creation was successful
	GLuint uStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if(uStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		PVRShellOutputDebug("ERROR: Failed to initialise FBO");
		return false;
	}

	// Clear the colour and depth buffers for the FBO / PBuffer surface
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Unbind the frame buffer object so rendering returns back to the backbuffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_i32OriginalFbo);

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
bool OGLES2RenderToTexture::RenderScene()
{
	if (PVRShellIsKeyPressed(PVRShellKeyNameLEFT))
		if (m_bMultisampledSupported)
			m_bUseMultisampled = !m_bUseMultisampled;

	// Enable the vertex attribute arrays
	glEnableVertexAttribArray(eVERTEX_ARRAY);
	glEnableVertexAttribArray(eNORMAL_ARRAY);
	glEnableVertexAttribArray(eTEXCOORD_ARRAY);

	// Use shader program
	glUseProgram(m_ShaderProgram.uiId);

	// Setup the lighting direction

	// Reads the light direction from the scene.
	PVRTVec4 vLightDirection;
	PVRTVec3 vPos;

	vLightDirection = m_Scene.GetLightDirection(0);

	// Update out angle used for rotating the mask
	PVRTMat4 mWorld, mMVP;
	PVRTVec4 vLightDir;
	PVRTVec3 vLightDirModel;

	m_fAngleY += (2*PVRT_PI/60)/7;

	// Render to our texture
	{
		// Bind our FBO
		if (m_bUseMultisampled)
			glBindFramebuffer(GL_FRAMEBUFFER, m_uFBOMultisampled);
		else
			glBindFramebuffer(GL_FRAMEBUFFER, m_uFBO);

		// Setup the Viewport to the dimensions of the texture
		glViewport(0, 0, m_i32TexSize, m_i32TexSize);

		// Set the colour to clear our texture to
		glClearColor(0.8f, 1.0f, 0.6f, 1.0f);

		// Clear the colour and depth buffer of our FBO / PBuffer surface
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Render our objects as we usually would
		mWorld = PVRTMat4::RotationY(m_fAngleY);

		// Pass the light direction in model space to the shader
		vLightDir = mWorld.inverse() * vLightDirection;

		vLightDirModel = PVRTVec3(&vLightDir.x);
		vLightDirModel.normalize();

		glUniform3fv(m_ShaderProgram.auiLoc[eLightDirection], 1, &vLightDirModel.x);

		// Set the model-view-projection matrix
		mMVP = m_mR2TProjection * m_mR2TView * mWorld;
		glUniformMatrix4fv(m_ShaderProgram.auiLoc[eMVPMatrix], 1, GL_FALSE, mMVP.f);

		// Bind the mask's texture
		glBindTexture(GL_TEXTURE_2D, m_uiTextureID);

		// Draw our mask
		DrawMesh(m_Scene.pNode[0].nIdx);

		if(m_bDiscard) // Was GL_EXT_discard_framebuffer supported?
		{
			/*
				Give the drivers a hint that we don't want the depth and stencil information stored for future use.

				Note: This training course doesn't have any stencil information so the STENCIL_ATTACHMENT enum
				is used for demonstrations purposes only and will be ignored by the driver.
			*/
			const GLenum attachments[] = { GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT };
			m_Extensions.glDiscardFramebufferEXT(GL_FRAMEBUFFER, 2, attachments);
		}

		// We are done with rendering to our FBO so switch back to the back buffer.
		glBindFramebuffer(GL_FRAMEBUFFER, m_i32OriginalFbo);
	}

	// Set the clear colour
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	// Clear the colour and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Setup the Viewport to the dimensions of the screen
	glViewport(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	// Get the node model matrix
	mWorld = m_Scene.GetWorldMatrix(m_Scene.pNode[1]);

	// Pass the light direction in model space to the shader
	vLightDir = mWorld.inverse() * vLightDirection;

	vLightDirModel = PVRTVec3(&vLightDir.x);
	vLightDirModel.normalize();

	glUniform3fv(m_ShaderProgram.auiLoc[eLightDirection], 1, &vLightDirModel.x);

	// Set the model-view-projection matrix
	mMVP = m_mProjection * m_mView * mWorld ;
	glUniformMatrix4fv(m_ShaderProgram.auiLoc[eMVPMatrix], 1, GL_FALSE, mMVP.f);

	// Bind our texture that we have rendered to
	if (m_bUseMultisampled)
		glBindTexture(GL_TEXTURE_2D, m_uiTextureToRenderToMultisampled);
	else
		glBindTexture(GL_TEXTURE_2D, m_uiTextureToRenderTo);

	// Draw our textured cube
	DrawMesh(m_Scene.pNode[1].nIdx);

	// Safely disable the vertex attribute arrays
	glDisableVertexAttribArray(eVERTEX_ARRAY);
	glDisableVertexAttribArray(eNORMAL_ARRAY);
	glDisableVertexAttribArray(eTEXCOORD_ARRAY);

	// Unbind our VBOs
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Display the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("RenderToTexture", g_pszDesc[m_eMultisampleMode], ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			mesh		The mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the meterial prepared.
******************************************************************************/
void OGLES2RenderToTexture::DrawMesh(unsigned int ui32MeshID)
{
	SPODMesh& Mesh = m_Scene.pMesh[ui32MeshID];

	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[ui32MeshID]);
	// bind the index buffer, won't hurt if the handle is 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[ui32MeshID]);

	// Set the vertex attribute offsets
	glVertexAttribPointer(eVERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, Mesh.sVertex.nStride, Mesh.sVertex.pData);
	glVertexAttribPointer(eNORMAL_ARRAY, 3, GL_FLOAT, GL_FALSE, Mesh.sNormals.nStride, Mesh.sNormals.pData);
	glVertexAttribPointer(eTEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, Mesh.psUVW[0].nStride, Mesh.psUVW[0].pData);

	// Indexed Triangle list
	glDrawElements(GL_TRIANGLES, Mesh.nNumFaces*3, GL_UNSIGNED_SHORT, 0);
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
	return new OGLES2RenderToTexture();
}

/******************************************************************************
 End of file (OGLES2RenderToTexture.cpp)
******************************************************************************/

