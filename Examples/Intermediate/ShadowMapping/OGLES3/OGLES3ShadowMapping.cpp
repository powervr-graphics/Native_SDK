/******************************************************************************

 @File         OGLES3ShadowMapping.cpp

 @Title        Shadow mapping

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows shadow mapping

******************************************************************************/
#include <string.h>

#include "PVRShell.h"
#include "OGLES3Tools.h"

/******************************************************************************
 Defines
******************************************************************************/
// Index to bind the attributes to vertex shaders
#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY	2

/******************************************************************************
 Consts
******************************************************************************/

// Const for the shadow map texture size
const unsigned int m_ui32ShadowMapSize = 512;

const CPVRTStringHash c_RenderShadowMapEffectName = CPVRTStringHash("RenderShadowMap");
const CPVRTStringHash c_RenderSceneEffectName     = CPVRTStringHash("RenderSceneUsingShadowMap");

const CPVRTStringHash c_sTextureNames[] = 
{ 
	CPVRTStringHash("Mask"), 
	CPVRTStringHash("TableCover"), 
	CPVRTStringHash("Torus")
};
const unsigned int c_uiNumTextureNames = sizeof(c_sTextureNames) / sizeof(c_sTextureNames[0]);

/******************************************************************************
 Structures and enums
******************************************************************************/

enum eCustomSemantics
{
	eCUSTOMSEMANTIC_SHADOWTRANSMATRIX = ePVRTPFX_NumSemantics + 1
};

const SPVRTPFXUniformSemantic c_CustomSemantics[] = 
{ 
	{ "CUSTOMSEMANTIC_SHADOWTRANSMATRIX", eCUSTOMSEMANTIC_SHADOWTRANSMATRIX },
};
const unsigned int c_uiNumCustomSemantics = sizeof(c_CustomSemantics)/sizeof(c_CustomSemantics[0]);

enum eIndices
{
	INDEX_RENDERSHADOW = 0,
	INDEX_RENDERSCENE,
	NUM_INDICES
};

/******************************************************************************
 Content file names
******************************************************************************/

const char c_szPFXFile[]   = "effect.pfx";
const char c_szSceneFile[] = "Scene.pod";

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES3ShadowMapping : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D  m_Print3D;
	CPVRTModelPOD m_Scene;
	SPVRTContext  m_sContext;

	GLuint* m_puiVbo;
	GLuint* m_puiIndexVbo;
	GLuint  m_uiShadowMapTexture;
	GLuint  m_uiFrameBufferObject;
    GLint   m_i32OriginalFbo;
		
	PVRTVec3 m_vLightDirection;
	PVRTVec3 m_vLightPosition;

	PVRTMat4 m_mView, m_mProjection;
	PVRTMat4 m_mLightProjection, m_mLightView;
	PVRTMat4 m_mBiasMatrix;

	// The effect file handlers
	CPVRTPFXParser	*m_pPFXEffectParser;
	CPVRTPFXEffect **m_ppPFXEffects;	
	int				 m_iEffectIndex[NUM_INDICES];

	// A map of cached textures
	CPVRTMap<CPVRTStringHash, GLuint> m_TextureCache;

	// Screen orientation variable
	bool m_bRotate;


	bool m_bDebug;
	float m_fBias;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	bool LoadTextures(CPVRTString* pErrorStr);
	bool LoadPFX(CPVRTString* pErrorStr);
	bool LoadVbos(CPVRTString* pErrorStr);
	bool CreateFBO(CPVRTString* pErrorStr);

	void Update();		
	bool RenderSceneWithEffect(const int uiEffectId, const PVRTMat4 &mProjection, const PVRTMat4 &mView);
};

/*!****************************************************************************
 @Function		LoadTextures
 @Output		pErrorStr		A CPVRTString describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course
******************************************************************************/
bool OGLES3ShadowMapping::LoadTextures(CPVRTString* const pszErrorStr)
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
	
	return true;
}

