/******************************************************************************

 @File         OGLESOptimizeMesh.cpp

 @Title        OptimizeMesh

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows the difference between a mesh that has been optimised by our
               POD exporters and one that hasn't.

******************************************************************************/

/*****************************************************************************
** INCLUDES
*****************************************************************************/
#include "PVRShell.h"
#include "OGLESTools.h"

/******************************************************************************
 Content file names
******************************************************************************/

// POD File
const char c_szSatyr[]	  = "Satyr.pod";
const char c_szSatyrOpt[] = "SatyrOpt.pod";

/****************************************************************************
** Constants
****************************************************************************/
const unsigned int m_ui32VBONo = 2;

const unsigned int m_ui32IndexVBONo = 2;
const unsigned int m_ui32PageNo		= 2;

const float g_fViewDistance = 2000.0f;

// Times in milliseconds
const unsigned int g_ui32TimeAutoSwitch = 4000;
const unsigned int g_ui32TimeFPSUpdate  = 500;

// Assuming a 4:3 aspect ratio:
const float g_fCameraNear = 4.0f;
const float g_fCameraFar  = 5000.0f;

/****************************************************************************
** Class: OGLESOptimizeMesh
****************************************************************************/
class OGLESOptimizeMesh : public PVRShell
{
	// Print3D, Extension and POD Class Objects
	CPVRTPrint3D 		m_Print3D;
	CPVRTModelPOD		m_Model;	// Model
	CPVRTModelPOD		m_ModelOpt;	// Triangle optimized model

	// OpenGL handles for textures and VBOs
	GLuint*	m_puiVbo;
	GLuint*	m_puiIndexVbo;

	// View and Projection Matrices
	PVRTMat4	m_mView, m_mProj;
	float		m_fViewAngle;

	// Used to switch mode (not optimized / optimized) after a while
	unsigned long	m_ui32SwitchTimeDiff;
	int				m_i32Page;

	// Time variables
	unsigned long	m_ui32LastTime, m_ui32TimeDiff;

	// FPS variables
	unsigned long	m_ui32FPSTimeDiff, m_ui32FPSFrameCnt;
	float			m_fFPS;

public:
	OGLESOptimizeMesh() : 	m_puiVbo(0),
							m_puiIndexVbo(0)
	{
	}

	// PVRShell functions
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	/****************************************************************************
	** Function Definitions
	****************************************************************************/
	void DrawModel( int iOptim );
	void LoadVbos();
};

/*******************************************************************************
 * Function Name  : InitApplication
 * Inputs		  :
 * Returns        : true if no error occurred
 * Description    : Code in InitApplication() will be called by the Shell ONCE per
 *					run, early on in the execution of the program.
 *					Used to initialize variables that are not dependent on the
 *					rendering context (e.g. external modules, loading meshes, etc.)
 *******************************************************************************/
bool OGLESOptimizeMesh::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	// Set some parameters in the Shell
	PVRShellSet(prefAppName, "OptimizeMesh");
	PVRShellSet(prefSwapInterval, 0);

	// Load POD File Data

	// Load the meshes
	if(m_Model.ReadFromFile(c_szSatyr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load Satyr_*.pod!");
		return false;
	}

	if(m_ModelOpt.ReadFromFile(c_szSatyrOpt) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load SatyrOpt_*.pod!");
		return false;
	}

	// Init values to defaults
	m_i32Page = 0;
	return true;
}


/*******************************************************************************
 * Function Name  : QuitApplication
 * Returns        : true if no error occurred
 * Description    : Code in QuitApplication() will be called by the Shell ONCE per
 *					run, just before exiting the program.
 *******************************************************************************/
bool OGLESOptimizeMesh::QuitApplication()
{
	m_Model.Destroy();
	m_ModelOpt.Destroy();

	delete[] m_puiVbo;
	delete[] m_puiIndexVbo;

	return true;
}

/*******************************************************************************
 * Function Name  : InitView
 * Inputs		  :
 * Returns        : true if no error occurred
 * Description    : Code in InitView() will be called by the Shell upon a change
 *					in the rendering context.
 *					Used to initialize variables that are dependent on the rendering
 *					context (e.g. textures, vertex buffers, etc.)
 *******************************************************************************/
