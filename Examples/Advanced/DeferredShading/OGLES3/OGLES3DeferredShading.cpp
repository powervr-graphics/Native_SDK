/******************************************************************************

 @File         OGLES3DeferredShading.cpp

 @Title        Deferred Shading

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Implements a deferred shading technique supporting point and
               directional lights.

******************************************************************************/

#include "PVRShell.h"
#include "OGLES3Tools.h"

/******************************************************************************
 Constants
******************************************************************************/

const float c_fDemoFrameRate = 1.0f / 80.0f;

const float c_fPointLightScale = 50.0f;
const float c_fPointlightIntensity = 100.0f;

const float c_fDirectionallightIntensity = 3.0f;

/******************************************************************************
 Defines
******************************************************************************/

#define VERTEX_ARRAY		0
#define NORMAL_ARRAY		1
#define TEXCOORD_ARRAY		2
#define TANGENT_ARRAY		3

enum eFBO
{
	FBO_ALBEDO = 0,
	FBO_NORMAL,
	FBO_DEPTH,
	FBO_DEFERRED,
	NUM_FBOS
};

enum RenderMode
{	
	RENDER_ALBEDO = FBO_ALBEDO,
	RENDER_NORMALS = FBO_NORMAL,		
	RENDER_DEPTH = FBO_DEPTH,
	RENDER_DEFERRED,	
	RENDER_GEOMETRY,
	NUM_RENDER_MODES
};

/******************************************************************************
 Global strings
******************************************************************************/

const CPVRTStringHash c_sGBufferEffectName          = CPVRTStringHash("RenderGBuffer");
const CPVRTStringHash c_sPointLightEffectName       = CPVRTStringHash("RenderPointLight");
const CPVRTStringHash c_sDirectionalLightEffectName = CPVRTStringHash("RenderDirectionalLight");
const CPVRTStringHash c_sSimpleTextureEffectName    = CPVRTStringHash("RenderSimpleTexture");
const CPVRTStringHash c_sCubeTextureEffectName      = CPVRTStringHash("RenderCubeTexture");
const CPVRTStringHash c_sDepthTextureEffectName     = CPVRTStringHash("RenderDepthTexture");
const CPVRTStringHash c_sSolidColourEffectName      = CPVRTStringHash("RenderSolidColour");

/******************************************************************************
 Structures and enums
******************************************************************************/

enum eCustomSemantics
{
	eCUSTOMSEMANTIC_FARCLIPDISTANCE = ePVRTPFX_NumSemantics + 1,
	eCUSTOMSEMANTIC_SPECULARPOWER,
	eCUSTOMSEMANTIC_DIFFUSECOLOUR,
	eCUSTOMSEMANTIC_POINTLIGHT_VIEWPOSITION,
	eCUSTOMSEMANTIC_DIRECTIONALLIGHT_DIRECTION
};

const SPVRTPFXUniformSemantic c_CustomSemantics[] = 
{ 
	{ "CUSTOMSEMANTIC_FARCLIPDISTANCE",            eCUSTOMSEMANTIC_FARCLIPDISTANCE },
	{ "CUSTOMSEMANTIC_SPECULARPOWER",	           eCUSTOMSEMANTIC_SPECULARPOWER },
	{ "CUSTOMSEMANTIC_DIFFUSECOLOUR",              eCUSTOMSEMANTIC_DIFFUSECOLOUR },
	{ "CUSTOMSEMANTIC_POINTLIGHT_VIEWPOSITION",    eCUSTOMSEMANTIC_POINTLIGHT_VIEWPOSITION },
	{ "CUSTOMSEMANTIC_DIRECTIONALLIGHT_DIRECTION", eCUSTOMSEMANTIC_DIRECTIONALLIGHT_DIRECTION },
};
const unsigned int c_uiNumCustomSemantics = sizeof(c_CustomSemantics)/sizeof(c_CustomSemantics[0]);

/******************************************************************************
 Content file names
******************************************************************************/

const char *c_pszRenderModes[NUM_RENDER_MODES] = { "Albedo", "Normals", "Depth", "Deferred", "Geometry" };

const char *c_pszPointLightModel     = "pointlight.pod";
const char *c_pszLightEnvironmentMap = "light_cubemap.pvr";

const char c_szSceneFile[]  = "scene.pod";
const char c_szPFXSrcFile[]	= "effect.pfx";

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES3DeferredShading : public PVRShell
{
	CPVRTPrint3D m_Print3D;	
	SPVRTContext m_sContext;

	// The effect file handlers
	CPVRTPFXParser	   *m_pPFXEffectParser;
	CPVRTPFXEffect    **m_ppPFXEffects;
	CPVRTMap<int, int> *m_pUniformMapping;

	// Frame counters for animation
	float        m_fFrame;
	bool         m_bScreenRotated;
	bool         m_bPaused;
	unsigned int m_uiCameraId;
	unsigned int m_RenderMode;
					
	// Projection and Model View matrices
	PVRTVec3   m_vCameraPosition;
	PVRTMat4   m_mView;	
	PVRTMat4   m_mProjection;	
	PVRTMat4   m_mViewProjection;
	PVRTMat4   m_mInverseView;
	float      m_fFarClipDistance;

	int   m_iWindowWidth;
	int   m_iWindowHeight;
	int   m_iFboWidth;
	int   m_iFboHeight;
	int   m_iViewportOffsets[2];
	
	// Handles for textures
	GLuint  m_uiDefaultDiffuseTexture;
	GLuint  m_uiDefaultBumpTexture;
	GLuint  m_uiLightEnvironmentMap;

	// Handles for FBOs and surfaces
	GLint  m_iOriginalFBO;
	GLuint m_uiGBufferFBO;
	GLuint m_uiGBufferDepthStencilRenderBuffer;			
	GLuint m_uiRenderTextures[NUM_FBOS];
	
	// Light proxy models	
	CPVRTModelPOD m_PointLightModel;
	GLuint        m_uiPointLightModelVAO;
	GLuint        m_uiPointLightModelVBO;
	GLuint        m_uiPointLightModelIBO;
	
	struct Material
	{
		GLuint   uiTexture;
		GLuint   uiBumpmap;
		GLfloat  fSpecularPower;
		PVRTVec3 vDiffuseColour;
	}
	*m_pMaterials;
	
	struct Model
	{
		CPVRTModelPOD pod;
		GLuint   *puiVAOs;
		GLuint   *puiVBOs;
		GLuint   *puiIBOs;
	}
	m_Scene;
			
	// Point lights
	unsigned int m_uiNumPointLights;
	struct PointLight
	{
		unsigned int uiNodeIdx;
		PVRTVec3     vColour;
		PVRTMat4     mProxyScale;
		PVRTMat4     mTransformation;
	}
	*m_pPointLights;

	// Directional lights
	unsigned int m_uiNumDirectionalLights;
	struct DirectionalLight
	{		
		unsigned int uiNodeIdx;
		PVRTVec3     vColour;
		PVRTMat4     mTransformation;

		PVRTVec4     vDirection;
	}
	*m_pDirectionalLights;
				
public:

	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	bool LoadTextures(CPVRTString* const pErrorStr);
	bool LoadVbos(CPVRTString* const pErrorStr);
	bool LoadLights(CPVRTString* const pErrorStr);
	bool LoadPFX(CPVRTString* const pErrorStr);
		
	void DrawAxisAlignedQuad(PVRTVec2 afLowerLeft, PVRTVec2 afUpperRight);

	bool AllocateGBuffer(CPVRTString *pErrorStr);	
	void RenderGBuffer();	

	void DrawSceneDeferred();
	void DrawSceneFlatColoured();		
	void DrawLightSources();
	void DrawPointLightGeometry(const float alpha);

	void DrawPointLightProxies();	
	void DrawDirectionalLightProxies();

	void HandleInput();	
	void UpdateAnimation();
};

