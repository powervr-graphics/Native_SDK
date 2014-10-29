/******************************************************************************

 @File         OGLESIntroducingPOD.cpp

 @Title        Introducing the POD 3D file format

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows how to load POD files and play the animation with basic
               lighting

******************************************************************************/
#include <string.h>

#include "PVRShell.h"
#include "OGLESTools.h"

/******************************************************************************
 Content file names
******************************************************************************/

// Scene
/*
	The .pod file was exported from 3DSMax using PVRGeoPOD.
*/
const char c_szSceneFile[] = "IntroducingPOD.pod";


/******************************************************************************
 Defines
******************************************************************************/

/******************************************************************************
 Consts
******************************************************************************/
// Camera constants. Used for making the projection matrix
const float g_fCameraNear = 4.0f;
const float g_fCameraFar  = 5000.0f;

const float g_fDemoFrameRate = 1.0f / 30.0f;

// The camera to use from the pod file
const int g_ui32Camera = 0;

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLESIntroducingPOD : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// Vertex Buffer Object (VBO) handles
	GLuint*	m_puiVbo;
	GLuint*	m_puiIndexVbo;

	// 3D Model
	CPVRTModelPOD	m_Scene;

	// Projection and Model View matrices
	PVRTMat4		m_mProjection, m_mView;

	// Array to lookup the textures for each material in the scene
	GLuint*			m_puiTextureIDs;

	// Variables to handle the animation in a time-based manner
	unsigned long	m_ulTimePrev;
	float		m_fFrame;

