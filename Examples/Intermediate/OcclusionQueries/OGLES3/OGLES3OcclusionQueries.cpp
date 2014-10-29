/******************************************************************************

 @File         OGLES3OcclusionQueries.cpp

 @Title        Occlusion Queries

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows how to use occlusion queries for visibility culling.

******************************************************************************/

#include "PVRShell.h"
#include "OGLES3Tools.h"

/******************************************************************************
 Global strings
******************************************************************************/

const CPVRTStringHash c_RenderDiffuseEffectName  = CPVRTStringHash("RenderDiffuse");
const CPVRTStringHash c_RenderMaterialColourEffectName = CPVRTStringHash("RenderMaterialColour");

/******************************************************************************
 Defines
******************************************************************************/

// Index to bind the attributes to vertex shaders
#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY	2

/******************************************************************************
 Content file names
******************************************************************************/

const CPVRTStringHash c_sTextureNames[] = 
{ 
	CPVRTStringHash("floor"), CPVRTStringHash("texture"),
};
const unsigned int c_uiNumTextureNames = sizeof(c_sTextureNames) / sizeof(c_sTextureNames[0]);

const char c_szPFXSrcFile[]	= "effect.pfx";
const char c_szSceneFile[]   = "scene.pod";

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES3OcclusionQueries : public PVRShell
{
	CPVRTPrint3D	m_Print3D;
	SPVRTContext	m_sContext;
	CPVRTModelPOD	m_Scene;
	
	// Transformation matrices
	PVRTMat4 m_mProjection;
	PVRTMat4 m_mView;
	PVRTMat4 m_mViewProjection;
	PVRTVec3 m_vLightPosition;
	
	GLenum m_eOcclusionQueryMethod;

	bool m_bRenderBoundingBoxes;
	bool m_bPause;
	bool m_bRotate;

	// OpenGL handles for shaders, textures and VBOs	
	GLuint* m_puiVbo;
	GLuint* m_puiIndexVbo;
	GLuint* m_puiBoundingBoxVbos;
	GLuint  m_uiBoundingBoxIBO;
	GLuint  m_uiBoundingBoxWireframeIBO;
	GLuint* m_puiQueryObjects;
		
	struct OcclusionQueryData
	{
		GLuint texture;
		unsigned int numVertices;
		unsigned int numTriangles;		
		bool   query_ongoing;
		bool   visible;
	};
	OcclusionQueryData *m_pOcclusionQueryData;

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

	bool LoadTextures(CPVRTString* pErrorStr);
	void LoadVbos();
	void LoadOcclusionQueryData();

	void UpdateOcclusionData();
	void Update();
	void CalculateBoundingBox(const SPODMesh& Mesh, PVRTVec3 &minCoord, PVRTVec3 &maxCoord);

	void RenderBoundingBox(const unsigned int uiEffectId, const GLuint vbo, const PVRTMat4 mModel, const PVRTVec4 vColour, const bool bWireframe);
	bool RenderSceneWithEffect(const int uiEffectId, const PVRTMat4 &mProjection, const PVRTMat4 &mView);
	bool LoadPFX(CPVRTString *pErrorMsg);
};

