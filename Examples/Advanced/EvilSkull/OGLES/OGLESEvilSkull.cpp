/******************************************************************************

 @File         OGLESEvilSkull.cpp

 @Title        EvilSkull

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows Animation using Morphing between Key Frames in Software.
               Requires the PVRShell.

******************************************************************************/

/****************************************************************************
 ** INCLUDES                                                               **
 ****************************************************************************/

#include "PVRShell.h"

#include <math.h>
#include <string.h>

#include "OGLESTools.h"

/******************************************************************************
 Content file names
******************************************************************************/

// PVR texture files
const char c_szIrisTexFile[]	= "Iris.pvr";	// Eyes
const char c_szMetalTexFile[]	= "Metal.pvr";	// Skull
const char c_szFire02TexFile[]	= "Fire02.pvr";	// Background
const char c_szFire03TexFile[]	= "Fire03.pvr";	// Background

// POD file
const char c_szSceneFile[] = "EvilSkull.pod";

/****************************************************************************
 ** Enums                                                                **
 ****************************************************************************/
enum EMeshes
{
	eSkull,
	eJaw = 4
};

/****************************************************************************
 ** DEFINES                                                                **
 ****************************************************************************/

/****************************************************************************
 ** CONSTS                                                                **
 ****************************************************************************/

// Geometry Software Processing Defines
const unsigned int g_ui32NoOfMorphTargets = 4;

// Animation Define
const float g_fExprTime = 75.0f;

const unsigned int g_ui32NoOfTextures = 4;

/****************************************************************************
** Class: OGLESEvilSkull
****************************************************************************/
class OGLESEvilSkull : public PVRShell
{
    // Print 3D Class Object
	CPVRTPrint3D 	m_Print3D;

	// 3D Model
	CPVRTModelPOD	m_Scene;

	// m_LightPos
	PVRTVec4	m_LightPos;

	PVRTVec3 m_CameraPos, m_CameraTo, m_CameraUp;
	PVRTMat4	m_mView;

	// OpenGL handles for textures and VBOs
	GLuint*	m_puiVbo;
	GLuint*	m_puiIndexVbo;

	// Objects
	GLuint		m_ui32Texture[g_ui32NoOfTextures];

	// Software processing buffers
	float	*m_pMorphedVertices;
	float	*m_pAVGVertices;
	float	*m_pDiffVertices[g_ui32NoOfMorphTargets];

	// Animation Params
	float m_fSkullWeights[5];
	float m_fExprTable[4][7];
	float m_fJawRotation[7];
	float m_fBackRotation[7];
	int m_i32BaseAnim;
	int m_i32TargetAnim;

