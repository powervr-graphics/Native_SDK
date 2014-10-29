/******************************************************************************

 @File         OGLES3Navigation3D.cpp

 @Title        Navigation

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Demonstrates a method of rendering a 3D navigation application
               using OpenGL ES 3.0 and various culling techniques

******************************************************************************/

#include "PVRShell.h"
#include "OGLES3Tools.h"

#ifndef FLT_MAX
#define FLT_MAX 3.402823466E+38F
#endif

#define ENABLE_UI
//#define ENABLE_ADVANCED_OUTPUT

/******************************************************************************
 Constants
******************************************************************************/
const float c_UserCameraMovementSpeed = 2.0f;
const float c_CameraMovementSpeedScale = 0.075f;
const float c_fShadowVolumesMaxDistance = 0.25f;

/******************************************************************************
 Defines
******************************************************************************/

// Index the attributes that are bound to vertex shaders
#define VERTEX_ARRAY	 0
#define NORMAL_ARRAY	 1
#define TEXCOORD_ARRAY   2

#define VERTEX_ARRAY_UI	  0
#define TEXCOORD_ARRAY_UI 1

// Culling results
#define INTERSECT_NONE    0
#define INTERSECT_PARTIAL 1
#define INTERSECT_FULL    2

// Button and feature IDs
#define STATE_PAUSE      0
#define STATE_SHADOW     1
#define STATE_DEBUG      2
#define STATE_CULLING    3
#define STATE_OCCLUSION  4
#define STATE_INPUT_MODE 5
#define STATE_UI         6
#define NUM_STATES       7

/****************************************************************************
** Structures
****************************************************************************/

/*!***********************************************************************
 *	@Struct PVRTModelVertex
 ************************************************************************/
struct PVRTModelVertex
{
	PVRTVec3 position;
	PVRTVec3 normal;
	PVRTVec2 texcoord;
};
const unsigned int SNormalOffset = (unsigned int)sizeof(PVRTVec3);
const unsigned int STexCoordOffset = SNormalOffset + (unsigned int)sizeof(PVRTVec3);

/*!***********************************************************************
 *	@Struct PVRTBoundingBox2D
 ************************************************************************/
struct PVRTBoundingBox2D
{
	// Min and max coordinates
	PVRTVec2 minCoords;
	PVRTVec2 maxCoords;
};

/*!***********************************************************************
 *	@Struct PVRTBoundingBox3D
 ************************************************************************/
struct PVRTBoundingBox3D
{
	// Min and max coordinates
	PVRTVec3 minCoords;
	PVRTVec3 maxCoords;
};

/*!***********************************************************************
 *	@Struct PVRTVisibilityInfo
 *  @Brief  Used to store the visible city blocks and the chosen level of detail.
 ************************************************************************/
struct PVRTVisibilityInfo
{
	int            tile;
	unsigned short lod;
	unsigned short visibility;
};


/*!**************************************************************************
 *	@Struct PVRTCityBlockEntity
 *  @Brief  Describes an element of a city block (roads/buildings/landmarks/etc).
 *          Contains the bounding box for efficient culling, a reference index
 *          for the original Collada node and various attributes required for rendering.
 ****************************************************************************/
struct PVRTCityBlockEntity
{
	PVRTBoundingBox2D boundingbox;
	PVRTuint32        numSubObjects;
	unsigned int     *pNodeIdx;
	unsigned int     *paNumIndices;
	unsigned int     *paIndexOffsets;
	GLuint           *pauiTextures;
};

/*!**************************************************************************
 *	@Struct PVRTCityBlockLod
 *  @Brief  A model tile LOD stores the filename of the POD file and various
 *          attributes like the number of (parent) objects it contains.
 *          Furthermore it stores two OpenGL buffer object identifiers, one
 *          for the vertex and another one for the index data.
 *          The last members keep a list of visible nodes which are determined
 *          in a seperate visibility update pass.
 ****************************************************************************/
struct PVRTCityBlockLod
{
	char               *pszFilename;
	PVRTuint32          numObjects;
	PVRTCityBlockEntity *paObjects;
	GLuint              vbos[2];
	unsigned int       *paVisibleNodes;
	unsigned int        numVisibleNodes;
	bool                bLoaded;
};

/*!**************************************************************************
 *	@Struct PVRTCityBlock
 *	@Brief  A city block is a container for all models located within a certain
 *          region of the city defined by a bounding box. There can be several
 *          levels of detail for each city block.
 ****************************************************************************/
struct PVRTCityBlock
{
	PVRTBoundingBox2D boundingbox;
	PVRTuint32        numLod;
	PVRTCityBlockLod *paLod;
};

/*!**************************************************************************
 *	@Struct PVRTOcclusionData
 *	@Brief  TODO
 ****************************************************************************/
struct PVRTOcclusionData
{
	PVRTVec3 position;
	unsigned int numRefObjects;
	unsigned int *pRefTile;
	unsigned int *pNumRefObject;
	unsigned int **ppRefObjects;
};

/*!**************************************************************************
 *	@Struct ShaderDescription
 *	@Brief  This structure describes a shader program and is just used to
 *          conveniently store each available shader.
 ****************************************************************************/
struct ShaderDescription
{
	const char  *pszVertShaderSrcFile;
	const char  *pszVertShaderBinFile;
	const char  *pszFragShaderSrcFile;
	const char  *pszFragShaderBinFile;
	const unsigned int ui32NumAttributes;
	const char  **pszAttributes;

	ShaderDescription(const char *pszVSSF, const char *pszVSBF, const char *pszFSSF, const char *pszFSBF, const unsigned int ui32NA, const char **pszA)
		: pszVertShaderSrcFile(pszVSSF), pszVertShaderBinFile(pszVSBF), pszFragShaderSrcFile(pszFSSF), pszFragShaderBinFile(pszFSBF), ui32NumAttributes(ui32NA), pszAttributes(pszA) {}
};


/******************************************************************************
  Functions
******************************************************************************/

static bool PointInBoundingBox(PVRTVec2 p, PVRTBoundingBox2D bbox)
{
	if (p.x < bbox.minCoords.x || p.y < bbox.minCoords.y || p.x > bbox.maxCoords.x || p.y > bbox.maxCoords.y)
		return false;
	else
		return true;
}

/******************************************************************************
 Content file names
******************************************************************************/

const char *g_pszAttributes[] = { "inVertex", "inNormal", "inTexCoord" };
const char *g_pszTexAttributes[] = { "inVertex", "inTexCoord" };
const char *g_pszShadowAttributes[] = { "inVertex", "inExtrude" };

const ShaderDescription CityEntityShaderDescription     ( "BuildingVertShader.vsh",   "BuildingVertShader.vsc",   "BuildingFragShader.fsh", "BuildingFragShader.fsc",  3, (const char**)g_pszAttributes );
const ShaderDescription SkyboxShaderDescription       ( "SkyboxVertShader.vsh",     "SkyboxVertShader.vsc",     "SkyboxFragShader.fsh",   "SkyboxFragShader.fsc",    1, (const char**)g_pszAttributes );
const ShaderDescription FullscreenShaderDescription   ( "FullscreenVertShader.vsh", "FullscreenVertShader.vsc", "FullscreenFragShader.fsh", "FullscreenFragShader.fsc", 2, (const char**)g_pszTexAttributes );
const ShaderDescription ShadowVolShaderDescription    ( "ShadowVolVertShader.vsh",  "ShadowVolVertShader.vsc",  "ShadowVolFragShader.fsh",   "ShadowVolFragShader.fsc", 2, (const char**)g_pszShadowAttributes );
const ShaderDescription UIShaderDescription           ( "UIVertShader.vsh", "UIVertShader.vsc", "UIFragShader.fsh", "UIFragShader.fsc", 2, (const char**)g_pszTexAttributes );

// Textures
const char c_szTextureNameSkybox[] = "Skybox.pvr";
const char c_szTextureNameUI[] = "UIElements.pvr";

// Scene files
const char c_szTrackFile[] = "cameratrack.pod";
const char c_szModelIndexFilename[] = "modelindex.nav";
const char c_szOcclusionDataFilename[] = "occlusiondata.nav";

// Texture files required for the city model
const char *c_paszTextures[] = { "006_RUS.PNG", "007_RUG.PNG", "008_RUG.PNG", "009_RUG.PNG", "011_GIE.PNG", "012_RSR.PNG", "016_FOC.PNG", "016_RTR.PNG", "017_FOD.PNG",
                                 "018_FOD.PNG", "019_FOC.PNG", "019_GOC.PNG", "019_RZG.PNG", "020_FOC.PNG", "021_FOC.PNG", "022_FOC.PNG", "022_RUG.PNG",
                                 "023_FOB.PNG", "023_RUG.PNG", "024_FOB.PNG", "025_FOC.PNG", "025_RUW.PNG", "026_FOD.PNG", "026_RUW.PNG", "027_FOD.PNG",
                                 "027_RUW.PNG", "028_GOF.PNG", "029_GCC.PNG", "030_GOC.PNG", "031_GOD.PNG", "032_FOC.PNG", "032_GOC.PNG", "033_FOA.PNG",
                                 "033_GOA.PNG", "034_FOC.PNG", "034_GOC.PNG", "035_FOC.PNG", "035_GOC.PNG", "036_FOC.PNG", "036_GOC.PNG", "037_FOC.PNG",
                                 "037_GOC.PNG", "041_FRB.PNG", "041_GRB.PNG", "044_GRC.PNG", "046_GRC.PNG", "055_GRC.PNG", "056_GRC.PNG", "060_FRC.PNG", "063_GRC.PNG",
                                 "064_GRC.PNG", "066_FCB.PNG", "066_GCB.PNG", "067_FCC.PNG", "067_GCC.PNG", "068_GCD.PNG", "069_FCA.PNG", "069_GCA.PNG",
                                 "070_GOD.PNG", "071_FRC.PNG", "072_FRC.PNG", "073_FRC.PNG", "074_FRC.PNG", "075_FRC.PNG", "076_FRC.PNG", "077_FRC.PNG",
                                 "080_GCB.PNG", "083_FRC.PNG", "085_GRC.PNG", "086_FOF.PNG", "086_GOF.PNG", "087_FCA.PNG", "087_GCA.PNG", "087_GCC.PNG",
								 "054_GRC.PNG", "059_FRC.PNG", "061_FRD.PNG", "082_FCD.PNG", "088_FRC.PNG", "089_FRC.PNG", "092_GCA.PNG", "094_FOD.PNG", "095_FOD.PNG",
								 "US_IL_CHICAGO_MMART_L.PNG", "US_IL_13443_CHICAGO_35EAST_L.PNG", "US_IL_13444_CHICAGO_LEOBURNETT_L.PNG",
                                 "US_IL_13447_CHICAGO_REIDMURDOCH_L.PNG", "US_IL_13448_CHICAGO_CARBIDE_L.PNG", "US_IL_13449_CHICAGO_CROWNFOUNTAIN_L.PNG",
                                 "US_IL_13451_CHICAGO_CULTURAL_L.PNG", "US_IL_13453_CHICAGO_PRUDENTIAL_PART1_L.PNG", "US_IL_13454_CHICAGO_UNITED_L.PNG",
								 "US_IL_13458_CHICAGO_SMURFIT_L.PNG", "US_IL_13459_CHICAGO_LASALLE_L.PNG", "US_IL_13461_CHICAGO_UNITRIN_L.PNG", "US_IL_13462_CHICAGO_WILLOUGHBY_L.PNG",
								 "US_IL_13490_CHICAGO_PRUDENTIAL_PART2_L.PNG", "US_IL_CHICAGO_AONCENTER_L.PNG", "US_IL_CHICAGO_ARTINSTITUTE_L.PNG", "US_IL_CHICAGO_BOARDOFTHETRADE_L.PNG",
								 "US_IL_CHICAGO_BOEINGBUILDING_L.PNG", "US_IL_CHICAGO_CHICAGOTHEATRE_L.PNG", "US_IL_CHICAGO_CITYHALL_L.PNG", "US_IL_CHICAGO_DALEY_L.PNG",
								 "US_IL_CHICAGO_HILTON_L.PNG", "US_IL_CHICAGO_JAMESTHOMPSON_L.PNG", "US_IL_CHICAGO_LIBRARY_L.PNG", "US_IL_CHICAGO_MILLENIUMPARK1_L.PNG",
								 "US_IL_CHICAGO_MILLENIUMPARK2_L.PNG", "US_IL_CHICAGO_OGILVIE_L.PNG", "US_IL_CHICAGO_SEARSTOWER_L.PNG", "US_L_CONCRETE-COLOUR.PNG",
								 "US_L_CONCRETE-DETAIL.PNG", "US_L_PARK-COLOUR.PNG", "US_L_WATER-COLOUR.PNG", "US_R_CONCRETE.PNG", "US_R_STREET-DASHED.PNG", "US_R_STREET-INNER-SHOULDER.PNG",
								 "US_R_STREET-LANE-FILLER.PNG", "US_R_STREET-SOLID.PNG", "US_R_STREET-UNMARKED.PNG", "US_R_WALKWAY-SOLID.PNG", "US_R_WALKWAY-UNMARKED.PNG", "US_T_RAILROAD.PNG",
                                 "US_R_HIGHWAY-SOLID.PNG", "US_IL_CHICAGO_UNIONSTATION_L.PNG", "US_IL_13460_CHICAGO_TRUMP_L.PNG", "US_IL_13456_CHICAGO_SEVENTEENTH_L.PNG" };

