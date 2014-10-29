/******************************************************************************

 @File         OGLES3UniformBufferObjects.cpp

 @Title        Uniform Buffer Objects

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows how to use uniform buffer objects

******************************************************************************/

#include "PVRShell.h"
#include "OGLES3Tools.h"

/******************************************************************************
 Constants
******************************************************************************/

// Camera constants used to generate the projection matrix
const float g_fCameraNear	= 1.0f;
const float g_fCameraFar	= 500.0f;

#define VERTEX_ARRAY    0
#define NORMAL_ARRAY    1
#define TEXCOORD_ARRAY  2

#define UNIFORM_BUFFER_TRANSFORM_BLOCK_ID 0

const char *c_szUniformBufferBlockName = "transforms";

/*
 *  Group all transforms in the order as they appear in the shader.
 *  This will allow to upload blocks of uniform data at once, that can be shared across shaders.
 */
struct sUniformBlock
{
	PVRTMat4 ViewProjectionMatrix; 
	PVRTVec4 Light0PosWorld;       
	PVRTVec4 Light0Colour;         
	PVRTVec4 Light1PosWorld;       
	PVRTVec4 Light1Colour;         
}; 

const unsigned int c_uiNumUniformBlockUniforms = 5;

const unsigned int c_uiHostOffsets[c_uiNumUniformBlockUniforms] = 
{
	0, 
	sizeof(PVRTMat4),
	sizeof(PVRTMat4) + sizeof(PVRTVec4),
	sizeof(PVRTMat4) + sizeof(PVRTVec4) * 2,
	sizeof(PVRTMat4) + sizeof(PVRTVec4) * 3,
};

const char *c_pszUniformNames[c_uiNumUniformBlockUniforms] = 
{
	"ViewProjectionMatrix", // mat4 
	"Light0PosWorld",       // vec4
	"Light0Colour",         // vec4
	"Light1PosWorld",       // vec4
	"Light1Colour",         // vec4
};

/******************************************************************************
 Content file names
******************************************************************************/

const char c_szPFXSrcFile[]	= "effect.pfx";
const char c_szSceneFile[] = "scene.pod";

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES3UniformBufferObjects : public PVRShell, public PVRTPFXEffectDelegate
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;
	SPVRTContext	m_sContext;
	CPVRTModelPOD   m_Scene;

	PVRTMat4 m_mProjection;
	PVRTMat4 m_mView;
	bool m_bRotate;

	GLuint* m_puiVBO;
	GLuint* m_puiIBO;
		
	// The uniform block host representation
	sUniformBlock m_sTransforms;

	// The uniform block data storage handle
	GLuint m_uiTransformsUBO;

	// The uniform block index that references a uniform block in a specific shader
	GLuint m_uiTransformsBlockIndex;

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

	EPVRTError PVRTPFXOnLoadTexture(const CPVRTStringHash& TextureName, GLuint& uiHandle, unsigned int& uiFlags);

	void LoadVbos(CPVRTModelPOD *pModel);
	bool LoadPFX(CPVRTString *pErrorMsg);
	void Update();
	
	bool RenderSceneWithEffects();
};

