/******************************************************************************

 @File         OGLES2MultipleRenderTargets.cpp

 @Title        Multiple Render Targets (MRT)

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows how to output to several buffers in a single pass.

******************************************************************************/

#include "PVRShell.h"
#include "OGLES2Tools.h"

/******************************************************************************
 Constants
******************************************************************************/

const unsigned int c_uiRenderTargetSize = 512;
const unsigned int c_uiNumRenderTargets = 4;

/******************************************************************************
 Global strings
******************************************************************************/

const CPVRTStringHash c_RenderMRTsEffectName  = CPVRTStringHash("RenderMRTs");
const CPVRTStringHash c_BlitTextureEffectName = CPVRTStringHash("BlitTexture");
const CPVRTStringHash c_BlitGrayTextureEffectName = CPVRTStringHash("BlitSingleChannelTexture");

/******************************************************************************
 Defines
******************************************************************************/

// Index to bind the attributes to vertex shaders
#define VERTEX_ARRAY	0
#define TEXCOORD_ARRAY	1

/******************************************************************************
 Content file names
******************************************************************************/

const char c_szPFXSrcFile[]	 = "effect.pfx";
const char c_szSceneFile[]   = "scene.pod";

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES2MultipleRenderTargets : public PVRShell, public PVRTPFXEffectDelegate
{
	CPVRTPrint3D	m_Print3D;
	CPVRTModelPOD	m_Scene;
	CPVRTgles2Ext   m_Extensions;

	// Projection and Model View matrices
	PVRTMat4        m_mProjection;	
	PVRTMat4        m_mView;
	PVRTVec3        m_vLightDirection;
	
	// OpenGL handles for shaders, textures and VBOs
	GLuint* m_puiVbo;
	GLuint* m_puiIndexVbo;

	// Rendertarget data
	bool   m_bRotate;
	GLint  m_iInitialFramebuffer;
	GLuint m_uiFBO;
	GLuint m_uiDepthBuffer;
	GLuint m_uiRenderTextures[c_uiNumRenderTargets];

	// The effect file handlers
	CPVRTPFXParser	*m_pPFXEffectParser;
	CPVRTPFXEffect **m_ppPFXEffects;	

	// A map of cached textures
	CPVRTMap<CPVRTStringHash, GLuint> m_TextureCache;
	
public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();
	
	bool CreateFBO(CPVRTString *pszErrorMsg);
	void LoadVbos();
	void Update();

	void DrawAxisAlignedQuad(PVRTVec2 afLowerLeft, PVRTVec2 afUpperRight, CPVRTPFXEffect *pEffect);

	EPVRTError PVRTPFXOnLoadTexture(const CPVRTStringHash& TextureName, GLuint& uiHandle, unsigned int& uiFlags);
	bool RenderSceneWithEffect(const int uiEffectId, const PVRTMat4 &mProjection, const PVRTMat4 &mView);
	bool LoadPFX(CPVRTString *pErrorMsg);
};