/*!****************************************************************************
 @Function		LoadTextures
 @Output		pErrorStr		A CPVRTString describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this example
******************************************************************************/
bool OGLES3DeferredShading::LoadTextures(CPVRTString* const pErrorStr)
{
	if (m_Scene.pod.nNumMaterial == 0)
	{
		*pErrorStr = CPVRTString("ERROR: The scene does not contain any materials.");
		return false;
	}

	// Initialise the default (fall back) albedo texture as white
	unsigned char pDiffuse[3] = { 255, 255, 255 };
	glGenTextures(1, &m_uiDefaultDiffuseTexture);
	glBindTexture(GL_TEXTURE_2D, m_uiDefaultDiffuseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, pDiffuse);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);		
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Initialise the default (fall back) normal texture as an outward pointing normal
	unsigned char pBump[3] = { 0, 0, 255 };
	glGenTextures(1, &m_uiDefaultBumpTexture);
	glBindTexture(GL_TEXTURE_2D, m_uiDefaultBumpTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, pBump);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);		
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Load the cubemap that is used as a light environment map
	if(PVRTTextureLoadFromPVR(c_pszLightEnvironmentMap, &m_uiLightEnvironmentMap) != PVR_SUCCESS)
	{
		*pErrorStr = CPVRTString("ERROR: Failed to load texture ") + CPVRTString(c_pszLightEnvironmentMap);
		return false;
	}	

	// Load the materials from the POD file
	m_pMaterials = new Material[m_Scene.pod.nNumMaterial];

	for (unsigned int i=0; i < m_Scene.pod.nNumMaterial; i++)
	{
		const SPODMaterial &material = m_Scene.pod.pMaterial[i];

		m_pMaterials[i].fSpecularPower = material.fMatShininess;
		m_pMaterials[i].vDiffuseColour = PVRTVec3(material.pfMatDiffuse);
		
		if (material.nIdxTexDiffuse != -1)
		{
			// Load the diffuse texture map
			PVRTextureHeaderV3 header;			
			if(PVRTTextureLoadFromPVR(m_Scene.pod.pTexture[material.nIdxTexDiffuse].pszName, &m_pMaterials[i].uiTexture, &header) != PVR_SUCCESS)
			{
				*pErrorStr = CPVRTString("ERROR: Failed to load texture ") + CPVRTString(m_Scene.pod.pTexture[material.nIdxTexDiffuse].pszName);
				return false;
			}

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);		
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		else
		{
			// Otherwise assign the default texture map
			m_pMaterials[i].uiTexture = m_uiDefaultDiffuseTexture;
		}

		if (material.nIdxTexBump != -1)
		{
			// Load the bumpmap
			if(PVRTTextureLoadFromPVR(m_Scene.pod.pTexture[material.nIdxTexBump].pszName, &m_pMaterials[i].uiBumpmap) != PVR_SUCCESS)
			{
				*pErrorStr = CPVRTString("ERROR: Failed to load texture ") + CPVRTString(m_Scene.pod.pTexture[material.nIdxTexBump].pszName);
				return false;
			}

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);		
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		else
		{
			// Otherwise assign the default texture map
			m_pMaterials[i].uiBumpmap = m_uiDefaultBumpTexture;
		}
	}	

	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this example into
				vertex buffer objects
******************************************************************************/
bool OGLES3DeferredShading::LoadVbos(CPVRTString* const pErrorStr)
{
	//
	// Load the scene
	//
	m_Scene.puiVAOs = new GLuint[m_Scene.pod.nNumMesh];
	m_Scene.puiVBOs = new GLuint[m_Scene.pod.nNumMesh];
	m_Scene.puiIBOs = new GLuint[m_Scene.pod.nNumMesh];
		
	glGenVertexArrays(m_Scene.pod.nNumMesh, m_Scene.puiVAOs);
	glGenBuffers(m_Scene.pod.nNumMesh, m_Scene.puiVBOs);
	for (unsigned int i = 0; i < m_Scene.pod.nNumMesh; ++i)
	{
		glBindVertexArray(m_Scene.puiVAOs[i]);

		// Load vertex data into buffer object
		SPODMesh& Mesh = m_Scene.pod.pMesh[i];
		// Only indexed triangles are supported
		if (!Mesh.sFaces.pData)
		{
			*pErrorStr = CPVRTString("ERROR: Failed loading scene, only indexed geometry is supported.");
			return false;
		}

		unsigned int uiSize = Mesh.nNumVertex * Mesh.sVertex.nStride;
		glBindBuffer(GL_ARRAY_BUFFER, m_Scene.puiVBOs[i]);
		glBufferData(GL_ARRAY_BUFFER, uiSize, Mesh.pInterleaved, GL_STATIC_DRAW);

		glGenBuffers(1, &m_Scene.puiIBOs[i]);
		uiSize = PVRTModelPODCountIndices(Mesh) * Mesh.sFaces.nStride;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Scene.puiIBOs[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, Mesh.sFaces.pData, GL_STATIC_DRAW);

		glEnableVertexAttribArray(VERTEX_ARRAY);
		glEnableVertexAttribArray(NORMAL_ARRAY);
		glEnableVertexAttribArray(TEXCOORD_ARRAY);
		glEnableVertexAttribArray(TANGENT_ARRAY);

		glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, Mesh.sVertex.nStride, Mesh.sVertex.pData);
		glVertexAttribPointer(NORMAL_ARRAY, 3, GL_FLOAT, GL_FALSE, Mesh.sNormals.nStride, Mesh.sNormals.pData);
		glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, Mesh.psUVW->nStride, Mesh.psUVW->pData);
		glVertexAttribPointer(TANGENT_ARRAY, 3, GL_FLOAT, GL_FALSE, Mesh.sTangents.nStride, Mesh.sTangents.pData);	
	}
	glBindVertexArray(0);

	// 
	//  Load the point light model
	// 	
	{		
		glGenBuffers(1, &m_uiPointLightModelVBO);
		glGenBuffers(1, &m_uiPointLightModelIBO);

		// Load vertex data into buffer object
		SPODMesh& Mesh = m_PointLightModel.pMesh[0];
		// Only indexed triangles are supported
		if (!Mesh.sFaces.pData)
		{
			*pErrorStr = CPVRTString("ERROR: Failed loading point light proxy, only indexed geometry is supported.");
			return false;
		}

		glGenVertexArrays(1, &m_uiPointLightModelVAO);
		glBindVertexArray(m_uiPointLightModelVAO);		

		unsigned int uiSize = Mesh.nNumVertex * Mesh.sVertex.nStride;
		glBindBuffer(GL_ARRAY_BUFFER, m_uiPointLightModelVBO);
		glBufferData(GL_ARRAY_BUFFER, uiSize, Mesh.pInterleaved, GL_STATIC_DRAW);		

		uiSize = PVRTModelPODCountIndices(Mesh) * sizeof(GLshort);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiPointLightModelIBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, Mesh.sFaces.pData, GL_STATIC_DRAW);			

		glEnableVertexAttribArray(VERTEX_ARRAY);
		glEnableVertexAttribArray(NORMAL_ARRAY);

		glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, Mesh.sVertex.nStride, Mesh.sVertex.pData);
		glVertexAttribPointer(NORMAL_ARRAY, 3, GL_FLOAT, GL_FALSE, Mesh.sNormals.nStride, Mesh.sNormals.pData);

		glBindVertexArray(0);		
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);		

	return true;
}


