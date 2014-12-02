/******************************************************************************

 @File         OGLESStencilBuffer.cpp

 @Title        StencilBuffer

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Demonstrates how to use the stencil buffer for modifier volumes

******************************************************************************/
#include "PVRShell.h"
#include "OGLESTools.h"


/******************************************************************************
 Content file names
******************************************************************************/

// PVR texture files
const char c_szCylinderTexFile[] = "Lattice.pvr";
const char c_szStoneTexFile[]    = "Stone.pvr";
const char c_szTileTexFile[]     = "Tile.pvr";

// POD scene files
const char c_szCylinderFile[]    = "Cylinder.pod";
const char c_szSphereFile[]      = "Sphere.pod";

/******************************************************************************
 consts
******************************************************************************/
// Description message
const char g_DescriptionNoSupport[] = "Error: We have no stencil buffer";
const char g_DescriptionSupport[] = "";

/*!****************************************************************************
 Class encapsulating model data and methods
******************************************************************************/
class CModel
{
protected:
	CPVRTModelPOD m_Scene;
	GLuint* m_puiVbo;
	GLuint*	m_puiIndexVbo;

public:
	CModel();
	~CModel();

	bool ReadFromFile(const char* pszFilename);
	void LoadVbos();
	void DeleteVbos();
	void DrawMesh(int i32NodeIndex);
};

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLESStencilBuffer : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// 3D Models
	CModel m_Cylinder, m_Sphere;

	// OpenGL handles for textures
	GLuint	m_uiCylinderTex;
	GLuint	m_uiStoneTex;
	GLuint	m_uiTileTex;

	// The angle of rotation
	float m_fAngle;

	const char * m_pDescription;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	bool LoadTextures(CPVRTString* pErrorStr);
};

