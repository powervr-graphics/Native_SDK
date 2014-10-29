/******************************************************************************

 @File         OGLESMouse.cpp

 @Title        OGLESMouse

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Demonstrates cell-shading (cartoon style)

******************************************************************************/
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "PVRShell.h"
#include "OGLESTools.h"

/******************************************************************************
 Content file names
******************************************************************************/

// PVR texture files
const char c_szToonTexFile[]		= "Toon.pvr";		// Special Texture to generate Toon Shading
const char c_szMouseToonTexFile[]	= "MouseToon.pvr";	// Mouse Base Texture
const char c_szWallToonTexFile[]	= "WallToon.pvr";	// Wall Texture
const char c_szFloorToonTexFile[]	= "FloorToon.pvr";	// Floor Texture

// POD File
const char c_szSceneFile[]	= "Mouse.pod";

/******************************************************************************
 Defines
******************************************************************************/

/******************************************************************************
 Consts
******************************************************************************/

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLESMouse : public PVRShell
{
	// 3D Model
	CPVRTModelPOD	m_Scene;

	// Texture handle
	GLuint			m_ui32Mouse, m_ui32Floor, m_ui32Wall, m_ui32Toon;

	// Array to lookup the textures for each material in the scene
	GLuint*			m_pui32Textures;

	// Animation Related
	GLfloat m_fFrame, m_fFrameIncr;

	// Print3D class used to display text
	CPVRTPrint3D m_Print3D;

	PVRTVec4	m_CameraInverse;
	PVRTVec4	m_LightInverse;
	float	*m_pUVBuffer;	// Used for software processing

	// Vertex buffer object handles
	GLuint* m_puiVbo;
	GLuint* m_puiIndexVbo;

public:
	OGLESMouse();

	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	void DrawMesh(unsigned int ui32MeshID);
	bool InitVertexProgram(const void *pCSH, int nSzCSH);
	bool LoadTextures(CPVRTString* const pErrorStr);
	void LoadVbos();
	void SetupCamAndLightInverse(PVRTMat4 mModelView);
};

