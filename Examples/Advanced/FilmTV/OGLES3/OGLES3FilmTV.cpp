/******************************************************************************

 @File         OGLES3FilmTV.cpp

 @Title        Introducing the POD 3D file format

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows how to load POD files and play the animation with basic
               lighting

******************************************************************************/
#include <string.h>

#include "PVRShell.h"
#include "OGLES3Tools.h"

/******************************************************************************
 Defines
******************************************************************************/
// Index to bind the attributes to vertex shaders
#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY	2

/******************************************************************************
 Consts
******************************************************************************/
// Camera constants. Used for making the projection matrix
const float g_fCameraNear = 1.0f;
const float g_fCameraFar  = 150.0f;

// The camera to use from the pod file
const int g_ui32Camera = 0;

const float g_ui32CameraLoopSpeed = 10.0f;
const unsigned int g_ui32CameraMesh = 9;
const unsigned int g_ui32TvScreen = 7;

/******************************************************************************
 Content file names
******************************************************************************/

// Source and binary shaders
const char c_szFragShaderSrcFile[]	= "FragShader.fsh";
const char c_szFragShaderBinFile[]	= "FragShader.fsc";
const char c_szBWFragShaderSrcFile[]= "BWFragShader.fsh";
const char c_szBWFragShaderBinFile[]= "BWFragShader.fsc";
const char c_szVertShaderSrcFile[]	= "VertShader.vsh";
const char c_szVertShaderBinFile[]	= "VertShader.vsc";

// POD scene files
const char c_szSceneFile[]			= "FilmTVScene.pod";

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES3FilmTV : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// 3D Model
	CPVRTModelPOD	m_Scene;

	// OpenGL handles for shaders, textures and VBOs
	GLuint m_uiVertShader;
	GLuint m_uiFragShader;
	GLuint m_uiBWFragShader;
	GLuint* m_puiVbo;
	GLuint* m_puiIndexVbo;
	GLuint* m_puiTextureIDs;
	GLint	m_i32OriginalFB;

	// Group shader programs and their uniform locations together
	struct
	{
		GLuint uiId;
		GLuint uiMVPMatrixLoc;
		GLuint uiLightPosLoc;
	}
	m_ShaderProgram, m_BWShaderProgram;

	// App Variables
	int		 m_i32TexSize;

	GLuint	m_uiTexture[2];
    GLuint	m_uiFbo[2];
	GLuint	m_uiDepthBuffer[2];
	int		m_i32CurrentFBO;
	int		m_uiTVScreen;
	int		m_i32Frame;

	PVRTMat4 m_MiniCamView;
	PVRTMat4 m_MiniCamViewProj;
	PVRTMat4 m_View;
	PVRTMat4 m_ViewProjection;

    // A boolean to indicate that our FBOs were created successfully
    bool m_bFBOsCreated;

	// Start time
	unsigned long m_ulStartTime;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	bool LoadTextures(CPVRTString* pErrorStr);
	bool LoadShaders(CPVRTString* pErrorStr);
	bool LoadVbos(CPVRTString* pErrorStr);

	void DrawPODScene(PVRTMat4 &mViewProjection, bool bDrawCamera);
	void DrawMesh(int i32MeshIndex);
	void CalcMiniCameraView();

	OGLES3FilmTV() : m_puiVbo(0),
					 m_puiIndexVbo(0),
					 m_puiTextureIDs(0),
					 m_i32TexSize(1),
					 m_i32CurrentFBO(1),
					 m_uiTVScreen(-1),
                     m_i32Frame(0),
                     m_bFBOsCreated(true)
	{
	}
};

