/******************************************************************************

 @File         OGLES2Reflections.cpp

 @Title        Template

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Template

******************************************************************************/

#include "PVRShell.h"
#include "OGLES2Tools.h"

/******************************************************************************
 Constants
******************************************************************************/

// Camera constants used to generate the projection matrix
const float CAM_NEAR	= 75.0f;
const float CAM_FAR		= 2000.0f;

// reflection function description
const char* const g_MappingList[] = {"Reflection: 2D Mapping", "Reflection: Cube Mapping"};

/******************************************************************************
 shader attributes
******************************************************************************/
// vertex attributes
enum EVertexAttrib {
	VERTEX_ARRAY, NORMAL_ARRAY, eNumAttribs };
const char* g_aszAttribNames[] = {
	"inVertex", "inNormal" };

// shader uniforms
enum EUniform {
	eMVPMatrix, eModelWorld, eEyePosModel, eCubeReflect, eNumUniforms };
const char* g_aszUniformNames[] = {
	"MVPMatrix", "ModelWorld", "EyePosModel", "bCubeReflection" };

/******************************************************************************
 Content file names
******************************************************************************/

// Source and binary shaders
const char c_szFragShaderSrcFile[]	= "FragShader.fsh";
const char c_szFragShaderBinFile[]	= "FragShader.fsc";
const char c_szVertShaderSrcFile[]	= "VertShader.vsh";
const char c_szVertShaderBinFile[]	= "VertShader.vsc";

// PVR texture files
const char c_sz2dTexFile[]			= "Basetex.pvr";
const char c_szCubeTexFile[]		= "Cubemap.pvr";

// POD scene files
const char c_szSceneFile[]			= "Mask.pod";


