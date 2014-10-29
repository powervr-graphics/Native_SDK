/******************************************************************************

 @File         OGLES2ShadowMapping.cpp

 @Title        Shadow mapping

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows shadow mapping

******************************************************************************/
#include <string.h>

#include "PVRShell.h"
#include "OGLES2Tools.h"

/******************************************************************************
 Defines
******************************************************************************/
// Index to bind the attributes to vertex shaders
#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY	2

#ifndef GL_TEXTURE_COMPARE_MODE_EXT    
#   define GL_TEXTURE_COMPARE_MODE_EXT      0x884C
#endif
#ifndef GL_TEXTURE_COMPARE_FUNC_EXT
#   define GL_TEXTURE_COMPARE_FUNC_EXT      0x884D
#endif
#ifndef GL_COMPARE_REF_TO_TEXTURE_EXT
#   define GL_COMPARE_REF_TO_TEXTURE_EXT    0x884E
#endif

/******************************************************************************
 Consts
******************************************************************************/
// Camera constants. Used for making the projection matrix
const float g_fCameraNear = 5.0f;
const float g_fCameraFar  = 400.0f;

// Const for the shadow map texture size
const unsigned int m_ui32ShadowMapSize = 512;

/******************************************************************************
 Content file names
******************************************************************************/

// Source and binary shaders
const char c_szFragShaderSrcFile[]	= "FragShader.fsh";
const char c_szFragShaderBinFile[]	= "FragShader.fsc";
const char c_szVertShaderSrcFile[]	= "VertShader.vsh";
const char c_szVertShaderBinFile[]	= "VertShader.vsc";
const char c_szShadowMapppingFragSrcFile[]	= "ShadowFragShader.fsh";
const char c_szShadowMapppingFragBinFile[]	= "ShadowFragShader.fsc";
const char c_szShadowMapppingVertSrcFile[]	= "ShadowVertShader.vsh";
const char c_szShadowMapppingVertBinFile[]	= "ShadowVertShader.vsc";

// POD scene files
const char c_szSceneFile[]			= "Scene.pod";
const char c_szMaskTex[]			= "Mask.pvr";
const char c_szTableCoverTex[]		= "TableCover.pvr";
const char c_szTorusTex[]			= "Torus.pvr";

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES2ShadowMapping : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// 3D Model
	CPVRTModelPOD	m_Scene;

	// OpenGL handles for shaders, textures and VBOs
	GLuint m_uiSimpleVertShader;
	GLuint m_uiSimpleFragShader;
	GLuint m_uiShadowVertShader;
	GLuint m_uiShadowFragShader;
	GLuint* m_puiVbo;
	GLuint* m_puiIndexVbo;
	GLuint* m_puiTextureIDs;
	GLuint m_uiMask;
	GLuint m_uiTableCover;
	GLuint m_uiTorus;
	GLuint m_uiShadowMapTexture;
	GLuint m_uiFrameBufferObject;
    GLint m_i32OriginalFbo;

	float m_fLightDistance;
	float m_fLightAngle;
	// Group shader programs and their uniform locations together
	struct
	{
		GLuint uiId;
		GLuint uiLightDirLoc;
		GLuint uiProjectionMatrixLoc;
		GLuint uiTexProjMatrixLoc;
		GLuint uiModelViewMatrixLoc;
	}
	m_ShadowShaderProgram;

	struct
	{
		GLuint uiId;
		GLuint uiModelViewMatrixLoc;
		GLuint uiProjectionMatrixLoc;
	}
	m_SimpleShaderProgram;

	PVRTVec4 m_vLightDirection;
	PVRTVec3 m_vLightPosition;

	PVRTMat4 m_View, m_Projection;
	PVRTMat4 m_LightProjection, m_LightView;
	PVRTMat4 m_BiasMatrix;

	// Screen orientation variable
	bool m_bRotate;
    
    // Program options
    bool m_bUseShadowSamplerExt;

	// Framebuffer extension flag.
	bool m_bDiscard;

	CPVRTgles2Ext	m_Extensions;	// GL Extensions Class

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	bool LoadTextures(CPVRTString* pErrorStr);
	bool LoadShaders(CPVRTString* pErrorStr);
	bool LoadVbos(CPVRTString* pErrorStr);
	void SetUpMatrices();
	void DrawSceneWithShadow(PVRTMat4 viewMat);
	void DrawMesh(int i32NodeIndex);
	void RenderWorld();
};

