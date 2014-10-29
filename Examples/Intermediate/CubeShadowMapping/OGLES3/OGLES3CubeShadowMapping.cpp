/******************************************************************************

 @File         OGLES3CubeShadowMapping.cpp

 @Title        Demonstrating how to implement cube shadow mapping

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows how to apply cube shadow mapping for point light sources.

******************************************************************************/

#include "PVRShell.h"
#include "OGLES3Tools.h"

/******************************************************************************
 Global defines
******************************************************************************/

#define SHADOWMAP_SIZE 256

/******************************************************************************
 Content file names
******************************************************************************/

const char c_szSceneFile[]	= "scene.pod";
const char c_szPFXSrcFile[]	= "effect.pfx";

/******************************************************************************
 Global strings
******************************************************************************/

const CPVRTStringHash c_BuildShadowMapEffectName      = CPVRTStringHash("RenderShadowMap");
const CPVRTStringHash c_RenderSceneShadowedEffectName = CPVRTStringHash("RenderSceneWithShadows");
const CPVRTStringHash c_RenderSceneMaterialColourName = CPVRTStringHash("RenderSceneMaterialColour");

const CPVRTStringHash c_sTextureNames[] = 
{ 
	CPVRTStringHash("wall_left"), CPVRTStringHash("wall_right"), CPVRTStringHash("wall_top"),
	CPVRTStringHash("wall_bottom"), CPVRTStringHash("wall_back"), CPVRTStringHash("mask"),
};
const unsigned int c_uiNumTextureNames = sizeof(c_sTextureNames) / sizeof(c_sTextureNames[0]);

/******************************************************************************
 Structures and enums
******************************************************************************/

enum eCustomSemantics
{
	eCUSTOMSEMANTIC_INVFARPLANEDIST = ePVRTPFX_NumSemantics + 1
};

const SPVRTPFXUniformSemantic c_CustomSemantics[] = 
{ 
	{ "CUSTOMSEMANTIC_INVFARPLANEDIST",		eCUSTOMSEMANTIC_INVFARPLANEDIST },
};
const unsigned int c_uiNumCustomSemantics = sizeof(c_CustomSemantics)/sizeof(c_CustomSemantics[0]);

enum eIndices
{
	INDEX_RENDERSCENE = 0,
	INDEX_RENDERSHADOW,
	INDEX_RENDERDIFFUSE,
	NUM_INDICES
};

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES3CubeShadowMapping : public PVRShell
{
	CPVRTPrint3D	m_Print3D;
	SPVRTContext	m_sContext;
	CPVRTModelPOD	m_Scene;

	// Projection and Model View matrices
	PVRTMat4		m_mProjection, m_mView;
	PVRTMat4        m_mLightProjection, m_mLightView;
	PVRTVec3        m_vLightPosition;

	// Variables to handle the animation in a time-based manner
	unsigned long	m_ulTimePrev;
	float			m_fFrame;
	bool            m_bRotate;
	bool            m_bAnimate;
	float           m_fLightNearPlane;
	float           m_fLightFarPlane;
	
	GLint            m_iInitialFBO;
	GLuint           m_uiShadowFBO;
	GLuint           m_uiCubeTexturemap;
	GLuint           m_uiCubeShadowmap;	

	GLuint           m_uiDefaultTexture;

	// The effect file handlers
	CPVRTPFXParser	*m_pPFXEffectParser;
	CPVRTPFXEffect **m_ppPFXEffects;	
	int				 m_iEffectIndex[NUM_INDICES];
	
	// The Vertex buffer object handle array.
	GLuint			*m_aiVboID;
	GLuint			*m_aiIndexVboID;

	// A map of cached textures
	CPVRTMap<CPVRTStringHash, GLuint> m_TextureCache;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	void Update();

	bool LoadTextures(CPVRTString *pszErrorStr);
	bool CreateFBO(CPVRTString *pszErrorStr);

	bool RenderSceneWithEffect(const int uiEffectId, const PVRTMat4 &mProjection, const PVRTMat4 &mView);
	bool LoadPFX();
};


