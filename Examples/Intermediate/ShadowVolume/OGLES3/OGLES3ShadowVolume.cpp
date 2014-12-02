/******************************************************************************

 @File         OGLES3ShadowVolume.cpp

 @Title        Shadow Volumes

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows how to use PVRTools to generate shadow volumes for stencil
               shadows.

******************************************************************************/
#include <string.h>

#include "PVRShell.h"
#include "OGLES3Tools.h"

/******************************************************************************
 Constants
******************************************************************************/

// Camera constants used to generate the projection matrix
const float CAM_NEAR	= 75.0f;
const float CAM_FAR		= 2000.0f;

/******************************************************************************
 shader attributes
******************************************************************************/
// vertex attributes
enum EVertexAttrib {
	VERTEX_ARRAY, NORMAL_ARRAY, TEXCOORD_ARRAY, eNumAttribs };

// shader uniforms
enum EUniform {
	eMVPMatrix, eLightPosModel, eVolumeScale, eColor, eNumUniforms };
const char* g_aszUniformNames[] = {
	"MVPMatrix", "LightPosModel", "VolumeScale", "Color" };

// Enum to decide which cog is which
enum eCog
{
	eBigCog = 1,
	eSmallCog = 2,
	eNumMeshes
};

enum eObjectType
{
	eDoesntCast,	// This object type doesn't cast shadows
	eStaticObject,	// This object type doesn't move so we only need to calculate its volume once (as long as the light doesn't move)
	eDynamicObject	// This object type changes every frame so we need to calculate its volume every frame also.
};

/******************************************************************************
 Content file names
******************************************************************************/

// Source and binary shaders
const char c_szBaseFragSrcFile[]		= "BaseFragShader.fsh";
const char c_szBaseFragBinFile[]		= "BaseFragShader.fsc";
const char c_szConstFragSrcFile[]		= "ConstFragShader.fsh";
const char c_szConstFragBinFile[]		= "ConstFragShader.fsc";
const char c_szBaseVertSrcFile[]		= "BaseVertShader.vsh";
const char c_szBaseVertBinFile[]		= "BaseVertShader.vsc";
const char c_szShadowVolVertSrcFile[]	= "ShadowVolVertShader.vsh";
const char c_szShadowVolVertBinFile[]	= "ShadowVolVertShader.vsc";
const char c_szFullscreenVertSrcFile[]	= "FullscreenVertShader.vsh";
const char c_szFullscreenVertBinFile[]	= "FullscreenVertShader.vsc";

// PVR texture files
const char c_szBackgroundTexFile[]	= "Background.pvr";
const char c_szRustTexFile[]		= "Rust.pvr";

