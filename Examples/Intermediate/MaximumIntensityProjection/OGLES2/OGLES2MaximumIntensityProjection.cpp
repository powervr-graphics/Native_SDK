/******************************************************************************
 
 @File         OGLES2MaximumIntensityBlend.cpp
 
 @Title        Maximum Intensity Blend
 
 @Version
 
 @Copyright    Copyright (c) Imagination Technologies Limited.
 
 @Platform     Independent
 
 @Description  Demonstrates EXT_blend_minmax
 
 ******************************************************************************/
#include <string.h>

#include "PVRShell.h"
#include "OGLES2Tools.h"

/******************************************************************************
 Constants
 ******************************************************************************/
// PFX semanitcs
enum ECustomSematic
{
	eUsINTENSITY = ePVRTPFX_NumSemantics
};

SPVRTPFXUniformSemantic c_sCustomSemantics[] =
{
	{ "INTENSITY", eUsINTENSITY },
};

// Camera constants used to generate the projection matrix
const float CAM_NEAR	= 1.0f;
const float CAM_FAR		= 2000.0f;

const float DEMO_FRAME_RATE	= 1.f / 60.f;

const char* c_pszTitle      = "MaximumIntensityProjection";
const char* c_pszTitleShort = "MaxIntensityProjection";

/******************************************************************************
 Content file names
 ******************************************************************************/

// Effect file
const char c_szPfxFile[]			= "effect.pfx";

// POD scene files
const char c_szSceneFile[]			= "blend_minmax_scene.POD";

/*!****************************************************************************
 Class implementing the PVRShell functions.
 ******************************************************************************/
class OGLES2MaximumIntensityBlend : public PVRShell, public PVRTPFXEffectDelegate
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;
	
	// 3D Model
	CPVRTModelPOD	m_Scene;
	
	// Projection and Model View matrices
	PVRTMat4		m_mProjection, m_mView;
	
	// Variables to handle the animation in a time-based manner
	unsigned long	m_ulTimePrev;
	float			m_fFrame;
	
	// The effect file handlers
	CPVRTPFXParser	*m_pEffectParser;
	CPVRTPFXEffect	*m_pEffect;
	CPVRTPFXEffect	*m_pEffectTextured;
	
	// The Vertex buffer object handle array.
	GLuint			*m_aiVboID;
	
public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();
	
protected:
	virtual EPVRTError PVRTPFXOnLoadTexture(const CPVRTStringHash& TextureName, GLuint& uiHandle, unsigned int& uiFlags);
	
private:
	void LoadVBOs();
	void DrawMesh(SPODMesh* mesh);
	void ProcessTexturedEffect();
	void ProcessEffect();
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
bool OGLES2MaximumIntensityBlend::InitApplication()
{
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
	
	// The cameras are stored in the file. We check it contains at least one.
	if(m_Scene.nNumCamera == 0)
	{
		PVRShellSet(prefExitMessage, "ERROR: The scene does not contain a camera\n");
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
	m_fFrame      = 0;
	m_ulTimePrev  = PVRShellGetTime();
	
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
bool OGLES2MaximumIntensityBlend::QuitApplication()
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
bool OGLES2MaximumIntensityBlend::InitView()
{
	/*
	 Check that EXT_blend_minmax is supported
	 */
	if(!CPVRTgles2Ext::IsGLExtensionSupported("GL_EXT_blend_minmax"))
	{
		PVRShellSet(prefExitMessage, "ERROR: GL_EXT_blend_minmax extension is required to run this example.");
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
	
	// Sets the clear color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	
	// Load the VBOs
	LoadVBOs();
	
	/*
	 Load the effect file
	 */
	CPVRTString	error;
	unsigned int uiUnknownUniforms;
	
	// Parse the file
	m_pEffectParser = new CPVRTPFXParser;
	if(m_pEffectParser->ParseFromFile(c_szPfxFile, &error) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, error.c_str());
		return false;
	}
	
	// --- Load an effect from the file
	m_pEffect = new CPVRTPFXEffect();
	m_pEffect->RegisterUniformSemantic(c_sCustomSemantics, sizeof(c_sCustomSemantics) / sizeof(c_sCustomSemantics[0]), &error);
	
	/*
	 Load the effect.
	 */
	if(m_pEffect->Load(*m_pEffectParser, "Effect", c_szPfxFile, NULL, uiUnknownUniforms, &error) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, error.c_str());
		return false;
	}
	
	// --- Load the textured effect
	m_pEffectTextured = new CPVRTPFXEffect();
	m_pEffectTextured->RegisterUniformSemantic(c_sCustomSemantics, sizeof(c_sCustomSemantics) / sizeof(c_sCustomSemantics[0]), &error);
	
	/*
	 Load the effect.
	 */
	if(m_pEffectTextured->Load(*m_pEffectParser, "TexturedEffect", c_szPfxFile, this, uiUnknownUniforms, &error) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, error.c_str());
		return false;
	}
	
	return true;
}