/*!****************************************************************************
 @Function		LoadLights
 @Return		bool		true if no error occured
 @Description	Loads all lights from the scene and prepares helper structures.
******************************************************************************/
bool OGLES3DeferredShading::LoadLights(CPVRTString* const pErrorStr)
{	
	if (m_Scene.pod.nNumLight == 0)
		return false;		

	const unsigned int uiLightNodeOffset = m_Scene.pod.nNumMeshNode;	

	// Iterate through the scene and count and tag the lights
	for (unsigned int i=0; i < m_Scene.pod.nNumLight; i++)
	{
		switch (m_Scene.pod.pLight[i].eType)
		{		
		case ePODPoint:       
			m_uiNumPointLights++; 
			break;
		case ePODDirectional: 
			m_uiNumDirectionalLights++; 
			break;
		default:             
			*pErrorStr = CPVRTString("ERROR: Only point and directional light sources are supported.");
			return false;
		}
	}

	//
	// Allocate per-light buffers
	//
	if (m_uiNumPointLights > 0)
		m_pPointLights = new PointLight[m_uiNumPointLights];	
	if (m_uiNumDirectionalLights > 0)
		m_pDirectionalLights = new DirectionalLight[m_uiNumDirectionalLights];
	
	//
	// Determine the indices for the individual lights as they are found in the pod file
	//		
	unsigned int uiPointLightIdx = 0;
	unsigned int uiDirectionalLightIdx = 0;
	for (unsigned int i=0; i < m_Scene.pod.nNumLight; i++)
	{
		const SPODLight &light = m_Scene.pod.pLight[i];					
		
		switch (light.eType)
		{		
		case ePODPoint:       
			{
				m_pPointLights[uiPointLightIdx].uiNodeIdx = uiLightNodeOffset + i;
				m_pPointLights[uiPointLightIdx].vColour = PVRTVec3(light.pfColour);
				m_pPointLights[uiPointLightIdx].mTransformation = PVRTMat4::Identity();
				m_pPointLights[uiPointLightIdx].mProxyScale = PVRTMat4::Identity();
				uiPointLightIdx++;
			}
			break;

		case ePODDirectional: 
			{
				m_pDirectionalLights[uiDirectionalLightIdx].uiNodeIdx = uiLightNodeOffset + i;				
				m_pDirectionalLights[uiDirectionalLightIdx].vColour = PVRTVec3(light.pfColour);
				uiDirectionalLightIdx++;
			}
			break;

		default:              
			break;
		}
	}

	return true;
}


/*!****************************************************************************
 @Function		LoadPFX
 @Return		bool			true if no error occurred
 @Description	Loads and compiles the shaders and links the shader programs
				required for this training course
******************************************************************************/
bool OGLES3DeferredShading::LoadPFX(CPVRTString* const pErrorStr)
{
	CPVRTString error;

	// Parse the whole PFX and store all data.
	m_pPFXEffectParser = new CPVRTPFXParser();
	if(m_pPFXEffectParser->ParseFromFile(c_szPFXSrcFile, &error) != PVR_SUCCESS)
	{
		*pErrorStr = CPVRTString("Parse failed:\n\n") + error;
		return false;
	}
	
	// Setup all effects in the PFX file so we initialize the shaders and
	// store uniforms and attributes locations.
	unsigned int uiNumEffects = m_pPFXEffectParser->GetNumberEffects();
	m_ppPFXEffects = new CPVRTPFXEffect*[uiNumEffects];
	m_pUniformMapping = new CPVRTMap<int,int>[uiNumEffects];

	for (unsigned int i=0; i < uiNumEffects; i++)
		m_ppPFXEffects[i] = new CPVRTPFXEffect(m_sContext);

	// Load one by one the effects. This will also compile the shaders.
	for (unsigned int i=0; i < uiNumEffects; i++)
	{
		if(m_ppPFXEffects[i]->RegisterUniformSemantic(c_CustomSemantics, c_uiNumCustomSemantics, &error))
		{
			*pErrorStr = CPVRTString("Failed to set custom semantics:\n\n") + error;
			return false;
		}

		unsigned int nUnknownUniformCount = 0;
		if(m_ppPFXEffects[i]->Load(*m_pPFXEffectParser, m_pPFXEffectParser->GetEffect(i).Name.c_str(), NULL, NULL, nUnknownUniformCount, &error)  != PVR_SUCCESS)
		{
			*pErrorStr = CPVRTString("Failed to load effect ") + m_pPFXEffectParser->GetEffect(i).Name.String() + CPVRTString(":\n\n") + error;
			return false;
		}

		// .. upps, some uniforms are not in our table. Better to quit because something is not quite right.
		if(nUnknownUniformCount)
		{
			*pErrorStr = CPVRTString("Unknown uniforms found in effect: ") + m_pPFXEffectParser->GetEffect(i).Name.String();
			return false;
		}		

		// Create the mapping so we can reference the uniforms more easily
		m_ppPFXEffects[i]->Activate();
		const CPVRTArray<SPVRTPFXUniform>& Uniforms = m_ppPFXEffects[i]->GetUniformArray();
		for(unsigned int j = 0; j < Uniforms.GetSize(); ++j)
		{
			m_pUniformMapping[i][Uniforms[j].nSemantic] = Uniforms[j].nLocation;
			if (Uniforms[j].nSemantic == ePVRTPFX_UsTEXTURE)
				glUniform1i(Uniforms[j].nLocation, Uniforms[j].nIdx);
		}
	}
	
	return true;
}