public:
	OGLESIntroducingPOD() : m_puiVbo(0),
							m_puiIndexVbo(0),
							m_puiTextureIDs(0)
	{
	}

	// PVRShell functions
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	bool LoadVbos(CPVRTString* pErrorStr);
	void DrawMesh(unsigned int ui32MeshID);
	bool LoadTextures(CPVRTString* pErrorStr);
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
bool OGLESIntroducingPOD::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	/*
		Loads the scene from the .pod file into a CPVRTModelPOD object.
		We could also export the scene as a header file and
		load it with ReadFromMemory().
	*/

	if(m_Scene.ReadFromFile(c_szSceneFile) != PVR_SUCCESS)
	{
		CPVRTString ErrorStr = "ERROR: Couldn't load '" + CPVRTString(c_szSceneFile) + "'.";
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// The cameras are stored in the file. We check it contains at least one.
	if(m_Scene.nNumCamera == 0)
	{
		PVRShellSet(prefExitMessage, "ERROR: The scene does not contain a camera. Please add one to your scene and re-export.\n");
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
bool OGLESIntroducingPOD::QuitApplication()
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
bool OGLESIntroducingPOD::InitView()
{
	CPVRTString ErrorStr;
	/*
		Initialize Print3D
	*/
    bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	// Sets the clear color
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	// Enables texturing
	glEnable(GL_TEXTURE_2D);

	//	Initialize VBO data
	if(!LoadVbos(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	/*
		Load textures
	*/
	if(!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Enable the depth test
	glEnable(GL_DEPTH_TEST);

	// Enable culling
	glEnable(GL_CULL_FACE);

	// Initialise variables used for the animation
	m_fFrame = 0;
	m_ulTimePrev = PVRShellGetTime();
	return true;
}

/*!****************************************************************************
 @Function		LoadTextures
 @Output		pErrorStr		A CPVRTString describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course
******************************************************************************/
bool OGLESIntroducingPOD::LoadTextures(CPVRTString* pErrorStr)
{
	/*
		Loads the textures.
		For a more detailed explanation, see Texturing and IntroducingPVRTools
	*/

	/*
		Initialises an array to lookup the textures
		for each material in the scene.
	*/
	m_puiTextureIDs = new GLuint[m_Scene.nNumMaterial];

	if(!m_puiTextureIDs)
	{
		*pErrorStr = "ERROR: Insufficient memory.";
		return false;
	}

	for(int i = 0; i < (int) m_Scene.nNumMaterial; ++i)
	{
		m_puiTextureIDs[i] = 0;
		SPODMaterial* pMaterial = &m_Scene.pMaterial[i];

		if(pMaterial->nIdxTexDiffuse != -1)
		{
			/*
				Using the tools function PVRTTextureLoadFromPVR load the textures required by the pod file.

				Note: This function only loads .pvr files. You can set the textures in 3D Studio Max to .pvr
				files using the PVRTexTool plug-in for max. Alternatively, the pod material properties can be
				modified in PVRShaman.
			*/

			CPVRTString sTextureName = m_Scene.pTexture[pMaterial->nIdxTexDiffuse].pszName;

			if(PVRTTextureLoadFromPVR(sTextureName.c_str(), &m_puiTextureIDs[i]) != PVR_SUCCESS)
			{
				*pErrorStr = "ERROR: Failed to load " + sTextureName + ".";

				// Check to see if we're trying to load .pvr or not
				CPVRTString sFileExtension = PVRTStringGetFileExtension(sTextureName);

				if(sFileExtension.toLower() != "pvr")
					*pErrorStr += "Note: IntroducingPOD can only load pvr files.";

				return false;
			}
		}
	}

	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
bool OGLESIntroducingPOD::LoadVbos(CPVRTString* pErrorStr)
{
	if(m_Scene.nNumMesh == 0) // If there are no VBO to create return
		return true;

	if(!m_Scene.pMesh[0].pInterleaved)
	{
		*pErrorStr = "ERROR: IntroducingPOD requires the pod data to be interleaved. Please re-export with the interleaved option enabled.";
		return false;
	}

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
			uiSize = PVRTModelPODCountIndices(Mesh) * Mesh.sFaces.nStride;
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, Mesh.sFaces.pData, GL_STATIC_DRAW);
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLESIntroducingPOD::ReleaseView()
{
	// Deletes the textures
	glDeleteTextures(m_Scene.nNumMaterial, &m_puiTextureIDs[0]);

	// Frees the texture lookup array
	delete[] m_puiTextureIDs;
	m_puiTextureIDs = 0;

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
bool OGLESIntroducingPOD::RenderScene()
{
	// Clears the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/*
		Calculates the frame number to animate in a time-based manner.
		Uses the shell function PVRShellGetTime() to get the time in milliseconds.
	*/
	unsigned long ulTime = PVRShellGetTime();

	if(m_ulTimePrev > ulTime)
		m_ulTimePrev = ulTime;

	unsigned long ulDeltaTime = ulTime - m_ulTimePrev;
	m_ulTimePrev	= ulTime;
	m_fFrame += (float)ulDeltaTime * g_fDemoFrameRate;

	while(m_fFrame > m_Scene.nNumFrame-1)
		m_fFrame -= m_Scene.nNumFrame-1;

	// Sets the scene animation to this frame
	m_Scene.SetFrame(m_fFrame);

	// Setup the camera
	{
		PVRTVec3	vFrom, vTo(0.0f), vUp(0.0f, 1.0f, 0.0f);
		float	fFOV;

		// Camera nodes are after the mesh and light nodes in the array
		int i32CamID = m_Scene.pNode[m_Scene.nNumMeshNode + m_Scene.nNumLight + g_ui32Camera].nIdx;

		// Get the camera position, target and field of view (fov)
		if(m_Scene.pCamera[i32CamID].nIdxTarget != -1) // Does the camera have a target?
			fFOV = m_Scene.GetCameraPos( vFrom, vTo, g_ui32Camera); // vTo is taken from the target node
		else
			fFOV = m_Scene.GetCamera( vFrom, vTo, vUp, g_ui32Camera); // vTo is calculated from the rotation

		/*
			We can build the model view matrix from the camera position, target and an up vector.
			For this we use PVRTMa4::LookAtRH().
		*/
		m_mView = PVRTMat4::LookAtRH(vFrom, vTo, vUp);

		// Calculates the projection matrix
		bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);
		m_mProjection = PVRTMat4::PerspectiveFovRH(fFOV, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), g_fCameraNear, g_fCameraFar, PVRTMat4::OGL, bRotate);

		// Loads the projection matrix
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(m_mProjection.f);
	}

	// Specify the view matrix to OpenGL ES so we can specify the light in world space
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(m_mView.f);

	/*
		Load the light direction from the scene if we have one
	*/
	if(m_Scene.nNumLight != 0)
	{
		// Enables lighting. See BasicTnL for a detailed explanation
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);

		// Reads the light direction from the scene.
		PVRTVec4 vLightDirection;
		PVRTVec3 vPos;

		m_Scene.GetLight(vPos, *(PVRTVec3*)&vLightDirection, 0);
		vLightDirection.x = -vLightDirection.x;
		vLightDirection.y = -vLightDirection.y;
		vLightDirection.z = -vLightDirection.z;

		/*
			Set the w component to 0, so when passing it to glLight(), it is
			considered as a directional light (as opposed to a spot light).
		*/
		vLightDirection.w = 0;

		// Specify the light direction in world space
		glLightfv(GL_LIGHT0, GL_POSITION, (float*)&vLightDirection);
	}

	// Enable the vertex position attribute array
	glEnableClientState(GL_VERTEX_ARRAY);

	/*
		A scene is composed of nodes. There are 3 types of nodes:
		- MeshNodes :
			references a mesh in the pMesh[].
			These nodes are at the beginning of the pNode[] array.
			And there are nNumMeshNode number of them.
			This way the .pod format can instantiate several times the same mesh
			with different attributes.
		- lights
		- cameras
		To draw a scene, you must go through all the MeshNodes and draw the referenced meshes.
	*/
	for (unsigned int i = 0; i < m_Scene.nNumMeshNode; ++i)
	{
		SPODNode& Node = m_Scene.pNode[i];

		// Get the node model matrix
		PVRTMat4 mWorld;
		m_Scene.GetWorldMatrix(mWorld, Node);

		// Multiply the view matrix by the model (mWorld) matrix to get the model-view matrix
		PVRTMat4 mModelView;
		mModelView = m_mView * mWorld;

		glLoadMatrixf(mModelView.f);

		// Loads the correct texture using our texture lookup table
		if(!m_Scene.nNumMaterial || Node.nIdxMaterial == -1)
		{
			// It has no pMaterial defined. Use blank texture (0)
			glBindTexture(GL_TEXTURE_2D, 0);

			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,  PVRTVec4(1.0f).ptr());
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  PVRTVec4(1.0f).ptr());
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, m_puiTextureIDs[Node.nIdxMaterial]);

			// Load the meshes material properties
			SPODMaterial& Material = m_Scene.pMaterial[Node.nIdxMaterial];

			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,  PVRTVec4(Material.pfMatAmbient,  1.0f).ptr());
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  PVRTVec4(Material.pfMatDiffuse,  1.0f).ptr());
		}

		/*
			Now that the model-view matrix is set and the materials are ready,
			call another function to actually draw the mesh.
		*/
		DrawMesh(Node.nIdx);
	}

	// Disable the vertex positions
	glDisableClientState(GL_VERTEX_ARRAY);

	// Display the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("IntroducingPOD", "", ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			mesh		The mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the meterial prepared.