/*!****************************************************************************
 @Function		LoadTextures
 @Return		bool		true if no error occured
 @Description	Loads all textures that are used in this training course.
******************************************************************************/
bool OGLES3CubeShadowMapping::LoadTextures(CPVRTString *pszErrorStr)
{
	for (unsigned int i=0; i < c_uiNumTextureNames; i++)
	{		
		// Check if the texture already exists in the map
		if(m_TextureCache.Exists(c_sTextureNames[i]))
			continue;

		CPVRTString filename = c_sTextureNames[i].String() + CPVRTString(".pvr");
		
		// Texture is not loaded. Load and add to the map.
		GLuint uiHandle;
		PVRTextureHeaderV3 sHeader;
		if(PVRTTextureLoadFromPVR(filename.c_str(), &uiHandle, &sHeader) != PVR_SUCCESS)
		{
			*pszErrorStr = CPVRTString("Failed to load texture: ") + filename;
			return false;		
		}

		m_TextureCache[c_sTextureNames[i]] = uiHandle;		
	}

	/*
	    Create a default checkerboard texture
	 */
	glGenTextures(1, &m_uiDefaultTexture);
	glBindTexture(GL_TEXTURE_2D, m_uiDefaultTexture);
	unsigned char pData[16*16*3];
	for (int y=0; y < 16; y++)
	{
		for (int x=0; x < 16; x++)
		{
			pData[y*16*3+x*3] = (((y & 0x8) == 0) ^ ((x & 0x8) == 0))*255;
			pData[y*16*3+x*3+1] = (((y & 0x8) == 0) ^ ((x & 0x8) == 0))*255;
			pData[y*16*3+x*3+2] = (((y & 0x8) == 0) ^ ((x & 0x8) == 0))*255;
		}
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 16, 16, 0, GL_RGB, GL_UNSIGNED_BYTE, pData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return true;
}

/*!****************************************************************************
 @Function		CreateFBO
 @Return		bool		true if no error occured
 @Description	Creates the FBO and the attachments that are used to render the
                cube shadow map.
******************************************************************************/
bool OGLES3CubeShadowMapping::CreateFBO(CPVRTString *pszErrorStr)
{
	// Create FBO	
	glGenTextures(1, &m_uiCubeShadowmap);	
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_uiCubeShadowmap);
	for (unsigned int i=0; i < 6; i++)	
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT32F, SHADOWMAP_SIZE, SHADOWMAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	glGenTextures(1, &m_uiCubeTexturemap);	
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_uiCubeTexturemap);
	for (unsigned int i=0; i < 6; i++)		
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_R32UI, SHADOWMAP_SIZE, SHADOWMAP_SIZE, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);

	// Set the textures parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Create a frame buffer with only the depth buffer attached					
	glGenFramebuffers(1, &m_uiShadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_uiShadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, m_uiCubeTexturemap, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, m_uiCubeShadowmap, 0);
	GLenum drawbuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawbuffers);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		*pszErrorStr = "Frame buffer not set up correctly";
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_iInitialFBO);
	drawbuffers[0] = GL_BACK;
	glDrawBuffers(1, drawbuffers);

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
bool OGLES3CubeShadowMapping::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));
	
	m_bAnimate = true;
	m_fLightNearPlane = 1.0f;
	m_fLightFarPlane = 60.0f;

	// Load the scene
	if (m_Scene.ReadFromFile(c_szSceneFile) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Couldn't load the .pod file\n");
		return false;
	}

	// The cameras are stored in the file. We check it contains at least one.
	if(m_Scene.nNumCamera == 0)
	{
		PVRShellSet(prefExitMessage, "ERROR: The scene does not contain a camera\n");
		return false;
	}

	// The scene should at least contain a single light
	if (m_Scene.nNumLight == 0)
	{
		PVRShellSet(prefExitMessage, "ERROR: The scene does not contain a light\n");
		return false;
	}

	// Ensure that all meshes use an indexed triangle list
	for(unsigned int i = 0; i < m_Scene.nNumMesh; ++i)
	{
		if(m_Scene.pMesh[i].nNumStrips || !m_Scene.pMesh[i].sFaces.pData)
		{
			PVRShellSet(prefExitMessage, "ERROR: The meshes in the scene should use an indexed triangle list\n");
			return false;
		}
	}

	// Initialize variables used for the animation
	m_fFrame = 0;
	m_ulTimePrev = PVRShellGetTime();

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
bool OGLES3CubeShadowMapping::QuitApplication()
{
	// Frees the memory allocated for the scene
	m_Scene.Destroy();

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
bool OGLES3CubeShadowMapping::InitView()
{	
	CPVRTString szErrorString= "";
	m_bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	/*
		Initialize Print3D
	*/
	if(m_Print3D.SetTextures(&m_sContext,PVRShellGet(prefWidth),PVRShellGet(prefHeight), m_bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialize Print3D\n");
		return false;
	}	

	//
	// Load the PFX file containing all shaders
	//
	if (!LoadPFX())
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load PFX file.\n");
		return false;
	}

	// Sets the clear color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Enables depth test using the z-buffer
	glEnable(GL_DEPTH_TEST);

	// Create buffer objects.
	m_aiVboID = new GLuint[m_Scene.nNumMeshNode];
	glGenBuffers(m_Scene.nNumMeshNode, m_aiVboID);
	m_aiIndexVboID = new GLuint[m_Scene.nNumMeshNode];	
	glGenBuffers(m_Scene.nNumMeshNode, m_aiIndexVboID);

	for(int i = 0; i < (int)m_Scene.nNumMeshNode ; i++)
	{
		SPODNode* pNode = &m_Scene.pNode[i];
		SPODMesh* pMesh = &m_Scene.pMesh[pNode->nIdx];

		// Generate a vertex buffer and set the interleaved vertex data.
		unsigned int uiSize = pMesh->sVertex.nStride * pMesh->nNumVertex;
		glBindBuffer(GL_ARRAY_BUFFER, m_aiVboID[i]);
		glBufferData(GL_ARRAY_BUFFER, uiSize, pMesh->pInterleaved, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		uiSize = PVRTModelPODCountIndices(*pMesh) * sizeof(GLshort);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_aiIndexVboID[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, pMesh->sFaces.pData, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}	

	if (!LoadTextures(&szErrorString))
	{
		PVRShellSet(prefExitMessage, szErrorString.c_str());
		return false;
	}
	
	/*
	   Allocate the shadow FBO. The attachment will store the depth values required for shadow tests.
	 */

	// Store original FBO
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_iInitialFBO);

	if (!CreateFBO(&szErrorString))
	{
		PVRShellSet(prefExitMessage, szErrorString.c_str());
		return false;
	}		

	//
	// Store the indices into the individual arrays
	//
	for (unsigned int i=0; i < NUM_INDICES; i++)
		m_iEffectIndex[i] = -1;

	for (unsigned int i=0; i < m_pPFXEffectParser->GetNumberEffects(); i++)
	{
		// Store the indices for the individual effects
		if (m_pPFXEffectParser->GetEffect(i).Name == c_BuildShadowMapEffectName)
			m_iEffectIndex[INDEX_RENDERSHADOW] = i;
		else if (m_pPFXEffectParser->GetEffect(i).Name == c_RenderSceneShadowedEffectName)
			m_iEffectIndex[INDEX_RENDERSCENE] = i;
		else if (m_pPFXEffectParser->GetEffect(i).Name == c_RenderSceneMaterialColourName)
			m_iEffectIndex[INDEX_RENDERDIFFUSE] = i;
	}

	// security check
	for (unsigned int i=0; i < NUM_INDICES; i++)
	{
		if (m_iEffectIndex[i] == -1)
		{
			PVRShellSet(prefExitMessage, "ERROR: Not all necessary objects/effects found.\n");
			return false;
		}
	}

	return true;
}

/*!****************************************************************************
 @Function		LoadPFX
  @Return		bool			true if no error occurred
 @Description	Loads and compiles the shaders and links the shader programs
				required for this training course
******************************************************************************/
bool OGLES3CubeShadowMapping::LoadPFX(void)
{
	CPVRTString error = "";

	// Parse the whole PFX and store all data.
	m_pPFXEffectParser = new CPVRTPFXParser();
	if(m_pPFXEffectParser->ParseFromFile(c_szPFXSrcFile, &error) != PVR_SUCCESS)
	{
		error = "Parse failed:\n\n" + error;
		PVRShellSet(prefExitMessage, error.c_str());
		return false;
	}

	// Setup all effects in the PFX file so we initialize the shaders and
	// store uniforms and attributes locations.
	unsigned int uiNumEffects = m_pPFXEffectParser->GetNumberEffects();
	m_ppPFXEffects = new CPVRTPFXEffect*[uiNumEffects];

	// Load one by one the effects. This will also compile the shaders.
	for (unsigned int i=0; i < uiNumEffects; i++)
	{
		m_ppPFXEffects[i] = new CPVRTPFXEffect(m_sContext);
				
		if(m_ppPFXEffects[i]->RegisterUniformSemantic(c_CustomSemantics, c_uiNumCustomSemantics, &error))
		{
			error = "Failed to set custom semantics:\n\n" + error;
			PVRShellSet(prefExitMessage, error.c_str());
			return false;
		}

		unsigned int nUnknownUniformCount = 0;
		if(m_ppPFXEffects[i]->Load(*m_pPFXEffectParser, m_pPFXEffectParser->GetEffect(i).Name.c_str(), NULL, NULL, nUnknownUniformCount, &error)  != PVR_SUCCESS)
		{
			error = "Effect load failed:\n\n" + error;
			PVRShellSet(prefExitMessage, error.c_str());
			return false;
		}

		// .. upps, some uniforms are not in our table. Better to quit because something is not quite right.
		if(nUnknownUniformCount)
		{
			PVRShellOutputDebug(error.c_str());
			PVRShellOutputDebug("Unknown uniform semantic count: %d\n", nUnknownUniformCount);
			return false;
		}		
	}
	
	return true;
}


/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES3CubeShadowMapping::ReleaseView()
{
	// Release textures
	{
		const CPVRTArray<SPVRTPFXTexture>&	sTex = m_ppPFXEffects[0]->GetTextureArray();
		for(unsigned int i = 0; i < sTex.GetSize(); ++i)
			glDeleteTextures(1, &sTex[i].ui);
	}

	// Release the effect[s] then the parser
	for (unsigned int i=0; i < m_pPFXEffectParser->GetNumberEffects(); i++)
		delete m_ppPFXEffects[i];
	delete [] m_ppPFXEffects;
	delete m_pPFXEffectParser;

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

	// Release Vertex buffer objects.
	glDeleteBuffers(m_Scene.nNumMeshNode, m_aiVboID);
	glDeleteBuffers(m_Scene.nNumMeshNode, m_aiIndexVboID);
	delete [] m_aiVboID;
	delete [] m_aiIndexVboID;

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
bool OGLES3CubeShadowMapping::RenderScene()
{
	Update();

	glDisable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);	
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
			
	//
	//  Render the shadow maps for each direction
	//
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_uiShadowFBO);	
		glViewport(0, 0, SHADOWMAP_SIZE, SHADOWMAP_SIZE);			

		// Cull front facing faces, we only want the back facing ones in our shadow map
		glCullFace(GL_FRONT);

		PVRTMatrixPerspectiveFovRH(m_mLightProjection, PVRT_PI_OVER_TWO, 1.0f, m_fLightNearPlane, m_fLightFarPlane, m_bRotate);
		
		PVRTVec3 lightDir;
		m_Scene.GetLight(m_vLightPosition, lightDir, 0);

		PVRTVec3 light_to[] = 
		{ 
			PVRTVec3(1.0f, 0.0f, 0.0f), PVRTVec3(-1.0f,  0.0f,  0.0f), // posx, negx
			PVRTVec3(0.0f, 1.0f, 0.0f), PVRTVec3( 0.0f, -1.0f,  0.0f), // posy, negy
			PVRTVec3(0.0f, 0.0f, 1.0f), PVRTVec3( 0.0f,  0.0f, -1.0f), // posz, negz
		};
		PVRTVec3 light_up[] = 
		{ 			
			PVRTVec3(0.0f, -1.0f, 0.0f), PVRTVec3(0.0f, -1.0f,  0.0f), // posx, negx
			PVRTVec3(0.0f,  0.0f, 1.0f), PVRTVec3(0.0f,  0.0f, -1.0f), // posy, negy
			PVRTVec3(0.0f, -1.0f, 0.0f), PVRTVec3(0.0f, -1.0f,  0.0f), // posz, negz 
		};
						
		for (unsigned int i=0; i < 6; i++)
		{
			PVRTMat4 mLightView;
			PVRTVec3 lookat = m_vLightPosition + light_to[i];
			PVRTMatrixLookAtRH(mLightView, m_vLightPosition, lookat, light_up[i]);
			if (i == 0)
				m_mLightView = mLightView;
						
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_uiCubeShadowmap, 0);			
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_uiCubeTexturemap, 0);				
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				return false;
			
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			RenderSceneWithEffect(m_iEffectIndex[INDEX_RENDERSHADOW], m_mLightProjection, mLightView);
		}		

		// Restore the culling state
		glCullFace(GL_BACK);

		//Invalidate the color attachment we don't need to avoid unnecessary copying to system memory
		const GLenum attachment = GL_DEPTH_ATTACHMENT;
		glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attachment);
	}	

	
	//
	// Render the scene with the cubic shadow map
	//
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_iInitialFBO);		
		glViewport(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		PVRTVECTOR3	vFrom, vTo, vUp;
		// We can get the camera position, target and field of view (fov) with GetCameraPos()		
		float fFOV = m_Scene.GetCamera(vFrom, vTo, vUp, 0) * 0.75f;
		PVRTMatrixLookAtRH(m_mView, vFrom, vTo, vUp);
		PVRTMatrixPerspectiveFovRH(m_mProjection, fFOV, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), m_Scene.pCamera[0].fNear, m_Scene.pCamera[0].fFar, m_bRotate);
				
		// Texture unit 1 holds the shadow map
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_uiCubeTexturemap);

		// Texture unit 0 stores the albedo texture map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_uiDefaultTexture);
		
		RenderSceneWithEffect(m_iEffectIndex[INDEX_RENDERSCENE], m_mProjection, m_mView);
	}	

	//
	// Render the light source
	//
	RenderSceneWithEffect(m_iEffectIndex[INDEX_RENDERDIFFUSE], m_mProjection, m_mView);

	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("OGLES3CubeShadowMapping", "", ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		Update
 @Description	Handles user input and updates all timing data.
******************************************************************************/
void OGLES3CubeShadowMapping::Update()
{
	if (PVRShellIsKeyPressed(PVRShellKeyNameSELECT)) m_bAnimate = !m_bAnimate;
	
	// Calculates the frame number to animate in a time-based manner.
	// Uses the shell function PVRShellGetTime() to get the time in milliseconds.
	unsigned long ulTime = PVRShellGetTime();
	unsigned long ulDeltaTime = ulTime - m_ulTimePrev;
	m_ulTimePrev = ulTime;
	if (m_bAnimate)
		m_fFrame += (float)ulDeltaTime * 0.05f;

	if (m_fFrame > m_Scene.nNumFrame-1)
		m_fFrame = 0;

	m_Scene.SetFrame(m_fFrame);
}


/*!****************************************************************************
 @Function		RenderSceneWithEffect
 @Return		bool		true if no error occured
 @Description	Renders the whole scene with a single effect.
******************************************************************************/
bool OGLES3CubeShadowMapping::RenderSceneWithEffect(const int uiEffectId, const PVRTMat4 &mProjection, const PVRTMat4 &mView)
{
	CPVRTPFXEffect *pEffect = m_ppPFXEffects[uiEffectId];

	// Activate the passed effect
	pEffect->Activate();
	
	for (unsigned int i=0; i < m_Scene.nNumMeshNode; i++)
	{
		SPODNode* pNode = &m_Scene.pNode[i];
		SPODMesh* pMesh = &m_Scene.pMesh[pNode->nIdx];
		SPODMaterial *pMaterial = &m_Scene.pMaterial[pNode->nIdxMaterial];
		
		// Don't render the primitive indicating the light source (indicated by opacity != 1.0) 
		if (pMaterial->fMatOpacity < 1.0f && uiEffectId != m_iEffectIndex[INDEX_RENDERDIFFUSE])
			continue;

		// Bind the texture if there is one bound to this object
		if (pMaterial->nIdxTexDiffuse != -1)
		{
			CPVRTString texname = CPVRTString(m_Scene.pTexture[pMaterial->nIdxTexDiffuse].pszName);
			texname.substitute(".png", "");
			CPVRTStringHash hashedName(texname);
			if (m_TextureCache.Exists(hashedName))
				glBindTexture(GL_TEXTURE_2D, m_TextureCache[hashedName]);
			else
				glBindTexture(GL_TEXTURE_2D, m_uiDefaultTexture);
		}
		else
			glBindTexture(GL_TEXTURE_2D, m_uiDefaultTexture);
		
		glBindBuffer(GL_ARRAY_BUFFER, m_aiVboID[i]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_aiIndexVboID[i]);

		// Pre-calculate commonly used matrices
		PVRTMat4 mWorld;
		m_Scene.GetWorldMatrix(mWorld, *pNode);
		PVRTMat4 mWorldView = mView * mWorld;

		// Bind semantics
		const CPVRTArray<SPVRTPFXUniform>& Uniforms = pEffect->GetUniformArray();
		for(unsigned int j = 0; j < Uniforms.GetSize(); ++j)
		{
			switch(Uniforms[j].nSemantic)
			{
			case ePVRTPFX_UsPOSITION:
				{
					glVertexAttribPointer(Uniforms[j].nLocation, 3, GL_FLOAT, GL_FALSE, pMesh->sVertex.nStride, pMesh->sVertex.pData);
					glEnableVertexAttribArray(Uniforms[j].nLocation);
				}
				break;
			case ePVRTPFX_UsNORMAL:
				{
					glVertexAttribPointer(Uniforms[j].nLocation, 3, GL_FLOAT, GL_FALSE, pMesh->sNormals.nStride, pMesh->sNormals.pData);
					glEnableVertexAttribArray(Uniforms[j].nLocation);
				}
				break;
			case ePVRTPFX_UsUV:
				{
					glVertexAttribPointer(Uniforms[j].nLocation, 2, GL_FLOAT, GL_FALSE, pMesh->psUVW[0].nStride, pMesh->psUVW[0].pData);
					glEnableVertexAttribArray(Uniforms[j].nLocation);
				}
				break;
			case ePVRTPFX_UsMATERIALCOLORDIFFUSE:
				{										
					glUniform4f(Uniforms[j].nLocation, pMaterial->pfMatDiffuse[0], pMaterial->pfMatDiffuse[1], pMaterial->pfMatDiffuse[2], 1.0f);
				}
				break;
			case ePVRTPFX_UsVIEW:
				{
					glUniformMatrix4fv(Uniforms[j].nLocation, 1, GL_FALSE, m_mView.f);
				}
				break;
			case ePVRTPFX_UsWORLDIT:
				{
					PVRTMat3 mWorldIT3x3(mWorld.inverse().transpose());
					glUniformMatrix3fv(Uniforms[j].nLocation, 1, GL_FALSE, mWorldIT3x3.f);
				}
				break;
			case ePVRTPFX_UsWORLDVIEWPROJECTION:
				{
					PVRTMat4 mWorldViewProj = mProjection * mWorldView;					
					glUniformMatrix4fv(Uniforms[j].nLocation, 1, GL_FALSE, mWorldViewProj.f);
				}
				break;
			case ePVRTPFX_UsWORLDVIEWIT:
				{
					PVRTMat3 mWorldViewIT3x3(mWorldView.inverse().transpose());
					glUniformMatrix3fv(Uniforms[j].nLocation, 1, GL_FALSE, mWorldViewIT3x3.f);
				}
				break;
			case ePVRTPFX_UsTEXTURE:
				{
					// Set the sampler variable to the texture unit
					glUniform1i(Uniforms[j].nLocation, Uniforms[j].nIdx);
				}		
				break;
			case ePVRTPFX_UsWORLD:
				{
					glUniformMatrix4fv(Uniforms[j].nLocation, 1, GL_FALSE, mWorld.f);
				}
				break;
			case ePVRTPFX_UsLIGHTPOSWORLD:
				{
					glUniform3fv(Uniforms[j].nLocation, 1, m_vLightPosition.ptr());
				}
				break;			
			case eCUSTOMSEMANTIC_INVFARPLANEDIST:
				{
					float fInvFarPlaneDist = 1.0f / m_fLightFarPlane;
					glUniform1f(Uniforms[j].nLocation, fInvFarPlaneDist);
				}
				break;
			}
		}

		//	Now that all uniforms are set and the materials ready, draw the mesh.		
		glDrawElements(GL_TRIANGLES, pMesh->nNumFaces*3, GL_UNSIGNED_SHORT, 0);

		// Disable all vertex attributes
		for(unsigned int j = 0; j < Uniforms.GetSize(); ++j)
		{
			switch(Uniforms[j].nSemantic)
			{
			case ePVRTPFX_UsPOSITION:
			case ePVRTPFX_UsNORMAL:
			case ePVRTPFX_UsUV:
				glDisableVertexAttribArray(Uniforms[j].nLocation);
				break;
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return true;
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
	return new OGLES3CubeShadowMapping();
}

/******************************************************************************
 End of file (OGLES3CubeShadowMapping.cpp)
******************************************************************************/

