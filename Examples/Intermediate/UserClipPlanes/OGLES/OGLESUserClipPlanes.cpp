/******************************************************************************

 @File         OGLESUserClipPlanes.cpp

 @Title        User Defined Clip Planes

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows how to use multiple user defined clip planes using a PowerVR
               OGLES extension.

******************************************************************************/

/*************************
		Includes
*************************/
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "PVRShell.h"
#include "OGLESTools.h"

/******************************************************************************
 Content file names
******************************************************************************/

// PVR texture files
const char c_szGraniteTexFile[]	= "Granite.pvr";		// Special Texture to generate Toon Shading

// POD File
const char c_szSceneFile[]	= "Mesh.pod";

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLESUserClipPlanes : public PVRShell
{
	CPVRTPrint3D 	m_Print3D;
	CPVRTModelPOD	m_Scene;

	PVRTVec4		m_vLightPos;
	GLuint			m_ui32TexID;
	long			m_i32Frame;
	int				m_i32ClipPlaneNo;

	// Vertex Buffer Object (VBO) handles
	GLuint*	m_puiVbo;
	GLuint*	m_puiIndexVbo;

public:
	OGLESUserClipPlanes() : m_i32Frame(0L),
							m_puiVbo(0),
							m_puiIndexVbo(0)
	{
		m_vLightPos = PVRTVec4(-1.0f, 1.0f, 1.0f, 0.0f);
	}

	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	void DrawSphere();
	void SetupUserClipPlanes();
	void DisableClipPlanes();
	void LoadVbos();
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
bool OGLESUserClipPlanes::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	// Load the meshes
	if(m_Scene.ReadFromFile(c_szSceneFile) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load Mesh_*.pod!");
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
bool OGLESUserClipPlanes::QuitApplication()
{
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
bool OGLESUserClipPlanes::InitView()
{
	PVRTMat4	mPerspective;

	// Retrieve max number of clip planes
	glGetIntegerv(GL_MAX_CLIP_PLANES, &m_i32ClipPlaneNo);

	SPVRTContext Context;

	// Setup textures
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	if(m_Print3D.SetTextures(&Context, PVRShellGet(prefWidth), PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D.\n");
		return false;
	}

	// Load texture
	if(PVRTTextureLoadFromPVR(c_szGraniteTexFile, &m_ui32TexID) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load granite texture.\n");
		return false;
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Perspective matrix
	glMatrixMode(GL_PROJECTION);

	mPerspective = PVRTMat4::PerspectiveFovRH(20.0f*(PVRT_PIf/180.0f), (float) PVRShellGet(prefWidth)/(float) PVRShellGet(prefHeight), 10.0f, 1200.0f, PVRTMat4::OGL, bRotate);
	glLoadMatrixf(mPerspective.f);

	// Modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Setup culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	// Setup single light
	glEnable(GL_LIGHTING);

	PVRTVec4 fAmbient, fDiffuse, fSpecular;

	// Light 0 (White directional light)
	fAmbient  = PVRTVec4(0.4f, 0.4f, 0.4f, 1.0f);
	fDiffuse  = PVRTVec4(1.0f, 1.0f, 1.0f, 1.0f);
	fSpecular = PVRTVec4(1.0f, 1.0f, 1.0f, 1.0f);

	glLightfv(GL_LIGHT0, GL_AMBIENT,  fAmbient.ptr());
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  fDiffuse.ptr());
	glLightfv(GL_LIGHT0, GL_SPECULAR, fSpecular.ptr());
	glLightfv(GL_LIGHT0, GL_POSITION, m_vLightPos.ptr());

	glEnable(GL_LIGHT0);

	PVRTVec4 ambient_light = PVRTVec4(0.8f, 0.8f, 0.8f, 1.0f);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_light.ptr());

	// Setup all materials
	fAmbient  = PVRTVec4(0.1f, 0.1f, 0.1f, 1.0f);
	fDiffuse  = PVRTVec4(0.5f, 0.5f, 0.5f, 1.0f);
	fSpecular = PVRTVec4(1.0f, 1.0f, 1.0f, 1.0f);

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   fAmbient.ptr());
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   fDiffuse.ptr());
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  fSpecular.ptr());
	glMaterialf(GL_FRONT_AND_BACK,  GL_SHININESS, 10.0f);	// Nice and shiny so we don't get aliasing from the 1/2 angle

	// Set states
	glEnable(GL_DEPTH_TEST);

	// Set clear colour
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	LoadVbos();
	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLESUserClipPlanes::LoadVbos()
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
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLESUserClipPlanes::ReleaseView()
{
	// Release textures
	glDeleteTextures(1, &m_ui32TexID);
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
bool OGLESUserClipPlanes::RenderScene()
{
	// Clear the buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// Texturing
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_ui32TexID);
	glActiveTexture(GL_TEXTURE0);

	glDisable(GL_BLEND);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Transformations
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -500.0f);
	glRotatef((float)m_i32Frame/5.0f,0,1,0);

	// Draw sphere with user clip planes
	SetupUserClipPlanes();
	glDisable(GL_CULL_FACE);

	DrawSphere();

	glDisable(GL_TEXTURE_2D);
	DisableClipPlanes();

	// Display info text
	m_Print3D.DisplayDefaultTitle("UserClipPlanes", "User defined clip planes", ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	// Increase frame number
	++m_i32Frame;

	return true;
}

/*!****************************************************************************
 @Function		DrawSphere
 @Description	Draw the rotating sphere
******************************************************************************/
void OGLESUserClipPlanes::DrawSphere()
{
	// Bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[0]);

	// Bind the index buffer, won't hurt if the handle is 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[0]);

	// Enable States
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// Set Data Pointers
	SPODMesh* pMesh = &m_Scene.pMesh[0];

	// Used to display non interleaved geometry
	glVertexPointer(3, GL_FLOAT, pMesh->sVertex.nStride, pMesh->sVertex.pData);
	glNormalPointer(GL_FLOAT, pMesh->sNormals.nStride, pMesh->sNormals.pData);
	glTexCoordPointer(2, GL_FLOAT, pMesh->psUVW->nStride, pMesh->psUVW[0].pData);

	// Indexed Triangle list
	glDrawElements(GL_TRIANGLES, pMesh->nNumFaces * 3, GL_UNSIGNED_SHORT, 0);

	// Disable States
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	// Unbind the vertex buffers as we don't need them bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/*!****************************************************************************
 @Function		SetupUserClipPlanes
 @Description	Setup the user clip planes
******************************************************************************/
void OGLESUserClipPlanes::SetupUserClipPlanes()
{
	float ofs = (float)sin(-m_i32Frame / 50.0f) * 10;

	if (m_i32ClipPlaneNo < 1)
		return;

	float equation0[] = {1, 0, -1, 65+ofs};
	glClipPlanef(GL_CLIP_PLANE0, equation0);
	glEnable(GL_CLIP_PLANE0);

	if (m_i32ClipPlaneNo < 2)
		return;

	float equation1[] = {-1, 0, -1, 65+ofs};
	glClipPlanef( GL_CLIP_PLANE1, equation1);
	glEnable(GL_CLIP_PLANE1);

	if (m_i32ClipPlaneNo < 3)
		return;

	float equation2[] = {-1, 0, 1, 65+ofs};
	glClipPlanef( GL_CLIP_PLANE2, equation2);
	glEnable(GL_CLIP_PLANE2);

	if (m_i32ClipPlaneNo < 4)
		return;

	float equation3[] = {1, 0, 1, 65+ofs};
	glClipPlanef( GL_CLIP_PLANE3, equation3);
	glEnable(GL_CLIP_PLANE3);

	if (m_i32ClipPlaneNo < 5)
		return;

	float equation4[] = {0, 1, 0, 40+ofs};
	glClipPlanef(GL_CLIP_PLANE4, equation4);
	glEnable(GL_CLIP_PLANE4);

	if (m_i32ClipPlaneNo < 6)
		return;

	float equation5[] = {0, -1, 0, 40+ofs};
	glClipPlanef(GL_CLIP_PLANE5, equation5);
	glEnable( GL_CLIP_PLANE5 );
}

/*!****************************************************************************
 @Function		DisableClipPlanes
 @Description	Disable all the user clip planes
******************************************************************************/
void OGLESUserClipPlanes::DisableClipPlanes()
{
	for(int i=0;i<m_i32ClipPlaneNo;++i) {
		glDisable(GL_CLIP_PLANE0 + i);
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
	return new OGLESUserClipPlanes();
}

/*****************************************************************************
 End of file (OGLESUserClipPlanes.cpp)
*****************************************************************************/

