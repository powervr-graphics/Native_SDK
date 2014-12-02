/******************************************************************************

 @File         OGLES3MagicLantern.cpp

 @Title        Dynamic Light-maps

 @Version

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  This demo shows how to project a light-map as if it were projected
               from an illuminated sphere. The demo also shows how to handle a
               PFX file with PVRShaman semantics.  Please refer to
               MagicLanternShaders.pfx for a detailed explanation  of how the shaders work.

******************************************************************************/
#include <string.h>

#include "PVRShell.h"
#include "OGLES3Tools.h"

/******************************************************************************
 Content filenames
******************************************************************************/
const char c_szPFXSrcFile[]	= "MagicLanternShaders.pfx";
const char c_szSceneFile[]	= "MagicLantern.pod";
const CPVRTStringHash c_SphereEffectName = CPVRTStringHash("Sphere");

/******************************************************************************
 Structures
******************************************************************************/
struct STextureData
{
	GLuint			uiHandle;		// The GL handle
	unsigned int	uiFlags;		// Data such as cube map, etc.
};

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES3MagicLantern : public PVRShell, public PVRTPFXEffectDelegate
{
	// Print3D class used to display text.
	CPVRTPrint3D						m_Print3D;

	// 3D Model class that will hold all the model data, materials, lights and cameras.
	CPVRTModelPOD						m_Scene;

	// Transformation matrices which are global for all objects.
	PVRTMat4							m_mView, m_mProjection;

	// OpenGL handles for vertex and index buffer objects.
	GLuint*								m_puiVbo;
	GLuint*								m_puiIndexVbo;
	
	// Array to hold all the materials from the POD file.
	// This will allow us later to link a mesh with an effect.
	GLuint*								m_puiMaterialEffectID;

	// Structures to hold the effects data.
	CPVRTPFXParser*						m_ppEffectParser;
	CPVRTPFXEffect*						m_pFX[3];

	// A map of cached textures
	CPVRTMap<CPVRTStringHash, STextureData>	m_TextureCache;

	// Variables to handle the animation in a time-based manner.
	unsigned long	m_ulTimePrev;
	float			m_fFrame;
	PVRTVec3		m_LightPosition;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	virtual EPVRTError PVRTPFXOnLoadTexture(const CPVRTStringHash& TextureName, GLuint& uiHandle, unsigned int& uiFlags);

	bool LoadPFX	(void);
	bool LoadVbos	(void);
	void DrawMesh	(int i32NodeIndex, CPVRTPFXEffect *pCurrentFX);
};