/*!****************************************************************************
 @Function		LoadTextures
 @Output		pErrorStr		A CPVRTString describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course
******************************************************************************/
bool OGLES2ShadowMapping::LoadTextures(CPVRTString* const pErrorStr)
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

	if(PVRTTextureLoadFromPVR(c_szTableCoverTex, &m_uiTableCover) != PVR_SUCCESS)
		return false;

	if(PVRTTextureLoadFromPVR(c_szTorusTex, &m_uiTorus) != PVR_SUCCESS)
		return false;

	if(PVRTTextureLoadFromPVR(c_szMaskTex, &m_uiMask) != PVR_SUCCESS)
		return false;

	for(unsigned int i = 0; i < m_Scene.nNumMaterial; ++i)
	{
		m_puiTextureIDs[i] = 0;
		SPODMaterial* pMaterial = &m_Scene.pMaterial[i];

		if(!strcmp(pMaterial->pszName, "Material #1"))
			m_puiTextureIDs[i] = m_uiTableCover;
		else if(!strcmp(pMaterial->pszName, "Material #2"))
			m_puiTextureIDs[i] = m_uiTorus;
		else
			m_puiTextureIDs[i] = m_uiMask;
	}

	//Create the shadow map texture
	glGenTextures(1, &m_uiShadowMapTexture);
	glBindTexture(GL_TEXTURE_2D, m_uiShadowMapTexture);

	// Create the depth texture.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_ui32ShadowMapSize, m_ui32ShadowMapSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

	// Set the textures parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if(m_bUseShadowSamplerExt)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_EXT, GL_COMPARE_REF_TO_TEXTURE_EXT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_EXT, GL_LEQUAL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
        
        
	return true;
}

