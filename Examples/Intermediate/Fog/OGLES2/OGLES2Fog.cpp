/******************************************************************************

 @File         OGLES2Fog.cpp

 @Title        Fog effect with Linear and Exponential function

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows how to using vertex shader for Fog effect.

******************************************************************************/
#include <math.h>

#include "PVRShell.h"
#include "OGLES2Tools.h"

/******************************************************************************
 Constants
******************************************************************************/

// Camera constants. Used for making the projection matrix
const float CAM_FOV  = PVRT_PI / 6;
const float CAM_NEAR = 50.0f;

// Index to bind the attributes to vertex shaders
const int VERTEX_ARRAY   = 0;
const int NORMAL_ARRAY   = 1;
const int TEXCOORD_ARRAY = 2;

// Enum and List to select the fog function
enum EFogMode
{
	eNoFog = 0,
	eLinearFog,
	eExponentialFog,
	eExponentialSquaredFog,
	eNumFogModes
};

const char * const g_FogFunctionList[] = { "Fog Mode: No fog", "Fog Mode: Linear", "Fog Mode: Exponential", "Fog Mode: Exponential squared" };

/******************************************************************************
 Content file names
******************************************************************************/

// Source and binary shaders
const char c_szFragShaderSrcFile[]	= "FragShader.fsh";
const char c_szFragShaderBinFile[]	= "FragShader.fsc";
const char c_szVertShaderSrcFile[]	= "VertShader.vsh";
const char c_szVertShaderBinFile[]	= "VertShader.vsc";

// PVR texture files
const char c_szTextureFile[]		= "Basetex.pvr";

// POD scene files
const char c_szSceneFile[]			= "Mask.pod";

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES2Fog : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// 3D Model
	CPVRTModelPOD	m_Scene;

	// Projection and Model View matrices
	PVRTMat4 m_mView, m_mProjection;

	// OpenGL handles for shaders, textures and VBOs
	GLuint m_uiVertShader;
	GLuint m_uiFragShader;
	GLuint m_uiTexture;
	GLuint* m_puiVbo;
	GLuint* m_puiIndexVbo;

	// Group shader programs and their uniform locations together
	struct
	{
		GLuint uiId;

		GLuint uiModelViewLoc;
		GLuint uiMVPMatrixLoc;
		GLuint uiLightDirLoc;
		GLuint uiFogFuncLoc;
		GLuint uiFogDensityLoc;
		GLuint uiFogEndLoc;
		GLuint uiFogRcpDiffLoc;
		GLuint uiFogColorLoc;
	}
	m_ShaderProgram;

	// The translation and Rotate parameter of Model
	float			m_fAngleY;
	float			m_fPositionZ;

	// Selecting Fog function
	EFogMode		m_eFogMode;

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
};