/*!****************************************************************************
 @Function		LoadTextures
 @Output		pErrorStr		A CPVRTString describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course
******************************************************************************/
bool OGLESStencilBuffer::LoadTextures(CPVRTString* const pErrorStr)
{
	if(PVRTTextureLoadFromPVR(c_szCylinderTexFile, &m_uiCylinderTex) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Failed to load texture.";
		return false;
	}

	if(PVRTTextureLoadFromPVR(c_szStoneTexFile, &m_uiStoneTex) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Failed to load texture.";
		return false;
	}

	if(PVRTTextureLoadFromPVR(c_szTileTexFile, &m_uiTileTex) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Failed to load texture.";
		return false;
	}

	return true;
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
bool OGLESStencilBuffer::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	// Load the scene files
	if (!m_Cylinder.ReadFromFile(c_szCylinderFile) ||
		!m_Sphere.ReadFromFile(c_szSphereFile))
	{
		PVRShellSet(prefExitMessage, "ERROR: Couldn't load the .pod file\n");
		return false;
	}

	// Initialise the angle variable
	m_fAngle = 0.0f;

	// Request Stencil Buffer support
	PVRShellSet(prefStencilBufferContext, true);
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
bool OGLESStencilBuffer::QuitApplication()
{
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
bool OGLESStencilBuffer::InitView()
{
	// Set the description depending of if we managed to get our stencil buffer?
	m_pDescription = PVRShellGet(prefStencilBufferContext) ? g_DescriptionSupport : g_DescriptionNoSupport;

	CPVRTString ErrorStr;

	// Initialise VBO data
	m_Cylinder.LoadVbos();
	m_Sphere.LoadVbos();

	// Load textures
	if(!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Is the screen rotated?
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	// Initialise Print3D
	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D");
		return false;
	}

	// Set OpenGL ES render states needed for this training course

	// Enable texturing
	glEnable(GL_TEXTURE_2D);

	// Use a nice bright blue as clear colour
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	// Set stencil clear value
	glClearStencil(0);

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLESStencilBuffer::ReleaseView()
{
	// Delete textures
	glDeleteTextures(1, &m_uiCylinderTex);
	glDeleteTextures(1, &m_uiStoneTex);
	glDeleteTextures(1, &m_uiTileTex);

	// Delete buffer objects
	m_Cylinder.DeleteVbos();
	m_Sphere.DeleteVbos();

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
bool OGLESStencilBuffer::RenderScene()
{
	// Enable backface culling and depth test
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	m_fAngle += 0.005f;

	// Clear the color, depth and stencil buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Set up the transformation matrices for our two shapes (the cylinder and the sphere)
	PVRTMat4 mSphere, mCylinder;
	PVRTMat4 mScaling;
	mScaling = PVRTMat4::Scale((float)PVRShellGet(prefHeight)/(float)PVRShellGet(prefWidth), 1.0f, 1.0f);
	mSphere = mScaling * PVRTMat4::RotationX(m_fAngle);
	mCylinder = mScaling * PVRTMat4::RotationX(m_fAngle) * PVRTMat4::RotationZ(m_fAngle) * PVRTMat4::Translation(-0.4f, -0.5f, 0.0f);

	// Bind texture and set transform
	glBindTexture(GL_TEXTURE_2D, m_uiStoneTex);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&mSphere.f[0]);

	// Set culling to cull the front faces
	glCullFace(GL_FRONT);

	/*
		Draw the sphere

		This sphere is textured with the stone texture and will be visible outside the stencil volume as
		we are drawing a second sphere with a green tiles texture everywhere within the stencil
		geometry.

		Also this sphere is used to set the depth values in the Z-Buffer.
	*/
	m_Sphere.DrawMesh(0);


	/*
		Enable the stencil test, disable colour and depth writes
	*/
	glEnable(GL_STENCIL_TEST);
	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
	glDepthMask(GL_FALSE);

	/*
		What we are going to do is draw a volume (a cylinder) so that all front faces that are in front of
		the already rendered geometry (the sphere) increase the stencil value by one, while all back faces
		that are in front of the rendered geometry decrease the stencil value. This way only surfaces that
		intersect the stencil volume will get a stencil value != 0.
	*/

	/*
		glStencilFunc tells OGLES the type of per-pixel test that we want to do. In the case below GL_ALWAYS says we
		want the test to always pass. The third value is the mask value which is ANDed with the second value
		(the reference value) to create the value that is put in the stencil buffer.

		Alternative values for the first value are

		GL_NEVER which causes the test to never pass.
		GL_LESS	    Passes if (	ref & mask )  < ( stencil & mask )
		GL_LEQUAL	Passes if (	ref & mask ) <= ( stencil & mask )
		GL_GREATER	Passes if (	ref & mask )  > ( stencil & mask )
		GL_GEQUAL	Passes if (	ref & mask ) >= ( stencil & mask )
		GL_EQUAL	Passes if (	ref & mask )  = ( stencil & mask )
		GL_NOTEQUAL	Passes if (	ref & mask ) != ( stencil & mask )
	*/

	glStencilFunc(GL_ALWAYS, 1, 1);

	/*
		glStencilOp has 3 parameters. The first parameter specifies the action to take if the
		stencil test fails. The second specifies the stencil action to take if the stencil test passes
		but the depth test fails. The third one specifies the stencil action when the stencil test and
		the depth test pass, or when the stencil test passes and their is no depth testing done.

		These three parameters can be set to one of the following

		GL_KEEP Keeps the current value.
		GL_ZERO Sets the stencil buffer value to zero.
		GL_REPLACE Sets the stencil buffer value to ref, as specified by glStencilFunc.
		GL_INCR Increments the current stencil buffer value. Clamps to the maximum representable unsigned value.
		GL_DECR Decrements the current stencil buffer value. Clamps to zero.
		GL_INVERT	Bitwise inverts the current stencil buffer value.

		In our case as we are not using the depth test we are setting the third parameter to GL_INCR and
		a value of 1 will be put in the stencil buffer where ever the pixel is within the stencil geometry.
	*/
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

	glLoadMatrixf(&mCylinder.f[0]);

	m_Cylinder.DrawMesh(0);

	/*
		Using frontface culling we have just rendered all the back faces of our stencil
		geometry. What we are going to do now is to render all the front faces but where
		a pixel is behind the geometry we are going to decrement the stencil buffer value.
	*/
	glStencilFunc(GL_ALWAYS, 0, 1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);

	// Draw the cylinder with back face culling enable.
	glCullFace(GL_BACK);
	m_Cylinder.DrawMesh(0);

	/*
		Enable drawing to the colour buffer again as what we draw now we want to be visible.
		Switch back to front face culling and enable the depth test again.
	*/

#if defined(__PALMPDK__)
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE); // The alpha part is false as we don't want to blend with the video layer on the Palm Pre
#else
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
#endif
	glDepthMask(GL_TRUE);
	glCullFace(GL_FRONT);

	/*
		Set the stencil test to draw only pixels that are inside the stencil volume
	*/
	glStencilFunc(GL_NOTEQUAL, 0, 0xFFFFFFFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glBindTexture(GL_TEXTURE_2D, m_uiTileTex);

	glLoadMatrixf(&mSphere.f[0]);

	m_Sphere.DrawMesh(0);

	// Disable the stencil test as it is no longer needed
	glDisable(GL_STENCIL_TEST);

	/*
		Draw the cylinder with alpha blending, back faces first then front faces
	*/
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, m_uiCylinderTex);

	glLoadMatrixf(&mCylinder.f[0]);

	// Draw the front faces
	glCullFace(GL_BACK);
	m_Cylinder.DrawMesh(0);

	// Draw back faces of the cylinder
	glCullFace(GL_FRONT);
	m_Cylinder.DrawMesh(0);

	// Disable blending as it is no longer required
	glDisable(GL_BLEND);

	// Display the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("Stencil Buffer", m_pDescription, ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();
	return true;
}

/*!****************************************************************************
 @Function		CModel
 @Description	Constructor
******************************************************************************/
CModel::CModel() : m_puiVbo(0), m_puiIndexVbo(0)
{
}

/*!****************************************************************************
 @Function		~CModel
 @Description	Destructor
******************************************************************************/
CModel::~CModel()
{
	m_Scene.Destroy();
}

/*!****************************************************************************
 @Function		ReadFromFile
 @Input			pszFilename		filename of file to read
 @Return		bool			true if no error occured
 @Description	Loads POD file
******************************************************************************/
bool CModel::ReadFromFile(const char* const pszFilename)
{
	return m_Scene.ReadFromFile(pszFilename) == PVR_SUCCESS;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads data from model into vertex buffer objects
******************************************************************************/
void CModel::LoadVbos()
{
	if (!m_puiVbo)
		m_puiVbo = new GLuint[m_Scene.nNumMesh];

	if (!m_puiIndexVbo)
		m_puiIndexVbo = new GLuint[m_Scene.nNumMesh];

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
 @Function		DeleteVbos
 @Description	Deletes vertex buffer objects for model
******************************************************************************/
void CModel::DeleteVbos()
{
	// Delete buffer objects
	glDeleteBuffers(m_Scene.nNumMesh, m_puiVbo);
	glDeleteBuffers(m_Scene.nNumMesh, m_puiIndexVbo);

	delete [] m_puiVbo;
	delete [] m_puiIndexVbo;
	m_puiVbo = 0;
	m_puiIndexVbo = 0;
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			i32NodeIndex		Node index of the mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the meterial prepared.
******************************************************************************/
void CModel::DrawMesh(int i32NodeIndex)
{
	int i32MeshIndex = m_Scene.pNode[i32NodeIndex].nIdx;
	SPODMesh* pMesh = &m_Scene.pMesh[i32MeshIndex];

	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[i32MeshIndex]);

	// bind the index buffer, won't hurt if the handle is 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[i32MeshIndex]);

	// Enable the vertex client states
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// Set the vertex attribute offsets
	glVertexPointer(3, GL_FLOAT, pMesh->sVertex.nStride, pMesh->sVertex.pData);
	glTexCoordPointer(2, GL_FLOAT, pMesh->psUVW[0].nStride, pMesh->psUVW[0].pData);

	// Draw our indexed Triangle list
	glDrawElements(GL_TRIANGLES, pMesh->nNumFaces*3, GL_UNSIGNED_SHORT, 0);

	// Safely disable the vertex client states
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

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
	return new OGLESStencilBuffer();
}

/******************************************************************************
 End of file (OGLESStencilBuffer.cpp)
******************************************************************************/