bool OGLESOptimizeMesh::InitView()
{
	SPVRTContext sContext;

	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	// Init Print3D to display text on screen
	if(m_Print3D.SetTextures(&sContext, PVRShellGet(prefWidth), PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D.\n");
		return false;
	}

	/*********************************
	** View and Projection Matrices **
	*********************************/

	// Get Camera info from POD file
	PVRTVec3 From, To;

	// View
	m_mView = PVRTMat4::LookAtRH(From, To, PVRTVec3(0.0f, 1.0f, 0.0f));

	// Projection
	m_mProj = PVRTMat4::PerspectiveFovRH(PVRT_PIf / 6, (float) PVRShellGet(prefWidth) / (float) PVRShellGet(prefHeight), g_fCameraNear, g_fCameraFar, PVRTMat4::OGL, bRotate);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(m_mProj.f);

	/******************************
	** GENERIC RENDER STATES     **
	******************************/

	// The Type Of Depth Test To Do
	glDepthFunc(GL_LEQUAL);

	// Enables Depth Testing
	glEnable(GL_DEPTH_TEST);

	// Enables Smooth Color Shading
	glShadeModel(GL_SMOOTH);

	// Define front faces
	glFrontFace(GL_CW);

	// Sets the clear colour
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	// Reset the model view matrix to position the light
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Setup timing variables
	m_ui32LastTime = PVRShellGetTime();
	m_ui32FPSFrameCnt = 0;
	m_fFPS = 0;
	m_fViewAngle = 0;
	m_ui32SwitchTimeDiff = 0;

	LoadVbos();

	// Enable culling
	glEnable(GL_CULL_FACE);
	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLESOptimizeMesh::LoadVbos()
{
	if(!m_puiVbo)
		m_puiVbo = new GLuint[m_ui32VBONo];

	if(!m_puiIndexVbo)
		m_puiIndexVbo = new GLuint[m_ui32IndexVBONo];

	glGenBuffers(m_ui32VBONo, m_puiVbo);
	glGenBuffers(m_ui32IndexVBONo, m_puiIndexVbo);

	// Create vertex buffer for Model

	// Load vertex data into buffer object
	unsigned int uiSize = m_Model.pMesh[0].nNumVertex * m_Model.pMesh[0].sVertex.nStride;

	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[0]);
	glBufferData(GL_ARRAY_BUFFER, uiSize, m_Model.pMesh[0].pInterleaved, GL_STATIC_DRAW);

	// Load index data into buffer object if available
	uiSize = PVRTModelPODCountIndices(m_Model.pMesh[0]) * sizeof(GLshort);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, m_Model.pMesh[0].sFaces.pData, GL_STATIC_DRAW);

	// Create vertex buffer for ModelOpt

	// Load vertex data into buffer object
	uiSize = m_ModelOpt.pMesh[0].nNumVertex * m_ModelOpt.pMesh[0].sVertex.nStride;

	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[1]);
	glBufferData(GL_ARRAY_BUFFER, uiSize, m_ModelOpt.pMesh[0].pInterleaved, GL_STATIC_DRAW);

	// Load index data into buffer object if available
	uiSize = PVRTModelPODCountIndices(m_ModelOpt.pMesh[0]) * sizeof(GLshort);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, m_ModelOpt.pMesh[0].sFaces.pData, GL_STATIC_DRAW);

	// Unbind buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns        : Nothing
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool OGLESOptimizeMesh::ReleaseView()
{
	// Release the Print3D textures
	m_Print3D.ReleaseTextures();
	return true;
}

