/******************************************************************************

 @File         OGLES3AnisotropicLighting.cpp

 @Title        Anisotropic Lighting

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows two methods to create an anisotropic lighting effect.

******************************************************************************/
#include "PVRShell.h"
#include "OGLES3Tools.h"

#include <math.h>



/******************************************************************************
 Constants
******************************************************************************/

// Camera constants. Used for making the projection matrix
const float CAM_FOV  = PVRT_PI / 6;
const float CAM_NEAR = 4.0f;

// Index to bind the attributes to vertex shaders
const int VERTEX_ARRAY = 0;
const int NORMAL_ARRAY = 1;

enum ERenderMode
{
	eTexLookup = 0,
	eMath,
	eNumRenderModes
};

const char* c_aszRenderModes[] = { "with texture lookup", "per vertex" };

/******************************************************************************
 Content file names
******************************************************************************/

// Source and binary shaders
const char c_szFastFsSrcFile[]	= "FastFragShader.fsh";
const char c_szFastFsBinFile[]	= "FastFragShader.fsc";
const char c_szFastVsSrcFile[]	= "FastVertShader.vsh";
const char c_szFastVsBinFile[]	= "FastVertShader.vsc";
const char c_szSlowFsSrcFile[]	= "SlowFragShader.fsh";
const char c_szSlowFsBinFile[]	= "SlowFragShader.fsc";
const char c_szSlowVsSrcFile[]	= "SlowVertShader.vsh";
const char c_szSlowVsBinFile[]	= "SlowVertShader.vsc";

// PVR texture files
const char c_szTextureFile[]		= "Basetex.pvr";

