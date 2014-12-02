/******************************************************************************

 @File         OGLES3AlphaTest.cpp

 @Title        Shows alpha testing

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows the advantage of alpha testing over alpha blending in cases
               where per pixel sorting is required (e.g., interlaced polygons in
               tree-tops or bushes where sorting by hand is not possible). The
               depth test will not be applied to fully transparent pixels in this
               case. Alpha test is slower than alpha blending so use it sparingly
               and only when absolutely necessary.

******************************************************************************/
#include "PVRShell.h"
#include "OGLES3Tools.h"

#include <stddef.h>



/******************************************************************************
 Constants
******************************************************************************/

// Camera constants. Used for making the projection matrix
const float g_fFOV  = 0.5f;
const float g_fNear = 0.01f;

// Index to bind the attributes to vertex shaders
const unsigned int g_ui32VertexArray   = 0;
const unsigned int g_ui32TexcoordArray = 1;

const unsigned int g_ui32TriangleNo  = 8;

/******************************************************************************
 Content file names
******************************************************************************/

// Source and binary shaders
const char c_szTexFragShaderSrcFile[]		= "TexFragShader.fsh";
const char c_szTexFragShaderBinFile[]		= "TexFragShader.fsc";
const char c_szDiscardFragShaderSrcFile[]	= "DiscardFragShader.fsh";
const char c_szDiscardFragShaderBinFile[]	= "DiscardFragShader.fsc";
const char c_szVertShaderSrcFile[]			= "VertShader.vsh";
const char c_szVertShaderBinFile[]			= "VertShader.vsc";

// PVR texture files
const char c_szTextureFile[]		= "Wallwire.pvr";

struct SVertexFormat
{
	float x, y, z;
	float u, v;
};

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES3AlphaTest : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// View and projection matrix
	PVRTMat4 m_mViewProj;

	// OpenGL handles for shaders, textures and VBOs
	GLuint m_uiVertShader;
	GLuint m_uiTexFragShader;
	GLuint m_uiDiscardFragShader;
	GLuint m_uiTexture;
	GLuint m_uiVbo;
	GLuint m_uiIndexVbo;

	// Group shader programs and their uniform locations together
	struct
	{
		GLuint uiId;
		GLuint uiMVPMatrixLoc;
		GLuint uiAlphaRefLoc;
	}
	m_DiscardShaderProgram;

	struct
	{
		GLuint uiId;
		GLuint uiMVPMatrixLoc;
	}
	m_TexShaderProgram;

	// Angle to rotate the meshes
	float			m_fAngleY;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	bool LoadTextures(CPVRTString* pErrorStr);
	bool LoadShaders(CPVRTString* pErrorStr);
	void LoadVbos();

	void DrawModel();
};

