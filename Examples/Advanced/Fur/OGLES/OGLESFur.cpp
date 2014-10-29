/******************************************************************************

 @File         OGLESFur.cpp

 @Title        Demonstrates a technique for giving the illusion of fur.

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Demonstrates a technique for giving the illusion of fur.

******************************************************************************/

/****************************************************************************
** Includes
****************************************************************************/
#include <math.h>

#include "PVRShell.h"	// Shell header
#include "OGLESTools.h"	// Tools header

/****************************************************************************
** Defines
****************************************************************************/

// Stuff for the information window
#define WINDOW_TITLE "FUR - Fur Simulation"

#if defined(_WIN32) && !defined(__BADA__)
	#define snprintf _snprintf
#endif

/****************************************************************************
** Structures
****************************************************************************/
struct SVertex
{
	float x, y, z;
	float nx, ny, nz;
	float tu, tv;
};

struct SMaterial
{
	const PVRTVec4	*pvDiffuse;
	const PVRTVec4	*pvAmbient;
	const PVRTVec4	*pvSpecular;
	float fShininess;
};

/****************************************************************************
** Consts
****************************************************************************/

// The various page descriptions
const char c_szWindowDesc1[] = "This is the duck model. The dark skin helps the deep fur appear to be in shadow.";

const char c_szWindowDesc2[] = "To simulate fur, translucent \"shells\" of the duck are rendered; the \
vertices of the shell are the original vertices displaced by some multiple of the vertex normal.\
\n\n\
Here one shell is rendered; the more shells, the better the illusion.";

const char c_szWindowDesc3[] = "Now two shells are rendered. Each dot in the alpha map of the fur shell \
represents where a hair passes through the layer.";

const char c_szWindowDesc4[] = "Seven fur shells seems to be sufficient to carry off the illusion.";

const char c_szWindowDesc5[] = "No doubt your landscape will look a little prettier!";

// The max number of fur shells
const unsigned int g_ui32MaxNoOfFurShells = 7;

// Camera properties
const float g_fFOV = 0.5890485f;
const float g_fNear= 50.0f;
const float g_fFar = 1500.0f;

static const PVRTVec3	c_vUp(0, 1.0f, 0);

// Fur parameters
static const float	c_fFurDepth		= 1.8f;

// Water and cloud plane equations
static const PVRTVec4 c_vPlaneWater(0, 1.0f, 0, 0);
static const PVRTVec4 c_vPlaneCloud(0, -1.0f, 0, 150.0f);

// Fog definition
static const PVRTVec4 c_vFogColour(0.729f, 0.796f, 0.863f, 0);
static float	c_fFogDensity	= 0.0013f;

// Light definition
static const PVRTVec4 c_vLightPosition(1.0f, 0.8f, -1.0f, 0.0f);
static const PVRTVec4 c_vLightColour (1.0f, 1.0f, 1.0f, 1.0f);
static const PVRTVec4 c_vLightAmbient(0.627f, 0.627f, 0.627f, 1.0f);

// Material colours
static const PVRTVec4 c_vColourWhite(1.0f, 1.0f, 1.0f, 1.0f);
static const PVRTVec4 c_vColourBlack(0.0f, 0.0f, 0.0f, 1.0f);
static const PVRTVec4 c_vColourGrey25(0.25f, 0.25f, 0.25f, 1.0f);
static const PVRTVec4 c_vColourBeak(0.93f, 0.55f, 0.15f, 1.0f);

// Materials
static const SMaterial c_pMaterial[4] = {
	{ &c_vColourWhite,	&c_vColourWhite,	&c_vColourGrey25,	10.0f },	/* 0 = Fur */
	{ &c_vColourBlack,	&c_vColourBlack,	&c_vColourWhite,	50.0f },	/* 1 = Eye */
	{ &c_vColourBeak,	&c_vColourBeak,		&c_vColourBeak,		20.0f },	/* 2 = Beak */
	{ &c_vColourWhite,	&c_vColourWhite,	&c_vColourBlack,	0.0f }		/* 3 = White */
};

/****************************************************************************
** Enums
****************************************************************************/

// The enum for each mesh
enum EMesh
{
	eLand,
	eDuckBody,
	eDuckBeak,
	eDuckEyeR,
	eDuckEyeL,
	eBridge,
	eMeshNo
};

