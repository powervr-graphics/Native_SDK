/******************************************************************************

 @File         OGLES3Instancing.cpp

 @Title        Instancing

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows how to use instancing to draw several instances of the same
               mesh with a single draw call.

******************************************************************************/

#include "PVRShell.h"
#include "OGLES3Tools.h"

/******************************************************************************
 Constants
******************************************************************************/

// Camera constants used to generate the projection matrix
const float g_fCameraNear = 1.0f;
const float g_fCameraFar  = 500.0f;

const unsigned int c_uiDefaultNumInstances = 64;

#define VERTEX_ARRAY  0
#define NORMAL_ARRAY  1

const PVRTVec3 g_asModelColours[6] = 
{ 
	PVRTVec3(1.0f, 0.0f, 0.0f), PVRTVec3(0.0f, 1.0f, 0.0f), PVRTVec3(0.0f, 0.0f, 1.0f), 
    PVRTVec3(1.0f, 1.0f, 0.0f), PVRTVec3(1.0f, 0.0f, 1.0f), PVRTVec3(0.0f, 1.0f, 1.0f) 
};
const unsigned int g_uiNumModelColours = sizeof(g_asModelColours) / sizeof(g_asModelColours[0]);

/******************************************************************************
 Content file names
******************************************************************************/

const char c_szPFXSrcFile[]	= "effect.pfx";
const char c_szSceneFile[] = "scene.pod";

/******************************************************************************
 Global strings
******************************************************************************/

const CPVRTStringHash c_RenderSceneInstancedEffectName = CPVRTStringHash("RenderSceneInstanced");

/******************************************************************************
 Structures and enums
******************************************************************************/

enum eCustomSemantics
{
	eCUSTOMSEMANTIC_INSTANCESPERROW = ePVRTPFX_NumSemantics + 1
};

const SPVRTPFXUniformSemantic c_CustomSemantics[] = 
{ 
	{ "CUSTOMSEMANTIC_INSTANCESPERROW",		eCUSTOMSEMANTIC_INSTANCESPERROW },
};
const unsigned int c_uiNumCustomSemantics = sizeof(c_CustomSemantics)/sizeof(c_CustomSemantics[0]);

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES3Instancing : public PVRShell
{
	CPVRTPrint3D	m_Print3D;
	SPVRTContext	m_sContext;
	
	CPVRTModelPOD   m_Scene;
	GLuint* m_puiVBO;
	GLuint* m_puiIBO;
	GLuint* m_puiNumIndices;

	PVRTVec4 m_vLightDirection;
	PVRTMat4 m_mProjection;
	PVRTMat4 m_mModelViewProjection;
	PVRTMat3 m_mModelIT;

	unsigned int m_uiNumInstances;
	int          m_iInstancePerRow;

	// The effect file handlers
	CPVRTPFXParser	*m_pPFXEffectParser;
	CPVRTPFXEffect **m_ppPFXEffects;

	// Start time
	unsigned long m_ulStartTime;
		
public:

	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	void LoadVbos(CPVRTModelPOD *pModel);

	bool RenderSceneWithEffect();
	bool LoadPFX(CPVRTString *pErrorMsg);
};