/*!****************************************************************************
 Class declarations
******************************************************************************/

/*!***********************************************************************
 *	@Class  OGLES3Navigation3D
 *	@Brief  Navigation demo main class.
 ************************************************************************/
class OGLES3Navigation3D : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D  m_Print3D;

	// Camera attributes
	float         m_fCameraAnimation;
	CPVRTModelPOD m_CameraPod;
	unsigned int  m_ActiveCameraTrack;

	bool          m_bRotate;
	float         m_fFOV;
	float         m_fAspectRatio;
	float         m_fNearClipPlane;
	float         m_fFarClipPlane;
	PVRTVec3      m_vCameraFrom;
	PVRTVec3      m_vCameraTo;
	PVRTVec3      m_vCameraUp;

	PVRTVec3      m_vLightDirection;

	// Viewing matrices
	PVRTMat4      m_mViewMatrix;
	PVRTMat4      m_mProjectionMatrix;
	PVRTMat4      m_mViewProjectionMatrix;
	PVRTMat4      m_mViewProjectionMatrixNonRotated;

	// 3D Models
	unsigned int        m_uiNumPVRTCityBlocks;
	PVRTCityBlock      *m_paPVRTCityBlocks;
	PVRTVisibilityInfo *m_pauiVisibleTiles;
	unsigned int        m_uiNumVisibleTiles;

	float       m_afLodDistances[2];
	float       m_afSquaredLodDistances[2];

	// Occlusion data
	PVRTOcclusionData *m_paPVRTOcclusionData;
	unsigned int       m_uiNumOcclusionData;

	// Skybox
	GLuint       m_uiSkyboxVBO;
	GLuint       m_uiTextureIdSkybox;

	// Textures
	unsigned int m_uiNumTextures;
	GLuint      *m_pauiTextureIds;

	// General options
	bool  m_abStates[NUM_STATES];

	// Mouse input
	bool     m_bMousePressed;
	unsigned long m_ulLastMouseClick;
	PVRTVec2 m_vMouseClickPos;
	PVRTVec2 m_vMousePrevPos;
	PVRTMat4 m_mMouseLookMatrix;
	PVRTMat4 m_mMouseLightMatrix;

	// Shadow volumes
	bool  m_bUpdateShadowData;
	PVRTShadowVolShadowMesh	**m_ppaShadowMesh;
	PVRTShadowVolShadowVol	**m_ppaShadowVol;
	float                   **m_ppaVolumeScale;
	unsigned int             *m_paNumShadowVols;

#ifdef ENABLE_UI
	PVRTBoundingBox2D m_Buttons[NUM_STATES];
	PVRTVec2          m_ButtonCoordinates[NUM_STATES][4];
	PVRTVec2          m_ButtonTexCoords[NUM_STATES][4];
	GLuint            m_uiTextureIdUI;
#endif

	// Shader objects
	struct Shader
	{
		GLuint uiId;
		GLuint uiVertexShaderId;
		GLuint uiFragmentShaderId;
	};

	struct SkyboxShader : Shader
	{
		GLuint uiModelViewProjMatrixLoc;
	}
	m_SkyboxShader;

	struct ModelShader : Shader
	{
		GLuint uiModelViewProjMatrixLoc;
		GLuint uiLightDirectionLoc;
	}
	m_CityEntityShader;

	struct ShadowVolShader : Shader
	{
		GLuint uiModelViewProjMatrixLoc;
		GLuint uiLightDirectionLoc;
		GLuint uiVolumeScaleLoc;
		GLuint uiColourLoc;
	}
	m_ShadowVolShader;

	struct FullscreenShader : Shader
	{
		GLuint uiColourLoc;
	}
	m_FullscreenShader;

	struct UIShader : Shader
	{
		GLuint uiRoationMatrixLoc;
		GLuint uiColourScaleLoc;
	}
	m_UIShader;

	unsigned int m_ui32InitState;
	unsigned int m_ui32TextureBase;
	unsigned int m_ui32BlockBase;
	
public:

	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	bool LoadAssets(bool &finished);
	bool LoadTextures(CPVRTString* pErrorStr);
	bool LoadShader(CPVRTString* pErrorStr, const ShaderDescription &descr, Shader &shader);
	bool LoadShaders(CPVRTString* pErrorStr);
	bool ReleaseShader(Shader &shader);

	void UpdateTimer();
	void HandleInput();
	void HandleMouseClick(PVRTVec2 pos);
	void HandleMouseDrag(PVRTVec2 dir);

	void Update3dModelWorkingset();

	void Render3dModelsVisibilitySet();
	void Render3dModelsOcclusion();
	void RenderSkyBox();

	void CreateModelVbo(const unsigned int tile, const unsigned int lod);
	void ReleaseModelVbo(const unsigned int tile, const unsigned int lod);

	void CalculateLightMatrices();
	void CalculateCameraMatrices();
	void GetCameraFrame(PVRTVec3 &from, PVRTVec3 &to, PVRTVec3 &up, float time);

	void ExtractViewFrustumPlanes(const PVRTMat4 &matrix, PVRTVec4 &left, PVRTVec4 &right, PVRTVec4 &front, PVRTVec4 &back) const;
	int BoundingBoxIntersectsFrustum(const PVRTBoundingBox2D &bbox, const PVRTVec4 planes[4]) const;

	bool Load3dModelIndex(const char *pszFilename, CPVRTString* pErrorStr);
	void Release3dModelIndex();

	bool LoadOcclusionData(const char *pszFilename, CPVRTString* pErrorStr);
	void ReleaseOcclusionData();

	void CreateShadowVolumes(unsigned int tile);
	void ReleaseShadowVolumes();
	void UpdateShadowVolumes();
	void RenderShadowVolumes();

#ifdef ENABLE_UI
	void RenderUI();
	void InitUI();
#endif
};

/*!****************************************************************************
 @Function		LoadAssets
 @Output		finished		True if loading has finished, false otherwise.
 @Return		bool			true if no error occures
 @Description	Loads the assets required for the demo and display progress
                messages.
******************************************************************************/
bool OGLES3Navigation3D::LoadAssets(bool &finished)
{
	finished = false;

	// Print the message that we will load the textures in the next iteration
	if (m_ui32InitState == 0)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		m_Print3D.DisplayDefaultTitle("3D Navigation", NULL, ePVRTPrint3DSDKLogo);
		m_Print3D.Print3D(35.0f, 45.0f, 1.0f, 0xFFFFFFFF, "PLEASE WAIT");
		m_Print3D.Print3D(35.0f, 55.0f, 0.5f, 0xAAFFFFFF, "Loading textures ...");
		m_Print3D.Flush();
		++m_ui32InitState;
		return true;
	}
	// Print the message that we will load the models in the next iteration and
	// actually load the textures now
	else if (m_ui32InitState == 1)
	{
		static unsigned int numTexturesIteration = 5;

		// Load all 3d model textures
		char buffer[512];
		for (unsigned int i=0; i < numTexturesIteration; ++i)
		{
			// Loaded all textures
			if (m_ui32TextureBase >= m_uiNumTextures)
			{
				++m_ui32InitState;
				break;
			}
			strcpy(buffer, c_paszTextures[m_ui32TextureBase]);
			strcat(buffer, ".pvr");

			if (PVR_SUCCESS == PVRTTextureLoadFromPVR(buffer, &m_pauiTextureIds[m_ui32TextureBase]))
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}
			else
			{
				PVRShellOutputDebug("Failed to load texture: %s\n", buffer);
			}

			++m_ui32TextureBase;
		}

		glClear(GL_COLOR_BUFFER_BIT);
		m_Print3D.DisplayDefaultTitle("3D Navigation", NULL, ePVRTPrint3DSDKLogo);
		m_Print3D.Print3D(35.0f, 45.0f, 1.0f, 0xFFFFFFFF, "PLEASE WAIT");
		m_Print3D.Print3D(35.0f, 55.0f, 0.5f, 0xAAFFFFFF, "Loading textures ... (%d / %d)", m_ui32TextureBase, m_uiNumTextures);
		m_Print3D.Flush();
		return true;
	}
	// Load the models and indicate that we are finished afterwards
	else if (m_ui32InitState == 2)
	{
		if (m_ui32BlockBase >= m_uiNumPVRTCityBlocks)
		{
			UpdateShadowVolumes();
			m_ui32InitState++;
		}
		else
		{
			for (unsigned int j=0; j < m_paPVRTCityBlocks[m_ui32BlockBase].numLod; ++j)
			{
				CreateModelVbo(m_ui32BlockBase, j);
			}

			CreateShadowVolumes(m_ui32BlockBase);

			++m_ui32BlockBase;

			glClear(GL_COLOR_BUFFER_BIT);
			m_Print3D.DisplayDefaultTitle("3D Navigation", NULL, ePVRTPrint3DSDKLogo);
			m_Print3D.Print3D(35.0f, 45.0f, 1.0f, 0xFFFFFFFF, "PLEASE WAIT");
			m_Print3D.Print3D(35.0f, 55.0f, 0.5f, 0xAAFFFFFF, "Loading textures ... done");
			m_Print3D.Print3D(35.0f, 59.0f, 0.5f, 0xAAFFFFFF, "Loading models ... (%d / %d)", m_ui32BlockBase, m_uiNumPVRTCityBlocks);
			m_Print3D.Flush();
			return true;
		}
	}

	finished = true;
	return true;
}