/*!****************************************************************************
 @Function		LoadTextures
 @Output		pErrorStr		A CPVRTString describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course
******************************************************************************/
bool OGLES3OcclusionQueries::LoadTextures(CPVRTString* const pszErrorStr)
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
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLES3OcclusionQueries::LoadVbos()
{
	if (!m_puiVbo) m_puiVbo = new GLuint[m_Scene.nNumMesh];
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
 @Function		LoadOcclusionQueryData
 @Description	Loads and calculates the data necessary for occlusion
                queries, which includes the bounding box information.
******************************************************************************/
void OGLES3OcclusionQueries::LoadOcclusionQueryData()
{
	if (!m_pOcclusionQueryData) m_pOcclusionQueryData = new OcclusionQueryData[m_Scene.nNumMesh];
	if (!m_puiBoundingBoxVbos) m_puiBoundingBoxVbos = new GLuint[m_Scene.nNumMesh];
	if (!m_puiQueryObjects) m_puiQueryObjects = new GLuint[m_Scene.nNumMesh];

	glGenQueries(m_Scene.nNumMesh, m_puiQueryObjects);
	glGenBuffers(m_Scene.nNumMesh, m_puiBoundingBoxVbos);

	for (unsigned int i = 0; i < m_Scene.nNumMesh; ++i)
	{
		// Load vertex data into buffer object
		SPODMesh& Mesh = m_Scene.pMesh[i];
		
		if (Mesh.sFaces.pData)
			m_pOcclusionQueryData[i].numTriangles = PVRTModelPODCountIndices(Mesh) / 3;

		PVRTVec3 minCoords, maxCoords;
		CalculateBoundingBox(Mesh, minCoords, maxCoords);

		// Make the bounding box slightly larger to avoid flickering
		PVRTVec3 middle = (minCoords + maxCoords) * 0.5f;
		minCoords = middle + (minCoords - middle) * 1.01f;
		maxCoords = middle + (maxCoords - middle) * 1.01f;

		PVRTVec3 bboxCoords[] = 
		{ 
			minCoords, PVRTVec3(maxCoords.x, minCoords.y, minCoords.z), 
			PVRTVec3(maxCoords.x, maxCoords.y, minCoords.z), PVRTVec3(minCoords.x, maxCoords.y, minCoords.z), 
			PVRTVec3(minCoords.x, minCoords.y, maxCoords.z), PVRTVec3(maxCoords.x, minCoords.y, maxCoords.z), 
			maxCoords, PVRTVec3(minCoords.x, maxCoords.y, maxCoords.z), 
		};

		glBindBuffer(GL_ARRAY_BUFFER, m_puiBoundingBoxVbos[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(bboxCoords), bboxCoords, GL_STATIC_DRAW);		
		
		m_pOcclusionQueryData[i].query_ongoing = false;
		m_pOcclusionQueryData[i].visible = true;
		
		m_pOcclusionQueryData[i].numVertices = Mesh.nNumVertex;		
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	
	GLushort wireframe_indices[] = 
	{ 
		0, 1, 1, 2,	2, 3, 3, 0,
		4, 5, 5, 6, 6, 7, 7, 4,
		0, 4, 1, 5, 2, 6, 3, 7
	};
	
	glGenBuffers(1, &m_uiBoundingBoxWireframeIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiBoundingBoxWireframeIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(wireframe_indices), wireframe_indices, GL_STATIC_DRAW);

	// Indices for the front, back, left, right, bottom and top faces
	GLushort indices[] = 
	{ 
		4, 5, 6,  4, 6, 7,
		1, 0, 3,  1, 3, 2,
		0, 4, 7,  0, 7, 3,
		5, 1, 2,  5, 2, 6,
		0, 4, 5,  0, 5, 1,
		3, 7, 6,  3, 6, 2
	};
	
	glGenBuffers(1, &m_uiBoundingBoxIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiBoundingBoxIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

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
bool OGLES3OcclusionQueries::InitApplication()
{
	m_puiVbo = 0;
	m_puiIndexVbo = 0;
	m_puiBoundingBoxVbos = 0;
	m_puiQueryObjects = 0;
	m_pOcclusionQueryData = 0;

	// Set the default to the binary query method
	m_eOcclusionQueryMethod = GL_ANY_SAMPLES_PASSED;

	m_bRenderBoundingBoxes = false;
	m_bPause = false;

	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	// Load the scene
	if (m_Scene.ReadFromFile(c_szSceneFile) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Couldn't load the scene.pod file\n");
		return false;
	}

	if (m_Scene.nNumCamera == 0)
	{
		PVRShellSet(prefExitMessage, "ERROR: The .pod file does not contain any cameras\n");
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
bool OGLES3OcclusionQueries::QuitApplication()
{
	// Free the memory allocated for the scene
	m_Scene.Destroy();

	delete [] m_puiVbo;
	delete [] m_puiIndexVbo;
	delete [] m_puiBoundingBoxVbos;
	delete [] m_puiQueryObjects;
	delete [] m_pOcclusionQueryData;

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
bool OGLES3OcclusionQueries::InitView()
{
	CPVRTString ErrorStr;

	//  Initialize VBO data
	//
	LoadVbos();

	//	Load textures
	//
	if (!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	//	Load and compile the shaders & link programs
	//
	if (!LoadPFX(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}		
    
	// Is the screen rotated?
	m_bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);
	
	//	Initialize Print3D
	//
	if(m_Print3D.SetTextures(NULL, PVRShellGet(prefWidth),PVRShellGet(prefHeight), m_bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}
				
	//  Initialize the occlusion data
	//
	LoadOcclusionQueryData();

	//	Set OpenGL render states needed for this training course
	//
	// Enable backface culling and depth test
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	
	glDepthFunc(GL_LEQUAL);
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
bool OGLES3OcclusionQueries::ReleaseView()
{
	// Delete query objects
	glDeleteQueries(1, m_puiQueryObjects);
	
	// Delete buffer objects
	glDeleteBuffers(m_Scene.nNumMesh, m_puiVbo);
	glDeleteBuffers(m_Scene.nNumMesh, m_puiIndexVbo);
	glDeleteBuffers(m_Scene.nNumMesh, m_puiBoundingBoxVbos);

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
 @Function		RenderScene
 @Return		bool		true if no error occured
 @Description	Main rendering loop function of the program. The shell will
				call this function every frame.
				eglSwapBuffers() will be performed by PVRShell automatically.
				PVRShell will also manage important OS events.
				Will also manage relevent OS events. The user has access to
				these events through an abstraction layer provided by PVRShell.
******************************************************************************/
bool OGLES3OcclusionQueries::RenderScene()
{	
	// Handle user input
	if (PVRShellIsKeyPressed(PVRShellKeyNameSELECT))
		m_bRenderBoundingBoxes = !m_bRenderBoundingBoxes;
	if (PVRShellIsKeyPressed(PVRShellKeyNameACTION2))
		m_bPause = !m_bPause;
	if (PVRShellIsKeyPressed(PVRShellKeyNameACTION1))
		m_eOcclusionQueryMethod = (m_eOcclusionQueryMethod == GL_ANY_SAMPLES_PASSED) ? GL_ANY_SAMPLES_PASSED_CONSERVATIVE : GL_ANY_SAMPLES_PASSED;		

	// Update the timer related information like camera animation
	Update();	

	// Clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
	// Render the visible objects
	//
	RenderSceneWithEffect(m_pPFXEffectParser->FindEffectByName(c_RenderDiffuseEffectName), m_mProjection, m_mView);
	
	/*
	   Occlusion queries enable the developer to query the amount of fragments drawn by OpenGL:

	   The first step is to generate a query object (glGenQueries) and use this to issue a query (glBeginQuery/glEndQuery).
	   All fragments that are written to the framebuffer within the glBeginQuery/glEndQuery pair will be counted with the
	   query object's fragment counter.
	   This counter can be read back with glGetQueryObject(GL_QUERY_RESULT),
	   but as occlusion queries run asynchronously to the program execution the developer has to make sure first 
	   that the query actually finished by checking the status with GL_QUERY_RESULT_AVAILABLE.   
	 */

	// Disable depth and colour writes to preserve the buffers during the occlusion query
	glDepthMask(GL_FALSE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	const int iEffectId = m_pPFXEffectParser->FindEffectByName(c_RenderMaterialColourEffectName);
	m_ppPFXEffects[iEffectId]->Activate();

	// Issue an occlusion query for each object's bounding box
	//
	for (unsigned int i=0; i < m_Scene.nNumMesh; i++)
	{
		// Check first that the previous query has finished
		if (!m_pOcclusionQueryData[i].query_ongoing)
		{
			// No active query for this object, so issue a new one
			m_pOcclusionQueryData[i].query_ongoing = true;
			glBeginQuery(m_eOcclusionQueryMethod, m_puiQueryObjects[i]); 								
			RenderBoundingBox(iEffectId, m_puiBoundingBoxVbos[i], m_Scene.GetWorldMatrix(m_Scene.pNode[i]), PVRTVec4(1.0f), false);			
			glEndQuery(m_eOcclusionQueryMethod);
		}				
	}	

	// Enable the depth and colour masks again for subsequent renders
	glDepthMask(GL_TRUE);	
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);	
	
	// Render the wireframe bounding boxes
	//
	if (m_bRenderBoundingBoxes)
	{		
		// First render the visible boxes
		for (unsigned int i=0; i < m_Scene.nNumMesh; i++)
		{
			if (m_pOcclusionQueryData[i].visible)
				RenderBoundingBox(iEffectId, m_puiBoundingBoxVbos[i], m_Scene.GetWorldMatrix(m_Scene.pNode[i]), PVRTVec4(1.0f), true);			
		}

		// Then render the invisible ones. 
		// Disable the depth test so they are drawn on top of the objects to highlight they fact that they are occluded.
		glDisable(GL_DEPTH_TEST);
		for (unsigned int i=0; i < m_Scene.nNumMesh; i++)
		{
			if (!m_pOcclusionQueryData[i].visible)
				RenderBoundingBox(iEffectId, m_puiBoundingBoxVbos[i], m_Scene.GetWorldMatrix(m_Scene.pNode[i]), PVRTVec4(1.0f, 0.0f, 0.0f, 1.0f), true);			
		}
		glEnable(GL_DEPTH_TEST);
	}
	
	// Update the visibility information
	UpdateOcclusionData();
	
	// Update the visibility numbers
	unsigned int numActiveObjects = 0;
	unsigned int numSubmittedTriangles = 0;	

	for (unsigned int i=0; i < m_Scene.nNumMesh; ++i)
	{
		if (m_pOcclusionQueryData[i].visible)
		{
			numActiveObjects++;
			numSubmittedTriangles += m_pOcclusionQueryData[i].numTriangles;
		}
	}
	
	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("Occlusion Queries", "", ePVRTPrint3DSDKLogo);
	m_Print3D.Print3D(0.5f, 9.0f, 1.0f, 0xFFFFFFFF, "Visible objects: %d", numActiveObjects);
	m_Print3D.Print3D(0.5f, 92.0f, 1.0f, 0xFFFFFFFF, "Triangles: %d", numSubmittedTriangles);	
	m_Print3D.Flush();		
	
	return true;
}


/*!****************************************************************************
 @Function		UpdateOcclusionData
 @Description	Reads back the occlusion query data from all query objects.
******************************************************************************/
void OGLES3OcclusionQueries::UpdateOcclusionData()
{	
	// Read back the occlusion query results
	//
	for (unsigned int i=0; i < m_Scene.nNumMesh; i++)
	{
		// First check if they are available
		//
		GLuint available = GL_FALSE;		
		glGetQueryObjectuiv(m_puiQueryObjects[i], GL_QUERY_RESULT_AVAILABLE, &available);

		if (available == GL_TRUE)
		{
			// Then read back the amount of fragments written (regardless whether GL_SAMPLES_PASSED/GL_ANY_SAMPLES_PASSED
			// has been used as we simply test fragments_written > 0)
			//
			m_pOcclusionQueryData[i].query_ongoing = false;
			GLuint samplesPassed = 0;
			glGetQueryObjectuiv(m_puiQueryObjects[i], GL_QUERY_RESULT, &samplesPassed);
			if (samplesPassed > 0)
				m_pOcclusionQueryData[i].visible = true;
			else
				m_pOcclusionQueryData[i].visible = false;
		}
	}
}

/*!****************************************************************************
 @Function		UpdateTimer
 @Description	Updates timer information for camera animation
******************************************************************************/
void OGLES3OcclusionQueries::Update()
{
	// Calculates the frame number to animate in a time-based manner.
	// Uses the shell function PVRShellGetTime() to get the time in milliseconds.
	static unsigned long ulTimePrev = PVRShellGetTime();
	unsigned long ulTime = PVRShellGetTime();
	unsigned long ulDeltaTime = ulTime - ulTimePrev;
	ulTimePrev = ulTime;
	static float fFrame = 0;
	if (!m_bPause)
		fFrame += (float)ulDeltaTime * 0.05f;

	if (fFrame > m_Scene.nNumFrame-1)
		fFrame = 0;

	// Update the animation data
	m_Scene.SetFrame(fFrame);

	PVRTVec3 vFrom, vTo, vUp;	
	float fFOV = m_Scene.GetCamera(vFrom, vTo, vUp, 0) * 0.75f;

	m_mProjection = PVRTMat4::PerspectiveFovRH(fFOV, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), m_Scene.pCamera[0].fNear, m_Scene.pCamera[0].fFar, PVRTMat4::OGL, m_bRotate);
	m_mView = PVRTMat4::LookAtRH(vFrom, vTo, vUp);
	m_mViewProjection = m_mProjection * m_mView;

	PVRTVec3 vDir;
	m_Scene.GetLight(m_vLightPosition, vDir, 0);
}


/*!****************************************************************************
 @Function		CalculateBoundingBox
 @Description	Calculates the axis aligned bounding box for a SPODMesh.
******************************************************************************/
void OGLES3OcclusionQueries::CalculateBoundingBox(const SPODMesh& mesh, PVRTVec3 &minCoord, PVRTVec3 &maxCoord)
{
	minCoord = PVRTVec3(999999.9f);
	maxCoord = PVRTVec3(-999999.9f);
	for (PVRTuint32 i=0; i < mesh.nNumVertex; i++)
	{
		PVRTVec3 v = *(PVRTVec3*)(mesh.pInterleaved + mesh.sVertex.nStride * i);
		minCoord.x = PVRT_MIN(minCoord.x, v.x);
		minCoord.y = PVRT_MIN(minCoord.y, v.y);
		minCoord.z = PVRT_MIN(minCoord.z, v.z);
		maxCoord.x = PVRT_MAX(maxCoord.x, v.x);
		maxCoord.y = PVRT_MAX(maxCoord.y, v.y);
		maxCoord.z = PVRT_MAX(maxCoord.z, v.z);
	}
}

/*!****************************************************************************
 @Function		RenderBoundingBox
 @Description	Renders the whole bounding box cube as filled polygons.
******************************************************************************/
void OGLES3OcclusionQueries::RenderBoundingBox(const unsigned int uiEffectId, const GLuint vbo, const PVRTMat4 mModel, const PVRTVec4 vColour, const bool bWireframe)
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// Bind semantics
	const CPVRTArray<SPVRTPFXUniform>& Uniforms = m_ppPFXEffects[uiEffectId]->GetUniformArray();
	for(unsigned int j = 0; j < Uniforms.GetSize(); ++j)
	{
		switch(Uniforms[j].nSemantic)
		{
		case ePVRTPFX_UsPOSITION:
			{
				glVertexAttribPointer(Uniforms[j].nLocation, 3, GL_FLOAT, GL_FALSE, 0, NULL);
				glEnableVertexAttribArray(Uniforms[j].nLocation);
			}
			break;		
		case ePVRTPFX_UsMATERIALCOLORDIFFUSE:
			{										
				glUniform4fv(Uniforms[j].nLocation, 1, &vColour.x);
			}
			break;			
		case ePVRTPFX_UsWORLDVIEWPROJECTION:
			{
				PVRTMat4 mModelViewProjection = m_mViewProjection * mModel;
				glUniformMatrix4fv(Uniforms[j].nLocation, 1, GL_FALSE, mModelViewProjection.f);
			}
			break;
		default:
			{
				PVRShellOutputDebug("Error: Unhandled semantic in RenderBoundingBox()\n");
				return;
			}
		}
	}
		
	if (bWireframe)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiBoundingBoxWireframeIBO);
		glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, NULL);
	}
	else
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiBoundingBoxIBO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, NULL);
	}

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

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/*!****************************************************************************
 @Function		RenderSceneWithEffect
 @Return		bool		true if no error occured
 @Description	Renders the whole scene with a single effect.
******************************************************************************/
bool OGLES3OcclusionQueries::RenderSceneWithEffect(const int uiEffectId, const PVRTMat4 &mProjection, const PVRTMat4 &mView)
{
	CPVRTPFXEffect *pEffect = m_ppPFXEffects[uiEffectId];

	// Activate the passed effect
	pEffect->Activate();
	
	for (unsigned int i=0; i < m_Scene.nNumMesh; i++)
	{
		// Check if the object is visible and skip if invisible
		if (!m_pOcclusionQueryData[i].visible)
			continue;
		
		SPODNode* pNode = &m_Scene.pNode[i];
		SPODMesh* pMesh = &m_Scene.pMesh[pNode->nIdx];
		SPODMaterial *pMaterial = &m_Scene.pMaterial[pNode->nIdxMaterial];	

		// Bind the texture if there is one bound to this object
		if (pMaterial->nIdxTexDiffuse != -1)
		{
			CPVRTString texname = CPVRTString(m_Scene.pTexture[pMaterial->nIdxTexDiffuse].pszName);
			texname.substitute(".png", "");
			CPVRTStringHash hashedName(texname.toLower());
			if (m_TextureCache.Exists(hashedName))
				glBindTexture(GL_TEXTURE_2D, m_TextureCache[hashedName]);
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
					glUniform4f(Uniforms[j].nLocation, pMaterial->pfMatDiffuse[0], pMaterial->pfMatDiffuse[1], pMaterial->pfMatDiffuse[2], 1.0f);
				}
				break;			
			case ePVRTPFX_UsWORLDVIEWPROJECTION:
				{
					PVRTMat4 mWorldViewProj = mProjection * mWorldView;					
					glUniformMatrix4fv(Uniforms[j].nLocation, 1, GL_FALSE, mWorldViewProj.f);
				}
				break;
			case ePVRTPFX_UsWORLDVIEW:
				{
					glUniformMatrix4fv(Uniforms[j].nLocation, 1, GL_FALSE, mWorldView.f);
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
			case ePVRTPFX_UsLIGHTPOSEYE:
				{				
					PVRTVec4 vLightPosView = (m_mView * PVRTVec4(m_vLightPosition, 1.0f));
					glUniform3fv(Uniforms[j].nLocation, 1, vLightPosView.ptr());
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
 @Function		LoadPFX
  @Return		bool			true if no error occurred
 @Description	Loads and compiles the shaders and links the shader programs
				required for this training course
******************************************************************************/
bool OGLES3OcclusionQueries::LoadPFX(CPVRTString *pErrorMsg)
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
	return new OGLES3OcclusionQueries();
}

/******************************************************************************
 End of file (OGLES3OcclusionQueries.cpp)
******************************************************************************/