/*!***************************************************************************
 @Function		LoadVBOs
 @Description	Generates and populates VBOs for the mesh elements.
 *****************************************************************************/
void OGLES2MaximumIntensityBlend::LoadVBOs()
{
	// Create buffer objects.
	m_aiVboID = new GLuint[m_Scene.nNumMeshNode];
	glGenBuffers(m_Scene.nNumMeshNode, m_aiVboID);
	for(int i = 0; i < (int)m_Scene.nNumMeshNode ; i++)
	{
		SPODNode* pNode = &m_Scene.pNode[i];
		
		// Gets pMesh referenced by the pNode
		SPODMesh* pMesh = &m_Scene.pMesh[pNode->nIdx];
		
		// Generate a vertex buffer and set the interleaved vertex data.
		glBindBuffer(GL_ARRAY_BUFFER, m_aiVboID[i]);
		glBufferData(GL_ARRAY_BUFFER, pMesh->sVertex.nStride*pMesh->nNumVertex, pMesh->pInterleaved, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

/*!***************************************************************************
 @Function		PVRTPFXOnLoadTexture
 @Input			TextureName
 @Output		uiHandle
 @Output		uiFlags
 @Return		EPVRTError			PVR_SUCCESS on success.
 @Description	Callback for texture load.
 *****************************************************************************/
EPVRTError OGLES2MaximumIntensityBlend::PVRTPFXOnLoadTexture(const CPVRTStringHash& TextureName, GLuint& uiHandle, unsigned int& uiFlags)
{
	return PVRTTextureLoadFromPVR(TextureName.String().c_str(), &uiHandle);
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
                application quits or before a change in the rendering context.
 ******************************************************************************/
bool OGLES2MaximumIntensityBlend::ReleaseView()
{
	// Release textures
	{
		const CPVRTArray<SPVRTPFXTexture>& sTex = m_pEffect->GetTextureArray();
		for(unsigned int i = 0; i < sTex.GetSize(); ++i)
		{
			glDeleteTextures(1, &sTex[i].ui);
		}
	}
	
	// Release the effect[s] then the parser
	delete m_pEffect;
	delete m_pEffectTextured;
	delete m_pEffectParser;
	
	// Release Print3D Textures
	m_Print3D.ReleaseTextures();
	
	// Release Vertex buffer objects.
	glDeleteBuffers(m_Scene.nNumMeshNode, m_aiVboID);
	delete[] m_aiVboID;
	
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
bool OGLES2MaximumIntensityBlend::RenderScene()
{
	// Clears the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Enable blending
	glEnable(GL_BLEND);
	glBlendEquation(GL_MAX_EXT);
	
	/*
	 Calculates the frame number to animate in a time-based manner.
	 Uses the shell function PVRShellGetTime() to get the time in milliseconds.
	 */
	unsigned long ulTime = PVRShellGetTime();
	unsigned long ulDeltaTime = ulTime - m_ulTimePrev;
	m_ulTimePrev	= ulTime;
	m_fFrame	+= (float)ulDeltaTime * DEMO_FRAME_RATE;
	
	if (m_fFrame > m_Scene.nNumFrame-1)
		m_fFrame = 0;	
	
	// Sets the scene animation to this frame
	m_Scene.SetFrame(m_fFrame);
	PVRTVec3 vLightDir;
	
	{
		PVRTVec3	vFrom, vTo, vUp;
		VERTTYPE	fFOV;
		vUp.x = 0.0f;
		vUp.y = 1.0f;
		vUp.z = 0.0f;
		
		// We can get the camera position, target and field of view (fov) with GetCameraPos()
		fFOV = m_Scene.GetCameraPos(vFrom, vTo, 0) * 0.6f;
		
		/*
		 We can build the world view matrix from the camera position, target and an up vector.
		 For this we use PVRTMat4LookAtRH().
		 */
		m_mView = PVRTMat4::LookAtRH(vFrom, vTo, vUp);
		
		vLightDir = vFrom;
		
		// Calculates the projection matrix
		bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);
		m_mProjection = PVRTMat4::PerspectiveFovRH(fFOV, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), CAM_NEAR, CAM_FAR, PVRTMat4::OGL, bRotate);
	}
	
	/*
	 A scene is composed of nodes. There are 3 types of nodes:
	 - MeshNodes :
	 references a mesh in the pMesh[].
	 These nodes are at the beginning of the pNode[] array.
	 And there are nNumMeshNode number of them.
	 This way the .pod format can instantiate several times the same mesh
	 with different attributes.
	 - lights
	 - cameras
	 To draw a scene, you must go through all the MeshNodes and draw the referenced meshes.
	 */
	
	for (int i=0; i<(int)m_Scene.nNumMeshNode; i++)
	{
		SPODNode* pNode = &m_Scene.pNode[i];
		
		// Gets pMesh referenced by the pNode
		SPODMesh* pMesh = &m_Scene.pMesh[pNode->nIdx];
		
		glBindBuffer(GL_ARRAY_BUFFER, m_aiVboID[i]);
		
		// Gets the node model matrix
		PVRTMat4 mWorld;
		mWorld = m_Scene.GetWorldMatrix(*pNode);
		
		PVRTMat4 mWorldView;
		mWorldView = m_mView * mWorld;
		
		// Retrieve the list of required uniforms
		CPVRTPFXEffect* pEffect;
		
		SPODMaterial* pMat = &m_Scene.pMaterial[pNode->nIdxMaterial];
		if(pMat->nIdxTexDiffuse != -1)
		{
			pEffect = m_pEffectTextured;
		}
		else
		{
			pEffect = m_pEffect;
		}
		
		pEffect->Activate();
		const CPVRTArray<SPVRTPFXUniform>& aUniforms = pEffect->GetUniformArray();
		
		/*
		 Now we loop over the uniforms requested by the PFX file.
		 Using the switch statement allows us to handle all of the required semantics
		 */
		for(unsigned int j = 0; j < aUniforms.GetSize(); ++j)
		{
			switch(aUniforms[j].nSemantic)
			{
				case ePVRTPFX_UsPOSITION:
				{
					glVertexAttribPointer(aUniforms[j].nLocation, 3, GL_FLOAT, GL_FALSE, pMesh->sVertex.nStride, pMesh->sVertex.pData);
					glEnableVertexAttribArray(aUniforms[j].nLocation);
				}
					break;
				case ePVRTPFX_UsNORMAL:
				{
					glVertexAttribPointer(aUniforms[j].nLocation, 3, GL_FLOAT, GL_FALSE, pMesh->sNormals.nStride, pMesh->sNormals.pData);
					glEnableVertexAttribArray(aUniforms[j].nLocation);
				}
					break;
				case ePVRTPFX_UsUV:
				{
					glVertexAttribPointer(aUniforms[j].nLocation, 2, GL_FLOAT, GL_FALSE, pMesh->psUVW[0].nStride, pMesh->psUVW[0].pData);
					glEnableVertexAttribArray(aUniforms[j].nLocation);
				}
					break;
				case ePVRTPFX_UsWORLDVIEWPROJECTION:
				{
					PVRTMat4 mWVP;
					
					// Passes the world-view-projection matrix (WVP) to the shader to transform the vertices
					mWVP = m_mProjection * mWorldView;
					glUniformMatrix4fv(aUniforms[j].nLocation, 1, GL_FALSE, mWVP.f);
				}
					break;
				case eUsINTENSITY:
				{
					int iMat           = pNode->nIdxMaterial;
					SPODMaterial* pMat = &m_Scene.pMaterial[iMat];
					float fIntensity   = pMat->pfMatDiffuse[0];			// Take R value for intensity
					
					glUniform1f(aUniforms[j].nLocation, fIntensity);
				}
					break;
				case ePVRTPFX_UsTEXTURE:
				{
					glUniform1i(aUniforms[j].nLocation, 0);
				}
					break;
				case ePVRTPFX_UsWORLDVIEWIT:
				{
					PVRTMat3 mWorldViewIT3x3(mWorldView.inverse().transpose());
					glUniformMatrix3fv(aUniforms[j].nLocation, 1, GL_FALSE, mWorldViewIT3x3.f);
				}
					break;
				case ePVRTPFX_UsLIGHTDIREYE:
				{
					PVRTVec4 vLightDirView = (m_mView * PVRTVec4(-vLightDir, 1.0f)).normalize();
					glUniform3fv(aUniforms[j].nLocation, 1, vLightDirView.ptr());
				}
					break;
			}
		}
		
		/*
		 Now that the model-view matrix is set and the materials ready,
		 call another function to actually draw the mesh.
		 */
		DrawMesh(pMesh);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		/*
		 Now disable all of the enabled attribute arrays that the PFX requested.
		 */
		for(unsigned int j = 0; j < aUniforms.GetSize(); ++j)
		{
			switch(aUniforms[j].nSemantic)
			{
				case ePVRTPFX_UsNORMAL:
				case ePVRTPFX_UsUV:
				case ePVRTPFX_UsPOSITION:
				{
					glDisableVertexAttribArray(aUniforms[j].nLocation);
				}
					break;
			}
		}
	}
	
	// Reset blending
	// Enable blending
	glBlendEquation(GL_FUNC_ADD);
	glDisable(GL_BLEND);
	
	// Determine which title to show. The default title is quite long, so we display a shortened version if
	// it cannot fit on the screen.
	const char* pszTitle = NULL;
	{
		bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);
		
		float fW, fH;
		m_Print3D.MeasureText(&fW, &fH, 1.0f, c_pszTitle);
		
		int iScreenW = bRotate ? PVRShellGet(prefHeight) : PVRShellGet(prefWidth);
		if((int)fW >= iScreenW)
		{
			pszTitle = c_pszTitleShort;
		}
		else
		{
			pszTitle = c_pszTitle;
		}
	}
	
	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle(pszTitle, "", ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();
	
	return true;
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			mesh		The mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set and
 the meterial prepared.
 ******************************************************************************/
void OGLES2MaximumIntensityBlend::DrawMesh(SPODMesh* pMesh)
{
	/*
	 Submit geometry as an indexed triangle list.
	 
	 "IntroducingPOD" demonstrates how to support all possible formats
	 (e.g. non-indexed, or stripped).
	 */
	glDrawElements(GL_TRIANGLES, pMesh->nNumFaces*3, GL_UNSIGNED_SHORT, pMesh->sFaces.pData);
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
	return new OGLES2MaximumIntensityBlend();
}

/******************************************************************************
 End of file (OGLES2MaximumIntensityBlend.cpp)
 ******************************************************************************/