/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES2Reflections : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// 3D Model
	CPVRTModelPOD	m_Scene;

	// Projection and view matrices
	PVRTMat4		m_mProjection, m_mView;

	// OpenGL handles for shaders, textures and VBOs
	GLuint	m_uiVertShader;
	GLuint	m_uiFragShader;
	GLuint	m_ui2dTex;
	GLuint	m_uiCubeTex;
	GLuint*	m_puiVbo;
	GLuint*	m_puiIndexVbo;

	// Group shader programs and their uniform locations together
	struct
	{
		GLuint uiId;
		GLuint auiLoc[eNumUniforms];
	}
	m_ShaderProgram;

	// Rotation angle for the model
	float	m_fAngleX;
	float	m_fAngleY;

	// Select either 2D or cube map reflection
	bool	m_bCubeReflection;

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
bool OGLES2Reflections::LoadTextures(CPVRTString* const pErrorStr)
{
	if(PVRTTextureLoadFromPVR(c_sz2dTexFile, &m_ui2dTex) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Failed to load texture.";
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szCubeTexFile, &m_uiCubeTex) != PVR_SUCCESS)
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
bool OGLES2Reflections::LoadShaders(CPVRTString* pErrorStr)
{
	/*
		Load and compile the shaders from files.
		Binary shaders are tried first, source shaders
		are used as fallback.
	*/
	if (PVRTShaderLoadFromFile(
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

	if (PVRTCreateProgram(&m_ShaderProgram.uiId, m_uiVertShader, m_uiFragShader, g_aszAttribNames, eNumAttribs, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	// Store the location of uniforms for later use
	for (int i = 0; i < eNumUniforms; ++i)
	{
		m_ShaderProgram.auiLoc[i] = glGetUniformLocation(m_ShaderProgram.uiId, g_aszUniformNames[i]);
	}

	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLES2Reflections::LoadVbos()
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
bool OGLES2Reflections::InitApplication()
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

	m_bCubeReflection = true;
	m_fAngleX = 0.0f;
	m_fAngleY = 0.0f;

	return true;
}

/*!****************************************************************************
 @Function		QuitApplication
 @Return		bool		true if no error occured
 @Description	Code in QuitApplication() will be called by PVRShell once per
				run, just before exiting the program.
				If the rendering context is lost, QuitApplication() will
				not be called.x
******************************************************************************/
bool OGLES2Reflections::QuitApplication()
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
bool OGLES2Reflections::InitView()
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
	glUniform1i(glGetUniformLocation(m_ShaderProgram.uiId, "s2DMap"), 0);
	glUniform1i(glGetUniformLocation(m_ShaderProgram.uiId, "sCubeMap"), 1);

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

	/*
		Calculate the projection and view matrices
	*/
	m_mProjection = PVRTMat4::PerspectiveFovRH(PVRT_PI/6, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), CAM_NEAR, CAM_FAR, PVRTMat4::OGL, bRotate);

	m_mView = PVRTMat4::LookAtRH(PVRTVec3(0, 0, 150), PVRTVec3(0, 0, 0), PVRTVec3(0, 1, 0));

	/*
		Set OpenGL ES render states needed for this training course
	*/
	// Enable backface culling and depth test
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);

	// Use a nice bright blue as clear colour
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES2Reflections::ReleaseView()
{
	// Delete textures
	glDeleteTextures(1, &m_ui2dTex);
	glDeleteTextures(1, &m_uiCubeTex);

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
 @Return		bool		true if no error occured
 @Description	Main rendering loop function of the program. The shell will
				call this function every frame.
				eglSwapBuffers() will be performed by PVRShell automatically.
				PVRShell will also manage important OS events.
				Will also manage relevent OS events. The user has access to
				these events through an abstraction layer provided by PVRShell.
******************************************************************************/
bool OGLES2Reflections::RenderScene()
{
	// Clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Keyboard input (cursor keys to change reflection)
	if (PVRShellIsKeyPressed(PVRShellKeyNameLEFT) || PVRShellIsKeyPressed(PVRShellKeyNameRIGHT))
	{
		m_bCubeReflection = !m_bCubeReflection;
	}

	// Use shader program
	glUseProgram(m_ShaderProgram.uiId);

	// Bind textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_ui2dTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_uiCubeTex);

	// Set reflection flag
	glUniform1i(m_ShaderProgram.auiLoc[eCubeReflect], m_bCubeReflection);

	// Rotate the model matrix
	PVRTMat4 mModel, mRotX, mRotY;
	mRotX = PVRTMat4::RotationX(m_fAngleX);
	mRotY = PVRTMat4::RotationY(m_fAngleY);

	mModel = mRotY * mRotX;

	m_fAngleX += 0.01f;
	m_fAngleY += 0.011f;

	// Set model view projection matrix
	PVRTMat4 mModelView, mMVP;
	mModelView = m_mView * mModel;
	mMVP =  m_mProjection * mModelView;

	glUniformMatrix4fv(m_ShaderProgram.auiLoc[eMVPMatrix], 1, GL_FALSE, mMVP.ptr());

	// Set model matrix
	PVRTMat3 Model3x3 = PVRTMat3(mModel);

	glUniformMatrix3fv(m_ShaderProgram.auiLoc[eModelWorld], 1, GL_FALSE, Model3x3.ptr());

	// Set eye position in model space
	PVRTVec4 vEyePosModel;
	vEyePosModel = mModelView.inverse() * PVRTVec4(0, 0, 0, 1);

	glUniform3fv(m_ShaderProgram.auiLoc[eEyePosModel], 1, &vEyePosModel.x);

	/*
		Now that the uniforms are set, call another function to actually draw the mesh.
	*/
	DrawMesh(0);

	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("Reflections", g_MappingList[m_bCubeReflection], ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			i32NodeIndex		Node index of the mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the meterial prepared.
******************************************************************************/
void OGLES2Reflections::DrawMesh(int i32NodeIndex)
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
 @Function		NewDemo
 @Return		PVRShell*		The demo supplied by the user
 @Description	This function must be implemented by the user of the shell.
				The user should return its PVRShell object defining the
				behaviour of the application.
******************************************************************************/
PVRShell* NewDemo()
{
	return new OGLES2Reflections();
}

/******************************************************************************
 End of file (OGLES2Reflections.cpp)
******************************************************************************/