// POD scene files
const char c_szSceneFile[]			= "scene.pod";

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES3ShadowVolumes : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// 3D Model
	CPVRTModelPOD	m_Scene;
	PVRTVec4		m_vLightPosWorld;

	// Projection and view matrices
	PVRTMat4		m_mProjection, m_mView;

	// OpenGL handles for shaders, textures and VBOs
	GLuint	m_uiBaseVertShader;
	GLuint	m_uiShadowVolVertShader;
	GLuint	m_uiFullscreenVertShader;
	GLuint	m_uiBaseFragShader;
	GLuint	m_uiConstFragShader;
	GLuint	m_uiBackgroundTex;
	GLuint  m_uiRustTex;
	GLuint*	m_puiVbo;
	GLuint*	m_puiIndexVbo;

	// Group shader programs and their uniform locations together
	struct
	{
		GLuint uiId;
		GLuint auiLoc[eNumUniforms];
	}
	m_BaseShader, m_FullscreenShader, m_ShadowVolShader;

	// Array to lookup the textures for each material in the scene
	GLuint*			m_puiTextures;

	// Variables to handle the animation in a time-based manner
	unsigned long	m_ulTimePrev;
	float			m_fBigCogAngle;
	float			m_fSmallCogAngle;

	PVRTShadowVolShadowMesh		*m_pShadowMesh;	// Model definition suitable for shadow volumes
	PVRTShadowVolShadowVol			*m_pShadowVol;	// Geometry of actual shadow volume

	// The number of shadows we have in the scene
	unsigned int m_ui32NoOfShadows;

	// An array that will store the ID of the SPODMesh that the shadows are cast from
	unsigned int* m_pui32MeshIndex;

	// An array to store the mesh type. It stores eObjectType values.
	int m_i32ObjectType[eNumMeshes];

	// A boolean used to decide whether to display the shadow volumes
	bool m_bDisplayVolumes;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	bool LoadTextures(CPVRTString* pErrorStr);
	bool LoadShaders(CPVRTString* pErrorStr);
	void LoadVbos();

	void DrawMesh(int i32NodeIndex);
	void DoStencilTest();
	bool BuildShadowVolume(PVRTShadowVolShadowMesh *pShadowMesh, PVRTShadowVolShadowVol *pVolume, SPODMesh* pMesh);
	void DrawScene();
	void DrawShadowVolumes(PVRTVec4 *pLightPos);
	void DrawFullScreenQuad();
	bool BuildVolume(unsigned int ui32ShadowVol, PVRTVec4 *pLightPos);
};

/*!****************************************************************************
 @Function		LoadTextures
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course
******************************************************************************/
bool OGLES3ShadowVolumes::LoadTextures(CPVRTString* const pErrorStr)
{
	if(PVRTTextureLoadFromPVR(c_szBackgroundTexFile, &m_uiBackgroundTex) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Failed to load texture.";
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szRustTexFile, &m_uiRustTex) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Failed to load texture.";
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return true;
}