/*!***************************************************************************
@Function		PVRTPFXOnLoadTexture
@Input			TextureName
@Output			uiHandle
@Output			uiFlags
@Return			EPVRTError	PVR_SUCCESS on success.
@Description	Callback function on texture load.
*****************************************************************************/
EPVRTError OGLES3UniformBufferObjects::PVRTPFXOnLoadTexture(const CPVRTStringHash& TextureName, GLuint& uiHandle, unsigned int& uiFlags)
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
void OGLES3UniformBufferObjects::LoadVbos(CPVRTModelPOD *pModel)
{
	m_puiVBO = new GLuint[pModel->nNumMesh];
	m_puiIBO = new GLuint[pModel->nNumMesh];	
	
	/*
		Load vertex data of all meshes in the scene into VBOs

		The meshes have been exported with the "Interleave Vectors" option,
		so all data is interleaved in the buffer at pMesh->pInterleaved.
		Interleaving data improves the memory access pattern and cache efficiency,
		thus it can be read faster by the hardware.
	*/

	glGenBuffers(pModel->nNumMesh, m_puiVBO);

	for (unsigned int i = 0; i < pModel->nNumMesh; ++i)
	{
		// Load vertex data into buffer object
		SPODMesh& Mesh = pModel->pMesh[i];
		unsigned int uiSize = Mesh.nNumVertex * Mesh.sVertex.nStride;

		glBindBuffer(GL_ARRAY_BUFFER, m_puiVBO[i]);
		glBufferData(GL_ARRAY_BUFFER, uiSize, Mesh.pInterleaved, GL_STATIC_DRAW);

		// Load index data into buffer object if available
		m_puiIBO[i] = 0;

		if (Mesh.sFaces.pData)
		{
			glGenBuffers(1, &m_puiIBO[i]);
			
			uiSize = PVRTModelPODCountIndices(Mesh) * sizeof(GLshort);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIBO[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, Mesh.sFaces.pData, GL_STATIC_DRAW);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/*!****************************************************************************
 @Function		LoadPFX
  @Return		bool			true if no error occurred
 @Description	Loads and compiles the shaders and links the shader programs
				required for this training course
******************************************************************************/
bool OGLES3UniformBufferObjects::LoadPFX(CPVRTString *pErrorMsg)
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

		// Verify that the uniform buffer blocks defined in the shaders match the size of the host structure
		GLuint uiProgram = m_ppPFXEffects[i]->GetProgramHandle();
		GLuint uiTransformsBlockIndex = glGetUniformBlockIndex(uiProgram, c_szUniformBufferBlockName);	
		if (uiTransformsBlockIndex == GL_INVALID_INDEX)
		{
			*pErrorMsg = CPVRTString("Uniform buffer block not found in effect ") + m_pPFXEffectParser->GetEffect(i).Name.String();
			return false;
		}

		/*
		 * Check if the uniform buffer offsets matches those of our host struct.
		 * This step is optional when using std140 layout as it is predefined, but good for catching non-obvious errors.
		 */
		{
			// First step, get the indices for the individual UBO entries
			GLuint indices[c_uiNumUniformBlockUniforms];
			glGetUniformIndices(uiProgram, c_uiNumUniformBlockUniforms, c_pszUniformNames, indices);

			// Query the offsets
			GLint offsets[c_uiNumUniformBlockUniforms];
			glGetActiveUniformsiv(uiProgram, c_uiNumUniformBlockUniforms, indices, GL_UNIFORM_OFFSET, offsets);

			for (unsigned int j=0; j < c_uiNumUniformBlockUniforms; j++)
			{
				if (c_uiHostOffsets[j] != offsets[j])
				{
					*pErrorMsg = CPVRTString("Host struct does not match shader layout in effect ") + m_pPFXEffectParser->GetEffect(i).Name.String();
					return false;
				}
			}			
		}		
		
		/*
		 * Finally bind the uniform block of that shader program to the custom defined slot ID 
		 * We previously bound a uniform block to that ID in InitView().
		 */
		glUniformBlockBinding(uiProgram, uiTransformsBlockIndex, UNIFORM_BUFFER_TRANSFORM_BLOCK_ID);
	}
	
	return true;
}

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
bool OGLES3UniformBufferObjects::InitApplication()
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
		
	m_puiVBO = 0;
	m_puiIBO = 0;

	m_pPFXEffectParser = 0;
	m_ppPFXEffects = 0;
	
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
bool OGLES3UniformBufferObjects::QuitApplication()
{
	// Free the memory allocated for the scene
	m_Scene.Destroy();

	delete [] m_puiVBO;
	delete [] m_puiIBO;
	
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
bool OGLES3UniformBufferObjects::InitView()
{				
	CPVRTString ErrorStr;

	/*
		Initialize VBO data
	*/
	LoadVbos(&m_Scene);

	/*
		Load and compile the shaders & link programs
	*/
	if (!LoadPFX(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	/*
		Initialize Print3D
	*/

	// Is the screen rotated?
	m_bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	if(m_Print3D.SetTextures(0, PVRShellGet(prefWidth), PVRShellGet(prefHeight), m_bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialize Print3D\n");
		return false;
	}

	/*
		Set OpenGL ES render states needed for this training course
	*/
	// Enable backface culling and depth test
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	// Use a nice bright blue as clear colour
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);		
		
	/*
	 * Create storage for the uniform data.
	 */
	glGenBuffers(1, &m_uiTransformsUBO);	
	glBindBuffer(GL_UNIFORM_BUFFER, m_uiTransformsUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(m_sTransforms), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Bind the uniform buffer object to a specific self-defined binding point (here: UNIFORM_BUFFER_TRANSFORM_BLOCK_ID)	
	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BUFFER_TRANSFORM_BLOCK_ID, m_uiTransformsUBO);

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occurred
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES3UniformBufferObjects::ReleaseView()
{	
	// Delete buffer objects
	glDeleteBuffers(m_Scene.nNumMesh, m_puiVBO);
	glDeleteBuffers(m_Scene.nNumMesh, m_puiIBO);
	glDeleteBuffers(1, &m_uiTransformsUBO);
	
	// Release textures
	{
		const CPVRTArray<SPVRTPFXTexture>&	sTex = m_ppPFXEffects[0]->GetTextureArray();
		for(unsigned int i = 0; i < sTex.GetSize(); ++i)
			glDeleteTextures(1, &sTex[i].ui);
	}

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
 @Function		Update
 @Description	Handles user input and updates all timing data.
******************************************************************************/
void OGLES3UniformBufferObjects::Update()
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
	// Calculate the projection and view matrix using utility functions
	m_mProjection = PVRTMat4::PerspectiveFovRH(fFOV, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), m_Scene.pCamera[0].fNear, m_Scene.pCamera[0].fFar, PVRTMat4::OGL, m_bRotate);
	m_mView = PVRTMat4::LookAtRH(vFrom, vTo, vUp);		

	m_sTransforms.ViewProjectionMatrix = m_mProjection * m_mView;

	PVRTVec3 vPos, vLightDir;
	m_Scene.GetLight(vPos, vLightDir, 0);
	m_sTransforms.Light0PosWorld = PVRTVec4(vPos, 1.0f);
	m_sTransforms.Light0Colour = PVRTVec4(PVRTVec3(m_Scene.pLight[0].pfColour), 1.0f);

	m_Scene.GetLight(vPos, vLightDir, 1);
	m_sTransforms.Light1PosWorld = PVRTVec4(vPos, 1.0f);
	m_sTransforms.Light1Colour = PVRTVec4(PVRTVec3(m_Scene.pLight[1].pfColour), 1.0f);

	/*
	 *  Update the uniform buffer with the most recent transformations.
	 *  All shaders that use the same uniform buffer binding point will use the updated values.
	 */
	glBindBuffer(GL_UNIFORM_BUFFER, m_uiTransformsUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(m_sTransforms), &m_sTransforms, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BUFFER_TRANSFORM_BLOCK_ID, m_uiTransformsUBO);
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
bool OGLES3UniformBufferObjects::RenderScene()
{	
	Update();	
	GLenum glerror = glGetError();
	if (glerror != GL_NO_ERROR)
		PVRShellOutputDebug("ERROR after update()!\n");

	// Clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		
			
	RenderSceneWithEffects();

	// Display the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("Uniform Buffer Objects", "", ePVRTPrint3DSDKLogo);	
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		RenderSceneWithEffect
 @Return		bool		true if no error occurred
 @Description	Renders the whole scene with a single effect.
******************************************************************************/
bool OGLES3UniformBufferObjects::RenderSceneWithEffects()
{		
	for (unsigned int i=0; i < m_Scene.nNumMeshNode; i++)
	{
		m_ppPFXEffects[i]->Activate();

		SPODNode* pNode = &m_Scene.pNode[i];
		SPODMesh* pMesh = &m_Scene.pMesh[pNode->nIdx];

		// Pre-calculate commonly used matrices
		PVRTMat4 mModel;
		m_Scene.GetWorldMatrix(mModel, *pNode);		
		
		glBindBuffer(GL_ARRAY_BUFFER, m_puiVBO[pNode->nIdx]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIBO[pNode->nIdx]);		
		
		// Bind semantics
		const CPVRTArray<SPVRTPFXUniform>& Uniforms = m_ppPFXEffects[i]->GetUniformArray();
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
					// Set the sampler variable to the texture unit
					glUniform1i(Uniforms[j].nLocation, Uniforms[j].nIdx);
				}		
				break;
			case ePVRTPFX_UsWORLD:
				{
					// Set the sampler variable to the texture unit
					glUniformMatrix4fv(Uniforms[j].nLocation, 1, GL_FALSE, mModel.f);
				}
				break;
			case ePVRTPFX_UsWORLDIT:
				{
					// Set the sampler variable to the texture unit
					PVRTMat3 mModelIT(mModel.inverse().transpose());
					glUniformMatrix3fv(Uniforms[j].nLocation, 1, GL_FALSE, mModelIT.f);
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
	return new OGLES3UniformBufferObjects();
}

/******************************************************************************
 End of file (OGLES3UniformBufferObjects.cpp)
******************************************************************************/