OGLESMouse::OGLESMouse() :  m_pui32Textures(0),
							m_fFrame(1.0f),
							m_fFrameIncr(1.0f),
							m_pUVBuffer(0),
							m_puiVbo(0),
							m_puiIndexVbo(0)
{
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
bool OGLESMouse::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	/*
		Loads the scene from the .pod file into a CPVRTModelPOD object.
	*/

	// Load the meshes
	if(m_Scene.ReadFromFile(c_szSceneFile) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load Mouse_*.pod!");
		return false;
	}

	// The cameras are stored in the file. We check it contains at least one.
	if (m_Scene.nNumCamera == 0)
	{
		PVRShellSet(prefExitMessage, "ERROR: The scene does not contain a camera\n");
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
bool OGLESMouse::QuitApplication()
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
bool OGLESMouse::InitView()
{
	CPVRTString ErrorStr;
	PVRTMATRIX	 ProjectionMatrix;

	float fWidth  = (float)PVRShellGet(prefWidth);
	float fHeight = (float)PVRShellGet(prefHeight);

	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	// Init Print3D to display text on screen
	if(m_Print3D.SetTextures(0, PVRShellGet(prefWidth), PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialize Print3D\n");
		return false;
	}

	/*
		Initialize VBO data
	*/
	LoadVbos();

	/*
		Load the textures.
	*/

	if(!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Use depth testing and no blending
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	// Set the clear colour
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	/*
		Create the perspective matrix.
	*/
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if(bRotate)
	{
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		fWidth  = (float)PVRShellGet(prefHeight);
		fHeight = (float)PVRShellGet(prefWidth);
	}

	PVRTMatrixPerspectiveFovRH(ProjectionMatrix, 20.0f * (3.14f / 180.0f), fWidth / fHeight, 800.0f, 2800.0f);
	glMultMatrixf(ProjectionMatrix.f);

	// Switch back to the modelview matrix
	glMatrixMode(GL_MODELVIEW);

	/*
		Build an array to map the textures within the pod file
		to the textures we loaded earlier.
	*/

	m_pui32Textures = new GLuint[m_Scene.nNumMaterial];

	for(unsigned int i = 0; i < m_Scene.nNumMaterial; ++i)
	{
		m_pui32Textures[i] = 0;
		SPODMaterial* pMaterial = &m_Scene.pMaterial[i];

		if(!strcmp(pMaterial->pszName, "mouse"))
			m_pui32Textures[i] = m_ui32Mouse;
		else if(!strcmp(pMaterial->pszName, "floor"))
			m_pui32Textures[i] = m_ui32Floor;
		else if(!strcmp(pMaterial->pszName, "wall"))
			m_pui32Textures[i] = m_ui32Wall;
	}

	// Calculate the second set of UVs
	{
		int i32MaxVertexNo = 0;
		int i32VertexNo;

		// Calculate the max number of UVs we'll require
		for(unsigned int i = 0; i < m_Scene.nNumMeshNode; ++i)
		{
			i32VertexNo = m_Scene.pMesh[m_Scene.pNode[i].nIdx].nNumVertex;

			if(i32VertexNo > i32MaxVertexNo)
				i32MaxVertexNo = i32VertexNo;
		}

		m_pUVBuffer = new float[i32MaxVertexNo * 2];

		if(!m_pUVBuffer)
		{
			PVRShellSet(prefExitMessage, "ERROR: Unable to allocate the UV buffer for software processing.");
			return false;
		}
	}

	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLESMouse::LoadVbos()
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
bool OGLESMouse::LoadTextures(CPVRTString* const pErrorStr)
{
	// Special Texture to generate Toon Shading
	if(PVRTTextureLoadFromPVR(c_szToonTexFile, &m_ui32Toon) != PVR_SUCCESS)
	{
		*pErrorStr = CPVRTString("ERROR: Failed to load texture ") + c_szToonTexFile;
		return false;
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Special Toon Texture Option to Clamp to Edge, no wrap around of tex coords.
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

	// Mouse Base Texture
	if(PVRTTextureLoadFromPVR(c_szMouseToonTexFile, &m_ui32Mouse) != PVR_SUCCESS)
	{
		*pErrorStr = CPVRTString("ERROR: Failed to load texture ") + c_szMouseToonTexFile;
		return false;
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Floor Texture
	if(PVRTTextureLoadFromPVR(c_szFloorToonTexFile, &m_ui32Floor) != PVR_SUCCESS)
	{
		*pErrorStr = CPVRTString("ERROR: Failed to load texture ") + c_szFloorToonTexFile;
		return false;
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Wall Texture
	if(PVRTTextureLoadFromPVR(c_szWallToonTexFile, &m_ui32Wall) != PVR_SUCCESS)
	{
		*pErrorStr = CPVRTString("ERROR: Failed to load texture ") + c_szWallToonTexFile;
		return false;
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLESMouse::ReleaseView()
{
	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

	// Frees the texture
	glDeleteTextures(1, &m_ui32Mouse);
	glDeleteTextures(1, &m_ui32Floor);
	glDeleteTextures(1, &m_ui32Wall);
	glDeleteTextures(1, &m_ui32Wall);

	delete[] m_pui32Textures;
	m_pui32Textures = 0;

	// Delete the UV buffer used for software processing
	delete[] m_pUVBuffer;
	m_pUVBuffer = 0;

	// Delete buffer objects
	glDeleteBuffers(m_Scene.nNumMesh, m_puiVbo);
	glDeleteBuffers(m_Scene.nNumMesh, m_puiIndexVbo);

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
bool OGLESMouse::RenderScene()
{
	PVRTVec3 vFrom, vTo, vUp;
	PVRTMat4 mWorld;
	PVRTMat4 mModelView;

	// Clears the color buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Enable texturing
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);

	/*
		Draw the scene.
	*/

	// Set the current frame
	m_Scene.SetFrame(m_fFrame);

	// Get the camera position and target
	m_Scene.GetCameraPos(vFrom, vTo, 0);
	m_Scene.GetCamera(vFrom, vTo, vUp, 0);
	// Build the model view matrix from the camera position, target and an up vector.
	PVRTMat4 mView = PVRTMat4::LookAtRH(vFrom, vTo, vUp);

	// Enable the client states
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	for(unsigned int i = 0; i < m_Scene.nNumMeshNode; ++i)
	{
		SPODNode& Node = m_Scene.pNode[i];

		// Gets the node model matrix
		m_Scene.GetWorldMatrix(mWorld, Node);

		// Multiply the view matrix by the model (mWorld) matrix to get the model-view matrix
		mModelView = mView * mWorld;
		glLoadMatrixf(mModelView.f);

		// Loads the correct texture using our texture lookup table
		glBindTexture(GL_TEXTURE_2D, m_pui32Textures[Node.nIdxMaterial]);

		/*
			Second layer multitexture for the cartoon effect.
			This layer will draw the black halo around the mesh and generate the banded lighting
			using modulate blending.
			Dynamic UV mapping is calculated in the Vertex Program
		*/
		if(m_pui32Textures[Node.nIdxMaterial] == m_ui32Mouse) // Only the mouse is multitextured
		{
			SetupCamAndLightInverse(mModelView);

			// Enable the second texture layer
			glActiveTexture(GL_TEXTURE1);
			glEnable(GL_TEXTURE_2D);

			// Sets the Correct Texture. Texture 100 is the special Toon Shading Texture
			glBindTexture(GL_TEXTURE_2D, m_ui32Toon);

			// Back to the default (0) texture layer
			glActiveTexture(GL_TEXTURE0);

			// Use Texture Combine Mode
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		}

		DrawMesh(Node.nIdx);

		/*
			If we were drawing the multitextured mouse,
			disable the second texture layer.
		*/
		if(m_pui32Textures[Node.nIdxMaterial] == m_ui32Mouse)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D,0);
			glDisable(GL_TEXTURE_2D);
			glActiveTexture(GL_TEXTURE0);
		}
	}

	// Update mouse animation
	if(m_fFrame >= m_Scene.nNumFrame - 1 || m_fFrame <= 0)
		m_fFrameIncr *= -1;

	m_fFrame += m_fFrameIncr;

	// Disable the normals before our drawing of the print3D content
	glDisableClientState(GL_NORMAL_ARRAY);

	// Draw Text
	m_Print3D.DisplayDefaultTitle("Mouse", "Toon Shading", ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		RenderPrimitive
 @Input			object		Mesh to render
 @Description	Draws a mesh.
******************************************************************************/
void OGLESMouse::DrawMesh(unsigned int ui32MeshID)
{
	SPODMesh& Mesh = m_Scene.pMesh[ui32MeshID];

	// Bind the vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[ui32MeshID]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[ui32MeshID]);

	// Setup pointers
	glVertexPointer(3, GL_FLOAT, Mesh.sVertex.nStride, Mesh.sVertex.pData);
	glNormalPointer(GL_FLOAT, Mesh.sNormals.nStride, Mesh.sNormals.pData);
	glTexCoordPointer(2, GL_FLOAT, Mesh.psUVW[0].nStride, Mesh.psUVW[0].pData);

	// unbind the vertex buffers as we don't need them bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/*
		Do software processing of the second set of UVs to generate
		the Toon texture coordinates.
	*/
	{
		float *pNormals;
		unsigned char* pData = Mesh.pInterleaved + (size_t) Mesh.sNormals.pData;

		for(unsigned int i = 0; i < Mesh.nNumVertex; ++i)
		{
			pNormals = (float*) pData;

            m_pUVBuffer[i * 2 + 0] = pNormals[0] * m_CameraInverse.x +
									 pNormals[1] * m_CameraInverse.y +
									 pNormals[2] * m_CameraInverse.z;

			m_pUVBuffer[i * 2 + 1] = pNormals[0] * m_LightInverse.x +
									 pNormals[1] * m_LightInverse.y +
									 pNormals[2] * m_LightInverse.z;

			pData += Mesh.sNormals.nStride;
		}

		glClientActiveTexture(GL_TEXTURE1);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2,GL_FLOAT,0, m_pUVBuffer);
	}

	glDrawElements(GL_TRIANGLES, Mesh.nNumFaces * 3, GL_UNSIGNED_SHORT, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Disable second layer texture coords
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glClientActiveTexture(GL_TEXTURE0);
}

/*!****************************************************************************
 @Function		SetAnimMatrix
 @Input			object		Object to set the matrix for
 @Input			uFrameCount	Frame number to set the matrix for
 @Description	To set a animation from 3DStudio MAX, feed the transformation matrix
				with the fValues exported by the PVRexp plug-in. And setup VGP Constants.
******************************************************************************/
void OGLESMouse::SetupCamAndLightInverse(PVRTMat4 mModelView)
{
	PVRTMat4  mModelViewInverse;
	PVRTVec4 LightPos;
	PVRTVec4 CameraPos;

	//	Get the Inverse of the world matrix
	PVRTMatrixInverseEx(mModelViewInverse,  mModelView);

	// Setup the camera position in the VGP code.
	CameraPos.x = 0.0f;
	CameraPos.y = 0.0f;
	CameraPos.z = 1000.0f;
	CameraPos.w = 1.0f;

	m_CameraInverse = mModelViewInverse * CameraPos;
	m_CameraInverse.normalize();

	//	Setup the light position in the VGP code.
	LightPos.x = 10000.0f;
	LightPos.y = 0.0f;
	LightPos.z = 0.0f;
	LightPos.w = 1.0f;

	m_LightInverse = mModelViewInverse * LightPos;
	m_LightInverse.normalize();
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
	return new OGLESMouse();
}

/******************************************************************************
 End of file (OGLESMouse.cpp)
******************************************************************************/