/*!****************************************************************************
 @Function		LoadShaders
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads and compiles the shaders and links the shader programs
				required for this training course
******************************************************************************/
bool OGLES3ShadowVolumes::LoadShaders(CPVRTString* pErrorStr)
{
	/*
		Load and compile the shaders from files.
		Binary shaders are tried first, source shaders
		are used as fallback.
	*/
	if (
		(PVRTShaderLoadFromFile(c_szBaseFragBinFile, c_szBaseFragSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_uiBaseFragShader, pErrorStr) != PVR_SUCCESS) ||
		(PVRTShaderLoadFromFile(c_szConstFragBinFile, c_szConstFragSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_uiConstFragShader, pErrorStr) != PVR_SUCCESS) ||
		(PVRTShaderLoadFromFile(c_szBaseVertBinFile, c_szBaseVertSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiBaseVertShader, pErrorStr) != PVR_SUCCESS) ||
		(PVRTShaderLoadFromFile(c_szShadowVolVertBinFile, c_szShadowVolVertSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiShadowVolVertShader, pErrorStr) != PVR_SUCCESS) ||
		(PVRTShaderLoadFromFile(c_szFullscreenVertBinFile, c_szFullscreenVertSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiFullscreenVertShader, pErrorStr) != PVR_SUCCESS))
	{
		return false;
	}

	/*
		Set up and link the shader program
	*/

	const char* aszBaseAttribNames[] = { "inVertex", "inNormal", "inTexCoord" };
	const char* aszFullscreenAttribNames[] = { "inVertex" };
	const char* aszShadowVolAttribNames[] = { "inVertex", "inExtrude" };

	if (
		(PVRTCreateProgram(&m_BaseShader.uiId, m_uiBaseVertShader, m_uiBaseFragShader, aszBaseAttribNames, 3, pErrorStr) != PVR_SUCCESS) ||
		(PVRTCreateProgram(&m_FullscreenShader.uiId, m_uiFullscreenVertShader, m_uiConstFragShader, aszFullscreenAttribNames, 1, pErrorStr) != PVR_SUCCESS) ||
		(PVRTCreateProgram(&m_ShadowVolShader.uiId, m_uiShadowVolVertShader, m_uiConstFragShader, aszShadowVolAttribNames, 2, pErrorStr) != PVR_SUCCESS))
	{
		return false;
	}

	// Store the location of uniforms for later use
	for (int i = 0; i < eNumUniforms; ++i)
	{
		m_BaseShader.auiLoc[i] = glGetUniformLocation(m_BaseShader.uiId, g_aszUniformNames[i]);
		m_FullscreenShader.auiLoc[i] = glGetUniformLocation(m_FullscreenShader.uiId, g_aszUniformNames[i]);
		m_ShadowVolShader.auiLoc[i] = glGetUniformLocation(m_ShadowVolShader.uiId, g_aszUniformNames[i]);
	}

	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLES3ShadowVolumes::LoadVbos()
{
	if (!m_puiVbo)      m_puiVbo = new GLuint[m_Scene.nNumMesh];
	if (!m_puiIndexVbo) m_puiIndexVbo = new GLuint[m_Scene.nNumMesh];

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
 @Function		InitApplication
 @Return		bool		true if no error occured
 @Description	Code in InitApplication() will be called by PVRShell once per
				run, before the rendering context is created.
				Used to initialize variables that are not dependant on it
				(e.g. external modules, loading meshes, etc.)
				If the rendering context is lost, InitApplication() will
				not be called again.
******************************************************************************/
bool OGLES3ShadowVolumes::InitApplication()
{
	m_puiVbo = 0;
	m_puiIndexVbo = 0;

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

	// The cameras are stored in the file. We check it contains at least one.
	if(m_Scene.nNumCamera == 0)
	{
		PVRShellSet(prefExitMessage, "ERROR: The scene does not contain a camera\n");
		return false;
	}

	// The scene must contain at least one light
	if(m_Scene.nNumLight == 0)
	{
		PVRShellSet(prefExitMessage, "ERROR: The scene does not contain a light\n");
		return false;
	}

	// Get the Light Position
	m_vLightPosWorld = m_Scene.GetLightPosition(0);
	m_vLightPosWorld.w = 1.0f;

	// Initialise variables
	m_bDisplayVolumes = false;

	m_fBigCogAngle   = 0.0f;
	m_fSmallCogAngle = 0.0f;

	// Set up the object type for each mesh in the scene
	m_i32ObjectType[0] = eDoesntCast;	 // The mesh that makes up the background
	m_i32ObjectType[1] = eDynamicObject; // The big cog
	m_i32ObjectType[2] = eStaticObject;  // The small cog

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
bool OGLES3ShadowVolumes::QuitApplication()
{
	// Free the memory allocated for the scene
	m_Scene.Destroy();

	delete [] m_puiVbo;
	delete [] m_puiIndexVbo;

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
bool OGLES3ShadowVolumes::InitView()
{
	CPVRTString ErrorStr;

	/*
		Initialize VBO data
	*/
	LoadVbos();

	/*
		Load textures
	*/
	if (!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	/*
		Load and compile the shaders & link programs
	*/
	if (!LoadShaders(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Set the sampler2D uniforms to corresponding texture units
	glUseProgram(m_BaseShader.uiId);
	glUniform1i(glGetUniformLocation(m_BaseShader.uiId, "sTexture"), 0);

	/*
		Initialise an array to lookup the textures
		for each material in the scene.
	*/
	m_puiTextures = new GLuint[m_Scene.nNumMaterial];

	for(unsigned int i = 0; i < m_Scene.nNumMaterial; ++i)
	{
		m_puiTextures[i] = 0;
		SPODMaterial* pMaterial = &m_Scene.pMaterial[i];

		if (!strcmp(pMaterial->pszName, "background"))
		{
			m_puiTextures[i] = m_uiBackgroundTex;
		}
		else if (!strcmp(pMaterial->pszName, "rust"))
		{
			m_puiTextures[i] = m_uiRustTex;
		}
	}

	// Go through the object type and find out how many shadows we are going to need
	m_ui32NoOfShadows = 0;

	for (int i = 0; i < eNumMeshes; ++i)
	{
		if(m_i32ObjectType[i] != eDoesntCast) ++m_ui32NoOfShadows;
	}

	// Build the shadow volumes and meshes

	// Create the number of shadow meshes and volumes we require
	m_pShadowMesh = new PVRTShadowVolShadowMesh[m_ui32NoOfShadows];
	m_pShadowVol  = new PVRTShadowVolShadowVol [m_ui32NoOfShadows];

	// Create the array that stores the SPODNode ID for each shadow
	m_pui32MeshIndex = new unsigned int[m_ui32NoOfShadows];

	// Go through the meshes and initialise the shadow meshes, volumes and mesh index for each requried shadow
	int i32Index = 0;
	for (int i = 0; i < eNumMeshes; ++i)
	{
		if(m_i32ObjectType[i] != eDoesntCast)
		{
			m_pui32MeshIndex[i32Index] = i;

			SPODNode* pNode = &m_Scene.pNode[i];

			/*
				This function will take the POD mesh referenced by the current node and generate a
				new mesh suitable for creating shadow volumes and the shadow volume itself.
			*/
			BuildShadowVolume(&m_pShadowMesh[i32Index], &m_pShadowVol[i32Index], &m_Scene.pMesh[pNode->nIdx]);

			/*
				The function will initialise the shadow volume with regard to the meshes current transformation
				and the light position.

				As the light position is fixed this is only done once for static objects where as dynamic objects
				are updated every frame.
			*/
			BuildVolume(i32Index, &m_vLightPosWorld);

			++i32Index;
		}
	}

	// Is the screen rotated?
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	/*
		Initialize Print3D
	*/
	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	// Calculate the view matrix
	PVRTVec3	vFrom, vTo;
	float fFOV;

	// We can get the camera position, target and field of view (fov) with GetCamera()
	fFOV = m_Scene.GetCameraPos( vFrom, vTo, 0);
	m_mView = PVRTMat4::LookAtRH(vFrom, vTo, PVRTVec3(0, 1, 0));

	// Calculate the projection matrix
	m_mProjection = PVRTMat4::PerspectiveFovRH(fFOV,  (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), CAM_NEAR, CAM_FAR, PVRTMat4::OGL, bRotate);

	/*
		Set OpenGL ES render states needed for this training course
	*/
	// Enable backface culling and depth test
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	// Use a nice bright blue as clear colour
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
	glClearStencil(0);

	m_ulTimePrev = PVRShellGetTime();
	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES3ShadowVolumes::ReleaseView()
{
	// Frees the texture lookup array
	delete[] m_puiTextures;

	// Delete textures
	glDeleteTextures(1, &m_uiBackgroundTex);
	glDeleteTextures(1, &m_uiRustTex);

	// Delete program and shader objects
	glDeleteProgram(m_BaseShader.uiId);
	glDeleteProgram(m_ShadowVolShader.uiId);
	glDeleteProgram(m_FullscreenShader.uiId);

	glDeleteShader(m_uiBaseVertShader);
	glDeleteShader(m_uiShadowVolVertShader);
	glDeleteShader(m_uiFullscreenVertShader);
	glDeleteShader(m_uiBaseFragShader);
	glDeleteShader(m_uiConstFragShader);

	// Delete buffer objects
	glDeleteBuffers(m_Scene.nNumMesh, m_puiVbo);
	glDeleteBuffers(m_Scene.nNumMesh, m_puiIndexVbo);

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

	// Free the shadow volumes and meshes
	for(unsigned int i = 0; i < m_ui32NoOfShadows; ++i)
	{
		PVRTShadowVolMeshReleaseVol(&m_pShadowVol[i]);
		PVRTShadowVolMeshReleaseMesh(&m_pShadowMesh[i]);
		PVRTShadowVolMeshDestroyMesh(&m_pShadowMesh[i]);
	}

	delete[] m_pShadowMesh;
	delete[] m_pShadowVol;
	delete[] m_pui32MeshIndex;

	return true;
}

/*!****************************************************************************
 @Function		BuildVolume
 @Return		bool		true if no error occured
 @Description	This function will create the volume that we will be drawn
				in the stenciltest.
******************************************************************************/
bool OGLES3ShadowVolumes::BuildVolume(unsigned int ui32ShadowVol, PVRTVec4 *pLightPos)
{
	SPODNode* pNode;
	PVRTMat4 mWorld;
	PVRTVec4 vModelLightPos;

	int i32MeshIndex = m_pui32MeshIndex[ui32ShadowVol];

	pNode = &m_Scene.pNode[i32MeshIndex];

	// Get the world matrix for this particular node.
	switch(i32MeshIndex)
	{
		case eBigCog:
			mWorld = PVRTMat4::RotationZ(m_fBigCogAngle);
		break;
		case eSmallCog:
			mWorld = PVRTMat4::RotationZ(m_fSmallCogAngle);
		break;
		default:
			mWorld = m_Scene.GetWorldMatrix(*pNode);
	}

	/*
		Convert the light position into model space for the current Node.
	*/
	vModelLightPos = mWorld.inverse() * (*pLightPos);

	/*
		Using the light position set up the shadow volume so it can be extruded in the shader.
	*/

	unsigned int ui32Flags = PVRTSHADOWVOLUME_VISIBLE | PVRTSHADOWVOLUME_NEED_CAP_FRONT | PVRTSHADOWVOLUME_NEED_CAP_BACK;
	PVRTShadowVolSilhouetteProjectedBuild(&m_pShadowVol[ui32ShadowVol], ui32Flags , &m_pShadowMesh[ui32ShadowVol], (PVRTVec3*) &vModelLightPos, true);

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
bool OGLES3ShadowVolumes::RenderScene()
{
	//Calculate the time passes since the last frame so we can rotate the cogs in a time-based manner
	unsigned long ulTime = PVRShellGetTime();
	unsigned long ulDeltaTime = ulTime - m_ulTimePrev;

	m_ulTimePrev = ulTime;

    // If the cog is classed as dynamic then we need to update its angle of rotation
	if(m_i32ObjectType[eBigCog] == eDynamicObject)
	{
		m_fBigCogAngle   += ulDeltaTime * 0.001f;

		while(m_fBigCogAngle > PVRT_TWO_PI)
			m_fBigCogAngle -= PVRT_TWO_PI;
	}

	if(m_i32ObjectType[eSmallCog] == eDynamicObject)
	{
		m_fSmallCogAngle -= ulDeltaTime * 0.004f;

		while(m_fSmallCogAngle > PVRT_TWO_PI)
			m_fSmallCogAngle -= PVRT_TWO_PI;
	}

	// If the action key has been pressed then switch between drawing and not drawing the shadow volumes
	if (PVRShellIsKeyPressed(PVRShellKeyNameACTION1))
		m_bDisplayVolumes = !m_bDisplayVolumes;

	// Clear the colour, stencil and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	/*
		To create shadows we are going to do the following steps

		1) Using the tools we are going to update any of the shadow volumes for the dynamic objects
		2) Draw the scene as we would any other.
		3) Enable the stencil test.
		4) Draw Shadow Volumes to fill the stencil buffer with data.
		5) Then we are going to draw a fullscreen quad which will only appear where the stencil buffer is not zero.
		6) Disable the stencil test
	*/

	/*
		Update the shadow volumes for any dynamic objects as they have moved so we requrie a different
		shadow volume. If the light position was also dynamic we would have to update volumes for all
		the static objects as well.
	*/

	for(unsigned int i = 0; i < m_ui32NoOfShadows; ++i)
	{
		if(m_i32ObjectType[m_pui32MeshIndex[i]] == eDynamicObject)
		{
			BuildVolume(i, &m_vLightPosWorld);
		}
	}

	// Draw the scene lit.
	DrawScene();

	// Enable the stencil test
	glEnable(GL_STENCIL_TEST);

	// Do the stencil test
	DoStencilTest();

	// Draw a full screen quad
	DrawFullScreenQuad();

	// Disable the stencil test as it is no longer needed.
	glDisable(GL_STENCIL_TEST);

	// Display the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("ShadowVolumes", "", ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		DrawFullScreenQuad
 @Description	Draws a fullscreen quad
******************************************************************************/
void OGLES3ShadowVolumes::DrawFullScreenQuad()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_COLOR, GL_ZERO);

	// Use the shader program for the scene
	glUseProgram(m_FullscreenShader.uiId);
	const float afColor[] = { 0.4f, 0.4f, 0.4f, 1.0f };
	glUniform4fv(m_FullscreenShader.auiLoc[eColor], 1, afColor);

	// Enable vertex arributes
	glEnableVertexAttribArray(VERTEX_ARRAY);

	const float afVertexData[] = { -1, -1,  1, -1,  -1, 1,  1, 1 };
	glVertexAttribPointer(VERTEX_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, afVertexData);

	// Draw the quad
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// Disable vertex arributes
	glDisableVertexAttribArray(VERTEX_ARRAY);

	// Disable blending
	glDisable(GL_BLEND);
}

/*!****************************************************************************
 @Function		DrawScene
 @Input			bLight	If true then the scene is drawn lit, otherwise it isn't
 @Description	Draws the scene
******************************************************************************/
void OGLES3ShadowVolumes::DrawScene()
{
	SPODNode* pNode;
	PVRTMat4 mWorld;
	PVRTMat4 mModelView, mMVP;

	// Use the shader program for the scene
	glUseProgram(m_BaseShader.uiId);

	// Go through the meshes drawing each one
	for(unsigned int i = 0; i < m_Scene.nNumMeshNode; ++i)
	{
		pNode = &m_Scene.pNode[i];

		// Get the world matrix for this particular node.
		switch(i)
		{
			case eBigCog:
				mWorld = PVRTMat4::RotationZ(m_fBigCogAngle);
			break;
			case eSmallCog:
				mWorld = PVRTMat4::RotationZ(m_fSmallCogAngle);
			break;
			default:
				mWorld = m_Scene.GetWorldMatrix(*pNode);
		}

		// Pass the model-view-projection matrix (MVP) to the shader to transform the vertices
		mMVP = m_mProjection * m_mView * mWorld;
		glUniformMatrix4fv(m_BaseShader.auiLoc[eMVPMatrix], 1, GL_FALSE, mMVP.ptr());

		// Pass the light position in model space to the shader
		PVRTVec4 vLightPosModel;
		vLightPosModel = mWorld.inverse() * m_vLightPosWorld;

		glUniform3fv(m_BaseShader.auiLoc[eLightPosModel], 1, &vLightPosModel.x);

		// Loads the correct texture using our texture lookup table
		glBindTexture(GL_TEXTURE_2D, m_puiTextures[pNode->nIdxMaterial]);

		// Draw the mesh node
		DrawMesh(i);
	}
}

/*!****************************************************************************
 @Function		DoStencilTest
 @Description	Performs the Stencil test
******************************************************************************/
void OGLES3ShadowVolumes::DoStencilTest()
{
	/*
		For a detailed explanation on how to use the Stencil Buffer see the training course
		Stencil Buffer.
	*/
	// Use the shader program that is used for the shadow volumes
	glUseProgram(m_ShadowVolShader.uiId);

	// Set the VolumeScale variable in the shader to say how much to extrude the volume by
	glUniform1f(m_ShadowVolShader.auiLoc[eVolumeScale], 1000.0f);

	const float afColor[] = { 0.4f, 1.0f, 0.0f, 0.2f };
	glUniform4fv(m_ShadowVolShader.auiLoc[eColor], 1, afColor);


	//If we want to display the shadow volumes don't disable the colour mask and enable blending
	if(m_bDisplayVolumes)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else // Disable the colour mask so we don't draw to the colour buffer
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	// Disable writing to the depth buffer
	glDepthMask(GL_FALSE);

	// disable culling as we will want the front and back faces
	glDisable(GL_CULL_FACE);

	// Setup the stencil function
	glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);

	/*
		Setup the stencil operations for front facing triangles and for the back facing triangles

		Note:

		We are using INCR_WRAP and DECR_WRAP since we are submitting the front and back faces
		together so we won't be rendering all the INCR faces first. This way it stops the
		stencil value getting clamped at 0 or the maximum possible value.
	*/
	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);

	DrawShadowVolumes(&m_vLightPosWorld);

	// Enable Culling as we would like it back
	glEnable(GL_CULL_FACE);

	// Set the stencil function so we only draw where the stencil buffer isn't 0
	glStencilFunc(GL_NOTEQUAL, 0, 0xFFFFFFFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	// Enable writing to the depth buffer
	glDepthMask(GL_TRUE);

	// If we're displaying the volumes disable blending else enable the colour buffer
	if(m_bDisplayVolumes)
		glDisable(GL_BLEND);
	else
#if defined(__PALMPDK__)
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE); // The alpha part is false as we don't want to blend with the video layer on the Palm Pre
#else
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
#endif
}

