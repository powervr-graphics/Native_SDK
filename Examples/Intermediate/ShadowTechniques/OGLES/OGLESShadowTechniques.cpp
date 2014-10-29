/******************************************************************************

 @File         OGLESShadowTechniques.cpp

 @Title        Introducing the POD 3d file format

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows how to load POD files and play the animation with basic
               lighting

******************************************************************************/
#include <string.h>

#include "PVRShell.h"
#include "OGLESTools.h"
#include <math.h>


/******************************************************************************
 Content file names
******************************************************************************/

// PVR texture files
const char c_szKettleTexFile[]		= "Kettle.pvr";
const char c_szTableCoverTexFile[]	= "TableCover.pvr";
const char c_szBlobTexFile[]		= "Blob.pvr";

// POD File
const char c_szSceneFile[]	= "Scene.pod";

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
const float	g_fCharWidth   = 45.0f;	// Used to draw the basic blob shadow
const float g_fFloorHeight = 0.5f;

// Camera constants. Used for making the projection matrix
const float g_fCameraNear = 10.0f;
const float g_fCameraFar  = 1000.0f;

const unsigned int g_ui32TextureSize = 128;

enum ESceneObjects
{
	eGround,
	eLight,
	eShadowCaster
};

enum ShadowModes
{
	BASEBLOBMODE,
	ADVANCEDBLOBMODE,
	PROJGEOMMODE,
	R2TEXMODE
};
/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLESShadowTechniques : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// Texture handle
	GLuint	m_uiTableCover, m_uiKettle, m_uiBlobMap, m_uiShadow;

	// VBO Handles
	GLuint*	m_puiVbo;
	GLuint*	m_puiIndexVbo;

	// 3D Model
	CPVRTModelPOD	m_Scene;

	// Projection and Model View matrices
	PVRTMat4		m_mProjection, m_mView;

	// Array to lookup the textures for each material in the scene
	GLuint*		m_puiTextures;

	PVRTVec3	m_fLightPos;

	PVRTMat4	m_mfloorShadow;
	PVRTMat4    m_mLightView;
	PVRTMat4	m_mObjectRotation;

	PVRTVec4 m_fPlane;
	PVRTVec3 m_fObjectCentre;
	unsigned int m_ui32Mode;
	unsigned long m_ulTime;
	unsigned long m_ulSwitchTime;

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

	float m_fAngle;
	float m_fObjectAngle;

	// Discard the frame buffer attachments
	bool m_bDiscard;

public:
	OGLESShadowTechniques() : m_puiVbo(0),
							  m_puiIndexVbo(0),
							  m_i32OriginalFbo(0),
							  m_bDiscard(false)
	{
	}

	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	bool DrawShadowTexture();
	void DrawMesh(SPODNode* pNode, bool bProjectTexture = false);
	void DrawBaseBlob(PVRTVec3 fCentre);
	void DrawAdvancedBlobShadow();
	void shadowMatrix(PVRTMat4 &shadowMat, const PVRTVec4 &vPlane, const PVRTVec4 &vlightPos);
	void findPlane(PVRTVec4 &plane, const PVRTVec3 *pV0, const PVRTVec3 *pV1, const PVRTVec3 *pV2);
	void DrawProjectedShadow(SPODNode* pNode);
	bool RenderToTexture(SPODNode *pNode);
	bool RenderFromLightsView();
	bool CreateFBOorPBuffer();
#if !defined(EGL_NOT_PRESENT)
	EGLConfig SelectEGLConfig();