// The enum for each texture
enum ETexture
{
	eTexSkin,
	eTexFur,
	eTexBridge,
	eTexCloud,
	eTexGrass,
	eTexWater,
	eTextureNo // The number of textures
};

/******************************************************************************
 Content file names
******************************************************************************/

// PVR texture files
const char c_szBridgeTexFile[]		= "tBridge.pvr";
const char c_szGrassTexFile[]		= "tGrass.pvr";
const char c_szWaterTexFile[]		= "tWater.pvr";
const char c_szSkinTexFile[]		= "tSkin.pvr";
const char c_szFurTexFile[]			= "tFur.pvr";
const char c_szCloudTexFile[]		= "tCloud.pvr";

// POD files
const char c_szSceneFile[] = "Scene.pod";

/****************************************************************************
** Classes
****************************************************************************/
class OGLESFur : public PVRShell
{
protected:
	PVRTMat4	m_mDuckWorld, m_mView, m_mProj;

	PVRTVec3	m_vCamFrom, m_vCamTo;

	// The cloud and water planes
	SVertex	m_pvPlaneWater[5];	// Procedurally generated water plane
	int	m_i32WaterPlaneNo;
	SVertex	m_pvPlaneCloud[5];	// Procedurally generated cloud plane
	int	m_i32CloudPlaneNo;

	bool m_bViewMode;
	bool m_bPause;

	unsigned long	m_ui32PrevTime;

	float			m_fDuckRot;
	float			m_fCameraRot;

	unsigned int	m_aTexIDs[eTextureNo];

	// Information Window
	CPVRTPrint3D	m_Print3D;			// Print3D class
	bool			m_bWndRender;		// show text?
	int				m_i32WndPage;			// which page?

	// 3D Model
	CPVRTModelPOD	m_Scene;

	// Vertex Buffer Object (VBO) handles
	GLuint*	m_puiVbo;
	GLuint*	m_puiIndexVbo;

	// Fur shells
	GLuint m_uiShellVbo[g_ui32MaxNoOfFurShells]; // vbos for the shells
	int	m_i32FurShellNo; // Number of fur shells to be rendered right now

public:
	OGLESFur() : m_bViewMode(false),
				 m_bPause(false),
				 m_fDuckRot(0.0f),
				 m_fCameraRot(1.0f),
				 m_bWndRender(true),
				 m_i32WndPage(0),
				 m_puiVbo(0),
				 m_puiIndexVbo(0),
				 m_i32FurShellNo(7)
	{
	}

