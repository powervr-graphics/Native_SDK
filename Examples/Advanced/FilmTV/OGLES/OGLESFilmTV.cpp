/******************************************************************************

 @File         OGLESFilmTV.cpp

 @Title        Introducing the POD 3D file format

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows how to load POD files and play the animation with basic
               lighting

******************************************************************************/
#include "PVRShell.h"
#include "OGLESTools.h"

/******************************************************************************
 Defines
******************************************************************************/
#if !defined(EGL_VERSION_1_0) // Do we have access to EGL?
#if !defined(EGL_NOT_PRESENT)
#define EGL_NOT_PRESENT 1
#endif
#endif

/******************************************************************************
 Consts
******************************************************************************/
// Camera constants. Used for making the projection matrix
const float g_fCameraNear = 1.0f;
const float g_fCameraFar  = 150.0f;

// The camera to use from the pod file
const int g_ui32Camera = 0;

const float g_ui32CameraLoopSpeed = 10.0f;
const unsigned int g_ui32CameraMesh = 9;

/******************************************************************************
 Content file names
******************************************************************************/

// POD scene files
const char c_szSceneFile[] = "FilmTVScene.pod";

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLESFilmTV : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// 3D Model
	CPVRTModelPOD	m_Scene;

	// OpenGL handles for textures and VBOs
	GLuint*	m_puiVbo;
	GLuint*	m_puiIndexVbo;
	GLuint*	m_puiTextureIDs;

	// App Variables
	int		m_i32TexSize;

	GLuint	m_uiTexture[2];
	int		m_uiTVScreen;
	int		m_i32Frame;

	PVRTMat4 m_MiniCamView;
	PVRTMat4 m_MiniCamProj;
	PVRTMat4 m_View;
	PVRTMat4 m_Projection;

	// Render contexts, etc
	GLint	   m_CurrentFBO;

#if !defined(EGL_NOT_PRESENT)
	EGLDisplay m_CurrentDisplay;
	EGLContext m_CurrentContext;
	EGLSurface m_CurrentSurface;

	// We require 2 PBuffer surfaces.
	EGLSurface m_PBufferSurface[2];
#endif

	// We require 2 FBOs, which will require a depth buffer
	GLuint m_uFBO[2];
	GLuint m_uDepthBuffer;

	unsigned int m_ui32CurrentBuffer;
	unsigned int m_ui32PreviousBuffer;

	enum
	{
		eNone,
#if !defined(EGL_NOT_PRESENT)
		ePBuffer,
#endif
		eFBO
	} m_eR2TType;

	CPVRTglesExt m_Extensions;

	// Discard the frame buffer attachments
	bool m_bDiscard;

	// Start time
	unsigned long m_ulStartTime;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	bool LoadTextures(CPVRTString* pErrorStr);
	bool LoadVbos(CPVRTString* pErrorStr);

	void DrawPODScene(PVRTMat4 &mViewProjection, bool bDrawCamera);
	void DrawMesh(int i32MeshIndex);
	void CalcMiniCameraView();
	bool CreateFBOsorPBuffers();
#if !defined(EGL_NOT_PRESENT)
	EGLConfig SelectEGLConfig();
#endif
	bool StartRenderToTexture();
	bool EndRenderToTexture();

	OGLESFilmTV() : m_puiVbo(0),
					m_puiIndexVbo(0),
					m_puiTextureIDs(0),
					m_i32TexSize(1),
					m_uiTVScreen(-1),
					m_i32Frame(0),
					m_ui32CurrentBuffer(0),
					m_ui32PreviousBuffer(1),
					m_bDiscard(false)
	{
	}
};