#endif
	bool StartRenderToTexture();
	bool EndRenderToTexture();
	void LoadVbos();
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
bool OGLESShadowTechniques::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	// Request PBuffer support
	PVRShellSet(prefPBufferContext, true);

	//Loads the scene from the .pod file into a CPVRTModelPOD object.
	if(m_Scene.ReadFromFile(c_szSceneFile) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load Scene.pod!");
		return false;
	}

	// The cameras are stored in the file. We check it contains at least one.
	if(m_Scene.nNumCamera == 0)
	{
		PVRShellSet(prefExitMessage, "ERROR: The scene does not contain a camera\n");
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
bool OGLESShadowTechniques::QuitApplication()
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
bool OGLESShadowTechniques::InitView()
{
	m_fAngle = 0;

	// Create a FBO or PBuffer
	if(!CreateFBOorPBuffer())
		return false;

	// Start the demo with the advanced blob
	m_ui32Mode = ADVANCEDBLOBMODE;

	// Is the screen rotated?
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	// Initialize Print3D
	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	//	Initialize VBO data
	LoadVbos();

	// Enables texturing
	glEnable(GL_TEXTURE_2D);

	//	Load the textures from the headers.
	if(PVRTTextureLoadFromPVR(c_szTableCoverTexFile, &m_uiTableCover) != PVR_SUCCESS)
		return false;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szKettleTexFile, &m_uiKettle) != PVR_SUCCESS)
		return false;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szBlobTexFile, &m_uiBlobMap) != PVR_SUCCESS)
		return false;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Enables lighting. See BasicTnL for a detailed explanation
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	PVRTVec3 From, To, Up(0.0f, 1.0f, 0.0f);

	// We can get the camera position, target and field of view (fov) with GetCameraPos()
	m_Scene.GetCameraPos(From, To, 0);

	m_mView = PVRTMat4::LookAtRH(From, To, Up);

	// Calculates the projection matrix
	m_mProjection = PVRTMat4::PerspectiveFovRH(1.0f, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), g_fCameraNear, g_fCameraFar, PVRTMat4::OGL, bRotate);

	// Reads the eLight direction from the scene.
	PVRTVec4 LightDirection;
	PVRTMat4 mWorld;

	// Set the eLight direction using the position of one of the meshes.
	SPODNode* pNode;

	pNode = &m_Scene.pNode[eLight];
	m_Scene.GetWorldMatrix(mWorld, *pNode);

	PVRTMat4 fRot;
	mWorld = PVRTMat4::RotationY(m_fAngle) * mWorld;

	LightDirection.x = m_fLightPos.x = mWorld.f[12];
	LightDirection.y = m_fLightPos.y = mWorld.f[13];
	LightDirection.z = m_fLightPos.z = mWorld.f[14];
	LightDirection.w = 0.0f;

	// Specify the eLight direction in world space
	glLightfv(GL_LIGHT0, GL_POSITION, LightDirection.ptr());

	glShadeModel( GL_SMOOTH );

	/*
		Build an array to map the textures within the pod header files
		to the textures we loaded a bit further up.
	*/
	m_puiTextures = new GLuint[m_Scene.nNumMaterial];

	for(unsigned int i = 0; i < m_Scene.nNumMaterial; ++i)
	{
		m_puiTextures[i] = 0;
		SPODMaterial* pMaterial = &m_Scene.pMaterial[i];

		if(!strcmp(pMaterial->pszName, "Material #1"))
			m_puiTextures[i] = m_uiTableCover;
		else if(!strcmp(pMaterial->pszName, "Material #2"))
			m_puiTextures[i] = m_uiKettle;
	}

	glEnable(GL_DEPTH_TEST);

	/*
		Get the centre of the mesh that I have called the shadow caster.
		This is used for the advanced blob.
	*/
	pNode = &m_Scene.pNode[eShadowCaster];
	SPODMesh* pMesh = &m_Scene.pMesh[pNode->nIdx];

	m_fObjectCentre = PVRTVec3(0.0f, 5.0f, 0.0f);

	/*
		Get the plane for the eGround mesh. Obviously this relys on the
		eGround being flat.
	*/
	pNode = &m_Scene.pNode[eGround];
	pMesh = &m_Scene.pMesh[pNode->nIdx];

	PVRTVec3* paVertices[3];

	paVertices[0] = (PVRTVec3*) (pMesh->pInterleaved + (size_t) pMesh->sVertex.pData);
	paVertices[1] = (PVRTVec3*) (pMesh->pInterleaved + (size_t) pMesh->sVertex.pData + pMesh->sVertex.nStride);
	paVertices[2] = (PVRTVec3*) (pMesh->pInterleaved + (size_t) pMesh->sVertex.pData + (pMesh->sVertex.nStride * 2));

	// Setup floor plane for projected shadow calculations.
	findPlane(m_fPlane, paVertices[0] , paVertices[1], paVertices[2]);

	// Get the start time.
	m_ulTime = PVRShellGetTime();
	m_ulSwitchTime = m_ulTime;

	m_fObjectAngle = 0.0f;
	m_mObjectRotation = PVRTMat4::Identity();

	// polygon offset for shadow to avoid ZFighting between the shadow and floor
	glPolygonOffset(-10.0f,-25.0f);
	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLESShadowTechniques::LoadVbos()
{
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
}

