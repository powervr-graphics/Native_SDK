/******************************************************************************

 @File         OGLES3Water.cpp

 @Title        Water

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Demonstrates a method of rendering a water effect efficiently
               using OpenGL ES 3.0

******************************************************************************/
#include "PVRShell.h"
#include "OGLES3Tools.h"

/******************************************************************************
 Defines
******************************************************************************/

// Camera constants. Used for making the projection matrix
#define CAM_NEAR	(12.0f)
#define CAM_FAR		(4000.0f)

// Index the attributes that are bound to vertex shaders
#define VERTEX_ARRAY		0
#define NORMAL_ARRAY		1
#define TEXCOORD_ARRAY		2

#define ENABLE_UI			// Remove when user input is not required
//#define DEBUG_MODE		// Only use for debugging
//#define FREE_CAMERA_MODE	// Only use for debugging

/****************************************************************************
** Enums
****************************************************************************/
enum ETextureNames
{
	eSKYBOX_TEX,
	eWATER_NORMAL_TEX,
	eTEX_NAME_SIZE
};

enum EShaderNames
{
	eREFLECTION_ONLY_SHADER,
	eSKYBOX_SHADER,
	eMODEL_SHADER,
	eTEX2D_SHADER,
	ePLANE_TEX_SHADER,
	eSHADER_SIZE
};

enum EDefineShaderNames
{
	eFULL_WATER_SHADER,
	eNO_FRESNEL_SHADER,
	eFOG_MODEL_SHADER,
	eLIGHT_MODEL_SHADER,
	eBUMP_REFLECT_WATER_SHADER,
	eSPECULAR_MODEL_SHADER,
	ePERTURB_MODEL_SHADER,
	eDEFINE_SHADER_SIZE
};

enum EVertexBufferObjects
{
	eSKYBOX_VBO,
	eVBO_SIZE
};

enum EFrameBufferObjects
{
	eREFLECTION_FBO,
	eREFRACTION_FBO,
	eWATER_FBO,
	eFBO_SIZE
};

enum EUserInterface
{
	eUI_NULL,
	eTOGGLE_REFRACTION,
	eTOGGLE_FRESNEL,
	eTOGGLE_FOG,
	eFOG_DEPTH,
	eWAVE_DISTORTION,
	eARTEFACT_FIX,
	eRENDER_WATER_SCREEN_RES,
#ifdef DEBUG_MODE
	#ifdef FREE_CAMERA_MODE
		eMOVE_X,
		eMOVE_Y,
		eMOVE_Z,
		eCAMERA_X,
		eCAMERA_Y,
		eCAMERA_Z,
		eLOOK_AT_X,
		eLOOK_AT_Y,
		eLOOK_AT_Z,
	#endif
	eWATER_HEIGHT,
	eWATER_COLOUR_R,
	eWATER_COLOUR_G,
	eWATER_COLOUR_B,
	eTOGGLE_DEBUG_WINDOWS,
#endif
	eUI_SIZE
};

enum ENodeNames
{
	eNODE_GROUND,
	eNODE_BOXES,
	eNODE_OLDBOAT,
	eNODE_COINS,
	eNODE_SHIP,
	eNODE_SAILS,
	eNODE_SHIPFLAG,
	eNODE_PALMTREETRUNK,
	eNODE_PALMLEAVES,

	eNODE_SIZE,
};

/****************************************************************************
** Structures
****************************************************************************/
// Group shader programs and their uniform locations together
struct WaterShader
{
	GLuint uiId;
	GLuint uiMVMatrixLoc;
	GLuint uiMVPMatrixLoc;
	GLuint uiEyePosLoc;
	GLuint uiWaterColourLoc;
	GLuint uiBumpTranslation0Loc;
	GLuint uiBumpScale0Loc;
	GLuint uiBumpTranslation1Loc;
	GLuint uiBumpScale1Loc;
	GLuint uiWaveDistortionLoc;
	GLuint uiRcpWindowSizeLoc;
	GLuint uiRcpMaxFogDepthLoc;
	GLuint uiFogColourLoc;
};

struct SkyboxShader
{
	GLuint uiId;
	GLuint uiMVPMatrixLoc;
	GLuint uiModelMatrixLoc;
	GLuint uiLightDirLoc;
	GLuint uiEyePosLoc;
	GLuint uiWaterHeightLoc;
	GLuint uiFogColourLoc;
	GLuint uiMaxFogDepthLoc;
};

struct ModelShader
{
	GLuint uiId;
	GLuint uiMVPMatrixLoc;
	GLuint uiModelMatrixLoc;
	GLuint uiEyePosLoc;
	GLuint uiLightDirectionLoc;
	GLuint uiWaterHeightLoc;
	GLuint uiFogColourLoc;
	GLuint uiMaxFogDepthLoc;
	GLuint uiTimeLoc;
	GLuint uiEmissiveColLoc;
	GLuint uiDiffuseColLoc;
	GLuint uiSpecularColLoc;
};
struct Tex2DShader
{
	GLuint uiId;
	GLuint uiMVPMatrixLoc;
}
m_Tex2DShader;

struct PlaneTexShader
{
	GLuint uiId;
	GLuint uiMVPMatrixLoc;
	GLuint uiRcpWindowSizeLoc;
}
m_PlaneTexShader;

struct STexture
{
	GLuint uiDiffuse;
	GLuint uiSpecular;
};

/****************************************************************************
** Consts
****************************************************************************/

// Water plane equations
static const GLuint c_uiNumberOfSkyboxTextures = 1;
static const GLuint c_uiNoOfDefines[eDEFINE_SHADER_SIZE] = {3,2,2,1,1,2,2};
static const float c_fDemoFrameRate = 1.0f / 30.0f;	// Used during animation
static const GLuint c_uiCamera = 0;					// The camera to use from the .pod file

static const char* c_aszNodeNames[eNODE_SIZE] =
{
	"Ground",
	"Boxes",
	"OldBoat",
	"Coins",
	"Ship",
	"Sails",
	"ShipFlag",
	"PalmTreeTrunk",
	"PalmTreeLeaves",
};

/******************************************************************************
 Content file names
******************************************************************************/

// Source and binary shaders
const char * const c_aszFragShaderSrcFile[eSHADER_SIZE] =
{
	"FragShader.fsh",
	"SkyboxFShader.fsh",
	"ModelFShader.fsh",
	"Tex2DFShader.fsh",
	"PlaneTexFShader.fsh"
};
const char * const c_aszFragShaderBinFile[eSHADER_SIZE] =
{
	"FragShader.fsc",
	"SkyboxFShader.fsc",
	"ModelFShader.fsc",
	"Tex2DFShader.fsc",
	"PlaneTexFShader.fsc"
};
const char * const c_aszVertShaderSrcFile[eSHADER_SIZE] =
{
	"VertShader.vsh",
	"SkyboxVShader.vsh",
	"ModelVShader.vsh",
	"Tex2DVShader.vsh",
	"PlaneTexVShader.vsh"
};
const char * const c_aszVertShaderBinFile[eSHADER_SIZE] =
{
	"VertShader.vsc",
	"SkyboxVShader.vsc",
	"ModelVShader.vsc",
	"Tex2DVShader.vsc",
	"PlaneTexVShader.vsc"
};

// PVR texture files
const char * const c_aszTextureNames[eTEX_NAME_SIZE] =
{
	"skybox.pvr",
	"normalmap.pvr"
};

// Shader defines are used to control which code path is taken in each shader
static const char* c_aszFullWaterShaderDefines[] =
{
	"ENABLE_REFRACTION",
	"ENABLE_FRESNEL",
	"ENABLE_DISTORTION"
};
static const char* c_aszFogShaderDefines[] =
{
	"ENABLE_FOG_DEPTH",
	"ENABLE_LIGHTING"
};
static const char* c_aszNoFresnelShaderDefines[] =
{
	"ENABLE_REFRACTION",
	"ENABLE_DISTORTION"

};
const char* c_aszModelLightingDefines[] =
{
	"ENABLE_LIGHTING"
};
const char* c_aszModelSpecularDefines[] =
{
	"ENABLE_LIGHTING",
	"ENABLE_SPECULAR",
};
const char* c_aszModelPerturbDefines[] =
{
	"ENABLE_LIGHTING",
	"ENABLE_PERTURB_VTX",
};
const char* c_aszBumpedReflectionShaderDefines[] =
{
	"ENABLE_DISTORTION"
};

// Array of pointers to the defines for shaders
const char** c_aszAllDefines[eDEFINE_SHADER_SIZE] =
{
	c_aszFullWaterShaderDefines,          // eFULL_WATER_SHADER
	c_aszNoFresnelShaderDefines,          // eNO_FRESNEL_SHADER
	c_aszFogShaderDefines,                // eFOG_MODEL_SHADER
	c_aszModelLightingDefines,            // eLIGHT_MODEL_SHADER
	c_aszBumpedReflectionShaderDefines,   // eBUMP_REFLECT_WATER_SHADER
	c_aszModelSpecularDefines,            // eSPECULAR_MODEL_SHADER
	c_aszModelPerturbDefines,             // ePERTURB_MODEL_SHADER
};

// POD scene files
static const char* c_aszModelFile = "Scene.pod";

/*!****************************************************************************
 Class declaration
******************************************************************************/
class OGLES3Water : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// 3D Model
	CPVRTModelPOD	m_Mesh;

	// Projection, view and other matrices
	PVRTMat4		m_mProjection, m_mView;

	// Camera settings
	PVRTVec3		m_vEyePos, m_vLookAt, m_vCamUp;
	GLfloat			m_fFOV;

	// Shared variables
	PVRTVec4 m_vLightDirection;

	// UI variables
	int m_iCurrentUIOption;

	// OpenGL handles for shaders, textures and VBOs
	GLuint* m_apuiModelVbo;
	GLuint* m_apuiModelIndexVbo;
	STexture* m_apuiModelTextureIds;
	GLuint m_auiTextureIds[eTEX_NAME_SIZE];
	GLuint m_uiNormalisationCubeMap;
	GLuint m_auiVertShaderIds[eSHADER_SIZE + eDEFINE_SHADER_SIZE];
	GLuint m_auiFragShaderIds[eSHADER_SIZE + eDEFINE_SHADER_SIZE];
	GLuint m_auiVBOIds[eVBO_SIZE];								// Vertex buffer objects
	GLint m_iOriginalFBO;										// Original frame buffer object
	GLuint m_auiFBOIds[eFBO_SIZE];								// Frame buffer objects

	// Render to texture variables
	GLuint m_uiTexSize;				// dimension for reflection and refraction mapping
	GLuint m_uiWaterTexSize;		// dimension for water mapping

	GLuint m_auiRendToTexture[eFBO_SIZE];
	GLuint m_auiDepthBuffer[eFBO_SIZE];						// Used for reflection render

	// Shader programs
	ModelShader m_ModelShader, m_FogModelShader, m_LightModelShader, m_SpecularModelShader, m_PerturbedModelShader;
	WaterShader m_ReflectionOnlyShader, m_FullWaterShader, m_NoFresnelWaterShader, m_BumpReflectionWaterShader;
	SkyboxShader m_SkyboxShader;

	// Water
	PVRTVec4 m_vPlaneWater;								// [A,B,C,D] plane definition
	PVRTVec3 m_pvPlaneWater[5];							// Procedurally generated water plane
	int	m_i32WaterPlaneNo;

	// Skybox
	GLfloat* m_SkyboxVertices;
	GLfloat* m_SkyboxTexCoords;

	// Time variables
	unsigned long m_ulPreviousTime, m_ulCurrentTime;
	float m_fElapsedTimeInSecs, m_fDeltaTime, m_fFrame, m_fCount;
	unsigned int m_uiFPS, m_uiFrameCount;
	bool m_bPause;

	// Water variables
	PVRTVec4 m_vWaterColour;
	GLfloat	m_fWaterHeight;
	GLfloat	m_fMaxFogDepth;
	bool	m_bFogDepth;
	GLfloat	m_fWaterArtefactFix;
	GLfloat m_fBoatSpeed;

	PVRTVec2 m_vBumpTranslation0;
	PVRTVec2 m_vBumpVelocity0;
	PVRTVec2 m_vBumpScale0;
	PVRTVec2 m_vBumpTranslation1;
	PVRTVec2 m_vBumpVelocity1;
	PVRTVec2 m_vBumpScale1;
	GLfloat	 m_fWaveDistortion;
	PVRTVec2 m_vRcpWindowSize;
	GLfloat m_fWindSpeed;

	// Fog/mist variables
	PVRTVec4 m_vFogColour;
	GLfloat m_fMaxFogHeight;

	CPVRTMap<GLuint, ENodeNames>	m_NodeIndexName;
	CPVRTMap<ENodeNames, GLuint>	m_NodeNameIndex;


	bool		m_bShaderRefraction;
	bool		m_bShaderFogging;
	bool		m_bShaderFresnel;
	bool		m_bDisplayDebugWindows;
	PVRTVec4	m_vClipPlane;
	bool		m_bClipPlane;
	bool		m_bWaterAtScreenRes;