/*!****************************************************************************
 @Function		InitApplication
 @Return		bool		true if no error occurred
 @Description	Code in InitApplication() will be called by PVRShell once per
				run, before the rendering context is created.
				Used to initialize variables that are not dependent on it
				(e.g. external modules, loading meshes, etc.)
				If the rendering context is lost, InitApplication() will
				not be called again.
******************************************************************************/
bool OGLES3DeferredShading::InitApplication()
{	
	m_RenderMode = RENDER_DEFERRED;

	m_fFrame = 0.0f;
	m_bPaused = false;
	m_uiCameraId = 0;
	
	m_Scene.puiVAOs = 0;
	m_Scene.puiVBOs = 0;
	m_Scene.puiIBOs = 0;

	m_pMaterials = 0;

	m_uiNumPointLights = 0;
	m_pPointLights = 0;	

	m_uiNumDirectionalLights = 0;
	m_pDirectionalLights = 0;	

	m_pPFXEffectParser = 0;
	m_ppPFXEffects = 0;
	m_pUniformMapping = 0;
	
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	// 
	//  Load the scene and the lights
	//
	if (m_Scene.pod.ReadFromFile(c_szSceneFile) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Couldn't load the scene pod file\n");
		return false;
	}

	if (m_Scene.pod.nNumLight == 0)
	{
		PVRShellSet(prefExitMessage, "ERROR: No lights found in scene\n");
		return false;
	}

	if (m_Scene.pod.nNumCamera == 0)
	{
		PVRShellSet(prefExitMessage, "ERROR: No cameras found in scene\n");
		return false;
	}

	//
	//	Load lights from the scene and convert them into the internal representation
	//
	CPVRTString errorStr;
	if (!LoadLights(&errorStr))
	{
		PVRShellSet(prefExitMessage, errorStr.c_str());
		return false;
	}

	//
	//	Load light proxy geometry
	//
	if (m_PointLightModel.ReadFromFile(c_pszPointLightModel) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Couldn't load the point light proxy pod file\n");
		return false;
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
				not be called.x
******************************************************************************/
bool OGLES3DeferredShading::QuitApplication()
{
	// Free the memory allocated for the scene
	m_Scene.pod.Destroy();

	delete [] m_Scene.puiVAOs;
	delete [] m_Scene.puiVBOs;
	delete [] m_Scene.puiIBOs;

	delete [] m_pPointLights;
	delete [] m_pDirectionalLights;

	delete [] m_pMaterials;

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
bool OGLES3DeferredShading::InitView()
{	
	int iMaxRenderbufferSize;
	glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &iMaxRenderbufferSize);	
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_iOriginalFBO); 

	PVRShellOutputDebug("Renderbuffer max. size: %d\n", iMaxRenderbufferSize);

	m_iWindowWidth = PVRShellGet(prefWidth);
	m_iWindowHeight = PVRShellGet(prefHeight);

	m_iFboWidth = m_iWindowWidth;
	m_iFboHeight = m_iWindowHeight;	

#ifdef __APPLE__
	m_iFboWidth = m_iFboHeight = PVRTGetPOTLower(PVRT_MIN(m_iWindowWidth, m_iWindowHeight), 0);
#endif

	int numCmdLineOpts = PVRShellGet(prefCommandLineOptNum);
	const SCmdLineOpt *pCmdLineOpts = (const SCmdLineOpt *)PVRShellGet(prefCommandLineOpts);
	for (int i=0; i < numCmdLineOpts; i++)
	{
		if (strcmp(pCmdLineOpts[i].pArg, "-fbowidth") == 0)
			m_iFboWidth = PVRT_MIN(atoi(pCmdLineOpts[i].pVal), m_iWindowWidth);
		else if (strcmp(pCmdLineOpts[i].pArg, "-fboheight") == 0)
			m_iFboHeight = PVRT_MIN(atoi(pCmdLineOpts[i].pVal), m_iWindowHeight);
	}

	m_iViewportOffsets[0] = (m_iWindowWidth - m_iFboWidth) / 2;
	m_iViewportOffsets[1] = (m_iWindowHeight - m_iFboHeight) / 2;

	PVRShellOutputDebug("FBO dimensions: %d x %d\n", m_iFboWidth, m_iFboHeight);
	PVRShellOutputDebug("Framebuffer dimensions: %d x %d\n", m_iWindowWidth, m_iWindowHeight);
				
	CPVRTString ErrorStr;

	//
	//  Load textures
	//
	if (!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}
		
	//
	//	Load objects from the scene into VBOs
	//
	if (!LoadVbos(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	//
	//	Load and compile the shaders & link programs
	//
	if (!LoadPFX(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	//
	//	Allocates the gbuffer buffer objects
	//
	if (!AllocateGBuffer(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Is the screen rotated?
	m_bScreenRotated = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	//
	//  Initialize Print3D
	//
	if(m_Print3D.SetTextures(NULL, PVRShellGet(prefWidth), PVRShellGet(prefHeight), m_bScreenRotated) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}	
				
	//
	//  Set default OpenGL render states 
	//
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);			
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);		

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occurred
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES3DeferredShading::ReleaseView()
{		
	// Delete buffer objects
	glDeleteBuffers(m_Scene.pod.nNumMesh, m_Scene.puiVAOs);
	glDeleteBuffers(m_Scene.pod.nNumMesh, m_Scene.puiVBOs);
	glDeleteBuffers(m_Scene.pod.nNumMesh, m_Scene.puiIBOs);

	glDeleteBuffers(1, &m_uiPointLightModelVAO);
	glDeleteBuffers(1, &m_uiPointLightModelVBO);
	glDeleteBuffers(1, &m_uiPointLightModelIBO);	

	glDeleteBuffers(1, &m_uiGBufferDepthStencilRenderBuffer);

	for (unsigned int i=0; i < m_Scene.pod.nNumMaterial; i++)
	{
		glDeleteTextures(1, &m_pMaterials[i].uiTexture);
		glDeleteTextures(1, &m_pMaterials[i].uiBumpmap);
	}
	glDeleteTextures(1, &m_uiDefaultDiffuseTexture);
	glDeleteTextures(1, &m_uiDefaultBumpTexture);
	
	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

	// Release the effect[s] then the parser
	for (unsigned int i=0; i < m_pPFXEffectParser->GetNumberEffects(); i++)
	{
		m_ppPFXEffects[i]->Destroy();
		delete m_ppPFXEffects[i];
		m_pUniformMapping[i].Clear();
	}
	delete [] m_ppPFXEffects;
	delete m_pPFXEffectParser;
	delete [] m_pUniformMapping;
	
	return true;
}

/*!****************************************************************************
 @Function		RenderScene
 @Return		bool		true if no error occured
 @Description	Main rendering loop function of the program. The shell will
				call this function every frame.
				eglSwapBuffers() will be performed by PVRShell automatically.
				PVRShell will also manage important OS events.
				Will also manage relevant OS events. The user has access to
				these events through an abstraction layer provided by PVRShell.
******************************************************************************/
bool OGLES3DeferredShading::RenderScene()
{
	const PVRTMat4 mId = PVRTMat4::Identity();
	//
	//  Handle user input and update object animations
	//
	HandleInput();
	UpdateAnimation();	

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);	
	glDisable(GL_STENCIL_TEST);
					
	switch (m_RenderMode)
	{
	case RENDER_DEFERRED:
		{			
			//
	        //  Render the G-Buffer
            //
			DrawSceneDeferred();			
		}
		break;

	case RENDER_ALBEDO:
	case RENDER_NORMALS:
	case RENDER_DEPTH:
		{	
			// Render the albedo part of the gbuffer
			glBindFramebuffer(GL_FRAMEBUFFER, m_uiGBufferFBO);	
			glViewport(0, 0, m_iFboWidth, m_iFboHeight);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClearStencil(0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);		
			RenderGBuffer();
			
			glDisable(GL_STENCIL_TEST);
			glBindFramebuffer(GL_FRAMEBUFFER, m_iOriginalFBO);
			glViewport(0, 0, m_iWindowWidth, m_iWindowHeight);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			if ((m_iFboWidth != m_iWindowWidth ) || (m_iFboHeight != m_iWindowHeight))
				glViewport(m_iViewportOffsets[0], m_iViewportOffsets[1], m_iFboWidth, m_iFboHeight);

			CPVRTStringHash sEffectName = (m_RenderMode == RENDER_DEPTH) ? c_sDepthTextureEffectName : c_sSimpleTextureEffectName;
			const int iEffectId = m_pPFXEffectParser->FindEffectByName(sEffectName);
			m_ppPFXEffects[iEffectId]->Activate();
			glUniformMatrix4fv(m_pUniformMapping[iEffectId][ePVRTPFX_UsWORLDVIEWPROJECTION], 1, GL_FALSE, mId.f);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_uiRenderTextures[m_RenderMode]);
			DrawAxisAlignedQuad(PVRTVec2(-1, -1), PVRTVec2(1, 1));
		}
		break;

	case RENDER_GEOMETRY:
		{
			glDisable(GL_STENCIL_TEST);
			glBindFramebuffer(GL_FRAMEBUFFER, m_iOriginalFBO);
			glViewport(0, 0, m_iWindowWidth, m_iWindowHeight);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			if ((m_iFboWidth != m_iWindowWidth ) || (m_iFboHeight != m_iWindowHeight))
				glViewport(m_iViewportOffsets[0], m_iViewportOffsets[1], m_iFboWidth, m_iFboHeight);
			
			DrawSceneFlatColoured();
			DrawLightSources();
			
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			DrawPointLightGeometry(0.75f);						
			glDisable(GL_BLEND);			
		}
		break;
	}
	
	if ((m_iFboWidth != m_iWindowWidth ) || (m_iFboHeight != m_iWindowHeight))
		glViewport(0, 0, m_iWindowWidth, m_iWindowHeight);
	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("Deferred Shading", c_pszRenderModes[m_RenderMode], ePVRTPrint3DSDKLogo);	
	if (m_bPaused) m_Print3D.Print3D(1.0f, 15.0f, 0.75f, 0xFFFFFFFF, "Paused");	
	m_Print3D.Flush();	

	return true;
}


/*!****************************************************************************
 @Function		DrawSceneFlatColoured
  @Description	Renders the scene flat coloured with a predefined palette
******************************************************************************/
void OGLES3DeferredShading::DrawSceneFlatColoured()
{				
	// The colour palette
	static const PVRTVec4 sRandColours[] = 
	{ 
		PVRTVec4(1.0f, 0.0f, 0.0f, 1.0f), PVRTVec4(0.0f, 1.0f, 0.0f, 1.0f), PVRTVec4(0.0f, 0.0f, 1.0f, 1.0f),
		PVRTVec4(1.0f, 0.0f, 1.0f, 1.0f), PVRTVec4(0.0f, 1.0f, 1.0f, 1.0f), PVRTVec4(1.0f, 1.0f, 1.0f, 1.0f),
		PVRTVec4(1.0f, 1.0f, 0.0f, 1.0f), PVRTVec4(0.0f, 0.0f, 0.0f, 1.0f), PVRTVec4(0.5f, 1.0f, 0.5f, 1.0f),
	};
	
	const int iEffectId = m_pPFXEffectParser->FindEffectByName(c_sSolidColourEffectName);
	m_ppPFXEffects[iEffectId]->Activate();

	for (unsigned int i=0; i < m_Scene.pod.nNumMeshNode; i++)
	{		
		const SPODNode &Node = m_Scene.pod.pNode[i];
		const SPODMesh &Mesh = m_Scene.pod.pMesh[Node.nIdx];		
		const PVRTMat4 mModelViewProj = m_mViewProjection * m_Scene.pod.GetWorldMatrix(Node);
		
		const Material &material = m_pMaterials[Node.nIdxMaterial];		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, material.uiTexture);		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, material.uiBumpmap);
		glUniformMatrix4fv(m_pUniformMapping[iEffectId][ePVRTPFX_UsWORLDVIEWPROJECTION], 1, GL_FALSE, mModelViewProj.f);
		glUniform4fv(m_pUniformMapping[iEffectId][ePVRTPFX_UsMATERIALCOLORAMBIENT], 1, &sRandColours[i%9].x);

		// Bind the vertex array object that stores all required attributes for that mesh
		glBindVertexArray(m_Scene.puiVAOs[Node.nIdx]);
				
		// Indexed Triangle list
		GLenum datatype = (Mesh.sFaces.eType == EPODDataUnsignedShort) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
		glDrawElements(GL_TRIANGLES, Mesh.nNumFaces*3, datatype, 0);
	}

	glBindVertexArray(0);
}


/*!****************************************************************************
 @Function		AllocateGBuffer
 @Description	Allocates the required FBOs and buffer objects
******************************************************************************/
bool OGLES3DeferredShading::AllocateGBuffer(CPVRTString *pErrorStr)
{
	//
	// Allocate the gbuffer surfaces with the following components
	//                           Albedo	           Normal            Depth             Offscreen	
	GLenum internalformats[] = { GL_RGBA,          GL_RGB,           GL_RGBA,          GL_RGB };
	GLenum formats[]         = { GL_RGBA,          GL_RGB,           GL_RGBA,          GL_RGB };
	GLenum types[]           = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE };	

	// Allocate the render targets
	glGenTextures(NUM_FBOS, m_uiRenderTextures);	
	for (unsigned int i=0; i < NUM_FBOS; i++)
	{
		glBindTexture(GL_TEXTURE_2D, m_uiRenderTextures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, internalformats[i], m_iFboWidth, m_iFboHeight, 0, formats[i], types[i], NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);				
	}

	glGenRenderbuffers(1, &m_uiGBufferDepthStencilRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_uiGBufferDepthStencilRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_iFboWidth, m_iFboHeight);	

	//
	// Allocate the gbuffer fbo and attach the surfaces
	//	
	glGenFramebuffers(1, &m_uiGBufferFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_uiGBufferFBO);								
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_uiGBufferDepthStencilRenderBuffer);		
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uiRenderTextures[FBO_ALBEDO], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_uiRenderTextures[FBO_NORMAL], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_uiRenderTextures[FBO_DEPTH], 0);

	// Setup the rendertargets to their corresponding attachment points
	GLenum drawbuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, drawbuffers);

	// Check if the fbo is setup correctly
	GLenum framebufferstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (framebufferstatus != GL_FRAMEBUFFER_COMPLETE)
	{
		switch (framebufferstatus)
		{
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			*pErrorStr = CPVRTString("ERROR: gbuffer not set up correctly: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
			*pErrorStr = CPVRTString("ERROR: gbuffer not set up correctly: GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS\n");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			*pErrorStr = CPVRTString("ERROR: gbuffer not set up correctly: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n");
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			*pErrorStr = CPVRTString("ERROR: gbuffer not set up correctly: GL_FRAMEBUFFER_UNSUPPORTED\n");
			break;
		}
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_iOriginalFBO);

	return true;
}


