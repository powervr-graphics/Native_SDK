/******************************************************************************

 @File         OGLES2LightMap.cpp

 @Title        LightMap

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows how to project a shadow texture on a model which has a base
               and reflection texture.

******************************************************************************/
#include "PVRShell.h"
#include "OGLES2Tools.h"

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
const char* g_aszAttribNames[] = {
	"inVertex", "inNormal", "inTexCoord" };

// shader uniforms
enum EUniform {
	eMVPMatrix, eShadowProj, eLightDirModel, eEyePosModel, eModelWorld, eNumUniforms };
const char* g_aszUniformNames[] = {
	"MVPMatrix", "ShadowProj", "LightDirModel", "EyePosModel", "ModelWorld" };

/******************************************************************************
 Content file names
******************************************************************************/

// Source and binary shaders
const char c_szFragShaderSrcFile[]	= "FragShader.fsh";
const char c_szFragShaderBinFile[]	= "FragShader.fsc";
const char c_szVertShaderSrcFile[]	= "VertShader.vsh";
const char c_szVertShaderBinFile[]	= "VertShader.vsc";

// PVR texture files
const char c_szBaseTexFile[]		= "Basetex.pvr";
const char c_szReflectTexFile[]		= "Reflection.pvr";
const char c_szShadowTexFile[]		= "Shadow.pvr";

// POD scene files
const char c_szMaskFile[]			= "Mask.pod";
const char c_szPlaneFile[]			= "Plane.pod";

/*!****************************************************************************
 Class encapsulating model data and methods
******************************************************************************/
class CModel
{
protected:
	CPVRTModelPOD m_Scene;
	GLuint* m_puiVbo;
	GLuint*	m_puiIndexVbo;

public:
	CModel();
	~CModel();

	bool ReadFromFile(const char* pszFilename);
	void LoadVbos();
	void DeleteVbos();
	void DrawMesh(int i32NodeIndex);
};

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES2LightMap : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// 3D Models
	enum EModels {
		eMask, ePlane, eNumModels
	};
	CModel m_Models[eNumModels];

	// Projection and view matrices
	PVRTMat4		m_mProjection, m_mView;

	// The shadow projection matrix
	PVRTMat4		m_mShadowViewProj;

	// OpenGL handles for shaders and textures
	GLuint	m_uiVertShader;
	GLuint	m_uiFragShader;
	GLuint	m_uiBaseTex;
	GLuint	m_uiReflectTex;
	GLuint	m_uiShadowTex;

	// Group shader programs and their uniform locations together
	struct
	{
		GLuint uiId;
		GLuint auiLoc[eNumUniforms];
	}
	m_ShaderProgram;

	// The Rotate parameter of Model
	float			m_fAngleX;
	float			m_fAngleY;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	bool LoadTextures(CPVRTString* pErrorStr);
	bool LoadShaders(CPVRTString* pErrorStr);
};

/*!****************************************************************************
 @Function		LoadTextures
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course
******************************************************************************/
bool OGLES2LightMap::LoadTextures(CPVRTString* const pErrorStr)
{
	if(PVRTTextureLoadFromPVR(c_szBaseTexFile, &m_uiBaseTex) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Failed to load texture.";
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szReflectTexFile, &m_uiReflectTex) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Failed to load texture.";
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szShadowTexFile, &m_uiShadowTex) != PVR_SUCCESS)
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
bool OGLES2LightMap::LoadShaders(CPVRTString* pErrorStr)
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

	if (PVRTCreateProgram(&m_ShaderProgram.uiId, m_uiVertShader, m_uiFragShader, g_aszAttribNames, eNumAttribs, pErrorStr))
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
 @Function		InitApplication
 @Return		bool		true if no error occured
 @Description	Code in InitApplication() will be called by PVRShell once per
				run, before the rendering context is created.
				Used to initialize variables that are not dependant on it
				(e.g. external modules, loading meshes, etc.)
				If the rendering context is lost, InitApplication() will
				not be called again.
