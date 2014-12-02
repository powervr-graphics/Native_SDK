/******************************************************************************

 @File         OGLESMatrixPalette.cpp

 @Title        OGLESMatrixPalette

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows the use of Matrix Palettes.

******************************************************************************/

/****************************************************************************
 ** INCLUDES                                                               **
 ****************************************************************************/
#include <math.h>
#include "PVRShell.h"
#include "OGLESTools.h"

/******************************************************************************
 Content file names
******************************************************************************/

// PVR texture files
const char c_szMalletTexFile[]		= "Mallet.pvr";

// POD File
const char c_szSceneFile[]	= "model.pod";

/****************************************************************************
 ** CONSTS                                                                **
 ****************************************************************************/

const float g_fCameraNear   = 3000.0f;
const float g_fCameraFar    = 4000.0f;

/****************************************************************************
** Class: OGLESMatrixPalette
****************************************************************************/
class OGLESMatrixPalette : public PVRShell
{
	// Texture IDs
	GLuint m_ui32MalletTexture;

	// Print3D, Extension and POD Class Objects
	CPVRTPrint3D 		m_Print3D;
	CPVRTModelPOD		m_Scene;

	// View and Projection Matrices
	PVRTMat4			m_mView, m_mProjection;

	// Used to go through the animation
	float m_fFrame;

	// Extensions
	CPVRTglesExt m_Extensions;

	// Vertex Buffer Object (VBO) handles
	GLuint*	m_puiVbo;
	GLuint*	m_puiIndexVbo;

public:
	OGLESMatrixPalette() :  m_puiVbo(0),
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
	void CameraGetMatrix();
	void ComputeViewMatrix();
	void LoadMaterial(int i32Index);
	void LoadVbos();
	void DrawModel();
};


/*******************************************************************************
 * Function Name  : InitApplication
 * Returns        : true if no error occured
 * Description    : Code in InitApplication() will be called by the Shell ONCE per
 *					run, early on in the execution of the program.
 *					Used to initialize variables that are not dependant on the
 *					rendering context (e.g. external modules, loading meshes, etc.)
 *******************************************************************************/
bool OGLESMatrixPalette::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	// Load the POD file.
	/*
		The vertex data in the pod file is interleaved. Due to requirements with alignment
		on some ARM based MBX platforms this data needs to be 32 bit aligned (the stride of
		a vertex should be divisible by 4). To achieve this we have padded out the vertex
		data by exporting a dummy second set of UV coordinates where each coordinate is a
		byte in size.
	*/
	if(m_Scene.ReadFromFile(c_szSceneFile) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "Error: Failed to load scene.\n");
		return false;
	}

	m_fFrame = 0;
	return true;
}

/*******************************************************************************
 * Function Name  : QuitApplication
 * Returns        : true if no error occured
 * Description    : Code in QuitApplication() will be called by the Shell ONCE per
 *					run, just before exiting the program.
 *******************************************************************************/
