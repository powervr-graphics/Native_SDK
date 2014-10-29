/******************************************************************************

 @File         OGLESTrilinear.cpp

 @Title        OGLES filtering modes

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows the bilinear and trilinear filtering modes. A POD scene file
               is loaded and displayed 3 times with no filtering, bilinear
               filtering and trilinear filtering. See IntroducingPOD for
               explanations on the use of POD files.

******************************************************************************/
#include <math.h>
#include <string.h>

#include "PVRShell.h"
#include "OGLESTools.h"

/******************************************************************************
 Content file names
******************************************************************************/

// PVR texture files
const char c_szBackTexFile[] = "Back.pvr";
const char c_szTapeTexFile[] = "Tape.pvr";
const char c_szBallTexFile[] = "Ball.pvr";
const char c_szInfoTexFile[] = "Info.pvr";

// POD File
const char c_szSceneFile[]	= "o_model.pod";

/******************************************************************************
 Defines
******************************************************************************/

// Camera constants. Used for making the projection matrix
const float g_CameraNear = 4.0f;
const float g_CameraFar  = 5000.0f;

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLESTrilinear : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// Projection and Model View matrices
	PVRTMat4		m_mProjection, m_mView;

	// Texture handles
	GLuint			m_ui32TexBackground;
	GLuint			m_ui32TexTape[3], m_ui32TexBall[3], m_ui32TexInfo[3];

	// Vertex Buffer Object (VBO) handles
	GLuint*	m_puiVbo;
	GLuint*	m_puiIndexVbo;

	// Values used to look up the nodes by their names
	int	m_i32NodeSphere, m_i32NodeTape, m_i32NodeBanner1, m_i32NodeBanner2, m_i32NodeBanner3, m_i32NodeBanner4;

	// 3D Model
	CPVRTModelPOD	m_Scene;

	// Enum to represent one of the three models displayed
	enum eTapePosition {eLeft, eMiddle, eRight};

	// Used for animating the models
	float m_fFrame;

	CPVRTBackground m_Background;

