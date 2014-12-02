/******************************************************************************

 @File         OGLESRenderToTexture.cpp

 @Title        RenderToTexture

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows how to render to a pBuffer surface or FBO and bind that to a
               texture.

******************************************************************************/

#include "PVRShell.h"
#include "OGLESTools.h"

/******************************************************************************
 Content file names
******************************************************************************/

// Scene
/*
	The .pod file was exported from 3DSMax using PVRGeoPOD.
*/
const char c_szSceneFile[] = "RenderToTexture.pod";
const char c_szMaskTex[]   = "YellowWood.pvr";

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
const float g_fCameraNear = 4.0f;
const float g_fCameraFar  = 500.0f;

const char* m_pFBODescription = "Using FBOs";

#if !defined(EGL_NOT_PRESENT)
const char *m_pPBufferDescription = "Using PBuffers";
#endif


/*****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLESRenderToTexture : public PVRShell
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

	// Variables used for the animation
	float	m_fAngleY;

	// App Variables
	int		m_i32TexSize;

	// Render contexts, etc
	GLint	  m_i32OriginalFbo;

#if !defined(EGL_NOT_PRESENT)
	EGLDisplay m_CurrentDisplay;
	EGLContext m_CurrentContext;
	EGLSurface m_CurrentSurface;

	// We require a PBuffer surface.
	EGLSurface m_PBufferSurface;
#endif

	// If supported we require an FBO, which itself will require a depth buffer
	GLuint m_uFBO;
	GLuint m_uDepthBuffer;

	enum
	{
		eNone,
#if !defined(EGL_NOT_PRESENT)
		ePBuffer,
#endif
		eFBO
	} m_eR2TType;

	CPVRTglesExt m_Extensions;

	const char * m_pDescription;

	// Discard the frame buffer attachments
	bool m_bDiscard;

public:
	OGLESRenderToTexture() : m_puiVbo(0),
							m_puiIndexVbo(0),
							m_uiTextureID(0),
							m_uiTextureToRenderTo(0),
							m_fAngleY(0),
							m_i32OriginalFbo(0),
							m_pDescription(0),
							m_bDiscard(false)
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
	bool CreateFBOorPBuffer();
#if !defined(EGL_NOT_PRESENT)
	EGLConfig SelectEGLConfig();
#endif
	bool StartRenderToTexture();
	bool EndRenderToTexture();
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
bool OGLESRenderToTexture::InitApplication()
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
bool OGLESRenderToTexture::QuitApplication()
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
bool OGLESRenderToTexture::InitView()
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

	// Enables texturing
	glEnable(GL_TEXTURE_2D);

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

	// Create a FBO or PBuffer
	if(!CreateFBOorPBuffer())
		return false;

	// Setup some render states

	// Enable the depth test
	glEnable(GL_DEPTH_TEST);

	// Enable culling
	glEnable(GL_CULL_FACE);

	// Setup the material parameters our meshes will use
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,  PVRTVec4(1.0f).ptr());
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  PVRTVec4(1.0f).ptr());

	// Setup view and projection matrices used for when rendering to the texture

	// Caculate the view matrix
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
bool OGLESRenderToTexture::LoadTextures(CPVRTString* pErrorStr)
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
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
bool OGLESRenderToTexture::LoadVbos(CPVRTString* pErrorStr)
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
bool OGLESRenderToTexture::ReleaseView()
{
	// Delete the texture
	glDeleteTextures(1, &m_uiTextureID);
	glDeleteTextures(1, &m_uiTextureToRenderTo);

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

	// Destroy the FBO or PBuffer surface we were using
	switch(m_eR2TType)
	{
		case eFBO:
			// Delete frame buffer objects
			m_Extensions.glDeleteFramebuffersOES(1, &m_uFBO);

			// Delete our depth buffer
			m_Extensions.glDeleteRenderbuffersOES(1, &m_uDepthBuffer);
		break;
#if !defined(EGL_NOT_PRESENT)
		case ePBuffer:
			// Destroy the surfaces we created
			eglDestroySurface(m_CurrentDisplay,	m_PBufferSurface);
		break;
#endif
		default:
			break;
	}

	return true;
}

/*!****************************************************************************
 @Function		CreateFBOorPBuffer
 @Return		bool		true if no error occured
 @Description	Attempts to create our FBO if supported or a PBuffer if they
				are not.
******************************************************************************/
bool OGLESRenderToTexture::CreateFBOorPBuffer()
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
		glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &m_i32OriginalFbo);

		// Generate and bind a render buffer which will become a depth buffer shared between our two FBOs
		m_Extensions.glGenRenderbuffersOES(1, &m_uDepthBuffer);
		m_Extensions.glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_uDepthBuffer);

		/*
			Currently it is unknown to GL that we want our new render buffer to be a depth buffer.
			glRenderbufferStorage will fix this and in this case will allocate a depth buffer
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
		PVRShellSet(prefExitMessage, "ERROR: Required extension \"GL_OES_framebuffer_object\" not present.");
		return false;
	}
#endif

	// Create a texture for rendering to
	glGenTextures(1, &m_uiTextureToRenderTo);
	glBindTexture(GL_TEXTURE_2D, m_uiTextureToRenderTo);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_i32TexSize, m_i32TexSize, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 0);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create the surface or object that will allow us to render to the aforementioned texture
	switch(m_eR2TType)
	{
		case eFBO: // Create FBO
		{
			m_Extensions.glGenFramebuffersOES(1, &m_uFBO);
			m_Extensions.glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_uFBO);

			// Attach the texture to the FBO
			m_Extensions.glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, m_uiTextureToRenderTo, 0);

			// Attach the depth buffer we created earlier to our FBO.
			m_Extensions.glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, m_uDepthBuffer);

			// Check that our FBO creation was successful
			GLuint uStatus = m_Extensions.glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);

			if(uStatus != GL_FRAMEBUFFER_COMPLETE_OES)
			{
				PVRShellSet(prefExitMessage, "ERROR: Failed to initialise FBO");
				return false;
			}

			// Clear the colour and depth buffers for the FBO
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Unbind the FBO now we are done with it
			m_Extensions.glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_i32OriginalFbo);

			// Set the description used by Print3D later on
			m_pDescription = m_pFBODescription;
		}
		break;
#if !defined(EGL_NOT_PRESENT)
		case ePBuffer: // Create a PBuffer surface
		{
			//Using our attribute list and our egl configuration setup our PBuffer.
			m_PBufferSurface = eglCreatePbufferSurface(m_CurrentDisplay, eglConfig, list);

			// If we don't have a surface return false.
			if(m_PBufferSurface == EGL_NO_SURFACE)
			{
				PVRShellSet(prefExitMessage, "ERROR: Failed to create pbuffer.");
				return false;
			}

			// Switch the render target to the pBuffer
			if(!eglMakeCurrent(m_CurrentDisplay, m_PBufferSurface, m_PBufferSurface, m_CurrentContext))
			{
				PVRShellSet(prefExitMessage, "ERROR: Unable to make the pbuffer context current.");
				return false;
			}

			// Bind the texture to this surface
			eglBindTexImage(m_CurrentDisplay, m_PBufferSurface, EGL_BACK_BUFFER);

			// Set the description used by Print3D later on
			m_pDescription = m_pPBufferDescription;

			// Clear the colour and depth buffers for the PBuffer surface
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
		break;
#endif
		default: {}
	}

	return true;
}

/*!****************************************************************************
 @Function		StartRenderToTexture
 @Return		bool		true if no error occured
 @Description	Setup the render to texture
******************************************************************************/
bool OGLESRenderToTexture::StartRenderToTexture()
{
	switch(m_eR2TType)
	{
		case eFBO:
			// Bind our FBO
			m_Extensions.glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_uFBO);
		break;
#if !defined(EGL_NOT_PRESENT)
		case ePBuffer:
			// Switch the render target to the pBuffer
			if(!eglMakeCurrent(m_CurrentDisplay, m_PBufferSurface, m_PBufferSurface, m_CurrentContext))
			{
				PVRShellSet(prefExitMessage, "ERROR: Unable to make the pbuffer context current.\n");
				return false;
			}

			/*
				As we would like to render to the surface we need to release it from
				all the textures it is bound to. Once released the textures no longer
				contain the contents of the surface.
			*/

			if(!eglReleaseTexImage(m_CurrentDisplay, m_PBufferSurface, EGL_BACK_BUFFER))
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
bool OGLESRenderToTexture::EndRenderToTexture()
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

			// We are done with rendering to our FBO so switch back to the back buffer.
			m_Extensions.glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_i32OriginalFbo);
		break;