/*!***************************************************************************
@Function		PVRTPFXOnLoadTexture
@Input			TextureName
@Output			uiHandle
@Output			uiFlags
@Return			EPVRTError	PVR_SUCCESS on success.
@Description	Callback function on texture load.
*****************************************************************************/
EPVRTError OGLES2MultipleRenderTargets::PVRTPFXOnLoadTexture(const CPVRTStringHash& TextureName, GLuint& uiHandle, unsigned int& uiFlags)
{
	uiFlags = 0;

	/*
		Because we have multiple effects being loaded yet the textures remain the same we can
		efficiently cache texture IDs and only load a texture once but assign it to
		multiple effects.
	*/

	// Check if the texture already exists in the map
	if(m_TextureCache.Exists(TextureName))
	{
		uiHandle = m_TextureCache[TextureName];
		return PVR_SUCCESS;
	}

	// Texture is not loaded. Load and add to the map.
	if(PVRTTextureLoadFromPVR(TextureName.c_str(), &uiHandle, NULL) != PVR_SUCCESS)
		return PVR_FAIL;
		
	m_TextureCache[TextureName] = uiHandle;
	
	return PVR_SUCCESS;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLES2MultipleRenderTargets::LoadVbos()
{
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
}


/*!****************************************************************************
 @Function		CreateFBO
 @Return		bool		true if no error occured
 @Description	Allocates FBO data
******************************************************************************/
bool OGLES2MultipleRenderTargets::CreateFBO(CPVRTString *pszErrorMsg)
{
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_iInitialFramebuffer);

	// Allocate the render targets
	glGenTextures(c_uiNumRenderTargets, m_uiRenderTextures);
	const GLenum formats[] = { GL_RGB, GL_RGB, GL_RGB, GL_RGB };
	const GLenum internalformat[] = { GL_RGB, GL_RGB, GL_RGB, GL_RGB };

	for (unsigned int i=0; i < c_uiNumRenderTargets; i++)
	{
		glBindTexture(GL_TEXTURE_2D, m_uiRenderTextures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, internalformat[i], c_uiRenderTargetSize, c_uiRenderTargetSize, 0, formats[i], GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
	}

	// Create the depth renderbuffer
	glGenRenderbuffers(1, &m_uiDepthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_uiDepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, c_uiRenderTargetSize, c_uiRenderTargetSize);

	// Allocate the framebuffer object
	glGenFramebuffers(1, &m_uiFBO);	
	glBindFramebuffer(GL_FRAMEBUFFER, m_uiFBO);				

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_uiDepthBuffer);
	for (unsigned int i=0; i < c_uiNumRenderTargets; i++)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_uiRenderTextures[i], 0);

	// Setup the rendertargets to their corresponding attachment points
	GLenum drawbuffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_COLOR_ATTACHMENT3_EXT };
	m_Extensions.glDrawBuffersEXT(c_uiNumRenderTargets, drawbuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		*pszErrorMsg = "ERROR: Frame buffer not set up correctly\n";
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_iInitialFramebuffer);
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
bool OGLES2MultipleRenderTargets::InitApplication()
{
	m_puiVbo = 0;
	m_puiIndexVbo = 0;
	
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
				not be called.x
******************************************************************************/
bool OGLES2MultipleRenderTargets::QuitApplication()
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
bool OGLES2MultipleRenderTargets::InitView()
{	
	CPVRTString ErrorStr;
	
	/*
	    Check that EXT_blend_minmax is supported
	 */
	if (!CPVRTgles2Ext::IsGLExtensionSupported("GL_EXT_draw_buffers"))
	{
		PVRShellSet(prefExitMessage, "ERROR: GL_EXT_draw_buffers extension is required to run this example.");
		return false;
	}
	
	/*
		Check that there's enough draw buffers available for this demo.
	 */
	GLint maxDrawBuffers;
	glGetIntegerv(GL_MAX_DRAW_BUFFERS_EXT, &maxDrawBuffers);
	if (maxDrawBuffers < 4)
	{
		PVRShellSet(prefExitMessage, "ERROR: This demo requires at least 4 available draw buffers to be present.");
		return false;
	}
	
	m_Extensions.LoadExtensions();

	/*
		Initialize VBO data
	*/
	LoadVbos();

	/*
		Load and compile the shaders & link programs
	*/
	if (!LoadPFX(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	/*
		Allocate the FBO and attachments for the MRTs
	*/
	if (!CreateFBO(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Is the screen rotated?
	m_bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);
	/*
		Initialize Print3D
	*/
	if(m_Print3D.SetTextures(NULL, PVRShellGet(prefWidth),PVRShellGet(prefHeight), m_bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}
		
	/*
		Set OpenGL render states needed for this training course
	*/
	// Enable backface culling and depth test
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	
	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES2MultipleRenderTargets::ReleaseView()
{
	// Release textures
	{
		const CPVRTArray<SPVRTPFXTexture>&	sTex = m_ppPFXEffects[0]->GetTextureArray();
		for(unsigned int i = 0; i < sTex.GetSize(); ++i)
			glDeleteTextures(1, &sTex[i].ui);
	}
		
	// Delete buffer objects
	glDeleteBuffers(m_Scene.nNumMesh, m_puiVbo);
	glDeleteBuffers(m_Scene.nNumMesh, m_puiIndexVbo);

	glDeleteRenderbuffers(1, &m_uiDepthBuffer);
	glDeleteTextures(c_uiNumRenderTargets, m_uiRenderTextures);
	glDeleteFramebuffers(1, &m_uiFBO);

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

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
bool OGLES2MultipleRenderTargets::RenderScene()
{
	Update();

	/*
		Render the scene to all colour attachments using MRTs
	*/
	glBindFramebuffer(GL_FRAMEBUFFER, m_uiFBO);
	glViewport(0, 0, c_uiRenderTargetSize, c_uiRenderTargetSize);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	RenderSceneWithEffect(m_pPFXEffectParser->FindEffectByName(c_RenderMRTsEffectName), m_mProjection, m_mView);

	// Blit each individual MRT to one corner of the screen
	glBindFramebuffer(GL_FRAMEBUFFER, m_iInitialFramebuffer);
	glViewport(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	CPVRTPFXEffect *pEffect = m_ppPFXEffects[m_pPFXEffectParser->FindEffectByName(c_BlitTextureEffectName)];
	pEffect->Activate();
	glBindTexture(GL_TEXTURE_2D, m_uiRenderTextures[0]);
	DrawAxisAlignedQuad(PVRTVec2(-1, 0), PVRTVec2(0, 1), pEffect);	
	glBindTexture(GL_TEXTURE_2D, m_uiRenderTextures[3]);
	DrawAxisAlignedQuad(PVRTVec2(0, -1), PVRTVec2(1, 0), pEffect);

	pEffect = m_ppPFXEffects[m_pPFXEffectParser->FindEffectByName(c_BlitGrayTextureEffectName)];
	pEffect->Activate();
	glBindTexture(GL_TEXTURE_2D, m_uiRenderTextures[1]);
	DrawAxisAlignedQuad(PVRTVec2(0, 0), PVRTVec2(1, 1), pEffect);
	glBindTexture(GL_TEXTURE_2D, m_uiRenderTextures[2]);
	DrawAxisAlignedQuad(PVRTVec2(-1, -1), PVRTVec2(0, 0), pEffect);
	
	glBindTexture(GL_TEXTURE_2D, 0);

	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("MRT (Multiple RenderTargets)", "", ePVRTPrint3DSDKLogo);
	m_Print3D.Print3D(5.0f, 45.0f, 0.5f, 0xFFFFFFFF, "Albedo");
	m_Print3D.Print3D(5.0f, 95.0f, 0.5f, 0xFFFFFFFF, "Specular");
	m_Print3D.Print3D(55.0f, 45.0f, 0.5f, 0xFFFFFFFF, "Diffuse");
	m_Print3D.Print3D(55.0f, 95.0f, 0.5f, 0xFFFFFFFF, "Lit");
	m_Print3D.Flush();
	return true;
}

/*!***************************************************************************
@Function		DrawAxisAlignedQuad
@Input			afLowerLeft		Lower left corner of the quad in normalized device coordinates
@Input			afUpperRight	Upper right corner of the quad in normalized device coordinates
@Description	Renders an axis-aligned quad
*****************************************************************************/
void OGLES2MultipleRenderTargets::DrawAxisAlignedQuad(PVRTVec2 afLowerLeft, PVRTVec2 afUpperRight, CPVRTPFXEffect *pEffect)
{	
	// Enable vertex arributes
	glEnableVertexAttribArray(VERTEX_ARRAY);
	glEnableVertexAttribArray(TEXCOORD_ARRAY);

	const float afVertexData[] = { afLowerLeft.x, afLowerLeft.y,  afUpperRight.x, afLowerLeft.y,  
		                           afLowerLeft.x, afUpperRight.y,  afUpperRight.x, afUpperRight.y };
	const float afTexCoordData[] = { 0, 0,  1, 0,  0, 1,  1, 1 };

	const CPVRTArray<SPVRTPFXUniform>& Uniforms = pEffect->GetUniformArray();
	for(unsigned int j = 0; j < Uniforms.GetSize(); ++j)
	{
		switch(Uniforms[j].nSemantic)
		{
		case ePVRTPFX_UsPOSITION:
			glVertexAttribPointer(VERTEX_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, afVertexData);
			break;
		case ePVRTPFX_UsUV:
			glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, afTexCoordData);
			break;		
		case ePVRTPFX_UsTEXTURE:
			glUniform1i(Uniforms[j].nLocation, Uniforms[j].nIdx);
			break;			
		default:
			PVRShellOutputDebug("Error: Unhandled semantic in DrawAxisAlignedQuad()\n");			
		}
	}
	
	// Draw the quad
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// Disable vertex arributes
	glDisableVertexAttribArray(VERTEX_ARRAY);
	glDisableVertexAttribArray(TEXCOORD_ARRAY);
}

/*!****************************************************************************
 @Function		LoadPFX
  @Return		bool			true if no error occurred
 @Description	Loads and compiles the shaders and links the shader programs
				required for this training course
******************************************************************************/
bool OGLES2MultipleRenderTargets::LoadPFX(CPVRTString *pErrorMsg)
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
		m_ppPFXEffects[i] = new CPVRTPFXEffect();

		unsigned int nUnknownUniformCount = 0;
		if(m_ppPFXEffects[i]->Load(*m_pPFXEffectParser, m_pPFXEffectParser->GetEffect(i).Name.c_str(), NULL, this, nUnknownUniformCount, &error)  != PVR_SUCCESS)
		{
			*pErrorMsg = CPVRTString("Failed to load effect ") + m_pPFXEffectParser->GetEffect(i).Name.String() + CPVRTString(":\n\n") + error;
			return false;
		}

		// .. upps, some uniforms are not in our table. Better to quit because something is not quite right.
		if(nUnknownUniformCount)
		{
			*pErrorMsg = CPVRTString("Unknown uniforms found in effect: ") + m_pPFXEffectParser->GetEffect(i).Name.String();
			return false;
		}		
	}
	
	return true;
}

/*!****************************************************************************
 @Function		Update
 @Description	Handles user input and updates all timing data.
******************************************************************************/
void OGLES2MultipleRenderTargets::Update()
{
	// Calculates the frame number to animate in a time-based manner.
	// Uses the shell function PVRShellGetTime() to get the time in milliseconds.
	static unsigned long ulTimePrev = PVRShellGetTime();
	unsigned long ulTime = PVRShellGetTime();
	unsigned long ulDeltaTime = ulTime - ulTimePrev;
	ulTimePrev = ulTime;
	static float fFrame = 0;
	fFrame += (float)ulDeltaTime * 0.05f;

	if (fFrame > m_Scene.nNumFrame-1)
		fFrame = 0;

	// Update the animation data
	m_Scene.SetFrame(fFrame);

	PVRTVec3 vFrom, vTo, vUp;	
	float fFOV = m_Scene.GetCamera(vFrom, vTo, vUp, 0) * 0.75f;
		
	m_mProjection = PVRTMat4::PerspectiveFovRH(fFOV, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), m_Scene.pCamera[0].fNear, m_Scene.pCamera[0].fFar, PVRTMat4::OGL, m_bRotate);
	m_mView = PVRTMat4::LookAtRH(vFrom, vTo, vUp);
	
	PVRTVec3 vPos;
	m_Scene.GetLight(vPos, m_vLightDirection, 0);
}


/*!****************************************************************************
 @Function		RenderSceneWithEffect
 @Return		bool		true if no error occured
 @Description	Renders the whole scene with a single effect.
******************************************************************************/
bool OGLES2MultipleRenderTargets::RenderSceneWithEffect(const int uiEffectId, const PVRTMat4 &mProjection, const PVRTMat4 &mView)
{
	CPVRTPFXEffect *pEffect = m_ppPFXEffects[uiEffectId];

	// Activate the passed effect
	pEffect->Activate();
	
	for (unsigned int i=0; i < m_Scene.nNumMeshNode; i++)
	{
		SPODNode* pNode = &m_Scene.pNode[i];
		SPODMesh* pMesh = &m_Scene.pMesh[pNode->nIdx];
		SPODMaterial *pMaterial = &m_Scene.pMaterial[pNode->nIdxMaterial];	
		
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
					glUniform4f(Uniforms[j].nLocation, pMaterial->pfMatDiffuse[0], pMaterial->pfMatDiffuse[1], pMaterial->pfMatDiffuse[2], 1.0f);
				}
				break;			
			case ePVRTPFX_UsWORLDVIEWPROJECTION:
				{
					PVRTMat4 mWorldViewProj = mProjection * mWorldView;					
					glUniformMatrix4fv(Uniforms[j].nLocation, 1, GL_FALSE, mWorldViewProj.f);
				}
				break;
			case ePVRTPFX_UsVIEW:
				{
					glUniformMatrix4fv(Uniforms[j].nLocation, 1, GL_FALSE, m_mView.f);
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
			case ePVRTPFX_UsLIGHTDIREYE:
				{				
					PVRTVec4 vLightDir = (m_mView * PVRTVec4(m_vLightDirection, 0.0f)) * -1.0f;
					glUniform3fv(Uniforms[j].nLocation, 1, vLightDir.ptr());
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
	return new OGLES2MultipleRenderTargets();
}

/******************************************************************************
 End of file (OGLES2MultipleRenderTargets.cpp)
******************************************************************************/