/*!****************************************************************************
 @Function		LoadTextures
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course
******************************************************************************/
bool OGLES2Fog::LoadTextures(CPVRTString* const pErrorStr)
{
	if(PVRTTextureLoadFromPVR(c_szTextureFile, &m_uiTexture) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Failed to load texture.";
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
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
bool OGLES2Fog::LoadShaders(CPVRTString* pErrorStr)
{
	/*
		Load and compile the shaders from files.
		Binary shaders are tried first, source shaders
		are used as fallback.
	*/
	if (PVRTShaderLoadFromFile(
			c_szVertShaderBinFile, c_szVertShaderSrcFile,
			GL_VERTEX_SHADER, GL_SGX_BINARY_IMG,
			&m_uiVertShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	if (PVRTShaderLoadFromFile(
			c_szFragShaderBinFile, c_szFragShaderSrcFile,
			GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG,
			&m_uiFragShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	/*
		Set up and link the shader program
	*/
	// Set up a mapping of attribute names to locations
	const char* aszAttribs[] = { "inVertex", "inNormal", "inTexCoord" };

	if (PVRTCreateProgram(&m_ShaderProgram.uiId, m_uiVertShader, m_uiFragShader, aszAttribs, 3, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}
	// Set the sampler2D variable to the first texture unit
	glUniform1i(glGetUniformLocation(m_ShaderProgram.uiId, "sTexture"), 0);

	// Store the location of uniforms for later use
	m_ShaderProgram.uiModelViewLoc  = glGetUniformLocation(m_ShaderProgram.uiId, "ModelViewMatrix");
	m_ShaderProgram.uiMVPMatrixLoc  = glGetUniformLocation(m_ShaderProgram.uiId, "MVPMatrix");
	m_ShaderProgram.uiLightDirLoc   = glGetUniformLocation(m_ShaderProgram.uiId, "LightDirection");
	m_ShaderProgram.uiFogFuncLoc    = glGetUniformLocation(m_ShaderProgram.uiId, "iFogMode");
	m_ShaderProgram.uiFogDensityLoc = glGetUniformLocation(m_ShaderProgram.uiId, "FogDensity");
	m_ShaderProgram.uiFogEndLoc     = glGetUniformLocation(m_ShaderProgram.uiId, "FogEnd");
	m_ShaderProgram.uiFogRcpDiffLoc = glGetUniformLocation(m_ShaderProgram.uiId, "FogRcpEndStartDiff");
	m_ShaderProgram.uiFogColorLoc   = glGetUniformLocation(m_ShaderProgram.uiId, "FogColor");

	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLES2Fog::LoadVbos()
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
 @Return		bool		true if no error occurred
 @Description	Code in InitApplication() will be called by PVRShell once per
				run, before the rendering context is created.
				Used to initialize variables that are not dependant on it
				(e.g. external modules, loading meshes, etc.)
				If the rendering context is lost, InitApplication() will
				not be called again.
******************************************************************************/
bool OGLES2Fog::InitApplication()
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
	if (m_Scene.ReadFromFile(c_szSceneFile) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Couldn't load the .pod file\n");
		return false;
	}

	m_fAngleY = 0.0f;
	m_fPositionZ = 0.0f;
	m_eFogMode = eLinearFog;

	return true;
}

/*!****************************************************************************
 @Function		QuitApplication
 @Return		bool		true if no error occurred
 @Description	Code in QuitApplication() will be called by PVRShell once per
				run, just before exiting the program.
				If the rendering context is lost, QuitApplication() will
				not be called.x
******************************************************************************/
bool OGLES2Fog::QuitApplication()
{
	// Frees the memory allocated for the scene
	m_Scene.Destroy();

	delete [] m_puiVbo;
	delete [] m_puiIndexVbo;

	return true;
}

/*!****************************************************************************
 @Function		InitView
 @Return		bool		true if no error occurred
 @Description	Code in InitView() will be called by PVRShell upon
				initialization or after a change in the rendering context.
				Used to initialize variables that are dependent on the rendering
				context (e.g. textures, vertex buffers, etc.)
******************************************************************************/
bool OGLES2Fog::InitView()
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

	// Is the screen rotated
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);
	/*
		Initialize Print3D
	*/
	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	/*
		Calculate the projection and view matrices
	*/
	float fAspect = PVRShellGet(prefWidth) / (float)PVRShellGet(prefHeight);
	m_mProjection = PVRTMat4::PerspectiveFovFloatDepthRH(CAM_FOV, fAspect, CAM_NEAR, PVRTMat4::OGL, bRotate);
	m_mView = PVRTMat4::LookAtRH(PVRTVec3(0.f, 0.f, 150.f), PVRTVec3(0.f), PVRTVec3(0.f, 1.f, 0.f));

	/*
		Set OpenGL ES render states needed for this training course
	*/
	// Enable z-buffer test
	// We are using a projection matrix optimized for a floating point depth buffer,
	// so the depth test and clear value need to be inverted (1 becomes near, 0 becomes far).
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_GEQUAL);
	glClearDepthf(0.0f);

	// Use a nice bright blue as clear colour
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	/*
		Set up constant fog shader uniforms
	*/
	const float fFogStart = 0.0f;
	const float fFogEnd = 1200.0f;
	const float fFogDensity = 0.002f;
	const float fFogRcpEndStartDiff = 1.0f / (fFogEnd - fFogStart);
	const float afFogColor[3] = { 0.6f, 0.8f, 1.0f }; // the fog RGB color

	glUniform1f(m_ShaderProgram.uiFogEndLoc, fFogEnd);
	glUniform1f(m_ShaderProgram.uiFogRcpDiffLoc, fFogRcpEndStartDiff);
	glUniform1f(m_ShaderProgram.uiFogDensityLoc, fFogDensity);
	glUniform3fv(m_ShaderProgram.uiFogColorLoc, 1, afFogColor);

	// Enable culling
	glEnable(GL_CULL_FACE);
	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occurred
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES2Fog::ReleaseView()
{
	// Delete textures
	glDeleteTextures(1, &m_uiTexture);

	// Delete program and shader objects
	glDeleteProgram(m_ShaderProgram.uiId);

	glDeleteShader(m_uiVertShader);
	glDeleteShader(m_uiFragShader);

	// Delete buffer objects
	glDeleteBuffers(m_Scene.nNumMesh, m_puiVbo);
	glDeleteBuffers(m_Scene.nNumMesh, m_puiIndexVbo);

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

	return true;
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
bool OGLES2Fog::RenderScene()
{
	// Clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Keyboard input (cursor to change fog function)
	if (PVRShellIsKeyPressed(PVRShellKeyNameLEFT))
	{
		m_eFogMode = EFogMode((m_eFogMode + eNumFogModes - 1) % eNumFogModes);
	}
	if (PVRShellIsKeyPressed(PVRShellKeyNameRIGHT))
	{
		m_eFogMode = EFogMode((m_eFogMode + 1) % eNumFogModes);
	}

	// Use the loaded shader program
	glUseProgram(m_ShaderProgram.uiId);

	// Bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uiTexture);

	// Set uniforms
	glUniform1i(m_ShaderProgram.uiFogFuncLoc, m_eFogMode);

	// Rotate and translate the model matrix
	PVRTMat4 mModel = PVRTMat4::RotationY(m_fAngleY);
	m_fAngleY += PVRT_PI / 90;
	mModel.preTranslate(0, 0, 500 * cos(m_fPositionZ) - 450);
	m_fPositionZ += (2*PVRT_PI)*0.0008f;

	// Feed Projection and Model View matrices to the shaders
	PVRTMat4 mModelView = m_mView * mModel;
	PVRTMat4 mMVP = m_mProjection * mModelView;

	glUniformMatrix4fv(m_ShaderProgram.uiModelViewLoc, 1, GL_FALSE, mModelView.ptr());
	glUniformMatrix4fv(m_ShaderProgram.uiMVPMatrixLoc, 1, GL_FALSE, mMVP.ptr());

	// Pass the light direction transformed with the inverse of the ModelView matrix
	// This saves the transformation of the normals per vertex. A simple dot3 between this direction
	// and the un-transformed normal will allow proper smooth shading.
	PVRTVec3 vMsLightDir = (PVRTMat3(mModel).inverse() * PVRTVec3(1, 1, 1)).normalized();
	glUniform3fv(m_ShaderProgram.uiLightDirLoc, 1, vMsLightDir.ptr());

	/*
		Now that the model-view matrix is set and the materials ready,
		call another function to actually draw the mesh.
	*/
	DrawMesh(0);

	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("Fog", g_FogFunctionList[m_eFogMode], ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			mesh		The mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the meterial prepared.
******************************************************************************/
void OGLES2Fog::DrawMesh(int i32NodeIndex)
{
	int i32MeshIndex = m_Scene.pNode[i32NodeIndex].nIdx;
	SPODMesh* pMesh = &m_Scene.pMesh[i32MeshIndex];

	//// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[i32MeshIndex]);
	//// bind the index buffer, won't hurt if the handle is 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[i32MeshIndex]);

	// Enable the vertex attribute arrays
	glEnableVertexAttribArray(VERTEX_ARRAY);
	glEnableVertexAttribArray(NORMAL_ARRAY);
	glEnableVertexAttribArray(TEXCOORD_ARRAY);

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
		for(int i = 0; i < (int)pMesh->nNumStrips; ++i)
		{
			int offset = 0;
			if(m_puiIndexVbo[i32MeshIndex])
			{
				// Indexed Triangle strips
				glDrawElements(GL_TRIANGLE_STRIP, pMesh->pnStripLength[i]+2, GL_UNSIGNED_SHORT, (GLshort*)(offset*2));
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
	glDisableVertexAttribArray(VERTEX_ARRAY);
	glDisableVertexAttribArray(NORMAL_ARRAY);
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
	return new OGLES2Fog();
}

/******************************************************************************
 End of file (OGLES2Fog.cpp)
******************************************************************************/