/*******************************************************************************
 * Function Name  : findPlane
 * Inputs		  : 3 Points
 * Outputs		  : Plane Equations
 * Description    : Find the plane equation given 3 points.
 *******************************************************************************/
void OGLESShadowTechniques::findPlane(PVRTVec4 &plane, const PVRTVec3 *pV0, const PVRTVec3 *pV1, const PVRTVec3 *pV2)
{
	PVRTVec3 vec0, vec1;

	// Need 2 vectors to find cross product.
	vec0 = *pV1 - *pV0;
	vec0.normalize();

	vec1 = *pV2 - *pV0;
	vec1.normalize();

	// find cross product to get A, B, and C of plane equation
	plane.x = vec0.y * vec1.z - vec0.z * vec1.y;
	plane.y = -(vec0.x * vec1.z - vec0.z * vec1.x);
	plane.z = vec0.x * vec1.y - vec0.y * vec1.x;

	plane.w = -(plane.x * pV0->x + plane.y * pV0->y + plane.z * pV0->z);
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLESShadowTechniques::ReleaseView()
{
	// Frees the texture lookup array
	delete[] m_puiTextures;

	// Frees the texture
	glDeleteTextures(1, &m_uiKettle);
	glDeleteTextures(1, &m_uiTableCover);

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

	// Release the FBO or PBuffer surface we were using
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
		default: { }
	}

	glDeleteTextures(1, &m_uiShadow);

	m_Scene.Destroy();
	return true;
}

/*!****************************************************************************
 @Function		CreateFBOorPBuffer
 @Return		bool		true if no error occured
 @Description	Attempts to create our FBO if supported or a PBuffer if they
				are not.
******************************************************************************/
bool OGLESShadowTechniques::CreateFBOorPBuffer()
{
#if !defined(EGL_NOT_PRESENT)
	EGLConfig eglConfig = 0;
	EGLint list[9];
#endif

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
			g_ui32TextureSize by g_ui32TextureSize.
		*/
		m_Extensions.glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, g_ui32TextureSize, g_ui32TextureSize);
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
		list[1] = g_ui32TextureSize;
		// ...then the height of the surface...
		list[2] = EGL_HEIGHT;
		list[3] = g_ui32TextureSize;
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
	glGenTextures(1, &m_uiShadow);
	glBindTexture(GL_TEXTURE_2D, m_uiShadow);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g_ui32TextureSize, g_ui32TextureSize, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 0);

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
			m_Extensions.glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, m_uiShadow, 0);

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

			// Get the original read and draw surfaces
			const EGLSurface originalReadSurface = eglGetCurrentSurface(EGL_READ);
			const EGLSurface originalDrawSurface = eglGetCurrentSurface(EGL_DRAW);

			// Switch the render target to the pBuffer
			if(!eglMakeCurrent(m_CurrentDisplay, m_PBufferSurface, m_PBufferSurface, m_CurrentContext))
			{
				PVRShellSet(prefExitMessage, "ERROR: Unable to make the pbuffer context current.");
				return false;
			}

			// Bind the texture to this surface
			eglBindTexImage(m_CurrentDisplay, m_PBufferSurface, EGL_BACK_BUFFER);

			// Clear the colour and depth buffers for the PBuffer surface
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Restore the original read and draw surfaces
			eglMakeCurrent(m_CurrentDisplay, originalDrawSurface, originalReadSurface, m_CurrentContext);
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
bool OGLESShadowTechniques::StartRenderToTexture()
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
bool OGLESShadowTechniques::EndRenderToTexture()
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
			glBindTexture(GL_TEXTURE_2D, m_uiShadow);

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
 @Function		RenderToTexture
 @Return		bool		true if no error occured
 @Description	Renders the mesh in pNode to texture from the cameras point
				of view.