	bool InitApplication();
	bool QuitApplication();
	bool InitView();
	bool ReleaseView();
	bool RenderScene();

protected:
	void DoAnimation();
	void SetMaterial(const SMaterial * const pMat, const int i32Tex);
	bool LoadTextures(CPVRTString* const pErrorStr);
	bool LoadVbos();
	void UpdateFurShells();
	void DrawFurShells();
	void DrawDuck();
	void DrawMesh(int i32NodeIndex);
	void DrawEnvironment();
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
bool OGLESFur::InitApplication()
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
bool OGLESFur::QuitApplication()
{
	// Frees the memory allocated for the scene
	m_Scene.Destroy();

	delete[] m_puiVbo;
	delete[] m_puiIndexVbo;
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
bool OGLESFur::InitView()
{
	// Setup the projection matrix
	glMatrixMode(GL_PROJECTION);

	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	m_mProj = PVRTMat4::PerspectiveFovRH(g_fFOV, (float)PVRShellGet(prefWidth) / (float)PVRShellGet(prefHeight), g_fNear, g_fFar, PVRTMat4::OGL, bRotate);
	glLoadMatrixf(m_mProj.f);

	// Set clear colour
	glClearColor(c_vFogColour.x, c_vFogColour.y, c_vFogColour.z, c_vFogColour.w);

	// Enable Smooth Color Shading
	glShadeModel(GL_SMOOTH);

	// Enable the depth buffer
	glEnable(GL_DEPTH_TEST);

	// Load fur data
	glGenBuffers(g_ui32MaxNoOfFurShells, m_uiShellVbo);
	UpdateFurShells();

	// Initialise 3D text
	if(m_Print3D.SetTextures(NULL, PVRShellGet(prefWidth), PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
		return false;

	// Load textures
	CPVRTString ErrorStr;

	if(!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Create VBOs
	if(!LoadVbos())
	{
		PVRShellSet(prefExitMessage, "Failed to create VBOs");
		return false;
	}

	// Initialise camera
	m_Scene.GetCameraPos(m_vCamFrom, m_vCamTo, 0);

	m_vCamFrom = PVRTVec3(0.0f, 400.0f, 0.0f);

	// Enable fog
	glFogf(GL_FOG_MODE, GL_EXP2);
	glFogf(GL_FOG_DENSITY, c_fFogDensity);
	glFogfv(GL_FOG_COLOR, &c_vFogColour.x);
	glEnable(GL_FOG);

	// Enable lighting
	glLightfv(GL_LIGHT0, GL_POSITION, &c_vLightPosition.x);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, &c_vLightColour.x);
	glLightfv(GL_LIGHT0, GL_AMBIENT, &c_vLightAmbient.x);
	glLightfv(GL_LIGHT0, GL_SPECULAR, &c_vLightColour.x);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	// Disable culling
	glDisable(GL_CULL_FACE);

	// Initialise time
	m_ui32PrevTime = PVRShellGetTime();
	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLESFur::ReleaseView()
{
	//	Free textures
	glDeleteTextures(eTextureNo, m_aTexIDs);

	// shutdown Print3D
	m_Print3D.ReleaseTextures();
	return true;
}

/*!****************************************************************************
 @Function		LoadTextures
 @Return		bool			true for success, false for failure
 @Description	Loads the textures.
******************************************************************************/
bool OGLESFur::LoadTextures(CPVRTString* const pErrorStr)
{
	// Load the textures.
	// For a detailed explanation see the Texturing training course.
	if(PVRTTextureLoadFromPVR(c_szBridgeTexFile, &m_aTexIDs[eTexBridge]) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Failed to load " + CPVRTString(c_szBridgeTexFile);
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szGrassTexFile, &m_aTexIDs[eTexGrass]) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Failed to load " + CPVRTString(c_szGrassTexFile);
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szWaterTexFile, &m_aTexIDs[eTexWater]) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Failed to load " + CPVRTString(c_szWaterTexFile);
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szSkinTexFile, &m_aTexIDs[eTexSkin]) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Failed to load " + CPVRTString(c_szSkinTexFile);
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szFurTexFile, &m_aTexIDs[eTexFur]) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Failed to load " + CPVRTString(c_szFurTexFile);
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szCloudTexFile, &m_aTexIDs[eTexCloud]) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Failed to load " + CPVRTString(c_szCloudTexFile);
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
bool OGLESFur::RenderScene()
{
	// Reset the states that print3D changes
	glDisable(GL_CULL_FACE);
	glEnable(GL_FOG);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);

	//	User input
	bool bNewPage = false;

	if(PVRShellIsKeyPressed(PVRShellKeyNameSELECT))
		m_bPause = !m_bPause;

	if(PVRShellIsKeyPressed(PVRShellKeyNameLEFT))
	{
		if(--m_i32WndPage < 0)
			m_i32WndPage = 5;

		bNewPage = true;
	}

	if(PVRShellIsKeyPressed(PVRShellKeyNameRIGHT))
	{
		if(++m_i32WndPage > 5)
			m_i32WndPage = 0;

		bNewPage = true;
	}

	if(bNewPage)
	{
		switch(m_i32WndPage)
		{
			case 0:
				m_bViewMode = false;
				m_i32FurShellNo = 7;
				break;
			case 1:
				m_bViewMode = true;
				m_i32FurShellNo = 0;
				break;
			case 2:
				m_bViewMode = true;
				m_i32FurShellNo = 1;
				break;
			case 3:
				m_bViewMode = true;
				m_i32FurShellNo = 2;
				break;
			case 4:
				m_bViewMode = true;
				m_i32FurShellNo = 7;
				break;
			case 5:
				m_bViewMode = false;
				m_i32FurShellNo = 7;
				break;
		}

		// Since the number of fur shells has changed, update them
		UpdateFurShells();
	}

	// Clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Animation
	DoAnimation();

	// View matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(m_mView.f);

	// Enable the vertex and normal arrays
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// Begin Scene
	if(!m_bViewMode)
		DrawEnvironment();

	// Draw the Duck
	DrawDuck();

	// Display Paused if the app is paused
	if(m_bPause)
		m_Print3D.Print3D(78.0f, 2.0f, 1.0f, PVRTRGBA(255,255,255,255), "Paused");

	// Disable the normals before our drawing of the print3D content
	glDisableClientState(GL_NORMAL_ARRAY);
	
	char szDesc[256];
	snprintf(szDesc, 256, "Displaying %d shells", m_i32FurShellNo);

	// Display the IMG logo
	m_Print3D.DisplayDefaultTitle("Fur", szDesc, ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();
	return true;
}

/*!****************************************************************************
 @Function		DrawFurShells
 @Description	Draw the duck shells that represent the fur
******************************************************************************/
void OGLESFur::DrawFurShells()
{
	// Get the mesh associated with the duck body
	int i32MeshID = m_Scene.pNode[eDuckBody].nIdx;
	SPODMesh* pMesh  = &m_Scene.pMesh[i32MeshID];

	// Bind the index buffer for the ducks body as the shells use the same indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[i32MeshID]);

	// Enable alpha blending. The Alpha-test is not required and would be slower than alpha-blend
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Set up the lighting
	glLightfv(GL_LIGHT0, GL_SPECULAR, &c_vColourBlack.x);

	// Enable the texture coordinates. The vertices and normals should already be enabled
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	SetMaterial(&c_pMaterial[0], m_aTexIDs[eTexFur]);

	for(int i = 0; i < m_i32FurShellNo; ++i)
	{
		// If we are drawing the most outer shell change the lighting
		if(i == m_i32FurShellNo - 1)
			glLightfv(GL_LIGHT0, GL_SPECULAR, &c_vLightColour.x);

		// Bind the VBO for the shells vertices
		glBindBuffer(GL_ARRAY_BUFFER, m_uiShellVbo[i]);

		// Setup the pointers
		glVertexPointer(3, GL_FLOAT, sizeof(SVertex), 0);
		glNormalPointer(GL_FLOAT, sizeof(SVertex), (GLvoid*) (sizeof(float) * 3));
		glTexCoordPointer(2, GL_FLOAT, sizeof(SVertex), (GLvoid*) (sizeof(float) * 6));

		// Draw primitive
		glDrawElements(GL_TRIANGLES, pMesh->nNumFaces * 3, GL_UNSIGNED_SHORT, 0);
	}

	// Unbind the VBOs
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Disable blending as it is no longer needed
	glDisable(GL_BLEND);
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			i32NodeIndex		Node index of the mesh to draw
 @Description	Draws a SPODMesh.
******************************************************************************/
void OGLESFur::DrawMesh(int i32NodeIndex)
{
	PVRTMat4 mWorld;

	glPushMatrix();
		// Setup the transformation for this mesh
		m_Scene.GetWorldMatrix(mWorld, m_Scene.pNode[i32NodeIndex]);
		glMultMatrixf(mWorld.f);

		// Get the mesh
		int ui32MeshID = m_Scene.pNode[i32NodeIndex].nIdx;
		SPODMesh* pMesh = &m_Scene.pMesh[ui32MeshID];

		// bind the VBO for the mesh
		glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[ui32MeshID]);

		// bind the index buffer, won't hurt if the handle is 0
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[ui32MeshID]);

		glVertexPointer(3, GL_FLOAT, pMesh->sVertex.nStride, pMesh->sVertex.pData);
		glNormalPointer(GL_FLOAT, pMesh->sNormals.nStride, pMesh->sNormals.pData);

		// Do we have texture co-ordinates
		if(pMesh->nNumUVW != 0)
		{
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, pMesh->psUVW[0].nStride, pMesh->psUVW[0].pData);
		}else
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		// Indexed Triangle list
		glDrawElements(GL_TRIANGLES, pMesh->nNumFaces * 3, GL_UNSIGNED_SHORT, 0);