/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLES3Instancing::LoadVbos(CPVRTModelPOD *pModel)
{
	m_puiVBO = new GLuint[pModel->nNumMesh];
	m_puiIBO = new GLuint[pModel->nNumMesh];	
	m_puiNumIndices = new GLuint[pModel->nNumMesh];

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
			m_puiNumIndices[i] = PVRTModelPODCountIndices(Mesh);
			uiSize = m_puiNumIndices[i] * sizeof(GLshort);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIBO[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, Mesh.sFaces.pData, GL_STATIC_DRAW);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
bool OGLES3Instancing::InitApplication()
{
	m_puiVBO = 0;
	m_puiIBO = 0;
	m_puiNumIndices = 0;

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
	
	// Initialise variables
	m_uiNumInstances = c_uiDefaultNumInstances;
	m_iInstancePerRow = (int)sqrtf((float)m_uiNumInstances);	

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
bool OGLES3Instancing::QuitApplication()
{
	// Free the memory allocated for the scene
	m_Scene.Destroy();

	delete [] m_puiVBO;
	delete [] m_puiIBO;
	delete [] m_puiNumIndices;	
	
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
bool OGLES3Instancing::InitView()
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
		PVRShellSet(prefExitMessage, "Failed to load PFX file.\n");
		return false;
	}

	/*
		Initialize Print3D
	*/

	// Is the screen rotated?
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	if(m_Print3D.SetTextures(0, PVRShellGet(prefWidth), PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
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

	m_mProjection = PVRTMat4::PerspectiveFovRH(PVRT_PI_OVER_TWO, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), g_fCameraNear, g_fCameraFar, PVRTMat4::OGL, bRotate);

	// Store initial time
	m_ulStartTime = PVRShellGetTime();
	
	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES3Instancing::ReleaseView()
{
	// Release the effect[s] then the parser
	for (unsigned int i=0; i < m_pPFXEffectParser->GetNumberEffects(); i++)
		delete m_ppPFXEffects[i];
	delete [] m_ppPFXEffects;
	delete m_pPFXEffectParser;

	// Delete buffer objects
	glDeleteBuffers(m_Scene.nNumMesh, m_puiVBO);
	glDeleteBuffers(m_Scene.nNumMesh, m_puiIBO);
	
	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

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
bool OGLES3Instancing::RenderScene()
{
	if (PVRShellIsKeyPressed(PVRShellKeyNameUP))
	{
		m_uiNumInstances++;
		m_iInstancePerRow = (int)sqrtf((float)m_uiNumInstances);
	}
	if (PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
	{
		if (m_uiNumInstances > 0)
		{
		  m_uiNumInstances--;
		  m_iInstancePerRow = (int)sqrtf((float)m_uiNumInstances);
		}
	}

	// Update all timing related data (camera movement, etc.)
	float time = (PVRShellGetTime() - m_ulStartTime) * 0.001f;
	float radius = m_iInstancePerRow * 32.0f;
	PVRTVec3 vFrom((float) sin(time) * radius, 50.0f, (float) cos(time) * radius);
	PVRTVec3 vUp = -vFrom.normalized().cross(-PVRTVec3(vFrom.z, 0.0f, -vFrom.x));
	
	// Calculate the projection and view matrices	
	PVRTMat4 mView = PVRTMat4::LookAtRH(vFrom, PVRTVec3(0.0), vUp);	
	PVRTMat4 mModelOffset = PVRTMat4::Translation(radius * -0.5f, 0.0f, radius * -0.5f);
	// Set up the View * Projection Matrix
	m_mModelViewProjection = m_mProjection * mView * mModelOffset;
	m_mModelIT = PVRTMat3(mModelOffset.inverse().transpose());

	// Read the light direction from the scene
	m_vLightDirection = PVRTVec4(0.25f, 1.0f, 0.25f, 0.0f);
	m_vLightDirection.normalize();	
	
	// Clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RenderSceneWithEffect();

	// Display the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("Instancing", "", ePVRTPrint3DSDKLogo);
	m_Print3D.Print3D(1.0f, 92.0f, 1.0f, 0xFFFFFFFF, "Instances: %d", m_uiNumInstances);
	m_Print3D.Flush();

	return true;
}


/*!****************************************************************************
 @Function		RenderSceneWithEffect
 @Return		bool		true if no error occurred
 @Description	Renders the whole scene with a single effect.
******************************************************************************/
bool OGLES3Instancing::RenderSceneWithEffect()
{
	m_ppPFXEffects[0]->Activate();
	
	for (unsigned int i=0; i < m_Scene.nNumMeshNode; i++)
	{
		SPODNode* pNode = &m_Scene.pNode[i];
		SPODMesh* pMesh = &m_Scene.pMesh[pNode->nIdx];
						
		glBindBuffer(GL_ARRAY_BUFFER, m_puiVBO[i]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIBO[i]);

		// Bind semantics
		const CPVRTArray<SPVRTPFXUniform>& Uniforms = m_ppPFXEffects[0]->GetUniformArray();
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
			case ePVRTPFX_UsMATERIALCOLORDIFFUSE:
				{										
					glUniform3fv(Uniforms[j].nLocation, g_uiNumModelColours, &g_asModelColours[0].x);
				}
				break;
			case ePVRTPFX_UsWORLDIT:
				{					
					glUniformMatrix3fv(Uniforms[j].nLocation, 1, GL_FALSE, m_mModelIT.f);
				}
				break;
			case ePVRTPFX_UsWORLDVIEWPROJECTION:
				{
					glUniformMatrix4fv(Uniforms[j].nLocation, 1, GL_FALSE, m_mModelViewProjection.f);
				}
				break;
			case ePVRTPFX_UsLIGHTDIRWORLD:
				{
					glUniform3fv(Uniforms[j].nLocation, 1, m_vLightDirection.ptr());
				}
				break;		
			case eCUSTOMSEMANTIC_INSTANCESPERROW:
				{
					glUniform1f(Uniforms[j].nLocation, (GLfloat)m_iInstancePerRow);
				}
				break;
			default:
				{
					PVRShellOutputDebug("Error: unknown uniform semantic.\n");
				}
				break;
			}
		}

		//	Now that all uniforms are set and the materials ready, draw the mesh.		
		glDrawElementsInstanced(GL_TRIANGLES, pMesh->nNumFaces*3, GL_UNSIGNED_SHORT, 0, m_uiNumInstances);

		// Disable all vertex attributes
		for(unsigned int j = 0; j < Uniforms.GetSize(); ++j)
		{
			switch(Uniforms[j].nSemantic)
			{
			case ePVRTPFX_UsPOSITION:
			case ePVRTPFX_UsNORMAL:
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
 @Function		LoadPFX
  @Return		bool			true if no error occurred
 @Description	Loads and compiles the shaders and links the shader programs
				required for this training course
******************************************************************************/
bool OGLES3Instancing::LoadPFX(CPVRTString *pErrorMsg)
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
			*pErrorMsg = CPVRTString("Failed to set custom semantics:\n\n") + error;
			return false;
		}

		unsigned int nUnknownUniformCount = 0;
		if(m_ppPFXEffects[i]->Load(*m_pPFXEffectParser, m_pPFXEffectParser->GetEffect(i).Name.c_str(), NULL, NULL, nUnknownUniformCount, &error)  != PVR_SUCCESS)
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
 @Function		NewDemo
 @Return		PVRShell*		The demo supplied by the user
 @Description	This function must be implemented by the user of the shell.
				The user should return its PVRShell object defining the
				behaviour of the application.
******************************************************************************/
PVRShell* NewDemo()
{
	return new OGLES3Instancing();
}

/******************************************************************************
 End of file (OGLES3Instancing.cpp)
******************************************************************************/