	// Generic
	int m_i32Frame;

public:
	OGLESEvilSkull() : m_puiVbo(0),
					   m_puiIndexVbo(0),
					   m_pMorphedVertices(0),
					   m_pAVGVertices(0),
					   m_i32BaseAnim(0),
					   m_i32TargetAnim(1),
					   m_i32Frame(0)
	{

		for(unsigned int i = 0; i < g_ui32NoOfMorphTargets; ++i)
			m_pDiffVertices[i] = 0;

		// Setup base constants in contructor

		// Camera and Light details
		m_LightPos  = PVRTVec4(-1.0f, 1.0f, 1.0f, 0.0f);

		m_CameraPos = PVRTVec3(0.0f, 0.0f, 300.0f);
		m_CameraTo  = PVRTVec3(0.0f, -30.0f, 0.0f);
		m_CameraUp  = PVRTVec3(0.0f, 1.0f, 0.0f);

		// Animation Table
		m_fSkullWeights[0] = 0.0f;
		m_fSkullWeights[1] = 1.0f;
		m_fSkullWeights[2] = 0.0f;
		m_fSkullWeights[3] = 0.0f;
		m_fSkullWeights[4] = 0.0f;

		m_fExprTable[0][0] = 1.0f;	m_fExprTable[1][0] = 1.0f;	m_fExprTable[2][0] = 1.0f;	m_fExprTable[3][0] = 1.0f;
		m_fExprTable[0][1] = 0.0f;	m_fExprTable[1][1] = 0.0f;	m_fExprTable[2][1] = 0.0f;	m_fExprTable[3][1] = 1.0f;
		m_fExprTable[0][2] = 0.0f;	m_fExprTable[1][2] = 0.0f;	m_fExprTable[2][2] = 1.0f;	m_fExprTable[3][2] = 1.0f;
		m_fExprTable[0][3] = 0.3f;	m_fExprTable[1][3] = 0.0f;	m_fExprTable[2][3] = 0.3f;	m_fExprTable[3][3] = 0.0f;
		m_fExprTable[0][4] =-1.0f;	m_fExprTable[1][4] = 0.0f;	m_fExprTable[2][4] = 0.0f;	m_fExprTable[3][4] = 0.0f;
		m_fExprTable[0][5] = 0.0f;	m_fExprTable[1][5] = 0.0f;	m_fExprTable[2][5] =-0.7f;	m_fExprTable[3][5] = 0.0f;
		m_fExprTable[0][6] = 0.0f;	m_fExprTable[1][6] = 0.0f;	m_fExprTable[2][6 ]= 0.0f;	m_fExprTable[3][6] =-0.7f;

		m_fJawRotation[0] = 45.0f;
		m_fJawRotation[1] = 25.0f;
		m_fJawRotation[2] = 40.0f;
		m_fJawRotation[3] = 20.0f;
		m_fJawRotation[4] = 45.0f;
		m_fJawRotation[5] = 25.0f;
		m_fJawRotation[6] = 30.0f;

		m_fBackRotation[0] = 0.0f;
		m_fBackRotation[1] = 25.0f;
		m_fBackRotation[2] = 40.0f;
		m_fBackRotation[3] = 90.0f;
		m_fBackRotation[4] = 125.0f;
		m_fBackRotation[5] = 80.0f;
		m_fBackRotation[6] = 30.0f;
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
	void RenderSkull();
	void RenderJaw();
	void CalculateMovement(int nType);
	void DrawQuad(float x, float y, float z, float Size, GLuint ui32Texture);
	void DrawDualTexQuad(float x, float y, float z, float Size, GLuint ui32Texture1, GLuint ui32Texture2);
	void LoadVbos();
	void CreateMorphData();
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
bool OGLESEvilSkull::InitApplication()
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

/*!****************************************************************************
 @Function		QuitApplication
 @Return		bool		true if no error occured
 @Description	Code in QuitApplication() will be called by PVRShell once per
				run, just before exiting the program.
				If the rendering context is lost, QuitApplication() will
				not be called.
******************************************************************************/
bool OGLESEvilSkull::QuitApplication()
{
	// Free the memory allocated for the scene
	m_Scene.Destroy();

	delete[] m_puiVbo;
	delete[] m_puiIndexVbo;

	delete[] m_pMorphedVertices;
	delete[] m_pAVGVertices;

	for(unsigned int i = 0; i < g_ui32NoOfMorphTargets; ++i)
		delete[] m_pDiffVertices[i];

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
bool OGLESEvilSkull::InitView()
{
	PVRTMat4		mPerspective;
	SPVRTContext	sContext;

	// Initialize Print3D textures
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	if(m_Print3D.SetTextures(&sContext, PVRShellGet(prefWidth), PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	/***********************
	** LOAD TEXTURES     **
	***********************/
	if(PVRTTextureLoadFromPVR(c_szIrisTexFile, &m_ui32Texture[0]) != PVR_SUCCESS)
		return false;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szMetalTexFile, &m_ui32Texture[1]) != PVR_SUCCESS)
		return false;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szFire02TexFile, &m_ui32Texture[2]) != PVR_SUCCESS)
		return false;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szFire03TexFile, &m_ui32Texture[3]) != PVR_SUCCESS)
		return false;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/******************************
	** GENERIC RENDER STATES     **
	*******************************/

	// The Type Of Depth Test To Do
	glDepthFunc(GL_LEQUAL);

	// Enables Depth Testing
	glEnable(GL_DEPTH_TEST);

	// Enables Smooth Color Shading
	glShadeModel(GL_SMOOTH);

	// Blending mode
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Create perspective matrix
	mPerspective = PVRTMat4::PerspectiveFovRH(70.0f*(3.14f/180.0f), (float)PVRShellGet(prefWidth) /(float)PVRShellGet(prefHeight), 10.0f, 10000.0f, PVRTMat4::OGL, bRotate);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(mPerspective.f);

	// Create viewing matrix
	m_mView = PVRTMat4::LookAtRH(m_CameraPos, m_CameraTo, m_CameraUp);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(m_mView.f);

	// Enable texturing
	glEnable(GL_TEXTURE_2D);

	// Lights (only one side lighting)
	glEnable(GL_LIGHTING);

	// Light 0 (White directional light)
	PVRTVec4 fAmbient  = PVRTVec4(0.2f, 0.2f, 0.2f, 1.0f);
	PVRTVec4 fDiffuse  = PVRTVec4(1.0f, 1.0f, 1.0f, 1.0f);
	PVRTVec4 fSpecular = PVRTVec4(1.0f, 1.0f, 1.0f, 1.0f);

	glLightfv(GL_LIGHT0, GL_AMBIENT,  fAmbient.ptr());
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  fDiffuse.ptr());
	glLightfv(GL_LIGHT0, GL_SPECULAR, fSpecular.ptr());
	glLightfv(GL_LIGHT0, GL_POSITION, m_LightPos.ptr());

	glEnable(GL_LIGHT0);

	glDisable(GL_LIGHTING);

	// Create the data used for the morphing
	CreateMorphData();

	// Sets the clear color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Create vertex buffer objects
	LoadVbos();

	return true;
}