/*!****************************************************************************
 @Function		LoadPFX
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occurred
 @Description	Loads and compiles the shaders and links the shader programs
				required for this training course
******************************************************************************/
bool OGLES3MagicLantern::LoadPFX(void)
{
	CPVRTString error = "";
	
	// Parse the whole PFX and store all data.
	m_ppEffectParser = new CPVRTPFXParser();
	if(m_ppEffectParser->ParseFromFile(c_szPFXSrcFile, &error) != PVR_SUCCESS)
	{
		error = "Parse failed:\n\n" + error;
		PVRShellSet(prefExitMessage, error.c_str());
		return false;
	}

	// Setup all effects in the PFX file so we initialise the shaders and
	// store uniforms and attributes locations.
	unsigned int uNumEffects = m_ppEffectParser->GetNumberEffects();
	for (unsigned int i=0; i<uNumEffects; i++)
	{
		// Load one by one the effects. This will also compile the shaders.
		m_pFX[i] = new CPVRTPFXEffect();
		unsigned int nUnknownUniformCount = 0;
		if(m_pFX[i]->Load(*m_ppEffectParser, m_ppEffectParser->GetEffect(i).Name.c_str(), NULL, this, nUnknownUniformCount, &error)  != PVR_SUCCESS)
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

	// Allocate an array of integers to hold an effect ID per material.
	m_puiMaterialEffectID = new GLuint[m_Scene.nNumMaterial];
	
	// Assign an effect to each material based on its name.
	// If there is a material with an effect name which is not in the PFX 
	// file, the application will quit and report an error.
	for(int i = 0; i < (int) m_Scene.nNumMaterial; ++i)
	{
		SPODMaterial* pMaterial = &m_Scene.pMaterial[i];

		m_puiMaterialEffectID[i] = 0xbad;

		for (unsigned int j=0; j<uNumEffects; j++)
		{
			// Compare the effect string in the material to each effect in our file so we can
			// match effects to meshes.
			// All effects are contained in the same PFX file so we do not have to worry about
			// effects spread across several files.
			// Note: Each material can only contain a single effect but the same effect
			// might be applied to several materials.
			if (m_ppEffectParser->GetEffect(j).Name == pMaterial->pszEffectName)
			{
				m_puiMaterialEffectID[i] = j;
			}
		}

		if(m_puiMaterialEffectID[i] == 0xbad)
		{
			PVRShellOutputDebug("ERROR: %s effect not found in PFX\n", pMaterial->pszEffectName);
			return false;
		}
	}

	return true;
}

/*!***************************************************************************
@Function		PVRTPFXOnLoadTexture
@Input			TextureName
@Output			uiHandle
@Output			uiFlags
@Return			EPVRTError	PVR_SUCCESS on success.
@Description	Callback function on texture load.
*****************************************************************************/
EPVRTError OGLES3MagicLantern::PVRTPFXOnLoadTexture(const CPVRTStringHash& TextureName, GLuint& uiHandle, unsigned int& uiFlags)
{
	/*
		Because we have multiple effects being loaded yet the textures remain the same we can
		efficiently cache texture IDs and only load a texture once but assign it to
		multiple effects.
	*/

	// Check if the texture already exists in the map
	if(m_TextureCache.Exists(TextureName))
	{
		const STextureData& TexData = m_TextureCache[TextureName];
		uiHandle = TexData.uiHandle;
		uiFlags  = TexData.uiFlags;
		return PVR_SUCCESS;
	}

	// Texture is not loaded. Load and add to the map.
	PVRTextureHeaderV3 sHeader;
	if(PVRTTextureLoadFromPVR(TextureName.c_str(), &uiHandle, &sHeader) != PVR_SUCCESS)
		return PVR_FAIL;

	uiFlags = 0;
	if(sHeader.u32NumFaces == 6)
		uiFlags |= PVRTEX_CUBEMAP;

	STextureData& TexData = m_TextureCache[TextureName];
	TexData.uiFlags  = uiFlags;
	TexData.uiHandle = uiHandle;

	return PVR_SUCCESS;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
bool OGLES3MagicLantern::LoadVbos(void)
{
	// A bit of warning when the data is not in the expected format.
	// This demo uses interleaved triangle list vertex data.
	// Anything else will not work. This allows to keep then code simple.
	if(!m_Scene.pMesh[0].pInterleaved)
	{
		PVRShellSet(prefExitMessage, "ERROR: This demo requires the pod data to be interleaved. Please re-export with the interleaved option enabled.");
		return false;
	}

	// Allocate the pointers to hold the vertex buffer objects and
	// the index buffer objects.
	if (!m_puiVbo)      m_puiVbo = new GLuint[m_Scene.nNumMesh];
	if (!m_puiIndexVbo) m_puiIndexVbo = new GLuint[m_Scene.nNumMesh];

	// Load vertex data of all meshes in the scene into VBOs.
	// The meshes have been exported with the "Interleave Vectors" option,
	// so all data is interleaved in the buffer at pMesh->pInterleaved.
	glGenBuffers(m_Scene.nNumMesh, m_puiVbo);
	for (unsigned int i = 0; i < m_Scene.nNumMesh; ++i)
	{
		// Load vertex data from POD into buffer object.
		SPODMesh& Mesh = m_Scene.pMesh[i];
		unsigned int uiSize = Mesh.nNumVertex * Mesh.sVertex.nStride;
		glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[i]);
		glBufferData(GL_ARRAY_BUFFER, uiSize, Mesh.pInterleaved, GL_STATIC_DRAW);

		// Load index data into buffer object if available.
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
 @Function		InitApplication
 @Return		bool		true if no error occurred
 @Description	Code in InitApplication() will be called by PVRShell once per
				run, before the rendering context is created.
				Used to initialize variables that are not dependant on it
				(e.g. external modules, loading meshes, etc.)
				If the rendering context is lost, InitApplication() will
				not be called again.
******************************************************************************/
bool OGLES3MagicLantern::InitApplication()
{
	m_puiVbo = 0;
	m_puiIndexVbo = 0;
	m_puiMaterialEffectID = 0;

	// Get and set the read path for content files.
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	// Load the scene
	if(m_Scene.ReadFromFile(c_szSceneFile) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Couldn't load the .pod file\n");
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
 @Return		bool		true if no error occurred
 @Description	Code in QuitApplication() will be called by PVRShell once per
				run, just before exiting the program.
				If the rendering context is lost, QuitApplication() will
				not be called.
******************************************************************************/
bool OGLES3MagicLantern::QuitApplication()
{
	// Free the memory allocated for the scene.
	m_Scene.Destroy();

	if(m_puiMaterialEffectID)	delete[] m_puiMaterialEffectID;
	if(m_puiVbo)				delete[] m_puiVbo;
	if(m_puiIndexVbo)			delete[] m_puiIndexVbo;

    return true;
}

/*!****************************************************************************
 @Function		InitView
 @Return		bool		true if no error occurred
 @Description	Code in InitView() will be called by PVRShell upon
				initialization or after a change in the rendering context.
				Used to initialize variables that are dependant on the rendering
				context (e.g. textures, vertex buffers, etc.)
******************************************************************************/
bool OGLES3MagicLantern::InitView()
{
	CPVRTString ErrorStr;

	// At this point m_Scene should have been already processed by InitApplication()
	// and all the POD data properly loaded, but lets do a little test just in case.
	 if (!m_Scene.IsLoaded())
	{
		PVRShellSet(prefExitMessage, "ERROR: POD file has not been loaded correctly. Cannot continue. \n");
		return false;
	}

	// Initialize VBO data
	if(!LoadVbos())
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	//	Load and compile the shaders, link programs and load textures.
	if(!LoadPFX())
	{
		return false;
	}

	// Initialize Print3D
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	// Enable backface culling and depth test
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	// Black as clear colour
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Disable blending
	glDisable(GL_BLEND);
	
	// Set up the view and projection matrices from the camera.
	// The camera does not moves so these matrices only need to be 
	// calculated once.
	// If you want to make the camera dynamic, re-calculate the view matrix 
	// every frame.
	PVRTVec3 vFrom, vTo(0.0f), vUp(0.0f, 1.0f, 0.0f);
	float fFOV;

	// Camera nodes are after the mesh and light nodes in the array.
	// We grab camera num 0 (the only one in the scene)
	const int g_ui32Camera = 0;
	int i32CamID = m_Scene.pNode[m_Scene.nNumMeshNode + m_Scene.nNumLight + g_ui32Camera].nIdx;

	// Get the camera position and target 
	if(m_Scene.pCamera[i32CamID].nIdxTarget != -1) // Does the camera have a target?
		m_Scene.GetCameraPos( vFrom, vTo, g_ui32Camera); // vTo is taken from the target node.
	else
		m_Scene.GetCamera( vFrom, vTo, vUp, g_ui32Camera); // vTo is calculated from the rotation.

	// Calculate the FOV depending of the screen dimensions so everything fit in view
	// regardless whether the screen is rotated or not.
	// if the aspect ratio is different than 640x480 adapt FOV so the scene still looks correct.
	float fRatioWoverH = (480.0f/640.0f) * ((!bRotate) ? (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight) :  (float)PVRShellGet(prefHeight)/(float)PVRShellGet(prefWidth));
	
	fFOV = m_Scene.pCamera[i32CamID].fFOV / fRatioWoverH;

	// We can build the model view matrix from the camera position, target and an up vector.
	m_mView = PVRTMat4::LookAtRH(vFrom, vTo, vUp);

	// Calculate the projection matrix.
	m_mProjection = PVRTMat4::PerspectiveFovRH(fFOV, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), 	m_Scene.pCamera[i32CamID].fNear, m_Scene.pCamera[i32CamID].fFar, PVRTMat4::OGL, bRotate);


	// Initialize variables used for the animation.
	m_fFrame = 0;
	m_ulTimePrev = PVRShellGetTime();

	m_LightPosition.x = 0.0f;  m_LightPosition.z = -130.0f;

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occurred
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES3MagicLantern::ReleaseView()
{
	// Delete all effects.
	for (unsigned int i=0; i<m_ppEffectParser->GetNumberEffects(); i++)
	{
		delete m_pFX[i];
	}

	// Delete the PFX container.
	delete m_ppEffectParser;

	// Delete buffer objects.
	glDeleteBuffers(m_Scene.nNumMesh, m_puiVbo);
	glDeleteBuffers(m_Scene.nNumMesh, m_puiIndexVbo);

	// Release loaded textures
	for(unsigned int uiIndex = 0; uiIndex < m_TextureCache.GetSize(); ++uiIndex)
	{
		GLuint uiHandle = m_TextureCache.GetDataAtIndex(uiIndex)->uiHandle;
		glDeleteTextures(1, &uiHandle);
	}

	m_TextureCache.Clear();

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

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
bool OGLES3MagicLantern::RenderScene()
{
	PVRTMat4 mLightWorld;

	// Clear the color and depth buffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Time based animation and locks the app to 60 FPS.
	// Uses the shell function PVRShellGetTime() to get the time in milliseconds.
	unsigned long ulTime = PVRShellGetTime();
	unsigned long ulDeltaTime = ulTime - m_ulTimePrev;
	m_ulTimePrev = ulTime;
	m_fFrame += (float)ulDeltaTime * (60.0f/1000.0f);

	// Create an animation for the the position and rotation of the light-ball.
	m_LightPosition.x = cos(m_fFrame/140.0f)*60.0f;
	m_LightPosition.z = sin(m_fFrame/70.0f)*60.0f - 40.0f;
	m_LightPosition.y = sin(m_fFrame/100.0f)*20.0f;

	mLightWorld  = PVRTMat4::Translation(m_LightPosition.x, m_LightPosition.y-10, m_LightPosition.z);
	mLightWorld *= PVRTMat4::RotationX(m_fFrame/540.0f);
	mLightWorld *= PVRTMat4::RotationZ(m_fFrame/370.0f);
	
	// To draw a scene, you must go through all the MeshNodes and draw the referenced meshes.
	for (unsigned int i = 0; i < m_Scene.nNumMeshNode; ++i)
	{
		SPODNode& Node = m_Scene.pNode[i]; 

		// Get the current effect from the material ID. The list of IDs was built in LoadPFX()
		GLuint uiFXID = m_puiMaterialEffectID[Node.nIdxMaterial];

		// Use the loaded effect (Vertex and Fragment shader)
		// And also bind all textures in the effect.
		m_pFX[uiFXID]->Activate();
		
		// Set the blend mode
		// Based in the info stored in the material by PVRShaman.
		// We check whether the blend mode is 'opaque' (ONE,ZERO).
		// Otherwise we enable blending and set the corresponding operations.
		if (m_Scene.pMaterial[Node.nIdxMaterial].eBlendSrcRGB == ePODBlendFunc_ONE && m_Scene.pMaterial[Node.nIdxMaterial].eBlendDstRGB == ePODBlendFunc_ZERO)
		{
			glDisable(GL_BLEND);
		}
		else
		{
			glEnable(GL_BLEND);
			glBlendFunc(m_Scene.pMaterial[Node.nIdxMaterial].eBlendSrcRGB, m_Scene.pMaterial[Node.nIdxMaterial].eBlendDstRGB);
		}

		// Now process PVRShaman semantics and set-up the associated uniforms.
		const CPVRTArray<SPVRTPFXUniform>& Uniforms = m_pFX[uiFXID]->GetUniformArray();
		for(unsigned int j = 0; j < Uniforms.GetSize() ; ++j)
		{
			switch(Uniforms[j].nSemantic)
			{
			case ePVRTPFX_UsWORLDVIEWPROJECTION:
				{
					PVRTMat4 mWVP;

					// The whole scene (except the ball) is static.
					// The POD file was saved with 'word space' coordinates so there is no need to apply the
					// transformation matrices for the meshes, just the identity.
					// If you are exporting animation you will need to set the corresponding mesh
					// transformation matrix here (see IntroducingPOD training course).
					PVRTMat4 mWorld = PVRTMat4::Identity();

					// Check whether the current effect is the sphere and set up the word matrix with the sphere animation.
					if (m_ppEffectParser->GetEffect(uiFXID).Name == c_SphereEffectName)
					{
						mWorld = mLightWorld;
					}

					// Pass the model-view-projection matrix (MVP) to the shader to transform the vertices.
					PVRTMat4 mModelView;
					mModelView = m_mView * mWorld;
					mWVP = m_mProjection * mModelView;

					glUniformMatrix4fv(Uniforms[j].nLocation, 1, GL_FALSE, mWVP.f);
				} 
				break;
			case ePVRTPFX_UsWORLDIT:
				{
					// Passes the inverse transpose of the light rotation matrix.
					// This is needed to transform the light direction (from the light to each vertex) 
					// and it will give us a three component texture vector to map the cubemap texture.
					PVRTMat3 mLightModelIT = PVRTMat3(mLightWorld).inverse().transpose();
					glUniformMatrix3fv(Uniforms[j].nLocation, 1, GL_FALSE, mLightModelIT.f);
				}
				break;
			case ePVRTPFX_UsLIGHTPOSWORLD:
				{
					glUniform3f(Uniforms[j].nLocation, m_LightPosition.x, m_LightPosition.y, m_LightPosition.z);
				}
				break;
			case ePVRTPFX_UsLIGHTCOLOR:
				{
					// Some color variation here to make it more interesting.
					float afLightColor[] = { 1.0f, sin(m_fFrame/300.0f)*0.3f+0.7f, cos(m_fFrame/400.0f)*0.3f+0.7f};
					glUniform3fv(Uniforms[j].nLocation, 1, afLightColor ); 
				}
				break;
			}
		}

		// Now that all transformation matrices and the materials are ready,
		// call a function to actually draw the mesh.
		// We need to pass the current effect to process 'attributes' properly.
		DrawMesh(i, m_pFX[uiFXID]);
	}

	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools.
	m_Print3D.DisplayDefaultTitle("Magic Lantern", "", ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}
/*!****************************************************************************
 @Function		DrawMesh
 @Input			i32NodeIndex		Node index of the mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the material prepared.
******************************************************************************/
void OGLES3MagicLantern::DrawMesh(int i32NodeIndex, CPVRTPFXEffect *pCurrentFX)
{
	// Get the mesh data from the POD file
	int i32MeshIndex = m_Scene.pNode[i32NodeIndex].nIdx;
	SPODMesh* pMesh = &m_Scene.pMesh[i32MeshIndex];

	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[i32MeshIndex]);

	// bind the index buffer, won't hurt if the handle is 0 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[i32MeshIndex]);

	const CPVRTArray<SPVRTPFXUniform>& Uniforms = pCurrentFX->GetUniformArray();

	// Set attributes and texture stages
	// The vertex data is already loaded into an VBO as 'interleaved data' (e.g. x,y,z,nx,ny,nz,u0,v0,...).
	// When using 'interleaved data' the last parameter in glVertexAttribPointer is just a numerical offset (in bytes) into the stream (e.g. 0, 12, 24).
	// Our tools use pMesh->sVertex.pData, etc. to store this offset only when using interleaved data.
	// If you do not want to use interleaved data you will need to create separated VBO from pMesh->sVertex.pData, pMesh->sNormals.pData, etc.
	// and set the offset (last parameter) to 0
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
			
		case ePVRTPFX_UsTEXTURE:
			{
				// Set the sampler variable to the texture unit/stage 0,1,2,etc.
				glUniform1i(Uniforms[j].nLocation, Uniforms[j].nIdx);
			}
			break;
		}
	}

	// Indexed Triangle list 
	// Note: if you export you model and want to use this code, please export it as 'interleaved' triangle list.
	glDrawElements(GL_TRIANGLES, pMesh->nNumFaces*3, GL_UNSIGNED_SHORT, 0);

	// Safely disable the vertex attribute arrays
	for(unsigned int j = 0; j < Uniforms.GetSize(); ++j)
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

	// Un-bind our vertex and index buffers.
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
	return new OGLES3MagicLantern();
}

/******************************************************************************
 End of file (OGLES3MagicLantern.cpp)
******************************************************************************/

