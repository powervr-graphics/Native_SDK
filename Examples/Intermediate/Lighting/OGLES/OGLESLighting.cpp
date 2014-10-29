/******************************************************************************

 @File         OGLESLighting.cpp

 @Title        Lighting

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows how to use multiple lights in OpenGL ES.

******************************************************************************/

/****************************************************************************
** Includes
****************************************************************************/
#include <math.h>
#include "PVRShell.h"
#include "OGLESTools.h"

/******************************************************************************
 Content file names
******************************************************************************/

// PVR texture files
const char c_szLightTexFile[]	= "LightTex.pvr";
const char c_szStoneTexFile[]	= "Stone.pvr";

// POD file
const char c_szSceneFile[] = "LightingScene.pod";

/****************************************************************************
** Defines
****************************************************************************/
const unsigned int g_ui32LightNo = 8;

/****************************************************************************
** Structures
****************************************************************************/
struct SLightVars
{
	PVRTVec4	Position;	// GL_LIGHT_POSITION
	PVRTVec4	Direction;	// GL_SPOT_DIRECTION
	PVRTVec4	Ambient;	// GL_AMBIENT
	PVRTVec4	Diffuse;	// GL_DIFFUSE
	PVRTVec4	Specular;	// GL_SPECULAR

	PVRTVec3	vRotationStep;
	PVRTVec3	vRotationCentre;
	PVRTVec3	vRotation;
	PVRTVec3	vPosition;
};


/****************************************************************************
** Class: OGLESLighting
****************************************************************************/
class OGLESLighting : public PVRShell
{
private:

	// Print3D class
	CPVRTPrint3D 	m_Print3D;

	// 3D Model
	CPVRTModelPOD	m_Scene;

	// OpenGL handles for textures and VBOs
	GLuint m_ui32Stone;
	GLuint m_ui32Light;

	GLuint*	m_puiVbo;
	GLuint*	m_puiIndexVbo;

	// Light properties
	SLightVars m_psLightData[8];

	// Number of frames
	int m_i32FrameNo;

	// Array to lookup the textures for each material in the scene
	GLuint*	m_pui32Textures;

public:
	OGLESLighting() : 	m_puiVbo(0),
						m_puiIndexVbo(0),
						m_i32FrameNo(0)
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
	void InitLight(SLightVars &Light);
	void StepLight(SLightVars &Light);
	void DrawLight(SLightVars &Light);
	void LoadVbos();
	bool LoadTextures(CPVRTString* const pErrorStr);
	void DrawMesh(unsigned int ui32MeshID);
};


/*******************************************************************************
 * Function Name  : InitApplication
 * Inputs		  : argc, *argv[], uWidth, uHeight
 * Returns        : true if no error occured
 * Description    : Code in InitApplication() will be called by the Shell ONCE per
 *					run, early on in the execution of the program.
 *					Used to initialize variables that are not dependant on the
 *					rendering context (e.g. external modules, loading meshes, etc.)
 *******************************************************************************/
bool OGLESLighting::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*) PVRShellGet(prefReadPath));

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

/*******************************************************************************
 * Function Name  : QuitApplication
 * Returns        : true if no error occured
 * Description    : Code in QuitApplication() will be called by the Shell ONCE per
 *					run, just before exiting the program.
 *******************************************************************************/
bool OGLESLighting::QuitApplication()
{
	// Free the memory allocated for the scene
	m_Scene.Destroy();

	delete[] m_puiVbo;
	delete[] m_puiIndexVbo;

	return true;
}

/*******************************************************************************
 * Function Name  : InitView
 * Inputs		  : uWidth, uHeight
 * Returns        : true if no error occured
 * Description    : Code in InitView() will be called by the Shell upon a change
 *					in the rendering context.
 *					Used to initialize variables that are dependant on the rendering
 *					context (e.g. textures, vertex buffers, etc.)
 *******************************************************************************/