/*!****************************************************************************
 @Function		LoadTextures
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course
******************************************************************************/
bool OGLESFilmTV::LoadTextures(CPVRTString* pErrorStr)
{
	/*
		Loads the textures.
		For a more detailed explanation, see Texturing and IntroducingPVRTools
	*/

	/*
		Initialises an array to lookup the textures
		for each material in the scene.
	*/
	m_puiTextureIDs = new GLuint[m_Scene.nNumMaterial];

	if(!m_puiTextureIDs)
	{
		*pErrorStr = "ERROR: Insufficient memory.";
		return false;
	}

	for(unsigned int i = 0; i < m_Scene.nNumMaterial; ++i)
	{
		m_puiTextureIDs[i] = 0;
		SPODMaterial* pMaterial = &m_Scene.pMaterial[i];

		if(pMaterial->nIdxTexDiffuse != -1)
		{
			/*
				Using the tools function PVRTTextureLoadFromPVR load the textures required by the pod file.

				Note: This function only loads .pvr files. You can set the textures in 3D Studio Max to .pvr
				files using the PVRTexTool plug-in for max. Alternatively, the pod material properties can be
				modified in PVRShaman.
			*/

			CPVRTString sTextureName = m_Scene.pTexture[pMaterial->nIdxTexDiffuse].pszName;

			if(sTextureName == "TV.pvr")
				m_uiTVScreen = i;

			if(PVRTTextureLoadFromPVR(sTextureName.c_str(), &m_puiTextureIDs[i]) != PVR_SUCCESS)
			{
				*pErrorStr = "ERROR: Failed to load " + sTextureName + ".";

				// Check to see if we're trying to load .pvr or not
				CPVRTString sFileExtension = PVRTStringGetFileExtension(sTextureName);

				if(sFileExtension.toLower() != "pvr")
					*pErrorStr += "Note: FilmTV can only load .pvr files.";

				return false;
			}
		}
	}

	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
bool OGLESFilmTV::LoadVbos(CPVRTString* pErrorStr)
{
	if(!m_Scene.pMesh[0].pInterleaved)
	{
		*pErrorStr = "ERROR: FilmTV requires the pod data to be interleaved. Please re-export with the interleaved option enabled.";
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
 @Function		InitApplication
 @Return		bool		true if no error occured
 @Description	Code in InitApplication() will be called by PVRShell once per
				run, before the rendering context is created.
				Used to initialize variables that are not dependant on it
				(e.g. external modules, loading meshes, etc.)
				If the rendering context is lost, InitApplication() will
				not be called again.
******************************************************************************/
bool OGLESFilmTV::InitApplication()
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
		CPVRTString ErrorStr = "ERROR: Couldn't load '" + CPVRTString(c_szSceneFile) + "'.";
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// The cameras are stored in the file. We check it contains at least one.
	if(m_Scene.nNumCamera == 0)
	{
		PVRShellSet(prefExitMessage, "ERROR: The scene does not contain a camera. Please add one and re-export.\n");
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
bool OGLESFilmTV::QuitApplication()
{
	// Free the memory allocated for the scene
	m_Scene.Destroy();

	delete[] m_puiVbo;
	delete[] m_puiIndexVbo;

    return true;
}

/*!****************************************************************************
 @Function		CreateFBOsorPBuffers
 @Return		bool		true if no error occured
 @Description	Attempts to create our FBOs if supported or PBuffers if they
				are not.
******************************************************************************/
bool OGLESFilmTV::CreateFBOsorPBuffers()
{
#if !defined(EGL_NOT_PRESENT)
	EGLConfig eglConfig = 0;
	EGLint list[9];
#endif

	// Find the largest square power of two texture that fits into the viewport
	m_i32TexSize = 1;
	int iSize = PVRT_MIN(PVRShellGet(prefWidth), PVRShellGet(prefHeight));
	while (m_i32TexSize * 2 < iSize) m_i32TexSize *= 2;

	// Check for FBO extension
	if(CPVRTglesExt::IsGLExtensionSupported("GL_OES_framebuffer_object"))
	{
		// FBOs are present so we're going to use them
		m_eR2TType = eFBO;

		// Load the extensions as they are required
		m_Extensions.LoadExtensions();

		// Check to see if the GL_EXT_discard_framebuffer extension is supported by seeing if 
		// CPVRTglesExt has a valid pointer for glDiscardFramebufferEXT
		m_bDiscard = m_Extensions.glDiscardFramebufferEXT != 0;

		// Get the currently bound frame buffer object. On most platforms this just gives 0.
		glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &m_CurrentFBO);

		// Generate and bind a render buffer which will become a depth buffer shared between our two FBOs
		m_Extensions.glGenRenderbuffersOES(1, &m_uDepthBuffer);
		m_Extensions.glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_uDepthBuffer);

		/*
			Currently it is unknown to GL that we want our new render buffer to be a depth buffer.
			glRenderbufferStorage will fix this and in this case will allocate a depth buffer of
			m_i32TexSize by m_i32TexSize.
		*/
		m_Extensions.glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, m_i32TexSize, m_i32TexSize);
	}
#if !defined(EGL_NOT_PRESENT)
	else
	{
		// FBOs aren't present so we're going to use PBuffers
		m_eR2TType = ePBuffer;

		// Set up a configuration and attribute list used for creating a PBuffer surface.
		eglConfig = SelectEGLConfig();

		// First we specify the width of the surface...
		list[0] = EGL_WIDTH;
		list[1] = m_i32TexSize;
		// ...then the height of the surface...
		list[2] = EGL_HEIGHT;
		list[3] = m_i32TexSize;
		/* ... then we specifiy the target for the texture
			that will be created when the pbuffer is created...*/
		list[4] = EGL_TEXTURE_TARGET;
		list[5] = EGL_TEXTURE_2D;
		/*..then the format of the texture that will be created
			when the pBuffer is bound to a texture...*/
		list[6] = EGL_TEXTURE_FORMAT;
		list[7] = EGL_TEXTURE_RGB;
		// The final thing is EGL_NONE which signifies the end.
		list[8] = EGL_NONE;

		/*
			Get the current display, context and surface so we can switch between the
			PBuffer surface and the main render surface.
		*/

		m_CurrentDisplay = eglGetCurrentDisplay();
		m_CurrentContext = eglGetCurrentContext();
		m_CurrentSurface = eglGetCurrentSurface(EGL_DRAW);
	}
#else
	else
	{
		PVRShellOutputDebug("ERROR: Required extension \"GL_OES_framebuffer_object\" not present.\n");
		return false;
	}
#endif

	for(int i = 0; i < 2; ++i)
	{
		// Create texture for rendering to
		glGenTextures(1, &m_uiTexture[i]);
		glBindTexture(GL_TEXTURE_2D, m_uiTexture[i]);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_i32TexSize, m_i32TexSize, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 0);

		// Create the surface or object that will allow us to render to the aforementioned texture
		switch(m_eR2TType)
		{
			case eFBO: // Create FBO
			{
				m_Extensions.glGenFramebuffersOES(1, &m_uFBO[i]);
				m_Extensions.glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_uFBO[i]);

				// Attach the texture to the FBO
				m_Extensions.glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, m_uiTexture[i], 0);

				// Attach the depth buffer we created earlier to our FBO.
				m_Extensions.glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, m_uDepthBuffer);

				// Check that our FBO creation was successful
				GLuint uStatus = m_Extensions.glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);

				if(uStatus != GL_FRAMEBUFFER_COMPLETE_OES)
				{
					PVRShellOutputDebug("ERROR: Failed to initialise FBO\n");
					return false;
				}

				// Unbind the FBO now we are done with it
				m_Extensions.glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_CurrentFBO);
			}
			break;