/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool OGLESOptimizeMesh::RenderScene()
{
	unsigned long ui32Time;

	// Clear the depth and frame buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//	Time
	ui32Time = PVRShellGetTime();
	m_ui32TimeDiff = ui32Time - m_ui32LastTime;
	m_ui32LastTime = ui32Time;

	// FPS
	++m_ui32FPSFrameCnt;
	m_ui32FPSTimeDiff += m_ui32TimeDiff;

	if(m_ui32FPSTimeDiff >= g_ui32TimeFPSUpdate)
	{
		m_fFPS = m_ui32FPSFrameCnt * 1000.0f / (float) m_ui32FPSTimeDiff;
		m_ui32FPSFrameCnt = 0;
		m_ui32FPSTimeDiff = 0;
	}

	// Change mode when necessary
	m_ui32SwitchTimeDiff += m_ui32TimeDiff;

	if((m_ui32SwitchTimeDiff > g_ui32TimeAutoSwitch) || PVRShellIsKeyPressed(PVRShellKeyNameACTION1))
	{
		m_ui32SwitchTimeDiff = 0;
		++m_i32Page;

		if(m_i32Page >= (int) m_ui32PageNo)
			m_i32Page = 0;
	}

	PVRTVec3 From;
	float fFactor;

	From.x = g_fViewDistance * PVRTSIN(m_fViewAngle);
	From.y = 0.0f;
	From.z = g_fViewDistance * PVRTCOS(m_fViewAngle);

	// Increase the rotation
	fFactor = 0.005f * (float) m_ui32TimeDiff;
	m_fViewAngle += fFactor;

	// Ensure it doesn't grow huge and lose accuracy over time
	while(m_fViewAngle > PVRT_PI)
		m_fViewAngle -= PVRT_TWO_PI;

	// Compute and set the matrix
	m_mView = PVRTMat4::LookAtRH(From, PVRTVec3(0,0,0), PVRTVec3(0,1,0));

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(m_mView.f);

	// Setup the lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	PVRTVec4 vLightDir(From.x, From.y, From.z, 0);
	vLightDir = vLightDir.normalize();

	// Set the light direction
	glLightfv(GL_LIGHT0, GL_POSITION, vLightDir.ptr());
	glLightfv(GL_LIGHT0, GL_DIFFUSE, PVRTVec4(0.8f,0.8f,0.8f,1.0f).ptr());

	// Draw the model
	DrawModel(m_i32Page);

	// Display the frame rate
	CPVRTString title;
	const char * pDesc;

	title = PVRTStringFromFormattedStr("Optimize Mesh %.1ffps", m_fFPS);

	// Print text on screen
	switch(m_i32Page)
	{
	default:
		pDesc = "Indexed Tri List: Unoptimized";
		break;
	case 1:
		pDesc = "Indexed Tri List: Optimized (at export time)";
		break;
	}

	m_Print3D.DisplayDefaultTitle(title.c_str(), pDesc, ePVRTPrint3DSDKLogo);

	// Flush all Print3D commands
	m_Print3D.Flush();

	return true;
}

/*******************************************************************************
 * Function Name  : DrawModel
 * Inputs		  : iOptim
 * Description    : Draws the balloon
 *******************************************************************************/
void OGLESOptimizeMesh::DrawModel( int iOptim )
{
	SPODMesh *pMesh;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	PVRTMATRIX worldMatrix;
	m_Model.GetWorldMatrix(worldMatrix, m_Model.pNode[0]);
	glMultMatrixf(worldMatrix.f);

	// Enable States
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	// Set Data Pointers and bing the VBOs
	switch(iOptim)
	{
	default:
		pMesh = m_Model.pMesh;

		glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[0]);
		break;
	case 1:
		pMesh = m_ModelOpt.pMesh;

		glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[1]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[1]);
		break;
	}

	// Load the meshes material properties
	SPODMaterial& Material = m_Model.pMaterial[m_Model.pNode[0].nIdxMaterial];

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,  PVRTVec4(Material.pfMatAmbient,  1.0f).ptr());
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  PVRTVec4(Material.pfMatDiffuse,  1.0f).ptr());

	// Used to display interleaved geometry
	glVertexPointer(3, GL_FLOAT, pMesh->sVertex.nStride, pMesh->sVertex.pData);
	glNormalPointer(GL_FLOAT, pMesh->sNormals.nStride, pMesh->sNormals.pData);

	// Draw
	glDrawElements(GL_TRIANGLES, pMesh->nNumFaces * 3, GL_UNSIGNED_SHORT, 0);

	// Disable States
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	// unbind the vertex buffers as we don't need them bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glPopMatrix();
}

/*******************************************************************************
 * Function Name  : NewDemo
 * Description    : Called by the Shell to initialize a new instance to the
 *					demo class.
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new OGLESOptimizeMesh();
}

/*****************************************************************************
 End of file (OGLESOptimizeMesh.cpp)
*****************************************************************************/