******************************************************************************/
bool OGLESShadowTechniques::RenderToTexture(SPODNode *pNode)
{
	PVRTMat4 mWorld;
	PVRTMat4 mModelView;

	// Gets the node model matrix
	m_Scene.GetWorldMatrix(mWorld, *pNode);

	// Set the Shadow Color and Alpha
	glColor4f(0.25f, 0.25f, 0.25f, 1.0f);

	mWorld = m_mObjectRotation * mWorld;

	// Multiply the view matrix by the model (mWorld) matrix to get the model-view matrix
	glMatrixMode(GL_MODELVIEW);

	mModelView = m_mLightView * mWorld;
	glLoadMatrixf(mModelView.f);

	glEnableClientState(GL_VERTEX_ARRAY);

	glDisable(GL_TEXTURE_2D);

	/*
		For some reason on some platforms the rendering to a PBuffer fails if culling is enabled.
		Therefore I'm disabling culling before I render to the PBuffer.
	*/
	glDisable(GL_CULL_FACE);

	DrawMesh(pNode);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uiShadow);

	glEnable(GL_CULL_FACE);
	return true;
}

/*!****************************************************************************
 @Function		DrawShadowTexture
 @Return		bool		true if no error occured
 @Description	Draws the texture that has been rendered to for the shadow.
******************************************************************************/
bool OGLESShadowTechniques::DrawShadowTexture()
{
	glPushMatrix();
	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	float fWidth = (float) PVRShellGet(prefWidth);
	float fHeight= (float) PVRShellGet(prefHeight);
	float fScale = fHeight / fWidth;

	bool bRotate = PVRShellGet(prefIsRotated);

	if(bRotate)
	{
		fScale = fWidth / fHeight;
		glRotatef(90.0f, 0,0,1);
	}

	glTranslatef(-1, -1, 0.5f);
	glScalef(fScale, 1 , 1);

	static float VerticesLeft[] = {
			0.02f , 0.6f , 0.0f,
			0.02f , 0.02f, 0.0f,
			0.6f  , 0.02f, 0.0f,
			0.6f  , 0.6f , 0.0f,
	};

	static float	UVs[] = {
			0.0f, 1.0f,
			0.0f, 0.0f,
			1.0f, 0.0f,
	 		1.0f, 1.0f
		};

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT,0, (float*)&VerticesLeft);

	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2,GL_FLOAT,0, (float*)&UVs);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_uiShadow);

	glDrawArrays(GL_TRIANGLE_FAN,0,4);

	glDisableClientState(GL_VERTEX_ARRAY);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glClientActiveTexture(GL_TEXTURE0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	return true;
}