#if !defined(EGL_NOT_PRESENT)
		case ePBuffer:
			// We are done with rendering to our PBuffer so we now switch back to the backbuffer.
			if(!eglMakeCurrent(m_CurrentDisplay, m_CurrentSurface, m_CurrentSurface, m_CurrentContext))
			{
				PVRShellOutputDebug("ERROR: Unable to make the main context current.\n");
				return false;
			}

			// To use the contents of our PBuffer as a texture we need to bind the two together
			glBindTexture(GL_TEXTURE_2D, m_uiTextureToRenderTo);

			if(!eglBindTexImage(m_CurrentDisplay, m_PBufferSurface, EGL_BACK_BUFFER))
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
 @Function		RenderScene
 @Return		bool		true if no error occured
 @Description	Main rendering loop function of the program. The shell will
				call this function every frame.
				eglSwapBuffers() will be performed by PVRShell automatically.
				PVRShell will also manage important OS events.
				Will also manage relevent OS events. The user has access to
				these events through an abstraction layer provided by PVRShell.
******************************************************************************/
bool OGLESRenderToTexture::RenderScene()
{
	// Enable the attribute arrays
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	// Setup the lighting direction

	// Enables lighting. See BasicTnL for a detailed explanation
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// Reads the light direction from the scene.
	PVRTVec4 vLightDirection;
	PVRTVec3 vPos;

	m_Scene.GetLight(vPos, *(PVRTVec3*)&vLightDirection, 0);
	vLightDirection.x = -vLightDirection.x;
	vLightDirection.y = -vLightDirection.y;
	vLightDirection.z = -vLightDirection.z;
	vLightDirection.w = 0;

	// Update out angle used for rotating the mask
	PVRTMat4 mModel;
	m_fAngleY += (2*PVRT_PI/60)/7;

	// Setup everything needed to render to our texture
	if(StartRenderToTexture())
	{
		// Setup the Viewport to the dimensions of the texture
		glViewport(0, 0, m_i32TexSize, m_i32TexSize);

		// Set the colour to clear our texture to
		glClearColor(0.8f, 1.0f, 0.6f, 1.0f);

		// Clear the colour and depth buffer of our FBO / PBuffer surface
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Render our objects as we usually would

		// Load the projection matrix we would like to use
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(m_mR2TProjection.f);

		// Switch to the MODELVIEW matrix mode
		glMatrixMode(GL_MODELVIEW);

		// Specify the view matrix to OpenGL ES so we can specify the light in world space
		glLoadMatrixf(m_mR2TView.f);

		// Set the light direction
		glLightfv(GL_LIGHT0, GL_POSITION, vLightDirection.ptr());

		// Set the model-view matrix
		PVRTMat4 mModelView;
		mModelView = m_mR2TView * PVRTMat4::RotationY(m_fAngleY);

		glLoadMatrixf(mModelView.f);

		// Bind the mask's texture
		glBindTexture(GL_TEXTURE_2D, m_uiTextureID);

		// Draw our mask
		DrawMesh(m_Scene.pNode[0].nIdx);

		// We're done rendering to texture so revert back to rendering to the back buffer
		EndRenderToTexture();
	}

	// Set the clear colour
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	// Clear the colour and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Load the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(m_mProjection.f);

	glMatrixMode(GL_MODELVIEW);

	// Specify the view matrix to OpenGL ES so we can specify the light in world space
	glLoadMatrixf(m_mView.f);

	// Set the light direction
	glLightfv(GL_LIGHT0, GL_POSITION, vLightDirection.ptr());

	// Setup the Viewport to the dimensions of the screen
	glViewport(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	// Get the node model matrix
	PVRTMat4 mWorld = m_Scene.GetWorldMatrix(m_Scene.pNode[1]);

	// Set the model-view matrix
	PVRTMat4 mModelView;
	mModelView = m_mView * mWorld ;

	glLoadMatrixf(mModelView.f);

	// Bind our texture that we have rendered to
	glBindTexture(GL_TEXTURE_2D, m_uiTextureToRenderTo);

	// Draw our textured cube
	DrawMesh(m_Scene.pNode[1].nIdx);

	// Disable the vertex attribute arrays
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	// Display the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("RenderToTexture", m_pDescription, ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			mesh		The mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the meterial prepared.
******************************************************************************/
void OGLESRenderToTexture::DrawMesh(unsigned int ui32MeshID)
{
	SPODMesh& Mesh = m_Scene.pMesh[ui32MeshID];

	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[ui32MeshID]);
	// bind the index buffer, won't hurt if the handle is 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[ui32MeshID]);

	// Setup pointers
	glVertexPointer(Mesh.sVertex.n, GL_FLOAT, Mesh.sVertex.nStride, Mesh.sVertex.pData);
	glTexCoordPointer(Mesh.psUVW[0].n, GL_FLOAT, Mesh.psUVW[0].nStride, Mesh.psUVW[0].pData);
	glNormalPointer(GL_FLOAT, Mesh.sNormals.nStride, Mesh.sNormals.pData);

	// Indexed Triangle list
	glDrawElements(GL_TRIANGLES, Mesh.nNumFaces * 3, GL_UNSIGNED_SHORT, 0);
}

#if !defined(EGL_NOT_PRESENT)
/*!****************************************************************************
 @Function		SelectEGLConfig
 @Description	Finds an EGL config with required options based on Mode Requested - for PBuffer
******************************************************************************/
EGLConfig OGLESRenderToTexture::SelectEGLConfig()
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
	return new OGLESRenderToTexture();
}

/******************************************************************************
 End of file (OGLESRenderToTexture.cpp)
******************************************************************************/