/*!****************************************************************************
 @Function		RenderGBuffer
 @Description	Renders the gbuffer
******************************************************************************/
void OGLES3DeferredShading::RenderGBuffer()
{
	const int iEffectId = m_pPFXEffectParser->FindEffectByName(c_sGBufferEffectName);
	m_ppPFXEffects[iEffectId]->Activate();	

	glUniform1f(m_pUniformMapping[iEffectId][eCUSTOMSEMANTIC_FARCLIPDISTANCE], m_fFarClipDistance);
	
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	for (unsigned int i=0; i < m_Scene.pod.nNumMeshNode; i++)
	{
		const SPODNode &Node = m_Scene.pod.pNode[i];		
		const SPODMesh &Mesh = m_Scene.pod.pMesh[Node.nIdx];	

		const PVRTMat4 mWorld = m_Scene.pod.GetWorldMatrix(Node);
		const PVRTMat4 mWorldView = m_mView * mWorld;
		const PVRTMat4 mModelViewProj = m_mViewProjection * mWorld;
		PVRTMat3 mWorldView3x3(mWorldView);
		const PVRTMat3 mWorldViewIT3x3 = mWorldView3x3.inverse().transpose();		

		const Material &material = m_pMaterials[Node.nIdxMaterial];
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, material.uiTexture);		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, material.uiBumpmap);
		
		glUniformMatrix4fv(m_pUniformMapping[iEffectId][ePVRTPFX_UsWORLDVIEW], 1, GL_FALSE, mWorldView.f);
		glUniformMatrix4fv(m_pUniformMapping[iEffectId][ePVRTPFX_UsWORLDVIEWPROJECTION], 1, GL_FALSE, mModelViewProj.f);
		glUniformMatrix3fv(m_pUniformMapping[iEffectId][ePVRTPFX_UsWORLDVIEWIT], 1, GL_FALSE, mWorldViewIT3x3.f);

		glUniform1f(m_pUniformMapping[iEffectId][eCUSTOMSEMANTIC_SPECULARPOWER], material.fSpecularPower);
		glUniform3fv(m_pUniformMapping[iEffectId][eCUSTOMSEMANTIC_DIFFUSECOLOUR], 1, &material.vDiffuseColour.x);

		// Bind the vertex array object that stores all required attributes for that mesh
		glBindVertexArray(m_Scene.puiVAOs[Node.nIdx]);
		
		GLenum datatype = (Mesh.sFaces.eType == EPODDataUnsignedShort) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
		glDrawElements(GL_TRIANGLES, Mesh.nNumFaces*3, datatype, 0);		
	}
	
	glDisable(GL_STENCIL_TEST);

	glBindVertexArray(0);
}