		// unbind the vertex buffers as we don't need them bound anymore
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glPopMatrix();
}

/*!****************************************************************************
 @Function		DrawEnvironment
 @Description	Draws the environment, land, bridge etc.
******************************************************************************/
void OGLESFur::DrawEnvironment()
{
	PVRTMat4 mWorld;

	// Draw land
	SetMaterial(&c_pMaterial[3], m_aTexIDs[eTexGrass]);
	DrawMesh(eLand);

	// Draw bridge
	// Use the material from before but use a different texture
	SetMaterial(0, m_aTexIDs[eTexBridge]);
	DrawMesh(eBridge);

	// Draw Ground
	if(m_i32WaterPlaneNo)
	{
		glVertexPointer(3, GL_FLOAT, sizeof(SVertex), &m_pvPlaneWater->x);
		glNormalPointer(GL_FLOAT, sizeof(SVertex), &m_pvPlaneWater->nx);
		glTexCoordPointer(2, GL_FLOAT, sizeof(SVertex), &m_pvPlaneWater->tu);

		SetMaterial(0, m_aTexIDs[eTexWater]);

		// Draw primitive
		glDrawArrays(GL_TRIANGLE_FAN, 0, m_i32WaterPlaneNo);
	}

	// Draw Cloud
	if(m_i32CloudPlaneNo)
	{
		glVertexPointer(3, GL_FLOAT, sizeof(SVertex), &m_pvPlaneCloud->x);
		glNormalPointer(GL_FLOAT, sizeof(SVertex), &m_pvPlaneCloud->nx);
		glTexCoordPointer(2, GL_FLOAT, sizeof(SVertex), &m_pvPlaneCloud->tu);

		SetMaterial(0, m_aTexIDs[eTexCloud]);

		// Draw primitive
		glEnable(GL_BLEND);
		glDrawArrays(GL_TRIANGLE_FAN, 0, m_i32CloudPlaneNo);
		glDisable(GL_BLEND);
	}
}