/*!****************************************************************************
 @Function		LoadTextures
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course
******************************************************************************/
bool OGLES3AlphaTest::LoadTextures(CPVRTString* const pErrorStr)
{
	if(PVRTTextureLoadFromPVR(c_szTextureFile, &m_uiTexture) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Failed to load texture.";
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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
bool OGLES3AlphaTest::LoadShaders(CPVRTString* pErrorStr)
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
			c_szTexFragShaderBinFile, c_szTexFragShaderSrcFile,
			GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG,
			&m_uiTexFragShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	if (PVRTShaderLoadFromFile(
			c_szDiscardFragShaderBinFile, c_szDiscardFragShaderSrcFile,
			GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG,
			&m_uiDiscardFragShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	/*
		Set up and link the shader program
	*/
	const char* aszAttribs[] = { "inVertex", "inTexCoord" };
	// Shader program for alpha blend
	if (PVRTCreateProgram(&m_TexShaderProgram.uiId, m_uiVertShader, m_uiTexFragShader, aszAttribs, 2, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	// Set the sampler2D variable to the first texture unit
	glUniform1i(glGetUniformLocation(m_TexShaderProgram.uiId, "sTexture"), 0);

	// Store the location of uniforms for later use
	m_TexShaderProgram.uiMVPMatrixLoc = glGetUniformLocation(m_TexShaderProgram.uiId, "MVPMatrix");

	// Shader program for alpha test
	if (PVRTCreateProgram(&m_DiscardShaderProgram.uiId, m_uiVertShader, m_uiDiscardFragShader, aszAttribs, 2, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}
	// Set the sampler2D variable to the first texture unit
	glUniform1i(glGetUniformLocation(m_DiscardShaderProgram.uiId, "sTexture"), 0);

	// Store the location of uniforms for later use
	m_DiscardShaderProgram.uiMVPMatrixLoc = glGetUniformLocation(m_DiscardShaderProgram.uiId, "MVPMatrix");
	m_DiscardShaderProgram.uiAlphaRefLoc = glGetUniformLocation(m_DiscardShaderProgram.uiId, "AlphaReference");

	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLES3AlphaTest::LoadVbos()
{
	static SVertexFormat aVertexData[] = {
		{ -0.5f, -0.5f, -0.1f,  0, 0 }, { +0.5f, -0.5f, -0.1f,  1, 0 },
		{ -0.5f, +0.5f, -0.1f,  0, 1 }, { +0.5f, +0.5f, -0.1f,  1, 1 },

		{ -0.5f, -0.5f, +0.1f,  0, 0 }, { +0.5f, -0.5f, +0.1f,  1, 0 },
		{ -0.5f, +0.5f, +0.1f,  0, 1 }, { +0.5f, +0.5f, +0.1f,  1, 1 },

		{ -0.1f, -0.5f, -0.5f,  0, 0 }, { -0.1f, +0.5f, -0.5f,  1, 0 },
		{ -0.1f, -0.5f, +0.5f,  0, 1 }, { -0.1f, +0.5f, +0.5f,  1, 1 },

		{ +0.1f, -0.5f, -0.5f,  0, 0 }, { +0.1f, +0.5f, -0.5f,  1, 0 },
		{ +0.1f, -0.5f, +0.5f,  0, 1 }, { +0.1f, +0.5f, +0.5f,  1, 1 },
	};

	static GLushort aIndices[] = {
		0,  1,  2,  2,  1,  3,
		4,  5,  6,  6,  5,  7,
		8,  9,  10, 10, 9,  11,
		12, 13, 14, 14, 13, 15
	};

	glGenBuffers(1, &m_uiVbo);
	glGenBuffers(1, &m_uiIndexVbo);

	glBindBuffer(GL_ARRAY_BUFFER, m_uiVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(aVertexData), aVertexData, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiIndexVbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(aIndices), aIndices, GL_STATIC_DRAW);

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
bool OGLES3AlphaTest::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	m_fAngleY = 0;

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
bool OGLES3AlphaTest::QuitApplication()
{
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
bool OGLES3AlphaTest::InitView()
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

	// Is the screen rotated?
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	/*
		Initialize Print3D
	*/
	if(m_Print3D.SetTextures(0, PVRShellGet(prefWidth), PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	/*
		Calculate the projection and view matrices
	*/
	float fAspect = PVRShellGet(prefWidth) / (float)PVRShellGet(prefHeight);
	m_mViewProj = PVRTMat4::PerspectiveFovFloatDepthRH(g_fFOV, fAspect, g_fNear, PVRTMat4::OGL, bRotate);
	m_mViewProj *= PVRTMat4::LookAtRH(PVRTVec3(0, 2, -2.5f), PVRTVec3(0, 0.2f, 0), PVRTVec3(0, 1, 0));

	/*
		Set OpenGL ES render states needed for this training course
	*/
	// Use a nice bright blue as clear colour
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	// Enable z-buffer test
	// We are using a projection matrix optimized for a floating point depth buffer,
	// so the depth test and clear value need to be inverted (1 becomes near, 0 becomes far).
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_GEQUAL);
	glClearDepthf(0.0f);

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES3AlphaTest::ReleaseView()
{
	// Delete textures
	glDeleteTextures(1, &m_uiTexture);

	// Delete program and shader objects
	glDeleteProgram(m_TexShaderProgram.uiId);
	glDeleteProgram(m_DiscardShaderProgram.uiId);

	glDeleteShader(m_uiVertShader);
	glDeleteShader(m_uiTexFragShader);
	glDeleteShader(m_uiDiscardFragShader);

	// Delete buffer objects
	glDeleteBuffers(1, &m_uiVbo);
	glDeleteBuffers(1, &m_uiIndexVbo);

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
bool OGLES3AlphaTest::RenderScene()
{
	// Clear color and z buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set texture
	glBindTexture(GL_TEXTURE_2D, m_uiTexture);

	/*
		Draw the left cube using alpha blending
	*/
	glUseProgram(m_TexShaderProgram.uiId);

	glEnable(GL_BLEND);

	// Setup blending for transparency
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Calculate the model matrix for the left cube
	PVRTMat4 mModel = PVRTMat4::RotationY(m_fAngleY);
	m_fAngleY += .005f;

	mModel.preTranslate(0.6f, 0, 0);

	// Calculate the model view projection (MVP) matrix and pass it to the shader
	PVRTMat4 mMVP = m_mViewProj * mModel;
	glUniformMatrix4fv(m_TexShaderProgram.uiMVPMatrixLoc, 1, GL_FALSE, mMVP.ptr());

	// Draw left cube
	DrawModel();

	/*
		Draw the right cube using alpha test.
	*/
	glUseProgram(m_DiscardShaderProgram.uiId);

	glDisable(GL_BLEND);

	// Set alpha test to discard fragments with an alpha value of less than 0.2
	glUniform1f(m_DiscardShaderProgram.uiAlphaRefLoc, 0.2f);

	// Calculate the model matrix for the right cube
	mModel.preTranslate(-1.2f, 0, 0);

	// Calculate the model view projection (MVP) matrix and pass it to the shader
	mMVP = m_mViewProj * mModel;
	glUniformMatrix4fv(m_DiscardShaderProgram.uiMVPMatrixLoc, 1, GL_FALSE, mMVP.ptr());

	// Draw right cube
	DrawModel();

	// Display the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("AlphaTest", "", ePVRTPrint3DSDKLogo);
	m_Print3D.Print3D(10.0f, 10.0f, 1.0f, 0xFFFF00FF, "Alpha Blend");
	m_Print3D.Print3D(60.0f, 10.0f, 1.0f, 0xFFFF00FF, "Alpha Test");
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		DrawModel
 @Description	Draws 4 intersecting rectangles.
******************************************************************************/
void OGLES3AlphaTest::DrawModel()
{
	// Set up vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, m_uiVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiIndexVbo);

	glEnableVertexAttribArray(g_ui32VertexArray);
	glEnableVertexAttribArray(g_ui32TexcoordArray);

	glVertexAttribPointer(g_ui32VertexArray, 3, GL_FLOAT, GL_FALSE, sizeof(SVertexFormat), (void*)offsetof(SVertexFormat, x));
	glVertexAttribPointer(g_ui32TexcoordArray, 2, GL_FLOAT, GL_FALSE, sizeof(SVertexFormat), (void*)offsetof(SVertexFormat, u));

	// Draws an indexed triangle list
	glDrawElements(GL_TRIANGLES, 3 * g_ui32TriangleNo, GL_UNSIGNED_SHORT, 0);

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
	return new OGLES3AlphaTest();
}

/******************************************************************************
 End of file (OGLES3AlphaTest.cpp)
******************************************************************************/