/*!****************************************************************************
 @Function		Update
 @Description	Handles user input and updates all timing data.
******************************************************************************/
void OGLES3ShadowMapping::Update()
{
	if (PVRShellIsKeyPressed(PVRShellKeyNameSELECT)) m_bDebug = !m_bDebug;
	if (PVRShellIsKeyPressed(PVRShellKeyNameLEFT)) m_fBias *= 0.9f;
	if (PVRShellIsKeyPressed(PVRShellKeyNameRIGHT)) m_fBias *= 1.1f;

	// Calculates the frame number to animate in a time-based manner.
	// Uses the shell function PVRShellGetTime() to get the time in milliseconds.
	static unsigned long ulTimePrev = PVRShellGetTime();
	unsigned long ulTime = PVRShellGetTime();
	unsigned long ulDeltaTime = ulTime - ulTimePrev;
	ulTimePrev = ulTime;
	static float fFrame = 0;
	if (!m_bDebug)
		fFrame += (float)ulDeltaTime * 0.05f;

	if (fFrame > m_Scene.nNumFrame-1)
		fFrame = 0;

	// Update the animation data
	m_Scene.SetFrame(fFrame);

	PVRTVec3 vFrom, vTo, vUp;	
	float fFOV = m_Scene.GetCamera(vFrom, vTo, vUp, 0) * 0.75f;
		

	m_mProjection = PVRTMat4::PerspectiveFovRH(fFOV, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), m_Scene.pCamera[0].fNear, m_Scene.pCamera[0].fFar, PVRTMat4::OGL, m_bRotate);
	m_mView = PVRTMat4::LookAtRH(vFrom, vTo, vUp);
	
	m_Scene.GetLight(m_vLightPosition, m_vLightDirection, 0);
	
	PVRTVec3 lightFrom, lightTo, lightUp;
	m_Scene.GetCamera(lightFrom, lightTo, lightUp, 1);
	m_mLightView = PVRTMat4::LookAtRH(lightFrom, lightTo, lightUp);
	m_mLightProjection = PVRTMat4::PerspectiveFovRH(PVRT_PI_OVER_TWO, 1.0f, m_Scene.pCamera[1].fNear, m_Scene.pCamera[1].fFar, PVRTMat4::OGL, m_bRotate);	
}