public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	bool LoadTextures(CPVRTString* pErrorStr);
	bool LoadWaterShader(WaterShader& shaderProgram, GLuint uiShaderId, CPVRTString* pErrorStr);
	bool LoadModelShader(ModelShader& shaderProgram,GLuint uiShaderId, CPVRTString* pErrorStr);
	bool LoadShaders(CPVRTString* pErrorStr);
	bool LoadVbos(CPVRTString* pErrorStr);

	void SetProjection(const float fFOV = 60.0f * (PVRT_PI/180.0f), const float fFarClip = CAM_FAR);
	void SetView();

	void ResetVariables();

	void RenderReflectionTexture();
	void RenderRefractionTexture();
	void RenderWaterTexture(const WaterShader& shaderProgram);

	void DrawMesh(int i32NodeIndex, const ModelShader& shaderProgram);

	void DrawInfinitePlane(const PVRTVec4& vPlane, float fFarDistance = CAM_FAR);
	void DrawWater(const WaterShader& shaderProgram, GLuint uiViewPortWidth, GLuint uiViewPortHeight, const PVRTVec4& vPlane, float fFarDistance = CAM_FAR);
	void DrawWaterFromTexture(float farDistance = CAM_FAR);
	void DrawSkybox(GLuint uiCubeMapHandle, const SkyboxShader& shaderProgram, GLuint uiVboId,
					const PVRTVec3& vTranslation = PVRTVec3(0.0f,0.0f,0.0f));

	void DrawScene();
	void DrawRefractionScene();

	void DrawTestQuad(GLuint uiTextureHandle, const PVRTVec2 &vBottomLeftPosition = PVRTVec2(-1,-1));

	void ModifyProjectionForClipping(const PVRTVec4 &vClipPlane);
	bool GenerateNormalisationCubeMap(int uiTextureSize = 32);

	void UpdateTimer();

	inline GLfloat sgn(GLfloat a);
};

/*!****************************************************************************
 @Function		sgn
 @Return		a			Returns the result of the signum function.
 @Description	Takes a float input and determines if it's value is greater than,
				equal to or less than zero. It returns a value within normal
				space to reflect this outcome
******************************************************************************/
GLfloat OGLES3Water::sgn(GLfloat a)
{
	if(a > 0.0f) return(1.0f);
	if(a < 0.0f) return(-1.0f);
	return 0.0f;
}

