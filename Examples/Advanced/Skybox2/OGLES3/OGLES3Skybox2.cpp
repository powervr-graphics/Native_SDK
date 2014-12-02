/******************************************************************************

 @File         OGLES3Skybox2.cpp

 @Title        Introducing the POD 3d file format

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows how to load POD files and play the animation with basic
               lighting

******************************************************************************/
#include <string.h>
#include <math.h>

#include "PVRShell.h"
#include "OGLES3Tools.h"

/******************************************************************************
 Constants
******************************************************************************/
const float g_fFrameRate = 1.0f / 30.0f;
const unsigned int g_ui32NoOfEffects = 8;
const unsigned int g_ui32TexNo = 5;

const bool g_bBlendShader[g_ui32NoOfEffects] = {
	false,
	false,
	false,
	false,
	true,
	false,
	false,
	true
};

/******************************************************************************
 Content file names
******************************************************************************/

// POD scene files
const char c_szSceneFile[] = "Scene.pod";

// Textures
const char * const g_aszTextureNames[g_ui32TexNo] = {
	"Balloon.pvr",
	"Balloon_pvr.pvr",
	"Noise.pvr",
	"Skybox.pvr",
	"SkyboxMidnight.pvr"
};

// PFX file
const char * const g_pszEffectFileName = "effects.pfx";

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES3Skybox2 : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// IDs for the various textures
	GLuint m_ui32TextureIDs[g_ui32TexNo];

	// 3D Model
	CPVRTModelPOD	m_Scene;

	// Projection and Model View matrices
	PVRTMat4		m_mProjection, m_mView;

	// Variables to handle the animation in a time-based manner
	unsigned long	m_iTimePrev;
	float			m_fFrame;

	// The effect currently being displayed
	int m_i32Effect;

	// The Vertex buffer object handle array.
	GLuint			*m_aiVboID;
	GLuint			m_iSkyVboID;

	/* View Variables */
	float fViewAngle;
	float fViewDistance, fViewAmplitude, fViewAmplitudeAngle;
	float fViewUpDownAmplitude, fViewUpDownAngle;

	/* Vectors for calculating the view matrix and saving the camera position*/
	PVRTVec3 vTo, vUp, vCameraPosition;

	//animation
	float fBurnAnim;

	bool bPause;
	float fDemoFrame;

	// The variables required for the effects
	CPVRTPFXParser*		m_pEffectParser;
	CPVRTPFXEffect**	m_ppEffects;

public:
	OGLES3Skybox2() : 	m_iTimePrev(0),
						m_fFrame(0),
						m_i32Effect(0),
						m_aiVboID(0),
						fViewAngle(PVRT_PI_OVER_TWO),
						fViewDistance(100.0f),
						fViewAmplitude(60.0f),
						fViewAmplitudeAngle(0.0f),
						fViewUpDownAmplitude(50.0f),
						fViewUpDownAngle(0.0f),
						fBurnAnim(0),
						bPause(false),
						fDemoFrame(0),
						m_pEffectParser(0),
						m_ppEffects(0)
	{
		// Init values to defaults
		vTo.x = 0;
		vTo.y = 0;
		vTo.z = 0;

		vUp.x = 0;
		vUp.y = 1;
		vUp.z = 0;
	}

	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	void DrawMesh(SPODMesh* mesh);
	void ComputeViewMatrix();
	void DrawSkybox();
	bool LoadEffect(CPVRTPFXEffect **ppEffect, const char * pszEffectName, const char *pszFileName);
	bool LoadTextures(CPVRTString* const pErrorStr);
	bool DestroyEffect(CPVRTPFXEffect **ppEffect);
	void ChangeSkyboxTo(CPVRTPFXEffect *pEffect, GLuint ui32NewSkybox);
};


/*!****************************************************************************
 @Function		InitApplication
 @Return		bool		true if no error occurred
 @Description	Code in InitApplication() will be called by PVRShell once per
				run, before the rendering context is created.
				Used to initialize variables that are not dependent on it
				(e.g. external modules, loading meshes, etc.)
				If the rendering context is lost, InitApplication() will
				not be called again.
******************************************************************************/
bool OGLES3Skybox2::InitApplication()
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

	return true;
}