/*!****************************************************************************
 @Function		DrawSceneDeferred
 @Description	Renders the scene using the gbuffer
******************************************************************************/
void OGLES3DeferredShading::DrawSceneDeferred()
{
	//
	//  Render the GBuffer
	//
	glBindFramebuffer(GL_FRAMEBUFFER, m_uiGBufferFBO);	
	glViewport(0, 0, m_iFboWidth, m_iFboHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);		
	RenderGBuffer();

	// Give the drivers a hint that we don't want stencil or depth information to be stored for later.
	const GLenum attachments[] = { GL_STENCIL_ATTACHMENT, GL_DEPTH_ATTACHMENT };
	glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments);

	//
	//  Bind the main framebuffer object, render the geometry to the depth and stencil buffers and 
	//  finally add the light contributions using the GBuffer.
	// 
	//  At first render the directional light contributions, utilising the stencil buffer to avoid executing
	//  the shaders in areas that don't need to be lit (e.g. sky box).
	//  After that render the point light source contributions; in order to limit the amount of shaded fragments
	//  make use of the stencil buffer to imprint the areas that are actually affected by the light sources.
	//  This is similar to the stencil buffer shadow algorithm which runs very efficiently on tile based renderers.
	//

	glBindFramebuffer(GL_FRAMEBUFFER, m_iOriginalFBO);		
	glViewport(0, 0, m_iWindowWidth, m_iWindowHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	
	glClearStencil(0);

	if ((m_iFboWidth != m_iWindowWidth ) || (m_iFboHeight != m_iWindowHeight))
		glViewport(m_iViewportOffsets[0], m_iViewportOffsets[1], m_iFboWidth, m_iFboHeight);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);

	// Imprint a 1 into the stencil buffer to indicate where geometry is found.
	// This optimizes the rendering of directional light sources as the shader then
	// only has to be executed where necessary.
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	
	// Render the objects to the depth and stencil buffers but not to the framebuffer
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	DrawSceneFlatColoured();

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDisable(GL_STENCIL_TEST);

	// Bind the GBuffer to the various texture channels so we can access it in the shader
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uiRenderTextures[FBO_ALBEDO]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_uiRenderTextures[FBO_NORMAL]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_uiRenderTextures[FBO_DEPTH]);			

	// Disable depth writes as we do not want to modify the depth buffer while rendering the light sources.
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);

	// Additively blend the light contributions
	glEnable(GL_BLEND);			
	glBlendFunc(GL_ONE, GL_ONE);	

	//
	//  Render the directional light contribution
	//	
	if (m_uiNumDirectionalLights > 0)
	{
		// Make use of the stencil buffer contents to only shade pixels where actual geometry is located.
		// Reset the stencil buffer to 0 at the same time to avoid the stencil clear operation afterwards.
		glEnable(GL_STENCIL_TEST);		
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
		
		DrawDirectionalLightProxies();
	}
	else
	{
		// A directional essentially does a clear for free as it renders a full screen quad 
		// for each directional light and resets the stencil buffer to zero.
		// If there aren't any directional lights do a manual clear.
		glClear(GL_STENCIL_BUFFER_BIT);
	}

	//
	//  Render the point light contribution
	//	
	
	if (m_uiNumPointLights > 0)
	{
		// Disable back face culling as we are using z-fail similar to shadow volumes to update 
		// the stencil buffer with regions that are affected by the light sources.
		glDisable(GL_CULL_FACE);

		// Set the stencil test to the z-fail method and disable colour writes
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 0, 0xFF);
		glStencilMask(0xFF);			
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_INCR_WRAP, GL_KEEP);			
		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);					

		DrawPointLightGeometry(1.0f);				

		// Set the stencil test to only shade the lit areas and re-enable colour writes.
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF);			
		glStencilMask(0xFF);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		
		DrawPointLightProxies();
	}	

	// Restore state
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);	
	glDepthMask(GL_TRUE);		
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);						
	glDepthFunc(GL_LEQUAL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	

	// Render the actual light sources to indicate where the light is coming from
	DrawLightSources();
}


