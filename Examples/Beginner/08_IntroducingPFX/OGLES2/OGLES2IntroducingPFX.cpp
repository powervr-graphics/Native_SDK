/******************************************************************************

 @File         OGLES2IntroducingPFX.cpp

 @Title        Introducing the PFX file format

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows how to load PFX files

******************************************************************************/
#include <string.h>

#include "PVRShell.h"
#include "OGLES2Tools.h"

/******************************************************************************
 Constants
******************************************************************************/
/*
	PVRTools includes a large number of built-in semantics and should cover most
	situations. However, it's possible to extend default semantics with user
	provided values. In this training course we are extending the default
	semantics by appending a custom 'scale' uniform which will be uses in
	the GLSL code.
*/
enum ECustomSemantic
{
	eUsMYCUSTOMSCALE = ePVRTPFX_NumSemantics		// Be sure to begin at the end of the default semantic list.
													// otherwise an error will be reported.
};

struct SPVRTPFXUniformSemantic c_sCustomSemantics[] =
{
	{ "MYCUSTOMSCALE",		eUsMYCUSTOMSCALE	},
};

// Camera constants used to generate the projection matrix
const float CAM_NEAR	= 75.0f;
const float CAM_FAR		= 2000.0f;

const float DEMO_FRAME_RATE	= 1.f / 30.f;

/******************************************************************************
 Content file names
******************************************************************************/

// Effect file
const char c_szPfxFile[]			= "effect.pfx";

// PVR texture files
const char c_szBaseTexFile[]		= "Basetex.pvr";
const char c_szReflectTexFile[]		= "Reflection.pvr";

// POD scene files
const char c_szSceneFile[]			= "Scene.pod";

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES2IntroducingPFX : public PVRShell, public PVRTPFXEffectDelegate
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

	// The Vertex buffer object handle array.
	GLuint			*m_aiVboID;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	virtual EPVRTError PVRTPFXOnLoadTexture(const CPVRTStringHash& TextureName, GLuint& uiHandle, unsigned int& uiFlags);

	void LoadVBOs();
	void DrawMesh(SPODMesh* mesh);
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
bool OGLES2IntroducingPFX::InitApplication()
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
bool OGLES2IntroducingPFX::QuitApplication()
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
bool OGLES2IntroducingPFX::InitView()
{
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
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	// Enables depth test using the z-buffer
	glEnable(GL_DEPTH_TEST);

	/*
		Loads the light direction from the scene.
	*/
	// We check the scene contains at least one
	if (m_Scene.nNumLight == 0)
	{
		PVRShellSet(prefExitMessage, "ERROR: The scene does not contain a light\n");
		return false;
	}

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

	// Register a custom uniform
	if(m_pEffect->RegisterUniformSemantic(c_sCustomSemantics, sizeof(c_sCustomSemantics) / sizeof(c_sCustomSemantics[0]), &error) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, error.c_str());
		return false;
	}

	/* 
		Load the effect.
		We pass 'this' as an argument as we wish to receive callbacks as the PFX is loaded.
		This is optional and supplying NULL implies that the developer will take care
		of all texture loading and binding to to the Effect instead.
	*/
	if(m_pEffect->Load(*m_pEffectParser, "Effect", c_szPfxFile, this, uiUnknownUniforms, &error) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, error.c_str());
		return false;
	}

	/*
		'Unknown uniforms' are uniform semantics that have been detected in the PFX file
		but are unknown to PVRTools. If you wish to utilise this semantic, register
		the semantic by calling RegisterUniformSemantic(). This is performed above.
	*/
	if(uiUnknownUniforms)
	{
		PVRShellOutputDebug(error.c_str());
		PVRShellOutputDebug("Unknown uniform semantic count: %u\n", uiUnknownUniforms);
	}

	// Enable culling
	glEnable(GL_CULL_FACE);
	return true;
}

/*!***************************************************************************
@Function		PVRTPFXOnLoadTexture
@Input			TextureName
@Output			uiHandle
@Output			uiFlags
@Return			EPVRTError	PVR_SUCCESS on success.
@Description	Callback for texture load.
*****************************************************************************/
EPVRTError OGLES2IntroducingPFX::PVRTPFXOnLoadTexture(const CPVRTStringHash& TextureName, GLuint& uiHandle, unsigned int& uiFlags)
{
	/*
		This is an optional callback function for PVRTPFXEffect and can be used to automate 
		the texture loading process.
		If multiple effects are to be loaded and they share textures it would be
		prudent to have a caching system in place so texture memory is not wasted.
		Please see OGLES2MagicLantern for an example of this.
	*/
	if(PVRTTextureLoadFromPVR(TextureName.String().c_str(), &uiHandle) != PVR_SUCCESS)
		return PVR_FAIL;

	return PVR_SUCCESS;
}