******************************************************************************/
bool OGLES2LightMap::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	// Load the scene files
	if(!m_Models[0].ReadFromFile(c_szMaskFile) ||
		!m_Models[1].ReadFromFile(c_szPlaneFile))
	{
		PVRShellSet(prefExitMessage, "ERROR: Couldn't load the .pod file\n");
		return false;
	}

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
bool OGLES2LightMap::QuitApplication()
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
bool OGLES2LightMap::InitView()
{
	CPVRTString ErrorStr;

	/*
		Initialize VBO data
	*/
	m_Models[0].LoadVbos();
	m_Models[1].LoadVbos();

	/*
		Load textures
	*/
	if (!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Clamp the shadow texture to edge (not repeat).
	glBindTexture(GL_TEXTURE_2D, m_uiShadowTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	/*
		Load and compile the shaders & link programs
	*/
	if (!LoadShaders(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Set the sampler2D uniforms to corresponding texture units
	glUniform1i(glGetUniformLocation(m_ShaderProgram.uiId, "sBasetex"), 0);
	glUniform1i(glGetUniformLocation(m_ShaderProgram.uiId, "sReflect"), 1);
	glUniform1i(glGetUniformLocation(m_ShaderProgram.uiId, "sShadow"), 2);

	// Is the scene rotated?
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

	// Defines the shadow matrix and stores it, shadow matrix for shadow texture mapping
	PVRTVec3 vLightFrom = PVRTVec3( 85, -85, 100);
	PVRTVec3 vLightTo = PVRTVec3(0, 0, -25);
	PVRTVec3 vLightUp = PVRTVec3(0, 1, 0);

	m_mShadowViewProj = PVRTMat4::LookAtRH(vLightFrom, vLightTo, vLightUp);

	// Project the shadow from a point (near and far clipping plane are not important here)
	PVRTMat4 mShadowProj;
	mShadowProj = PVRTMat4::PerspectiveFovRH(PVRT_PI/6, 1, 1, 2, PVRTMat4::OGL);

	m_mShadowViewProj = mShadowProj * m_mShadowViewProj;
	glUniformMatrix4fv(m_ShaderProgram.auiLoc[eShadowProj], 1, GL_FALSE, m_mShadowViewProj.ptr());

	/*
		Set OpenGL ES render states needed for this training course
	*/
	// Enable backface culling and depth test
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

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
bool OGLES2LightMap::ReleaseView()
{
	// Delete textures
	glDeleteTextures(1, &m_uiBaseTex);
	glDeleteTextures(1, &m_uiReflectTex);
	glDeleteTextures(1, &m_uiShadowTex);

	// Delete program and shader objects
	glDeleteProgram(m_ShaderProgram.uiId);

	glDeleteShader(m_uiVertShader);
	glDeleteShader(m_uiFragShader);

	// Delete buffer objects
	m_Models[0].DeleteVbos();
	m_Models[1].DeleteVbos();

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
bool OGLES2LightMap::RenderScene()
{
	// Clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use shader program
	glUseProgram(m_ShaderProgram.uiId);

	// Bind textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uiBaseTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_uiReflectTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_uiShadowTex);

	// draw two models, mask and plane
	for (int i = 0; i < eNumModels; ++i)
	{
		// rotate and translate the model matrix
		PVRTMat4 mModel;

		if (i == eMask)
		{
			PVRTMat4 mRotX, mRotY;
			mRotX = PVRTMat4::RotationX(m_fAngleX);
			m_fAngleX += PVRT_PI / 300;
			mRotY = PVRTMat4::RotationY(m_fAngleY);
			m_fAngleY += PVRT_PI / 250;

			mModel = mRotY * mRotX;
		}
		else
		{
			mModel = PVRTMat4::Translation(0.0, 0.0, -25);
		}

		// Set model view projection matrix
		PVRTMat4 mModelView, mMVP;
		mModelView = m_mView * mModel;
		mMVP = m_mProjection * mModelView;
		glUniformMatrix4fv(m_ShaderProgram.auiLoc[eMVPMatrix], 1, GL_FALSE, mMVP.ptr());

		// Set shadow projection matrix
		PVRTMat4 mShadowProj;
		mShadowProj = m_mShadowViewProj * mModel;
		glUniformMatrix4fv(m_ShaderProgram.auiLoc[eShadowProj], 1, GL_FALSE, mShadowProj.ptr());

		// Set model world matrix
		PVRTMat3 fModelWorld = PVRTMat3(mModel);

		glUniformMatrix3fv(m_ShaderProgram.auiLoc[eModelWorld], 1, GL_FALSE, fModelWorld.ptr());

		// Set light position in model space
		PVRTVec4 vLightDirModel;
		vLightDirModel =  mModel.inverse() *  PVRTVec4( 1, 1, 1, 0 );

		glUniform3fv(m_ShaderProgram.auiLoc[eLightDirModel], 1, &vLightDirModel.x);

		// Set eye position in model space
		PVRTVec4 vEyePosModel;
		vEyePosModel = mModelView.inverse() * PVRTVec4(0, 0, 0, 1);

		glUniform3fv(m_ShaderProgram.auiLoc[eEyePosModel], 1, &vEyePosModel.x);

		m_Models[i].DrawMesh(0);
	}

	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("LightMap", "", ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		CModel
 @Description	Constructor
******************************************************************************/
CModel::CModel() : m_puiVbo(0), m_puiIndexVbo(0)
{}

/*!****************************************************************************
 @Function		~CModel
 @Description	Destructor
******************************************************************************/
CModel::~CModel()
{
	m_Scene.Destroy();
}

/*!****************************************************************************
 @Function		ReadFromFile
 @Input			pszFilename		filename of file to read
 @Return		bool			true if no error occured
 @Description	Loads POD file
******************************************************************************/
bool CModel::ReadFromFile(const char* const pszFilename)
{
	return m_Scene.ReadFromFile(pszFilename) == PVR_SUCCESS;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads data from model into vertex buffer objects
******************************************************************************/
void CModel::LoadVbos()
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
 @Function		DeleteVbos
 @Description	Deletes vertex buffer objects for model
******************************************************************************/
void CModel::DeleteVbos()
{
	// Delete buffer objects
	glDeleteBuffers(m_Scene.nNumMesh, m_puiVbo);
	glDeleteBuffers(m_Scene.nNumMesh, m_puiIndexVbo);
	delete [] m_puiVbo;
	delete [] m_puiIndexVbo;
	m_puiVbo = 0;
	m_puiIndexVbo = 0;
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			i32NodeIndex		Node index of the mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the meterial prepared.
******************************************************************************/
void CModel::DrawMesh(int i32NodeIndex)
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
		int offset = 0;
		for(int i = 0; i < (int)pMesh->nNumStrips; ++i)
		{
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
	return new OGLES2LightMap();
}

/******************************************************************************
 End of file (OGLES2LightMap.cpp)
******************************************************************************/