/*!****************************************************************************
 @Function		LoadTextures
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course
******************************************************************************/
bool OGLES3Water::LoadTextures(CPVRTString* const pErrorStr)
{
	// Load textures to array
	GLuint i = 0;

	// Load cubemaps first
	for(; i < c_uiNumberOfSkyboxTextures; ++i)
	{
		if(PVRTTextureLoadFromPVR(c_aszTextureNames[i], &m_auiTextureIds[i]) != PVR_SUCCESS)
		{
			*pErrorStr = CPVRTString("ERROR: Could not open texture file ") + c_aszTextureNames[i];
			return false;
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	// Load remaining textures
	for(; i < eTEX_NAME_SIZE; ++i)
	{
		if(PVRTTextureLoadFromPVR(c_aszTextureNames[i],&m_auiTextureIds[i]) != PVR_SUCCESS)
		{
			*pErrorStr = CPVRTString("ERROR: Could not open texture file ") + c_aszTextureNames[i];
			return false;
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	//Create normalisation cube map
	glGenTextures(1, &m_uiNormalisationCubeMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_uiNormalisationCubeMap);
	GenerateNormalisationCubeMap(8);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);



	glGenTextures(eFBO_SIZE, m_auiRendToTexture);
	// Allocate textures for reflection and refraction FBOs
	for(i = 0; i < eFBO_SIZE - 1; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, m_auiRendToTexture[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB/*A*/, m_uiTexSize, m_uiTexSize, 0, GL_RGB/*A*/, GL_UNSIGNED_BYTE, 0);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	// Allocate texture for water FBO
	glBindTexture(GL_TEXTURE_2D, m_auiRendToTexture[eWATER_FBO]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_uiWaterTexSize, m_uiWaterTexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	// Load all textures for each model
	m_apuiModelTextureIds = new STexture[m_Mesh.nNumMaterial];

	if(!m_apuiModelTextureIds)
	{
		*pErrorStr = "ERROR: Insufficient memory.";
		return false;
	}

	for(int i = 0; i < (int) m_Mesh.nNumMaterial; ++i)
	{
		m_apuiModelTextureIds[i].uiDiffuse  = 0;
		m_apuiModelTextureIds[i].uiSpecular = 0;
		SPODMaterial* pMaterial = &m_Mesh.pMaterial[i];

		if(pMaterial->nIdxTexDiffuse != -1)
		{
			/*
				Using the tools function PVRTTextureLoadFromPVR load the textures required by the pod file.

				Note: This function only loads .pvr files. You can set the textures in 3D Studio Max to .pvr
				files using the PVRTexTool plug-in for max. Alternatively, the pod material properties can be
				modified in PVRShaman.
			*/

			CPVRTString sTextureName = m_Mesh.pTexture[pMaterial->nIdxTexDiffuse].pszName;

			if(PVRTTextureLoadFromPVR(sTextureName.c_str(), &m_apuiModelTextureIds[i].uiDiffuse) != PVR_SUCCESS)
			{
				*pErrorStr = "ERROR: Failed to load " + sTextureName + ". ";

				// Check to see if we're trying to load .pvr or not
				CPVRTString sFileExtension = PVRTStringGetFileExtension(sTextureName);

				if(sFileExtension.toLower() != "pvr")
				{
					*pErrorStr += "Note: Demo can only load pvr files.";
				}

				return false;
			}

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			if(sTextureName == "sand.pvr" || sTextureName == "coins.pvr")
			{
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}
			else
			{
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
		}
		if(pMaterial->nIdxTexSpecularLevel != -1)
		{
			/*
			 Using the tools function PVRTTextureLoadFromPVR load the textures required by the pod file.

			 Note: This function only loads .pvr files. You can set the textures in 3D Studio Max to .pvr
			 files using the PVRTexTool plug-in for max. Alternatively, the pod material properties can be
			 modified in PVRShaman.
			 */

			CPVRTString sTextureName = m_Mesh.pTexture[pMaterial->nIdxTexSpecularLevel].pszName;

			if(PVRTTextureLoadFromPVR(sTextureName.c_str(), &m_apuiModelTextureIds[i].uiSpecular) != PVR_SUCCESS)
			{
				*pErrorStr = "ERROR: Failed to load " + sTextureName + ".";

				// Check to see if we're trying to load .pvr or not
				CPVRTString sFileExtension = PVRTStringGetFileExtension(sTextureName);

				if(sFileExtension.toLower() != "pvr")
				{
					*pErrorStr += "Note: Demo can only load pvr files.";
				}

				return false;
			}

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			if(sTextureName == "coins-specular.pvr")
			{
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}
			else
			{
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
		}
	}

	return true;
}

/*!****************************************************************************
 @Function		LoadWaterShader
 @Input/output	shaderProgram	The water shader to load
 @Input			uiShaderId		The shader's ID
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads and compiles a water shader and links it to a shader program
******************************************************************************/
bool OGLES3Water::LoadWaterShader(WaterShader& shaderProgram, GLuint uiShaderId, CPVRTString* pErrorStr)
{
	const char* aszWaterAttribs[] = { "inVertex"};
	if (PVRTCreateProgram(&shaderProgram.uiId, m_auiVertShaderIds[uiShaderId], m_auiFragShaderIds[uiShaderId], aszWaterAttribs, 1, pErrorStr))
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	// Set the sampler2D variables
	glUniform1i(glGetUniformLocation(shaderProgram.uiId, "NormalTex"), 0);
	glUniform1i(glGetUniformLocation(shaderProgram.uiId, "ReflectionTex"),1);
	glUniform1i(glGetUniformLocation(shaderProgram.uiId, "RefractionTex"),2);
	glUniform1i(glGetUniformLocation(shaderProgram.uiId, "NormalisationCubeMap"),3);

	// Store the location of uniforms for later use
	shaderProgram.uiMVMatrixLoc					= glGetUniformLocation(shaderProgram.uiId, "ModelViewMatrix");
	shaderProgram.uiMVPMatrixLoc				= glGetUniformLocation(shaderProgram.uiId, "MVPMatrix");
	shaderProgram.uiEyePosLoc					= glGetUniformLocation(shaderProgram.uiId, "EyePosition");
	shaderProgram.uiWaterColourLoc				= glGetUniformLocation(shaderProgram.uiId, "WaterColour");
	shaderProgram.uiBumpTranslation0Loc			= glGetUniformLocation(shaderProgram.uiId, "BumpTranslation0");
	shaderProgram.uiBumpScale0Loc				= glGetUniformLocation(shaderProgram.uiId, "BumpScale0");
	shaderProgram.uiBumpTranslation1Loc			= glGetUniformLocation(shaderProgram.uiId, "BumpTranslation1");
	shaderProgram.uiBumpScale1Loc				= glGetUniformLocation(shaderProgram.uiId, "BumpScale1");
	shaderProgram.uiWaveDistortionLoc			= glGetUniformLocation(shaderProgram.uiId, "WaveDistortion");
	shaderProgram.uiRcpWindowSizeLoc			= glGetUniformLocation(shaderProgram.uiId, "RcpWindowSize");
	shaderProgram.uiRcpMaxFogDepthLoc           = glGetUniformLocation(shaderProgram.uiId, "RcpMaxFogDepth");
	shaderProgram.uiFogColourLoc                = glGetUniformLocation(shaderProgram.uiId, "FogColour");

	return true;
}

/*!****************************************************************************
 @Function		LoadModelShader
 @Input/output	shaderProgram	The model shader to load
 @Input			uiShaderId		The shader's ID
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads and compiles a model shader and links it to a shader program
******************************************************************************/
bool OGLES3Water::LoadModelShader(ModelShader &shaderProgram, GLuint uiShaderId, CPVRTString *pErrorStr)
{
	const char* aszModelAttribs[] = { "inVertex", "inNormal", "inTexCoord"};
	if (PVRTCreateProgram(&shaderProgram.uiId, m_auiVertShaderIds[uiShaderId], m_auiFragShaderIds[uiShaderId], aszModelAttribs, 3, pErrorStr))
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	glUniform1i(glGetUniformLocation(shaderProgram.uiId, "ModelTexture"),0);
	glUniform1i(glGetUniformLocation(shaderProgram.uiId, "ModelTextureSpec"),1);

	shaderProgram.uiMVPMatrixLoc				= glGetUniformLocation(shaderProgram.uiId, "MVPMatrix");
	shaderProgram.uiModelMatrixLoc				= glGetUniformLocation(shaderProgram.uiId, "ModelMatrix");
	shaderProgram.uiEyePosLoc              		= glGetUniformLocation(shaderProgram.uiId, "EyePos");
	shaderProgram.uiLightDirectionLoc			= glGetUniformLocation(shaderProgram.uiId, "LightDirection");
	shaderProgram.uiWaterHeightLoc				= glGetUniformLocation(shaderProgram.uiId, "WaterHeight");
	shaderProgram.uiFogColourLoc				= glGetUniformLocation(shaderProgram.uiId, "FogColour");
	shaderProgram.uiMaxFogDepthLoc				= glGetUniformLocation(shaderProgram.uiId, "RcpMaxFogDepth");
	shaderProgram.uiTimeLoc		    	    	= glGetUniformLocation(shaderProgram.uiId, "fTime");
	shaderProgram.uiEmissiveColLoc              = glGetUniformLocation(shaderProgram.uiId, "EmissiveColour");
	shaderProgram.uiDiffuseColLoc               = glGetUniformLocation(shaderProgram.uiId, "DiffuseColour");
	shaderProgram.uiSpecularColLoc              = glGetUniformLocation(shaderProgram.uiId, "SpecularColour");

	return true;
}

/*!****************************************************************************
 @Function		LoadShaders
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads and compiles shaders and links them to shader programs
******************************************************************************/
bool OGLES3Water::LoadShaders(CPVRTString* pErrorStr)
{
	/*
		Load and compile the shaders from files.
		Binary shaders are tried first, source shaders
		are used as fallback.
	*/

	for(GLuint i = 0; i < eSHADER_SIZE; ++i)
	{
		if(PVRTShaderLoadFromFile(
			c_aszVertShaderBinFile[i],c_aszVertShaderSrcFile[i],GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_auiVertShaderIds[i], pErrorStr) != PVR_SUCCESS)
		{
			return false;
		}

		if(PVRTShaderLoadFromFile(
			c_aszFragShaderBinFile[i],c_aszFragShaderSrcFile[i],GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_auiFragShaderIds[i], pErrorStr) != PVR_SUCCESS)
		{
			return false;
		}
	}
	// Assign pointers to the original source files the defines need to be prepended to
	const char* pDefVertShaderSrcFile[eDEFINE_SHADER_SIZE] = {	c_aszVertShaderSrcFile[eREFLECTION_ONLY_SHADER],
																c_aszVertShaderSrcFile[eREFLECTION_ONLY_SHADER],
																c_aszVertShaderSrcFile[eMODEL_SHADER],
																c_aszVertShaderSrcFile[eMODEL_SHADER],
																c_aszVertShaderSrcFile[eREFLECTION_ONLY_SHADER],
	                                                            c_aszVertShaderSrcFile[eMODEL_SHADER],
	                                                            c_aszVertShaderSrcFile[eMODEL_SHADER]};
	const char* pDefFragShaderSrcFile[eDEFINE_SHADER_SIZE] = {	c_aszFragShaderSrcFile[eREFLECTION_ONLY_SHADER],
																c_aszFragShaderSrcFile[eREFLECTION_ONLY_SHADER],
																c_aszFragShaderSrcFile[eMODEL_SHADER],
																c_aszFragShaderSrcFile[eMODEL_SHADER],
																c_aszFragShaderSrcFile[eREFLECTION_ONLY_SHADER],
	                                                            c_aszFragShaderSrcFile[eMODEL_SHADER],
																c_aszFragShaderSrcFile[eMODEL_SHADER]};

	// Load shaders using defines
	for(GLuint i = 0; i < eDEFINE_SHADER_SIZE; ++i)
	{
		if(PVRTShaderLoadFromFile(0,pDefVertShaderSrcFile[i],GL_VERTEX_SHADER, 0, &m_auiVertShaderIds[eSHADER_SIZE + i], pErrorStr, 0, c_aszAllDefines[i], c_uiNoOfDefines[i]) != PVR_SUCCESS)
		{
			return false;
		}

		if(PVRTShaderLoadFromFile(0, pDefFragShaderSrcFile[i],GL_FRAGMENT_SHADER, 0, &m_auiFragShaderIds[eSHADER_SIZE + i], pErrorStr, 0, c_aszAllDefines[i], c_uiNoOfDefines[i]) != PVR_SUCCESS)
		{
			return false;
		}
	}


	/*
		Set up and link to water shader programs
	*/

	if(!LoadWaterShader(m_ReflectionOnlyShader, eREFLECTION_ONLY_SHADER, pErrorStr))
	{
		return false;
	}
	if(!LoadWaterShader(m_FullWaterShader, eSHADER_SIZE + eFULL_WATER_SHADER, pErrorStr))
	{
		return false;
	}
	if(!LoadWaterShader(m_NoFresnelWaterShader, eSHADER_SIZE + eNO_FRESNEL_SHADER, pErrorStr))
	{
		return false;
	}
	if(!LoadWaterShader(m_BumpReflectionWaterShader, eSHADER_SIZE + eBUMP_REFLECT_WATER_SHADER, pErrorStr))
	{
		return false;
	}

	/*
		Set up and link the sky box shader program
	*/
	const char* aszSkyboxAttribs[] = { "inVertex"};
	if (PVRTCreateProgram(&m_SkyboxShader.uiId, m_auiVertShaderIds[eSKYBOX_SHADER], m_auiFragShaderIds[eSKYBOX_SHADER], aszSkyboxAttribs, 1, pErrorStr))
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	glUniform1i(glGetUniformLocation(m_SkyboxShader.uiId, "CubeMap"),0);

	m_SkyboxShader.uiMVPMatrixLoc				= glGetUniformLocation(m_SkyboxShader.uiId, "MVPMatrix");
	m_SkyboxShader.uiModelMatrixLoc				= glGetUniformLocation(m_SkyboxShader.uiId, "ModelMatrix");
	m_SkyboxShader.uiEyePosLoc					= glGetUniformLocation(m_SkyboxShader.uiId, "EyePosition");
	m_SkyboxShader.uiWaterHeightLoc				= glGetUniformLocation(m_SkyboxShader.uiId, "WaterHeight");
	m_SkyboxShader.uiFogColourLoc				= glGetUniformLocation(m_SkyboxShader.uiId, "FogColour");
	m_SkyboxShader.uiMaxFogDepthLoc				= glGetUniformLocation(m_SkyboxShader.uiId, "RcpMaxFogDepth");

	/*
		Set up and link to the model shader programs
	*/
	if(!LoadModelShader(m_ModelShader, eMODEL_SHADER, pErrorStr))
	{
		return false;
	}
	if(!LoadModelShader(m_FogModelShader, eSHADER_SIZE + eFOG_MODEL_SHADER, pErrorStr))
	{
		return false;
	}
	if(!LoadModelShader(m_LightModelShader, eSHADER_SIZE + eLIGHT_MODEL_SHADER, pErrorStr))
	{
		return false;
	}
	if(!LoadModelShader(m_SpecularModelShader, eSHADER_SIZE + eSPECULAR_MODEL_SHADER, pErrorStr))
	{
		return false;
	}
	if(!LoadModelShader(m_PerturbedModelShader, eSHADER_SIZE + ePERTURB_MODEL_SHADER, pErrorStr))
	{
		return false;
	}

	/*
		Set up and link to the Tex2D shader program
	*/
	const char* aszTex2DAttribs[] = { "inVertex", "inNormal", "inTexCoord"};
	if (PVRTCreateProgram(&m_Tex2DShader.uiId, m_auiVertShaderIds[eTEX2D_SHADER], m_auiFragShaderIds[eTEX2D_SHADER], aszTex2DAttribs, 3, pErrorStr))
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	glUniform1i(glGetUniformLocation(m_Tex2DShader.uiId, "Texture"),0);

	m_Tex2DShader.uiMVPMatrixLoc					= glGetUniformLocation(m_Tex2DShader.uiId, "MVPMatrix");

	/*
		Set up and link to plane texturing shader program
	*/
	const char* aszPlaneTexAttribs[] = { "inVertex"};
	if (PVRTCreateProgram(&m_PlaneTexShader.uiId, m_auiVertShaderIds[ePLANE_TEX_SHADER], m_auiFragShaderIds[ePLANE_TEX_SHADER], aszPlaneTexAttribs, 1, pErrorStr))
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	glUniform1i(glGetUniformLocation(m_PlaneTexShader.uiId, "Texture"),0);

	m_PlaneTexShader.uiMVPMatrixLoc					= glGetUniformLocation(m_PlaneTexShader.uiId, "MVPMatrix");
	m_PlaneTexShader.uiRcpWindowSizeLoc				= glGetUniformLocation(m_PlaneTexShader.uiId, "RcpWindowSize");

	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Output		pErrorStr		A string describing the error on failure
 @Description	Loads data into	vertex buffer objects
******************************************************************************/
bool OGLES3Water::LoadVbos(CPVRTString* pErrorStr)
{
	// Load models into VBOs
	if(!m_Mesh.pMesh[0].pInterleaved)
	{
		*pErrorStr = "ERROR: The demo requires the pod data to be interleaved. Please re-export with the interleaved option enabled.";
		return false;
	}

	if(!m_apuiModelVbo)		m_apuiModelVbo = new GLuint[m_Mesh.nNumMesh];
	if(!m_apuiModelIndexVbo) m_apuiModelIndexVbo = new GLuint[m_Mesh.nNumMesh];

	/*
		Load vertex data of all meshes in the scene into VBOs

		The meshes have been exported with the "Interleave Vectors" option,
		so all data is interleaved in the buffer at pMesh->pInterleaved.
		Interleaving data improves the memory access pattern and cache efficiency,
		thus it can be read faster by the hardware.
	*/
	glGenBuffers(m_Mesh.nNumMesh, m_apuiModelVbo);
	for (unsigned int i = 0; i < m_Mesh.nNumMesh; ++i)
	{
		// Load vertex data into buffer object
		SPODMesh& Mesh = m_Mesh.pMesh[i];
		unsigned int uiSize = Mesh.nNumVertex * Mesh.sVertex.nStride;
		glBindBuffer(GL_ARRAY_BUFFER, m_apuiModelVbo[i]);
		glBufferData(GL_ARRAY_BUFFER, uiSize, Mesh.pInterleaved, GL_STATIC_DRAW);

		// Load index data into buffer object if available
		m_apuiModelIndexVbo[i] = 0;
		if (Mesh.sFaces.pData)
		{
			glGenBuffers(1, &m_apuiModelIndexVbo[i]);
			uiSize = PVRTModelPODCountIndices(Mesh) * sizeof(GLshort);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_apuiModelIndexVbo[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, Mesh.sFaces.pData, GL_STATIC_DRAW);
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Skybox
	glGenBuffers(1,&m_auiVBOIds[eSKYBOX_VBO]);
	glBindBuffer(GL_ARRAY_BUFFER,m_auiVBOIds[eSKYBOX_VBO]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*3*24, m_SkyboxVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER,0);

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
bool OGLES3Water::InitApplication()
{
#ifdef DEBUG_MODE
	PVRShellSet(prefSwapInterval,0);	// Disable v-sync for testing
	m_bPause = true;					// Pause initially as a benchmark reference point
#else
	m_bPause = false;					// NOTE: Should be set to false!
#endif
	// Set null pointers
	m_apuiModelVbo        = 0;
	m_apuiModelIndexVbo   = 0;
	m_apuiModelTextureIds = 0;

	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	// Set timer variables
	m_ulCurrentTime = PVRShellGetTime();
	m_ulPreviousTime = m_ulCurrentTime;
	m_fCount = 0;
	m_uiFrameCount = 0;
	m_uiFPS = 0;

	// Load model
	if(m_Mesh.ReadFromFile(c_aszModelFile) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Couldn't load the .pod file\n");
		return false;
	}

	// Retrieve node indexes
	for(int i = 0; i < eNODE_SIZE; ++i)
	{
		for(int j = 0; j < m_Mesh.nNumNode; ++j)
		{
			SPODNode& node = m_Mesh.pNode[j];
			if(strcmp(node.pszName, c_aszNodeNames[i]) == 0)
			{
				m_NodeIndexName[j] = (ENodeNames)i;
				m_NodeNameIndex[(ENodeNames)i] = j;
			}
		}
	}

	// Set UI variables
	m_iCurrentUIOption = 0;
	ResetVariables();

	// Set animation variables
	m_fFOV = 60.0f * (PVRT_PI/180.0f);
	m_fFrame = 0;

	return true;
}

/*!****************************************************************************
 @Function		ResetVariables
 @Description	Resets all variables to their original value. This allows
				the user to reset the scene during run-time
******************************************************************************/
void OGLES3Water::ResetVariables()
{
#ifdef FREE_CAMERA_MODE
	// Set camera variables
	m_vEyePos = PVRTVec3(0.0001f, 100.0001f, 400.0001f);	// Slight offset is used to  prevent divide by
	m_vLookAt = PVRTVec3(0.0001f, 10.0001f, 0.0001f);	// zero when altering the camera position and orientation
#endif
	m_vCamUp = PVRTVec3(0.00f, 1.0001f, 0.00f);

	// Set light direction
	m_vLightDirection = m_Mesh.GetLightDirection(0);

	// Set variables
	m_vPlaneWater        = PVRTVec4(0.0f, 1.0f, 0.0f, 0.0f);
	m_vWaterColour       = PVRTVec4(0.05f,0.15f,0.10f,1.0f);
	m_vFogColour         = PVRTVec4(0.85f, 0.95f, 1.0f, 1.0f);
	m_fWaterHeight       = 0.0f;
	m_fMaxFogDepth       = 80.0f;
	m_fMaxFogHeight      = 2000.0f;
	m_fWaveDistortion    = 100.0f;
	m_fWindSpeed         = 10.0f;
	m_bFogDepth          = false;
	m_fWaterArtefactFix  = 3.0f;
	m_fBoatSpeed         = 0.05f;

	// Normal map values
	m_vBumpVelocity0    = PVRTVec2(0.016f,-0.014f);
	m_vBumpTranslation0 = PVRTVec3(0.0f,0.0f,0.0f);	// No translation should be applied
	m_vBumpScale0       = PVRTVec2(0.0012f,0.0012f);
	m_vBumpVelocity1    = PVRTVec2(0.025f,-0.03f);
	m_vBumpTranslation1 = PVRTVec3(0.0f,0.0f,0.0f);	// No translation should be applied
	m_vBumpScale1       = PVRTVec2(0.0005f,0.0005f);

	m_bShaderRefraction    = true;
	m_bShaderFogging       = true;
	m_bShaderFresnel       = true;
	m_bDisplayDebugWindows = false;
	m_bClipPlane           = false;
	m_bWaterAtScreenRes    = true;
}

/*!****************************************************************************
 @Function		QuitApplication
 @Return		bool		true if no error occured
 @Description	Code in QuitApplication() will be called by PVRShell once per
				run, just before exiting the program.
				If the rendering context is lost, QuitApplication() will
				not be called.
******************************************************************************/
bool OGLES3Water::QuitApplication()
{
	// Free the memory allocated for the scene
	delete[] m_apuiModelVbo;
	delete[] m_apuiModelIndexVbo;
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
bool OGLES3Water::InitView()
{
	CPVRTString ErrorStr;

	// Calculate our FBO sizes based on the window dimensions
	m_uiWaterTexSize = m_uiTexSize = PVRTGetPOTLower(PVRT_MIN(PVRShellGet(prefWidth),PVRShellGet(prefHeight)), 1);

	// Create the skybox
	PVRTCreateSkybox( 1500.0f, true, 512, &m_SkyboxVertices, &m_SkyboxTexCoords);

	/*
		Load textures
	*/
	if (!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	/*
		Load in the vertex buffered objects
	*/
	if(!LoadVbos(&ErrorStr))
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
	SetProjection();
	SetView();

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_iOriginalFBO);

	// Enable culling and depth test
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	// Use the water colour for clearing
	glClearColor(m_vWaterColour.x, m_vWaterColour.y, m_vWaterColour.z, 1.0f);

	glGenFramebuffers(eFBO_SIZE, m_auiFBOIds);
	glGenRenderbuffers(eFBO_SIZE, m_auiDepthBuffer);

	// Reflection and refraction FBO
	for(GLuint i = 0; i < eFBO_SIZE - 1; ++i)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_auiFBOIds[i]);

		// Attach the texture that the frame buffer will render to
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_auiRendToTexture[i],0);
		glClear(GL_COLOR_BUFFER_BIT);

		// Create anda attach a depth buffer
		glBindRenderbuffer(GL_RENDERBUFFER, m_auiDepthBuffer[i]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_uiTexSize, m_uiTexSize);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_auiDepthBuffer[i]);

		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			PVRShellSet(prefExitMessage,"ERROR: Frame buffer did not set up correctly\n");
			return false;
		}
	}

	// The water texture size may be different from the reflection & refraction textures, so it is set up seperately
	glBindFramebuffer(GL_FRAMEBUFFER, m_auiFBOIds[eWATER_FBO]);

	// Attach the texture that the frame buffer will render to
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_auiRendToTexture[eWATER_FBO],0);
	glClear(GL_COLOR_BUFFER_BIT);

	// Create anda attach a depth buffer
	glBindRenderbuffer(GL_RENDERBUFFER, m_auiDepthBuffer[eWATER_FBO]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_uiWaterTexSize, m_uiWaterTexSize);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_auiDepthBuffer[eWATER_FBO]);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		PVRShellSet(prefExitMessage,"ERROR: Frame buffer did not set up correctly\n");
		return false;
	}

	// Bind the original frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_iOriginalFBO);

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES3Water::ReleaseView()
{
	// Delete textures
	glDeleteTextures(eTEX_NAME_SIZE,m_auiTextureIds);
	glDeleteTextures(eFBO_SIZE, m_auiRendToTexture);

	for(GLuint i = 0 ; i < m_Mesh.nNumMaterial ; ++i)
	{
		if(m_apuiModelTextureIds[i].uiDiffuse)
			glDeleteTextures(1, &m_apuiModelTextureIds[i].uiDiffuse);

		if(m_apuiModelTextureIds[i].uiSpecular)
			glDeleteTextures(1, &m_apuiModelTextureIds[i].uiSpecular);
	}

	delete[] m_apuiModelTextureIds;
	m_apuiModelTextureIds = 0;

	// Delete program and shader objects
	glDeleteProgram(m_ReflectionOnlyShader.uiId);
	glDeleteProgram(m_SkyboxShader.uiId);
	glDeleteProgram(m_Tex2DShader.uiId);
	glDeleteProgram(m_FullWaterShader.uiId);
	glDeleteProgram(m_BumpReflectionWaterShader.uiId);
	glDeleteProgram(m_NoFresnelWaterShader.uiId);
	glDeleteProgram(m_ModelShader.uiId);
	glDeleteProgram(m_FogModelShader.uiId);
	glDeleteProgram(m_PlaneTexShader.uiId);
	glDeleteProgram(m_LightModelShader.uiId);
	glDeleteProgram(m_SpecularModelShader.uiId);
	glDeleteProgram(m_PerturbedModelShader.uiId);
	for(GLuint i = 0 ; i < eSHADER_SIZE + eDEFINE_SHADER_SIZE; ++i)
	{
		glDeleteShader(m_auiVertShaderIds[i]);
		glDeleteShader(m_auiFragShaderIds[i]);
	}

	// Delete buffer objects
	glDeleteBuffers(eVBO_SIZE, m_auiVBOIds);
	glDeleteFramebuffers(eFBO_SIZE, m_auiFBOIds);

	glDeleteBuffers(m_Mesh.nNumMesh, m_apuiModelVbo);
	glDeleteBuffers(m_Mesh.nNumMesh, m_apuiModelIndexVbo);

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

	// Destroy the Skybox
	PVRTDestroySkybox( m_SkyboxVertices, m_SkyboxTexCoords);

	return true;
}

/*!****************************************************************************
 @Function		RenderScene
 @Return		bool		true if no error occured
 @Description	Main rendering loop function of the program. The shell will
				call this function every frame.
				eglSwapBuffers() will be performed by PVRShell automatically.
				PVRShell will also manage important OS events.
				The user has access to these events through an abstraction
				layer provided by PVRShell.
******************************************************************************/
bool OGLES3Water::RenderScene()
{
	UpdateTimer();			// Update timer variables

	// Set the scene animation
	if(!m_bPause)
	{
		m_fFrame += ((GLfloat)((m_ulCurrentTime - m_ulPreviousTime) * c_fDemoFrameRate)) * m_fBoatSpeed;	// value is scaled by animation speed
		if(m_fFrame > m_Mesh.nNumFrame - 1)
		{
			m_fFrame = 0;
		}
	}
	m_Mesh.SetFrame(m_fFrame);

	// Perform reflection render pass
	glCullFace(GL_FRONT);

	RenderReflectionTexture();

	glCullFace(GL_BACK);

	if(m_bShaderRefraction)
	{
		RenderRefractionTexture();	// Only perform the refraction render pass if it is needed
	}

	if(!m_bWaterAtScreenRes)
	{
		// Render water texture
		if(m_bShaderRefraction && m_bShaderFresnel)
		{
			RenderWaterTexture(m_FullWaterShader);
		}
		else if(m_bShaderRefraction)
		{
			RenderWaterTexture(m_NoFresnelWaterShader);
		}
		else
		{
			RenderWaterTexture(m_BumpReflectionWaterShader);
		}
	}

	// Bind the main frame bufffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_iOriginalFBO);
	glViewport(0,0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));
	// Clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	SetView();
	SetProjection(m_fFOV);

	/*
		Draw the scene
	*/
	DrawScene();

	/*
		The water can be rendered at the screen resolution,
		or at a lower res to reduce the fragment processing workload
	*/
	if(m_bWaterAtScreenRes)
	{
		if(m_bShaderRefraction && m_bShaderFresnel)
		{
			DrawWater(m_FullWaterShader, PVRShellGet(prefWidth), PVRShellGet(prefHeight), m_vPlaneWater);
		}
		else if(m_bShaderRefraction)
		{
			DrawWater(m_NoFresnelWaterShader, PVRShellGet(prefWidth), PVRShellGet(prefHeight), m_vPlaneWater);
		}
		else
		{
			DrawWater(m_BumpReflectionWaterShader, PVRShellGet(prefWidth), PVRShellGet(prefHeight), m_vPlaneWater);
		}
	}
	else
	{
		DrawWaterFromTexture();
	}

#ifdef DEBUG_MODE
	if(m_bDisplayDebugWindows)
	{
		// Display reflection, refraction and water textures to debug windows
		DrawTestQuad(m_auiRendToTexture[eREFLECTION_FBO],PVRTVec2(-1.0f,-0.8f));
		DrawTestQuad(m_auiRendToTexture[eREFRACTION_FBO],PVRTVec2(-1.0f,-0.25f));
		DrawTestQuad(m_auiRendToTexture[eWATER_FBO],	 PVRTVec2(-1.0f, 0.325f));
	}
#endif

#ifdef ENABLE_UI
	// UI keyboard input
	if(PVRShellIsKeyPressed(PVRShellKeyNameRIGHT))
	{
		if(++m_iCurrentUIOption >= (int)eUI_SIZE)
		{
			m_iCurrentUIOption = 0;
		}
	}
	if(PVRShellIsKeyPressed(PVRShellKeyNameLEFT))
	{
		if(--m_iCurrentUIOption < 0)
		{
			m_iCurrentUIOption = eUI_SIZE - 1;
		}
	}
	if(PVRShellIsKeyPressed(PVRShellKeyNameSELECT) || PVRShellIsKeyPressed(PVRShellKeyNameACTION1))
	{
		m_bPause = !m_bPause;
	}

	// UI options
	switch(m_iCurrentUIOption)
	{
		case eUI_NULL: break;
		case eTOGGLE_REFRACTION:
			{
				if (PVRShellIsKeyPressed(PVRShellKeyNameUP) || PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
				{
					m_bShaderRefraction = !m_bShaderRefraction;
				}
				m_Print3D.Print3D(2.0f,90.0f,0.75f, 0xffffffff, "Toggle refraction : %1i", m_bShaderRefraction);
				break;
			}
		case eTOGGLE_FRESNEL:
				if (PVRShellIsKeyPressed(PVRShellKeyNameUP) || PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
				{
					m_bShaderFresnel = !m_bShaderFresnel;
				}
				m_Print3D.Print3D(2.0f,90.0f,0.75f, 0xffffffff, "Toggle Fresnel : %1i", m_bShaderFresnel);
				break;
		case eTOGGLE_FOG:
			{
				if (PVRShellIsKeyPressed(PVRShellKeyNameUP) || PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
				{
					m_bShaderFogging = !m_bShaderFogging;
				}

				m_Print3D.Print3D(2.0f,90.0f,0.75f, 0xffffffff, "Toggle depth fogging : %1i", m_bShaderFogging);
				break;
			}
		case eFOG_DEPTH:
			{
				if (PVRShellIsKeyPressed(PVRShellKeyNameUP))
				{
					m_fMaxFogDepth += 1.0f;
				}
				else if(PVRShellIsKeyPressed(PVRShellKeyNameDOWN) && (m_fMaxFogDepth > 0))
				{
					m_fMaxFogDepth -= 1.0f;
				}
				m_Print3D.Print3D(2.0f,90.0f,0.75f, 0xffffffff, "Fog depth : %4.2f", m_fMaxFogDepth);
				break;
			}
		case eWAVE_DISTORTION:
			{
				if (PVRShellIsKeyPressed(PVRShellKeyNameUP))
				{
					m_fWaveDistortion += 1.0f;
				}
				else if(PVRShellIsKeyPressed(PVRShellKeyNameDOWN)&& (m_fWaveDistortion - 0.01f >= 0.0f))
				{
					m_fWaveDistortion -= 1.0f;
				}
				m_Print3D.Print3D(2.0f,90.0f,0.75f, 0xffffffff, "Wave distortion : %4.2f", m_fWaveDistortion);
				break;
			}
		case eARTEFACT_FIX:
			{
				if (PVRShellIsKeyPressed(PVRShellKeyNameUP))
				{
					m_fWaterArtefactFix += 0.1f;
				}
				else if(PVRShellIsKeyPressed(PVRShellKeyNameDOWN)&& (m_fWaterArtefactFix - 0.1f >= 0.0f))
				{
					m_fWaterArtefactFix -= 0.1f;
				}
				m_Print3D.Print3D(2.0f,90.0f,0.75f, 0xffffffff, "Water's edge artifact fix : %4.2f", m_fWaterArtefactFix);
				break;
			}
		case eRENDER_WATER_SCREEN_RES:
			{
				if (PVRShellIsKeyPressed(PVRShellKeyNameUP) || PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
				{
					m_bWaterAtScreenRes = !m_bWaterAtScreenRes;
				}
				m_Print3D.Print3D(2.0f,90.0f,0.75f, 0xffffffff, "Water rendered at screen resolution : %1i", m_bWaterAtScreenRes);
				break;
			}

	#ifdef DEBUG_MODE
		#ifdef FREE_CAMERA_MODE
			case eMOVE_X:
				{
					if (PVRShellIsKeyPressed(PVRShellKeyNameUP))
					{
						m_vEyePos.x += 1.0f;
						m_vLookAt.x += 1.0f;
					}
					else if(PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
					{
						m_vEyePos.x -= 1.0f;
						m_vLookAt.x -= 1.0f;
					}
					m_Print3D.Print3D(2.0f, 90.0f, 0.75f, 0xffffffff, "MOVE: Camera x-axis : %4.2f \nLook at x-axis : %4.2f", m_vEyePos.x, m_vLookAt.x);
					break;
				}
			case eMOVE_Y:
				{
					if (PVRShellIsKeyPressed(PVRShellKeyNameUP))
					{
						m_vEyePos.y += 1.0f;
						m_vLookAt.y += 1.0f;
					}
					else if(PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
					{
						m_vEyePos.y -= 1.0f;
						m_vLookAt.y -= 1.0f;
					}
					m_Print3D.Print3D(2.0f,90.0f,0.75f, 0xffffffff, "MOVE: Camera y-axis : %4.2f \nLook at y-axis : %4.2f", m_vEyePos.y, m_vLookAt.y);
					break;
				}
			case eMOVE_Z:
				{
					if (PVRShellIsKeyPressed(PVRShellKeyNameUP))
					{
						m_vEyePos.z += 1.0f;
						m_vLookAt.z += 1.0f;
					}
					else if(PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
					{
						m_vEyePos.z -= 1.0f;
						m_vLookAt.z -= 1.0f;
					}
					m_Print3D.Print3D(2.0f,90.0f,0.75f, 0xffffffff, "MOVE: Camera z-axis : %4.2f \nLook at z-axis : %4.2f", m_vEyePos.z, m_vLookAt.z);
					break;
				}
			case eCAMERA_X:
				{
					if (PVRShellIsKeyPressed(PVRShellKeyNameUP))
					{
						m_vEyePos.x += 1.0f;
					}
					else if(PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
					{
						m_vEyePos.x -= 1.0f;
					}
					m_Print3D.Print3D(2.0f, 90.0f, 0.75f, 0xffffffff, "Camera x-axis : %4.2f", m_vEyePos.x);
					break;
				}
			case eCAMERA_Y:
				{
					if (PVRShellIsKeyPressed(PVRShellKeyNameUP))
					{
						m_vEyePos.y += 1.0f;
					}
					else if(PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
					{
						m_vEyePos.y -= 1.0f;
					}
					m_Print3D.Print3D(2.0f,90.0f,0.75f, 0xffffffff, "Camera y-axis : %4.2f", m_vEyePos.y);
					break;
				}
			case eCAMERA_Z:
				{
					if (PVRShellIsKeyPressed(PVRShellKeyNameUP))
					{
						m_vEyePos.z += 1.0f;
					}
					else if(PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
					{
						m_vEyePos.z -= 1.0f;
					}
					m_Print3D.Print3D(2.0f,90.0f,0.75f, 0xffffffff, "Camera z-axis : %4.2f", m_vEyePos.z);
					break;
				}
			case eLOOK_AT_X:
				{
					if (PVRShellIsKeyPressed(PVRShellKeyNameUP))
					{
						m_vLookAt.x += 1.0f;
					}
					else if(PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
					{
						m_vLookAt.x -= 1.0f;
					}
					m_Print3D.Print3D(2.0f, 90.0f, 0.75f, 0xffffffff, "Look at x-axis : %4.2f", m_vLookAt.x);
					break;
				}
			case eLOOK_AT_Y:
				{
					if (PVRShellIsKeyPressed(PVRShellKeyNameUP))
					{
						m_vLookAt.y += 1.0f;
					}
					else if(PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
					{
						m_vLookAt.y -= 1.0f;
					}
					m_Print3D.Print3D(2.0f,90.0f,0.75f, 0xffffffff, "Look at y-axis : %4.2f", m_vLookAt.y);
					break;
				}
			case eLOOK_AT_Z:
				{
					if (PVRShellIsKeyPressed(PVRShellKeyNameUP))
					{
						m_vLookAt.z += 1.0f;
					}
					else if(PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
					{
						m_vLookAt.z -= 1.0f;
					}
					m_Print3D.Print3D(2.0f,90.0f,0.75f, 0xffffffff, "Look at z-axis : %4.2f", m_vLookAt.z);
					break;
				}
		#endif
		case eWATER_HEIGHT:
			{
				if (PVRShellIsKeyPressed(PVRShellKeyNameUP))
				{
					m_vPlaneWater.w -= 0.2f;
				}
				else if(PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
				{
					m_vPlaneWater.w += 0.2f;
				}
				m_Print3D.Print3D(2.0f,90.0f,0.75f, 0xffffffff, "Water height : %4.2f", -m_vPlaneWater.w); // Negate to represent in world space
				break;
			}
		case eWATER_COLOUR_R:
			{
				if (PVRShellIsKeyPressed(PVRShellKeyNameUP) && (m_vWaterColour.x + 0.05f <= 1.0f))
				{
					m_vWaterColour.x += 0.05f;
				}
				else if(PVRShellIsKeyPressed(PVRShellKeyNameDOWN)&& (m_vWaterColour.x - 0.05f > 0.0f))
				{
					m_vWaterColour.x -= 0.05f;
				}
				m_Print3D.Print3D(2.0f,90.0f,0.75f, 0xffffffff, "Water colour red : %4.2f", m_vWaterColour.x);
				break;
			}
		case eWATER_COLOUR_G:
			{
				if (PVRShellIsKeyPressed(PVRShellKeyNameUP) && (m_vWaterColour.x + 0.05f <= 1.0f))
				{
					m_vWaterColour.y += 0.05f;
				}
				else if(PVRShellIsKeyPressed(PVRShellKeyNameDOWN)&& (m_vWaterColour.x - 0.05f > 0.0f))
				{
					m_vWaterColour.y -= 0.05f;
				}
				m_Print3D.Print3D(2.0f,90.0f,0.75f, 0xffffffff, "Water colour green : %4.2f", m_vWaterColour.y);
				break;
			}
		case eWATER_COLOUR_B:
			{
				if (PVRShellIsKeyPressed(PVRShellKeyNameUP) && (m_vWaterColour.x + 0.05f <= 1.0f))
				{
					m_vWaterColour.z += 0.05f;
				}
				else if(PVRShellIsKeyPressed(PVRShellKeyNameDOWN)&& (m_vWaterColour.x - 0.05f > 0.0f))
				{
					m_vWaterColour.z -= 0.05f;
				}
				m_Print3D.Print3D(2.0f,90.0f,0.75f, 0xffffffff, "Water colour blue : %4.2f", m_vWaterColour.z);
				break;
			}
		case eTOGGLE_DEBUG_WINDOWS:
			{
				if (PVRShellIsKeyPressed(PVRShellKeyNameUP) || PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
				{
					m_bDisplayDebugWindows = !m_bDisplayDebugWindows;
				}
				m_Print3D.Print3D(2.0f,90.0f,0.75f, 0xffffffff, "Toggle debug windows : %1i", m_bDisplayDebugWindows);
				break;
			}
	#endif
	}
#endif

#ifdef DEBUG_MODE
	// Display debugging data
	m_Print3D.Print3D(2.0f, 10.0f, 0.75f, 0xffffff00, "%4i fps", m_uiFPS);
#endif

	// Displays the demo name using the Print3D tool. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("Water", "", ePVRTPrint3DSDKLogo);

	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		RenderReflectionTexture
 @Description	Renders the scence (excluding the water) so a reflection
				texture for the frame can be calculated. The water plane is used
				during clipping so that only objects above the water are rendered.
				See section 2.3 of the corresponding white paper for more information
******************************************************************************/
void OGLES3Water::RenderReflectionTexture()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_auiFBOIds[eREFLECTION_FBO]);	// Bind a frame buffer that has a texture attached (stores the render in a texture)
	glViewport(0,0, m_uiTexSize, m_uiTexSize);							// Set the viewport to the texture size
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	SetView();
	SetProjection(m_fFOV);

	// Mirror the view matrix about the plane.
	PVRTMat4 mMirrorCam(PVRTMat4::Identity());
	mMirrorCam.ptr()[1] = -m_vPlaneWater.x;
	mMirrorCam.ptr()[5] = -m_vPlaneWater.y;
	mMirrorCam.ptr()[9] = -m_vPlaneWater.z;
	mMirrorCam.ptr()[13] = -(2.0f * m_vPlaneWater.w);

	m_mView = m_mView * mMirrorCam;

	ModifyProjectionForClipping(m_vPlaneWater + PVRTVec4(0.0f,0.0f,0.0f,m_fWaterArtefactFix));

	DrawScene();

	//Invalidate the framebuffer attachments we don't need to avoid unnecessary copying to system memory
	const GLenum attachment = GL_DEPTH_ATTACHMENT;
	glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attachment);
}

/*!****************************************************************************
 @Function		RenderRefractionTexture
 @Description	Renders the scence (excluding the water) so that refraction
				(including depth, when enabled) for the frame can be calculated.
				When depth shading is enabled, the skybox is ommited from the render.
				See section 2.4 of the corresponding white paper for more information
******************************************************************************/
void OGLES3Water::RenderRefractionTexture()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_auiFBOIds[eREFRACTION_FBO]);	// Bind a frame buffer that has a texture and depth texture attached (stores the render in a texture)
	glViewport(0,0, m_uiTexSize, m_uiTexSize);
	// Use the water colour for clearing
	glClearColor(m_vWaterColour.x, m_vWaterColour.y, m_vWaterColour.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	SetView();
	SetProjection(m_fFOV);

	PVRTVec4 vPlaneView = PVRTVec4( -m_vPlaneWater.x, -m_vPlaneWater.y, -m_vPlaneWater.z, -m_vPlaneWater.w + m_fWaterArtefactFix);
	ModifyProjectionForClipping(vPlaneView);

	// Allow fogging to be toggled by the user
	if(m_bShaderFogging)
	{
		DrawRefractionScene();
	}
	else
	{
		DrawScene();
	}

	//Invalidate the framebuffer attachments we don't need to avoid unnecessary copying to system memory
	const GLenum attachment = GL_DEPTH_ATTACHMENT;
	glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attachment);
}

/*!****************************************************************************
 @Function		RenderWaterTexture
 @Input			shaderProgram	The water shader program to be applied during the render
 @Description	Render the water effect to a lower resolution texture
				that can then be applied to the plane's surface.
				See section 3.3.3 of the corresponding white paper for more information
******************************************************************************/
void OGLES3Water::RenderWaterTexture(const WaterShader& shaderProgram)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_auiFBOIds[eWATER_FBO]);
	glViewport(0,0, m_uiWaterTexSize, m_uiWaterTexSize);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	SetView();
	SetProjection(m_fFOV);

	DrawMesh(m_NodeNameIndex[eNODE_GROUND], m_LightModelShader);				// Only draw the terrain

	DrawWater(shaderProgram, m_uiWaterTexSize, m_uiWaterTexSize, m_vPlaneWater);
	//Invalidate the framebuffer attachments we don't need to avoid unnecessary copying to system memory
	const GLenum attachment = GL_DEPTH_ATTACHMENT;
	glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attachment);
}

/*!****************************************************************************
 @Function		DrawScene
 @Description	Draw all elements of the scene, excluding the water
******************************************************************************/
void OGLES3Water::DrawScene()
{
	// Draw meshes
	for (unsigned int i = 0; i < m_Mesh.nNumMeshNode; ++i)
	{
		const ModelShader* pShaderProgram = NULL;
		switch (m_NodeIndexName[i])
		{
			case eNODE_COINS:
				pShaderProgram = &m_SpecularModelShader;
				break;
			case eNODE_SHIPFLAG:
				pShaderProgram = &m_PerturbedModelShader;
				break;
			default:
				pShaderProgram = &m_LightModelShader;
				break;
		}

		DrawMesh(i, *pShaderProgram);
	}

	// Reset the projection before the skyox so that the sky box wont be clipped, as it should appear infinite.
	SetProjection(m_fFOV);

	DrawSkybox(m_auiTextureIds[eSKYBOX_TEX], m_SkyboxShader, eSKYBOX_VBO, PVRTVec3(0.0f, 0.0f, 0.0f));
}

/*!****************************************************************************
 @Function		DrawRefractionScene
 @Description	Draw
				all elements of the scene, excluding the water and the sky box.
******************************************************************************/
void OGLES3Water::DrawRefractionScene()
{
	for (unsigned int i = 0; i < m_Mesh.nNumMeshNode; ++i)
	{
		const ModelShader* pShaderProgram = NULL;
		switch (m_NodeIndexName[i])
		{
			case eNODE_COINS:
			    pShaderProgram = &m_SpecularModelShader;  // Override the incoming shader.
			    break;
			default:
			    pShaderProgram = &m_FogModelShader;
			    break;
		}
		DrawMesh(i, *pShaderProgram);
	}
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			uiModelNumber		The element to draw from the POD array
 @Input			i32NodeIndex		Node index of the mesh to draw
 @Input			shaderProgram		The water shader program to be applied during the render
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the material has been prepared.
******************************************************************************/
void OGLES3Water::DrawMesh(int i32NodeIndex, const ModelShader& shaderProgram)
{
	SPODNode& node = m_Mesh.pNode[i32NodeIndex];
	int i32MeshIndex = m_Mesh.pNode[i32NodeIndex].nIdx;
	SPODMesh* pMesh = &m_Mesh.pMesh[i32MeshIndex];

	// Load the correct texture using the texture lookup table
	GLuint uiDiffuseTex = 0, uiSpecularTex = 0;
	if(node.nIdxMaterial != -1)
	{
		uiDiffuseTex  = m_apuiModelTextureIds[node.nIdxMaterial].uiDiffuse;
		uiSpecularTex = m_apuiModelTextureIds[node.nIdxMaterial].uiSpecular;
	}
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, uiDiffuseTex);

	// Activate the specular map unit
	if(uiSpecularTex)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, uiSpecularTex);
		glActiveTexture(GL_TEXTURE0);
	}

	// Use shader program
	glUseProgram(shaderProgram.uiId);

	/*
		Set the shading parameters
	*/

	// Extract the world matrix for the model from the POD file
	PVRTMat4 mModel(m_Mesh.GetWorldMatrix(node));

	PVRTMat4 mModelView(m_mView * mModel);
	glUniformMatrix4fv(shaderProgram.uiModelMatrixLoc,1, GL_FALSE, mModel.ptr());

	// Set eye position in world space
	glUniform3fv(shaderProgram.uiEyePosLoc, 1, &m_vEyePos.x);

	PVRTMat4 mMVP(m_mProjection * mModelView);
	glUniformMatrix4fv(shaderProgram.uiMVPMatrixLoc,1, GL_FALSE, mMVP.ptr());

	glUniform3fv(shaderProgram.uiLightDirectionLoc, 1, &m_vLightDirection.x);

	glUniform1f(shaderProgram.uiWaterHeightLoc, -m_vPlaneWater.w);		// Negate the scale to represent the water's distance along the y-axis in world coordinates
	glUniform3fv(shaderProgram.uiFogColourLoc,1, &m_vWaterColour.x);	// Only requires the rgb values of colour
	glUniform1f(shaderProgram.uiMaxFogDepthLoc, 1.0f/m_fMaxFogDepth);	// Invert fog depth to avoid division in fragment shader
	glUniform1f(shaderProgram.uiTimeLoc, m_fElapsedTimeInSecs * m_fWindSpeed);

	// Colours
	SPODMaterial& mat = m_Mesh.pMaterial[node.nIdxMaterial];
	glUniform3fv(shaderProgram.uiDiffuseColLoc,  1, mat.pfMatDiffuse);
	glUniform3fv(shaderProgram.uiEmissiveColLoc, 1, mat.pfMatAmbient);
	glUniform3fv(shaderProgram.uiSpecularColLoc, 1, mat.pfMatSpecular);

	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, m_apuiModelVbo[i32MeshIndex]);
	// bind the index buffer, won't hurt if the handle is 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_apuiModelIndexVbo[i32MeshIndex]);

	// Enable the vertex attribute arrays
	glEnableVertexAttribArray(VERTEX_ARRAY);
	glEnableVertexAttribArray(NORMAL_ARRAY);
	glEnableVertexAttribArray(TEXCOORD_ARRAY);

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
		if(m_apuiModelIndexVbo[i32MeshIndex])
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
			if(m_apuiModelIndexVbo[i32MeshIndex])
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
 @Function		DrawInfinitePlane
 @Input			vPlane				The plane (in the form (A,B,C,D)) that represents the plane in the world
 @Input			fFarDistance		The far clip plane distance
 @Description	Draws an infinite plane using variables from the program
******************************************************************************/
void OGLES3Water::DrawInfinitePlane(const PVRTVec4& vPlane, float fFarDistance)
{
	// Calc ViewProjInv matrix
	PVRTMat4 mTmp(m_mProjection * m_mView);
	mTmp = mTmp.inverseEx();

	// Calculate the water plane
	m_i32WaterPlaneNo = PVRTMiscCalculateInfinitePlane(&m_pvPlaneWater->x, sizeof(*m_pvPlaneWater), &vPlane, &mTmp, &m_vEyePos, fFarDistance);

	glDisable(GL_CULL_FACE);

	// Enable the vertex attribute arrays
	glEnableVertexAttribArray(VERTEX_ARRAY);

	// Draw water
	if(m_i32WaterPlaneNo)
	{
		// Set the vertex attribute offsets
		glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, 0, &m_pvPlaneWater->x);

		// Draw primitive
		glDrawArrays(GL_TRIANGLE_FAN, 0, m_i32WaterPlaneNo);
	}

	// Safely disable the vertex attribute arrays
	glDisableVertexAttribArray(VERTEX_ARRAY);

	glEnable(GL_CULL_FACE);
}

/*!****************************************************************************
 @Function		DrawWater
 @Input			shaderProgram		The shader to be applied to the water plane
 @Input			uiViewPortWidth		The width of current viewport
 @Input			uiViewPortHeight	The height of the current viewport
 @Input			vPlane				The plane (in the form (A,B,C,D)) that represents the plane in the world
 @Input			fFarDistance		The far clip plane distance
 @Description	Draws the water
******************************************************************************/
void OGLES3Water::DrawWater(const WaterShader& shaderProgram, GLuint uiViewPortWidth, GLuint uiViewPortHeight, const PVRTVec4& vPlane, float fFarDistance)
{
	// Use shader program
	glUseProgram(shaderProgram.uiId);

	// Bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_auiTextureIds[eWATER_NORMAL_TEX]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_auiRendToTexture[eREFLECTION_FBO]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_auiRendToTexture[eREFRACTION_FBO]);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_uiNormalisationCubeMap);

	// Set model view matrix for water
	PVRTMat4 mModelView(m_mView);			// Model matrix is assumed to be identity
	glUniformMatrix4fv(shaderProgram.uiMVMatrixLoc, 1, GL_FALSE, mModelView.ptr());

	// Set model view projection matrix for water
	PVRTMat4 mMVP(m_mProjection * mModelView);
	glUniformMatrix4fv(shaderProgram.uiMVPMatrixLoc, 1, GL_FALSE, mMVP.ptr());


	// Set eye position in model space
	PVRTVec4 vEyePosModel(mModelView.inverseEx() * PVRTVec4(0.0f, 0.0f, 0.0f, 1.0f));
	glUniform3fv(shaderProgram.uiEyePosLoc, 1, &vEyePosModel.x);

	/*
		Set the remaining shader parameters
	*/
	glUniform4fv(shaderProgram.uiWaterColourLoc,1, &m_vWaterColour.x);

	if(!m_bPause)
	{
		m_vBumpTranslation0 += m_vBumpVelocity0 * m_fDeltaTime;
		m_vBumpTranslation0 = PVRTVec2(	fmod(m_vBumpTranslation0.x, 1.0f),
										fmod(m_vBumpTranslation0.y, 1.0f));
		m_vBumpTranslation1 += m_vBumpVelocity1 * m_fDeltaTime;
		m_vBumpTranslation1 = PVRTVec2(	fmod(m_vBumpTranslation1.x, 1.0f),
										fmod(m_vBumpTranslation1.y, 1.0f));
	}

	glUniform2fv(shaderProgram.uiBumpTranslation0Loc,1, &m_vBumpTranslation0.x);
	glUniform2fv(shaderProgram.uiBumpScale0Loc,1, &m_vBumpScale0.x);
	glUniform2fv(shaderProgram.uiBumpTranslation0Loc,1, &m_vBumpTranslation1.x);
	glUniform2fv(shaderProgram.uiBumpScale1Loc,1, &m_vBumpScale1.x);
	glUniform1f(shaderProgram.uiWaveDistortionLoc, m_fWaveDistortion);
	glUniform1f(shaderProgram.uiRcpMaxFogDepthLoc, 1.0f/m_fMaxFogHeight);
	glUniform4fv(shaderProgram.uiFogColourLoc, 1, &m_vFogColour.x);

	m_vRcpWindowSize.x = 1.0f/uiViewPortWidth;
	m_vRcpWindowSize.y = 1.0f/uiViewPortHeight;

	glUniform2fv(shaderProgram.uiRcpWindowSizeLoc,1, &m_vRcpWindowSize.x);

	DrawInfinitePlane(vPlane, fFarDistance);
}

/*!****************************************************************************
 @Function		DrawWaterFromTexture
 @Input			fFarDistance			The far clip plane distance
 @Description	Renders a plane that is textured with the texture created in the
				water texture render pass
******************************************************************************/
void OGLES3Water::DrawWaterFromTexture(float fFarDistance)
{
	// Use shader program
	glUseProgram(m_PlaneTexShader.uiId);

	// Bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_auiRendToTexture[eWATER_FBO]);

	PVRTMat4 mModelView(m_mView);		// Model is assumed to be the identity matrix
	PVRTMat4 mMVP(m_mProjection * mModelView);
	glUniformMatrix4fv(m_PlaneTexShader.uiMVPMatrixLoc, 1, GL_FALSE, mMVP.ptr());

	m_vRcpWindowSize.x = 1.0f/PVRShellGet(prefWidth);
	m_vRcpWindowSize.y = 1.0f/PVRShellGet(prefHeight);

	glUniform2fv(m_PlaneTexShader.uiRcpWindowSizeLoc,1, &m_vRcpWindowSize.x);

	DrawInfinitePlane(m_vPlaneWater, fFarDistance);
}

/*!****************************************************************************
 @Function		DrawSkybox
 @Input			uiCubeMapHandle		The handle to the skybox's cube map
 @Input			shaderProgram		The shader to apply to the skybox
 @Input			uiVboId				The id of the skybox's VBO
 @Input			vTranslation		The translation appied to the skybox
 @Description	Draws the skybox
******************************************************************************/
void OGLES3Water::DrawSkybox(GLuint uiCubeMapHandle, const SkyboxShader& shaderProgram, GLuint uiVboId,
							 const PVRTVec3& vTranslation)
{
	// Use shader program
	glUseProgram(shaderProgram.uiId);

	// Bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, uiCubeMapHandle);

	// Rotate and Translate the model matrix (if required)
	PVRTMat4 mModel(PVRTMat4::Identity());
	mModel *= mModel.Translation(vTranslation.x,vTranslation.y,vTranslation.z);
	glUniformMatrix4fv(shaderProgram.uiModelMatrixLoc, 1, GL_FALSE, mModel.ptr());

	// Set model view projection matrix
	PVRTMat4 mModelView(m_mView * mModel);
	PVRTMat4 mMVP(m_mProjection * mModelView);
	glUniformMatrix4fv(shaderProgram.uiMVPMatrixLoc, 1, GL_FALSE, mMVP.ptr());

	// Set eye position in model space
	PVRTVec4 vEyePosModel(mModelView.inverse() * PVRTVec4(0.0f, 0.0f, 0.0f, 1.0f));
	glUniform3fv(shaderProgram.uiEyePosLoc, 1, &vEyePosModel.x);

	glUniform1f(shaderProgram.uiWaterHeightLoc, -m_vPlaneWater.w);		// Negate the scale to represent it in world sapce
	glUniform4fv(shaderProgram.uiFogColourLoc, 1, &m_vFogColour.x);	// Only requires the rgb values of colour
	glUniform1f(shaderProgram.uiMaxFogDepthLoc, 1.0f/(m_fMaxFogHeight/5.0f));	// Invert fog depth to save division in fragment shader. Compress the height.

	glDisable(GL_CULL_FACE);

	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, m_auiVBOIds[uiVboId]);
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*3, NULL);

	// Enable the vertex attribute arrays
	glEnableVertexAttribArray(VERTEX_ARRAY);

	for(int i = 0; i < 6; ++i)
	{
		// Draw primitive
		glDrawArrays(GL_TRIANGLE_STRIP, i*4, 4);
	}

	// Safely disable the vertex attribute arrays
	glDisableVertexAttribArray(VERTEX_ARRAY);

	glEnable(GL_CULL_FACE);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

/*!****************************************************************************
 @Function		DrawTestQuad
 @Input			uiTextureHandle		The texture to apply to the quad
 @Input			vBottomLeftPosition	The bottom left position of the quad on the screen
 @Description	Draws a small quad to the screen where textures like reflection etc
				can be drawn for debugging purposes
******************************************************************************/
void OGLES3Water::DrawTestQuad(GLuint uiTextureHandle, const PVRTVec2 &vBottomLeftPosition)
{
	// Check the ortho values!
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);
	m_mProjection = PVRTMat4::Ortho(-1,1,1,-1,0,1,PVRTMat4::OGL, bRotate);

	PVRTVec4 vVertices[4];
	PVRTVec2 vTexCoords[4];

	GLfloat fQuadSize = 0.5f;

	vVertices[0] = PVRTVec4(vBottomLeftPosition.x,				vBottomLeftPosition.y + fQuadSize,	0, 1);
	vVertices[1] = PVRTVec4(vBottomLeftPosition.x,				vBottomLeftPosition.y,				0, 1);
	vVertices[2] = PVRTVec4(vBottomLeftPosition.x + fQuadSize,	vBottomLeftPosition.y,				0, 1);
	vVertices[3] = PVRTVec4(vBottomLeftPosition.x + fQuadSize,	vBottomLeftPosition.y + fQuadSize,	0, 1);

	vTexCoords[0] = PVRTVec2(0,1);
	vTexCoords[1] = PVRTVec2(0,0);
	vTexCoords[2] = PVRTVec2(1,0);
	vTexCoords[3] = PVRTVec2(1,1);

	// Use shader program
	glUseProgram(m_Tex2DShader.uiId);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, uiTextureHandle);

	// Dont pass in MVP as you want to align quad with screen (projection will distort)
	glUniformMatrix4fv(m_Tex2DShader.uiMVPMatrixLoc, 1, GL_FALSE, m_mProjection.ptr());

	glDisable(GL_CULL_FACE);

	// Enable the vertex attribute arrays
	glEnableVertexAttribArray(VERTEX_ARRAY);
	glEnableVertexAttribArray(TEXCOORD_ARRAY);

	glVertexAttribPointer(VERTEX_ARRAY, 4, GL_FLOAT, GL_FALSE, 0, &vVertices);
	glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, &vTexCoords);

	// Draw primitive
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// Safely disable the vertex attribute arrays
	glDisableVertexAttribArray(VERTEX_ARRAY);
	glDisableVertexAttribArray(TEXCOORD_ARRAY);

	glEnable(GL_CULL_FACE);
}

/*!****************************************************************************
 @Function		SetProjection
 @Input			fVOV		The field of view required.
 @Input			fFarClip	The far clip plane
 @Description	Sets the projection matrix using the width, height, near clipping,
				far clipping etc.
******************************************************************************/
void OGLES3Water::SetProjection(const float fFOV, const float fFarClip)
{
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	const float fAspect = (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight);

	m_mProjection = PVRTMat4::PerspectiveFovRH(fFOV, fAspect, CAM_NEAR, fFarClip, PVRTMat4::OGL, bRotate);
}

/*!****************************************************************************
 @Function		SetView
 @Description	Sets the view matrix using the camera variables
******************************************************************************/
void OGLES3Water::SetView()
{
#ifdef FREE_CAMERA_MODE
	m_mView = PVRTMat4::LookAtRH(m_vEyePos, m_vLookAt, m_vCamUp);
#else
	int i32CamID = m_Mesh.pNode[m_Mesh.nNumMeshNode + m_Mesh.nNumLight + c_uiCamera].nIdx;

	// Get the camera position, target and field of view
	if(m_Mesh.pCamera[i32CamID].nIdxTarget != -1)
	{
		m_fFOV = m_Mesh.GetCameraPos(m_vEyePos, m_vLookAt, c_uiCamera);			// vLookAt is taken from the target node
	}
	else
	{
		m_fFOV = m_Mesh.GetCamera(m_vEyePos, m_vLookAt, m_vCamUp, c_uiCamera);	// vLookAt is calculated from the rotation
	}

	// Create the view matrix from the camera information extracted from the pod
	m_mView = PVRTMat4::LookAtRH(m_vEyePos, m_vLookAt, m_vCamUp);
#endif
}
/*!****************************************************************************
 @Function		ModifyProjectionForClipping
 @Input			vClipPlane	The user defined clip plane
 @Description	Modifies the projection matrix so that the near clipping plane
				matches that of the clip plane that has been passed in. This allows
				the projection matrix to be used to perform clipping -
				See section 3.1 of the corresponding white paper for more information
******************************************************************************/
void OGLES3Water::ModifyProjectionForClipping(const PVRTVec4 &vClipPlane)
{
	PVRTVec4 vClipPlaneView(vClipPlane * m_mView.inverseEx());	// put clip plane into view coords
	/*
		Calculate the clip-space corner point opposite the clipping plane
		and transform it into camera space by multiplying it by the inverse
		projection matrix.
	*/
	PVRTVec4 vClipSpaceCorner(sgn(vClipPlaneView.x),sgn(vClipPlaneView.y),1.0f,1.0f);
	vClipSpaceCorner *= m_mProjection.inverseEx();

	// Calculate the scaled plane vector
	PVRTVec4 vScaledPlane = vClipPlaneView * (2.0f / vClipSpaceCorner.dot(vClipPlaneView));

	// Replace the third row of the matrix
	m_mProjection.ptr()[2] = vScaledPlane.x;
	m_mProjection.ptr()[6] = vScaledPlane.y;
	m_mProjection.ptr()[10] = vScaledPlane.z + 1.0f;
	m_mProjection.ptr()[14] = vScaledPlane.w;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	//	Generate normalisation cube map
//	Downloaded from: www.paulsprojects.net
//	Created:	20th July 2002
//
//	Copyright (c) 2006, Paul Baker
//	Distributed under the New BSD Licence. (http://www.paulsprojects.net/NewBSDLicense.txt)
//////////////////////////////////////////////////////////////////////////////////////////
/*!****************************************************************************
 @Function		GenerateNormalisationCubeMap
 @Input			uiTextureSize		The size of the cube map's textures
 @Description	Generates a normalization cube map for the shaders to use -
				See section 3.3.1 of the whitepaper for more information
******************************************************************************/
bool OGLES3Water::GenerateNormalisationCubeMap(int uiTextureSize)
{
	// variables
	float fOffset = 0.5f;
	float fHalfSize = uiTextureSize *0.5f;
	PVRTVec3 vTemp;
	unsigned char* pByte;
	unsigned char* pData = new unsigned char[uiTextureSize*uiTextureSize*3];
    if(!pData)
    {
        PVRShellOutputDebug("Unable to allocate memory for texture data for cube map\n");
        return false;
    }

	// Positive X
	pByte = pData;

	for(int j = 0; j < uiTextureSize; ++j)
	{
		for(int i = 0; i < uiTextureSize; ++i)
		{
			vTemp.x = fHalfSize;
			vTemp.y = -(j + fOffset - fHalfSize);
			vTemp.z = -(i + fOffset - fHalfSize);

			// normalize, pack 0 to 1 here, and normalize again
			vTemp = vTemp.normalize() *0.5 + 0.5;

			pByte[0] = (unsigned char)(vTemp.x * 255);
			pByte[1] = (unsigned char)(vTemp.y * 255);
			pByte[2] = (unsigned char)(vTemp.z * 255);

			pByte += 3;
		}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB /*GL_RGBA8*/, uiTextureSize, uiTextureSize, 0, GL_RGB, GL_UNSIGNED_BYTE, pData);

	// Negative X
	pByte = pData;

	for(int j = 0; j < uiTextureSize; ++j)
	{
		for(int i = 0; i < uiTextureSize; ++i)
		{
			vTemp.x = -fHalfSize;
			vTemp.y = -(j + fOffset - fHalfSize);
			vTemp.z = (i + fOffset - fHalfSize);


			// normalize, pack 0 to 1 here, and normalize again
			vTemp = vTemp.normalize() *0.5 + 0.5;

			pByte[0] = (unsigned char)(vTemp.x * 255);
			pByte[1] = (unsigned char)(vTemp.y * 255);
			pByte[2] = (unsigned char)(vTemp.z * 255);

			pByte += 3;
		}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB /*GL_RGBA8*/, uiTextureSize, uiTextureSize, 0, GL_RGB, GL_UNSIGNED_BYTE, pData);

	// Positive Y
	pByte = pData;

	for(int j = 0; j < uiTextureSize; ++j)
	{
		for(int i = 0; i < uiTextureSize; ++i)
		{
			vTemp.x = i + fOffset - fHalfSize;
			vTemp.y = fHalfSize;
			vTemp.z = j + fOffset - fHalfSize;

			// normalize, pack 0 to 1 here, and normalize again
			vTemp = vTemp.normalize() *0.5 + 0.5;

			pByte[0] = (unsigned char)(vTemp.x * 255);
			pByte[1] = (unsigned char)(vTemp.y * 255);
			pByte[2] = (unsigned char)(vTemp.z * 255);

			pByte += 3;
		}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB /*GL_RGBA8*/, uiTextureSize, uiTextureSize, 0, GL_RGB, GL_UNSIGNED_BYTE, pData);

	// Negative Y
	pByte = pData;

	for(int j = 0; j < uiTextureSize; ++j)
	{
		for(int i = 0; i < uiTextureSize; ++i)
		{
			vTemp.x = i + fOffset - fHalfSize;
			vTemp.y = -fHalfSize;
			vTemp.z = -(j + fOffset - fHalfSize);

			// normalize, pack 0 to 1 here, and normalize again
			vTemp = vTemp.normalize() *0.5 + 0.5;

			pByte[0] = (unsigned char)(vTemp.x * 255);
			pByte[1] = (unsigned char)(vTemp.y * 255);
			pByte[2] = (unsigned char)(vTemp.z * 255);

			pByte += 3;
		}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB /*GL_RGBA8*/, uiTextureSize, uiTextureSize, 0, GL_RGB, GL_UNSIGNED_BYTE, pData);

	// Positive Z
	pByte = pData;

	for(int j = 0; j < uiTextureSize; ++j)
	{
		for(int i = 0; i < uiTextureSize; ++i)
		{
			vTemp.x = i + fOffset - fHalfSize;
			vTemp.y = -(j + fOffset - fHalfSize);
			vTemp.z = fHalfSize;

			// normalize, pack 0 to 1 here, and normalize again
			vTemp = vTemp.normalize() *0.5 + 0.5;

			pByte[0] = (unsigned char)(vTemp.x * 255);
			pByte[1] = (unsigned char)(vTemp.y * 255);
			pByte[2] = (unsigned char)(vTemp.z * 255);

			pByte += 3;
		}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB /*GL_RGBA8*/, uiTextureSize, uiTextureSize, 0, GL_RGB, GL_UNSIGNED_BYTE, pData);

	// Negative Z
	pByte = pData;

	for(int j = 0; j < uiTextureSize; ++j)
	{
		for(int i = 0; i < uiTextureSize; ++i)
		{
			vTemp.x = -(i + fOffset - fHalfSize);
			vTemp.y = -(j + fOffset - fHalfSize);
			vTemp.z = -fHalfSize;

			// normalize, pack 0 to 1 here, and normalize again
			vTemp = vTemp.normalize() *0.5 + 0.5;

			pByte[0] = (unsigned char)(vTemp.x * 255);
			pByte[1] = (unsigned char)(vTemp.y * 255);
			pByte[2] = (unsigned char)(vTemp.z * 255);

			pByte += 3;
		}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB /*GL_RGBA8*/, uiTextureSize, uiTextureSize, 0, GL_RGB, GL_UNSIGNED_BYTE, pData);


	delete[] pData;

	return true;
}

/*!****************************************************************************
 @Function		UpdateTimer
 @Input			none
 @Description	Updates the values of the current time, previous time, current time
				in seconds, delta time and the FPS counter used in the program
******************************************************************************/
void OGLES3Water::UpdateTimer()
{
	m_uiFrameCount++;

	m_ulPreviousTime = m_ulCurrentTime;
	m_ulCurrentTime = PVRShellGetTime();

	m_fElapsedTimeInSecs = m_ulCurrentTime * 0.001f;
	m_fDeltaTime = ((float)(m_ulCurrentTime - m_ulPreviousTime))*0.001f;

	m_fCount += m_fDeltaTime;

	if(m_fCount >= 1.0f)			// Update FPS once a second
	{
		m_uiFPS = m_uiFrameCount;
		m_uiFrameCount = 0;
		m_fCount = 0;
	}
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
	return new OGLES3Water();
}

/******************************************************************************
 End of file (OGLES3Water.cpp)
******************************************************************************/