bool OGLESLighting::InitView()
{
	CPVRTString ErrorStr;
	SPVRTContext sContext;
	PVRTMat4	mProjection;
	int			i;

	// Is the screen rotated?
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	// Initialize Print3D textures
	if(m_Print3D.SetTextures(&sContext, PVRShellGet(prefWidth), PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	// Load textures
	if(!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	//	Initialize VBO data
	LoadVbos();

	// Setup all materials
	float Ambient[]	= {0.1f, 0.1f, 0.1f, 1.0f};
	float Diffuse[]	= {0.5f, 0.5f, 0.5f, 1.0f};
	float Specular[]= {1.0f, 1.0f, 1.0f, 1.0f};

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10.0f);	// Nice and shiny so we don't get aliasing from the 1/2 angle

	// Initialize all lights
	srand(0);

	for(i = 0; i < 8; ++i)
		InitLight(m_psLightData[i]);

	// Perspective matrix
	
	// Calculate the projection matrix
	mProjection = PVRTMat4::PerspectiveFovRH(20.0f*(PVRT_PIf/180.0f), (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), 10.0f, 1200.0f, PVRTMat4::OGL, bRotate);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(mProjection.f);

	// Modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -500.0f);

	// Setup culling
	glEnable(GL_CULL_FACE);

	// Enable texturing
	glEnable(GL_TEXTURE_2D);

	/*
		Build an array to map the textures within the pod file
		to the textures we loaded earlier.
	*/

	m_pui32Textures = new GLuint[m_Scene.nNumMaterial];

	for(i = 0; i < (int) m_Scene.nNumMaterial; ++i)
	{
		m_pui32Textures[i] = 0;
		SPODMaterial* pMaterial = &m_Scene.pMaterial[i];

		if(!strcmp(pMaterial->pszName, "Stone"))
			m_pui32Textures[i] = m_ui32Stone;
	}

	// Set the clear colour
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLESLighting::LoadVbos()
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
bool OGLESLighting::LoadTextures(CPVRTString* const pErrorStr)
{
	if(PVRTTextureLoadFromPVR(c_szStoneTexFile, &m_ui32Stone) != PVR_SUCCESS)
	{
		*pErrorStr = CPVRTString("ERROR: Failed to load texture ") + c_szStoneTexFile;
		return false;
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szLightTexFile, &m_ui32Light) != PVR_SUCCESS)
	{
		*pErrorStr = CPVRTString("ERROR: Failed to load texture ") + c_szLightTexFile;
		return false;
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return true;
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns        : Nothing
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool OGLESLighting::ReleaseView()
{
	// Release textures
	glDeleteTextures(1, &m_ui32Stone);
	glDeleteTextures(1, &m_ui32Light);

	delete[] m_pui32Textures;

	// Release Print3D textures
	m_Print3D.ReleaseTextures();
	return true;
}

/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool OGLESLighting::RenderScene()
{
	unsigned int i;
	PVRTMat4 RotationMatrix;

	// Clear the buffers
	glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Lighting

	// Enable lighting (needs to be specified every frame as Print3D will turn it off
	glEnable(GL_LIGHTING);

	// Increase number of frames
	++m_i32FrameNo;
	m_i32FrameNo = m_i32FrameNo % 3600;

	RotationMatrix = PVRTMat4::RotationY((-m_i32FrameNo * 0.1f) * PVRT_PIf / 180.0f);
	
	// Loop through all lights
	for(i = 0; i < 8; ++i)
	{
		// Only process lights that we are actually using
		if(i < g_ui32LightNo)
		{
			// Transform light
			StepLight(m_psLightData[i]);

			// Set light properties
			glLightfv(GL_LIGHT0 + i, GL_POSITION, &m_psLightData[i].Position.x);
			glLightfv(GL_LIGHT0 + i, GL_AMBIENT,  &m_psLightData[i].Ambient.x);
			glLightfv(GL_LIGHT0 + i, GL_DIFFUSE,  &m_psLightData[i].Diffuse.x);
			glLightfv(GL_LIGHT0 + i, GL_SPECULAR, &m_psLightData[i].Specular.x);

			// Enable light
			glEnable(GL_LIGHT0 + i);
		}
		else
		{
			// Disable remaining lights
			glDisable(GL_LIGHT0 + i);
		}
	}

	// Draw Scene

	// Enable client states
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// Save matrix by pushing it on the stack
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	// Add a small Y rotation to the model
	glMultMatrixf(RotationMatrix.f);

	// Loop through and draw all meshes
	for(i = 0; i < m_Scene.nNumMeshNode; ++i)
	{
		SPODNode& Node = m_Scene.pNode[i];

		// Loads the correct texture using our texture lookup table
		glBindTexture(GL_TEXTURE_2D, m_pui32Textures[Node.nIdxMaterial]);

		DrawMesh(Node.nIdx);
	}

	// Disable normals as the light quads do not have any
	glDisableClientState(GL_NORMAL_ARRAY);

	// Restore matrix
	glPopMatrix();

	// draw lights

	// No lighting for lights
	glDisable(GL_LIGHTING);

	// Disable Z writes
	glDepthMask(GL_FALSE);

	// Set additive blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_ONE);

	// Set texture and texture environment
	glBindTexture(GL_TEXTURE_2D, m_ui32Light);

	// Render all lights in use
	for(i = 0; i < g_ui32LightNo; ++i)
		DrawLight(m_psLightData[i]);

	// Disable blending
	glDisable(GL_BLEND);

	// Restore Z writes
	glDepthMask(GL_TRUE);

	// Disable client states
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	// Display info text
	m_Print3D.DisplayDefaultTitle("Lighting", "8 point lights", ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}

/*******************************************************************************
 * Function Name  : InitLight
 * Inputs		  : *pLight
 * Description    : Initialize light structure
 *******************************************************************************/
void OGLESLighting::InitLight(SLightVars &Light)
{
	// Light Ambient colour
	Light.Ambient.x = 0.0;
	Light.Ambient.y = 0.0;
	Light.Ambient.z = 0.0;
	Light.Ambient.w = 1.0;

	// Light Diffuse colour
	double difFac = 0.4;
	Light.Diffuse.x = (float)( difFac * (rand()/(double)RAND_MAX) ) * 2.0f; //1.0;
	Light.Diffuse.y = (float)( difFac * (rand()/(double)RAND_MAX) ) * 2.0f; //1.0;
	Light.Diffuse.z = (float)( difFac * (rand()/(double)RAND_MAX) ) * 2.0f; //1.0;
	Light.Diffuse.w = 1.0f;

	// Light Specular colour
	double specFac = 0.1;
	Light.Specular.x = (float)( specFac * (rand()/(double)RAND_MAX) ) * 2.0f; //1.0;
	Light.Specular.y = (float)( specFac * (rand()/(double)RAND_MAX) ) * 2.0f; //1.0;
	Light.Specular.z = (float)( specFac * (rand()/(double)RAND_MAX) ) * 2.0f; //1.0;
	Light.Specular.w = 1.0f;

	// Randomize some of the other parameters
	float lightDist = 80.0f;
	Light.vPosition.x = (float)((rand()/(double)RAND_MAX) * lightDist/2.0f ) + lightDist/2.0f;
	Light.vPosition.y = (float)((rand()/(double)RAND_MAX) * lightDist/2.0f ) + lightDist/2.0f;
	Light.vPosition.z = (float)((rand()/(double)RAND_MAX) * lightDist/2.0f ) + lightDist/2.0f;

	float rStep = 2;
	Light.vRotationStep.x = (float)( rStep/2.0 - (rand()/(double)RAND_MAX) * rStep );
	Light.vRotationStep.y = (float)( rStep/2.0 - (rand()/(double)RAND_MAX) * rStep );
	Light.vRotationStep.z = (float)( rStep/2.0 - (rand()/(double)RAND_MAX) * rStep );

	Light.vRotation.x = 0.0f;
	Light.vRotation.y = 0.0f;
	Light.vRotation.z = 0.0f;

	Light.vRotationCentre.x = 0.0f;
	Light.vRotationCentre.y = 0.0f;
	Light.vRotationCentre.z = 0.0f;
}

/*******************************************************************************
 * Function Name  : StepLight
 * Inputs		  : *pLight
 * Description    : Advance one step in the light rotation.
 *******************************************************************************/
void OGLESLighting::StepLight(SLightVars &Light)
{
	PVRTMat4 RotationMatrix, RotationMatrixX, RotationMatrixY, RotationMatrixZ;

	// Increase rotation angles
	Light.vRotation.x += Light.vRotationStep.x;
	Light.vRotation.y += Light.vRotationStep.y;
	Light.vRotation.z += Light.vRotationStep.z;

	while(Light.vRotation.x > 360.0f) Light.vRotation.x -= 360.0f;
	while(Light.vRotation.y > 360.0f) Light.vRotation.y -= 360.0f;
	while(Light.vRotation.z > 360.0f) Light.vRotation.z -= 360.0f;

	// Create three rotations from rotation angles
	RotationMatrixX = PVRTMat4::RotationX(Light.vRotation.x * (PVRT_PIf / 180.0f));
	RotationMatrixY = PVRTMat4::RotationY(Light.vRotation.y * (PVRT_PIf / 180.0f));
	RotationMatrixZ = PVRTMat4::RotationZ(Light.vRotation.z * (PVRT_PIf / 180.0f));

	// Build transformation matrix by concatenating all rotations
	RotationMatrix =  RotationMatrixZ * RotationMatrixY * RotationMatrixX;

	// Transform light with transformation matrix, setting w to 1 to indicate point light
	PVRTTransformArray((PVRTVec3*)&Light.Position, &Light.vPosition, 1, &RotationMatrix);

	Light.Position.w = 1.0f;
}

/*******************************************************************************
 * Function Name  : DrawLight
 * Inputs		  : *pLight
 * Description    : Draw every light as a quad.
 *******************************************************************************/
void OGLESLighting::DrawLight(SLightVars &Light)
{
	float	quad_verts[4 * 4];
	
	// Set Quad UVs
	float	quad_uvs[2 * 4] = {0, 0,
								1, 0,
								0, 1,
								1, 1};

	float	fLightSize = 5.0f;

	// Set quad vertices
	quad_verts[0]  = Light.Position.x - fLightSize;
	quad_verts[1]  = Light.Position.y - fLightSize;
	quad_verts[2]  = Light.Position.z;

	quad_verts[3]  = Light.Position.x + fLightSize;
	quad_verts[4]  = Light.Position.y - fLightSize;
	quad_verts[5]  = Light.Position.z;

	quad_verts[6]  = Light.Position.x - fLightSize;
	quad_verts[7]  = Light.Position.y + fLightSize;
	quad_verts[8]  = Light.Position.z;

	quad_verts[9]  = Light.Position.x + fLightSize;
	quad_verts[10] = Light.Position.y + fLightSize;
	quad_verts[11] = Light.Position.z;

	// Set data
	glVertexPointer(3, GL_FLOAT, 0, quad_verts);
	glTexCoordPointer(2, GL_FLOAT, 0, quad_uvs);

	// Set light colour 2x overbright for more contrast (will be modulated with texture)
	glColor4f(Light.Diffuse.x * 2.0f, Light.Diffuse.y * 2.0f, Light.Diffuse.z * 2.0f, 1.0f);

	// Draw quad
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			ID of mesh to draw
 @Description	Draws a mesh.
******************************************************************************/
void OGLESLighting::DrawMesh(unsigned int ui32MeshID)
{
	SPODMesh& Mesh = m_Scene.pMesh[ui32MeshID];

	// Bind the vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[ui32MeshID]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[ui32MeshID]);

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
 * Function Name  : NewDemo
 * Description    : Called by the Shell to initialize a new instance to the
 *					demo class.
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new OGLESLighting;
}

/*****************************************************************************
 End of file (OGLESLighting.cpp)
*****************************************************************************/

