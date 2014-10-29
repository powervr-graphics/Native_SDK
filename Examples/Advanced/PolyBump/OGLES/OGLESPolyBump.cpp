/******************************************************************************

 @File         OGLESPolyBump.cpp

 @Title        Demonstrates DOT3 lighting

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Per-pixel lighting done using Dot3 bumpmapping. A very complex
               model has been computed in the normal map for Dot3, that will be
               applied to a very low polygon count model. The model used in this
               demo is one of the free models suplied by Crytek
               (http://www.crytek.com/polybump). Currently there are several
               companies supliying tools and plug-ins to compute these maps.

******************************************************************************/

// Includes
#include "PVRShell.h"
#include "OGLESTools.h"

/******************************************************************************
 Content file names
******************************************************************************/

// PVR texture files
const char c_szHeadCloneSpacePVRTCTexFile[]	= "Head_clonespacePVRTC.pvr";
const char c_szHeadCloneSpaceBGRATexFile[]	= "Head_clonespaceBGRA.pvr"; // A BGRA version only supported on some devices
const char c_szHeadDiffuseTexFile[]			= "Head_diffuse.pvr";

// POD files
const char c_szSceneFile[] = "Head.pod";

/******************************************************************************
 Consts
******************************************************************************/
const char c_szDescriptionBGRA[] = "DOT3 per-pixel lighting using a BGRA texture";
const char c_szDescriptionPVRTC[]= "DOT3 per-pixel lighting using a PVRTC texture";
/******************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLESPolybump : public PVRShell
{
	// Print3D class
	CPVRTPrint3D 	m_Print3D;

	// 3D Model
	CPVRTModelPOD	m_Scene;

	// OpenGL handles for textures and VBOs
	GLuint m_ui32DiffuseMap;
	GLuint m_ui32CloneMap;

	GLuint*	m_puiVbo;
	GLuint*	m_puiIndexVbo;

	// Dynamic data
	int				m_i32Frame;
	PVRTMat4		m_mView;
	PVRTMat4		m_mProjection;

	// bools to indicate support for the GL_ARB_texture_env_combine and GL_IMG_texture_env_enhanced_fixed_function extensions
	bool  m_bCombinersPresent;
	bool  m_bIMGTextureFFExtPresent;

	// A bool to indicate support for the GL_IMG_texture_format_BGRA8888 extension
	bool  m_bBGRASupported;

	// A bool to specifiy whether to draw with Dot3 or not
	bool m_bDrawWithDot3;

	// A pointer to the description for print3D when Dot3 is being used
	const char* m_pDescription;

public:
	OGLESPolybump()  :	m_ui32DiffuseMap(0),
						m_ui32CloneMap(0),
						m_puiVbo(0),
						m_puiIndexVbo(0),
						m_i32Frame(0),
						m_bCombinersPresent(false),
						m_bIMGTextureFFExtPresent(false),
						m_bDrawWithDot3(true)
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
	void CalculateDot3LightDirection(PVRTVec4 Dot3LightPos);
	void DrawMesh(unsigned int ui32MeshID);
	bool LoadTextures(CPVRTString* const pErrorStr);
	void LoadVbos();
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
bool OGLESPolybump::InitApplication()
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

/*******************************************************************************
 * Function Name  : QuitApplication
 * Returns        : true if no error occured
 * Description    : Code in QuitApplication() will be called by the Shell ONCE per
 *					run, just before exiting the program.
 *******************************************************************************/