/*!****************************************************************************
 @Function		RenderFromLightsView
 @Return		bool		true if no error occured
 @Description	Renders the teapot from the eLight's view.
******************************************************************************/
bool OGLESShadowTechniques::RenderFromLightsView()
{
	glEnable(GL_DEPTH_TEST);

	StartRenderToTexture();

	glViewport(0, 0, g_ui32TextureSize, g_ui32TextureSize);
	glClearColor(1, 1, 1, 1);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	PVRTMat4 proj;

	proj = PVRTMat4::PerspectiveFovRH(1.0f, 1.0f, g_fCameraNear, g_fCameraFar, PVRTMat4::OGL, false);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(proj.f);

	RenderToTexture(&m_Scene.pNode[eShadowCaster]);

	EndRenderToTexture();

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
bool OGLESShadowTechniques::RenderScene()
{
	//	If the time and circumstances are right then switch the mode.
	unsigned long ulTime = PVRShellGetTime();
	bool bUpdateTexture = false;

	if(ulTime - m_ulSwitchTime > 5000)
	{
		++m_ui32Mode;

		if(m_ui32Mode == R2TEXMODE)
			bUpdateTexture = true;

		if(m_ui32Mode > R2TEXMODE)
			m_ui32Mode = ADVANCEDBLOBMODE;

		m_ulSwitchTime = ulTime;
	}

	//	Initialise the viewport and stuff
	SPODNode* pNode = 0;
	PVRTMat4 mWorld;
	PVRTMat4 mModelView;
	PVRTMat4 fTransform;

	// If the time is right then update the eLight's angle and the kettle's angle
	if(ulTime - m_ulTime > 10)
	{
		m_fAngle += 0.01f;
		m_ulTime = ulTime;

		m_fObjectAngle += 0.009f;

		m_mObjectRotation = PVRTMat4::Translation(0, 21, 0) * PVRTMat4::RotationX(-m_fObjectAngle) * PVRTMat4::Translation(0, -21, 0);

		// Update eLight position
		PVRTVec4 LightDirection;

		pNode = &m_Scene.pNode[eLight];

		m_Scene.GetWorldMatrix(mWorld, *pNode);

		mWorld = PVRTMat4::RotationY(m_fAngle) * mWorld;

		LightDirection.x = m_fLightPos.x = mWorld.f[12];
		LightDirection.y = m_fLightPos.y = mWorld.f[13];
		LightDirection.z = m_fLightPos.z = mWorld.f[14];
		LightDirection.w = 0.0f;

		// Specify the eLight direction in world space
		glLightfv(GL_LIGHT0, GL_POSITION, LightDirection.ptr());

		PVRTVec3 Up(0.0f, 1.0f, 0.0f), fPointOfInterest;

		// The position of the teapot
		fPointOfInterest = PVRTVec3(0.0f, 2.0f, 0.0f);

		// Model View Matrix
		PVRTMatrixLookAtRH(m_mLightView, m_fLightPos, fPointOfInterest, Up);
		bUpdateTexture = m_ui32Mode == R2TEXMODE;
	}

	// If we are in Render To Texture mode then render the teapot from the eLight's point of view.
	if(bUpdateTexture)
	{
		RenderFromLightsView();
		bUpdateTexture = false;
	}

	glEnable(GL_DEPTH_TEST);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glViewport(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Loads the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(m_mProjection.f);

	// Specify the view matrix to OpenGL ES
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);

	// Draw the eLight
	glDisable(GL_LIGHTING);

	pNode = &m_Scene.pNode[eLight];

	m_Scene.GetWorldMatrix(mWorld, *pNode);

	mModelView = m_mView * PVRTMat4::RotationY(m_fAngle) * mWorld;
	glLoadMatrixf(mModelView.f);

	glActiveTexture(GL_TEXTURE0);

	if(pNode->nIdxMaterial == -1)
		glBindTexture(GL_TEXTURE_2D, 0);
	else
		glBindTexture(GL_TEXTURE_2D, m_puiTextures[pNode->nIdxMaterial]);

	DrawMesh(pNode);

	// Draw the eGround
	pNode = &m_Scene.pNode[eGround];

	m_Scene.GetWorldMatrix(mWorld, *pNode);

	mModelView = m_mView * mWorld;
	glLoadMatrixf(mModelView.f);

	glActiveTexture(GL_TEXTURE0);

	if(pNode->nIdxMaterial == -1)
		glBindTexture(GL_TEXTURE_2D, 0);
	else
		glBindTexture(GL_TEXTURE_2D, m_puiTextures[pNode->nIdxMaterial]);

	if(m_ui32Mode == R2TEXMODE)
	{
		/*
			If we are in render to texture mode then draw the eGround with the rendered
			texture applied.
		*/
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_uiShadow);
		glEnable(GL_TEXTURE_2D);

		DrawMesh(pNode, true);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}
	else
	{
		DrawMesh(pNode);
	}


	// Draw the shadow caster
	glPushMatrix();
		glEnable(GL_LIGHTING);

		pNode = &m_Scene.pNode[eShadowCaster];

		m_Scene.GetWorldMatrix(mWorld, *pNode);

		mModelView = m_mView * m_mObjectRotation * mWorld;

		glLoadMatrixf(mModelView.f);

		glActiveTexture(GL_TEXTURE0);

		if (pNode->nIdxMaterial == -1)
			glBindTexture(GL_TEXTURE_2D, 0);
		else
			glBindTexture(GL_TEXTURE_2D, m_puiTextures[pNode->nIdxMaterial]);

		DrawMesh(pNode);

		glDisable(GL_LIGHTING);
	glPopMatrix();

	// Draw the shadows
	PVRTVec3 fCentre;

	switch(m_ui32Mode)
	{
		case BASEBLOBMODE:
			fCentre = PVRTVec3(0.0f, 0.0f, 0.0f);

			// Set the modelview without the kettle rotation
			mModelView = m_mView * mWorld;
			glLoadMatrixf(mModelView.f);

			DrawBaseBlob(fCentre);
			m_Print3D.DisplayDefaultTitle("ShadowTechniques", "Base Blob", ePVRTPrint3DSDKLogo);
		break;
		case ADVANCEDBLOBMODE:
			// Set the modelview without the kettle rotation
			mModelView =  m_mView * mWorld;
			glLoadMatrixf(mModelView.f);

			DrawAdvancedBlobShadow();
			m_Print3D.DisplayDefaultTitle("ShadowTechniques", "Dynamic Blob", ePVRTPrint3DSDKLogo);
		break;
		case PROJGEOMMODE:
			glLoadMatrixf(m_mView.f);
			DrawProjectedShadow(pNode);
			m_Print3D.DisplayDefaultTitle("ShadowTechniques", "Projected geometry", ePVRTPrint3DSDKLogo);
		break;
		case R2TEXMODE:
			// This shadow is drawn when the eGround is drawn.
			m_Print3D.DisplayDefaultTitle("ShadowTechniques",
				m_eR2TType == eFBO ? "Projected render (Using FBOs)" : "Projected render (Using PBuffer)",
				ePVRTPrint3DSDKLogo);

			DrawShadowTexture();
		break;
	};

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			mesh		The mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the meterial prepared.
******************************************************************************/
void OGLESShadowTechniques::DrawMesh(SPODNode* pNode, bool bProjectTexture)
{
	unsigned int ui32MeshID = pNode->nIdx;

	// Get Mesh referenced by the pNode
	SPODMesh& Mesh = m_Scene.pMesh[ui32MeshID];

	// Bind the vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[ui32MeshID]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[ui32MeshID]);

	glVertexPointer(3, GL_FLOAT, Mesh.sVertex.nStride, Mesh.sVertex.pData);
	glNormalPointer(GL_FLOAT   , Mesh.sNormals.nStride, Mesh.sNormals.pData);

	glClientActiveTexture(GL_TEXTURE0);
	glTexCoordPointer(2, GL_FLOAT, Mesh.psUVW[0].nStride, Mesh.psUVW[0].pData);

	if(bProjectTexture)
	{
		glMatrixMode(GL_TEXTURE);
		glPushMatrix();
		glLoadIdentity();

		glClientActiveTexture(GL_TEXTURE1);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(3, GL_FLOAT, Mesh.sVertex.nStride, Mesh.sVertex.pData);

		glLoadIdentity();

		glTranslatef(0.5f, 0.5f, 0.0f);
		glScalef(0.003f, 0.003f, 1.0f);

		glMultMatrixf(m_mLightView.f);
	}

	// Indexed Triangle list
	glDrawElements(GL_TRIANGLES, Mesh.nNumFaces*3, GL_UNSIGNED_SHORT, 0);

	if(bProjectTexture)
	{
		glClientActiveTexture(GL_TEXTURE1);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
	}

	// unbind the vertex buffers as we don't need them bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/*!****************************************************************************
 @Function		DrawProjectedShadow
 @Return		void
 @Description	Squish the mesh to the eGround plane and draw it.
******************************************************************************/
void OGLESShadowTechniques::DrawProjectedShadow(SPODNode* pNode)
{
	glPushMatrix();

	// Multiply the view matrix by the model (mWorld) matrix to get the model-view matrix
	PVRTMat4 m_mfloorShadow;
	PVRTVec4 vCurLightPos;

	vCurLightPos = PVRTVec4(m_fLightPos.x, m_fLightPos.y, m_fLightPos.z, 0.0f);

	shadowMatrix(m_mfloorShadow, m_fPlane, vCurLightPos);
	glMultMatrixf(m_mfloorShadow.f);

	// Enable Polygon offset to avoid ZFighting between floor and shadow
	glEnable(GL_POLYGON_OFFSET_FILL);

	// Disable Blending since alpha blend does not work with projection
	glDisable (GL_BLEND);

	// Disable Texture
	glDisable(GL_TEXTURE_2D);

	// Set the Shadow Color and Alpha
	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	// Set the transformation of the kettle
	PVRTMat4 fTransform, mWorld;
	m_Scene.GetWorldMatrix(mWorld, *pNode);

	mWorld *= m_mObjectRotation;
	glMultMatrixf(mWorld.f);

	// Render the objects which will be slammed into the floor plane
	DrawMesh(pNode);

	// Disable Polygon offset to avoid ZFighting between floor and shadow
	glDisable(GL_POLYGON_OFFSET_FILL);
	glEnable(GL_TEXTURE_2D);
	glEnable (GL_BLEND);

	glPopMatrix();
}

/*!****************************************************************************
 @Function		shadowMatrix
 @Return		void
 @Description	Create a matrix to squish the mesh.
******************************************************************************/
void OGLESShadowTechniques::shadowMatrix(PVRTMat4 &shadowMat, const PVRTVec4 &vPlane, const PVRTVec4 &vlightPos)
{
	float fDot;

	// Find dot product between eLight position vector and eGround plane normal.
	fDot =
		vPlane.x * vlightPos.x +
		vPlane.y * vlightPos.y +
		vPlane.z * vlightPos.z +
		vPlane.w * vlightPos.w;

	shadowMat.f[ 0] = fDot - vlightPos.x * vPlane.x;
	shadowMat.f[ 4] = 0   - vlightPos.x * vPlane.y;
	shadowMat.f[ 8] = 0   - vlightPos.x * vPlane.z;
	shadowMat.f[12] = 0   - vlightPos.x * vPlane.w;

	shadowMat.f[ 1] = 0   - vlightPos.y * vPlane.x;
	shadowMat.f[ 5] = fDot - vlightPos.y * vPlane.y;
	shadowMat.f[ 9] = 0   - vlightPos.y * vPlane.z;
	shadowMat.f[13] = 0   - vlightPos.y * vPlane.w;

	shadowMat.f[ 2] = 0   - vlightPos.z * vPlane.x;
	shadowMat.f[ 6] = 0   - vlightPos.z * vPlane.y;
	shadowMat.f[10] = fDot - vlightPos.z * vPlane.z;
	shadowMat.f[14] = 0   - vlightPos.z * vPlane.w;

	shadowMat.f[ 3] = 0   - vlightPos.w * vPlane.x;
	shadowMat.f[ 7] = 0   - vlightPos.w * vPlane.y;
	shadowMat.f[11] = 0   - vlightPos.w * vPlane.z;
	shadowMat.f[15] = fDot - vlightPos.w * vPlane.w;
}

/*!****************************************************************************
 @Function		DrawBaseBlob
 @Return		void
 @Description	Draw a base blob around the input coordinate
******************************************************************************/
void OGLESShadowTechniques::DrawBaseBlob(PVRTVec3 fCentre)
{
	glDisableClientState(GL_NORMAL_ARRAY);

	float	Vertices[] = {
			fCentre.x + g_fCharWidth	, fCentre.y + g_fFloorHeight, fCentre.z + -g_fCharWidth,
			fCentre.x + -g_fCharWidth, fCentre.y + g_fFloorHeight, fCentre.z + -g_fCharWidth,
			fCentre.x + g_fCharWidth	, fCentre.y + g_fFloorHeight, fCentre.z + g_fCharWidth ,
	 		fCentre.x + -g_fCharWidth, fCentre.y + g_fFloorHeight, fCentre.z + g_fCharWidth
		};

	static float	UVs[] = {
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f, 1.0f,
	 		1.0f, 1.0f
		};

	// Enable Polygon offset to avoid ZFighting between floor and shadow
	glEnable(GL_POLYGON_OFFSET_FILL);

	// Enable Blending for Transparent Blob
	glEnable (GL_BLEND);
	glBlendFunc(GL_DST_COLOR, GL_ZERO);

	// Bind Blob Texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,m_uiBlobMap);

	// Draw Blob - in this case the object is "static" so blob position is "static" as well
	// In a Game the Blob position would be calculated from the Character Position.

	// Enable Client States and Setup Data Pointers
	glVertexPointer(3,GL_FLOAT,0,(float*)&Vertices);

	glClientActiveTexture(GL_TEXTURE0);
	glTexCoordPointer(2,GL_FLOAT,0,(float*)&UVs);

	// Draw Geometry
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// Disable blending
	glDisable (GL_BLEND);

	// Disable Polygon offset to avoid ZFighting between floor and shadow
	glDisable(GL_POLYGON_OFFSET_FILL);

	glEnableClientState(GL_NORMAL_ARRAY);
}

/*!****************************************************************************
 @Function		DrawAdvancedBlobShadow
 @Return		void
 @Description	Find the intersection point of the ray to the eGround plane
				and place a blob there.
******************************************************************************/
void OGLESShadowTechniques::DrawAdvancedBlobShadow()
{
	PVRTVec3 fRay, fNorm, fInter;

	fRay = m_fObjectCentre - m_fLightPos;

	fNorm = m_fPlane;

	fRay.normalize();

	float fAlpha = m_fPlane.w - fNorm.dot(m_fLightPos);
	float fK =  fNorm.dot(fRay);

	if(fK != 0.0f)
		fAlpha = fAlpha / fK;

	if(fK == 0.0f)
		fInter = PVRTVec3(0.0f, 0.0f, 0.0f);
	else
	{
		fInter.x = m_fLightPos.x + fAlpha * fRay.x;
		fInter.y = 0;
		fInter.z = m_fLightPos.z + fAlpha * fRay.z;
	}

	DrawBaseBlob(fInter);
}

#if !defined(EGL_NOT_PRESENT)
/*!****************************************************************************
 @Function		SelectEGLConfig
 @Description	Finds an EGL config with required options based on Mode Requested - for PBuffer
******************************************************************************/
EGLConfig OGLESShadowTechniques::SelectEGLConfig()
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
	return new OGLESShadowTechniques();
}

/******************************************************************************
 End of file (OGLESShadowTechniques.cpp)
******************************************************************************/