/*!****************************************************************************
 @Function		DrawTransparentObjects
 @Description	Renders the point light sources
******************************************************************************/
void OGLES3DeferredShading::DrawLightSources()
{
	const int iEffectId = m_pPFXEffectParser->FindEffectByName(c_sCubeTextureEffectName);
	m_ppPFXEffects[iEffectId]->Activate();	

	// Bind the vertex and index buffer for the point light
	const SPODMesh &Mesh = m_PointLightModel.pMesh[0];
	const unsigned int uiNumFaces = m_PointLightModel.pMesh[0].nNumFaces * 3;

	glBindVertexArray(m_uiPointLightModelVAO);
		
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_uiLightEnvironmentMap);		
	
	for (unsigned int i=0; i < m_uiNumPointLights; i++)
	{
		PVRTVec4 vColour(m_pPointLights[i].vColour.x, m_pPointLights[i].vColour.y, m_pPointLights[i].vColour.z, 0.8f);
		const PVRTMat4 mModelViewProj = m_mViewProjection * m_pPointLights[i].mTransformation;
		PVRTMat3 mModelIT(m_pPointLights[i].mTransformation);
		mModelIT = mModelIT.inverse().transpose();


		glUniformMatrix4fv(m_pUniformMapping[iEffectId][ePVRTPFX_UsWORLDVIEWPROJECTION], 1, GL_FALSE, mModelViewProj.f);	
		if (m_pUniformMapping[iEffectId].Exists(ePVRTPFX_UsWORLDIT))
			glUniformMatrix3fv(m_pUniformMapping[iEffectId][ePVRTPFX_UsWORLDIT], 1, GL_FALSE, mModelIT.f);
		glUniform4fv(m_pUniformMapping[iEffectId][ePVRTPFX_UsMATERIALCOLORAMBIENT], 1, &vColour.x);

		GLenum datatype = (Mesh.sFaces.eType == EPODDataUnsignedShort) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);	
		glDrawElements(GL_TRIANGLES, Mesh.nNumFaces*3, datatype, 0);
		glDisable(GL_BLEND);
	}

	glBindVertexArray(0);
}


/*!***************************************************************************
@Function		DrawPointLightGeometry
@Description	Renders the light proxy geometry using a simple shader
*****************************************************************************/
void OGLES3DeferredShading::DrawPointLightGeometry(const float alpha)
{
	const int iEffectId = m_pPFXEffectParser->FindEffectByName(c_sSolidColourEffectName);
	m_ppPFXEffects[iEffectId]->Activate();
	glEnableVertexAttribArray(VERTEX_ARRAY);

	// Bind the vertex and index buffer for the point light
	const SPODMesh &Mesh = m_PointLightModel.pMesh[0];
	const unsigned int uiNumFaces = m_PointLightModel.pMesh[0].nNumFaces * 3;

	glBindVertexArray(m_uiPointLightModelVAO);
	
	for (unsigned int i=0; i < m_uiNumPointLights; i++)
	{
		PVRTVec4 colour(m_pPointLights[i].vColour.x, m_pPointLights[i].vColour.y, m_pPointLights[i].vColour.z, alpha);
		const PVRTMat4 mWorldScale = m_pPointLights[i].mTransformation * m_pPointLights[i].mProxyScale;
		const PVRTMat4 mModelViewProj = m_mViewProjection * mWorldScale;

		glUniform4fv(m_pUniformMapping[iEffectId][ePVRTPFX_UsMATERIALCOLORAMBIENT], 1, colour.ptr());	
		glUniformMatrix4fv(m_pUniformMapping[iEffectId][ePVRTPFX_UsWORLDVIEWPROJECTION], 1, GL_FALSE, mModelViewProj.f);
		
		GLenum datatype = (Mesh.sFaces.eType == EPODDataUnsignedShort) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
		glDrawElements(GL_TRIANGLES, Mesh.nNumFaces*3, datatype, 0);
	}
	
	glBindVertexArray(0);
}


/*!***************************************************************************
@Function		DrawLightProxies
@Description	Renders all light proxies for debugging purposes
*****************************************************************************/
void OGLES3DeferredShading::DrawPointLightProxies()
{
	const int iEffectId = m_pPFXEffectParser->FindEffectByName(c_sPointLightEffectName);
	m_ppPFXEffects[iEffectId]->Activate();

	if (m_pUniformMapping[iEffectId].Exists(eCUSTOMSEMANTIC_FARCLIPDISTANCE))
		glUniform1f(m_pUniformMapping[iEffectId][eCUSTOMSEMANTIC_FARCLIPDISTANCE], m_fFarClipDistance);
		
	// Bind the vertex and index buffer for the point light
	const SPODMesh &Mesh = m_PointLightModel.pMesh[0];
	const unsigned int uiNumFaces = m_PointLightModel.pMesh[0].nNumFaces * 3;
	const GLenum datatype = (Mesh.sFaces.eType == EPODDataUnsignedShort) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

	glBindVertexArray(m_uiPointLightModelVAO);
	
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_uiLightEnvironmentMap);
			
	for (unsigned int i=0; i < m_uiNumPointLights; i++)
	{
		PVRTVec3 vLightIntensity = m_pPointLights[i].vColour * c_fPointlightIntensity;
		if (m_pUniformMapping[iEffectId].Exists(ePVRTPFX_UsLIGHTCOLOR))
			glUniform3fv(m_pUniformMapping[iEffectId][ePVRTPFX_UsLIGHTCOLOR], 1, vLightIntensity.ptr());

		const PVRTMat4 mWorldScale = m_pPointLights[i].mTransformation * m_pPointLights[i].mProxyScale;
		const PVRTMat4 mModelView = m_mView * mWorldScale;
		const PVRTMat4 mModelViewProj = m_mViewProjection * mWorldScale;
		PVRTMat3 mModelIT(m_pPointLights[i].mTransformation);
		mModelIT = mModelIT.inverse().transpose();

		glUniformMatrix4fv(m_pUniformMapping[iEffectId][ePVRTPFX_UsWORLDVIEWPROJECTION], 1, GL_FALSE, mModelViewProj.f);
		if (m_pUniformMapping[iEffectId].Exists(ePVRTPFX_UsWORLDVIEW))
			glUniformMatrix4fv(m_pUniformMapping[iEffectId][ePVRTPFX_UsWORLDVIEW], 1, GL_FALSE, mModelView.f);
		if (m_pUniformMapping[iEffectId].Exists(ePVRTPFX_UsWORLDIT))
			glUniformMatrix3fv(m_pUniformMapping[iEffectId][ePVRTPFX_UsWORLDIT], 1, GL_FALSE, mModelIT.f);

		const PVRTVec4 vLightPosView = m_mView * m_pPointLights[i].mTransformation * PVRTVec4(0.0f, 0.0f, 0.0f, 1.0);
		if (m_pUniformMapping[iEffectId].Exists(eCUSTOMSEMANTIC_POINTLIGHT_VIEWPOSITION))
			glUniform3fv(m_pUniformMapping[iEffectId][eCUSTOMSEMANTIC_POINTLIGHT_VIEWPOSITION], 1, &vLightPosView.x);				
				
		glDrawElements(GL_TRIANGLES, uiNumFaces, datatype, 0);		
	}
	
	glBindVertexArray(0);
}