bool OGLESPolybump::QuitApplication()
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
bool OGLESPolybump::InitView()
{
	CPVRTString  ErrorStr;
	SPVRTContext sContext;

	// Is the screen rotated?
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	// Initialise Print3D textures
	if(m_Print3D.SetTextures(&sContext, PVRShellGet(prefWidth), PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "Error: Failed to initialise Print3D\n");
		return false;
	}

	// Load textures
	if(!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	//	Initialise VBO data
	LoadVbos();

	// Retrieve OpenGL ES driver version
	int		i32OGLESVersionMajor = 1, i32OGLESVersionMinor = 0;
	char	*pszVersionNumber;

	pszVersionNumber = (char*) strchr((const char *)glGetString(GL_VERSION), '.');

	if(pszVersionNumber)
	{
		i32OGLESVersionMajor = pszVersionNumber[-1] - '0';
		i32OGLESVersionMinor = pszVersionNumber[+1] - '0';
	}

	// Check for support of required extensions
	if(i32OGLESVersionMajor > 1 || (i32OGLESVersionMajor == 1 && i32OGLESVersionMinor > 0))
	{
		m_bCombinersPresent = true;
	}
	else
	{
        m_bCombinersPresent = CPVRTglesExt::IsGLExtensionSupported("GL_ARB_texture_env_combine");

		if(!m_bCombinersPresent)
		{
			m_bIMGTextureFFExtPresent = CPVRTglesExt::IsGLExtensionSupported("GL_IMG_texture_env_enhanced_fixed_function");

			if(!m_bIMGTextureFFExtPresent)
			{
				PVRShellSet(prefExitMessage, "Error: Can't run this demo without support for GL_ARB_texture_env_combine or GL_IMG_texture_env_enhanced_fixed_function.\n");
				return false;
			}
		}
	}

	// Calculates the projection matrix
	m_mProjection = PVRTMat4::PerspectiveFovRH(30.0f*(3.14f/180.0f), (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), 10.0f, 8000.0f, PVRTMat4::OGL, bRotate);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(m_mProjection.f);

	// Set up the view matrix from the camera's position, taget and up vector using PVRTMatrixLookAtRH.
	PVRTVec3	vFrom, vTo, vUp(0.0f,1.0f,0.0f);

	// We can get the camera position, target with GetCameraPos()
	m_Scene.GetCameraPos( vFrom, vTo, 0);
	m_mView = PVRTMat4::LookAtRH(vFrom, vTo, vUp);

	// setup clear colour
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Enable states
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLESPolybump::LoadVbos()
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
bool OGLESPolybump::LoadTextures(CPVRTString* const pErrorStr)
{

	// Load Textures

	/*
		First we check for support of the GL_IMG_texture_format_BGRA8888 extension to determine which
		normal map to use. If it is supported then we will be using the BGRA format that is 32bits per
		pixel avoiding any artefacts related to compression. When using a normal map any artefacts
		present will be clearly visible as they will affect the normal directions. If the extension
		is unsupported we will be using the PVRTC compressed version.

		In general for performance using a 32bit texture is not recommended but in the case
		of a normal map maximum quality is required.
	*/

	m_bBGRASupported = CPVRTglesExt::IsGLExtensionSupported("GL_IMG_texture_format_BGRA8888");

	if(m_bBGRASupported)
	{
		if(PVRTTextureLoadFromPVR(c_szHeadCloneSpaceBGRATexFile, &m_ui32CloneMap) != PVR_SUCCESS)
		{
			*pErrorStr = CPVRTString("ERROR: Failed to load texture ") + c_szHeadCloneSpaceBGRATexFile;
			return false;
		}

		m_pDescription = &c_szDescriptionBGRA[0];
	}
	else
	{
		if(PVRTTextureLoadFromPVR(c_szHeadCloneSpacePVRTCTexFile, &m_ui32CloneMap) != PVR_SUCCESS)
		{
			*pErrorStr = CPVRTString("ERROR: Failed to load texture ") + c_szHeadCloneSpacePVRTCTexFile;
			return false;
		}

		m_pDescription = &c_szDescriptionPVRTC[0];
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szHeadDiffuseTexFile, &m_ui32DiffuseMap) != PVR_SUCCESS)
	{
		*pErrorStr = CPVRTString("ERROR: Failed to load texture ") + c_szHeadDiffuseTexFile;
		return false;
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return true;
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns        : Nothing
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool OGLESPolybump::ReleaseView()
{
	// Release textures
	m_Print3D.ReleaseTextures();

	glDeleteTextures(1, &m_ui32CloneMap);
	glDeleteTextures(1, &m_ui32DiffuseMap);
	return true;
}


/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool OGLESPolybump::RenderScene()
{
	if(PVRShellIsKeyPressed(PVRShellKeyNameACTION1))
		m_bDrawWithDot3 = !m_bDrawWithDot3;

	PVRTVec4 LightVector;
	PVRTMat4 mRotateY, mModelView;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	LightVector.x = PVRTSIN(m_i32Frame / 40.0f);
	LightVector.y = 0.0f;
	LightVector.z = -PVRTABS(PVRTCOS(m_i32Frame / 40.0f));
	LightVector.w = 0.0f;

	PVRTTransformBack(&LightVector, &LightVector, &m_mView);

	// Normalize light vector in case it is not
	LightVector.normalize();

	if(m_bDrawWithDot3)
	{
		// Setup texture blend modes

		// First layer (Dot3)
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_ui32CloneMap);

		if(m_bCombinersPresent)
		{
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGB);
			glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
			glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
		}
		else if(m_bIMGTextureFFExtPresent)
		{
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DOT3_RGBA);
		}

		// Second layer (modulate)
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, m_ui32DiffuseMap);

		if(m_bCombinersPresent)
		{
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
			glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
		}
		else if (m_bIMGTextureFFExtPresent)
		{
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		}

		// Calculate Dot3 light direction
		CalculateDot3LightDirection(LightVector);
	}
	else
	{
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, m_ui32DiffuseMap);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);

		LightVector.z = -LightVector.z;
		glLightfv(GL_LIGHT0, GL_POSITION, &LightVector.x);
	}

	glMatrixMode(GL_MODELVIEW);

	// Render mesh
	SPODNode& Node = m_Scene.pNode[0];

	// Rotate the mesh around a point
	mModelView = m_mView * PVRTMat4::RotationY((float) sin(m_i32Frame * 0.003f) - PVRT_PI_OVER_TWOf);
	glLoadMatrixf(mModelView.f);

	DrawMesh(Node.nIdx);

	// Disable the second layer of texturing
	if(m_bDrawWithDot3)
	{
		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
	}
	else
		glDisable(GL_LIGHTING);


	// Display info text
	m_Print3D.DisplayDefaultTitle("PolyBump", m_bDrawWithDot3 ? m_pDescription : "Standard GL lighting" , ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	// Increase frame counter
	++m_i32Frame;

	return true;
}