#if !defined(EGL_NOT_PRESENT)
			case ePBuffer: // Create a PBuffer surface
			{
				m_PBufferSurface[i] = eglCreatePbufferSurface(m_CurrentDisplay, eglConfig, list);

				// If we don't have both of the surfaces return false.
				if(m_PBufferSurface[i] == EGL_NO_SURFACE)
				{
					PVRShellOutputDebug("ERROR: Failed to create pbuffer.\n");
					return false;
				}

				// Switch the render target to the pBuffer
				if(!eglMakeCurrent(m_CurrentDisplay, m_PBufferSurface[i], m_PBufferSurface[i], m_CurrentContext))
				{
					PVRShellOutputDebug("ERROR: Unable to make the pbuffer context current.\n");
					return false;
				}

				// Bind the texture to this surface
				eglBindTexImage(m_CurrentDisplay, m_PBufferSurface[i], EGL_BACK_BUFFER);
			}
			break;
#endif
			default: {}
		}

		// Clear the colour buffer for this FBO
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

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
bool OGLESFilmTV::InitView()
{
	CPVRTString ErrorStr;

	//	Initialise VBO data
	if(!LoadVbos(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	//	Load textures
	if(!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	//	Initialise Print3D
    bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	// Set OpenGL ES render states needed for this demo

	// Enable backface culling and depth test
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);

	// Set the clear colour
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Create FBOs or PBuffers
	if(!CreateFBOsorPBuffers())
		m_eR2TType = eNone;

	// Enables texturing
	glEnable(GL_TEXTURE_2D);

	// Setup the main camera
	PVRTVec3	vFrom, vTo(0.0f), vUp(0.0f, 1.0f, 0.0f);
	float fFOV;

	// Camera nodes are after the mesh and light nodes in the array
	int i32CamID = m_Scene.pNode[m_Scene.nNumMeshNode + m_Scene.nNumLight + g_ui32Camera].nIdx;

	// Get the camera position, target and field of view (fov)
	if(m_Scene.pCamera[i32CamID].nIdxTarget != -1) // Does the camera have a target?
		fFOV = m_Scene.GetCameraPos( vFrom, vTo, g_ui32Camera); // vTo is taken from the target node
	else
		fFOV = m_Scene.GetCamera( vFrom, vTo, vUp, g_ui32Camera); // vTo is calculated from the rotation

	m_View = PVRTMat4::LookAtRH(vFrom, vTo, vUp);

	// Calculate the projection matrix
	m_Projection = PVRTMat4::PerspectiveFovRH(fFOV, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), g_fCameraNear, g_fCameraFar, PVRTMat4::OGL, bRotate);

	// Store initial time
	m_ulStartTime = PVRShellGetTime();

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLESFilmTV::ReleaseView()
{
	switch(m_eR2TType)
	{
		case eFBO:
			// Delete frame buffer objects
			m_Extensions.glDeleteFramebuffersOES(2, &m_uFBO[0]);

			// Delete our depth buffer
			m_Extensions.glDeleteRenderbuffersOES(1, &m_uDepthBuffer);
		break;
#if !defined(EGL_NOT_PRESENT)
		case ePBuffer:
			// Destroy the surfaces we created
			eglDestroySurface(m_CurrentDisplay,	m_PBufferSurface[0]);
			eglDestroySurface(m_CurrentDisplay,	m_PBufferSurface[1]);
		break;
#endif
		default: { }
	}

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

	// Deletes the textures
	glDeleteTextures(m_Scene.nNumMaterial, &m_puiTextureIDs[0]);
	glDeleteTextures(2, &m_uiTexture[0]);

	// Frees the texture lookup array
	delete[] m_puiTextureIDs;
	m_puiTextureIDs = 0;

	return true;
}

/*!****************************************************************************
 @Function		CalcMiniCameraView
 @Description	Calculate the mini camera view and perspective matrix
******************************************************************************/
void OGLESFilmTV::CalcMiniCameraView()
{
	float fValue = (PVRShellGetTime() - m_ulStartTime) * 0.001f * 2.0f * PVRT_PIf;

	float fZ = 1.0f + (2.40f * (float)sin(fValue * 1.0f / g_ui32CameraLoopSpeed));
	float fX = 0.50f * (float)cos(fValue * 2.0f / g_ui32CameraLoopSpeed);
	float fCamRot = 0.16f * (float)sin(fValue * 1.0f / g_ui32CameraLoopSpeed) - 0.17f;

	m_MiniCamView  = PVRTMat4::RotationX(((float)atan2(fX, 10.0f)));
	m_MiniCamView *= PVRTMat4::RotationY(fCamRot);
	m_MiniCamView *= PVRTMat4::RotationZ(((float)atan2(fZ, 10.0f)));

	// Setup the mini camera's view projection matrix
	m_MiniCamProj = PVRTMat4::PerspectiveFovRH(70.0f * (PVRT_PIf / 180.0f), 1, g_fCameraNear, g_fCameraFar, PVRTMat4::OGL, false);
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
bool OGLESFilmTV::RenderScene()
{
	// Enable the vertex attribute arrays
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	// Render everything from the mini-camera's point of view

	// Setup the Viewport to the dimensions of the texture
	glViewport(0, 0, m_i32TexSize, m_i32TexSize);

	CalcMiniCameraView();

	if(StartRenderToTexture())
	{
		// Loads the projection matrix for the mini-cam
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(m_MiniCamProj.f);
		glMatrixMode(GL_MODELVIEW);

		DrawPODScene(m_MiniCamView, false);

		EndRenderToTexture();
	}

	// Render everything

	// Setup the Viewport to the dimensions of the screen
	glViewport(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	// Loads the projection matrix for the camera
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(m_Projection.f);
	glMatrixMode(GL_MODELVIEW);

	DrawPODScene(m_View, true);

	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	const char *pDescription = "";

	if(m_eR2TType == eFBO)
		pDescription = "Using FBOs";
#if !defined(EGL_NOT_PRESENT)
	else if(m_eR2TType == ePBuffer)
		pDescription = "Using PBuffers";
#endif

	m_Print3D.DisplayDefaultTitle("FilmTV", pDescription, ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	// Swap the buffer that we want to render to
	m_ui32PreviousBuffer = m_ui32CurrentBuffer;
	m_ui32CurrentBuffer = 1 - m_ui32CurrentBuffer;

	++m_i32Frame;
	return true;
}

/*!****************************************************************************
 @Function		StartRenderToTexture
 @Return		bool		true if no error occured
 @Description	Setup the render to texture
******************************************************************************/
bool OGLESFilmTV::StartRenderToTexture()
{
	switch(m_eR2TType)
	{
		case eFBO:
			m_Extensions.glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_uFBO[m_ui32CurrentBuffer]);
		break;
#if !defined(EGL_NOT_PRESENT)
		case ePBuffer:
			// Switch the render target to the pBuffer
			if(!eglMakeCurrent(m_CurrentDisplay, m_PBufferSurface[m_ui32CurrentBuffer], m_PBufferSurface[m_ui32CurrentBuffer], m_CurrentContext))
			{
				PVRShellSet(prefExitMessage, "ERROR: Unable to make the pbuffer context current.\n");
				return false;
			}

			/*
				We no longer need the texture bound to the surface so we release the previous surface from
				all the textures it is bound to.
			*/

			if(!eglReleaseTexImage(m_CurrentDisplay, m_PBufferSurface[m_ui32CurrentBuffer], EGL_BACK_BUFFER))
			{
				PVRShellSet(prefExitMessage, "ERROR: Failed to release m_PBufferSurface.\n");
				return false;
			}
		break;
#endif
		default: {}
	}

	return true;
}

/*!****************************************************************************
 @Function		EndRenderToTexture
 @Return		bool		true if no error occured
 @Description	We have finished rendering to our texture. Switch rendering
				back to the backbuffer.
******************************************************************************/
bool OGLESFilmTV::EndRenderToTexture()
{
	switch(m_eR2TType)
	{
		case eFBO:
			if(m_bDiscard) // Was GL_EXT_discard_framebuffer supported?
			{
				/*
					Give the drivers a hint that we don't want the depth and stencil information stored for future use.

					Note: This training course doesn't have any stencil information so the STENCIL_ATTACHMENT enum 
					is used for demonstrations purposes only and will be ignored by the driver.
				*/
				const GLenum attachments[] = { GL_DEPTH_ATTACHMENT_OES, GL_STENCIL_ATTACHMENT_OES };
				m_Extensions.glDiscardFramebufferEXT(GL_FRAMEBUFFER_OES, 2, attachments);
			}

			m_Extensions.glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_CurrentFBO);
		break;
#if !defined(EGL_NOT_PRESENT)
		case ePBuffer:
			// We now switch back to the backbuffer for rendering.
			if(!eglMakeCurrent(m_CurrentDisplay, m_CurrentSurface, m_CurrentSurface, m_CurrentContext))
			{
				PVRShellOutputDebug("ERROR: Unable to make the main context current.\n");
				return false;
			}

			glBindTexture(GL_TEXTURE_2D, m_uiTexture[m_ui32CurrentBuffer]);

			if(!eglBindTexImage(m_CurrentDisplay, m_PBufferSurface[m_ui32CurrentBuffer], EGL_BACK_BUFFER))
			{
				PVRShellOutputDebug("ERROR: Failed to bind m_PBufferSurface.\n");
				return false;
			}
		break;
#endif
		default: {}
	}

	return true;
}

/*!****************************************************************************
 @Function		DrawPODScene
 @Description	Render the pod scene using the passed-in view matrix
******************************************************************************/
void OGLESFilmTV::DrawPODScene(PVRTMat4 &mView, bool bDrawCamera)
{
	// Clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for(unsigned int i = 0; i < m_Scene.nNumMeshNode; ++i)
	{
		SPODNode& Node = m_Scene.pNode[i];

		// Get the node model matrix
		PVRTMat4 mWorld = m_Scene.GetWorldMatrix(Node);

		if(i == g_ui32CameraMesh)
		{
			if(!bDrawCamera)
				continue;

			// Rotate camera model
			mWorld =  m_MiniCamView.inverse() * mWorld;
		}

		// Multiply the view matrix by the model (mWorld) matrix to get the model-view matrix
		PVRTMat4 mModelView = mView * mWorld;

		glLoadMatrixf(mModelView.f);

		// Load the correct texture using our texture lookup table
		GLuint uiTex = 0;

		if(Node.nIdxMaterial != -1)
		{
			if(Node.nIdxMaterial == m_uiTVScreen && m_i32Frame != 0 && m_eR2TType != eNone)
				uiTex = m_uiTexture[m_ui32PreviousBuffer];
			else
				uiTex = m_puiTextureIDs[Node.nIdxMaterial];
		}

		glBindTexture(GL_TEXTURE_2D, uiTex);

		/*
			Now that the model-view matrix is set and the materials ready,
			call another function to actually draw the mesh.
		*/
		DrawMesh(Node.nIdx);
	}
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			i32MeshIndex		Mesh index of the mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the meterial prepared.
******************************************************************************/
void OGLESFilmTV::DrawMesh(int i32MeshIndex)
{
	SPODMesh& Mesh = m_Scene.pMesh[i32MeshIndex];

	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[i32MeshIndex]);

	// bind the index buffer, won't hurt if the handle is 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[i32MeshIndex]);

	// Set the vertex attribute offsets
	glVertexPointer(Mesh.sVertex.n, GL_FLOAT, Mesh.sVertex.nStride, Mesh.sVertex.pData);
	glNormalPointer(GL_FLOAT, Mesh.sNormals.nStride, Mesh.sNormals.pData);

	if(Mesh.nNumUVW) // Do we have texture co-ordinates?
	{
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(Mesh.psUVW[0].n, GL_FLOAT, Mesh.psUVW[0].nStride, Mesh.psUVW[0].pData);
	}

	if(Mesh.sVtxColours.n) // Do we have vertex colours?
	{
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(Mesh.sVtxColours.n * PVRTModelPODDataTypeComponentCount(Mesh.sVtxColours.eType), GL_UNSIGNED_BYTE, Mesh.sVtxColours.nStride, Mesh.sVtxColours.pData);
	}

	// Draw the Indexed Triangle list
	glDrawElements(GL_TRIANGLES, Mesh.nNumFaces*3, GL_UNSIGNED_SHORT, 0);

	// Disable the color attribute array
	glDisableClientState(GL_COLOR_ARRAY);

	// Disable the uv attribute array
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	// Disable the uv attribute array
	glDisableClientState(GL_NORMAL_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

#if !defined(EGL_NOT_PRESENT)
/*!****************************************************************************
 @Function		SelectEGLConfig
 @Description	Finds an EGL config with required options based on Mode Requested - for PBuffer
******************************************************************************/
EGLConfig OGLESFilmTV::SelectEGLConfig()
{
	EGLConfig EglConfig = 0;
	EGLint i32ConfigID;
	EGLint i32BufferSize;
	EGLint i32SampleBuffers;
	EGLint i32Samples;

	// Get the colour buffer size and the anti-aliasing parameters of the current surface so we can create
	// a PBuffer surface that matches.
	EGLDisplay eglDisplay = eglGetCurrentDisplay();
	eglQueryContext(eglDisplay, eglGetCurrentContext(), EGL_CONFIG_ID, &i32ConfigID);

	eglGetConfigAttrib(eglDisplay, (EGLConfig) (size_t) i32ConfigID, EGL_BUFFER_SIZE,&i32BufferSize);
	eglGetConfigAttrib(eglDisplay, (EGLConfig) (size_t) i32ConfigID, EGL_SAMPLE_BUFFERS,&i32SampleBuffers);
	eglGetConfigAttrib(eglDisplay, (EGLConfig) (size_t) i32ConfigID, EGL_SAMPLES,&i32Samples);

    EGLint i32ConfigNo;

	// Setup the configuration list for our surface.
    EGLint conflist[] =
	{
		EGL_CONFIG_CAVEAT, EGL_NONE,
		/*
			Tell it the minimum size we want for our colour buffer, depth size and
			anti-aliasing settings so eglChooseConfig will choose a config that is
			a good match for our window context so we only need a single context.
		*/
		EGL_BUFFER_SIZE, i32BufferSize,
		EGL_DEPTH_SIZE, 16,

		EGL_SAMPLE_BUFFERS, i32SampleBuffers,
		EGL_SAMPLES, i32Samples,

		// The PBuffer bit is the important part as it shows we want a PBuffer
		EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
		EGL_BIND_TO_TEXTURE_RGB, EGL_TRUE,
		EGL_NONE
	};

	// Find and return the config
    if(!eglChooseConfig(eglDisplay, conflist, &EglConfig, 1, &i32ConfigNo) || i32ConfigNo != 1)
	{
		PVRShellOutputDebug("Error: Failed to find a suitable config.\n");
		return 0;
    }

    return EglConfig;
}
#endif
/*!****************************************************************************
 @Function		NewDemo
 @Return		PVRShell*		The demo supplied by the user
 @Description	This function must be implemented by the user of the shell.
				The user should return its PVRShell object defining the
				behaviour of the application.
******************************************************************************/
PVRShell* NewDemo()
{
	return new OGLESFilmTV();
}

/******************************************************************************
 End of file (OGLESFilmTV.cpp)
******************************************************************************/