/*!****************************************************************************
 @Function		LoadTextures
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course
******************************************************************************/
bool OGLES3Navigation3D::LoadTextures(CPVRTString* const pErrorStr)
{
	if (PVRTTextureLoadFromPVR(c_szTextureNameSkybox, &m_uiTextureIdSkybox) != PVR_SUCCESS)
	{
		*pErrorStr = CPVRTString("ERROR: Could not open texture file ") + c_szTextureNameSkybox;
		return false;
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#ifdef ENABLE_UI
	if (PVRTTextureLoadFromPVR(c_szTextureNameUI, &m_uiTextureIdUI) != PVR_SUCCESS)
	{
		*pErrorStr = CPVRTString("ERROR: Could not open texture file ") + c_szTextureNameUI;
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif // ENABLE_UI

	return true;
}


/*!****************************************************************************
 @Function		LoadShader
 @Output		pErrorStr		A string describing the error on failure
 @Input 		descr		    A struct containing all necessary information to
                                load and build the shader
 @Output		shader		    A struct containing all shader object identifiers
 @Return		bool			true if no error occured
 @Description	Loads and compiles a shader and links it into a shader program
******************************************************************************/
bool OGLES3Navigation3D::LoadShader(CPVRTString* pErrorStr, const ShaderDescription &descr, Shader &shader)
{
	if (PVRTShaderLoadFromFile(descr.pszVertShaderBinFile, descr.pszVertShaderSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &shader.uiVertexShaderId, pErrorStr) != PVR_SUCCESS)
	{
		*pErrorStr = descr.pszVertShaderSrcFile + CPVRTString(":\n") + *pErrorStr;
		return false;
	}

	if (PVRTShaderLoadFromFile(descr.pszFragShaderBinFile, descr.pszFragShaderSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &shader.uiFragmentShaderId, pErrorStr) != PVR_SUCCESS)
	{
		*pErrorStr = descr.pszFragShaderSrcFile + CPVRTString(":\n") + *pErrorStr;
		return false;
	}

	// Set up and link to the shader program
	if (PVRTCreateProgram(&shader.uiId, shader.uiVertexShaderId, shader.uiFragmentShaderId, (const char**)descr.pszAttributes, descr.ui32NumAttributes, pErrorStr))
	{
		*pErrorStr = descr.pszFragShaderSrcFile + CPVRTString(":\n") + *pErrorStr;
		return false;
	}

	return true;
}

/*!****************************************************************************
 @Function		LoadShaders
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads and compiles shaders and links them to shader programs
******************************************************************************/
bool OGLES3Navigation3D::LoadShaders(CPVRTString* pErrorStr)
{
	//	Load and compile the shaders from files.
	//	Binary shaders are tried first, source shaders are used as fallback.

	// CityEntityShader
	//
	if (!LoadShader(pErrorStr, CityEntityShaderDescription, m_CityEntityShader))
	{
		PVRShellOutputDebug("Shader compilation failed: CityEntityShader\n");
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	m_CityEntityShader.uiModelViewProjMatrixLoc = glGetUniformLocation(m_CityEntityShader.uiId, "ModelViewProjMatrix");
	m_CityEntityShader.uiLightDirectionLoc      = glGetUniformLocation(m_CityEntityShader.uiId, "LightDirection");
	glUniform1i(glGetUniformLocation(m_CityEntityShader.uiId, "sTexture"), 0);

	//  SkyboxShader
	//
	if (!LoadShader(pErrorStr, SkyboxShaderDescription, m_SkyboxShader))
	{
		PVRShellOutputDebug("Shader compilation failed: SkyBoxShader\n");
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	m_SkyboxShader.uiModelViewProjMatrixLoc = glGetUniformLocation(m_SkyboxShader.uiId, "ModelViewProjMatrix");
	glUniform1i(glGetUniformLocation(m_SkyboxShader.uiId, "sCubeMap"), 0);


	// ShadowVolShader
	//
	if (!LoadShader(pErrorStr, ShadowVolShaderDescription, m_ShadowVolShader))
	{
		PVRShellOutputDebug("Shader compilation failed: ShadowVolShader\n");
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	m_ShadowVolShader.uiModelViewProjMatrixLoc = glGetUniformLocation(m_ShadowVolShader.uiId, "ModelViewProjMatrix");
	m_ShadowVolShader.uiLightDirectionLoc      = glGetUniformLocation(m_ShadowVolShader.uiId, "LightDirection");
	m_ShadowVolShader.uiVolumeScaleLoc         = glGetUniformLocation(m_ShadowVolShader.uiId, "VolumeScale");
	m_ShadowVolShader.uiColourLoc              = glGetUniformLocation(m_ShadowVolShader.uiId, "FlatColour");

	// FullscreenShader
	//
	if (!LoadShader(pErrorStr, FullscreenShaderDescription, m_FullscreenShader))
	{
		PVRShellOutputDebug("Shader compilation failed: FullscreenShader\n");
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	m_FullscreenShader.uiColourLoc  = glGetUniformLocation(m_FullscreenShader.uiId, "FlatColour");
	glUniform1i(glGetUniformLocation(m_FullscreenShader.uiId, "sTexture"), 0);

#ifdef ENABLE_UI
	// UIShader
	//
	if (!LoadShader(pErrorStr, UIShaderDescription, m_UIShader))
	{
		PVRShellOutputDebug("Shader compilation failed: UIShader\n");
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	m_UIShader.uiRoationMatrixLoc = glGetUniformLocation(m_UIShader.uiId, "RotationMatrix");
	m_UIShader.uiColourScaleLoc = glGetUniformLocation(m_UIShader.uiId, "ColourScale");
	glUniform1i(glGetUniformLocation(m_UIShader.uiId, "sTexture"), 0);
#endif // ENABLE_UI

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
bool OGLES3Navigation3D::InitApplication()
{
	m_paPVRTCityBlocks = 0;
	m_pauiVisibleTiles = 0;
	m_paPVRTOcclusionData = 0;
	m_pauiTextureIds = 0;

	m_abStates[STATE_PAUSE] = false;
	m_abStates[STATE_SHADOW] = true;
	m_abStates[STATE_DEBUG] = false;
	m_abStates[STATE_CULLING] = true;
	m_abStates[STATE_OCCLUSION] = true;
	m_abStates[STATE_INPUT_MODE] = true;
	m_abStates[STATE_UI] = false;

	m_ppaShadowMesh = 0;
	m_ppaShadowVol = 0;
	m_ppaVolumeScale = 0;

	m_uiNumTextures = 0;
	m_uiNumPVRTCityBlocks = 0;
	m_uiNumVisibleTiles = 0;
	m_uiNumOcclusionData = 0;

	m_uiNumTextures = sizeof(c_paszTextures)/sizeof(c_paszTextures[0]);
	m_pauiTextureIds = new GLuint[m_uiNumTextures];

	m_vMousePrevPos = PVRTVec2(-1.0f);
	m_mMouseLookMatrix = PVRTMat4::Identity();
	m_mMouseLightMatrix = PVRTMat4::Identity();

	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	CPVRTString errorStr;
	if (!Load3dModelIndex(c_szModelIndexFilename, &errorStr))
	{
		PVRShellOutputDebug(errorStr.c_str());
		return false;
	}

	if (!LoadOcclusionData(c_szOcclusionDataFilename, &errorStr))
	{
		PVRShellOutputDebug(errorStr.c_str());
		return false;
	}

	m_pauiVisibleTiles = new PVRTVisibilityInfo[m_uiNumPVRTCityBlocks];

	if (PVR_SUCCESS != m_CameraPod.ReadFromFile("cameratrack.pod"))
	{
		PVRShellSet(prefExitMessage, "Error: Failed to parse POD cameratrack.\n");
		return false;
	}

	m_ActiveCameraTrack = 0;
	m_fNearClipPlane = m_CameraPod.pCamera->fNear;
	m_fFarClipPlane = m_CameraPod.pCamera->fFar;
	m_fFOV = m_CameraPod.pCamera->fFOV;

	const int cmdargs = PVRShellGet(prefCommandLineOptNum);
	const SCmdLineOpt *pCmdLine = (const SCmdLineOpt *)PVRShellGet(prefCommandLineOpts);
	for (int i=0; i < cmdargs; i++)
	{
		if (strcmp(pCmdLine[i].pArg, "-far") == 0)
		{
			PVRShellOutputDebug("Info: Changing far clip plane from %.0f to %s\n", m_fFarClipPlane, pCmdLine[i].pVal);
			m_fFarClipPlane = (float)atoi(pCmdLine[i].pVal);
		}
		if (strcmp(pCmdLine[i].pArg, "-near") == 0)
		{
			PVRShellOutputDebug("Info: Changing near clip plane from %.0f to %s\n", m_fNearClipPlane, pCmdLine[i].pVal);
			m_fNearClipPlane = (float)atoi(pCmdLine[i].pVal);
		}
	}

	m_afLodDistances[0] = (m_CameraPod.pCamera->fFar + m_CameraPod.pCamera->fNear) * 0.5f;
	m_afLodDistances[1] = m_CameraPod.pCamera->fFar;
	m_afSquaredLodDistances[0] = m_afLodDistances[0] * m_afLodDistances[0];
	m_afSquaredLodDistances[1] = m_afLodDistances[1] * m_afLodDistances[1];

	m_vLightDirection = PVRTVec3(0.1f, 0.1f, -1.0f).normalized();

	// Set timer variables
	m_fCameraAnimation = 0.0f;

	m_ppaShadowMesh = new PVRTShadowVolShadowMesh*[m_uiNumPVRTCityBlocks];
	m_ppaShadowVol = new PVRTShadowVolShadowVol*[m_uiNumPVRTCityBlocks];
	m_ppaVolumeScale = new float*[m_uiNumPVRTCityBlocks];
	m_paNumShadowVols = new unsigned int[m_uiNumPVRTCityBlocks];
	m_bUpdateShadowData = false;

	for (unsigned int i=0; i < m_uiNumPVRTCityBlocks; i++)
	{
		m_ppaShadowMesh[i] = 0;
		m_ppaShadowVol[i] = 0;
		m_ppaVolumeScale[i] = 0;
		m_paNumShadowVols[i] = 0;
	}

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
bool OGLES3Navigation3D::QuitApplication()
{
	Release3dModelIndex();
	ReleaseOcclusionData();

	delete [] m_pauiTextureIds;
	delete [] m_pauiVisibleTiles;

	ReleaseShadowVolumes();

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
bool OGLES3Navigation3D::InitView()
{
	m_fAspectRatio = PVRShellGet(prefWidth) / (float)PVRShellGet(prefHeight);

	CPVRTString ErrorStr;

	// Load and compile the shaders & link programs
	if (!LoadShaders(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Load textures
	if (!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Is the screen rotated?
	m_bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	// Initialize Print3D
	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), m_bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	// Generate the skybox VBO
	GLfloat *pSkyboxVertices, *pSkyboxTexCoords;
	PVRTCreateSkybox(10.0f, true, 512, &pSkyboxVertices, &pSkyboxTexCoords);
	glGenBuffers(1, &m_uiSkyboxVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_uiSkyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*3*24, pSkyboxVertices, GL_STATIC_DRAW);
	PVRTDestroySkybox(pSkyboxVertices, pSkyboxTexCoords);

#ifdef ENABLE_UI
	InitUI();
#endif

	// No stencil test required
	glDisable(GL_STENCIL_TEST);

	// Disable blending by default
	glDisable(GL_BLEND);

	// Paint it black
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Texture channel 0 will always be active
	glActiveTexture(GL_TEXTURE0);

	// Setup the viewport for the whole window
	glViewport(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	m_ui32InitState = 0;
	m_ui32TextureBase = 0;
	m_ui32BlockBase = 0;
	return true;
}

/*!****************************************************************************
 @Function		ReleaseShader
 @Return		bool		true if no error occured
 @Description	Releases a shader program including the individual shader objects.
******************************************************************************/
bool OGLES3Navigation3D::ReleaseShader(Shader &shader)
{
	glDeleteProgram(shader.uiId);
	glDeleteShader(shader.uiVertexShaderId);
	glDeleteShader(shader.uiFragmentShaderId);
	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES3Navigation3D::ReleaseView()
{
	// Delete textures
	glDeleteTextures(1, &m_uiTextureIdSkybox);
	glDeleteTextures(m_uiNumTextures, m_pauiTextureIds);

	// Delete program and shader objects
	ReleaseShader(m_CityEntityShader);
	ReleaseShader(m_SkyboxShader);
	ReleaseShader(m_ShadowVolShader);
	ReleaseShader(m_FullscreenShader);

#ifdef ENABLE_UI
	ReleaseShader(m_UIShader);
	glDeleteTextures(1, &m_uiTextureIdUI);
#endif

	glDeleteBuffers(1, &m_uiSkyboxVBO);

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
				The user has access to these events through an abstraction
				layer provided by PVRShell.
******************************************************************************/
bool OGLES3Navigation3D::RenderScene()
{
	//PVRShellOutputDebug("Nav3D: RenderScene() enter\n");
	// Return early as long as we are not finished loading the assets
	bool loading_finished;
	if (!LoadAssets(loading_finished))
		return false;
	if (!loading_finished)
		return true;

	// Handle user input and update the timer based variables
	HandleInput();
	UpdateTimer();

	// Update the camera interpolation and extract required matrices
	CalculateCameraMatrices();
	CalculateLightMatrices();

	// Clear the colour, depth and stencil buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Disable depth test to render the skybox
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	RenderSkyBox();

	// Enable depth test and render city blocks
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	if (m_abStates[STATE_OCCLUSION])
	{
		// Render using occlusion information
		Render3dModelsOcclusion();
	}
	else
	{
		// Render using visibility information
		Render3dModelsVisibilitySet();
	}

	// Render shadows
	if (m_abStates[STATE_SHADOW])
	{
		// if the lightsource moved recreate the shadow volumes
		if (m_bUpdateShadowData)
		{
			UpdateShadowVolumes();
			m_bUpdateShadowData = false;
		}
		// finally render them
		RenderShadowVolumes();
	}

#ifdef ENABLE_UI
	// Render UI on top of the 3D scene
	RenderUI();
#endif

	// Displays the demo name and other information using the Print3D tool.
	// For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("3D Navigation", NULL, ePVRTPrint3DSDKLogo);
	#if defined(ENABLE_ADVANCED_OUTPUT) && !defined(ENABLE_UI)
	m_Print3D.Print3D(0.5f, 94.0f, 0.5f, 0xFFFFFFFF, "Shadows %s  %s", (m_abStates[STATE_SHADOW] ? "enabled" : "disabled"), (m_abStates[STATE_DEBUG] ? "(debug)" : ""));
	m_Print3D.Print3D(0.5f, 97.0f, 0.5f, 0xFFFFFFFF, "Culling - Occlusion: %s  Frustum: %s", (m_abState[STATE_OCCLUSION] ? "On" : "Off"), (m_abStates[STATE_CULLING] ? "On" : "Off"));
    #endif
	m_Print3D.Flush();

	return true;
}

#ifdef ENABLE_UI
/*!****************************************************************************
 @Function		RenderUI
 @Description	Renders an alpha-blended UI consisting of simple buttons.
******************************************************************************/
void OGLES3Navigation3D::RenderUI()
{
	if (m_abStates[STATE_UI])
	{
		PVRTMat3 rotMatrix = PVRTMat3::Identity();
		if (m_bRotate)
			rotMatrix = PVRTMat3::Rotation2D(-PVRT_PI_OVER_TWO);
		float matrix[4] = { rotMatrix.f[0], rotMatrix.f[1], rotMatrix.f[3], rotMatrix.f[4] };

		unsigned short indices[6] = { 0, 1, 2, 0, 2, 3 };

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glUseProgram(m_UIShader.uiId);
		glUniformMatrix2fv(m_UIShader.uiRoationMatrixLoc, 1, false, matrix);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_uiTextureIdUI);

		glEnableVertexAttribArray(VERTEX_ARRAY_UI);
		glEnableVertexAttribArray(TEXCOORD_ARRAY_UI);

		for (unsigned int i=0; i < NUM_STATES; i++)
		{
			switch(i)
			{
			case STATE_PAUSE:
			case STATE_SHADOW:
			case STATE_DEBUG:
			case STATE_CULLING:
			case STATE_OCCLUSION:
				{
					glVertexAttribPointer(VERTEX_ARRAY_UI, 2, GL_FLOAT, GL_FALSE, sizeof(PVRTVec2), m_ButtonCoordinates[i][0].ptr());
					glVertexAttribPointer(TEXCOORD_ARRAY_UI, 2, GL_FLOAT, GL_FALSE, sizeof(PVRTVec2), m_ButtonTexCoords[i][0].ptr());
					if (m_abStates[i])
						glUniform4f(m_UIShader.uiColourScaleLoc, 1.0f, 1.0f, 1.0f, 1.0f);
					else
						glUniform4f(m_UIShader.uiColourScaleLoc, 0.8f, 0.8f, 0.8f, 0.4f);
					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
					break;
				}
			case STATE_INPUT_MODE:
				{
					if (m_abStates[STATE_PAUSE])
					{
						glUniform4f(m_UIShader.uiColourScaleLoc, 1.0f, 1.0f, 1.0f, 1.0f);

						glVertexAttribPointer(VERTEX_ARRAY_UI, 2, GL_FLOAT, GL_FALSE, sizeof(PVRTVec2), m_ButtonCoordinates[i][0].ptr());
						if (m_abStates[STATE_INPUT_MODE])
							glVertexAttribPointer(TEXCOORD_ARRAY_UI, 2, GL_FLOAT, GL_FALSE, sizeof(PVRTVec2), m_ButtonTexCoords[i][0].ptr());
						else
							glVertexAttribPointer(TEXCOORD_ARRAY_UI, 2, GL_FLOAT, GL_FALSE, sizeof(PVRTVec2), m_ButtonTexCoords[i+1][0].ptr());
						glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
					}
					break;
				}
			default:
				break;
			}
		}

		glDisableVertexAttribArray(VERTEX_ARRAY_UI);
		glDisableVertexAttribArray(TEXCOORD_ARRAY_UI);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

/*!****************************************************************************
 @Function		InitUI
 @Description	Sets up all button locations and rendering attributes.
******************************************************************************/
void OGLES3Navigation3D::InitUI()
{
	PVRTVec2 offset(-1.0f, -1.0f);
	PVRTVec2 buttonsize(0.3f, 0.2f);

	// Pause button - upper right corner
	m_Buttons[STATE_PAUSE].minCoords = PVRTVec2(0.7f, 0.2f);
	m_Buttons[STATE_PAUSE].maxCoords = m_Buttons[STATE_PAUSE].minCoords + buttonsize;
	m_ButtonCoordinates[STATE_PAUSE][0] = PVRTVec2(m_Buttons[STATE_PAUSE].minCoords.x, 1.0f - m_Buttons[STATE_PAUSE].minCoords.y) * 2.0f + offset;
	m_ButtonCoordinates[STATE_PAUSE][1] = PVRTVec2(m_Buttons[STATE_PAUSE].maxCoords.x, 1.0f - m_Buttons[STATE_PAUSE].minCoords.y) * 2.0f + offset;
	m_ButtonCoordinates[STATE_PAUSE][2] = PVRTVec2(m_Buttons[STATE_PAUSE].maxCoords.x, 1.0f - m_Buttons[STATE_PAUSE].maxCoords.y) * 2.0f + offset;
	m_ButtonCoordinates[STATE_PAUSE][3] = PVRTVec2(m_Buttons[STATE_PAUSE].minCoords.x, 1.0f - m_Buttons[STATE_PAUSE].maxCoords.y) * 2.0f + offset;
	m_ButtonTexCoords[STATE_PAUSE][0] = PVRTVec2(0.0f, 0.25f);
	m_ButtonTexCoords[STATE_PAUSE][1] = PVRTVec2(0.5f, 0.25f);
	m_ButtonTexCoords[STATE_PAUSE][2] = PVRTVec2(0.5f, 0.0f);
	m_ButtonTexCoords[STATE_PAUSE][3] = PVRTVec2(0.0f, 0.0f);

	// Input button - middle right
	m_Buttons[STATE_INPUT_MODE].minCoords = PVRTVec2(0.7f, 0.6f);
	m_Buttons[STATE_INPUT_MODE].maxCoords = m_Buttons[STATE_INPUT_MODE].minCoords + buttonsize;
	m_ButtonCoordinates[STATE_INPUT_MODE][0] = PVRTVec2(m_Buttons[STATE_INPUT_MODE].minCoords.x, 1.0f - m_Buttons[STATE_INPUT_MODE].minCoords.y) * 2.0f + offset;
	m_ButtonCoordinates[STATE_INPUT_MODE][1] = PVRTVec2(m_Buttons[STATE_INPUT_MODE].maxCoords.x, 1.0f - m_Buttons[STATE_INPUT_MODE].minCoords.y) * 2.0f + offset;
	m_ButtonCoordinates[STATE_INPUT_MODE][2] = PVRTVec2(m_Buttons[STATE_INPUT_MODE].maxCoords.x, 1.0f - m_Buttons[STATE_INPUT_MODE].maxCoords.y) * 2.0f + offset;
	m_ButtonCoordinates[STATE_INPUT_MODE][3] = PVRTVec2(m_Buttons[STATE_INPUT_MODE].minCoords.x, 1.0f - m_Buttons[STATE_INPUT_MODE].maxCoords.y) * 2.0f + offset;
	m_ButtonTexCoords[STATE_INPUT_MODE][0] = PVRTVec2(0.5f, 1.0f);
	m_ButtonTexCoords[STATE_INPUT_MODE][1] = PVRTVec2(1.0f, 1.0f);
	m_ButtonTexCoords[STATE_INPUT_MODE][2] = PVRTVec2(1.0f, 0.75f);
	m_ButtonTexCoords[STATE_INPUT_MODE][3] = PVRTVec2(0.5f, 0.75f);
	m_ButtonTexCoords[STATE_INPUT_MODE+1][0] = PVRTVec2(0.5f, 0.75f);
	m_ButtonTexCoords[STATE_INPUT_MODE+1][1] = PVRTVec2(1.0f, 0.75f);
	m_ButtonTexCoords[STATE_INPUT_MODE+1][2] = PVRTVec2(1.0f, 0.5f);
	m_ButtonTexCoords[STATE_INPUT_MODE+1][3] = PVRTVec2(0.5f, 0.5f);

	// Shadow button - upper left corner
	m_Buttons[STATE_SHADOW].minCoords = PVRTVec2(0.0f, 0.1f);
	m_Buttons[STATE_SHADOW].maxCoords = m_Buttons[STATE_SHADOW].minCoords + buttonsize;
	m_ButtonCoordinates[STATE_SHADOW][0] = PVRTVec2(m_Buttons[STATE_SHADOW].minCoords.x, 1.0f - m_Buttons[STATE_SHADOW].minCoords.y) * 2.0f + offset;
	m_ButtonCoordinates[STATE_SHADOW][1] = PVRTVec2(m_Buttons[STATE_SHADOW].maxCoords.x, 1.0f - m_Buttons[STATE_SHADOW].minCoords.y) * 2.0f + offset;
	m_ButtonCoordinates[STATE_SHADOW][2] = PVRTVec2(m_Buttons[STATE_SHADOW].maxCoords.x, 1.0f - m_Buttons[STATE_SHADOW].maxCoords.y) * 2.0f + offset;
	m_ButtonCoordinates[STATE_SHADOW][3] = PVRTVec2(m_Buttons[STATE_SHADOW].minCoords.x, 1.0f - m_Buttons[STATE_SHADOW].maxCoords.y) * 2.0f + offset;
	m_ButtonTexCoords[STATE_SHADOW][0] = PVRTVec2(0.0f, 0.75f);
	m_ButtonTexCoords[STATE_SHADOW][1] = PVRTVec2(0.5f, 0.75f);
	m_ButtonTexCoords[STATE_SHADOW][2] = PVRTVec2(0.5f, 0.5f);
	m_ButtonTexCoords[STATE_SHADOW][3] = PVRTVec2(0.0f, 0.5f);

	// Debug button - lower left corner
	m_Buttons[STATE_DEBUG].minCoords = PVRTVec2(0.0f, 0.7f);
	m_Buttons[STATE_DEBUG].maxCoords = m_Buttons[STATE_DEBUG].minCoords + buttonsize;
	m_ButtonCoordinates[STATE_DEBUG][0] = PVRTVec2(m_Buttons[STATE_DEBUG].minCoords.x, 1.0f - m_Buttons[STATE_DEBUG].minCoords.y) * 2.0f + offset;
	m_ButtonCoordinates[STATE_DEBUG][1] = PVRTVec2(m_Buttons[STATE_DEBUG].maxCoords.x, 1.0f - m_Buttons[STATE_DEBUG].minCoords.y) * 2.0f + offset;
	m_ButtonCoordinates[STATE_DEBUG][2] = PVRTVec2(m_Buttons[STATE_DEBUG].maxCoords.x, 1.0f - m_Buttons[STATE_DEBUG].maxCoords.y) * 2.0f + offset;
	m_ButtonCoordinates[STATE_DEBUG][3] = PVRTVec2(m_Buttons[STATE_DEBUG].minCoords.x, 1.0f - m_Buttons[STATE_DEBUG].maxCoords.y) * 2.0f + offset;
	m_ButtonTexCoords[STATE_DEBUG][0] = PVRTVec2(0.5f, 0.5f);
	m_ButtonTexCoords[STATE_DEBUG][1] = PVRTVec2(1.0f, 0.5f);
	m_ButtonTexCoords[STATE_DEBUG][2] = PVRTVec2(1.0f, 0.25f);
	m_ButtonTexCoords[STATE_DEBUG][3] = PVRTVec2(0.5f, 0.25f);

	// Culling button - upper middle left corner
	m_Buttons[STATE_CULLING].minCoords = PVRTVec2(0.0f, 0.3f);
	m_Buttons[STATE_CULLING].maxCoords = m_Buttons[STATE_CULLING].minCoords + buttonsize;
	m_ButtonCoordinates[STATE_CULLING][0] = PVRTVec2(m_Buttons[STATE_CULLING].minCoords.x, 1.0f - m_Buttons[STATE_CULLING].minCoords.y) * 2.0f + offset;
	m_ButtonCoordinates[STATE_CULLING][1] = PVRTVec2(m_Buttons[STATE_CULLING].maxCoords.x, 1.0f - m_Buttons[STATE_CULLING].minCoords.y) * 2.0f + offset;
	m_ButtonCoordinates[STATE_CULLING][2] = PVRTVec2(m_Buttons[STATE_CULLING].maxCoords.x, 1.0f - m_Buttons[STATE_CULLING].maxCoords.y) * 2.0f + offset;
	m_ButtonCoordinates[STATE_CULLING][3] = PVRTVec2(m_Buttons[STATE_CULLING].minCoords.x, 1.0f - m_Buttons[STATE_CULLING].maxCoords.y) * 2.0f + offset;
	m_ButtonTexCoords[STATE_CULLING][0] = PVRTVec2(0.0f, 1.0f);
	m_ButtonTexCoords[STATE_CULLING][1] = PVRTVec2(0.5f, 1.0f);
	m_ButtonTexCoords[STATE_CULLING][2] = PVRTVec2(0.5f, 0.75f);
	m_ButtonTexCoords[STATE_CULLING][3] = PVRTVec2(0.0f, 0.75f);

	// Occlusion button - lower middle left corner
	m_Buttons[STATE_OCCLUSION].minCoords = PVRTVec2(0.0f, 0.5f);
	m_Buttons[STATE_OCCLUSION].maxCoords = m_Buttons[STATE_OCCLUSION].minCoords + buttonsize;
	m_ButtonCoordinates[STATE_OCCLUSION][0] = PVRTVec2(m_Buttons[STATE_OCCLUSION].minCoords.x, 1.0f - m_Buttons[STATE_OCCLUSION].minCoords.y) * 2.0f + offset;
	m_ButtonCoordinates[STATE_OCCLUSION][1] = PVRTVec2(m_Buttons[STATE_OCCLUSION].maxCoords.x, 1.0f - m_Buttons[STATE_OCCLUSION].minCoords.y) * 2.0f + offset;
	m_ButtonCoordinates[STATE_OCCLUSION][2] = PVRTVec2(m_Buttons[STATE_OCCLUSION].maxCoords.x, 1.0f - m_Buttons[STATE_OCCLUSION].maxCoords.y) * 2.0f + offset;
	m_ButtonCoordinates[STATE_OCCLUSION][3] = PVRTVec2(m_Buttons[STATE_OCCLUSION].minCoords.x, 1.0f - m_Buttons[STATE_OCCLUSION].maxCoords.y) * 2.0f + offset;
	m_ButtonTexCoords[STATE_OCCLUSION][0] = PVRTVec2(0.5f, 0.25f);
	m_ButtonTexCoords[STATE_OCCLUSION][1] = PVRTVec2(1.0f, 0.25f);
	m_ButtonTexCoords[STATE_OCCLUSION][2] = PVRTVec2(1.0f, 0.0f);
	m_ButtonTexCoords[STATE_OCCLUSION][3] = PVRTVec2(0.5f, 0.0f);

	// Middle column
	m_Buttons[STATE_UI].minCoords = PVRTVec2(0.3f, 0.0f);
	m_Buttons[STATE_UI].maxCoords = PVRTVec2(0.7f, 1.0f);
}
#endif // ENABLE_UI


/*!****************************************************************************
 @Function		UpdateTimer
 @Input			none
 @Description	Updates the values of the current time, previous time, current time
				in seconds, delta time and the FPS counter used in the program
******************************************************************************/
void OGLES3Navigation3D::UpdateTimer()
{
	static unsigned long ulPreviousTime = PVRShellGetTime();
	static unsigned long ulLastUpdate = PVRShellGetTime();

	unsigned long ulCurrentTime = PVRShellGetTime();
	unsigned long ulTimeDelta = ulCurrentTime - ulPreviousTime;
	ulPreviousTime = ulCurrentTime;

	// Update the visible object four times per second
	if (ulCurrentTime - ulLastUpdate > 100)
	{
		ulLastUpdate = ulCurrentTime;
		if (!m_abStates[STATE_OCCLUSION])
			Update3dModelWorkingset();
	}

	// Advance camera animation when not paused
	if (!m_abStates[STATE_PAUSE])
	{
		m_fCameraAnimation += ulTimeDelta * c_CameraMovementSpeedScale;
		// Start from beginning when the end is near
		if (m_fCameraAnimation > (m_CameraPod.nNumFrame - 1))
		{
			m_fCameraAnimation = 0.0f;

			// Jump to next camera track (if there are any)
			++m_ActiveCameraTrack;
			m_ActiveCameraTrack = m_ActiveCameraTrack % m_CameraPod.nNumCamera;
		}
	}
}

/*!****************************************************************************
 @Function		Update3dModelWorkingset
 @Description	Updates the visible object set based on the camera view frustum.
******************************************************************************/
void OGLES3Navigation3D::Update3dModelWorkingset()
{
	PVRTVec4 planes[4];
	ExtractViewFrustumPlanes(m_mViewProjectionMatrixNonRotated, planes[0], planes[1], planes[2], planes[3]);

	const PVRTVec2 lodCenter(m_vCameraFrom);

	// Start from scratch and assume that we don't see anything
	m_uiNumVisibleTiles = 0;

	// Update the object set for each layer
	for (unsigned int i=0; i < m_uiNumPVRTCityBlocks; i++)
	{
		const PVRTBoundingBox2D bbox = m_paPVRTCityBlocks[i].boundingbox;
		int culltest = BoundingBoxIntersectsFrustum(bbox, planes);

		// If the tile intersects the viewfrustum:
		if (!(INTERSECT_NONE == culltest))
		{
			// Determine the LOD based on the distance to the camera position
			const float distToCameraSquared = ((bbox.maxCoords + bbox.minCoords) * 0.5f - lodCenter).lenSqr();
			unsigned int lod = (unsigned int)(m_paPVRTCityBlocks[i].numLod - 1);

			// Iterate over all LODs starting from the last until the max. suitable LOD is found
			for (unsigned int j=0; j < m_paPVRTCityBlocks[i].numLod; j++)
			{
				if (distToCameraSquared < m_afSquaredLodDistances[j])
				{
					lod = j;
					break;
				}
			}

			// and add it to our list of visible tiles
			m_pauiVisibleTiles[m_uiNumVisibleTiles].tile = i;
			m_pauiVisibleTiles[m_uiNumVisibleTiles].lod = (unsigned short)lod;
			m_pauiVisibleTiles[m_uiNumVisibleTiles].visibility = (unsigned short)culltest;
			m_uiNumVisibleTiles++;
		}
	}

	// Fine grained culling within each tile: create a list of objects within the view frustum
	for (unsigned int i=0; i < m_uiNumVisibleTiles; i++)
	{
		PVRTCityBlockLod *pLod = &m_paPVRTCityBlocks[m_pauiVisibleTiles[i].tile].paLod[m_pauiVisibleTiles[i].lod];

		// Assume that we don't see any object within the tile
		pLod->numVisibleNodes = 0;

		// and create a list of objects which are within the viewfrustum (only needed if we
		// partially intersect the tile, add all objects otherwise)
		if (m_abStates[STATE_CULLING] && (m_pauiVisibleTiles[i].visibility == INTERSECT_PARTIAL))
		{
			// Check each node in each tile against the view frustum and add it to the list
			// of visible nodes if it is visible
			for (unsigned int j=0; j < pLod->numObjects; j++)
				if (BoundingBoxIntersectsFrustum(pLod->paObjects[j].boundingbox, planes))
					pLod->paVisibleNodes[pLod->numVisibleNodes++] = j;
		}
		else
			for (unsigned int j=0; j < pLod->numObjects; j++)
				pLod->paVisibleNodes[pLod->numVisibleNodes++] = j;
	}
}


/*!****************************************************************************
 @Function		HandleInput
 @Description	Handles user input.
******************************************************************************/
void OGLES3Navigation3D::HandleInput()
{
#ifndef ENABLE_UI
	// Handle user input
	if (PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
	{
		m_abStates[STATE_PAUSE] = !m_abStates[STATE_PAUSE];
		m_mMouseLookMatrix = PVRTMat4::Identity();
	}

	if (PVRShellIsKeyPressed(PVRShellKeyNameUP))
		m_ActiveCameraTrack = ++m_ActiveCameraTrack % m_CameraPod.nNumCamera;

	if (PVRShellIsKeyPressed(PVRShellKeyNameLEFT))
		m_abStates[STATE_CULLING] = !m_abStates[STATE_CULLING];

	if (PVRShellIsKeyPressed(PVRShellKeyNameRIGHT))
	{
		m_abStates[STATE_OCCLUSION] = !m_abStates[STATE_OCCLUSION];
		if (!m_abStates[STATE_OCCLUSION])
			Update3dModelWorkingset();
	}

	if (PVRShellIsKeyPressed(PVRShellKeyNameACTION1))
		m_abStates[STATE_SHADOW] = !m_abStates[STATE_SHADOW];

	if (PVRShellIsKeyPressed(PVRShellKeyNameACTION2))
		m_abStates[STATE_DEBUG] = !m_abStates[STATE_DEBUG];

#else // ENABLE_UI

	int buttonState = PVRShellGet(prefButtonState);
	PVRTVec2 *pMousePtr = (PVRTVec2*)PVRShellGet(prefPointerLocation);
	if (0 != buttonState && NULL != pMousePtr)
	{
			PVRTVec2 mousePos = *pMousePtr;

			if (!m_bMousePressed)
			{
				m_vMouseClickPos = mousePos;
				m_vMousePrevPos = mousePos;
				m_ulLastMouseClick = PVRShellGetTime();
			}

			PVRTVec2 dragdir = mousePos - m_vMousePrevPos;
			if (dragdir.lenSqr() > 0.0001f)
				HandleMouseDrag(dragdir);

			m_vMousePrevPos = mousePos;
			m_bMousePressed = true;
	}
	else
	{
		unsigned long curTime = PVRShellGetTime();

		if (m_bMousePressed)
		{
			float dist = (m_vMouseClickPos - m_vMousePrevPos).lenSqr();
			if ((dist < 0.001f) && ((curTime - m_ulLastMouseClick) < 1000))
				HandleMouseClick(m_vMousePrevPos);

			m_bMousePressed = false;
		}
	}
#endif // ENABLE_UI
}


/*!****************************************************************************
 @Function		HandleMouseClick
 @Description	Handles user input via mouse interactions.
******************************************************************************/
void OGLES3Navigation3D::HandleMouseClick(PVRTVec2 pos)
{
	// Just rotate the clicked position if the screen has been rotated so that the physical
	// and logical click position coincide.
	if (m_bRotate)
	{
		PVRTMat3 rotMat = PVRTMat3::Rotation2D(PVRT_PI * -0.5f);
		PVRTVec3 offsetPos = PVRTVec3(pos.x - 0.5f, pos.y -0.5f, 0.0f);
		pos = PVRTVec2(rotMat * offsetPos + PVRTVec3(0.5f, 0.5f, 0.0f));
	}

#ifdef ENABLE_UI
	if (PointInBoundingBox(pos, m_Buttons[STATE_UI]))
		m_abStates[STATE_UI] = !m_abStates[STATE_UI];

	// Only allow button clicks if they are actual visible
	if (m_abStates[STATE_UI])
	{
		for (unsigned int i=0; i < STATE_UI; i++)
		{
			if (PointInBoundingBox(pos, m_Buttons[i]))
			{
				m_abStates[i] = !m_abStates[i];

				// Reset to the camera path defined view direction when changing the pause state
				if (i == STATE_PAUSE)
					m_mMouseLookMatrix = PVRTMat4::Identity();

				// Calculate the visibility set if view frustum visibility based culling should be used
				if (i == STATE_OCCLUSION && m_abStates[STATE_OCCLUSION] == false)
					Update3dModelWorkingset();

				break;
			}
		}
	}

#endif // ENABLE_UI
}


/*!****************************************************************************
 @Function		HandleMouseDrag
 @Description	Handles user input via mouse interactions.
******************************************************************************/
void OGLES3Navigation3D::HandleMouseDrag(PVRTVec2 dir)
{
	if (m_abStates[STATE_PAUSE])
	{
		if (m_bRotate)
			dir = PVRTVec2(dir.y, dir.x);

		if (m_abStates[STATE_INPUT_MODE])
		{
			dir *= c_UserCameraMovementSpeed;
			m_mMouseLookMatrix *= PVRTMat4::RotationZ(dir.x) * PVRTMat4::RotationY(-dir.y);
		}
		else
		{
			m_mMouseLightMatrix *= PVRTMat4::RotationY(dir.x) * PVRTMat4::RotationX(-dir.y);
			m_bUpdateShadowData = true;
		}
	}
}


/*!****************************************************************************
 @Function		Render3dModelsVisibilitySet
 @Description	Renders the visible city blocks based on the view frustum
                intersection tests.
******************************************************************************/
void OGLES3Navigation3D::Render3dModelsVisibilitySet()
{
	PVRTVec4 planes[4];
	ExtractViewFrustumPlanes(m_mViewProjectionMatrixNonRotated, planes[0], planes[1], planes[2], planes[3]);

	PVRTVec3 lightdir = m_vLightDirection;

	glUseProgram(m_CityEntityShader.uiId);
	glUniformMatrix4fv(m_CityEntityShader.uiModelViewProjMatrixLoc, 1, GL_FALSE, m_mViewProjectionMatrix.ptr());
	glUniform3fv(m_CityEntityShader.uiLightDirectionLoc, 1, lightdir.ptr());

	// Enable the vertex attribute arrays
	glEnableVertexAttribArray(VERTEX_ARRAY);
	glEnableVertexAttribArray(NORMAL_ARRAY);
	glEnableVertexAttribArray(TEXCOORD_ARRAY);

	GLuint prevTexture = 0;

	for (unsigned int i=0; i < m_uiNumVisibleTiles; i++)
	{
		const PVRTCityBlockLod *pLod = &m_paPVRTCityBlocks[m_pauiVisibleTiles[i].tile].paLod[m_pauiVisibleTiles[i].lod];
		const unsigned short tilevis = m_pauiVisibleTiles[i].visibility;
		if (!pLod->bLoaded)
			continue;

		glBindBuffer(GL_ARRAY_BUFFER, pLod->vbos[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pLod->vbos[1]);

		glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, sizeof(PVRTModelVertex), 0);
		glVertexAttribPointer(NORMAL_ARRAY, 3, GL_FLOAT, GL_FALSE, sizeof(PVRTModelVertex), (const void*)SNormalOffset);
		glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, sizeof(PVRTModelVertex), (const void*)STexCoordOffset);

		for (unsigned int j=0; j < pLod->numVisibleNodes; j++)
		{
			const PVRTCityBlockEntity *pObjectSet = &pLod->paObjects[pLod->paVisibleNodes[j]];

			// Ignore the object if it is not visible
			if (m_abStates[STATE_CULLING] && (tilevis == INTERSECT_PARTIAL))
				if (!BoundingBoxIntersectsFrustum(pObjectSet->boundingbox, planes))
					continue;

			for (unsigned int k=0; k < pObjectSet->numSubObjects; k++)
			{
				if (pObjectSet->pauiTextures[k] != prevTexture)
				{
					glBindTexture(GL_TEXTURE_2D, pObjectSet->pauiTextures[k]);
					prevTexture = pObjectSet->pauiTextures[k];
				}
				glDrawElements(GL_TRIANGLES, (GLsizei)pObjectSet->paNumIndices[k], GL_UNSIGNED_SHORT, (void *)(pObjectSet->paIndexOffsets[k]*sizeof(GLushort)));
			}
		}
	}

	glDisableVertexAttribArray(VERTEX_ARRAY);
	glDisableVertexAttribArray(NORMAL_ARRAY);
	glDisableVertexAttribArray(TEXCOORD_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


/*!****************************************************************************
 @Function		Render3dModelsOcclusion
 @Description	Renders the visible city blocks based on the occlusion culling
                intersection data.
******************************************************************************/
void OGLES3Navigation3D::Render3dModelsOcclusion()
{
	// Find the nearest reference spot containing occlusion data
	unsigned int nearest_pos = 0;
	float nearest_pos_dist = 99999999999.9f;
	for (unsigned int i=0; i < m_uiNumOcclusionData; i++)
	{
		float dist = (m_paPVRTOcclusionData[i].position - m_vCameraFrom).lenSqr();
		if (dist < nearest_pos_dist)
		{
			nearest_pos_dist = dist;
			nearest_pos = i;
		}
	}

	// Extract the view frustum planes for coarse culling
	PVRTVec4 planes[4];
	ExtractViewFrustumPlanes(m_mViewProjectionMatrixNonRotated, planes[0], planes[1], planes[2], planes[3]);

	glUseProgram(m_CityEntityShader.uiId);
	glUniformMatrix4fv(m_CityEntityShader.uiModelViewProjMatrixLoc, 1, GL_FALSE, m_mViewProjectionMatrix.ptr());
	glUniform3fv(m_CityEntityShader.uiLightDirectionLoc, 1, m_vLightDirection.ptr());

	// Enable the vertex attribute arrays
	glEnableVertexAttribArray(VERTEX_ARRAY);
	glEnableVertexAttribArray(NORMAL_ARRAY);
	glEnableVertexAttribArray(TEXCOORD_ARRAY);

	GLuint prevTexture = 0;

	PVRTOcclusionData *pRefData = &m_paPVRTOcclusionData[nearest_pos];
	for (unsigned int i=0; i < pRefData->numRefObjects; i++)
	{
		const unsigned int numRefObjects = pRefData->pNumRefObject[i];
		const unsigned int refTile = pRefData->pRefTile[i];
		const unsigned int *pRefObjects = pRefData->ppRefObjects[i];

		const PVRTVec2 lodReferencePosition(m_vCameraFrom);
		const float sqDist = ((m_paPVRTCityBlocks[refTile].boundingbox.maxCoords + m_paPVRTCityBlocks[refTile].boundingbox.minCoords) * 0.5f - lodReferencePosition).lenSqr();
		unsigned int lod = (unsigned int)(m_paPVRTCityBlocks[i].numLod - 1);
		for (unsigned int j=0; j < m_paPVRTCityBlocks[refTile].numLod; j++)
		{
			if (sqDist < m_afSquaredLodDistances[j])
			{
				lod = j;
				break;
			}
		}

		const PVRTCityBlockLod *pLod = &m_paPVRTCityBlocks[refTile].paLod[lod];
		if (!pLod->bLoaded)
			continue;

		// Check whether we shall employ view frustum culling, initialise as visible
		int cullingtest = INTERSECT_FULL;
		if (m_abStates[STATE_CULLING])
			cullingtest = BoundingBoxIntersectsFrustum(m_paPVRTCityBlocks[refTile].boundingbox, planes);

		// Skip the tile if it is not visible
		if (INTERSECT_NONE == cullingtest)
			continue;

		glBindBuffer(GL_ARRAY_BUFFER, pLod->vbos[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pLod->vbos[1]);

		for (unsigned int j=0; j < numRefObjects; j++)
		{
			const PVRTCityBlockEntity *pObjectSet = &pLod->paObjects[pRefObjects[j]];

			int objectvisible = INTERSECT_FULL;

			// If the tile is only partially within the frustum, test whether the current object is visible
			// otherwise assume full visibility
			if (m_abStates[STATE_CULLING] && (cullingtest == INTERSECT_PARTIAL))
				objectvisible = BoundingBoxIntersectsFrustum(pObjectSet->boundingbox, planes);

			if (INTERSECT_NONE == objectvisible)
				continue;

			for (unsigned int k=0; k < pObjectSet->numSubObjects; k++)
			{
				glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, sizeof(PVRTModelVertex), 0);
				glVertexAttribPointer(NORMAL_ARRAY, 3, GL_FLOAT, GL_FALSE, sizeof(PVRTModelVertex), (const void*)SNormalOffset);
				glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, sizeof(PVRTModelVertex), (const void*)STexCoordOffset);

				if (pObjectSet->pauiTextures[k] != prevTexture)
				{
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, pObjectSet->pauiTextures[k]);
					prevTexture = pObjectSet->pauiTextures[k];
				}
				glDrawElements(GL_TRIANGLES, (GLsizei)pObjectSet->paNumIndices[k], GL_UNSIGNED_SHORT, (void *)(pObjectSet->paIndexOffsets[k]*sizeof(GLushort)));
			}
		}
	}

	glDisableVertexAttribArray(VERTEX_ARRAY);
	glDisableVertexAttribArray(NORMAL_ARRAY);
	glDisableVertexAttribArray(TEXCOORD_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}



/*!****************************************************************************
 @Function		RenderSkyBox
 @Description	Renders a sky box centered around the view position.
******************************************************************************/
void OGLES3Navigation3D::RenderSkyBox()
{
	PVRTVec3 viewdir(m_mMouseLookMatrix * PVRTVec4(m_vCameraTo - m_vCameraFrom, 0.0f));
	PVRTVec3 updir(m_mMouseLookMatrix * PVRTVec4(m_vCameraUp, 0.0f));

	PVRTMat4 mv_matrix = PVRTMat4::LookAtRH(PVRTVec3(0.0f, -5.0f, 0.0f), viewdir + PVRTVec3(0.0f, -5.0f, 0.0f), updir);
	PVRTMat4 mvp_matrix = m_mProjectionMatrix * mv_matrix;

	glUseProgram(m_SkyboxShader.uiId);
	glUniformMatrix4fv(m_SkyboxShader.uiModelViewProjMatrixLoc, 1, GL_FALSE, mvp_matrix.f);

	glBindBuffer(GL_ARRAY_BUFFER, m_uiSkyboxVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_uiTextureIdSkybox);

	// Enable the vertex attribute arrays
	glEnableVertexAttribArray(VERTEX_ARRAY);
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, sizeof(PVRTVec3), NULL);

	// Draw primitive
	for(int i = 0; i < 6; ++i)
		glDrawArrays(GL_TRIANGLE_STRIP, i*4, 4);

	glDisableVertexAttribArray(VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


/*!****************************************************************************
 @Function		CreateModelVbo
 @Description	Loads a city block from file and creates vertex buffer objects.
                The model itself is being modified as the y and z axis are swapped
				and the resulting y axis has to be negated to coincide with 2D
				map data.
******************************************************************************/
void OGLES3Navigation3D::CreateModelVbo(const unsigned int tile, const unsigned int lod)
{
	PVRTCityBlockLod *pLod = &m_paPVRTCityBlocks[tile].paLod[lod];

	CPVRTModelPOD model;
	if (PVR_FAIL == model.ReadFromFile(pLod->pszFilename))
			return;
	else
		pLod->bLoaded = true;

	unsigned int totalVertexCount = 0;
	unsigned int totalIndexCount = 0;

	for (unsigned int i=0; i < pLod->numObjects; i++)
	{
		const PVRTuint32 numSubObjects = pLod->paObjects[i].numSubObjects;

		pLod->paObjects[i].paNumIndices = new unsigned int[numSubObjects];
		pLod->paObjects[i].paIndexOffsets = new unsigned int[numSubObjects];
		pLod->paObjects[i].pauiTextures = new GLuint[numSubObjects];

		for (unsigned int j=0; j < pLod->paObjects[i].numSubObjects; j++)
		{
			const int mesh_index = model.pNode[pLod->paObjects[i].pNodeIdx[j]].nIdx;
			const SPODMesh &mesh = model.pMesh[mesh_index];

			totalVertexCount += mesh.nNumVertex;
			totalIndexCount += PVRTModelPODCountIndices(mesh);

			pLod->paObjects[i].paNumIndices[j] = PVRTModelPODCountIndices(mesh);

			if (model.pMaterial)
			{
				const int material_index = model.pNode[pLod->paObjects[i].pNodeIdx[j]].nIdxMaterial;
				const SPODTexture &texture = model.pTexture[model.pMaterial[material_index].nIdxTexDiffuse];
				// Pre-init with invalid texture handle
				pLod->paObjects[i].pauiTextures[j] = 0;

				for (unsigned int k=0; k < m_uiNumTextures; k++)
				{
					if (strcmp(texture.pszName, c_paszTextures[k]) == 0)
					{
						pLod->paObjects[i].pauiTextures[j] = m_pauiTextureIds[k];
						break;
					}
				}
			}
		}
	}

	if (totalVertexCount > 65536)
	{
		PVRShellOutputDebug("Too many vertices to index with ushort in mesh %s!\n", pLod->pszFilename);
		pLod->bLoaded = false;
		return;
	}

	GLushort *pIndices = new GLushort[totalIndexCount];
	PVRTModelVertex *pVertices = new PVRTModelVertex[totalVertexCount];

	unsigned int indexOffset = 0;
	unsigned int vertexOffset = 0;
	for (unsigned int i=0; i < pLod->numObjects; i++)
	{
		for (unsigned int j=0; j < pLod->paObjects[i].numSubObjects; j++)
		{
			const int mesh_index = model.pNode[pLod->paObjects[i].pNodeIdx[j]].nIdx;
			const SPODMesh &mesh = model.pMesh[mesh_index];

			const unsigned int numIndices = PVRTModelPODCountIndices(mesh);

			PVRTModelVertex *pSrcVertices = (PVRTModelVertex *)mesh.pInterleaved;
			GLushort *pSrcIndices = (GLushort *)mesh.sFaces.pData;

			pLod->paObjects[i].paIndexOffsets[j] = indexOffset;

			for (unsigned int k=0; k < mesh.nNumVertex; k++)
			{
				const PVRTVec3 normal = pSrcVertices[k].normal;
				pVertices[vertexOffset + k].normal = PVRTVec3(normal.x, -normal.z, normal.y);
				const PVRTVec3 pos = pSrcVertices[k].position;
				pVertices[vertexOffset + k].position = PVRTVec3(pos.x, -pos.z, pos.y);
				pVertices[vertexOffset + k].texcoord = pSrcVertices[k].texcoord;
			}

			for (unsigned int k=0; k < numIndices; k++)
				pIndices[indexOffset + k] = (GLushort)(pSrcIndices[k] + vertexOffset);

			vertexOffset += mesh.nNumVertex;
			indexOffset += numIndices;
		}
	}

	glGenBuffers(2, pLod->vbos);
	glBindBuffer(GL_ARRAY_BUFFER, pLod->vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, totalVertexCount * sizeof(PVRTModelVertex), pVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pLod->vbos[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalIndexCount * sizeof(GLushort), pIndices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	delete [] pIndices;
	delete [] pVertices;
}

/*!****************************************************************************
 @Function		ReleaseModelVbo
 @Description	Releases all resources and memory related with a certain city block.
******************************************************************************/
void OGLES3Navigation3D::ReleaseModelVbo(const unsigned int tile, const unsigned int lod)
{
	PVRTCityBlockLod *pLod = &m_paPVRTCityBlocks[tile].paLod[lod];

	for (unsigned int i=0; i < pLod->numObjects; i++)
	{
		delete [] pLod->paObjects[i].paNumIndices;
		delete [] pLod->paObjects[i].paIndexOffsets;
		delete [] pLod->paObjects[i].pauiTextures;

		pLod->paObjects[i].paNumIndices = 0;
		pLod->paObjects[i].paIndexOffsets = 0;
		pLod->paObjects[i].pauiTextures = 0;
	}
}


/*!****************************************************************************
 @Function		CalculateLightMatrices
 @Description	TODO
******************************************************************************/
void OGLES3Navigation3D::CalculateLightMatrices()
{
	PVRTVec3 lightDir = PVRTVec3(0.1f, 0.1f, -1.0f).normalized();
	m_vLightDirection = m_mMouseLightMatrix * PVRTVec4(lightDir.x, lightDir.y, lightDir.z, 0.0f);
}


/*!****************************************************************************
 @Function		GetCameraFrame
 @Description	Retrieves camera frame from the active camera path and converts
                the units from imperial to metric system.
******************************************************************************/
void OGLES3Navigation3D::GetCameraFrame(PVRTVec3 &from, PVRTVec3 &to, PVRTVec3 &up, float time)
{
	m_CameraPod.SetFrame(time);

	m_CameraPod.GetCamera(from, to, up, m_ActiveCameraTrack);
	from *= 0.0254f;
	to *= 0.0254f;

	from = PVRTVec3(from.x, -from.z, from.y);
	to = PVRTVec3(to.x, -to.z, to.y);
	up = PVRTVec3(up.x, -up.z, up.y);
}

/*!****************************************************************************
 @Function		CalculateCameraMatrices
 @Description	Calculates and interpolates the current camera frame. The
                interpolation is being done to prevent the camera from stuttering
				at slow speed passages (due to floating point issues).
******************************************************************************/
void OGLES3Navigation3D::CalculateCameraMatrices()
{
	float clamped_delta = m_fCameraAnimation + 5.0f;
	if (clamped_delta > (m_CameraPod.nNumFrame - 1))
		clamped_delta -= (m_CameraPod.nNumFrame - 1);

	PVRTVec3 now_from, now_to, now_up;
	GetCameraFrame(now_from, now_to, now_up, m_fCameraAnimation);

	PVRTVec3 next_from, next_to, next_up;
	GetCameraFrame(next_from, next_to, next_up, clamped_delta);

	m_vCameraFrom = now_from;
	m_vCameraTo = now_from + (next_from - now_from).normalized();
	m_vCameraUp = now_up;

	m_mProjectionMatrix = PVRTMat4::PerspectiveFovRH(m_fFOV, m_fAspectRatio, m_fNearClipPlane, m_fFarClipPlane, PVRTMat4::OGL, m_bRotate);
	PVRTVec3 viewdir(m_mMouseLookMatrix * PVRTVec4(m_vCameraTo - m_vCameraFrom, 0.0f));
	PVRTVec3 updir(m_mMouseLookMatrix * PVRTVec4(m_vCameraUp, 0.0f));
	m_mViewMatrix = PVRTMat4::LookAtRH(m_vCameraFrom, m_vCameraFrom + viewdir, updir);
	m_mViewProjectionMatrix = m_mProjectionMatrix * m_mViewMatrix;

	if (!m_bRotate)
		m_mViewProjectionMatrixNonRotated = m_mViewProjectionMatrix;
	else
		m_mViewProjectionMatrixNonRotated = PVRTMat4::PerspectiveFovRH(m_fFOV, 1.0f / m_fAspectRatio, m_fNearClipPlane, m_fFarClipPlane, PVRTMat4::OGL, false) * m_mViewMatrix;
}

/*!****************************************************************************
 @Function		ExtractViewFrustumPlanes
 @Description	Extracts the (left, right, front and back) view frustum planes
                from the camera modelview-projection matrix.
******************************************************************************/
void OGLES3Navigation3D::ExtractViewFrustumPlanes(const PVRTMat4 &matrix, PVRTVec4 &left, PVRTVec4 &right, PVRTVec4 &front, PVRTVec4 &back) const
{
	left.x = (matrix.f[3]  + matrix.f[0]);
    left.y = (matrix.f[7]  + matrix.f[4]);
    left.z = (matrix.f[11] + matrix.f[8]);
	left.w = (matrix.f[15] + matrix.f[12]);
	float inv_len = 1.0f / PVRTVec3(left).length();
	left *= inv_len;

	right.x = (matrix.f[3]  - matrix.f[0]);
    right.y = (matrix.f[7]  - matrix.f[4]);
    right.z = (matrix.f[11] - matrix.f[8]);
	right.w = (matrix.f[15] - matrix.f[12]);
	inv_len = 1.0f / PVRTVec3(right).length();
	right *= inv_len;

	front.x = (matrix.f[3]  + matrix.f[2]);
    front.y = (matrix.f[7]  + matrix.f[6]);
    front.z = (matrix.f[11] + matrix.f[10]);
	front.w = (matrix.f[15] + matrix.f[14]);
	inv_len = 1.0f / PVRTVec3(front).length();
	front *= inv_len;

	back.x = (matrix.f[3]  - matrix.f[2]);
    back.y = (matrix.f[7]  - matrix.f[6]);
    back.z = (matrix.f[11] - matrix.f[10]);
	back.w = (matrix.f[15] - matrix.f[14]);
	inv_len = 1.0f / PVRTVec3(back).length();
	back *= inv_len;
}



/*!****************************************************************************
 @Function		Load3dModelIndex
 @Description	Loads hierarchical index data (please see the 3D Navigation
                whitepaper for a file format description).
******************************************************************************/
bool OGLES3Navigation3D::Load3dModelIndex(const char *pszFilename, CPVRTString* pErrorStr)
{
	CPVRTResourceFile file(pszFilename);
	if (!file.IsOpen())
	{
		*pErrorStr = "Error: Could not open 3d model hirarchy file!\n";
		return false;
	}

	const char *pData = (const char *)file.DataPtr();

	PVRTuint32 numTiles;
	memcpy((char *)&numTiles, pData, sizeof(numTiles));
	pData += sizeof(numTiles);

	m_uiNumPVRTCityBlocks = (unsigned int)numTiles;
	m_paPVRTCityBlocks = new PVRTCityBlock[m_uiNumPVRTCityBlocks];

	for (unsigned int i=0; i < m_uiNumPVRTCityBlocks; i++)
	{
		memcpy((char *)&m_paPVRTCityBlocks[i].boundingbox, pData, sizeof(PVRTBoundingBox2D));
		pData += sizeof(PVRTBoundingBox2D);
		memcpy((char *)&m_paPVRTCityBlocks[i].numLod, pData, sizeof(PVRTuint32));
		pData += sizeof(PVRTuint32);

		m_paPVRTCityBlocks[i].paLod = new PVRTCityBlockLod[m_paPVRTCityBlocks[i].numLod];

		for (unsigned int j=0; j < m_paPVRTCityBlocks[i].numLod; j++)
		{
			m_paPVRTCityBlocks[i].paLod[j].bLoaded = false;

			PVRTuint32 namelength;
			memcpy((char *)&namelength, pData, sizeof(namelength));
			pData += sizeof(namelength);

			m_paPVRTCityBlocks[i].paLod[j].pszFilename = new char[namelength+1];
			memcpy((char *)m_paPVRTCityBlocks[i].paLod[j].pszFilename, pData, sizeof(char) * namelength);
			pData += sizeof(char) * namelength;
			m_paPVRTCityBlocks[i].paLod[j].pszFilename[namelength] = '\0';

			memcpy((char *)&m_paPVRTCityBlocks[i].paLod[j].numObjects, pData, sizeof(PVRTuint32));
			pData += sizeof(PVRTuint32);
			m_paPVRTCityBlocks[i].paLod[j].paObjects = new PVRTCityBlockEntity[m_paPVRTCityBlocks[i].paLod[j].numObjects];

			m_paPVRTCityBlocks[i].paLod[j].paVisibleNodes = new unsigned int[m_paPVRTCityBlocks[i].paLod[j].numObjects];
			m_paPVRTCityBlocks[i].paLod[j].numVisibleNodes = 0;

			for (unsigned int k=0; k < m_paPVRTCityBlocks[i].paLod[j].numObjects; k++)
			{
				memcpy((char *)&m_paPVRTCityBlocks[i].paLod[j].paObjects[k].boundingbox, pData, sizeof(PVRTBoundingBox2D));
				pData += sizeof(PVRTBoundingBox2D);
				memcpy((char *)&m_paPVRTCityBlocks[i].paLod[j].paObjects[k].numSubObjects, pData, sizeof(PVRTuint32));
				pData += sizeof(PVRTuint32);

				m_paPVRTCityBlocks[i].paLod[j].paObjects[k].pNodeIdx = new unsigned int[m_paPVRTCityBlocks[i].paLod[j].paObjects[k].numSubObjects];

				memcpy((char *)m_paPVRTCityBlocks[i].paLod[j].paObjects[k].pNodeIdx, pData, sizeof(unsigned int) * m_paPVRTCityBlocks[i].paLod[j].paObjects[k].numSubObjects);
				pData += sizeof(unsigned int) * m_paPVRTCityBlocks[i].paLod[j].paObjects[k].numSubObjects;

				m_paPVRTCityBlocks[i].paLod[j].paObjects[k].pauiTextures = 0;
				m_paPVRTCityBlocks[i].paLod[j].paObjects[k].paNumIndices = 0;
				m_paPVRTCityBlocks[i].paLod[j].paObjects[k].paIndexOffsets = 0;
			}
		}
	}

	return true;
}

/*!****************************************************************************
 @Function		Release3dModelIndex
 @Description	Releases the hierarchical index data.
******************************************************************************/
void OGLES3Navigation3D::Release3dModelIndex()
{
	for (unsigned int i=0; i < m_uiNumPVRTCityBlocks; i++)
	{
		for (unsigned int j=0; j < m_paPVRTCityBlocks[i].numLod; j++)
		{
			for (unsigned int k=0; k < m_paPVRTCityBlocks[i].paLod[j].numObjects; k++)
			{
				delete [] m_paPVRTCityBlocks[i].paLod[j].paObjects[k].pNodeIdx;
				delete [] m_paPVRTCityBlocks[i].paLod[j].paObjects[k].paNumIndices;
				delete [] m_paPVRTCityBlocks[i].paLod[j].paObjects[k].paIndexOffsets;
				delete [] m_paPVRTCityBlocks[i].paLod[j].paObjects[k].pauiTextures;
			}
			delete [] m_paPVRTCityBlocks[i].paLod[j].paObjects;
			delete [] m_paPVRTCityBlocks[i].paLod[j].paVisibleNodes;
			delete [] m_paPVRTCityBlocks[i].paLod[j].pszFilename;
		}
		delete [] m_paPVRTCityBlocks[i].paLod;
	}
	delete [] m_paPVRTCityBlocks;
}


/*!****************************************************************************
 @Function		LoadOcclusionData
 @Description	Loads the occlusion (/visibility) data information (please see
                the 3D Navigation whitepaper for a file format description).
******************************************************************************/
bool OGLES3Navigation3D::LoadOcclusionData(const char *pszFilename, CPVRTString* pErrorStr)
{
	CPVRTResourceFile file(pszFilename);
	if (!file.IsOpen())
	{
		*pErrorStr = "Error: Could not open occlusion data!\n";
		return false;
	}

	const char *pData = (const char *)file.DataPtr();

	PVRTuint32 namelen;
	memcpy((char *)&namelen, pData, sizeof(namelen));
	pData += sizeof(namelen);
	// Skip the name
	pData += namelen;

	PVRTuint32 numTiles;
	memcpy((char *)&numTiles, pData, sizeof(numTiles));
	pData += sizeof(numTiles);
	for (unsigned int i=0; i < numTiles; i++)
	{
		memcpy((char *)&namelen, pData, sizeof(namelen));
		pData += sizeof(namelen);
		// Skip the name
		pData += namelen;
	}

	PVRTuint32 numpositions;
	memcpy((char *)&numpositions, pData, sizeof(numpositions));
	pData += sizeof(numpositions);
	m_uiNumOcclusionData = (unsigned int)numpositions;

	m_paPVRTOcclusionData = new PVRTOcclusionData[m_uiNumOcclusionData];

	for (unsigned int i=0; i < m_uiNumOcclusionData; i++)
	{
		memcpy((char *)&m_paPVRTOcclusionData[i].position, pData, sizeof(PVRTVec3));
		pData += sizeof(PVRTVec3);

		PVRTuint32 reftiles;
		memcpy((char *)&reftiles, pData, sizeof(reftiles));
		pData += sizeof(reftiles);

		m_paPVRTOcclusionData[i].numRefObjects = (unsigned int)reftiles;
		m_paPVRTOcclusionData[i].pRefTile = new unsigned int[reftiles];
		m_paPVRTOcclusionData[i].pNumRefObject = new unsigned int[reftiles];
		m_paPVRTOcclusionData[i].ppRefObjects = new unsigned int*[reftiles];

		for (unsigned int j=0; j < reftiles; j++)
		{
			PVRTuint32 tilenum;
			memcpy((char *)&tilenum, pData, sizeof(tilenum));
			pData += sizeof(tilenum);
			m_paPVRTOcclusionData[i].pRefTile[j] = (unsigned int)tilenum;

			PVRTuint32 num_ref_models;
			memcpy((char *)&num_ref_models, pData, sizeof(num_ref_models));
			pData += sizeof(num_ref_models);

			m_paPVRTOcclusionData[i].pNumRefObject[j] = (unsigned int)num_ref_models;
			m_paPVRTOcclusionData[i].ppRefObjects[j] = new unsigned int[num_ref_models];
			memcpy((char *)m_paPVRTOcclusionData[i].ppRefObjects[j], pData, sizeof(unsigned int) * num_ref_models);
			pData += sizeof(unsigned int) * num_ref_models;
		}
	}

	return true;
}

/*!****************************************************************************
 @Function		ReleaseOcclusionData
 @Description	Releases all occlusion data.
******************************************************************************/
void OGLES3Navigation3D::ReleaseOcclusionData()
{
	for (unsigned int i=0; i < m_uiNumOcclusionData; i++)
	{
		for (unsigned int j=0; j < m_paPVRTOcclusionData[i].numRefObjects; j++)
			delete [] m_paPVRTOcclusionData[i].ppRefObjects[j];
		delete [] m_paPVRTOcclusionData[i].pNumRefObject;
		delete [] m_paPVRTOcclusionData[i].pRefTile;
		delete [] m_paPVRTOcclusionData[i].ppRefObjects;
	}

	delete [] m_paPVRTOcclusionData;
}

/*!****************************************************************************
 @Function		BoundingBoxIntersectsFrustum
 @Description	Tests whether a 2D bounding box is intersected or enclosed
                by a camera view frustum. Only the front, left, right and back
				planes of the view frustum are taken into consideration to
				optimize the intersection test.
******************************************************************************/
int OGLES3Navigation3D::BoundingBoxIntersectsFrustum(const PVRTBoundingBox2D &bbox, const PVRTVec4 planes[4]) const
{
	int totalinside = 0;

	// Test the axis-aligned bounding box against each plane;
	// only cull if all points are outside of one the view frustum planes
	for (unsigned int p=0; p < 4; p++)
	{
		int pointsOut = 0;

		// Test the points against the plane
		if ((planes[p].x * bbox.minCoords.x + planes[p].y * bbox.minCoords.y + planes[p].w) < 0.0f)
			pointsOut++;
		if ((planes[p].x * bbox.maxCoords.x + planes[p].y * bbox.minCoords.y + planes[p].w) < 0.0f)
			pointsOut++;
		if ((planes[p].x * bbox.maxCoords.x + planes[p].y * bbox.maxCoords.y + planes[p].w) < 0.0f)
			pointsOut++;
		if ((planes[p].x * bbox.minCoords.x + planes[p].y * bbox.maxCoords.y + planes[p].w) < 0.0f)
			pointsOut++;

		// if all points are outside of a plane we can cull the whole bounding box
		if (pointsOut == 4)
			return INTERSECT_NONE;

		// if all points are inside of a plane, note it
		if (pointsOut == 0)
			totalinside++;
	}

	if (totalinside == 4)
		return INTERSECT_FULL;
	else
		return INTERSECT_PARTIAL;
}


/*!****************************************************************************
 @Function		CreateShadowVolumes
 @Description	Builds the shadow meshes and shadow volumes
******************************************************************************/
void OGLES3Navigation3D::CreateShadowVolumes(unsigned int tile)
{
	const unsigned int max_bbox_count = 256;
	PVRTBoundingBox3D bbox_array[max_bbox_count];

	for (unsigned int i=0; i < m_paPVRTCityBlocks[tile].numLod; i++)
	{
		CPVRTModelPOD model;
		if (PVR_SUCCESS != model.ReadFromFile(m_paPVRTCityBlocks[tile].paLod[i].pszFilename))
			continue;

		PVRTCityBlockLod *pLod = &m_paPVRTCityBlocks[tile].paLod[i];

		unsigned int count = 0;
		for (unsigned int j=0; j < pLod->numObjects; j++)
		{
			// Peek whether the current model is a building, if not skip it
			if (strstr(model.pNode[pLod->paObjects[j].pNodeIdx[0]].pszName, "BUILDING") == NULL)
					continue;

			// Calculate the 3d bounding box for the whole building
			PVRTBoundingBox3D bbox;
			bbox.minCoords = PVRTVec3(FLT_MAX);
			bbox.maxCoords = PVRTVec3(-FLT_MAX);
			for (unsigned int k=0; k < pLod->paObjects[j].numSubObjects; k++)
			{
				SPODNode *pNode = &model.pNode[pLod->paObjects[j].pNodeIdx[k]];
				SPODMesh *pMesh = &model.pMesh[pNode->nIdx];
				PVRTModelVertex *pData = (PVRTModelVertex *)pMesh->pInterleaved;
				for (unsigned int n=0; n < pMesh->nNumVertex; n++)
				{
					PVRTVec3 vertex = PVRTVec3(pData[n].position.x, -pData[n].position.z, pData[n].position.y);
					bbox.minCoords.x = PVRT_MIN(bbox.minCoords.x, vertex.x);
					bbox.minCoords.y = PVRT_MIN(bbox.minCoords.y, vertex.y);
					bbox.minCoords.z = PVRT_MIN(bbox.minCoords.z, vertex.z);
					bbox.maxCoords.x = PVRT_MAX(bbox.maxCoords.x, vertex.x);
					bbox.maxCoords.y = PVRT_MAX(bbox.maxCoords.y, vertex.y);
					bbox.maxCoords.z = PVRT_MAX(bbox.maxCoords.z, vertex.z);
				}
			}

			// If the whole building is flat, skip it
			if (bbox.minCoords.z == bbox.maxCoords.z)
					continue;

			bbox_array[count++] = bbox;
			if (count >= max_bbox_count)
					break;
		}

		if (count == 0)
		{
			m_ppaShadowMesh[tile] = 0;
			m_ppaShadowVol[tile] = 0;
			m_ppaVolumeScale[tile] = 0;
			m_paNumShadowVols[tile] = 0;
			return;
		}

		m_ppaShadowMesh[tile] = new PVRTShadowVolShadowMesh[count];
		m_ppaShadowVol[tile] = new PVRTShadowVolShadowVol[count];
		m_ppaVolumeScale[tile] = new float[count];
		m_paNumShadowVols[tile] = count;

		for (unsigned int j=0; j < count; j++)
		{
			PVRTBoundingBox3D bbox = bbox_array[j];
			// Reduce the size of the shadow casting bounding box to reduce depth-fighting artefacts
			PVRTVec3 midCoord = (bbox.maxCoords + bbox.minCoords) * 0.5f;
			bbox.minCoords = (bbox.minCoords - midCoord) * 0.99f + midCoord;
			bbox.maxCoords = (bbox.maxCoords - midCoord) * 0.99f + midCoord;

			PVRTVec3 pVertices[8];
			pVertices[0] = bbox.minCoords;
			pVertices[1] = PVRTVec3(bbox.maxCoords.x, bbox.minCoords.y, bbox.minCoords.z);
			pVertices[2] = PVRTVec3(bbox.maxCoords.x, bbox.maxCoords.y, bbox.minCoords.z);
			pVertices[3] = PVRTVec3(bbox.minCoords.x, bbox.maxCoords.y, bbox.minCoords.z);
			pVertices[4] = PVRTVec3(bbox.minCoords.x, bbox.minCoords.y, bbox.maxCoords.z);
			pVertices[5] = PVRTVec3(bbox.maxCoords.x, bbox.minCoords.y, bbox.maxCoords.z);
			pVertices[6] = bbox.maxCoords;
			pVertices[7] = PVRTVec3(bbox.minCoords.x, bbox.maxCoords.y, bbox.maxCoords.z);

			unsigned short pIndices[36];
			pIndices[ 0] = 3; pIndices[ 1] = 2; pIndices[ 2] = 1;
			pIndices[ 3] = 3; pIndices[ 4] = 1; pIndices[ 5] = 0;

			pIndices[ 6] = 0; pIndices[ 7] = 1; pIndices[ 8] = 5;
			pIndices[ 9] = 0; pIndices[10] = 5; pIndices[11] = 4;

			pIndices[12] = 3; pIndices[13] = 0; pIndices[14] = 4;
			pIndices[15] = 3; pIndices[16] = 4; pIndices[17] = 7;

			pIndices[18] = 2; pIndices[19] = 3; pIndices[20] = 7;
			pIndices[21] = 2; pIndices[22] = 7; pIndices[23] = 6;

			pIndices[24] = 1; pIndices[25] = 2; pIndices[26] = 6;
			pIndices[27] = 1; pIndices[28] = 6; pIndices[29] = 5;

			pIndices[30] = 4; pIndices[31] = 5; pIndices[32] = 6;
			pIndices[33] = 4; pIndices[34] = 6; pIndices[35] = 7;

			// Create a mesh format suitable for generating shadow volumes.
			PVRTShadowVolMeshCreateMesh(&m_ppaShadowMesh[tile][j], pVertices[0].ptr(), 8, pIndices, 12);

			// Init the mesh
			PVRTShadowVolMeshInitMesh(&m_ppaShadowMesh[tile][j], 0);

			// Create the shadow volume
			PVRTShadowVolMeshInitVol(&m_ppaShadowVol[tile][j], &m_ppaShadowMesh[tile][j], 0);

			m_ppaVolumeScale[tile][j] = bbox.maxCoords.z;
		}

		// Only generate shadow volumes for the first available LOD per tile
		return;
	}
}


/*!****************************************************************************
 @Function		ReleaseShadowVolumes
 @Description	Releases all shadow volume related data
******************************************************************************/
void OGLES3Navigation3D::ReleaseShadowVolumes()
{
	for (unsigned int i=0; i < m_uiNumPVRTCityBlocks; i++)
	{
		for (unsigned int j=0; j < m_paNumShadowVols[i]; j++)
		{
			if(m_ppaShadowMesh[i])
			{
				PVRTShadowVolMeshDestroyMesh(&m_ppaShadowMesh[i][j]);
				PVRTShadowVolMeshReleaseMesh(&m_ppaShadowMesh[i][j]);
				PVRTShadowVolMeshReleaseVol(&m_ppaShadowVol[i][j]);
			}
		}
		delete [] m_ppaShadowMesh[i];
		delete [] m_ppaShadowVol[i];
		delete [] m_ppaVolumeScale[i];
	}
	delete [] m_ppaShadowMesh;
	delete [] m_ppaShadowVol;
	delete [] m_paNumShadowVols;
	delete [] m_ppaVolumeScale;
}


/*!****************************************************************************
 @Function		UpdateShadowVolumes
 @Description	Updates the shadow volumes within a tile.
******************************************************************************/
void OGLES3Navigation3D::UpdateShadowVolumes()
{
	// Flags: PVRTSHADOWVOLUME_VISIBLE | PVRTSHADOWVOLUME_NEED_CAP_FRONT | PVRTSHADOWVOLUME_NEED_CAP_BACK
	//        0x01                       0x02                              0x04
	const unsigned int flags = PVRTSHADOWVOLUME_VISIBLE  | PVRTSHADOWVOLUME_NEED_CAP_FRONT | PVRTSHADOWVOLUME_NEED_CAP_BACK;

	for (unsigned int j=0; j < m_uiNumPVRTCityBlocks; j++)
	{
		for (unsigned int i=0; i < m_paNumShadowVols[j]; i++)
		{
			PVRTShadowVolSilhouetteProjectedBuild(&m_ppaShadowVol[j][i], flags, &m_ppaShadowMesh[j][i], (PVRTVec3*)m_vLightDirection.ptr(), false);
		}
	}
}

/*!****************************************************************************
 @Function		RenderShadowVolumes
 @Description	Renders the shadow volumes using the stencil shadow volumes
                algorithm (zfail variant).
******************************************************************************/
void OGLES3Navigation3D::RenderShadowVolumes()
{
	//	For a detailed explanation on how to use the Stencil Buffer see the training course:	Stencil Buffer.

	// Setup a smaller view frustum for the stencil shadow culling routine, as we only want to have
	// shadows near to the viewer.
	PVRTMat4 mStencilMVPMatrix;
	if (!m_bRotate)
		mStencilMVPMatrix = PVRTMat4::PerspectiveFovRH(m_fFOV, m_fAspectRatio, m_fNearClipPlane, m_fFarClipPlane * c_fShadowVolumesMaxDistance, PVRTMat4::OGL, m_bRotate) * m_mViewMatrix;
	else
		mStencilMVPMatrix = PVRTMat4::PerspectiveFovRH(m_fFOV, 1.0f / m_fAspectRatio, m_fNearClipPlane, m_fFarClipPlane * c_fShadowVolumesMaxDistance, PVRTMat4::OGL, false) * m_mViewMatrix;

	PVRTVec4 planes[4];
	ExtractViewFrustumPlanes(mStencilMVPMatrix, planes[0], planes[1], planes[2], planes[3]);

	glEnable(GL_STENCIL_TEST);

	// Calculate the scale factor for the shadow volume extrusion based on the light angle
	float angle_cos = m_vLightDirection.dot(PVRTVec3(0.0f, 0.0f, -1.0f));
	float scalefactor = 1.01f;
	if (angle_cos != 0.0f)
		scalefactor = (1.0f / angle_cos) + 0.01f;

	PVRTMat4 mMVP = m_mViewProjectionMatrix;

#ifdef USE_INFINITE_FAR_PLANE
	{
		mMVP = m_mProjectionMatrix;
		float cot = (1.0f/tan(m_fFOV * 0.5f));
		mMVP.f[0] = cot / m_fAspectRatio;
		mMVP.f[1] = mMVP.f[2] = mMVP.f[3] = 0.0f;

		mMVP.f[4] = 0.0f;
		mMVP.f[5] = cot;
		mMVP.f[6] = mMVP.f[7] = 0.0f;

		mMVP.f[8] = mMVP.f[9] = 0.0f;
		mMVP.f[10] = mMVP.f[11] = -1.0f;

		mMVP.f[12] = mMVP.f[13] = 0.0f;
		mMVP.f[14] = m_fNearClipPlane * -2.0f;
		mMVP.f[15] = 0.0f;

		mMVP = mMVP * m_mViewMatrix;
	}
#endif

	// Use the shader program that is used for the shadow volumes
	glUseProgram(m_ShadowVolShader.uiId);

	glUniformMatrix4fv(m_ShadowVolShader.uiModelViewProjMatrixLoc, 1, GL_FALSE, mMVP.f);

	const float afColor[] = { 0.4f, 1.0f, 0.0f, 0.2f };
	glUniform4fv(m_ShadowVolShader.uiColourLoc, 1, afColor);

	PVRTVec3 lightDir = m_vLightDirection;
	glUniform3fv(m_ShadowVolShader.uiLightDirectionLoc, 1, lightDir.ptr());

	//If we want to display the shadow volumes don't disable the colour mask and enable blending
	if(m_abStates[STATE_DEBUG])
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else // Disable the colour mask so we don't draw to the colour buffer
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	// Disable writing to the depth buffer
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LESS);

	glDisable(GL_CULL_FACE);
	glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);

	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);

	for (unsigned int i=0; i < m_uiNumPVRTCityBlocks; i++)
	{
		if (INTERSECT_NONE == BoundingBoxIntersectsFrustum(m_paPVRTCityBlocks[i].boundingbox, planes))
			continue;

		for (unsigned int j=0; j < m_paNumShadowVols[i]; j++)
		{
			glUniform1f(m_ShadowVolShader.uiVolumeScaleLoc, m_ppaVolumeScale[i][j] * scalefactor);
			PVRTShadowVolSilhouetteProjectedRender(&m_ppaShadowMesh[i][j], &m_ppaShadowVol[i][j], 0);
		}
	}

	// Enable Culling as we would like it back
	glEnable(GL_CULL_FACE);

	// Set the stencil function so we only draw where the stencil buffer isn't 0
	glStencilFunc(GL_NOTEQUAL, 0, 0xFFFFFFFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	if(!m_abStates[STATE_DEBUG])
	{
		// Enable the colour buffer
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		glEnable(GL_BLEND);

		// Use modulate blending
		glBlendFunc(GL_DST_COLOR, GL_ZERO);

		// Use the shader program for the scene
		glUseProgram(m_FullscreenShader.uiId);

		const float afShadowColor[] = { 0.6f, 0.6f, 0.6f, 1.0f };
		glUniform4fv(m_FullscreenShader.uiColourLoc, 1, afShadowColor);

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

	// Enable writing to the depth buffer
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);

	glDisable(GL_STENCIL_TEST);
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
	return new OGLES3Navigation3D();
}

/******************************************************************************
 End of file (OGLES3Navigation3D.cpp)
******************************************************************************/

