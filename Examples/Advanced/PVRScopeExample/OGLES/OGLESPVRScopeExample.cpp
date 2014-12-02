/******************************************************************************

 @File         OGLESPVRScopeExample.cpp

 @Title        Demonstrates how to use the example PVRScope graphing code

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Demonstrates how to use the example PVRScope graphing code

******************************************************************************/
#include <string.h>

#include "PVRShell.h"
#include "OGLESTools.h"
#include "PVRScopeGraph.h"

#if !defined(_WIN32) || defined(__WINSCW__)
#define _stricmp strcasecmp
#endif

/******************************************************************************
 Content file names
******************************************************************************/

// Scene
const char c_szSceneFile[] = "Mask.pod";

// PVR texture files
const char c_szTextureFile[] = "MaskTex.pvr";

/******************************************************************************
 Defines
******************************************************************************/
// Camera constants. Used for making the projection matrix
#define CAM_NEAR	(1.0f)
#define CAM_FAR		(500.0f)

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLESPVRScopeExample : public PVRShell
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
	GLuint m_uiTexture;

	// Variables to handle the animation in a time-based manner
	int			m_iTimePrev;
	float		m_fFrame;

	// The rotate parameter of Model
	float m_fAngleY;

	// The PVRScopeGraph variable
	CPVRScopeGraph *m_pScopeGraph;

	// Variables for the graphing code
	int m_i32Counter;
	int m_i32Group;
	int m_i32Interval;

public:
	OGLESPVRScopeExample() : m_puiVbo(0),
							m_puiIndexVbo(0),
							m_fAngleY(0),
							m_i32Counter(0)
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
bool OGLESPVRScopeExample::InitApplication()
{
	// At the time of writing, this counter is the USSE load for vertex + pixel processing
	m_i32Counter	= 46;
	m_i32Group		= 0;
	m_i32Interval	= 0;

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

	// Process the command line
	{
		const unsigned int	nOptNum			= PVRShellGet(prefCommandLineOptNum);
		const SCmdLineOpt	* const psOpt	= (const SCmdLineOpt*)PVRShellGet(prefCommandLineOpts);
		for(unsigned int i = 0; i < nOptNum; ++i)
		{
			if(_stricmp(psOpt[i].pArg, "-counter") == 0 && psOpt[i].pVal)
			{
				m_i32Counter = atoi(psOpt[i].pVal);
			}
			else if(_stricmp(psOpt[i].pArg, "-group") == 0 && psOpt[i].pVal)
			{
				m_i32Group = atoi(psOpt[i].pVal);
			}
			else if(_stricmp(psOpt[i].pArg, "-interval") == 0 && psOpt[i].pVal)
			{
				m_i32Interval = atoi(psOpt[i].pVal);
			}
		}
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
bool OGLESPVRScopeExample::QuitApplication()
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
bool OGLESPVRScopeExample::InitView()
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

	/*
		Calculate the projection and view matrices
	*/

	m_mProjection = PVRTMat4::PerspectiveFovRH(PVRT_PIf/6, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), CAM_NEAR, CAM_FAR, PVRTMat4::OGL, bRotate);

	m_mView = PVRTMat4::LookAtRH(PVRTVec3(0, 0, 75.0f), PVRTVec3(0, 0, 0), PVRTVec3(0, 1, 0));

	// Enable the depth test
	glEnable(GL_DEPTH_TEST);

	// Enable culling
	glEnable(GL_CULL_FACE);

	// Initialise variables used for the animation
	m_fFrame = 0;
	m_iTimePrev = PVRShellGetTime();

	// Initialise the graphing code
	m_pScopeGraph = new CPVRScopeGraph();

	if(m_pScopeGraph)
	{
		// Position the graph
		m_pScopeGraph->position(PVRShellGet(prefWidth), PVRShellGet(prefHeight), (int) (PVRShellGet(prefWidth) * 0.02f), (int) (PVRShellGet(prefHeight) * 0.02f), (int) (PVRShellGet(prefWidth) * 0.96f), (int) (PVRShellGet(prefHeight) * 0.96f) / 3);

		// Output the current active group and a list of all the counters
		PVRShellOutputDebug("Active Group %i\nCounter Number %i\n", m_pScopeGraph->GetActiveGroup(), m_pScopeGraph->GetCounterNum());
		PVRShellOutputDebug("Counters\n");

		for(unsigned int i = 0; i < m_pScopeGraph->GetCounterNum(); ++i)
		{
			PVRShellOutputDebug("(%i) Name %s Group %i %s\n", i,
				m_pScopeGraph->GetCounterName(i), m_pScopeGraph->GetCounterGroup(i),
				m_pScopeGraph->IsCounterPercentage(i) ? "percentage" : "absolute");
			m_pScopeGraph->ShowCounter(i, false);
		}

		// Set the active group to 0
		m_pScopeGraph->SetActiveGroup(m_i32Group);

		// Tell the graph to show an initial counter
		m_pScopeGraph->ShowCounter(m_i32Counter, true);

		// Set the update interval: number of updates [frames] before updating the graph
		m_pScopeGraph->SetUpdateInterval(m_i32Interval);
	}

	return true;
}