public:
	OGLESTrilinear() : m_puiVbo(0),
					   m_puiIndexVbo(0)
	{
	}

	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	void DrawTape(eTapePosition ePosition);
	void DrawSphere(eTapePosition ePosition);
	void DrawBanner(eTapePosition ePosition);

	void DrawMesh(SPODMesh* pMesh);
	void GetModelMatrixFromPosition(PVRTMat4& mModel, eTapePosition ePosition);
	void GetSpherePosition(eTapePosition ePosition, float* fSpherePosY, float* fSpherePosZ);
	void ComputeTapeVertices(eTapePosition ePosition);
	bool LoadTextures();
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
bool OGLESTrilinear::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	// Loads the scene.
	if(m_Scene.ReadFromFile(c_szSceneFile) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Couldn't load the .pod file\n");
		return false;
	}

	// Initialize the variable used for animation
	m_fFrame = 0;

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
bool OGLESTrilinear::QuitApplication()
{
	// Destroys the scene
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
bool OGLESTrilinear::InitView()
{
	// Is the screen rotated?
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	// Initialize Print3D
	if(m_Print3D.SetTextures(0, PVRShellGet(prefWidth), PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	// Initialize Background
	if(m_Background.Init(0, bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Background\n");
		return false;
	}

	// Enables texturing
	glEnable(GL_TEXTURE_2D);

	// Loads the camera.
	if(m_Scene.nNumCamera == 0)
	{
		PVRShellSet(prefExitMessage, "ERROR: The scene does not contain a camera\n");
		return false;
	}

	PVRTVec3 vFrom, vTo, vUp(0.0f, 1.0f, 0.0f);
	float	fFOV;

	fFOV = m_Scene.GetCameraPos(vFrom, vTo, 0);
	m_mView = PVRTMat4::LookAtRH(vFrom, vTo, vUp);

	// Calculates the projection matrix
	m_mProjection = PVRTMat4::PerspectiveFovRH(fFOV, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), g_CameraNear, g_CameraFar, PVRTMat4::OGL, bRotate);

	// Loads the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(m_mProjection.f);

	glMatrixMode(GL_MODELVIEW);

	// Loads the textures
	if(!LoadTextures())
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot load the textures\n");
		return false;
	}

	//	Initialize VBO data
	LoadVbos();

	/*
		Saves the indices of the nodes,
		according to their names in 3D Studio.
		Used for easily accessing the nodes we want later.
	*/
	for(unsigned int i = 0; i < m_Scene.nNumNode; ++i)
	{
		SPODNode* pNode = &m_Scene.pNode[i];

		if(!strcmp(pNode->pszName, "Sphere"))
			m_i32NodeSphere = i;
		else if(!strcmp(pNode->pszName, "Tape"))
			m_i32NodeTape = i;
		else if(!strcmp(pNode->pszName, "Banner1"))
			m_i32NodeBanner2 = i;
		else if(!strcmp(pNode->pszName, "Banner2"))
			m_i32NodeBanner1 = i;
		else if(!strcmp(pNode->pszName, "Banner3"))
			m_i32NodeBanner3 = i;
		else if(!strcmp(pNode->pszName, "Banner4"))
			m_i32NodeBanner4 = i;
	}

	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLESTrilinear::LoadVbos()
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
bool OGLESTrilinear::ReleaseView()
{
	// Frees the texture
	glDeleteTextures(3, m_ui32TexInfo);
	glDeleteTextures(3, m_ui32TexBall);
	glDeleteTextures(3, m_ui32TexTape);

	glDeleteTextures(1, &m_ui32TexBackground);

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
bool OGLESTrilinear::RenderScene()
{
	++m_fFrame;

	if(m_fFrame > 627.0f)
		m_fFrame = 0.0f;

	// Clears the color buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use PVRTools to draw a background image
	m_Background.Draw(m_ui32TexBackground);

	// Enables the z-buffer
	glEnable(GL_DEPTH_TEST);

	// Enables the vertices and texture coordinates arrays
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// Draws the model without filtering
	DrawTape(eLeft);
	DrawSphere(eLeft);
	DrawBanner(eLeft);

	// Draws the model with mipmap filtering
	DrawTape(eRight);
	DrawSphere(eRight);
	DrawBanner(eRight);

	// Draws the model with trilinear filtering
	DrawTape(eMiddle);
	DrawSphere(eMiddle);
	DrawBanner(eMiddle);

	// unbind the vertex buffers as we don't need them bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("Trilinear", "Texture filter comparison.", ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		DrawTape
 @Input			ePosition		Position of the tape to draw
 @Description	Draws one of the 3 waving tapes.
******************************************************************************/
void OGLESTrilinear::DrawTape(eTapePosition ePosition)
{
	// Gets the tape node and mesh
	SPODNode* pNode = &m_Scene.pNode[m_i32NodeTape];
	SPODMesh* pMesh = &m_Scene.pMesh[pNode->nIdx];

	// Disables blending and back face culling
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	// Sets the tape texture with the correct filtering mode
	glBindTexture(GL_TEXTURE_2D, m_ui32TexTape[ePosition]);

	// Recalculates the waving tapes vertices
	ComputeTapeVertices(ePosition);

	/*
		Gets the tape model matrix from its position.
		And then calculates the model-view matrix.
	*/
	PVRTMat4 mModel, mModelView;
	GetModelMatrixFromPosition(mModel, ePosition);

	mModelView = m_mView * mModel;
	glLoadMatrixf(mModelView.f);

	// Give the vertex and texture coordinates data to OpenGL ES

	// unbind the VBO for the tape as we modify the meshes vertices every frame so are using them directly.
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glVertexPointer(3, GL_FLOAT, pMesh->sVertex.nStride, pMesh->pInterleaved + (size_t) pMesh->sVertex.pData);

	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[pNode->nIdx]);
	glTexCoordPointer(2, GL_FLOAT, pMesh->psUVW[0].nStride, pMesh->psUVW[0].pData);

	// bind the index buffer, won't hurt if the handle is 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[pNode->nIdx]);

	// Draw Indexed Triangle list
	glDrawElements(GL_TRIANGLES, pMesh->nNumFaces * 3, GL_UNSIGNED_SHORT, 0);
}

/*!****************************************************************************
 @Function		DrawSphere
 @Input			ePosition		Position of the sphere to draw
 @Description	Draws one of the 3 spheres.
******************************************************************************/
void OGLESTrilinear::DrawSphere(eTapePosition ePosition)
{
	// Gets the sphere node and mesh
	SPODNode* pNode = &m_Scene.pNode[m_i32NodeSphere];
	SPODMesh* pMesh = &m_Scene.pMesh[pNode->nIdx];

	// Enables transparency using blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Sets the sphere texture with the correct filtering mode
	glBindTexture(GL_TEXTURE_2D, m_ui32TexBall[ePosition]);

	/*
		Gets the sphere model matrix from its position.
		Then finds the sphere specific model matrix.
		And finally calculates the model-view matrix.
	*/
	PVRTMat4 mModel, mSpecificModel, mModelView, mScale, mTranslate, mRotateX, mRotateY;

	float fSpherePosY, fSpherePosZ;
	GetSpherePosition(ePosition, &fSpherePosY, &fSpherePosZ);
	GetModelMatrixFromPosition(mModel, ePosition);

	// Calculates the sphere specific transformation matrices
	mScale     = PVRTMat4::Scale(0.9f, 0.9f, 0.9f);
	mTranslate = PVRTMat4::Translation(0.0f, fSpherePosY, fSpherePosZ);
	mRotateX   = PVRTMat4::RotationX(m_fFrame/50.0f);
	mRotateY   = PVRTMat4::RotationY(m_fFrame/50.0f);

	// Compose all those matrices to get the model-view matrix
	mSpecificModel = mScale * mTranslate * mRotateX * mRotateY;
	mModelView = m_mView * mModel * mSpecificModel;

	// Loads it in OpenGL ES
	glLoadMatrixf(mModelView.f);

	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[pNode->nIdx]);

	// bind the index buffer, won't hurt if the handle is 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[pNode->nIdx]);

	/*
		To display properly transparency using blending, the geometry must be
		drawn from back to front. If it is not, the z-test will prevent the
		faces behind to be drawn at all.
		So we use back face culling to first draw the front faces of the
		sphere then the faces behind.
	*/

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glFrontFace(GL_CW);
	DrawMesh(pMesh);

	glFrontFace(GL_CCW);
	DrawMesh(pMesh);
}

/*!****************************************************************************
 @Function		DrawBanner
 @Input			ePosition		Position of the banner to draw
 @Description	Draws one of the 3 banners (descriptive text on top of
				each model).
******************************************************************************/
void OGLESTrilinear::DrawBanner(eTapePosition ePosition)
{
	/*
		Gets the banner node and mesh.
		Unlike the tape or sphere, the model contains the three banners
		seperately so each can have its own set of texture coordinates.
	*/
	SPODNode* pNode = 0;

	switch (ePosition)
	{
		case eLeft:
			pNode = &m_Scene.pNode[m_i32NodeBanner1];
			break;
		case eMiddle:
			pNode = &m_Scene.pNode[m_i32NodeBanner2];
			break;
		case eRight:
			pNode = &m_Scene.pNode[m_i32NodeBanner3];
			break;
	}

	SPODMesh* pMesh = &m_Scene.pMesh[pNode->nIdx];

	// Enables transparency using blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Sets the banner texture with the correct filtering mode
	glBindTexture(GL_TEXTURE_2D, m_ui32TexInfo[ePosition]);

	/*
		Gets the banner model matrix from its position.
		Then finds the banner specific model matrix.
		And finally calculates the model-view matrix.
	*/
	PVRTMat4 mModel, mSpecificModel, mModelView;
	float fSpherePosY, fSpherePosZ;

	GetSpherePosition(ePosition, &fSpherePosY, &fSpherePosZ);
	GetModelMatrixFromPosition(mModel, ePosition);

	// Calculates the sphere specific transformation matrices
	mSpecificModel = PVRTMat4::Translation(0.0f, fSpherePosY, fSpherePosZ);

	// Compose all those matrices to get the model-view matrix
	mModelView = m_mView * mModel * mSpecificModel;

	// Loads it in OpenGL ES
	glLoadMatrixf(mModelView.f);

	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[pNode->nIdx]);

	// bind the index buffer, won't hurt if the handle is 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[pNode->nIdx]);

	// Draw the banner
	DrawMesh(pMesh);
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			mesh		The mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the meterial prepared.
******************************************************************************/
void OGLESTrilinear::DrawMesh(SPODMesh* pMesh)
{
	// Give the vertex and texture coordinates data to OpenGL ES
	glVertexPointer(3, GL_FLOAT, pMesh->sVertex.nStride, pMesh->sVertex.pData);
	glTexCoordPointer(2, GL_FLOAT, pMesh->psUVW[0].nStride, pMesh->psUVW[0].pData);

	// Draw Indexed Triangle list
	glDrawElements(GL_TRIANGLES, pMesh->nNumFaces * 3, GL_UNSIGNED_SHORT, 0);
}

/*!****************************************************************************
 @Function		GetModelMatrixFromPosition
 @Input			ePosition		Position of the model (eLeft, eMiddle or eRight)
 @Output		mModel			Returned mode matrix to use
 @Description	Returns the model matrix to use (to place to model to the
				eLeft, eMiddle or eRight).
******************************************************************************/
void OGLESTrilinear::GetModelMatrixFromPosition(PVRTMat4 &mModel, eTapePosition ePosition)
{
	switch(ePosition)
	{
		case eLeft:
			mModel = PVRTMat4::Translation(-110,0,0);
			break;
		case eMiddle:
			mModel = PVRTMat4::Identity();
			break;
		case eRight:
			mModel = PVRTMat4::Translation(110,0,0);
			break;
	}
}

/*!****************************************************************************
 @Function		GetSpherePosition
 @Input			ePosition		Position of the model (eLeft, eMiddle or eRight)
 @Output		fSpherePosY		Returned position along Y
 @Output		fSpherePosZ		Returned position along Z
 @Description	Returns the sphere position at a given time. Used to make the
				spheres go back and forth on the tape.
******************************************************************************/
void OGLESTrilinear::GetSpherePosition(eTapePosition ePosition, float* fSpherePosY, float* fSpherePosZ)
{
	float Offset = m_fFrame/20.0f;

	if(ePosition == eMiddle)
	{
		*fSpherePosZ = 700.0f * (float)PVRTFSIN(m_fFrame/100.0f) - 700.0f;
		float fAngle = *fSpherePosZ * (1.0f/100.0f) + Offset;
		*fSpherePosY = (float)PVRTFSIN(fAngle) * 15.0f;
	}
	else
	{
		*fSpherePosZ = 600.0f * (float)PVRTFSIN(m_fFrame/100.0f) - 700.0f;
		float fAngle = *fSpherePosZ * (1.0f/100.0f) + Offset;
		*fSpherePosY = (float)PVRTFCOS(fAngle) * 15.0f;
	}
}

/*!****************************************************************************
 @Function		ComputeTapeVertices
 @Input			ePosition		Position of the tape (eLeft, eMiddle or eRight)
 @Description	Recalculate the vertices of a given tape. Used to make the
				tapes move along waves.
******************************************************************************/
void OGLESTrilinear::ComputeTapeVertices(eTapePosition ePosition)
{
	float Offset = m_fFrame / 20.0f;
	SPODMesh* pMesh = &m_Scene.pMesh[m_Scene.pNode[m_i32NodeTape].nIdx];

	unsigned char* pfY = pMesh->pInterleaved + (size_t)pMesh->sVertex.pData + sizeof(float) * 1;
	unsigned char* pfZ = pMesh->pInterleaved + (size_t)pMesh->sVertex.pData + sizeof(float) * 2;

	for(unsigned int i = 0; i < pMesh->nNumVertex; ++i)
	{
		float fAngle = *(float*)pfZ * (1.0f/100.0f) + Offset;

		if (ePosition==eMiddle)
			*(float*)pfY = (float)PVRTFSIN(fAngle) * 15.0f - 30.0f;
		else
			*(float*)pfY = (float)PVRTFCOS(fAngle) * 15.0f - 30.0f;

		pfY += pMesh->sVertex.nStride;
		pfZ += pMesh->sVertex.nStride;
	}
}

/*!****************************************************************************
 @Function		LoadTextures
 @Return		bool			true for success, false for failure
 @Description	Loads the textures.
******************************************************************************/
bool OGLESTrilinear::LoadTextures()
{
	/*
		Loads the textures.
		For a detailed explanation see the Texturing training course.
	*/
	if(PVRTTextureLoadFromPVR(c_szBackTexFile, &m_ui32TexBackground))
		return false;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/*
		We need to load the tape, ball and info textures 3 times with
		different filtering modes.
		That is because changing the filtering mode every frame for
		these textures would make the drivers upload the textures every frame,
		so it would be very slow.
	*/
	/*
		Loads the textures used for nearest filtering mode.
	*/
	if(PVRTTextureLoadFromPVR(c_szTapeTexFile, &m_ui32TexTape[eLeft]))
		return false;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szBallTexFile, &m_ui32TexBall[eLeft]))
		return false;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szInfoTexFile, &m_ui32TexInfo[eLeft]))
		return false;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/*
		Loads the textures used for bilinear filtering mode.
	*/
	if(PVRTTextureLoadFromPVR(c_szTapeTexFile, &m_ui32TexTape[eRight]))
		return false;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szBallTexFile, &m_ui32TexBall[eRight]))
		return false;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szInfoTexFile, &m_ui32TexInfo[eRight]))
		return false;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/*
		Loads the textures used for trilinear filtering mode.
	*/
	if(PVRTTextureLoadFromPVR(c_szTapeTexFile, &m_ui32TexTape[eMiddle]))
		return false;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szBallTexFile, &m_ui32TexBall[eMiddle]))
		return false;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szInfoTexFile, &m_ui32TexInfo[eMiddle]))
		return false;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
	return new OGLESTrilinear();
}

/******************************************************************************
 End of file (OGLESTrilinear.cpp)
******************************************************************************/