/*!****************************************************************************
 @Function		DrawShadowVolumes
 @Input			pLightPos
 @Description	Draw the shadow volumes
******************************************************************************/
void OGLES3ShadowVolumes::DrawShadowVolumes(PVRTVec4 *pLightPos)
{
	PVRTMat4 mModelView, mMVP, mWorld;
	SPODNode *pNode;

	PVRTVec4 vModelLightPos;

	for(unsigned int i = 0; i < m_ui32NoOfShadows; ++i)
	{
		// Get the node
		pNode = &m_Scene.pNode[m_pui32MeshIndex[i]];

		// Get the world matrix for this particular node.
		switch (m_pui32MeshIndex[i])
		{
			case eBigCog:
				mWorld = PVRTMat4::RotationZ(m_fBigCogAngle);
				break;
			case eSmallCog:
				mWorld = PVRTMat4::RotationZ(m_fSmallCogAngle);
				break;
			default:
				mWorld = m_Scene.GetWorldMatrix(*pNode);
		}

		// Set the modeil view projection matrix
		mModelView =  m_mView * mWorld;
		mMVP =  m_mProjection * mModelView;
		glUniformMatrix4fv(m_ShadowVolShader.auiLoc[eMVPMatrix], 1, GL_FALSE, mMVP.ptr());

		// Convert the light position into model space
		vModelLightPos = mWorld.inverse() * (*pLightPos);

		glUniform3fv(m_ShadowVolShader.auiLoc[eLightPosModel], 1, &vModelLightPos.x);

		// Use the tools functions to draw the shadow volumes
		PVRTShadowVolSilhouetteProjectedRender(&m_pShadowMesh[i], &m_pShadowVol[i], 0);
	}
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			i32NodeIndex		Node index of the mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the meterial prepared.
******************************************************************************/
void OGLES3ShadowVolumes::DrawMesh(int i32NodeIndex)
{
	int i32MeshIndex = m_Scene.pNode[i32NodeIndex].nIdx;
	SPODMesh* pMesh = &m_Scene.pMesh[i32MeshIndex];

	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[i32MeshIndex]);
	// bind the index buffer, won't hurt if the handle is 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[i32MeshIndex]);

	// Enable the vertex attribute arrays
	for (int i = 0; i < eNumAttribs; ++i) glEnableVertexAttribArray(i);

	// Set the vertex attribute offsets
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, pMesh->sVertex.nStride, pMesh->sVertex.pData);
	glVertexAttribPointer(NORMAL_ARRAY, 3, GL_FLOAT, GL_FALSE, pMesh->sNormals.nStride, pMesh->sNormals.pData);
	glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, pMesh->psUVW[0].nStride, pMesh->psUVW[0].pData);

	/*
		The geometry can be exported in 4 ways:
		- Indexed Triangle list
		- Non-Indexed Triangle list
		- Indexed Triangle strips
		- Non-Indexed Triangle strips
	*/
	if(pMesh->nNumStrips == 0)
	{
		if(m_puiIndexVbo[i32MeshIndex])
		{
			// Indexed Triangle list
			glDrawElements(GL_TRIANGLES, pMesh->nNumFaces*3, GL_UNSIGNED_SHORT, 0);
		}
		else
		{
			// Non-Indexed Triangle list
			glDrawArrays(GL_TRIANGLES, 0, pMesh->nNumFaces*3);
		}
	}
	else
	{
		for (int i = 0; i < (int)pMesh->nNumStrips; ++i)
		{
			int offset = 0;

			if(m_puiIndexVbo[i32MeshIndex])
			{
				// Indexed Triangle strips
				glDrawElements(GL_TRIANGLE_STRIP, pMesh->pnStripLength[i]+2, GL_UNSIGNED_SHORT, (void*)(offset * sizeof(GLushort)));
			}
			else
			{
				// Non-Indexed Triangle strips
				glDrawArrays(GL_TRIANGLE_STRIP, offset, pMesh->pnStripLength[i]+2);
			}
			offset += pMesh->pnStripLength[i]+2;
		}
	}

	// Safely disable the vertex attribute arrays
	for (int i = 0; i < eNumAttribs; ++i) glDisableVertexAttribArray(i);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/*!****************************************************************************
 @Function		BuildShadowVolume
 @Output		pShadowMesh		The shadow mesh to be returned
 @Output		pVolume			The shadow volume to return
 @Input			pMesh			The mesh to build the shadow volume from
 @Description	Build the shadow mesh and shadow volumes from a SPODMesh
******************************************************************************/
bool OGLES3ShadowVolumes::BuildShadowVolume(PVRTShadowVolShadowMesh *pShadowMesh, PVRTShadowVolShadowVol *pVolume, SPODMesh* pMesh)
{
	if(!pMesh || !pShadowMesh)
		return false;

	PVRTVec3 *pVertices;

	// If the data is interleaved then we need to copy the vertex positions into a temporary array for ShadowMesh creation
	if(pMesh->pInterleaved)
	{
		pVertices = new PVRTVec3[pMesh->nNumVertex];

		if(pVertices)
		{
			unsigned char* pData = pMesh->pInterleaved + (size_t) pMesh->sVertex.pData;

			for(unsigned int i = 0; i < pMesh->nNumVertex; ++i)
			{
				pVertices[i] = *((PVRTVec3*) pData);
				pData += pMesh->sVertex.nStride;
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		// The data isn't interleaved so just use the vertex data as is.
		pVertices = (PVRTVec3 *) pMesh->sVertex.pData;
	}

	// Create a mesh format suitable for generating shadow volumes.
	PVRTShadowVolMeshCreateMesh(pShadowMesh, (float*) pVertices, pMesh->nNumVertex, (unsigned short*) pMesh->sFaces.pData, pMesh->nNumFaces);

	// If the data was interleaved then we created a tmp array which we can now delete.
	if(pMesh->pInterleaved)
	{
		delete[] pVertices;
	}

	// Init the mesh
	PVRTShadowVolMeshInitMesh(pShadowMesh, 0);

	if(pVolume)
	{
		// Create the shadow volume
		PVRTShadowVolMeshInitVol(pVolume, pShadowMesh, 0);
	}

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
	return new OGLES3ShadowVolumes();
}

/******************************************************************************
 End of file (OGLES3ShadowVolumes.cpp)
******************************************************************************/