/*!****************************************************************************
 @Function		QuitApplication
 @Return		bool		true if no error occurred
 @Description	Code in QuitApplication() will be called by PVRShell once per
				run, just before exiting the program.
				If the rendering context is lost, QuitApplication() will
				not be called.
******************************************************************************/
bool OGLES3Skybox2::QuitApplication()
{
	// Frees the memory allocated for the scene
	m_Scene.Destroy();

    return true;
}

/*!***************************************************************************
@Function		LoadTextures
@Input			pErrorStr
@Return			bool	
@Description	Loads all of the required textures.
*****************************************************************************/
bool OGLES3Skybox2::LoadTextures(CPVRTString* const pErrorStr)
{
	for(int i = 0; i < 3; ++i)
	{
		if(PVRTTextureLoadFromPVR(g_aszTextureNames[i], &m_ui32TextureIDs[i]) != PVR_SUCCESS)
			return false;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	// Load cube maps
	for(int i = 3; i < 5; ++i)
	{
		if(PVRTTextureLoadFromPVR(g_aszTextureNames[i], &m_ui32TextureIDs[i]))
		{
			*pErrorStr = CPVRTString("ERROR: Could not open texture file ") + g_aszTextureNames[i];
			return false;
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	return true;
}
/*!****************************************************************************
 @Function		InitView
 @Return		bool		true if no error occurred
 @Description	Code in InitView() will be called by PVRShell upon
				initialization or after a change in the rendering context.
				Used to initialize variables that are dependent on the rendering
				context (e.g. textures, vertex buffers, etc.)
******************************************************************************/
bool OGLES3Skybox2::InitView()
{
	// Sets the clear colour
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	// Enables depth test using the z-buffer
	glEnable(GL_DEPTH_TEST);

	CPVRTString ErrorStr;

	/*
		Load textures
	*/
	if(!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	/*********************/
	/* Create the Skybox */
	/*********************/
	float* skyboxVertices;
	float* skyboxUVs;

	PVRTCreateSkybox( 500.0f, true, 512, &skyboxVertices, &skyboxUVs );

	glGenBuffers(1, &m_iSkyVboID);
	glBindBuffer(GL_ARRAY_BUFFER, m_iSkyVboID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 24, &skyboxVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	PVRTDestroySkybox(skyboxVertices, skyboxUVs);

	/**********************/
	/* Create the Effects */
	/**********************/

	{
		// Parse the file
		m_pEffectParser = new CPVRTPFXParser();

		if(m_pEffectParser->ParseFromFile(g_pszEffectFileName, &ErrorStr) != PVR_SUCCESS)
		{
			delete m_pEffectParser;
			PVRShellSet(prefExitMessage, ErrorStr.c_str());
			return false;
		}

		m_ppEffects = new CPVRTPFXEffect*[g_ui32NoOfEffects];
		memset(m_ppEffects, 0, sizeof(CPVRTPFXEffect*) * g_ui32NoOfEffects);

		// Skybox shader
		if(!LoadEffect(&m_ppEffects[0], "skybox_effect", g_pszEffectFileName))
		{
			delete m_pEffectParser;
			delete[] m_ppEffects;
			return false;
		}

		// The Balloon Shaders
		if(!LoadEffect(&m_ppEffects[1], "balloon_effect1", g_pszEffectFileName) ||
			!LoadEffect(&m_ppEffects[2], "balloon_effect2", g_pszEffectFileName) ||
			!LoadEffect(&m_ppEffects[3], "balloon_effect3", g_pszEffectFileName) ||
			!LoadEffect(&m_ppEffects[4], "balloon_effect4", g_pszEffectFileName) ||
			!LoadEffect(&m_ppEffects[5], "balloon_effect5", g_pszEffectFileName) ||
			!LoadEffect(&m_ppEffects[6], "balloon_effect6", g_pszEffectFileName) ||
			!LoadEffect(&m_ppEffects[7], "balloon_effect7", g_pszEffectFileName))
		{
			delete m_pEffectParser;
			delete[] m_ppEffects;
			return false;
		}
	}

	// Create Geometry Buffer Objects.
	m_aiVboID = new GLuint[m_Scene.nNumMeshNode];
	glGenBuffers(m_Scene.nNumMeshNode, m_aiVboID);

	for(unsigned int i = 0; i < m_Scene.nNumMeshNode ; ++i)
	{
		SPODNode* pNode = &m_Scene.pNode[i];

		// Gets pMesh referenced by the pNode
		SPODMesh* pMesh = &m_Scene.pMesh[pNode->nIdx];

		// Genereta a vertex buffer and set the interleaved vertex datas.
		glBindBuffer(GL_ARRAY_BUFFER, m_aiVboID[i]);
		glBufferData(GL_ARRAY_BUFFER, pMesh->sVertex.nStride*pMesh->nNumVertex, pMesh->pInterleaved, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

	}

	/**********************
	** Projection Matrix **
	**********************/
	/* Projection */
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);
	m_mProjection = PVRTMat4::PerspectiveFovRH(PVRT_PI / 6, (float) PVRShellGet(prefWidth) / (float) PVRShellGet(prefHeight), 4.0f, 1000.0f, PVRTMat4::OGL, bRotate);

	// Calculate the model view matrix turning around the balloon
	ComputeViewMatrix();

	/* Init Values */
	bPause = false;
	fDemoFrame = 0.0;
	fBurnAnim = 0.0f;

	m_i32Effect = 1;

	// Initialise Print3D
	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	// Initialise variables used for the animation
	m_iTimePrev = PVRShellGetTime();

	return true;
}

/*!***************************************************************************
@Function		LoadEffect
@Output			ppEffect
@Input			pszEffectName
@Input			pszFileName
@Return			bool	
@Description	Loads a PFX Effect.
*****************************************************************************/
bool OGLES3Skybox2::LoadEffect( CPVRTPFXEffect **ppEffect, const char * pszEffectName, const char *pszFileName )
{
	if(!ppEffect)
		return false;

	unsigned int	nUnknownUniformCount;
	CPVRTString		error;
	CPVRTPFXEffect* pEffect = *ppEffect;

	// Load an effect from the file
	if(!pEffect)
	{
		*ppEffect = new CPVRTPFXEffect();
		pEffect   = *ppEffect;

		if(!pEffect)
		{
			delete m_pEffectParser;
			PVRShellSet(prefExitMessage, "Failed to create effect.\n");
			return false;
		}
	}

	if(pEffect->Load(*m_pEffectParser, pszEffectName, pszFileName, NULL, nUnknownUniformCount, &error) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, error.c_str());
		return false;
	}

	if(nUnknownUniformCount)
	{
		PVRShellOutputDebug(error.c_str());
		PVRShellOutputDebug("Unknown uniform semantic count: %d\n", nUnknownUniformCount);
	}

	/* Set the textures for each effect */
	unsigned int			i,j ;
	const CPVRTArray<SPVRTPFXTexture>& sTex = pEffect->GetTextureArray();

	for(i = 0; i < sTex.GetSize(); ++i)
	{
		for(j = 0; j < g_ui32TexNo; ++j)
		{
			int iTexIdx = m_pEffectParser->FindTextureByName(sTex[i].Name);
			const CPVRTStringHash& FileName = m_pEffectParser->GetTexture(iTexIdx)->FileName;
			if(FileName == g_aszTextureNames[j])
			{
				if(j == 3 || j == 4)
					pEffect->SetTexture(i, m_ui32TextureIDs[j], PVRTEX_CUBEMAP);
				else
					pEffect->SetTexture(i, m_ui32TextureIDs[j]);
				break;
			}
		}
	}

	return true;
}

/*!***************************************************************************
@Function		DestroyEffect
@Output			ppEffect
@Return			bool	
@Description	Deletes an effect and releases resources.
*****************************************************************************/
bool OGLES3Skybox2::DestroyEffect(CPVRTPFXEffect **ppEffect)
{
	if(ppEffect)
	{
		if(*ppEffect)
		{
			const CPVRTArray<SPVRTPFXTexture>& sTex = (*ppEffect)->GetTextureArray();

			for(unsigned int i = 0; i < sTex.GetSize(); ++i)
			{
				glDeleteTextures(1, &(sTex[i].ui));
			}

			delete *ppEffect;
			*ppEffect = NULL;
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
bool OGLES3Skybox2::ReleaseView()
{
	// Free the textures
	unsigned int i;

	for(i = 0; i < g_ui32TexNo; ++i)
	{
		glDeleteTextures(1, &(m_ui32TextureIDs[i]));
	}

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

	// Release Vertex buffer objects.
	glDeleteBuffers(m_Scene.nNumMeshNode, m_aiVboID);
	glDeleteBuffers(1, &m_iSkyVboID);
	delete[] m_aiVboID;

	// Destroy the effects
	for(i = 0; i < g_ui32NoOfEffects; ++i)
		DestroyEffect(&m_ppEffects[i]);

	delete[] m_ppEffects;
	delete m_pEffectParser;

	return true;
}

/*!****************************************************************************
 @Function		RenderScene
 @Return		bool		true if no error occurred
 @Description	Main rendering loop function of the program. The shell will
				call this function every frame.
				eglSwapBuffers() will be performed by PVRShell automatically.
				PVRShell will also manage important OS events.
				Will also manage relevant OS events. The user has access to
				these events through an abstraction layer provided by PVRShell.
******************************************************************************/
bool OGLES3Skybox2::RenderScene()
{
	unsigned int i, j;

	// Clears the colour and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/*
		Calculates the frame number to animate in a time-based manner.
		Uses the shell function PVRShellGetTime() to get the time in milliseconds.
	*/

	unsigned long iTime = PVRShellGetTime();

	if(!bPause)
	{
		// Calculate the model view matrix turning around the balloon
		ComputeViewMatrix();

		if(iTime > m_iTimePrev)
		{
			float fDelta = (float) (iTime - m_iTimePrev) * g_fFrameRate;
			m_fFrame   += fDelta;
			fDemoFrame += fDelta;
			fBurnAnim  += fDelta * 0.02f;

			if(fBurnAnim >= 1.0f)
				fBurnAnim = 1.0f;
		}
	}

	m_iTimePrev	= iTime;

	/* KeyBoard input processing */

	if(PVRShellIsKeyPressed(PVRShellKeyNameACTION1))
		bPause=!bPause;

	if(PVRShellIsKeyPressed(PVRShellKeyNameACTION2))
		fBurnAnim = 0.0f;

	/* Keyboard Animation and Automatic Shader Change over time */
	if(!bPause && (fDemoFrame > 500 || (m_i32Effect == 2 && fDemoFrame > 80)))
	{
		if(++m_i32Effect >= (int) g_ui32NoOfEffects)
		{
			m_i32Effect = 1;
			m_fFrame = 0.0f;
		}

		fDemoFrame = 0.0f;
		fBurnAnim  = 0.0f;
	}

	/* Change Shader Effect */

	if(PVRShellIsKeyPressed(PVRShellKeyNameRIGHT))
	{
		if(++m_i32Effect >= (int) g_ui32NoOfEffects)
			m_i32Effect = 1;

		fDemoFrame = 0.0f;
		fBurnAnim  = 0.0f;
		m_fFrame = 0.0f;
	}
	if(PVRShellIsKeyPressed(PVRShellKeyNameLEFT))
	{
		if(--m_i32Effect < 1)
			m_i32Effect = g_ui32NoOfEffects - 1;

		fDemoFrame = 0.0f;
		fBurnAnim  = 0.0f;
		m_fFrame = 0.0f;
	}

	/* Change Skybox Texture */
	if(PVRShellIsKeyPressed(PVRShellKeyNameUP))
	{
		for(i = 0; i < g_ui32NoOfEffects; ++i)
			ChangeSkyboxTo(m_ppEffects[i], m_ui32TextureIDs[4]);

		fBurnAnim = 0.0f;
	}

	if(PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
	{
		for(i = 0; i < g_ui32NoOfEffects; ++i)
			ChangeSkyboxTo(m_ppEffects[i], m_ui32TextureIDs[3]);

		fBurnAnim = 0.0f;
	}

	/* Setup Shader and Shader Constants */
	int location;

	glDisable(GL_CULL_FACE);

	DrawSkybox();

	glEnable(GL_CULL_FACE);

	m_ppEffects[m_i32Effect]->Activate();

	for(i = 0; i < m_Scene.nNumMeshNode; i++)
	{
		SPODNode* pNode = &m_Scene.pNode[i];

		// Gets pMesh referenced by the pNode
		SPODMesh* pMesh = &m_Scene.pMesh[pNode->nIdx];

		// Gets the node model matrix
		PVRTMat4 mWorld, mWORLDVIEW;
		mWorld = m_Scene.GetWorldMatrix(*pNode);

		mWORLDVIEW = m_mView * mWorld;

		glBindBuffer(GL_ARRAY_BUFFER, m_aiVboID[i]);

		const CPVRTArray<SPVRTPFXUniform>& Uniforms = m_ppEffects[m_i32Effect]->GetUniformArray();
		for(j = 0; j < Uniforms.GetSize(); ++j)
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
				case ePVRTPFX_UsWORLDVIEWPROJECTION:
				{
					PVRTMat4 mMVP;

					/* Passes the model-view-projection matrix (MVP) to the shader to transform the vertices */
					mMVP = m_mProjection * mWORLDVIEW;
					glUniformMatrix4fv(Uniforms[j].nLocation, 1, GL_FALSE, mMVP.f);
				}
				break;
				case ePVRTPFX_UsWORLDVIEW:
				{
					glUniformMatrix4fv(Uniforms[j].nLocation, 1, GL_FALSE, mWORLDVIEW.f);
				}
				break;
				case ePVRTPFX_UsWORLDVIEWIT:
				{
					PVRTMat4 mWORLDVIEWI, mWORLDVIEWIT;

					mWORLDVIEWI = mWORLDVIEW.inverse();
					mWORLDVIEWIT= mWORLDVIEWI.transpose();

					PVRTMat3 WORLDVIEWIT = PVRTMat3(mWORLDVIEWIT);

					glUniformMatrix3fv(Uniforms[j].nLocation, 1, GL_FALSE, WORLDVIEWIT.f);
				}
				break;
				case ePVRTPFX_UsVIEWIT:
				{
					PVRTMat4 mViewI, mViewIT;

					mViewI  = m_mView.inverse();
					mViewIT = mViewI.transpose();

					PVRTMat3 ViewIT = PVRTMat3(mViewIT);

					glUniformMatrix3fv(Uniforms[j].nLocation, 1, GL_FALSE, ViewIT.f);
				}
				break;
				case ePVRTPFX_UsLIGHTDIREYE:
				{
					PVRTVec4 vLightDirectionEyeSpace;

					// Passes the light direction in eye space to the shader
					vLightDirectionEyeSpace = m_mView * PVRTVec4(1.0,1.0,-1.0,0.0);
					glUniform3f(Uniforms[j].nLocation, vLightDirectionEyeSpace.x, vLightDirectionEyeSpace.y, vLightDirectionEyeSpace.z);
				}
				break;
				case ePVRTPFX_UsTEXTURE:
				{
					// Set the sampler variable to the texture unit
					glUniform1i(Uniforms[j].nLocation, Uniforms[j].nIdx);
				}
				break;
			}
		}

		location = glGetUniformLocation(m_ppEffects[m_i32Effect]->GetProgramHandle(), "myEyePos");

		if(location != -1)
			glUniform3f(location, vCameraPosition.x, vCameraPosition.y, vCameraPosition.z);

		//set animation
		location = glGetUniformLocation(m_ppEffects[m_i32Effect]->GetProgramHandle(), "fAnim");

		if(location != -1)
			glUniform1f(location, fBurnAnim);

		location = glGetUniformLocation(m_ppEffects[m_i32Effect]->GetProgramHandle(), "myFrame");

		if(location != -1)
			glUniform1f(location, m_fFrame);

		if(g_bBlendShader[m_i32Effect])
		{
			glEnable(GL_BLEND);

			// Correct render order for alpha blending through culling
			// Draw Back faces
			glCullFace(GL_FRONT);

			location = glGetUniformLocation(m_ppEffects[m_i32Effect]->GetProgramHandle(), "bBackFace");

			glUniform1i(location, 1);

			DrawMesh(pMesh);

			glUniform1i(location, 0);

			glCullFace(GL_BACK);
		}
		else
		{
			location = glGetUniformLocation(m_ppEffects[m_i32Effect]->GetProgramHandle(), "bBackFace");

			if(location != -1)
				glUniform1i(location, 0);

			glDisable(GL_BLEND);
		}

		/* Everything should now be setup, therefore draw the mesh*/
		DrawMesh(pMesh);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		for(j = 0; j < Uniforms.GetSize(); ++j)
		{
			switch(Uniforms[j].nSemantic)
			{
			case ePVRTPFX_UsPOSITION:
				{
					glDisableVertexAttribArray(Uniforms[j].nLocation);
				}
				break;
			case ePVRTPFX_UsNORMAL:
				{
					glDisableVertexAttribArray(Uniforms[j].nLocation);
				}
				break;
			case ePVRTPFX_UsUV:
				{
					glDisableVertexAttribArray(Uniforms[j].nLocation);
				}
				break;
			}
		}
	}

	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	if(!bPause)
		m_Print3D.DisplayDefaultTitle("Skybox2", "", ePVRTPrint3DSDKLogo);
	else
		m_Print3D.DisplayDefaultTitle("Skybox2", "Paused", ePVRTPrint3DSDKLogo);

	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			mesh		The mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the meterial prepared.
******************************************************************************/
void OGLES3Skybox2::DrawMesh(SPODMesh* pMesh)
{
	/*
		The geometry can be exported in 4 ways:
		- Non-Indexed Triangle list
		- Indexed Triangle list
		- Non-Indexed Triangle strips
		- Indexed Triangle strips
	*/
	if(!pMesh->nNumStrips)
	{
		if(pMesh->sFaces.pData)
		{
			// Indexed Triangle list
			glDrawElements(GL_TRIANGLES, pMesh->nNumFaces*3, GL_UNSIGNED_SHORT, pMesh->sFaces.pData);
		}
		else
		{
			// Non-Indexed Triangle list
			glDrawArrays(GL_TRIANGLES, 0, pMesh->nNumFaces*3);
		}
	}
	else
	{
		if(pMesh->sFaces.pData)
		{
			// Indexed Triangle strips
			int offset = 0;
			for(int i = 0; i < (int)pMesh->nNumStrips; i++)
			{
				glDrawElements(GL_TRIANGLE_STRIP, pMesh->pnStripLength[i]+2, GL_UNSIGNED_SHORT, pMesh->sFaces.pData + offset*2);
				offset += pMesh->pnStripLength[i]+2;
			}
		}
		else
		{
			// Non-Indexed Triangle strips
			int offset = 0;
			for(int i = 0; i < (int)pMesh->nNumStrips; i++)
			{
				glDrawArrays(GL_TRIANGLE_STRIP, offset, pMesh->pnStripLength[i]+2);
				offset += pMesh->pnStripLength[i]+2;
			}
		}
	}
}

/*******************************************************************************
 * Function Name  : ComputeViewMatrix
 * Description    : Calculate the view matrix turning around the balloon
 *******************************************************************************/
void OGLES3Skybox2::ComputeViewMatrix()
{
	PVRTVec3 vFrom;

	/* Calculate the distance to balloon */
	float fDistance = fViewDistance + fViewAmplitude * (float) sin(fViewAmplitudeAngle);
	fDistance = fDistance / 5.0f;
	fViewAmplitudeAngle += 0.004f;

	/* Calculate the vertical position of the camera */
	float updown = fViewUpDownAmplitude * (float) sin(fViewUpDownAngle);
	updown = updown / 5.0f;
	fViewUpDownAngle += 0.005f;

	/* Calculate the angle of the camera around the balloon */
	vFrom.x = fDistance * (float) cos(fViewAngle);
	vFrom.y = updown;
	vFrom.z = fDistance * (float) sin(fViewAngle);
	fViewAngle += 0.003f;

	/* Compute and set the matrix */
	m_mView = PVRTMat4::LookAtRH(vFrom, vTo, vUp);

	/* Remember the camera position to draw the Skybox around it */
	vCameraPosition = vFrom;
}

/*!***************************************************************************
@Function		ChangeSkyboxTo
@Output			pEffect
@Input			ui32NewSkybox
@Description	Changes the skybox to use the selected effect.
*****************************************************************************/
void OGLES3Skybox2::ChangeSkyboxTo(CPVRTPFXEffect *pEffect, GLuint ui32NewSkybox)
{
	const CPVRTArray<SPVRTPFXTexture>& sTex = pEffect->GetTextureArray();

	for(unsigned int i = 0; i < sTex.GetSize(); ++i)
	{
		int iTexIdx = m_pEffectParser->FindTextureByName(sTex[i].Name);
		const CPVRTStringHash& FileName = m_pEffectParser->GetTexture(iTexIdx)->FileName;

		if(FileName == g_aszTextureNames[3])
		{
			pEffect->SetTexture(i, ui32NewSkybox, PVRTEX_CUBEMAP);
			return;
		}
	}
}

/*******************************************************************************
 * Function Name  : DrawSkybox
 * Description    : Draws the Skybox
 *******************************************************************************/
void OGLES3Skybox2::DrawSkybox()
{
	// Use the loaded Skybox shader program
	m_ppEffects[0]->Activate();

	int iVertexUniform = 0;
	const CPVRTArray<SPVRTPFXUniform>& Uniforms = m_ppEffects[0]->GetUniformArray();
	for(unsigned int j = 0; j < Uniforms.GetSize(); ++j)
	{
		switch(Uniforms[j].nSemantic)
		{
			case ePVRTPFX_UsPOSITION:
			{
				iVertexUniform = j;
				glEnableVertexAttribArray(Uniforms[j].nLocation);
			}
			break;
			case ePVRTPFX_UsWORLDVIEWPROJECTION:
			{
				PVRTMat4 mTrans, mMVP;
				mTrans = PVRTMat4::Translation(-vCameraPosition.x, -vCameraPosition.y, -vCameraPosition.z);


				mMVP = m_mProjection * mTrans * m_mView;

				/* Passes the model-view-projection matrix (MVP) to the shader to transform the vertices */
				glUniformMatrix4fv(Uniforms[j].nLocation, 1, GL_FALSE, mMVP.f);
			}
			break;
			case ePVRTPFX_UsTEXTURE:
			{
				// Set the sampler variable to the texture unit
				glUniform1i(Uniforms[j].nLocation, Uniforms[j].nIdx);
			}
			break;
		}
	}

	// Draw the skybox
	glBindBuffer(GL_ARRAY_BUFFER, m_iSkyVboID);

	for (int i = 0; i < 6; ++i)
	{
		// Set Data Pointers
		size_t offset = sizeof(float) * i*4*3;
		glVertexAttribPointer(Uniforms[iVertexUniform].nLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*) offset);

		// Draw
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	/* Disable States */
	glDisableVertexAttribArray(Uniforms[iVertexUniform].nLocation);

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
	return new OGLES3Skybox2();
}

/******************************************************************************
 End of file (OGLES3Skybox2.cpp)
******************************************************************************/