/*!****************************************************************************
 @Function		SetUpMatrices
 @Description	Creates the view and projection matrices for the light and camera
******************************************************************************/
void OGLES2ShadowMapping::SetUpMatrices()
{
	PVRTVec3 vFrom = PVRTVec3(-140.0f, 130.0f, -140.0f ),
			vTo = PVRTVec3( 0, 10, 0 ),
			vUp = PVRTVec3( 0, 1, 0 );

	float fFOV = 0.78539819f;

	m_BiasMatrix = PVRTMat4(0.5f, 0.0f, 0.0f, 0.0f,
							0.0f, 0.5f, 0.0f, 0.0f,
							0.0f, 0.0f, 0.5f, 0.0f,
							0.5f, 0.5f, 0.5f, 1.0f);

	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	m_Projection = PVRTMat4::PerspectiveFovRH(fFOV, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), g_fCameraNear, g_fCameraFar, PVRTMat4::OGL, bRotate);

	m_View = PVRTMat4::LookAtRH(vFrom, vTo, vUp);

	m_LightProjection = PVRTMat4::PerspectiveFovRH(fFOV, 1.0f, 70.0f, 270.0f, PVRTMat4::OGL, m_bRotate);

	m_LightView = PVRTMat4::LookAtRH(m_vLightPosition, vTo, vUp);

}
/*!****************************************************************************
 @Function		LoadShaders
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads and compiles the shaders and links the shader programs
				required for this training course
******************************************************************************/
bool OGLES2ShadowMapping::LoadShaders(CPVRTString* pErrorStr)
{
	const char* aszAttribs[] = { "inVertex", "inNormal", "inTexCoord" };

	/*
		Load and compile the shaders from files.
		Binary shaders are tried first, source shaders
		are used as fallback.
	*/

	if(PVRTShaderLoadFromFile(
			c_szVertShaderBinFile, c_szVertShaderSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiSimpleVertShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	if(PVRTShaderLoadFromFile(
			c_szFragShaderBinFile, c_szFragShaderSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_uiSimpleFragShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	if(PVRTCreateProgram(&m_SimpleShaderProgram.uiId, m_uiSimpleVertShader, m_uiSimpleFragShader, aszAttribs, 3, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	m_SimpleShaderProgram.uiModelViewMatrixLoc	= glGetUniformLocation(m_SimpleShaderProgram.uiId, "ModelViewMatrix");
	m_SimpleShaderProgram.uiProjectionMatrixLoc	= glGetUniformLocation(m_SimpleShaderProgram.uiId, "ProjectionMatrix");
    
    const char* pszDefines = "";
    int iNumDefines        = 0;
    if(m_bUseShadowSamplerExt)
    {
        pszDefines  = "USE_SHADOW_SAMPLERS";
        iNumDefines = 1;
    }
    
    if(PVRTShaderLoadFromFile(
			c_szShadowMapppingVertBinFile, c_szShadowMapppingVertSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiShadowVertShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	if(PVRTShaderLoadFromFile(
			c_szShadowMapppingFragBinFile, c_szShadowMapppingFragSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_uiShadowFragShader, pErrorStr, NULL, &pszDefines, iNumDefines) != PVR_SUCCESS)
	{
		return false;
	}

 	if(PVRTCreateProgram(&m_ShadowShaderProgram.uiId, m_uiShadowVertShader, m_uiShadowFragShader, aszAttribs, 3, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	m_ShadowShaderProgram.uiTexProjMatrixLoc	= glGetUniformLocation(m_ShadowShaderProgram.uiId, "TexProjectionMatrix");
	m_ShadowShaderProgram.uiModelViewMatrixLoc	= glGetUniformLocation(m_ShadowShaderProgram.uiId, "ModelViewMatrix");
	m_ShadowShaderProgram.uiProjectionMatrixLoc	= glGetUniformLocation(m_ShadowShaderProgram.uiId, "ProjectionMatrix");
	m_ShadowShaderProgram.uiLightDirLoc	= glGetUniformLocation(m_ShadowShaderProgram.uiId, "LightDirection");

	glUniform1i(glGetUniformLocation(m_ShadowShaderProgram.uiId, "sShadow"), 0);
	glUniform1i(glGetUniformLocation(m_ShadowShaderProgram.uiId, "sTexture"), 1);
	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
bool OGLES2ShadowMapping::LoadVbos(CPVRTString* pErrorStr)
{
	if(!m_Scene.pMesh[0].pInterleaved)
{
		*pErrorStr = "ERROR: IntroducingPOD requires the pod data to be interleaved. Please re-export with the interleaved option enabled.";
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
 @Return		bool		true if no error occured
 @Description	Code in InitApplication() will be called by PVRShell once per
				run, before the rendering context is created.
				Used to initialize variables that are not dependant on it
				(e.g. external modules, loading meshes, etc.)
				If the rendering context is lost, InitApplication() will
				not be called again.
******************************************************************************/
bool OGLES2ShadowMapping::InitApplication()
{
	m_puiVbo = 0;
	m_puiIndexVbo = 0;
	m_puiTextureIDs = 0;
    m_i32OriginalFbo = 0;

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

	m_vLightPosition.x = 0.f;
	m_vLightPosition.y = 90.f;
	m_vLightPosition.z = 0.f;

	m_vLightDirection.x = -m_vLightPosition.x;
	m_vLightDirection.y = -m_vLightPosition.y;
	m_vLightDirection.z = -m_vLightPosition.z;
	m_vLightDirection.w = 1.0f;

	// Specify the light distance from origin. This should be at a distance to fit everything into the viewport
	// when rendering from the lights POV.
	m_fLightDistance = 130.0f;
	m_fLightAngle = PVRT_PI;

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
bool OGLES2ShadowMapping::QuitApplication()
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
bool OGLES2ShadowMapping::InitView()
{
	CPVRTString ErrorStr;

	if(!CPVRTgles2Ext::IsGLExtensionSupported("GL_OES_depth_texture"))
	{
		PVRShellSet(prefExitMessage, "Error: Unable to run this training course as it requires extension 'GL_OES_depth_texture'");
		return false;
	}
    
    m_bUseShadowSamplerExt = false;
    // Check if GL_EXT_shadow_samplers is supported so we can remove some work from the fragment shader.
    if(CPVRTgles2Ext::IsGLExtensionSupported("GL_EXT_shadow_samplers"))
    {
        m_bUseShadowSamplerExt = true;
    }
    
    // Get the original framebuffer object handle.
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_i32OriginalFbo);
    
	m_bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	// Initialize VBO data
	if(!LoadVbos(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	//	Load textures
	if (!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	//	Load and compile the shaders & link programs
	if (!LoadShaders(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Create a frame buffer with only the depth buffer attached
	glGenFramebuffers(1, &m_uiFrameBufferObject);
	glBindFramebuffer(GL_FRAMEBUFFER, m_uiFrameBufferObject);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_uiShadowMapTexture, 0);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		PVRShellSet(prefExitMessage, "ERROR: Frame buffer not set up correctly\n");
		return false;

	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_i32OriginalFbo);

	//	Initialize Print3D
	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight),m_bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	// Use a nice bright blue as clear colour
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
    
    // Enable culling so we can control how the shadow geometry is drawn
    if(m_bUseShadowSamplerExt)
        glEnable(GL_CULL_FACE);

	// Check to see if the GL_EXT_discard_framebuffer extension is supported
	m_bDiscard = CPVRTgles2Ext::IsGLExtensionSupported("GL_EXT_discard_framebuffer");
	if(m_bDiscard)
	{
		m_Extensions.LoadExtensions();
		m_bDiscard = m_Extensions.glDiscardFramebufferEXT != 0;
	}

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES2ShadowMapping::ReleaseView()
{
	// Deletes the textures
	glDeleteTextures(m_Scene.nNumMaterial, &m_puiTextureIDs[0]);

	// Frees the texture lookup array
	delete[] m_puiTextureIDs;
	m_puiTextureIDs = 0;

	// Delete program and shader objects
	glDeleteProgram(m_ShadowShaderProgram.uiId);

	glDeleteShader(m_uiShadowVertShader);
	glDeleteShader(m_uiShadowFragShader);

	// Delete program and shader objects
	glDeleteProgram(m_SimpleShaderProgram.uiId);

	glDeleteShader(m_uiSimpleVertShader);
	glDeleteShader(m_uiSimpleFragShader);
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
bool OGLES2ShadowMapping::RenderScene()
{
	//rotate light position
	m_fLightAngle += 0.01f;
	m_vLightPosition.x = m_fLightDistance * (float) cos(m_fLightAngle);
	m_vLightPosition.z = m_fLightDistance * (float) sin(m_fLightAngle);
	m_vLightDirection.x = -m_vLightPosition.x;
	m_vLightDirection.z = -m_vLightPosition.z;

	SetUpMatrices();

	glEnable(GL_DEPTH_TEST);

	// Bind the frame buffer object
	glBindFramebuffer(GL_FRAMEBUFFER, m_uiFrameBufferObject);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
	{
		// Clear the screen and depth buffer so we can render from the light's view
		glClear(GL_DEPTH_BUFFER_BIT);

        // Set the current viewport to our texture size but leave a one pixel margin.
        // As we are clamping to the edge of the texture when shadow mapping, no object
        // should be rendered to the border, otherwise stretching artefacts might occur
        // outside of the coverage of the shadow map.
        glViewport(1, 1, m_ui32ShadowMapSize-2, m_ui32ShadowMapSize-2);

		// Since we don't care about colour when rendering the depth values to
		// the shadow-map texture, we disable color writing to increase speed.
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        
        // Cull the front faces, so that only the backfaces are rendered into the shadowmap
        if(m_bUseShadowSamplerExt)
            glCullFace(GL_FRONT);

		// Enable the simple shader for the light view pass. This render will not be shown to the user
		// so only the simplest render needs to be implemented
		glUseProgram(m_SimpleShaderProgram.uiId);

		// Set the light projection matrix
		glUniformMatrix4fv(m_SimpleShaderProgram.uiProjectionMatrixLoc, 1, GL_FALSE, m_LightProjection.f);

		// Draw everything that we would like to cast a shadow
		for(unsigned int i = 2; i < m_Scene.nNumMeshNode; ++i)
		{
			SPODNode& Node = m_Scene.pNode[i];

			PVRTMat4 mWorld, mModelView;

			m_Scene.GetWorldMatrix(mWorld, Node);

			PVRTMatrixMultiply(mModelView, mWorld, m_LightView);

			glUniformMatrix4fv(m_SimpleShaderProgram.uiModelViewMatrixLoc, 1, GL_FALSE, mModelView.f);

			DrawMesh(i);
		}
        
        // Set the culling mode for the normal rendering
        if(m_bUseShadowSamplerExt)
            glCullFace(GL_BACK);

		// We can turn color writing back on since we already stored the depth values
#if defined(__PALMPDK__)
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE); // The alpha part is false as we don't want to blend with the video layer on the Palm Pre
#else
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
#endif

		// Restore our normal viewport size to our screen width and height
		glViewport(0, 0,PVRShellGet(prefWidth),PVRShellGet(prefHeight));

		if(m_bDiscard) // Was GL_EXT_discard_framebuffer supported?
		{
			//Give the drivers a hint that we don't want stencil or depth information to be stored for later.
			const GLenum attachment = GL_COLOR_ATTACHMENT0;
			m_Extensions.glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, &attachment);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_i32OriginalFbo);

	// Clear the colour and depth buffers, we are now going to render the scene again from scratch
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Load the shadow shader. This shader requires additional parameters; texProjMatrix for the depth buffer
	// look up and the light direction for diffuse light (the effect is a lot nicer with the additon of the
	// diffuse light).
	glUseProgram(m_ShadowShaderProgram.uiId);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uiShadowMapTexture);

	glUniformMatrix4fv(m_ShadowShaderProgram.uiProjectionMatrixLoc, 1, GL_FALSE, m_Projection.f);

	PVRTMat4 mViewInv, mTextureMatrix, mMatrix;
	mViewInv = m_View.inverse();

	// We need to calculate the texture projection matrix. This matrix takes the pixels from world space to previously rendered light projection space
	//where we can look up values from our saved depth buffer. The matrix is constructed from the light view and projection matrices as used for the previous render and
	//then multiplied by the inverse of the current view matrix.
	mTextureMatrix = m_BiasMatrix * m_LightProjection *  m_LightView * mViewInv;

	glUniformMatrix4fv(m_ShadowShaderProgram.uiTexProjMatrixLoc, 1, GL_FALSE, mTextureMatrix.f);

	DrawSceneWithShadow(m_View);

	// Re-enable the simple shader to draw the light source object
	glUseProgram(m_SimpleShaderProgram.uiId);

	SPODNode& Node = m_Scene.pNode[1];

	PVRTMat4 mWorld, mModelView;

	m_Scene.GetWorldMatrix(mWorld, Node);

	mWorld.f[12] = m_vLightPosition.x;
	mWorld.f[13] = m_vLightPosition.y;
	mWorld.f[14] = m_vLightPosition.z;

	mModelView = m_View * mWorld;

	glUniformMatrix4fv(m_SimpleShaderProgram.uiModelViewMatrixLoc, 1, GL_FALSE, mModelView.f);
	glUniformMatrix4fv(m_SimpleShaderProgram.uiProjectionMatrixLoc, 1, GL_FALSE, m_LightProjection.f);

	DrawMesh(1);

	m_Print3D.DisplayDefaultTitle("ShadowMap", "", ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		DrawSceneWithShadow
 @Input			viewMat The view matrix to use for rendering
 @Description	Draws the scene with the shadow
******************************************************************************/
void OGLES2ShadowMapping::DrawSceneWithShadow(PVRTMat4 viewMat)
{
	for (unsigned int i = 0; i < m_Scene.nNumMeshNode; ++i)
	{
		if(i == 1) continue;

		SPODNode& Node = m_Scene.pNode[i];

		PVRTMat4 mWorld, mModelView;
		m_Scene.GetWorldMatrix(mWorld, Node);
            
        PVRTMatrixMultiply(mModelView, mWorld, viewMat);
        
        // Add on a small bias if we're using the shadow sampler extension.
        if(m_bUseShadowSamplerExt)
            mModelView *= PVRTMat4::Translation(0.0f, 0.0f, 0.5f);

		glUniformMatrix4fv(m_ShadowShaderProgram.uiModelViewMatrixLoc, 1, GL_FALSE, mModelView.f);

		// Calculate the light direction for the diffuse lighting
		PVRTVec4 vLightDir;
		PVRTTransformBack(&vLightDir, &m_vLightDirection, &mWorld);
		PVRTVec3 vLightDirModel = *(PVRTVec3*)&vLightDir;
		PVRTMatrixVec3Normalize(vLightDirModel, vLightDirModel);
		glUniform3fv(m_ShadowShaderProgram.uiLightDirLoc, 1, &vLightDirModel.x);

		// Load the correct texture using our texture lookup table
		GLuint uiTex = 0;

		if (Node.nIdxMaterial != -1)
			uiTex = m_puiTextureIDs[Node.nIdxMaterial];

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, uiTex);

		DrawMesh(i);
	}
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			i32NodeIndex		Node index of the mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the meterial prepared.
******************************************************************************/
void OGLES2ShadowMapping::DrawMesh(int i32NodeIndex)
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
				glDrawElements(GL_TRIANGLE_STRIP, pMesh->pnStripLength[i]+2, GL_UNSIGNED_SHORT, &((GLshort*)0)[offset]);
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
	return new OGLES2ShadowMapping();
}

/******************************************************************************
 End of file (OGLES2ShadowMapping.cpp)
******************************************************************************/