// POD scene files
const char c_szSceneFile[]			= "Mask.pod";

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES3AnisotropicLighting : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// 3D Model
	CPVRTModelPOD	m_Scene;

	// Projection and Model View matrices
	PVRTMat4 m_mViewProj;

	// OpenGL handles for shaders, textures and VBOs
	GLuint m_uiFastVertShader;
	GLuint m_uiFastFragShader;
	GLuint m_uiSlowVertShader;
	GLuint m_uiSlowFragShader;
	GLuint m_uiTexture;
	GLuint* m_puiVbo;
	GLuint* m_puiIndexVbo;

	// Group shader programs and their uniform locations together
	struct SFastShader
	{
		GLuint uiId;
		GLuint uiMVPMatrixLoc;
		GLuint uiMsLightDirLoc;
		GLuint uiMsEyePosLoc;
	}
	m_FastShader;

	struct SSlowShader
	{
		GLuint uiId;
		GLuint uiMVPMatrixLoc;
		GLuint uiMsLightDirLoc;
		GLuint uiMsEyeDirLoc;
	}
	m_SlowShader;

	// View Angle for animation
	float m_fAngleY;

	ERenderMode m_eRenderMode;

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
bool OGLES3AnisotropicLighting::LoadTextures(CPVRTString* const pErrorStr)
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
bool OGLES3AnisotropicLighting::LoadShaders(CPVRTString* pErrorStr)
{
	/*
		Load and compile the shaders from files.
		Binary shaders are tried first, source shaders
		are used as fallback.
	*/
	if (PVRTShaderLoadFromFile(c_szFastVsBinFile,
			                   c_szFastVsSrcFile,
							   GL_VERTEX_SHADER,
							   GL_SGX_BINARY_IMG,
							   &m_uiFastVertShader,
							   pErrorStr) != PVR_SUCCESS)
	{
		*pErrorStr = CPVRTString("From file '") + c_szFastVsBinFile +
					 "' or '" + c_szFastVsSrcFile + "':\n" + *pErrorStr;
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	if (PVRTShaderLoadFromFile(c_szFastFsBinFile,
							   c_szFastFsSrcFile,
							   GL_FRAGMENT_SHADER,
							   GL_SGX_BINARY_IMG,
							   &m_uiFastFragShader,
							   pErrorStr) != PVR_SUCCESS)
	{
		*pErrorStr = CPVRTString("From file '") + c_szFastVsBinFile +
					 "' or '" + c_szFastVsSrcFile + "':\n" + *pErrorStr;
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	if (PVRTShaderLoadFromFile(c_szSlowVsBinFile,
							   c_szSlowVsSrcFile,
							   GL_VERTEX_SHADER,
							   GL_SGX_BINARY_IMG,
							   &m_uiSlowVertShader,
							   pErrorStr) != PVR_SUCCESS)
	{
		*pErrorStr = CPVRTString("From file '") + c_szFastVsBinFile +
					 "' or '" + c_szFastVsSrcFile + "':\n" + *pErrorStr;
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	if (PVRTShaderLoadFromFile(c_szSlowFsBinFile,
							   c_szSlowFsSrcFile,
							   GL_FRAGMENT_SHADER,
							   GL_SGX_BINARY_IMG,
							   &m_uiSlowFragShader,
							   pErrorStr) != PVR_SUCCESS)
	{
		*pErrorStr = CPVRTString("From file '") + c_szFastVsBinFile +
					 "' or '" + c_szFastVsSrcFile + "':\n" + *pErrorStr;
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	/*
		Set up and link the shader programs
	*/
	const char* aszAttribs[] = { "inVertex", "inNormal"	};
	if	(
		(PVRTCreateProgram(&m_FastShader.uiId, m_uiFastVertShader, m_uiFastFragShader, aszAttribs, 2, pErrorStr) != PVR_SUCCESS) ||
		(PVRTCreateProgram(&m_SlowShader.uiId, m_uiSlowVertShader, m_uiSlowFragShader, aszAttribs, 2, pErrorStr) != PVR_SUCCESS)
		)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	/*
		Store the location of uniforms for later use
	*/
	m_FastShader.uiMVPMatrixLoc		= glGetUniformLocation(m_FastShader.uiId, "MVPMatrix");
	m_FastShader.uiMsLightDirLoc	= glGetUniformLocation(m_FastShader.uiId, "msLightDir");
	m_FastShader.uiMsEyePosLoc		= glGetUniformLocation(m_FastShader.uiId, "msEyePos");

	m_SlowShader.uiMVPMatrixLoc		= glGetUniformLocation(m_SlowShader.uiId, "MVPMatrix");
	m_SlowShader.uiMsLightDirLoc	= glGetUniformLocation(m_SlowShader.uiId, "msLightDir");
	m_SlowShader.uiMsEyeDirLoc		= glGetUniformLocation(m_SlowShader.uiId, "msEyeDir");

	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLES3AnisotropicLighting::LoadVbos()
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
bool OGLES3AnisotropicLighting::InitApplication()
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
bool OGLES3AnisotropicLighting::QuitApplication()
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
bool OGLES3AnisotropicLighting::InitView()
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
	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	/*
		Calculate the projection and view matrices
	*/
	float fAspect = PVRShellGet(prefWidth) / (float)PVRShellGet(prefHeight);
	m_mViewProj = PVRTMat4::PerspectiveFovFloatDepthRH(CAM_FOV, fAspect, CAM_NEAR, PVRTMat4::OGL, bRotate);
	m_mViewProj *= PVRTMat4::LookAtRH(PVRTVec3(0.f, 0.f, 150.f), PVRTVec3(0.f), PVRTVec3(0.f, 1.f, 0.f));

	/*
		Set uniforms that are constant throughout this training course
	*/
	// Set the sampler2D variable to the first texture unit
	glUseProgram(m_FastShader.uiId);
	glUniform1i(glGetUniformLocation(m_FastShader.uiId, "sTexture"), 0);

	// Define material properties
	glUseProgram(m_SlowShader.uiId);
	float afMaterial[4] = {
		0.4f,	// Diffuse intensity scale
		0.6f,	// Diffuse intensity bias
		0.82f,	// Specular intensity scale
		0.0f,	// Specular bias
	};
	glUniform4fv(glGetUniformLocation(m_SlowShader.uiId, "Material"), 1, afMaterial);
	// Set surface grain direction
	PVRTVec3 vMsGrainDir = PVRTVec3(2, 1, 0).normalized();
	glUniform3fv(glGetUniformLocation(m_SlowShader.uiId, "GrainDir"), 1, vMsGrainDir.ptr());

	/*
		Set OpenGL ES render states needed for this training course
	*/
	// Enable backface culling and depth test
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	// Enable z-buffer test
	// We are using a projection matrix optimized for a floating point depth buffer,
	// so the depth test and clear value need to be inverted (1 becomes near, 0 becomes far).
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_GEQUAL);
	glClearDepthf(0.0f);

	// Use a nice bright blue as clear colour
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	m_fAngleY = 0;
	m_eRenderMode = eTexLookup;

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES3AnisotropicLighting::ReleaseView()
{
	// Delete textures
	glDeleteTextures(1, &m_uiTexture);

	// Delete program and shader objects
	glDeleteProgram(m_FastShader.uiId);
	glDeleteProgram(m_SlowShader.uiId);

	glDeleteShader(m_uiFastVertShader);
	glDeleteShader(m_uiFastFragShader);
	glDeleteShader(m_uiSlowVertShader);
	glDeleteShader(m_uiSlowFragShader);

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
bool OGLES3AnisotropicLighting::RenderScene()
{
	// Clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Keyboard input (cursor to change render mode)
	if (PVRShellIsKeyPressed(PVRShellKeyNameLEFT))
	{
		m_eRenderMode = ERenderMode((m_eRenderMode + eNumRenderModes - 1) % eNumRenderModes);
	}
	if (PVRShellIsKeyPressed(PVRShellKeyNameRIGHT))
	{
		m_eRenderMode = ERenderMode((m_eRenderMode + 1) % eNumRenderModes);
	}

	// Rotate the model matrix
	PVRTMat4 mModel = PVRTMat4::RotationY(m_fAngleY);
	m_fAngleY += 0.02f;

	// Calculate model view projection matrix
	PVRTMat4 mMVP = m_mViewProj * mModel;

	if (m_eRenderMode == eTexLookup)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_uiTexture);

		glUseProgram(m_FastShader.uiId);

		glUniformMatrix4fv(m_FastShader.uiMVPMatrixLoc, 1, GL_FALSE, mMVP.ptr());

		/*
			The inverse of a rotation matrix is the transposed matrix
			Because of v * M = transpose(M) * v, this means:
			v * R == inverse(R) * v
			So we don't have to actually invert or transpose the matrix
			to transform back from world space to model space
		*/
		PVRTVec3 vMsEyePos = PVRTVec3(PVRTVec4(0, 0, 150, 1) * mModel);
		glUniform3fv(m_FastShader.uiMsEyePosLoc, 1, vMsEyePos.ptr());

		PVRTVec3 vMsLightDir = PVRTVec3(PVRTVec4(1, 1, 1, 1) * mModel).normalized();
		glUniform3fv(m_FastShader.uiMsLightDirLoc, 1, vMsLightDir.ptr());
	}
	else
	{
		glUseProgram(m_SlowShader.uiId);

		glUniformMatrix4fv(m_SlowShader.uiMVPMatrixLoc, 1, GL_FALSE, mMVP.ptr());

		PVRTVec3 vMsEyeDir = PVRTVec3(PVRTVec4(0, 0, 150, 1) * mModel).normalized();
		glUniform3fv(m_SlowShader.uiMsEyeDirLoc, 1, vMsEyeDir.ptr());

		PVRTVec3 vMsLightDir = PVRTVec3(PVRTVec4(1, 1, 1, 1) * mModel).normalized();
		glUniform3fv(m_SlowShader.uiMsLightDirLoc, 1, vMsLightDir.ptr());
	}

	/*
		Now that the uniforms are set, call another function to actually draw the mesh.
	*/
	DrawMesh(0);

	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("AnisotropicLighting", c_aszRenderModes[m_eRenderMode], ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			i32NodeIndex		Node index of the mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the meterial prepared.
******************************************************************************/
void OGLES3AnisotropicLighting::DrawMesh(int i32NodeIndex)
{
	int i32MeshIndex = m_Scene.pNode[i32NodeIndex].nIdx;
	SPODMesh* pMesh = &m_Scene.pMesh[i32MeshIndex];

	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[i32MeshIndex]);
	// bind the index buffer, won't hurt if the handle is 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[i32MeshIndex]);

	// Enable the vertex attribute arrays
	glEnableVertexAttribArray(VERTEX_ARRAY);
	glEnableVertexAttribArray(NORMAL_ARRAY);

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
	return new OGLES3AnisotropicLighting();
}

/******************************************************************************
 End of file (OGLES3AnisotropicLighting.cpp)
******************************************************************************/

