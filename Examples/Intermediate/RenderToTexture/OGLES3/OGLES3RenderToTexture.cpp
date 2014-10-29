/******************************************************************************

 @File         OGLES3RenderToTexture.cpp

 @Title        RenderToTexture

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows how to render to a pBuffer surface or FBO and bind that to a
               texture.

******************************************************************************/

#include "PVRShell.h"
#include "OGLES3Tools.h"

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

/******************************************************************************
 Defines
******************************************************************************/

#define VERTEX_ARRAY    0
#define NORMAL_ARRAY    1
#define TEXCOORD_ARRAY  2

/*****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES3RenderToTexture : public PVRShell
{
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
	
	// Handles for our shaders
	GLuint	m_uiVertShader;
	GLuint	m_uiFragShader;

	// Group shader programs and their uniform locations together
	struct
	{
		GLuint uiId;
		GLint  iMVPMatrixLoc;
		GLint  iLightDirectionLoc;
	}
	m_ShaderProgram;

	// Variables used for the animation
	float	m_fAngleY;

	// App Variables
	int		m_i32TexSize;

	// Render contexts, etc
	GLint	  m_i32OriginalFbo;

	// Handle for our FBO and the depth buffer that it requires
	GLuint m_uiFBO;
	GLuint m_uiDepthBuffer;

	// Handle for our multisampled FBO and the depth buffer that it requires
	GLuint m_uiFBOMultisampled;
	GLuint m_uiDepthBufferMultisampled;
	GLuint m_uiColourBufferMultisampled;	
		
	bool m_bUseMultisampled;

public:
	OGLES3RenderToTexture() : m_puiVbo(0),
							m_puiIndexVbo(0),
							m_uiTextureID(0),
							m_uiTextureToRenderTo(0),							
							m_fAngleY(0),							
							m_i32OriginalFbo(0),
							m_uiFBOMultisampled(0),
						    m_uiDepthBufferMultisampled(0),
							m_uiColourBufferMultisampled(0),
							m_bUseMultisampled(true)
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
bool OGLES3RenderToTexture::InitApplication()
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
bool OGLES3RenderToTexture::QuitApplication()
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
bool OGLES3RenderToTexture::InitView()
{
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
bool OGLES3RenderToTexture::LoadTextures(CPVRTString* pErrorStr)
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
bool OGLES3RenderToTexture::LoadShaders(CPVRTString* pErrorStr)
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

	if (PVRTCreateProgram(&m_ShaderProgram.uiId, m_uiVertShader, m_uiFragShader, NULL, 0, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}
	
	// Store the location of uniforms for later use	
	m_ShaderProgram.iMVPMatrixLoc = glGetUniformLocation(m_ShaderProgram.uiId, "MVPMatrix");
	m_ShaderProgram.iLightDirectionLoc = glGetUniformLocation(m_ShaderProgram.uiId, "LightDirection");

	// Set the sampler2D variable to the first texture unit
	glUniform1i(glGetUniformLocation(m_ShaderProgram.uiId, "sTexture"), 0);

	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
bool OGLES3RenderToTexture::LoadVbos(CPVRTString* pErrorStr)
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
bool OGLES3RenderToTexture::ReleaseView()
{
	// Delete the texture
	glDeleteTextures(1, &m_uiTextureID);
	glDeleteTextures(1, &m_uiTextureToRenderTo);
	
	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

	// Delete program and shader objects
	glDeleteProgram(m_ShaderProgram.uiId);
	glDeleteShader(m_uiVertShader);
	glDeleteShader(m_uiFragShader);


	// Tidy up the FBOs and renderbuffers

	// Delete frame buffer objects
	glDeleteFramebuffers(1, &m_uiFBO);
	glDeleteFramebuffers(1, &m_uiFBOMultisampled);

	// Delete our depth buffer
	glDeleteRenderbuffers(1, &m_uiDepthBuffer);
	glDeleteRenderbuffers(1, &m_uiDepthBufferMultisampled);
	glDeleteRenderbuffers(1, &m_uiColourBufferMultisampled);

	return true;
}

/*!****************************************************************************
 @Function		CreateFBO
 @Return		bool		true if no error occured
 @Description	Attempts to create our FBO.
******************************************************************************/
bool OGLES3RenderToTexture::CreateFBO()
{
	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };

	// Query the max amount of samples that are supported, we are going to use the max
	GLint samples;
	glGetIntegerv(GL_MAX_SAMPLES, &samples);

	// Find the largest square power of two texture that fits into the viewport
	m_i32TexSize = 1;
	int iSize = PVRT_MIN(PVRShellGet(prefWidth), PVRShellGet(prefHeight));
	while (m_i32TexSize * 2 < iSize) m_i32TexSize *= 2;

	// Get the currently bound frame buffer object. On most platforms this just gives 0.
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_i32OriginalFbo);

	// Generate and bind a render buffer which will become a depth buffer shared between our two FBOs
	glGenRenderbuffers(1, &m_uiDepthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_uiDepthBuffer);

	/*
		Currently it is unknown to GL that we want our new render buffer to be a depth buffer.
		glRenderbufferStorage will fix this and in this case will allocate a depth buffer
		m_i32TexSize by m_i32TexSize.
	*/

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_i32TexSize, m_i32TexSize);

	// Create a texture for rendering to
	glGenTextures(1, &m_uiTextureToRenderTo);
	glBindTexture(GL_TEXTURE_2D, m_uiTextureToRenderTo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_i32TexSize, m_i32TexSize, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create the object that will allow us to render to the aforementioned texture
	glGenFramebuffers(1, &m_uiFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_uiFBO);
	
	glDrawBuffers(1, drawBuffers);
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	// Attach the texture to the FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uiTextureToRenderTo, 0);

	// Attach the depth buffer we created earlier to our FBO.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_uiDepthBuffer);

	// Check that our FBO creation was successful
	GLuint uStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(uStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to initialise FBO");
		return false;
	}

	/*
	  Create and initialize the multi-sampled FBO.
	 */

	// Create the object that will allow us to render to the aforementioned texture
	glGenFramebuffers(1, &m_uiFBOMultisampled);
	glBindFramebuffer(GL_FRAMEBUFFER, m_uiFBOMultisampled);
	
	glDrawBuffers(1, drawBuffers);
	glReadBuffer(GL_COLOR_ATTACHMENT0);			

	// Generate and bind a render buffer which will become a multisampled depth buffer shared between our two FBOs
	glGenRenderbuffers(1, &m_uiDepthBufferMultisampled);
	glBindRenderbuffer(GL_RENDERBUFFER, m_uiDepthBufferMultisampled);	
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT24, m_i32TexSize, m_i32TexSize);

	glGenRenderbuffers(1, &m_uiColourBufferMultisampled);
	glBindRenderbuffer(GL_RENDERBUFFER, m_uiColourBufferMultisampled);	
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGB8, m_i32TexSize, m_i32TexSize);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// Attach the multisampled depth buffer we created earlier to our FBO.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_uiDepthBufferMultisampled);

	// Attach the multisampled colour renderbuffer to the FBO
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_uiColourBufferMultisampled);

	// Check that our FBO creation was successful
	uStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(uStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to initialise multisampled FBO");
		return false;
	}		

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
bool OGLES3RenderToTexture::RenderScene()
{
	if (PVRShellIsKeyPressed(PVRShellKeyNameSELECT))
		m_bUseMultisampled = !m_bUseMultisampled;

	// Enable the vertex attribute arrays
	glEnableVertexAttribArray(VERTEX_ARRAY);
	glEnableVertexAttribArray(NORMAL_ARRAY);
	glEnableVertexAttribArray(TEXCOORD_ARRAY);

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
			glBindFramebuffer(GL_FRAMEBUFFER, m_uiFBOMultisampled);
		else
			glBindFramebuffer(GL_FRAMEBUFFER, m_uiFBO);

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

		glUniform3fv(m_ShaderProgram.iLightDirectionLoc, 1, &vLightDirModel.x);

		// Set the model-view-projection matrix
		mMVP = m_mR2TProjection * m_mR2TView * mWorld;
		glUniformMatrix4fv(m_ShaderProgram.iMVPMatrixLoc, 1, GL_FALSE, mMVP.f);
		
		// Bind the mask's texture
		glBindTexture(GL_TEXTURE_2D, m_uiTextureID);

		// Draw our mask
		DrawMesh(m_Scene.pNode[0].nIdx);
		
		//	Give the drivers a hint that we don't want the depth and stencil information stored for future use.
		const GLenum attachments[] = { GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT };
		glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments);	
		
		if (m_bUseMultisampled)
		{
			// Blit and resolve the multisampled render buffer to the non-multisampled FBO
			glBindFramebuffer(GL_READ_FRAMEBUFFER, m_uiFBOMultisampled);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_uiFBO);
			glBlitFramebuffer(0, 0, m_i32TexSize, m_i32TexSize, 0, 0, m_i32TexSize, m_i32TexSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
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

	glUniform3fv(m_ShaderProgram.iLightDirectionLoc, 1, &vLightDirModel.x);

	// Set the model-view-projection matrix
	mMVP = m_mProjection * m_mView * mWorld ;
	glUniformMatrix4fv(m_ShaderProgram.iMVPMatrixLoc, 1, GL_FALSE, mMVP.f);

	// Bind our texture that we have rendered to
	glBindTexture(GL_TEXTURE_2D, m_uiTextureToRenderTo);

	// Draw our textured cube
	DrawMesh(m_Scene.pNode[1].nIdx);

	// Safely disable the vertex attribute arrays
	glDisableVertexAttribArray(VERTEX_ARRAY);
	glDisableVertexAttribArray(NORMAL_ARRAY);
	glDisableVertexAttribArray(TEXCOORD_ARRAY);

	// Unbind our VBOs
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Display the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("RenderToTexture", (m_bUseMultisampled ? "Using multisampled FBOs" : "Using FBOs"), ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			mesh		The mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the meterial prepared.
******************************************************************************/
void OGLES3RenderToTexture::DrawMesh(unsigned int ui32MeshID)
{
	SPODMesh& Mesh = m_Scene.pMesh[ui32MeshID];

	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[ui32MeshID]);
	// bind the index buffer, won't hurt if the handle is 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[ui32MeshID]);

	// Set the vertex attribute offsets
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, Mesh.sVertex.nStride, Mesh.sVertex.pData);
	glVertexAttribPointer(NORMAL_ARRAY, 3, GL_FLOAT, GL_FALSE, Mesh.sNormals.nStride, Mesh.sNormals.pData);
	glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, Mesh.psUVW[0].nStride, Mesh.psUVW[0].pData);

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
	return new OGLES3RenderToTexture();
}

/******************************************************************************
 End of file (OGLES3RenderToTexture.cpp)
******************************************************************************/