/*!****************************************************************************
 @Function		DrawDuck
 @Description	Draws duck.
******************************************************************************/
void OGLESFur::DrawDuck()
{
	glPushMatrix();
		// Apply the transformation for the duck
		glMultMatrixf(m_mDuckWorld.f);

		// Draw the duck body
		SetMaterial(&c_pMaterial[3], m_aTexIDs[eTexSkin]);
		DrawMesh(eDuckBody);

		// Draw the duck's eyes
		SetMaterial(&c_pMaterial[1], 0);
		DrawMesh(eDuckEyeL);
		DrawMesh(eDuckEyeR);

		// Draw his beak
		SetMaterial(&c_pMaterial[2], 0);
		DrawMesh(eDuckBeak);

		// Draw the fur shells
		DrawFurShells();
	glPopMatrix();
}

/*!****************************************************************************
 @Function		UpdateFurShells
 @Description	Update the fur shells. This is only called when the number of
				shells change.
******************************************************************************/
void OGLESFur::UpdateFurShells()
{
	PVRTVec3	*pvSrcN, *pvSrcV;
	PVRTVec3	vTransNorm;
	PVRTVec4	vTransPos;
	SVertex		*pvData;
	int				i;
	unsigned int	j;
	float		fDepth, *pUV;

	int i32MeshIndex = m_Scene.pNode[eDuckBody].nIdx;
	SPODMesh* pMesh = &m_Scene.pMesh[i32MeshIndex];

	PVRTMat4 mModel;
	PVRTMat3 mModel3;

	m_Scene.GetWorldMatrix(mModel, m_Scene.pNode[eDuckBody]);
	mModel3 = PVRTMat3(mModel);

	pvData = new SVertex[pMesh->nNumVertex];

	if(!pvData)
		return;

	for(i = 0; i < m_i32FurShellNo; ++i)
	{
		fDepth = (c_fFurDepth * (float)(i+1) / (float)m_i32FurShellNo);

		for(j = 0; j < pMesh->nNumVertex; ++j)
		{
			pvSrcN	= (PVRTVec3*) (pMesh->pInterleaved + (size_t) pMesh->sNormals.pData + (j * pMesh->sNormals.nStride));
			pvSrcV	= (PVRTVec3*) (pMesh->pInterleaved + (size_t) pMesh->sVertex.pData  + (j * pMesh->sVertex.nStride));
			pUV		= (float*) (pMesh->pInterleaved + (size_t) pMesh->psUVW[0].pData + (j * pMesh->psUVW[0].nStride));

			// Transform the vertex position so it is in world space
			PVRTVec4 vPos4 = PVRTVec4(*pvSrcV, 1.0f);
			PVRTTransform(&vTransPos, &vPos4, &mModel);

			// Transform the vertex normal so it is in world space
			vTransNorm.x = mModel.f[0] * pvSrcN->x + mModel.f[4] * pvSrcN->y + mModel.f[8] * pvSrcN->z;
			vTransNorm.y = mModel.f[1] * pvSrcN->x + mModel.f[5] * pvSrcN->y + mModel.f[9] * pvSrcN->z;
			vTransNorm.z = mModel.f[2] * pvSrcN->x + mModel.f[6] * pvSrcN->y + mModel.f[10]* pvSrcN->z;
			vTransNorm.normalize();

			pvData[j].x = vTransPos.x + (vTransNorm.x * fDepth);
			pvData[j].y = vTransPos.y + (vTransNorm.y * fDepth);
			pvData[j].z = vTransPos.z + (vTransNorm.z * fDepth);

			pvData[j].nx = vTransNorm.x;
			pvData[j].ny = vTransNorm.y;
			pvData[j].nz = vTransNorm.z;

			pvData[j].tu = pUV[0];
			pvData[j].tv = pUV[1];
		}

		glBindBuffer(GL_ARRAY_BUFFER, m_uiShellVbo[i]);
		unsigned int uiSize = pMesh->nNumVertex * sizeof(SVertex);
		glBufferData(GL_ARRAY_BUFFER, uiSize, pvData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	delete[] pvData;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
bool OGLESFur::LoadVbos()
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

	return true;
}


/*!****************************************************************************
 @Function		DoAnimation
 @Description	Calculate the duck and camera animation as well as the cloud and
				water.
******************************************************************************/
void OGLESFur::DoAnimation()
{
	PVRTMat4		mCamera, mTmp;
	PVRTVec3		pvPlane[5];
	unsigned long	ui32Time;
	float			fDeltaTime;
	int i;

	if(!m_bPause)
	{
		ui32Time = PVRShellGetTime();

		fDeltaTime = (float) (ui32Time < m_ui32PrevTime ? m_ui32PrevTime - ui32Time : ui32Time - m_ui32PrevTime) + 0.1f;

		if(fDeltaTime > 50.0f) // Cap delta time
			fDeltaTime = 50.0f;

		m_ui32PrevTime = ui32Time;

		m_fCameraRot += 0.00006f * fDeltaTime;
		fDeltaTime = fDeltaTime * 0.001f;
	}
	else
		fDeltaTime = 0.0f;

	if(m_bViewMode)
	{
		// Viewing duck alone
		mCamera = PVRTMat4::Translation(0, 0, 160.0f);
		mTmp = PVRTMat4::RotationX(0.35f * (float) sin(0.0003f * m_ui32PrevTime) + 0.2f);
		mCamera = mTmp * mCamera;
		mTmp = PVRTMat4::RotationY(m_fCameraRot);
		mCamera = mTmp * mCamera;
		mTmp = PVRTMat4::Translation(m_mDuckWorld.f[12], m_mDuckWorld.f[13], m_mDuckWorld.f[14]);
		mCamera = mTmp * mCamera;

		m_vCamFrom.x += fDeltaTime * (mCamera.f[12] - m_vCamFrom.x);
		m_vCamFrom.y += fDeltaTime * (mCamera.f[13] - m_vCamFrom.y);
		m_vCamFrom.z += fDeltaTime * (mCamera.f[14] - m_vCamFrom.z);

		m_vCamTo.x	+= fDeltaTime * (m_mDuckWorld.f[12] - m_vCamTo.x);
		m_vCamTo.y	+= fDeltaTime * (m_mDuckWorld.f[13] + 25.0f - m_vCamTo.y);
		m_vCamTo.z	+= fDeltaTime * (m_mDuckWorld.f[14] - m_vCamTo.z);

		// Build view matrix
		m_mView = PVRTMat4::LookAtRH(m_vCamFrom, m_vCamTo, c_vUp);
	}
	else
	{
		//	Viewing duck in a wee river
		m_fDuckRot -= 0.1f * fDeltaTime;

		// Duck world transform
		m_mDuckWorld = PVRTMat4::Translation(140.0f, 0, 0);

		mTmp = PVRTMat4::RotationY(m_fDuckRot);
		m_mDuckWorld = mTmp * m_mDuckWorld;

		PVRTVec3	vFrom, vTo;

		// We can get the camera position, target with GetCameraPos()
		m_Scene.GetCameraPos(vFrom, vTo, 0);

		// Position camera
		mCamera = PVRTMat4::Translation(vFrom.x, vFrom.y, vFrom.z);

		mTmp = PVRTMat4::RotationY(m_fCameraRot);
		mCamera = mTmp * mCamera;

		m_vCamFrom.x += fDeltaTime * (mCamera.f[12] - m_vCamFrom.x);
		m_vCamFrom.y += fDeltaTime * (mCamera.f[13] - m_vCamFrom.y);
		m_vCamFrom.z += fDeltaTime * (mCamera.f[14] - m_vCamFrom.z);

		m_vCamTo.x	+= fDeltaTime * (2.0f * (m_mDuckWorld.f[12] - m_vCamTo.x));
		m_vCamTo.y	+= fDeltaTime * (2.0f * (m_mDuckWorld.f[13] + 25.0f - m_vCamTo.y));
		m_vCamTo.z	+= fDeltaTime * (2.0f * (m_mDuckWorld.f[14] - m_vCamTo.z));

		// Build view matrix
		m_mView = PVRTMat4::LookAtRH(m_vCamFrom, m_vCamTo, c_vUp);

		// Calc ViewProjInv matrix
		mTmp = m_mProj * m_mView;
		mTmp = mTmp.inverseEx();

		// Calculate the ground plane
		m_i32WaterPlaneNo = PVRTMiscCalculateInfinitePlane(&pvPlane->x, sizeof(*pvPlane), &c_vPlaneWater, &mTmp, &m_vCamFrom, g_fFar);

		for(i = 0; i < m_i32WaterPlaneNo; ++i)
		{
			m_pvPlaneWater[i].x		= pvPlane[i].x;
			m_pvPlaneWater[i].y		= pvPlane[i].y;
			m_pvPlaneWater[i].z		= pvPlane[i].z;
			m_pvPlaneWater[i].nx	= c_vPlaneWater.x;
			m_pvPlaneWater[i].ny	= c_vPlaneWater.y;
			m_pvPlaneWater[i].nz	= c_vPlaneWater.z;
			m_pvPlaneWater[i].tu	= pvPlane[i].x * 0.005f;
			m_pvPlaneWater[i].tv	= pvPlane[i].z * 0.005f;
		}

		// Calculate the Cloud plane
		m_i32CloudPlaneNo = PVRTMiscCalculateInfinitePlane(&pvPlane->x, sizeof(*pvPlane), &c_vPlaneCloud, &mTmp, &m_vCamFrom, g_fFar);

		for(i = 0; i < m_i32CloudPlaneNo; ++i)
		{
			m_pvPlaneCloud[i].x		= pvPlane[i].x;
			m_pvPlaneCloud[i].y		= pvPlane[i].y;
			m_pvPlaneCloud[i].z		= pvPlane[i].z;
			m_pvPlaneCloud[i].nx	= c_vPlaneCloud.x;
			m_pvPlaneCloud[i].ny	= c_vPlaneCloud.y;
			m_pvPlaneCloud[i].nz	= c_vPlaneCloud.z;
			m_pvPlaneCloud[i].tu	= pvPlane[i].x * ((1.0f / 100.0f) + m_ui32PrevTime * 0.0002f);
			m_pvPlaneCloud[i].tv	= pvPlane[i].z * (1.0f / 100.0f);
		}
	}
}

/*!****************************************************************************
 @Function		SetMaterial
 @Description	Set the material and the texture if they are not null.
******************************************************************************/
void OGLESFur::SetMaterial(const SMaterial * const pMat, const int i32Tex)
{
	if(pMat)
	{
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  &pMat->pvDiffuse->x);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,  &pMat->pvAmbient->x);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &pMat->pvSpecular->x);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, pMat->fShininess);
	}

	if(i32Tex)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, i32Tex);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
	}
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
	return new OGLESFur();
}


/*****************************************************************************
 End of file (OGLESFur.cpp)
*****************************************************************************/