/*!****************************************************************************
 @Function		LoadTextures
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course
******************************************************************************/
bool OGLES3FilmTV::LoadTextures(CPVRTString* pErrorStr)
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

	for(unsigned int i = 0; i < m_Scene.nNumMaterial; ++i)
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

			if(sTextureName == "TV.pvr")
				m_uiTVScreen = i;

			if(PVRTTextureLoadFromPVR(sTextureName.c_str(), &m_puiTextureIDs[i]) != PVR_SUCCESS)
			{
				*pErrorStr = "ERROR: Failed to load " + sTextureName + ".";

				// Check to see if we're trying to load .pvr or not
				CPVRTString sFileExtension = PVRTStringGetFileExtension(sTextureName);

				if(sFileExtension.toLower() != "pvr")
					*pErrorStr += "Note: FilmTV can only load .pvr files.";

				return false;
			}
		}
	}

	return true;
}

/*!****************************************************************************
 @Function		LoadShaders
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occurred
 @Description	Loads and compiles the shaders and links the shader programs
				required for this training course
******************************************************************************/
bool OGLES3FilmTV::LoadShaders(CPVRTString* pErrorStr)
{
	/*
		Load and compile the shaders from files.
		Binary shaders are tried first, source shaders
		are used as fallback.
	*/
	if(PVRTShaderLoadFromFile(
			c_szVertShaderBinFile, c_szVertShaderSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiVertShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	if (PVRTShaderLoadFromFile(
			c_szFragShaderBinFile, c_szFragShaderSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_uiFragShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	/*
		Set up and link the shader program
	*/
	const char* aszAttribs[] = { "inVertex", "inNormal", "inTexCoord" };

	if(PVRTCreateProgram(
			&m_ShaderProgram.uiId, m_uiVertShader, m_uiFragShader, aszAttribs, 3, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	// Set the sampler2D variable to the first texture unit
	glUniform1i(glGetUniformLocation(m_ShaderProgram.uiId, "sTexture"), 0);

	// Store the location of uniforms for later use
	m_ShaderProgram.uiMVPMatrixLoc	= glGetUniformLocation(m_ShaderProgram.uiId, "MVPMatrix");
	m_ShaderProgram.uiLightPosLoc	= glGetUniformLocation(m_ShaderProgram.uiId, "LightPosition");

	// Load and compile the Black and White shader used on the TV screen
	if(PVRTShaderLoadFromFile(
			c_szBWFragShaderBinFile, c_szBWFragShaderSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_uiBWFragShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	// Set up and link the shader program (re-using the already loaded vertex shader
	if(PVRTCreateProgram(
			&m_BWShaderProgram.uiId, m_uiVertShader, m_uiBWFragShader, aszAttribs, 3, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	// Set the sampler2D variable to the first texture unit
	glUniform1i(glGetUniformLocation(m_BWShaderProgram.uiId, "sTexture"), 0);

	// Store the location of uniforms for later use
	m_BWShaderProgram.uiMVPMatrixLoc= glGetUniformLocation(m_BWShaderProgram.uiId, "MVPMatrix");
	m_BWShaderProgram.uiLightPosLoc	= glGetUniformLocation(m_BWShaderProgram.uiId, "LightPosition");

	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
bool OGLES3FilmTV::LoadVbos(CPVRTString* pErrorStr)
{
	if(!m_Scene.pMesh[0].pInterleaved)
	{
		*pErrorStr = "ERROR: FilmTV requires the pod data to be interleaved. Please re-export with the interleaved option enabled.";
		return false;
	}

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

	return true;
}

/*!****************************************************************************
 @Function		InitApplication
 @Return		bool		true if no error occurred
 @Description	Code in InitApplication() will be called by PVRShell once per
				run, before the rendering context is created.
				Used to initialize variables that are not dependant on it
				(e.g. external modules, loading meshes, etc.)
				If the rendering context is lost, InitApplication() will
				not be called again.
******************************************************************************/
bool OGLES3FilmTV::InitApplication()
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
		CPVRTString ErrorStr = "ERROR: Couldn't load '" + CPVRTString(c_szSceneFile) + "'.";
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// The cameras are stored in the file. We check it contains at least one.
	if(m_Scene.nNumCamera == 0)
	{
		PVRShellSet(prefExitMessage, "ERROR: The scene does not contain a camera. Please add one and re-export.\n");
		return false;
	}

	// We also check that the scene contains at least one light
	if(m_Scene.nNumLight == 0)
	{
		PVRShellSet(prefExitMessage, "ERROR: The scene does not contain a light. Please add one and re-export.\n");
		return false;
	}

	return true;
}

/*!****************************************************************************
 @Function		QuitApplication
 @Return		bool		true if no error occurred
 @Description	Code in QuitApplication() will be called by PVRShell once per
				run, just before exiting the program.
				If the rendering context is lost, QuitApplication() will
				not be called.
******************************************************************************/
bool OGLES3FilmTV::QuitApplication()
{
	// Free the memory allocated for the scene
	m_Scene.Destroy();

	delete[] m_puiVbo;
	delete[] m_puiIndexVbo;

    return true;
}

/*!****************************************************************************
 @Function		InitView
 @Return		bool		true if no error occurred
 @Description	Code in InitView() will be called by PVRShell upon
				initialization or after a change in the rendering context.
				Used to initialize variables that are dependant on the rendering
				context (e.g. textures, vertex buffers, etc.)
******************************************************************************/
bool OGLES3FilmTV::InitView()
{
	CPVRTString ErrorStr;

	//	Initialize VBO data
	if(!LoadVbos(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	//	Load textures
	if(!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	//	Load and compile the shaders & link programs
	if(!LoadShaders(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	//	Initialize Print3D
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	//Set OpenGL ES render states needed for this demo

	// Enable backface culling and depth test
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Find the largest square power of two texture that fits into the viewport
	m_i32TexSize = 1;
	int iSize = PVRT_MIN(PVRShellGet(prefWidth), PVRShellGet(prefHeight));
	while (m_i32TexSize * 2 < iSize) m_i32TexSize *= 2;

	// Get the currently bound frame buffer object. On most platforms this just gives 0.
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_i32OriginalFB);

	for(int i = 0; i < 2; ++i)
	{
		// Create texture for the FBO
		glGenTextures(1, &m_uiTexture[i]);
		glBindTexture(GL_TEXTURE_2D, m_uiTexture[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_i32TexSize, m_i32TexSize, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 0);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// Create FBO
		glGenFramebuffers(1, &m_uiFbo[i]);
		glBindFramebuffer(GL_FRAMEBUFFER, m_uiFbo[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uiTexture[i], 0);

		glGenRenderbuffers(1, &m_uiDepthBuffer[i]);
		glBindRenderbuffer(GL_RENDERBUFFER, m_uiDepthBuffer[i]);

		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_i32TexSize, m_i32TexSize);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_uiDepthBuffer[i]);

        // Check that our FBO creation was successful
        GLuint uStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if(uStatus != GL_FRAMEBUFFER_COMPLETE)
        {
            m_bFBOsCreated = false;
            PVRShellOutputDebug("ERROR: Failed to initialise FBO");
            break;
        }

		// Clear the colour buffer for this FBO
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_i32OriginalFB);

	// Setup the main camera
	PVRTVec3	vFrom, vTo(0.0f), vUp(0.0f, 1.0f, 0.0f);
	float fFOV;

	// Camera nodes are after the mesh and light nodes in the array
	int i32CamID = m_Scene.pNode[m_Scene.nNumMeshNode + m_Scene.nNumLight + g_ui32Camera].nIdx;

	// Get the camera position, target and field of view (fov)
	if(m_Scene.pCamera[i32CamID].nIdxTarget != -1) // Does the camera have a target?
		fFOV = m_Scene.GetCameraPos( vFrom, vTo, g_ui32Camera); // vTo is taken from the target node
	else
		fFOV = m_Scene.GetCamera( vFrom, vTo, vUp, g_ui32Camera); // vTo is calculated from the rotation

	m_View = PVRTMat4::LookAtRH(vFrom, vTo, vUp);

	// Calculate the projection matrix
	PVRTMat4 mProjection = PVRTMat4::PerspectiveFovRH(fFOV, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), g_fCameraNear, g_fCameraFar, PVRTMat4::OGL, bRotate);
	m_ViewProjection = mProjection * m_View;

	// Store initial time
	m_ulStartTime = PVRShellGetTime();

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occurred
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES3FilmTV::ReleaseView()
{
	// Delete program and shader objects
	glDeleteProgram(m_ShaderProgram.uiId);
	glDeleteProgram(m_BWShaderProgram.uiId);

	glDeleteShader(m_uiVertShader);
	glDeleteShader(m_uiFragShader);
	glDeleteShader(m_uiBWFragShader);

	// Delete buffer objects
	glDeleteBuffers(m_Scene.nNumMesh, m_puiVbo);
	glDeleteBuffers(m_Scene.nNumMesh, m_puiIndexVbo);
	glDeleteFramebuffers(2, m_uiFbo);

	// Delete our depth buffer render buffers
	glDeleteRenderbuffers(2, m_uiDepthBuffer);

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

	// Deletes the textures
	glDeleteTextures(m_Scene.nNumMaterial, &m_puiTextureIDs[0]);
	glDeleteTextures(2, &m_uiTexture[0]);

	// Frees the texture lookup array
	delete[] m_puiTextureIDs;
	m_puiTextureIDs = 0;

	return true;
}

void OGLES3FilmTV::CalcMiniCameraView()
{
	float fValue = (PVRShellGetTime() - m_ulStartTime) * 0.001f * 2.0f * PVRT_PIf;

	float fZ = 1.0f + (2.40f * (float)sin(fValue * 1.0f / g_ui32CameraLoopSpeed));
	float fX = 0.50f * (float)cos(fValue * 2.0f / g_ui32CameraLoopSpeed);
	float fCamRot = 0.16f * (float)sin(fValue * 1.0f / g_ui32CameraLoopSpeed) - 0.17f;

	m_MiniCamView  = PVRTMat4::RotationX((float)atan2(fX, 10.0f));
	m_MiniCamView *= PVRTMat4::RotationY(fCamRot);
	m_MiniCamView *= PVRTMat4::RotationZ((float)atan2(fZ, 10.0f));

	// Setup the mini camera's view projection matrix
	PVRTMat4 mProjection = PVRTMat4::PerspectiveFovRH(70.0f * (PVRT_PIf / 180.0f), 1, g_fCameraNear, g_fCameraFar, PVRTMat4::OGL, false);
	m_MiniCamViewProj = mProjection * m_MiniCamView;
}

/*!****************************************************************************
 @Function		RenderScene
 @Return		bool		true if no error occurred
 @Description	Main rendering loop function of the program. The shell will
				call this function every frame.
				eglSwapBuffers() will be performed by PVRShell automatically.
				PVRShell will also manage important OS events.
				Will also manage relevant OS events. The user has access to
				these events through an abstraction layer provided by PVRShell.
******************************************************************************/
bool OGLES3FilmTV::RenderScene()
{
	// Use shader program
	glUseProgram(m_ShaderProgram.uiId);

	// Enable the vertex attribute arrays
	glEnableVertexAttribArray(VERTEX_ARRAY);
	glEnableVertexAttribArray(NORMAL_ARRAY);

    // Render everything from the mini-camera's point of view if we have the FBOs
    CalcMiniCameraView();

    if(m_bFBOsCreated)
    {
		// Setup the Viewport to the dimensions of the texture
		glViewport(0, 0, m_i32TexSize, m_i32TexSize);

		glBindFramebuffer(GL_FRAMEBUFFER, m_uiFbo[m_i32CurrentFBO]);

		DrawPODScene(m_MiniCamViewProj, false);

		//Invalidate the depth attachment we don't need to avoid unnecessary copying to system memory
		const GLenum attachment = GL_DEPTH_ATTACHMENT;
		glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attachment);

		glBindFramebuffer(GL_FRAMEBUFFER, m_i32OriginalFB);

		// Render everything

		// Setup the Viewport to the dimensions of the screen
		glViewport(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));
    }

	DrawPODScene(m_ViewProjection, true);

	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("FilmTV", "", ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	// Swap the FBO that we want to render to
	m_i32CurrentFBO = 1 - m_i32CurrentFBO;

	++m_i32Frame;
	return true;
}

void OGLES3FilmTV::DrawPODScene(PVRTMat4 &mViewProjection, bool bDrawCamera)
{
	// Clear the colour and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Get the position of the first light from the scene.
	PVRTVec4 vLightPosition = m_Scene.GetLightPosition(0);

	for(unsigned int i = 0; i < m_Scene.nNumMeshNode; ++i)
	{
		SPODNode& Node = m_Scene.pNode[i];

		// Get the node model matrix
		PVRTMat4 mWorld = m_Scene.GetWorldMatrix(Node);

		if(i == g_ui32CameraMesh)
		{
			if(!bDrawCamera)
				continue;

			// Rotate camera model
			mWorld =  m_MiniCamView.inverse() * mWorld;
		}
		else if(i == g_ui32TvScreen) // If we're drawing the TV screen change to the black and white shader
		{
			glUseProgram(m_BWShaderProgram.uiId);
		}

		// Pass the model-view-projection matrix (MVP) to the shader to transform the vertices
		PVRTMat4 mModelView, mMVP;
		mMVP = mViewProjection * mWorld;
		glUniformMatrix4fv(m_ShaderProgram.uiMVPMatrixLoc, 1, GL_FALSE, mMVP.f);

		// Pass the light position in model space to the shader
		PVRTVec4 vLightPos;
		vLightPos = mWorld.inverse() * vLightPosition;

		glUniform3fv(m_ShaderProgram.uiLightPosLoc, 1, &vLightPos.x);

		// Load the correct texture using our texture lookup table
		GLuint uiTex = 0;

		if(Node.nIdxMaterial != -1)
		{
            if(m_bFBOsCreated && Node.nIdxMaterial == m_uiTVScreen && m_i32Frame != 0)
				uiTex = m_uiTexture[1 - m_i32CurrentFBO];
			else
				uiTex = m_puiTextureIDs[Node.nIdxMaterial];
		}

		glBindTexture(GL_TEXTURE_2D, uiTex);

		/*
			Now that the model-view matrix is set and the materials ready,
			call another function to actually draw the mesh.
		*/
		DrawMesh(Node.nIdx);

		if(i == g_ui32TvScreen)
		{
			// Change back to the normal shader after drawing the g_ui32TvScreen
			glUseProgram(m_ShaderProgram.uiId);
		}
	}
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			i32MeshIndex		Mesh index of the mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the material prepared.
******************************************************************************/
void OGLES3FilmTV::DrawMesh(int i32MeshIndex)
{
	SPODMesh& Mesh = m_Scene.pMesh[i32MeshIndex];

	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[i32MeshIndex]);

	// bind the index buffer, won't hurt if the handle is 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[i32MeshIndex]);

	// Set the vertex attribute offsets
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, Mesh.sVertex.nStride, Mesh.sVertex.pData);
	glVertexAttribPointer(NORMAL_ARRAY, 3, GL_FLOAT, GL_FALSE, Mesh.sNormals.nStride, Mesh.sNormals.pData);

	if(Mesh.nNumUVW) // Do we have texture co-ordinates?
	{
		glEnableVertexAttribArray(TEXCOORD_ARRAY);
		glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, Mesh.psUVW[0].nStride, Mesh.psUVW[0].pData);
	}

	// Draw the Indexed Triangle list
	glDrawElements(GL_TRIANGLES, Mesh.nNumFaces*3, GL_UNSIGNED_SHORT, 0);

	// Disable the uv attribute array
	glDisableVertexAttribArray(TEXCOORD_ARRAY);

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
	return new OGLES3FilmTV();
}

/******************************************************************************
 End of file (OGLES3FilmTV.cpp)
******************************************************************************/