bool OGLESMatrixPalette::QuitApplication()
{
	// Destroy the scene
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
bool OGLESMatrixPalette::InitView()
{
	//	Check to see whether the matrix palette extension is supported.
	if(!CPVRTglesExt::IsGLExtensionSupported("GL_OES_matrix_palette"))
	{
		PVRShellSet(prefExitMessage, "ERROR: The extension GL_OES_matrix_palette is unsupported.\n");
		return false;
	}

	// Initialise the matrix palette extensions
	m_Extensions.LoadExtensions();

	// Init the model texture
	if(PVRTTextureLoadFromPVR(c_szMalletTexFile, &m_ui32MalletTexture) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load texture for Mallet.\n");
		return false;
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Init Print3D to display text on screen
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	if(m_Print3D.SetTextures(0, PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	// Model View Matrix
	CameraGetMatrix();

	// Projection Matrix
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(m_mProjection.f);

	// GENERIC RENDER STATES

	// Enables Depth Testing
	glEnable(GL_DEPTH_TEST);

	// Enables Smooth Colour Shading
	glShadeModel(GL_SMOOTH);

	// Enable texturing
	glEnable(GL_TEXTURE_2D);

	// Define front faces
	glFrontFace(GL_CW);

	// Enables texture clamping
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	// Reset the model view matrix to position the light
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Setup ambiant light
    glEnable(GL_LIGHTING);

	PVRTVec4 lightGlobalAmbient = PVRTVec4(1.0f, 1.0f, 1.0f, 1.0f);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightGlobalAmbient.ptr());

	// Setup a directional light source
	PVRTVec4 lightPosition = PVRTVec4(-0.7f, -1.0f, 0.2f, 0.0f);
    PVRTVec4 lightAmbient  = PVRTVec4(1.0f, 1.0f, 1.0f, 1.0f);
    PVRTVec4 lightDiffuse  = PVRTVec4(1.0f, 1.0f, 1.0f, 1.0f);
    PVRTVec4 lightSpecular = PVRTVec4(0.2f, 0.2f, 0.2f, 1.0f);

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition.ptr());
    glLightfv(GL_LIGHT0, GL_AMBIENT,  lightAmbient.ptr());
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiffuse.ptr());
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular.ptr());

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
void OGLESMatrixPalette::LoadVbos()
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


/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns        : Nothing
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool OGLESMatrixPalette::ReleaseView()
{
	// Release all Textures
	glDeleteTextures(1, &m_ui32MalletTexture);

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
bool OGLESMatrixPalette::RenderScene()
{
	// Increase the frame number
	m_fFrame += 1.0f;

	while(m_fFrame > m_Scene.nNumFrame-1)
		m_fFrame -= m_Scene.nNumFrame-1;

	// Clear the depth and frame buffer
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set Z compare properties
	glEnable(GL_DEPTH_TEST);

	// Disable Blending
	glDisable(GL_BLEND);

	// Calculate the model view matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(m_mView.f);

	// Draw the model
	DrawModel();

	// Print text on screen
	m_Print3D.DisplayDefaultTitle("MatrixPalette", "", ePVRTPrint3DSDKLogo);

	// Flush all Print3D commands
	m_Print3D.Flush();

	return true;
}

/*******************************************************************************
 * Function Name  : LoadMaterial
 * Input		  : i32Index index into the material list
 * Description    : Loads the material index
 *******************************************************************************/
void OGLESMatrixPalette::LoadMaterial(int i32Index)
{
	//	Load the model's material
	SPODMaterial* pMaterial = &m_Scene.pMaterial[i32Index];

	glBindTexture(GL_TEXTURE_2D, m_ui32MalletTexture);

	PVRTVec4 fMat;
	int i;
	fMat.ptr()[3] = 1.0f;

	for(i = 0; i < 3; ++i)
		fMat.ptr()[i] = pMaterial->pfMatAmbient[i];

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, fMat.ptr());

	for(i = 0; i < 3; ++i)
		fMat.ptr()[i] = pMaterial->pfMatDiffuse[i];

	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, fMat.ptr());

	for(i = 0; i < 3; ++i)
		fMat.ptr()[i] = pMaterial->pfMatSpecular[i];

	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, fMat.ptr());
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, pMaterial->fMatShininess);
}


/*******************************************************************************
 * Function Name  : DrawModel
 * Description    : Draws the model
 *******************************************************************************/