/*!****************************************************************************
 @Function		CreateMorphData
 @Description	Creates the data used for the morphing
******************************************************************************/
void OGLESEvilSkull::CreateMorphData()
{
	unsigned int i,j;

	unsigned int ui32VertexNo = m_Scene.pMesh[eSkull].nNumVertex;

	delete[] m_pMorphedVertices;
	delete[] m_pAVGVertices;

	m_pMorphedVertices = new float[ui32VertexNo * 3];
	m_pAVGVertices     = new float[ui32VertexNo * 3];

	for(i = 0; i < g_ui32NoOfMorphTargets; ++i)
	{
		delete[] m_pDiffVertices[i];
		m_pDiffVertices[i] = new float[ui32VertexNo * 3];
		memset(m_pDiffVertices[i], 0, sizeof(float) * ui32VertexNo * 3);
	}

	unsigned char* pData[g_ui32NoOfMorphTargets];

	for(j = 0; j < g_ui32NoOfMorphTargets; ++j)
		pData[j] = m_Scene.pMesh[eSkull + j].pInterleaved;

	float *pVertexData;

	// Calculate AVG Model for Morphing
	for(i = 0; i < ui32VertexNo * 3; i += 3)
	{
		m_pAVGVertices[i + 0] = 0.0f;
		m_pAVGVertices[i + 1] = 0.0f;
		m_pAVGVertices[i + 2] = 0.0f;

		for(j = 0; j < g_ui32NoOfMorphTargets; ++j)
		{
			pVertexData = (float*) pData[j];

			m_pAVGVertices[i + 0] += pVertexData[0] * 0.25f;
			m_pAVGVertices[i + 1] += pVertexData[1] * 0.25f;
			m_pAVGVertices[i + 2] += pVertexData[2] * 0.25f;

			pData[j] += m_Scene.pMesh[eSkull + j].sVertex.nStride;
		}
	}

	for(j = 0; j < g_ui32NoOfMorphTargets; ++j)
		pData[j] = m_Scene.pMesh[eSkull + j].pInterleaved;

	// Calculate Differences for Morphing
	for(i = 0; i < ui32VertexNo * 3; i += 3)
	{
		for(j = 0; j < g_ui32NoOfMorphTargets; ++j)
		{
			pVertexData = (float*) pData[j];

			m_pDiffVertices[j][i + 0] = m_pAVGVertices[i + 0] - pVertexData[0];
			m_pDiffVertices[j][i + 1] = m_pAVGVertices[i + 1] - pVertexData[1];
			m_pDiffVertices[j][i + 2] = m_pAVGVertices[i + 2] - pVertexData[2];

			pData[j] += m_Scene.pMesh[eSkull + j].sVertex.nStride;
		}
	}
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLESEvilSkull::LoadVbos()
{
	if(!m_puiVbo)
		m_puiVbo = new GLuint[2];

	if(!m_puiIndexVbo)
		m_puiIndexVbo = new GLuint[2];

	glGenBuffers(2, m_puiVbo);
	glGenBuffers(2, m_puiIndexVbo);

	// Create vertex buffer for Skull

	// Load vertex data into buffer object
	unsigned int uiSize = m_Scene.pMesh[eSkull].nNumVertex * m_Scene.pMesh[eSkull].sVertex.nStride;

	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[0]);
	glBufferData(GL_ARRAY_BUFFER, uiSize, m_Scene.pMesh[eSkull].pInterleaved, GL_STATIC_DRAW);

	// Load index data into buffer object if available
	uiSize = PVRTModelPODCountIndices(m_Scene.pMesh[eSkull]) * sizeof(GLshort);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, m_Scene.pMesh[eSkull].sFaces.pData, GL_STATIC_DRAW);

	// Create vertex buffer for Jaw

	// Load vertex data into buffer object
	uiSize = m_Scene.pMesh[eJaw].nNumVertex * m_Scene.pMesh[eJaw].sVertex.nStride;

	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[1]);
	glBufferData(GL_ARRAY_BUFFER, uiSize, m_Scene.pMesh[eJaw].pInterleaved, GL_STATIC_DRAW);

	// Load index data into buffer object if available
	uiSize = PVRTModelPODCountIndices(m_Scene.pMesh[eJaw]) * sizeof(GLshort);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, m_Scene.pMesh[eJaw].sFaces.pData, GL_STATIC_DRAW);

	// Unbind buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLESEvilSkull::ReleaseView()
{
	// release all textures
	glDeleteTextures(g_ui32NoOfTextures, m_ui32Texture);

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
bool OGLESEvilSkull::RenderScene()
{
	unsigned int i;
	float fCurrentfJawRotation, fCurrentfBackRotation;
	float fFactor, fInvFactor;

	// Update Skull Weights and Rotations using Animation Info
	if(m_i32Frame > g_fExprTime)
	{
		m_i32Frame = 0;
		m_i32BaseAnim = m_i32TargetAnim;

		++m_i32TargetAnim;

		if(m_i32TargetAnim > 6)
		{
			m_i32TargetAnim = 0;
		}
	}

	fFactor = float(m_i32Frame) / g_fExprTime;
	fInvFactor = 1.0f - fFactor;

	m_fSkullWeights[0] = (m_fExprTable[0][m_i32BaseAnim] * fInvFactor) + (m_fExprTable[0][m_i32TargetAnim] * fFactor);
	m_fSkullWeights[1] = (m_fExprTable[1][m_i32BaseAnim] * fInvFactor) + (m_fExprTable[1][m_i32TargetAnim] * fFactor);
	m_fSkullWeights[2] = (m_fExprTable[2][m_i32BaseAnim] * fInvFactor) + (m_fExprTable[2][m_i32TargetAnim] * fFactor);
	m_fSkullWeights[3] = (m_fExprTable[3][m_i32BaseAnim] * fInvFactor) + (m_fExprTable[3][m_i32TargetAnim] * fFactor);

	fCurrentfJawRotation = m_fJawRotation[m_i32BaseAnim] * fInvFactor + (m_fJawRotation[m_i32TargetAnim] * fFactor);
	fCurrentfBackRotation = m_fBackRotation[m_i32BaseAnim] * fInvFactor + (m_fBackRotation[m_i32TargetAnim] * fFactor);

	// Update Base Animation Value - FrameBased Animation for now
	++m_i32Frame;

	// Update Skull Vertex Data using Animation Params
	for(i = 0; i < m_Scene.pMesh[eSkull].nNumVertex * 3; ++i)
	{
		m_pMorphedVertices[i]= m_pAVGVertices[i] + (m_pDiffVertices[0][i] * m_fSkullWeights[0]) \
												 + (m_pDiffVertices[1][i] * m_fSkullWeights[1]) \
												 + (m_pDiffVertices[2][i] * m_fSkullWeights[2]) \
												 + (m_pDiffVertices[3][i] * m_fSkullWeights[3]);

	}

	// Buffer Clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Render Skull and Jaw Opaque with Lighting
	glDisable(GL_BLEND);		// Opaque = No Blending
	glEnable(GL_LIGHTING);		// Lighting On

	// Set skull and jaw texture
	glBindTexture(GL_TEXTURE_2D, m_ui32Texture[1]);

	// Enable and set vertices, normals and index data
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// Render Animated Jaw - Rotation Only
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glLoadIdentity();

	glMultMatrixf(m_mView.f);

	glTranslatef(0,-50.0f,-50.0f);

	glRotatef(-fCurrentfJawRotation, 1.0f, 0.0f, 0.0f);
	glRotatef(fCurrentfJawRotation - 30.0f, 0, 1.0f, -1.0f);

	RenderJaw();

	glPopMatrix();

	// Render Morphed Skull
	glPushMatrix();

	glRotatef(fCurrentfJawRotation - 30.0f, 0, 1.0f, -1.0f);

	RenderSkull();

	// Render Eyes and Background with Alpha Blending and No Lighting

	glEnable(GL_BLEND);			// Enable Alpha Blending
	glDisable(GL_LIGHTING);		// Disable Lighting


	// Disable the normals as they aren't needed anymore
	glDisableClientState(GL_NORMAL_ARRAY);

	// Render Eyes using Skull Model Matrix
	DrawQuad(-30.0f ,0.0f ,50.0f ,20.0f , m_ui32Texture[0]);
	DrawQuad( 33.0f ,0.0f ,50.0f ,20.0f , m_ui32Texture[0]);

	glPopMatrix();

	// Render Dual Texture Background with different base color, rotation, and texture rotation
	glPushMatrix();

	glDisable(GL_BLEND);			// Disable Alpha Blending

	glColor4f(0.7f+0.3f*((m_fSkullWeights[0])), 0.7f, 0.7f, 1.0f);	// Animated Base Color
	glTranslatef(10.0f, -50.0f, 0.0f);
	glRotatef(fCurrentfBackRotation*4.0f,0,0,-1.0f);	// Rotation of Quad

	// Animated Texture Matrix
	glActiveTexture(GL_TEXTURE0);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glTranslatef(-0.5f, -0.5f, 0.0f);
	glRotatef(fCurrentfBackRotation*-8.0f, 0, 0, -1.0f);
	glTranslatef(-0.5f, -0.5f, 0.0f);

	// Draw Geometry
	DrawDualTexQuad(0.0f ,0.0f ,-100.0f ,300.0f, m_ui32Texture[3], m_ui32Texture[2]);

	// Disable Animated Texture Matrix
	glActiveTexture(GL_TEXTURE0);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	// Make sure to disable the arrays
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	// Reset Colour
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// Display info text
	m_Print3D.DisplayDefaultTitle("EvilSkull", "Morphing", ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}

/*******************************************************************************
 * Function Name  : RenderSkull
 * Input		  : Texture Pntr and Filter Mode
 * Description    : Renders the Skull data using the Morphed Data Set.
 *******************************************************************************/
void OGLESEvilSkull::RenderSkull()
{
	SPODMesh& Mesh = m_Scene.pMesh[eSkull];

	glVertexPointer(3, GL_FLOAT,  sizeof(float) * 3, m_pMorphedVertices);

	// Bind the jaw vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[0]);

	// Setup pointers
	glNormalPointer(GL_FLOAT, Mesh.sNormals.nStride, Mesh.sNormals.pData);
	glTexCoordPointer(2, GL_FLOAT, Mesh.psUVW[0].nStride, Mesh.psUVW[0].pData);

	glDrawElements(GL_TRIANGLES, Mesh.nNumFaces * 3, GL_UNSIGNED_SHORT, 0);

	// unbind the vertex buffers as we don't need them bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/*******************************************************************************
 * Function Name  : RenderJaw
 * Input		  : Texture Pntr and Filter Mode
 * Description    : Renders the Skull Jaw - uses direct data no morphing
 *******************************************************************************/
void OGLESEvilSkull::RenderJaw()
{
	SPODMesh& Mesh = m_Scene.pMesh[eJaw];

	// Bind the jaw vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[1]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[1]);

	// Setup pointers
	glVertexPointer(3, GL_FLOAT, Mesh.sVertex.nStride, Mesh.sVertex.pData);
	glNormalPointer(GL_FLOAT, Mesh.sNormals.nStride, Mesh.sNormals.pData);
	glTexCoordPointer(2, GL_FLOAT, Mesh.psUVW[0].nStride, Mesh.psUVW[0].pData);

	glDrawElements(GL_TRIANGLES, Mesh.nNumFaces * 3, GL_UNSIGNED_SHORT, 0);

	// unbind the vertex buffers as we don't need them bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/*******************************************************************************
 * Function Name  : DrawQuad
 * Input		  : Size, (x,y,z) and texture pntr
 * Description    : Basic Draw Quad with Size in Location X, Y, Z.
 *******************************************************************************/
void OGLESEvilSkull::DrawQuad(float x, float y, float z, float Size, GLuint ui32Texture)
{
	// Bind correct texture
	glBindTexture(GL_TEXTURE_2D, ui32Texture);

	// Vertex Data
	float verts[] =		{	x+Size, y-Size, z,
							x+Size, y+Size, z,
							x-Size, y-Size, z,
							x-Size, y+Size, z
						};

	float texcoords[] =	{	0.0f, 1.0f,
							0.0f, 0.0f,
							1.0f, 1.0f,
							1.0f, 0.0f
						};

	// Set Arrays - Only need Vertex Array and Tex Coord Array
	glVertexPointer(3,GL_FLOAT,0,verts);
	glTexCoordPointer(2,GL_FLOAT,0,texcoords);

	// Draw Strip
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
}

/*******************************************************************************
 * Function Name  : DrawDualTexQuad
 * Input		  : Size, (x,y,z) and texture pntr
 * Description    : Basic Draw Dual Textured Quad with Size in Location X, Y, Z.
 *******************************************************************************/
void OGLESEvilSkull::DrawDualTexQuad(float x, float y, float z, float Size, GLuint ui32Texture1, GLuint ui32Texture2)
{
	// Set Texture and Texture Options
	glBindTexture(GL_TEXTURE_2D, ui32Texture1);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ui32Texture2);
	glEnable(GL_TEXTURE_2D);

	// Vertex Data
	float verts[] =		{	x+Size, y-Size, z,
							x+Size, y+Size, z,
							x-Size, y-Size, z,
							x-Size, y+Size, z
						};

	float texcoords[] =	{	0.0f, 1.0f,
							0.0f, 0.0f,
							1.0f, 1.0f,
							1.0f, 0.0f
						};

	// Set Arrays - Only need Vertex Array and Tex Coord Arrays
	glVertexPointer(3,GL_FLOAT,0,verts);

    glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2,GL_FLOAT,0,texcoords);

	glClientActiveTexture(GL_TEXTURE1);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2,GL_FLOAT,0,texcoords);

	// Draw Strip
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);

	// Disable Arrays
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glClientActiveTexture(GL_TEXTURE0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);
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
	return new OGLESEvilSkull();
}

/*****************************************************************************
 End of file (OGLESEvilSkull.cpp)
*****************************************************************************/

