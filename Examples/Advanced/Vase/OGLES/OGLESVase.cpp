/******************************************************************************

 @File         OGLESVase.cpp

 @Title        Vase

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows textured transparency and reflections. Requires the PVRShell.

******************************************************************************/

/****************************************************************************
** Includes
****************************************************************************/
#include <math.h>
#include "PVRShell.h"
#include "OGLESTools.h"

/******************************************************************************
 Content file names
******************************************************************************/

// PVR texture files
const char c_szBackgroundTexFile[]  = "Backgrnd.pvr";
const char c_szFloraTexFile[]		= "Flora.pvr";
const char c_szReflectionTexFile[]	= "Reflection.pvr";

// Scene
const char c_szSceneFile[] = "Vase.pod";

/******************************************************************************
 Global variables
******************************************************************************/

// Camera constants. Used for making the projection matrix
const float g_fCameraNear = 4.0f;
const float g_fCameraFar  = 500.0f;

enum EMesh
{
	eGlass,
	eVase
};

/****************************************************************************
** Class: OGLESVase
****************************************************************************/
class OGLESVase : public PVRShell
{
	/* Print3D class */
	CPVRTPrint3D 	m_Print3D;

	// 3D Model
	CPVRTModelPOD	m_Scene;

	// OpenGL handles for textures and VBOs
	GLuint	m_uiBackTex;
	GLuint	m_uiFloraTex;
	GLuint	m_uiReflectTex;

	GLuint*	m_puiVbo;
	GLuint*	m_puiIndexVbo;

	// Array to lookup the textures for each material in the scene
	GLuint*	m_pui32Textures;

    // Rotation variables
	float m_fAngleX, m_fAngleY;

	PVRTMat4 m_mProjection;
	PVRTMat4 m_mView;

	// Class for drawing the background
	CPVRTBackground m_Background;

public:
	OGLESVase()  :	m_puiVbo(0),
					m_puiIndexVbo(0),
					m_fAngleX(0),
					m_fAngleY(0)
	{
	}

	/* PVRShell functions */
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	/****************************************************************************
	** Function Definitions
	****************************************************************************/
	bool LoadTextures(CPVRTString* const pErrorStr);
	void LoadVbos();
	void DrawMesh(unsigned int ui32MeshID);
	void DrawReflectiveMesh(unsigned int ui32MeshID, PVRTMat4 *pNormalTx);
};


/*******************************************************************************
 * Function Name  : InitApplication
 * Returns        : true if no error occured
 * Description    : Code in InitApplication() will be called by the Shell ONCE per
 *					run, early on in the execution of the program.
 *					Used to initialize variables that are not dependant on the
 *					rendering context (e.g. external modules, loading meshes, etc.)
 *******************************************************************************/
bool OGLESVase::InitApplication()
{
	// Get and set the read path for content files
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

	return true;
}

/*******************************************************************************
 * Function Name  : QuitApplication
 * Returns        : true if no error occured
 * Description    : Code in QuitApplication() will be called by the Shell ONCE per
 *					run, just before exiting the program.
 *******************************************************************************/
bool OGLESVase::QuitApplication()
{
	// Free the memory allocated for the scene
	m_Scene.Destroy();

	delete[] m_puiVbo;
	delete[] m_puiIndexVbo;

	return true;
}

/*******************************************************************************
 * Function Name  : InitView
 * Returns        : true if no error occured
 * Description    : Code in InitView() will be called by the Shell upon a change
 *					in the rendering context.
 *					Used to initialize variables that are dependant on the rendering
 *					context (e.g. textures, vertex buffers, etc.)
 *******************************************************************************/