/*!***************************************************************************
@Function		DrawDirectionalLightProxies
@Description	Renders all directional lights
*****************************************************************************/
void OGLES3DeferredShading::DrawDirectionalLightProxies()
{
	const int iEffectId = m_pPFXEffectParser->FindEffectByName(c_sDirectionalLightEffectName);
	m_ppPFXEffects[iEffectId]->Activate();	

	for (unsigned int i=0; i < m_uiNumDirectionalLights; i++)
	{
		PVRTVec3 vLightIntensity = m_pDirectionalLights[i].vColour * c_fDirectionallightIntensity;
		glUniform3fv(m_pUniformMapping[iEffectId][ePVRTPFX_UsLIGHTCOLOR], 1, vLightIntensity.ptr());

		const PVRTMat4 mModelView = m_mView * m_pDirectionalLights[i].mTransformation;
		const PVRTMat4 mModelViewProj = m_mViewProjection * m_pDirectionalLights[i].mTransformation;

		PVRTVec4 vLightDirView  = mModelView * m_pDirectionalLights[i].vDirection;
		glUniform4fv(m_pUniformMapping[iEffectId][eCUSTOMSEMANTIC_DIRECTIONALLIGHT_DIRECTION], 1, vLightDirView.ptr());

		DrawAxisAlignedQuad(PVRTVec2(-1.0f, -1.0f), PVRTVec2(1.0f, 1.0f));	
	}			
}


/*!****************************************************************************
 @Function		DrawAxisAlignedQuad
 @Input			afLowerLeft		Lower left corner of the quad in normalized device coordinates
                afUpperRight    Upper right corner of the quad in normalized device coordinates
 @Description	Draws a textured quad
******************************************************************************/
void OGLES3DeferredShading::DrawAxisAlignedQuad(PVRTVec2 afLowerLeft, PVRTVec2 afUpperRight)
{
	// Enable vertex attributes
	glEnableVertexAttribArray(VERTEX_ARRAY);
	glEnableVertexAttribArray(TEXCOORD_ARRAY);

	const float afVertexData[] = { afLowerLeft.x, afLowerLeft.y, 0.0f,  afUpperRight.x, afLowerLeft.y, 0.0f,
		                           afLowerLeft.x, afUpperRight.y, 0.0f,  afUpperRight.x, afUpperRight.y, 0.0f };
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, 0, afVertexData);

	const float afTexCoordData[] = { 0, 0,  1, 0,  0, 1,  1, 1 };
	glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, afTexCoordData);

	// Draw the quad
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// Disable vertex arributes
	glDisableVertexAttribArray(VERTEX_ARRAY);
	glDisableVertexAttribArray(TEXCOORD_ARRAY);
}


/*!****************************************************************************
 @Function		HandleInput
 @Description	Handles user input
******************************************************************************/
void OGLES3DeferredShading::HandleInput()
{
	// Handle input
	if (PVRShellIsKeyPressed(PVRShellKeyNameLEFT))
	{
		if (m_RenderMode == 0)
			m_RenderMode = NUM_RENDER_MODES - 1;
		else
			m_RenderMode--;
	}
	else if (PVRShellIsKeyPressed(PVRShellKeyNameRIGHT))
	{
		m_RenderMode = ++m_RenderMode % NUM_RENDER_MODES;
	}
	else if (PVRShellIsKeyPressed(PVRShellKeyNameUP))
	{
		m_uiCameraId = ++m_uiCameraId % m_Scene.pod.nNumCamera;
	}
	else if (PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
	{
		if (m_uiCameraId == 0)
			m_uiCameraId = m_Scene.pod.nNumCamera - 1;
		else
			m_uiCameraId--;
	}	
	else if (PVRShellIsKeyPressed(PVRShellKeyNameSELECT))
	{
		m_bPaused = !m_bPaused;
	}
}

/*!****************************************************************************
 @Function		UpdateAnimation
 @Description	Updates animation variables and camera matrices.
******************************************************************************/
void OGLES3DeferredShading::UpdateAnimation()
{
	static unsigned long ulPrevTime = PVRShellGetTime();
	unsigned long ulTime = PVRShellGetTime();
	unsigned long ulDeltaTime = ulTime - ulPrevTime;
	ulPrevTime = ulTime;
	if (!m_bPaused)
	{
		m_fFrame += ulDeltaTime * c_fDemoFrameRate;		
		if (m_fFrame > m_Scene.pod.nNumFrame - 1) m_fFrame = 0;		
	}
	
	m_Scene.pod.SetFrame(m_fFrame);

	//
	// Copy current frames light attributes
	//		
	for (unsigned int i=0; i < m_uiNumPointLights; i++)
	{	
		m_pPointLights[i].mTransformation = m_Scene.pod.GetWorldMatrix(m_Scene.pod.pNode[m_pPointLights[i].uiNodeIdx]);
		m_pPointLights[i].mProxyScale = PVRTMat4::Scale(c_fPointLightScale, c_fPointLightScale, c_fPointLightScale) * c_fPointlightIntensity;
	}
			
	for (unsigned int i=0; i < m_uiNumDirectionalLights; i++)
	{		
		m_pDirectionalLights[i].mTransformation = m_Scene.pod.GetWorldMatrix(m_Scene.pod.pNode[m_pDirectionalLights[i].uiNodeIdx]);
		m_pDirectionalLights[i].vDirection = m_pDirectionalLights[i].mTransformation * PVRTVec4(0.0f, -1.0f, 0.0f, 0.0);			
	}	

	PVRTVec3 vTo, vUp;
	m_Scene.pod.GetCamera(m_vCameraPosition, vTo, vUp, m_uiCameraId);
	float fNearClipDistance = m_Scene.pod.pCamera[m_uiCameraId].fNear;
	m_fFarClipDistance = m_Scene.pod.pCamera[m_uiCameraId].fFar;
	float fFieldOfView = m_Scene.pod.pCamera[m_uiCameraId].fFOV;
	
	//
	// Update camera matrices
	//
	m_mProjection = PVRTMat4::PerspectiveFovRH(fFieldOfView, m_iFboWidth / (float)m_iFboHeight, fNearClipDistance, m_fFarClipDistance, PVRTMat4::OGL, m_bScreenRotated);
	m_mView = PVRTMat4::LookAtRH(m_vCameraPosition, vTo, vUp);	
	m_mViewProjection = m_mProjection * m_mView;
	m_mInverseView = m_mView.inverse();	
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
	return new OGLES3DeferredShading();
}

/******************************************************************************
 End of file (OGLES3DeferredShading.cpp)
******************************************************************************/