/*******************************************************************************
 * Function Name  : CalculateDot3LightDirection
 * Inputs		  : *pWorld
 * Outputs		  : *pLightVector
 * Returns		  : true if no error occured
 * Description    : Set global colour for all vertices with the light direction
 *					used for Dot3. Because the object normals have been computed
 *					already in the normal map this value is the same for all vertices
 *					and it coincides with the light direction transformed with the
 *					inverse of the world matrix.
 *******************************************************************************/
void OGLESPolybump::CalculateDot3LightDirection(PVRTVec4 Dot3LightPos)
{
	// Half shifting to have a value between 0.0f and 1.0f
	Dot3LightPos.x = Dot3LightPos.x * 0.5f + 0.5f;
	Dot3LightPos.y = Dot3LightPos.y * 0.5f + 0.5f;
	Dot3LightPos.z = Dot3LightPos.z * 0.5f + 0.5f;

	/*
		Set light direction as a colour
		(the colour ordering depend on how the normal map has been computed)
		red=y, green=z, blue=x
	 */
	glColor4f(Dot3LightPos.y, Dot3LightPos.z, Dot3LightPos.x, 1.0f);
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			ID of mesh to draw
 @Description	Draws a mesh.
******************************************************************************/
void OGLESPolybump::DrawMesh(unsigned int ui32MeshID)
{
	glEnableClientState(GL_VERTEX_ARRAY);

	SPODMesh& Mesh = m_Scene.pMesh[ui32MeshID];

	// Bind the vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[ui32MeshID]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[ui32MeshID]);

	// Setup pointers
	glVertexPointer(3, GL_FLOAT, Mesh.sVertex.nStride, Mesh.sVertex.pData);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, Mesh.sNormals.nStride, Mesh.sNormals.pData);

	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, Mesh.psUVW[0].nStride, Mesh.psUVW[0].pData);

	if(m_bDrawWithDot3)
	{
		glClientActiveTexture(GL_TEXTURE1);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, Mesh.psUVW[0].nStride, Mesh.psUVW[0].pData);
	}

	glDrawElements(GL_TRIANGLES, Mesh.nNumFaces * 3, GL_UNSIGNED_SHORT, 0);

	// unbind the vertex buffers as we don't need them bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glClientActiveTexture(GL_TEXTURE0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

}

/*!***************************************************************************
 @Function		NewDemo
 @Return		The demo supplied by the user
 @Description	This function must be implemented by the user of the shell.
				The user should return its PVRShell object defining the
				behaviour of the application
*****************************************************************************/
PVRShell* NewDemo()
{
	return new OGLESPolybump();
}

/*****************************************************************************
 End of file (OGLESPolybump.cpp)
*****************************************************************************/