void OGLESMatrixPalette::DrawModel()
{
	//Set the frame number
	m_Scene.SetFrame(m_fFrame);

	//Iterate through all the mesh nodes in the scene
	for(int iNode = 0; iNode < (int)m_Scene.nNumMeshNode; ++iNode)
	{
		//Get the mesh node.
		SPODNode* pNode = &m_Scene.pNode[iNode];

		//Get the mesh that the mesh node uses.
		SPODMesh* pMesh = &m_Scene.pMesh[pNode->nIdx];

		// bind the VBO for the mesh
		glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[pNode->nIdx]);

		// bind the index buffer, won't hurt if the handle is 0
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[pNode->nIdx]);

		//Load the material that belongs to the mesh node.
		LoadMaterial(pNode->nIdxMaterial);

		//If the mesh has bone weight data then we must be skinning.
		bool bSkinning = pMesh->sBoneWeight.n != 0;

		// If the mesh is used for skining then set up the matrix palettes.
		if(bSkinning)
		{
			//Enable the matrix palette extension
			glEnable(GL_MATRIX_PALETTE_OES);
			/*
				Enables the matrix palette stack extension, and apply subsequent
				matrix operations to the matrix palette stack.
			*/
			glMatrixMode(GL_MATRIX_PALETTE_OES);

			PVRTMat4	mBoneWorld;
			int			i32NodeID;

			//	Iterate through all the bones in the batch
			for(int j = 0; j < pMesh->sBoneBatches.pnBatchBoneCnt[0]; ++j)
			{
				/*
					Set the current matrix palette that we wish to change. An error
					will be returned if the index (j) is not between 0 and
					GL_MAX_PALETTE_MATRICES_OES. The value of GL_MAX_PALETTE_MATRICES_OES
					can be retrieved using glGetIntegerv, the initial value is 9.

					GL_MAX_PALETTE_MATRICES_OES does not mean you need to limit
					your character to 9 bones as you can overcome this limitation
					by using bone batching which splits the mesh up into sub-meshes
					which use only a subset of the bones.
				*/

				m_Extensions.glCurrentPaletteMatrixOES(j);

				// Generates the world matrix for the given bone in this batch.
				i32NodeID = pMesh->sBoneBatches.pnBatches[j];
				m_Scene.GetBoneWorldMatrix(mBoneWorld, *pNode, m_Scene.pNode[i32NodeID]);

				// Multiply the bone's world matrix by the view matrix to put it in view space
				PVRTMatrixMultiply(mBoneWorld, mBoneWorld, m_mView);

				// Load the bone matrix into the current palette matrix.
				glLoadMatrixf(mBoneWorld.f);
			}
		}
		else
		{
			// If we're not using matrix palette then get the world matrix for the mesh
			// and transform the model view matrix by it.
			glDisable(GL_MATRIX_PALETTE_OES);

			//Switch to the modelview matrix.
			glMatrixMode(GL_MODELVIEW);

			//Push the modelview matrix
			glPushMatrix();

			//Get the world matrix for the mesh and transform the model view matrix by it.
			PVRTMat4 worldMatrix;
			m_Scene.GetWorldMatrix(worldMatrix, *pNode);
			glMultMatrixf(worldMatrix.f);
		}

		// Modulate with vertex colour
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		// Enable lighting
		glEnable(GL_LIGHTING);

		// Enable States
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);

		// If the mesh has uv coordinates then enable the texture coord array state
		if(pMesh->psUVW)
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		if(bSkinning)
		{
			//If we are skinning then enable the relevant states.
			glEnableClientState(GL_MATRIX_INDEX_ARRAY_OES);
			glEnableClientState(GL_WEIGHT_ARRAY_OES);
		}


		// Set Data Pointers
		// Used to display non interleaved geometry
		glVertexPointer(pMesh->sVertex.n, GL_FLOAT, pMesh->sVertex.nStride, pMesh->sVertex.pData);
		glNormalPointer(GL_FLOAT, pMesh->sNormals.nStride, pMesh->sNormals.pData);

		if(pMesh->psUVW)
			glTexCoordPointer(pMesh->psUVW[0].n, GL_FLOAT, pMesh->psUVW[0].nStride, pMesh->psUVW[0].pData);

		if(bSkinning)
		{
			//Set up the indexes into the matrix palette.
			m_Extensions.glMatrixIndexPointerOES(pMesh->sBoneIdx.n, GL_UNSIGNED_BYTE, pMesh->sBoneIdx.nStride, pMesh->sBoneIdx.pData);
			m_Extensions.glWeightPointerOES(pMesh->sBoneWeight.n, GL_FLOAT, pMesh->sBoneWeight.nStride, pMesh->sBoneWeight.pData);
		}

		// Draw

		// Indexed Triangle list
		glDrawElements(GL_TRIANGLES, pMesh->nNumFaces*3, GL_UNSIGNED_SHORT, 0);

		// Disable States
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		if(bSkinning)
		{
			glDisableClientState(GL_MATRIX_INDEX_ARRAY_OES);
			glDisableClientState(GL_WEIGHT_ARRAY_OES);
		}
		else
		{
			//Reset the modelview matrix back to what it was before we transformed by the mesh node.
			glPopMatrix();
		}
	}


	// We are finished with the matrix pallete so disable it.
	glDisable(GL_MATRIX_PALETTE_OES);

	// unbind the vertex buffers as we don't need them bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


/*******************************************************************************
 * Function Name  : CameraGetMatrix
 * Global Used    :
 * Description    : Function to setup camera position
 *
 *******************************************************************************/
void OGLESMatrixPalette::CameraGetMatrix()
{
	PVRTVec3	vFrom, vTo, vUp;
	float	fFOV;
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	// Set the Up Vector
	vUp = PVRTVec3(0.0f, 1.0f, 0.0f);

	// If the scene contains a camera then...
	if(m_Scene.nNumCamera)
	{
		//.. get the Camera's position, direction and FOV.
		fFOV = m_Scene.GetCameraPos(vFrom, vTo, 0);
		/*
		Convert the camera's field of view from horizontal to vertical
		(the 0.75 assumes a 4:3 aspect ratio).
		*/
		fFOV = fFOV * 0.75f;
	}
	else
		fFOV = PVRT_PI * 0.16667f;

	// Set up the view matrix
	m_mView = PVRTMat4::LookAtRH(vFrom, vTo, vUp);

	// Set up the projection matrix
	m_mProjection = PVRTMat4::PerspectiveFovRH(fFOV, (float) PVRShellGet(prefWidth) / (float) PVRShellGet(prefHeight), g_fCameraNear, g_fCameraFar, PVRTMat4::OGL, bRotate);
}

/*******************************************************************************
 * Function Name  : NewDemo
 * Description    : Called by the Shell to initialise a new instance to the
 *					demo class.
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new OGLESMatrixPalette();
}

/*****************************************************************************
 End of file (OGLESMatrixPalette.cpp)
*****************************************************************************/