bool OGLESVase::InitView()
{
	CPVRTString  ErrorStr;
	SPVRTContext Context;

	// Is the screen rotated?
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	// Initialize Print3D textures
	if(m_Print3D.SetTextures(&Context, PVRShellGet(prefWidth), PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	// Load textures
	if(!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	//	Initialize VBO data
	LoadVbos();

	// Initialize Background
	if(m_Background.Init(0, bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Background\n");
		return false;
	}

	/*
		Build an array to map the textures within the pod file
		to the textures we loaded earlier.
	*/

	m_pui32Textures = new GLuint[m_Scene.nNumMaterial];

	for(unsigned int i = 0; i < m_Scene.nNumMaterial; ++i)
	{
		m_pui32Textures[i] = 0;
		SPODMaterial* pMaterial = &m_Scene.pMaterial[i];

		if(!strcmp(pMaterial->pszName, "Flora"))
			m_pui32Textures[i] = m_uiFloraTex;
		else if(!strcmp(pMaterial->pszName, "Reflection"))
			m_pui32Textures[i] = m_uiReflectTex;
	}

	// Calculates the projection matrix
	m_mProjection = PVRTMat4::PerspectiveFovRH(35.0f*(3.14f/180.0f), (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), g_fCameraNear, g_fCameraFar, PVRTMat4::OGL, bRotate);

	// Loads the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(m_mProjection.f);

	// Enable texturing
	glEnable(GL_TEXTURE_2D);

	// Setup clear colour
	glClearColor(1.0f,1.0f,1.0f,1.0f);

	// Set blend mode
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLESVase::LoadVbos()
{
	if(!m_puiVbo)
		m_puiVbo = new GLuint[m_Scene.nNumMesh];

	if(!m_puiIndexVbo)
		m_puiIndexVbo = new GLuint[m_Scene.nNumMesh];

	/*
		Load vertex data of all meshes in the scene into VBOs

		The meshes have been exported with the "Interleave Vectors" option,
		so all data is interleaved in the buffer at pMesh->pInterleaved.
		Interleaving data improves the memory access pattern and cache efficiency,
		thus it can be read faster by the hardware.
	*/

	glGenBuffers(m_Scene.nNumMesh, m_puiVbo);

	for(unsigned int i = 0; i < m_Scene.nNumMesh; ++i)
	{
		// Load vertex data into buffer object
		SPODMesh& Mesh = m_Scene.pMesh[i];
		unsigned int uiSize = Mesh.nNumVertex * Mesh.sVertex.nStride;

		glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[i]);
		glBufferData(GL_ARRAY_BUFFER, uiSize, Mesh.pInterleaved, GL_STATIC_DRAW);

		// Load index data into buffer object if available
		m_puiIndexVbo[i] = 0;

		if(Mesh.sFaces.pData)
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
 @Function		LoadTextures
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course
******************************************************************************/
bool OGLESVase::LoadTextures(CPVRTString* const pErrorStr)
{
	if(PVRTTextureLoadFromPVR(c_szBackgroundTexFile, &m_uiBackTex) != PVR_SUCCESS)
	{
		*pErrorStr = CPVRTString("ERROR: Failed to load texture ") + c_szBackgroundTexFile;
		return false;
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szFloraTexFile, &m_uiFloraTex) != PVR_SUCCESS)
	{
		*pErrorStr = CPVRTString("ERROR: Failed to load texture ") + c_szFloraTexFile;
		return false;
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szReflectionTexFile, &m_uiReflectTex) != PVR_SUCCESS)
	{
		*pErrorStr = CPVRTString("ERROR: Failed to load texture ") + c_szReflectionTexFile;
		return false;
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return true;
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns        : Nothing
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool OGLESVase::ReleaseView()
{
	// Release textures
	glDeleteTextures(1, &m_uiBackTex);
	glDeleteTextures(1, &m_uiFloraTex);
	glDeleteTextures(1, &m_uiReflectTex);

	delete[] m_pui32Textures;
	m_pui32Textures = 0;

	// Release Print3D textures
	m_Print3D.ReleaseTextures();
	return true;
}

/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool OGLESVase::RenderScene()
{
	PVRTMat4 RotationMatrix, RotateX, RotateY;

	// Increase rotation angles
	m_fAngleX += PVRT_PI / 100.0f;
	m_fAngleY += PVRT_PI /150.0f;

	if(m_fAngleX >= PVRT_PI)
		m_fAngleX -= PVRT_TWO_PI;

	if(m_fAngleY >= PVRT_PI)
		m_fAngleY -= PVRT_TWO_PI;

	// Clear the buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Setup the vase rotation

	// Calculate rotation matrix
	RotateX = PVRTMat4::RotationX(m_fAngleX);
	RotateY = PVRTMat4::RotationY(m_fAngleY);

	RotationMatrix = RotateY * RotateX;

	// Modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -200.0f);
	glMultMatrixf(RotationMatrix.f);

	// Draw the scene

	// Use PVRTools to draw a background image
	m_Background.Draw(m_uiBackTex);


	// Enable client states
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);

	// Draw vase outer
	glBindTexture(GL_TEXTURE_2D, m_pui32Textures[m_Scene.pNode[eVase].nIdxMaterial]);
	DrawReflectiveMesh(m_Scene.pNode[eVase].nIdx, &RotationMatrix);

	// Draw glass
	glEnable(GL_BLEND);

	glBindTexture(GL_TEXTURE_2D, m_pui32Textures[m_Scene.pNode[eGlass].nIdxMaterial]);

	// Pass 1: only render back faces (model has reverse winding)
	glFrontFace(GL_CW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	DrawMesh(m_Scene.pNode[eGlass].nIdx);

	// Pass 2: only render front faces (model has reverse winding)
	glCullFace(GL_FRONT);
	DrawMesh(m_Scene.pNode[eGlass].nIdx);

	// Disable blending as it isn't needed
	glDisable(GL_BLEND);

	// Disable client states
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	// Display info text
	m_Print3D.DisplayDefaultTitle("Vase", "Translucency and reflections", ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();
	return true;
}

/*!****************************************************************************
 @Function		DrawReflectiveMesh
 @Input			ui32MeshID ID of mesh to draw
 @Input			pNormalTx Rotation matrix
 @Description	Draws a mesh with the reflection
******************************************************************************/
void OGLESVase::DrawReflectiveMesh(unsigned int ui32MeshID, PVRTMat4 *pNormalTx)
{
	SPODMesh& Mesh = m_Scene.pMesh[ui32MeshID];

	float		*pUVs = new float[2 * Mesh.nNumVertex];
	PVRTMat4		EnvMapMatrix;
	unsigned int	i;

	// Calculate matrix for environment mapping: simple multiply by 0.5
	for(i = 0; i < 16; ++i)
		EnvMapMatrix.f[i] = pNormalTx->f[i] * 0.5f;

	unsigned char* pNormals = Mesh.pInterleaved + (size_t) Mesh.sNormals.pData;

	/* Calculate UVs for environment mapping */
	for(i = 0; i < Mesh.nNumVertex; ++i)
	{
		float *pVTNormals = (float*) pNormals;

		pUVs[2*i] =	pVTNormals[0] * EnvMapMatrix.f[0] +
					pVTNormals[1] * EnvMapMatrix.f[4] +
					pVTNormals[2] * EnvMapMatrix.f[8] +
					0.5f;

		pUVs[2*i+1] = pVTNormals[0] * EnvMapMatrix.f[1] +
					  pVTNormals[1] * EnvMapMatrix.f[5] +
					  pVTNormals[2] * EnvMapMatrix.f[9] +
					  0.5f;

		pNormals += Mesh.sNormals.nStride;
	}

	// Bind the vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[ui32MeshID]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[ui32MeshID]);

	// Setup pointers
	glVertexPointer(3, GL_FLOAT, Mesh.sVertex.nStride, Mesh.sVertex.pData);

	// unbind the vertex buffer as we don't need them bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glTexCoordPointer(2, GL_FLOAT, 0, pUVs);

	glDrawElements(GL_TRIANGLES, Mesh.nNumFaces * 3, GL_UNSIGNED_SHORT, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	delete[] pUVs;
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			ID of mesh to draw
 @Description	Draws a mesh.
******************************************************************************/
void OGLESVase::DrawMesh(unsigned int ui32MeshID)
{
	SPODMesh& Mesh = m_Scene.pMesh[ui32MeshID];

	// Bind the vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[ui32MeshID]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[ui32MeshID]);

	// Setup pointers
	glVertexPointer(3, GL_FLOAT, Mesh.sVertex.nStride, Mesh.sVertex.pData);
	glTexCoordPointer(2, GL_FLOAT, Mesh.psUVW[0].nStride, Mesh.psUVW[0].pData);

	glDrawElements(GL_TRIANGLES, Mesh.nNumFaces * 3, GL_UNSIGNED_SHORT, 0);

	// unbind the vertex buffers as we don't need them bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/*******************************************************************************
 * Function Name  : NewDemo
 * Description    : Called by the Shell to initialize a new instance to the
 *					demo class.
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new OGLESVase();
}

/*****************************************************************************
 End of file (OGLESVase.cpp)
*****************************************************************************/