******************************************************************************/
void OGLESIntroducingPOD::DrawMesh(unsigned int ui32MeshID)
{
	SPODMesh& Mesh = m_Scene.pMesh[ui32MeshID];

	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[ui32MeshID]);
	// bind the index buffer, won't hurt if the handle is 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[ui32MeshID]);

	// Setup pointers
	glVertexPointer(Mesh.sVertex.n, GL_FLOAT, Mesh.sVertex.nStride, Mesh.sVertex.pData);

	if(Mesh.nNumUVW) // Do we have texture co-ordinates?
	{
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(Mesh.psUVW[0].n, GL_FLOAT, Mesh.psUVW[0].nStride, Mesh.psUVW[0].pData);
	}

	if(Mesh.sNormals.n) // Do we have normals?
	{
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, Mesh.sNormals.nStride, Mesh.sNormals.pData);
	}

	if(Mesh.sVtxColours.n) // Do we have vertex colours?
	{
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(Mesh.sVtxColours.n * PVRTModelPODDataTypeComponentCount(Mesh.sVtxColours.eType), GL_UNSIGNED_BYTE, Mesh.sVtxColours.nStride, Mesh.sVtxColours.pData);
	}
	/*
		The geometry can be exported in 4 ways:
		- Indexed Triangle list
		- Non-Indexed Triangle list
		- Indexed Triangle strips
		- Non-Indexed Triangle strips
	*/
	if(Mesh.nNumStrips == 0)
	{
		if(m_puiIndexVbo[ui32MeshID])
		{
			// Indexed Triangle list
			glDrawElements(GL_TRIANGLES, Mesh.nNumFaces * 3, GL_UNSIGNED_SHORT, 0);
		}
		else
		{
			// Non-Indexed Triangle list
			glDrawArrays(GL_TRIANGLES, 0, Mesh.nNumFaces * 3);
		}
	}
	else
	{
		int offset = 0;

		for(int i = 0; i < (int) Mesh.nNumStrips; ++i)
		{
			if(m_puiIndexVbo[ui32MeshID])
			{
				// Indexed Triangle strips
				glDrawElements(GL_TRIANGLE_STRIP, Mesh.pnStripLength[i]+2, GL_UNSIGNED_SHORT, (void*) (offset * sizeof(GLushort)));
			}
			else
			{
				// Non-Indexed Triangle strips
				glDrawArrays(GL_TRIANGLE_STRIP, offset, Mesh.pnStripLength[i]+2);
			}
			offset += Mesh.pnStripLength[i]+2;
		}
	}

	// unbind the vertex buffers as we don't need them bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Disable the vertex attribute arrays
	if(Mesh.nNumUVW) // Do we have texture co-ordinates?
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if(Mesh.sNormals.n) // Do we have normals?
		glDisableClientState(GL_NORMAL_ARRAY);

	if(Mesh.sVtxColours.n) // Do we have vertex colours?
		glDisableClientState(GL_COLOR_ARRAY);
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
	return new OGLESIntroducingPOD();
}

/******************************************************************************
 End of file (OGLESIntroducingPOD.cpp)
******************************************************************************/