/*!***************************************************************************
@Function		LoadVBOs
@Description	Generates and populates VBOs for the mesh elements.
*****************************************************************************/
void OGLES2IntroducingPFX::LoadVBOs()
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

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES2IntroducingPFX::ReleaseView()
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
bool OGLES2IntroducingPFX::RenderScene()
{
	// Clears the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use the loaded effect
	m_pEffect->Activate();

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

	{
		PVRTVec3	vFrom, vTo, vUp;
		VERTTYPE	fFOV;
		vUp.x = 0.0f;
		vUp.y = 1.0f;
		vUp.z = 0.0f;

		// We can get the camera position, target and field of view (fov) with GetCameraPos()
		fFOV = m_Scene.GetCameraPos(vFrom, vTo, 0) * 0.4f;

		/*
			We can build the world view matrix from the camera position, target and an up vector.
			For this we use PVRTMat4LookAtRH().
		*/
		m_mView = PVRTMat4::LookAtRH(vFrom, vTo, vUp);

		// Calculates the projection matrix
		bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);
		m_mProjection = PVRTMat4::PerspectiveFovRH(fFOV, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), CAM_NEAR, CAM_FAR, PVRTMat4::OGL, bRotate);
	}

	// Retrieve the list of required uniforms
	const CPVRTArray<SPVRTPFXUniform>& aUniforms = m_pEffect->GetUniformArray();

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
			case ePVRTPFX_UsWORLDVIEWIT:
				{
					PVRTMat4 mWorldViewI, mWorldViewIT;

					// Passes the inverse transpose of the world-view matrix to the shader to transform the normals
					mWorldViewI  = mWorldView.inverse();
					mWorldViewIT = mWorldViewI.transpose();

					PVRTMat3 WorldViewIT = PVRTMat3(mWorldViewIT);

					glUniformMatrix3fv(aUniforms[j].nLocation, 1, GL_FALSE, WorldViewIT.f);
				}
				break;
			case ePVRTPFX_UsLIGHTDIREYE:
				{
					// Reads the light direction from the scene.
					PVRTVec4 vLightDirection;
					PVRTVec3 vPos;
					vLightDirection = m_Scene.GetLightDirection(0);

					vLightDirection.x = -vLightDirection.x;
					vLightDirection.y = -vLightDirection.y;
					vLightDirection.z = -vLightDirection.z;

					/*
						Sets the w component to 0, so when passing it to glLight(), it is
						considered as a directional light (as opposed to a spot light).
					*/
					vLightDirection.w = 0;

					// Passes the light direction in eye space to the shader
					PVRTVec4 vLightDirectionEyeSpace;
					vLightDirectionEyeSpace = m_mView * vLightDirection;

					glUniform3f(aUniforms[j].nLocation, vLightDirectionEyeSpace.x, vLightDirectionEyeSpace.y, vLightDirectionEyeSpace.z);
				}
				break;
			case ePVRTPFX_UsTEXTURE:
				{
					// Set the sampler variable to the texture unit
					glUniform1i(aUniforms[j].nLocation, aUniforms[j].nIdx);
				}
				break;
			case eUsMYCUSTOMSCALE:
				{
					PVRTMat4 mxScale = PVRTMat4::Identity();
					glUniformMatrix4fv(aUniforms[j].nLocation, 1, GL_FALSE, mxScale.ptr());
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
			case ePVRTPFX_UsPOSITION:
			case ePVRTPFX_UsNORMAL:
			case ePVRTPFX_UsUV:
				{
					glDisableVertexAttribArray(aUniforms[j].nLocation);
				}
				break;
			}
		}
	}

	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("IntroducingPFX", "", ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			mesh		The mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the meterial prepared.
******************************************************************************/
void OGLES2IntroducingPFX::DrawMesh(SPODMesh* pMesh)
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
	return new OGLES2IntroducingPFX();
}

/******************************************************************************
 End of file (OGLES2IntroducingPFX.cpp)
******************************************************************************/