/*!****************************************************************************
 @Function		LoadTextures
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course
******************************************************************************/
bool OGLESPVRScopeExample::LoadTextures(CPVRTString* pErrorStr)
{
	if(PVRTTextureLoadFromPVR(c_szTextureFile, &m_uiTexture) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Failed to load texture.";
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
bool OGLESPVRScopeExample::LoadVbos(CPVRTString* pErrorStr)
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
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLESPVRScopeExample::ReleaseView()
{
	// Deletes the texture
	glDeleteTextures(1, &m_uiTexture);

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

	if(m_pScopeGraph)
	{
		delete m_pScopeGraph;
		m_pScopeGraph = 0;
	}

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
bool OGLESPVRScopeExample::RenderScene()
{
	// Keyboard input (cursor up/down to cycle through counters)
	if(PVRShellIsKeyPressed(PVRShellKeyNameUP))
	{
		m_i32Counter++;

		if(m_i32Counter > (int) m_pScopeGraph->GetCounterNum())
			m_i32Counter = m_pScopeGraph->GetCounterNum();
	}

	if(PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
	{
		m_i32Counter--;

		if(m_i32Counter < 0)
			m_i32Counter = 0;
	}

	if(PVRShellIsKeyPressed(PVRShellKeyNameACTION2))
		m_pScopeGraph->ShowCounter(m_i32Counter, !m_pScopeGraph->IsCounterShown(m_i32Counter));

	// Keyboard input (cursor left/right to change active group)
	if(PVRShellIsKeyPressed(PVRShellKeyNameRIGHT))
	{
		m_pScopeGraph->SetActiveGroup(m_pScopeGraph->GetActiveGroup()+1);
	}

	if(PVRShellIsKeyPressed(PVRShellKeyNameLEFT))
	{
		m_pScopeGraph->SetActiveGroup(m_pScopeGraph->GetActiveGroup()-1);
	}

	// Clears the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Loads the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(m_mProjection.f);

	// Specify the modelview matrix
	PVRTMat4 mModel;
	SPODNode& Node = m_Scene.pNode[0];

	m_Scene.GetWorldMatrix(mModel, Node);

	// Rotate and Translate the model matrix
	m_fAngleY += (2*PVRT_PIf/60)/7;

	// Set model view projection matrix
	PVRTMat4 mModelView;
	mModelView = m_mView * PVRTMat4::RotationY(m_fAngleY) * mModel;

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(mModelView.f);

	/*
		Load the light direction from the scene if we have one
	*/

	// Enables lighting. See BasicTnL for a detailed explanation
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// Set light direction
	PVRTVec4 vLightDirModel;
	vLightDirModel = mModel.inverse() * PVRTVec4(1, 1, 1, 0);
	glLightfv(GL_LIGHT0, GL_POSITION, (float*)&vLightDirModel.x);

	// Enable the vertex position attribute array
	glEnableClientState(GL_VERTEX_ARRAY);

	// bind the texture
	glBindTexture(GL_TEXTURE_2D, m_uiTexture);

	/*
		Now that the model-view matrix is set and the materials are ready,
		call another function to actually draw the mesh.
	*/
	DrawMesh(Node.nIdx);

	// Disable the vertex positions
	glDisableClientState(GL_VERTEX_ARRAY);

	char Description[256];

	if(m_pScopeGraph->GetCounterNum())
	{
		sprintf(Description, "Active Grp %i\n\nCounter %i (Grp %i) \nName: %s\nShown: %s\nuser y-axis: %.2f  max: %.2f%s",
			m_pScopeGraph->GetActiveGroup(), m_i32Counter,
			m_pScopeGraph->GetCounterGroup(m_i32Counter),
			m_pScopeGraph->GetCounterName(m_i32Counter),
			m_pScopeGraph->IsCounterShown(m_i32Counter) ? "Yes" : "No",
			m_pScopeGraph->GetMaximum(m_i32Counter),
			m_pScopeGraph->GetMaximumOfData(m_i32Counter),
			m_pScopeGraph->IsCounterPercentage(m_i32Counter) ? "%%" : "");
	}
	else
	{
		sprintf(Description, "No counters present");
	}

	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("PVRScopeExample", Description, ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	// Update counters and draw the graph
	m_pScopeGraph->Ping();

	return true;
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			mesh		The mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the material prepared.
******************************************************************************/
void OGLESPVRScopeExample::DrawMesh(unsigned int ui32MeshID)
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
	return new OGLESPVRScopeExample();
}

/******************************************************************************
 End of file (OGLESPVRScopeExample.cpp)
******************************************************************************/