/*!****************************************************************************
 @Function		LoadPFX
 @Return		bool			true if no error occurred
 @Description	Loads and compiles the shaders and links the shader programs
				required for this training course
******************************************************************************/
bool OGLES3ShadowMapping::LoadPFX(CPVRTString* pErrorStr)
{
	// Parse the whole PFX and store all data.
	m_pPFXEffectParser = new CPVRTPFXParser();
	if(m_pPFXEffectParser->ParseFromFile(c_szPFXFile, pErrorStr) != PVR_SUCCESS)
	{
		*pErrorStr = "Parse failed:\n" + *pErrorStr;
		return false;
	}

	// Setup all effects in the PFX file so we initialize the shaders and
	// store uniforms and attributes locations.
	unsigned int uiNumEffects = m_pPFXEffectParser->GetNumberEffects();
	m_ppPFXEffects = new CPVRTPFXEffect*[uiNumEffects];
	for (unsigned int i=0; i < uiNumEffects; i++)
		m_ppPFXEffects[i] = 0;

	// Load one by one the effects. This will also compile the shaders.
	for (unsigned int i=0; i < uiNumEffects; i++)
	{
		m_ppPFXEffects[i] = new CPVRTPFXEffect(m_sContext);
				
		if(m_ppPFXEffects[i]->RegisterUniformSemantic(c_CustomSemantics, c_uiNumCustomSemantics, pErrorStr))
		{
			*pErrorStr = "Failed to set custom semantics:\n" + *pErrorStr;
			return false;
		}

		unsigned int nUnknownUniformCount = 0;
		if(m_ppPFXEffects[i]->Load(*m_pPFXEffectParser, m_pPFXEffectParser->GetEffect(i).Name.c_str(), NULL, NULL, nUnknownUniformCount, pErrorStr)  != PVR_SUCCESS)
		{
			*pErrorStr = "Failed to load effect " + m_pPFXEffectParser->GetEffect(i).Name.String() + CPVRTString(":\n") + *pErrorStr;
			return false;
		}

		// .. upps, some uniforms are not in our table. Better to quit because something is not quite right.
		if(nUnknownUniformCount)
		{
			*pErrorStr = "Unknown uniform semantic.\n";
			return false;
		}		
	}
	
	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
bool OGLES3ShadowMapping::LoadVbos(CPVRTString* pErrorStr)
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
 @Function		CreateFBO
 @Description	Creates the FBO that contains the shadow map texture.
******************************************************************************/
bool OGLES3ShadowMapping::CreateFBO(CPVRTString* pErrorStr)
{
	//Create the shadow map texture
	glGenTextures(1, &m_uiShadowMapTexture);
	glBindTexture(GL_TEXTURE_2D, m_uiShadowMapTexture);

	// Create the depth texture.
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_ui32ShadowMapSize, m_ui32ShadowMapSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_ui32ShadowMapSize, m_ui32ShadowMapSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

	// Set the textures parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// This configures the behaviour of the shadow2DProj function in the shader
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	if (glGetError() != GL_NO_ERROR)
	{
		*pErrorStr = "Error setting up depth texture format.\n";
		return false;
	}

	// Get the original framebuffer object handle.
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_i32OriginalFbo);

	// Create a frame buffer with only the depth buffer attached
	glGenFramebuffers(1, &m_uiFrameBufferObject);
	glBindFramebuffer(GL_FRAMEBUFFER, m_uiFrameBufferObject);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_uiShadowMapTexture, 0);

	GLenum drawbuffers[] = { GL_NONE };
	glDrawBuffers(1, drawbuffers);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		*pErrorStr = "ERROR: Frame buffer not set up correctly\n";
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_i32OriginalFbo);
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
bool OGLES3ShadowMapping::InitApplication()
{
	m_puiVbo = 0;
	m_puiIndexVbo = 0;
	m_ppPFXEffects = 0;
	m_pPFXEffectParser = 0;
    m_i32OriginalFbo = 0;

	m_bDebug = false;
	m_fBias = 0.001f;

	m_mBiasMatrix = PVRTMat4(0.5f, 0.0f, 0.0f, 0.0f,
	                         0.0f, 0.5f, 0.0f, 0.0f,
	                         0.0f, 0.0f, 0.5f, 0.0f,
	                         0.5f, 0.5f, 0.5f, 1.0f);

	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	// Load the scene
	if (m_Scene.ReadFromFile(c_szSceneFile) != PVR_SUCCESS)
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
bool OGLES3ShadowMapping::QuitApplication()
{
	// Free the memory allocated for the scene
	m_Scene.Destroy();

	delete [] m_puiVbo;
	delete [] m_puiIndexVbo;	

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
bool OGLES3ShadowMapping::InitView()
{
	CPVRTString ErrorStr;	    
    
	m_bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	// Initialize VBO data
	if(!LoadVbos(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	//	Load textures
	if (!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	//	Load and compile the shaders & link programs
	if (!LoadPFX(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Map the individual effects to make it easier to address them
	for (unsigned int i=0; i < m_pPFXEffectParser->GetNumberEffects(); i++)
	{
		// Store the indices for the individual effects
		if (m_pPFXEffectParser->GetEffect(i).Name == c_RenderShadowMapEffectName)
			m_iEffectIndex[INDEX_RENDERSHADOW] = i;
		else if (m_pPFXEffectParser->GetEffect(i).Name == c_RenderSceneEffectName)
			m_iEffectIndex[INDEX_RENDERSCENE] = i;		
	}	

	if (!CreateFBO(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	//	Initialize Print3D
	if (m_Print3D.SetTextures(0, PVRShellGet(prefWidth), PVRShellGet(prefHeight),m_bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialize Print3D\n");
		return false;
	}

	// Use a nice bright blue as clear colour
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	// Enable culling
	glEnable(GL_CULL_FACE);	

	// and depth testing
	glEnable(GL_DEPTH_TEST);

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES3ShadowMapping::ReleaseView()
{
	// Release textures
	{
		const CPVRTArray<SPVRTPFXTexture>&	sTex = m_ppPFXEffects[0]->GetTextureArray();
		for(unsigned int i = 0; i < sTex.GetSize(); ++i)
			glDeleteTextures(1, &sTex[i].ui);
	}

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

	glDeleteBuffers(m_Scene.nNumMeshNode, m_puiVbo);
	glDeleteBuffers(m_Scene.nNumMeshNode, m_puiIndexVbo);

	// Release the effect[s] then the parser
	if (m_pPFXEffectParser)
		for (unsigned int i=0; i < m_pPFXEffectParser->GetNumberEffects(); i++)
			if (m_ppPFXEffects[i])
				delete m_ppPFXEffects[i];
	delete [] m_ppPFXEffects;
	delete m_pPFXEffectParser;
	
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
bool OGLES3ShadowMapping::RenderScene()
{
	Update();
	
	// Bind the frame buffer object
	glBindFramebuffer(GL_FRAMEBUFFER, m_uiFrameBufferObject);

	// Clear the screen and depth buffer so we can render from the light's view
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	// Set the current viewport to our texture size but leave a one pixel margin.
	// As we are clamping to the edge of the texture when shadow mapping, no object
	// should be rendered to the border, otherwise stretching artefacts might occur
	// outside of the coverage of the shadow map.
	glViewport(1, 1, m_ui32ShadowMapSize-2, m_ui32ShadowMapSize-2);

	// Since we don't care about colour when rendering the depth values to
	// the shadow-map texture, we disable color writing to increase speed.
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	// Cull the front faces, so that only the backfaces are rendered into the shadowmap
	glCullFace(GL_FRONT);

	// Draw everything that we would like to cast a shadow
	RenderSceneWithEffect(INDEX_RENDERSHADOW, m_mLightProjection, m_mLightView);

	// Set the culling mode for the normal rendering
	glCullFace(GL_BACK);

	// Turn colour buffer writes back on again
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	// Restore our normal viewport size to our screen width and height
	glViewport(0, 0,PVRShellGet(prefWidth),PVRShellGet(prefHeight));

	//Invalidate the framebuffer attachments we don't need to avoid unnecessary copying to system memory
	const GLenum attachment = GL_COLOR_ATTACHMENT0;
	glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attachment);

	glBindFramebuffer(GL_FRAMEBUFFER, m_i32OriginalFbo);

	// Clear the colour and depth buffers, we are now going to render the scene again from scratch
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Load the shadow shader. This shader requires additional parameters; texProjMatrix for the depth buffer
	// look up and the light direction for diffuse light (the effect is a lot nicer with the addition of the
	// diffuse light).

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_uiShadowMapTexture);
	glActiveTexture(GL_TEXTURE0);

	RenderSceneWithEffect(INDEX_RENDERSCENE, m_mProjection, m_mView);
	
	m_Print3D.DisplayDefaultTitle("ShadowMap", "", ePVRTPrint3DSDKLogo);
	m_Print3D.Print3D(5.0f, 90.0f, 1.0f, 0xFFFFFFFF, "Bias: %f", m_fBias);
	m_Print3D.Flush();

	return true;
}


/*!****************************************************************************
 @Function		RenderSceneWithEffect
 @Return		bool		true if no error occured
 @Description	Renders the whole scene with a single effect.
******************************************************************************/
bool OGLES3ShadowMapping::RenderSceneWithEffect(const int uiEffectId, const PVRTMat4 &mProjection, const PVRTMat4 &mView)
{
	CPVRTPFXEffect *pEffect = m_ppPFXEffects[uiEffectId];

	// Activate the passed effect
	pEffect->Activate();
	
	for (unsigned int i=0; i < m_Scene.nNumMeshNode; i++)
	{
		SPODNode* pNode = &m_Scene.pNode[i];
		SPODMesh* pMesh = &m_Scene.pMesh[pNode->nIdx];
		SPODMaterial *pMaterial = 0;

		if (pNode->nIdxMaterial != -1)
		{
			pMaterial = &m_Scene.pMaterial[pNode->nIdxMaterial];	

			// Bind the texture if there is one bound to this object
			if (pMaterial->nIdxTexDiffuse != -1)
			{	
				CPVRTString texname = CPVRTString(m_Scene.pTexture[pMaterial->nIdxTexDiffuse].pszName).substitute(".png", "");
				CPVRTStringHash hashedName(texname);
				if (m_TextureCache.Exists(hashedName))
					glBindTexture(GL_TEXTURE_2D, m_TextureCache[hashedName]);
			}
		}
		
		glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[i]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[i]);

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
					if (pMaterial)
						glUniform4f(Uniforms[j].nLocation, pMaterial->pfMatDiffuse[0], pMaterial->pfMatDiffuse[1], pMaterial->pfMatDiffuse[2], 1.0f);
				}
				break;			
			case ePVRTPFX_UsWORLDVIEWPROJECTION:
				{
					PVRTMat4 mWorldViewProj = mProjection * mWorldView;					
					glUniformMatrix4fv(Uniforms[j].nLocation, 1, GL_FALSE, mWorldViewProj.f);
				}
				break;
			case ePVRTPFX_UsWORLDI:
				{
					PVRTMat3 mWorldI3x3(mWorld.inverse());
					glUniformMatrix3fv(Uniforms[j].nLocation, 1, GL_FALSE, mWorldI3x3.f);
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
			case ePVRTPFX_UsLIGHTPOSWORLD:
				{					
					glUniform3fv(Uniforms[j].nLocation, 1, m_vLightPosition.ptr());
				}
				break;			
			case eCUSTOMSEMANTIC_SHADOWTRANSMATRIX:
				{					
					// We need to calculate the texture projection matrix. This matrix takes the pixels from world space to previously rendered light projection space
					//where we can look up values from our saved depth buffer. The matrix is constructed from the light view and projection matrices as used for the previous render and 
					//then multiplied by the inverse of the current view matrix.
					//PVRTMat4 mTextureMatrix = m_mBiasMatrix * m_mLightProjection *  m_mLightView * mView.inverse();
					PVRTMat4 mTextureMatrix = m_mBiasMatrix * m_mLightProjection *  m_mLightView * mWorld;
					glUniformMatrix4fv(Uniforms[j].nLocation, 1, GL_FALSE, mTextureMatrix.f);
				}
				break;
			case ePVRTPFX_UsRANDOM:
				{					
					glUniform1f(Uniforms[j].nLocation, m_fBias);
				}
				break;			
			default:
				{
					PVRShellOutputDebug("Error: Unhandled semantic in RenderSceneWithEffect()\n");
					return false;
				}
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
	return new OGLES3ShadowMapping();
}

/******************************************************************************
 End of file (OGLES3ShadowMapping.cpp)
******************************************************************************/
