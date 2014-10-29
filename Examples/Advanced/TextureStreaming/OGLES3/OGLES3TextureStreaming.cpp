/*!****************************************************************************
 @File          OGLES3TextureStreaming.cpp

 @Title         Texture Streaming

 @Copyright     Copyright (C) by Imagination Technologies Limited.

 @Platform      Independent

 @Description   Demonstrates texture streaming using platform-specific functionality
******************************************************************************/
#include "PVRShell.h"
#include "OGLES3Tools.h"
#include "PVRTCameraInterface.h"

/******************************************************************************
 Shader attributes and uniforms
******************************************************************************/
// Vertex attributes
// We define an enum for the attribute position and an array of strings that
// correspond to the attribute names in the shader. These can be used by PVRTools
enum EVertexAttrib
{
	eVertexArray,
	eTexCoordArray,
	eNormalArray,

	eNumAttribs
};

enum EEffect
{
	eLit,
	eAmbient,
	eTVColour,
	eTVGreyscale,
	eTVNoise,

	eNumEffects
};

enum EHWCamera
{
	eHWCamFront,
	eHWCamBack
};

/****************************************************************************
 ** Structures
 ****************************************************************************/
// Group shader programs and their uniform locations together
struct SLitShader
{
	GLuint uiId;
	GLuint uiMVP;
	GLuint uiLightPosition;
	GLuint uiSampler;
};

struct SAmbientShader
{
	GLuint uiId;
	GLuint uiMVP;
	GLuint uiSampler;
};

struct STVColourShader
{
	GLuint uiId;
	GLuint uiMVP;
#if defined(__ANDROID__)
	GLuint uiVideoTexProjM;
	GLuint uiSampler;
#elif defined(__APPLE__)
	GLuint uiSamplerY;
	GLuint uiSamplerUV;
#endif
};

struct STVGreyscaleShader
{
	GLuint uiId;
	GLuint uiMVP;
#if defined(__ANDROID__)
	GLuint uiVideoTexProjM;
	GLuint uiSampler;
#elif defined(__APPLE__)
	GLuint uiSamplerY;
	GLuint uiSamplerUV;
#endif
};

struct STVNoiseShader
{
	GLuint uiId;
	GLuint uiMVP;
	GLuint uiScreenBand;
	GLuint uiNoiseLoc;
#if defined(__ANDROID__)
	GLuint uiVideoTexProjM;
	GLuint uiSampler;
#elif defined(__APPLE__)
	GLuint uiSamplerY;
	GLuint uiSamplerUV;
#endif
	GLuint uiSamplerNoise;
};

/******************************************************************************
 Consts
 ******************************************************************************/
// Camera constants. Used for making the projection matrix
const float c_fCameraNear      = 1.0f;
const float c_fCameraFar       = 15000.0f;

// The camera to use from the pod file
const int c_ui32Camera         = 0;

// Effect parameters
const float c_fBandWidth       = 0.025f;
const float c_fBandScrollSpeed = 0.01f;

// Descriptors
#if defined(__ANDROID__)
const char* c_pszDescription = "Android Surface Texture";
#elif defined(__APPLE__)
const char* c_pszDescription = "iOS CoreVideo";
#endif

const int     c_numTVScreens   = 6;  // Must match the number defined in the POD file.
const EEffect c_screenEffects[c_numTVScreens] =
{
	eTVColour,
	eTVGreyscale,
	eTVNoise,
	eTVNoise,
	eTVGreyscale,
	eTVColour
};

const char* c_pszLitDefines[] =
{
	"DIFFUSE",
#if defined(__ANDROID__)
	"ANDROID"
#endif
};

const char* c_pszTVGreyscaleDefines[] =
{
	"GREYSCALE",
#if defined(__ANDROID__)
	"ANDROID"
#endif
};

const char* c_pszTVNoiseDefines[] =
{
	"SCREEN_BANDS",
	"NOISE",
#if defined(__ANDROID__)
	"ANDROID"
#endif
};

const char* c_pszTVColourDefines[] =
{
#if defined(__ANDROID__)
	"ANDROID"
#endif
};

const char** c_pszEffectDefines[eNumEffects] =
{
	c_pszLitDefines,          // eLit
	NULL,                     // eAmbient
	c_pszTVColourDefines,     // eTVColour
	c_pszTVGreyscaleDefines,  // eTVGreyscale
	c_pszTVNoiseDefines       // eTVNoise
};

const int c_uiNumDefines[eNumEffects] =
{
	sizeof(c_pszLitDefines)         / sizeof(c_pszLitDefines[0]),          // eLit
	0,                                                                     // eAmbient
	sizeof(c_pszTVColourDefines)    / sizeof(c_pszTVColourDefines[0]),     // eTVColour
	sizeof(c_pszTVGreyscaleDefines) / sizeof(c_pszTVGreyscaleDefines[0]),  // eTVGreyscale
	sizeof(c_pszTVNoiseDefines)     / sizeof(c_pszTVNoiseDefines[0])       // eTVNoise
};

/******************************************************************************
 Content file names
 ******************************************************************************/
// Source shaders
const char* c_pszFragmentShaderSrc[eNumEffects] =
{
	"FragShader.fsh",    // eLit
	"FragShader.fsh",    // eAmbient
	"TVFragmentShader.fsh",      // eTVColour
	"TVFragmentShader.fsh",      // eTVGreyscal
	"TVFragmentShader.fsh"       // eTVNoise
};

const char* c_pszVertexShaderSrc[eNumEffects] =
{
	"VertShader.vsh",    // eLit
	"VertShader.vsh",    // eAmbient
	"TVVertexShader.vsh",      // eTVColour
	"TVVertexShader.vsh",      // eTVGreyscal
	"TVVertexShader.vsh"       // eTVNoise
};

// Texture files
const char* c_pszNoiseTexFile   = "rand.pvr";

// POD scene files
const char* c_pszSceneFile      = "tvscene.pod";

/*!****************************************************************************
 Class implementing the PVRShell functions.
 ******************************************************************************/
class OGLES3TextureStreaming : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D                m_Print3D;

	// 3D Model
	CPVRTModelPOD               m_Scene;

	// Camera interface
#if defined(__ANDROID__)
    CPVRTCameraInterfaceAndroid m_Camera;
#elif defined(__APPLE__)
	CPVRTCameraInterfaceiOS     m_Camera;
#endif

	// OpenGL handles for textures + VBOs
	GLuint*			            m_puiVbo;
	GLuint*                     m_puiIndexVbo;
	GLuint*                     m_puiTextureIDs;
	GLuint                      m_uiNoiseTex;

	// OpenGL handles for shaders
	GLuint                      m_uiVertexShaders[eNumEffects];
	GLuint                      m_uiFragmentShaders[eNumEffects];

	// Group shader programs and their uniform locations together
	SLitShader                  m_LitProgram;
	SAmbientShader              m_AmbientShaderProgram;
	STVColourShader             m_TVShaderProgram;
	STVGreyscaleShader          m_TVGreyscaleShaderProgram;
	STVNoiseShader              m_TVNoiseShaderProgram;

#if defined(__ANDROID__)
	PVRTMat4                    m_TexCoordsProjection;
#endif

	// App variables
	int		                    m_i32Frame;
	int		                    m_uiTVScreen;
	int                         m_uiRecordGlow;
	int                         m_iNoiseCoordIdx;
	bool                        m_bGlowState;
	unsigned long               m_ulGlowTime;

	// Variables to handle the animation in a time-based manner.
	unsigned long	            m_ulTimePrev;
	float			            m_fFrame;
	float                       m_fBandScroll;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	bool LoadTextures(CPVRTString* pErrorStr);
	bool LoadShaders(CPVRTString* pErrorStr);
	bool LoadVbos(CPVRTString* pErrorStr);

	void DrawPODScene(const PVRTMat4 &mViewProjection);
	void DrawMesh(int i32MeshIndex, bool bNormals);
};

/*!****************************************************************************
 @Function		LoadTextures
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course
 ******************************************************************************/
bool OGLES3TextureStreaming::LoadTextures(CPVRTString* pErrorStr)
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
		*pErrorStr = "ERROR: Insufficient memory.\n";
		return false;
	}

	for(unsigned int i = 0; i < m_Scene.nNumMaterial; ++i)
	{
		m_puiTextureIDs[i] = 0;
		SPODMaterial* pMaterial = &m_Scene.pMaterial[i];

		if(strcmp(pMaterial->pszName, "ScreenMat") == 0)
			m_uiTVScreen = i;
		else if(strcmp(pMaterial->pszName, "RecordGlow") == 0)
			m_uiRecordGlow = i;

		if(pMaterial->nIdxTexDiffuse != -1)
		{
			/*
			 Using the tools function PVRTTextureLoadFromPVR load the textures required by the pod file.

			 Note: This function only loads .pvr files. You can set the textures in 3D Studio Max to .pvr
			 files using the PVRTexTool plug-in for max. Alternatively, the pod material properties can be
			 modified in PVRShaman.
			 */

			const char* pszTexFile = m_Scene.pTexture[pMaterial->nIdxTexDiffuse].pszName;

			if(PVRTTextureLoadFromPVR(pszTexFile, &m_puiTextureIDs[i]) != PVR_SUCCESS)
			{
				*pErrorStr += PVRTStringFromFormattedStr("ERROR: Failed to load %s\n", pszTexFile);
				return false;
			}
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		}
	}

	// Load random texture
	if(PVRTTextureLoadFromPVR(c_pszNoiseTexFile, &m_uiNoiseTex) != PVR_SUCCESS)
	{
		*pErrorStr += "ERROR: Failed to load noise texture.\n";
		return false;
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return true;
}

/*!****************************************************************************
 @Function		LoadShaders
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occurred
 @Description	Loads and compiles the shaders and links the shader programs
                required for this training course
 ******************************************************************************/
bool OGLES3TextureStreaming::LoadShaders(CPVRTString* pErrorStr)
{
	/*
	 Load and compile the shaders from files.
	 */
	for(int idx = 0; idx < eNumEffects; ++idx)
	{
		if(PVRTShaderLoadFromFile(NULL,                         // Binary shader source file
								  c_pszVertexShaderSrc[idx],    // Text source file
								  GL_VERTEX_SHADER,             // Shader type
								  0,                            // Binary shader GL format
								  &m_uiVertexShaders[idx],      // Output GL handle
								  pErrorStr,                    // Output error string
								  NULL,                         // Context (not required for OGLES3)
								  c_pszEffectDefines[idx],      // Define array
								  c_uiNumDefines[idx]           // Number of defines in the array
								  ) != PVR_SUCCESS)
		{
			return false;
		}

		if(PVRTShaderLoadFromFile(NULL,                         // Binary shader source file
								  c_pszFragmentShaderSrc[idx],  // Text source file
								  GL_FRAGMENT_SHADER,           // Shader type
								  0,                            // Binary shader GL format
								  &m_uiFragmentShaders[idx],    // Output GL handle
								  pErrorStr,                    // Output error string
								  NULL,                         // Context (not required for OGLES3)
								  c_pszEffectDefines[idx],      // Define array
								  c_uiNumDefines[idx]           // Number of defines in the array
								  ) != PVR_SUCCESS)
		{
			return false;
		}
	}

	/*
	 Set up and link the shader program
	 */
	{
		// Lit shader
		const char* aszAttribs[] = { "inVertex", "inTexCoord", "inNormal" };
		if(PVRTCreateProgram(&m_LitProgram.uiId, m_uiVertexShaders[eLit], m_uiFragmentShaders[eLit],
							 aszAttribs, 3, pErrorStr) != PVR_SUCCESS)
		{
			return false;
		}

		// Store the location of uniforms for later use
		m_LitProgram.uiLightPosition = glGetUniformLocation(m_LitProgram.uiId, "vLightPosition");
		m_LitProgram.uiMVP           = glGetUniformLocation(m_LitProgram.uiId, "MVPMatrix");
		m_LitProgram.uiSampler       = glGetUniformLocation(m_LitProgram.uiId, "SamplerTexture");

		// Set the sampler2D variable to the first texture unit
		glUniform1i(m_LitProgram.uiSampler, 0);
	}
	{
		// Ambient shader
		const char* aszAttribs[] = { "inVertex", "inTexCoord", };
		if(PVRTCreateProgram(&m_AmbientShaderProgram.uiId, m_uiVertexShaders[eAmbient], m_uiFragmentShaders[eAmbient],
							 aszAttribs, 2, pErrorStr) != PVR_SUCCESS)
		{
			return false;
		}

		// Store the location of uniforms for later use
		m_AmbientShaderProgram.uiMVP           = glGetUniformLocation(m_AmbientShaderProgram.uiId, "MVPMatrix");
		m_AmbientShaderProgram.uiSampler       = glGetUniformLocation(m_AmbientShaderProgram.uiId, "SamplerTexture");

		// Set the sampler2D variable to the first texture unit
		glUniform1i(m_AmbientShaderProgram.uiSampler, 0);
	}
	{
		// TV colour shader
		const char* aszAttribs[] = { "inVertex", "inTexCoord" };
		if(PVRTCreateProgram(&m_TVShaderProgram.uiId, m_uiVertexShaders[eTVColour], m_uiFragmentShaders[eTVColour],
							 aszAttribs, 2, pErrorStr) != PVR_SUCCESS)
		{
			return false;
		}

		// Store the location of uniforms for later use
		m_TVShaderProgram.uiMVP       = glGetUniformLocation(m_TVShaderProgram.uiId, "MVPMatrix");
#if defined(__ANDROID__)
		m_TVShaderProgram.uiVideoTexProjM = glGetUniformLocation(m_TVShaderProgram.uiId, "TexSamplerPMatrix");
		m_TVShaderProgram.uiSampler       = glGetUniformLocation(m_TVShaderProgram.uiId, "Sampler");

		// Set the sampler2D variables
		glUniform1i(m_TVShaderProgram.uiSampler,  0);
#elif defined(__APPLE__)
		m_TVShaderProgram.uiSamplerUV = glGetUniformLocation(m_TVShaderProgram.uiId, "SamplerUV");
		m_TVShaderProgram.uiSamplerY  = glGetUniformLocation(m_TVShaderProgram.uiId, "SamplerY");

		// Set the sampler2D variables
		glUniform1i(m_TVShaderProgram.uiSamplerY,  0);
		glUniform1i(m_TVShaderProgram.uiSamplerUV, 1);
#endif
	}
	{
		// TV greyscale shader
		const char* aszAttribs[] = { "inVertex", "inTexCoord" };
		if(PVRTCreateProgram(&m_TVGreyscaleShaderProgram.uiId, m_uiVertexShaders[eTVGreyscale], m_uiFragmentShaders[eTVGreyscale],
							 aszAttribs, 2, pErrorStr) != PVR_SUCCESS)
		{
			return false;
		}

		// Store the location of uniforms for later use
		m_TVGreyscaleShaderProgram.uiMVP       = glGetUniformLocation(m_TVGreyscaleShaderProgram.uiId, "MVPMatrix");
#if defined(__ANDROID__)
		m_TVGreyscaleShaderProgram.uiVideoTexProjM = glGetUniformLocation(m_TVGreyscaleShaderProgram.uiId, "TexSamplerPMatrix");
		m_TVGreyscaleShaderProgram.uiSampler = glGetUniformLocation(m_TVGreyscaleShaderProgram.uiId, "Sampler");

		// Set the sampler2D variables
		glUniform1i(m_TVGreyscaleShaderProgram.uiSampler,  0);
#elif defined(__APPLE__)
		m_TVGreyscaleShaderProgram.uiSamplerUV = glGetUniformLocation(m_TVGreyscaleShaderProgram.uiId, "SamplerUV");
		m_TVGreyscaleShaderProgram.uiSamplerY  = glGetUniformLocation(m_TVGreyscaleShaderProgram.uiId, "SamplerY");

		// Set the sampler2D variables
		glUniform1i(m_TVGreyscaleShaderProgram.uiSamplerY,  0);
		glUniform1i(m_TVGreyscaleShaderProgram.uiSamplerUV, 1);
#endif
	}
	{
		// TV noise shader
		const char* aszAttribs[] = { "inVertex", "inTexCoord" };
		if(PVRTCreateProgram(&m_TVNoiseShaderProgram.uiId, m_uiVertexShaders[eTVNoise], m_uiFragmentShaders[eTVNoise],
							 aszAttribs, 2, pErrorStr) != PVR_SUCCESS)
		{
			return false;
		}

		// Store the location of uniforms for later use
		m_TVNoiseShaderProgram.uiMVP          = glGetUniformLocation(m_TVNoiseShaderProgram.uiId, "MVPMatrix");
		m_TVNoiseShaderProgram.uiScreenBand   = glGetUniformLocation(m_TVNoiseShaderProgram.uiId, "vScreenBand");
		m_TVNoiseShaderProgram.uiNoiseLoc     = glGetUniformLocation(m_TVNoiseShaderProgram.uiId, "vNoiseLoc");
		m_TVNoiseShaderProgram.uiSamplerNoise = glGetUniformLocation(m_TVNoiseShaderProgram.uiId, "SamplerNoise");

#if defined(__ANDROID__)
		m_TVNoiseShaderProgram.uiSampler       = glGetUniformLocation(m_TVNoiseShaderProgram.uiId, "Sampler");
		m_TVNoiseShaderProgram.uiVideoTexProjM = glGetUniformLocation(m_TVNoiseShaderProgram.uiId, "TexSamplerPMatrix");

		// Set the sampler2D variables
		glUniform1i(m_TVNoiseShaderProgram.uiSampler, 0);
#elif defined(__APPLE__)
		m_TVNoiseShaderProgram.uiSamplerUV    = glGetUniformLocation(m_TVNoiseShaderProgram.uiId, "SamplerUV");
		m_TVNoiseShaderProgram.uiSamplerY     = glGetUniformLocation(m_TVNoiseShaderProgram.uiId, "SamplerY");

		// Set the sampler2D variables
		glUniform1i(m_TVNoiseShaderProgram.uiSamplerY,  0);
		glUniform1i(m_TVNoiseShaderProgram.uiSamplerUV, 1);
#endif

		// Set the noise texture unit
		glUniform1i(m_TVNoiseShaderProgram.uiSamplerNoise, 2);
	}

	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Output		pErrorStr		A string describing the error on failure
 @Return        bool            True is successful
 @Description	Loads the mesh data required for this training course into
 vertex buffer objects
 ******************************************************************************/
bool OGLES3TextureStreaming::LoadVbos(CPVRTString* pErrorStr)
{
	if(!m_Scene.pMesh[0].pInterleaved)
	{
		*pErrorStr = "ERROR: This demo requires the pod data to be interleaved. Please re-export with the interleaved option enabled.\n";
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
bool OGLES3TextureStreaming::InitApplication()
{
	m_i32Frame       = 0;
	m_puiVbo         = 0;
	m_puiIndexVbo    = 0;
	m_ulGlowTime     = 0;
	m_iNoiseCoordIdx = 0;
	m_uiTVScreen     = -1;
	m_bGlowState     = false;

	/*
		CPVRTResourceFile is a resource file helper class. Resource files can
		be placed on disk next to the executable or in a platform dependent
		read path. We need to tell the class where that read path is.
		Additionally, it is possible to wrap files into cpp modules and
		link them directly into the executable. In this case no path will be
		used. Files on disk will override "memory files".
	*/

	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	// Load the scene
	if(m_Scene.ReadFromFile(c_pszSceneFile) != PVR_SUCCESS)
	{
		CPVRTString ErrorStr = "ERROR: Couldn't load '" + CPVRTString(c_pszSceneFile) + "'.\n";
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

	// Initialize variables used for the animation.
	m_fFrame      = 0.0f;
	m_fBandScroll = -c_fBandWidth;
	m_ulTimePrev = PVRShellGetTime();

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
bool OGLES3TextureStreaming::QuitApplication()
{
	// Free the memory allocated for the scene
	m_Scene.Destroy();

	delete[] m_puiVbo;
	delete[] m_puiIndexVbo;

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
bool OGLES3TextureStreaming::InitView()
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

	/*
		Initialize the textures used by Print3D.
		To properly display text, Print3D needs to know the viewport dimensions
		and whether the text should be rotated. We get the dimensions using the
		shell function PVRShellGet(prefWidth/prefHeight). We can also get the
		rotate parameter by checking prefIsRotated and prefFullScreen.
	*/
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	if(m_Print3D.SetTextures(0, PVRShellGet(prefWidth), PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	//Set OpenGL ES render states needed for this demo

	// Enable backface culling and depth test
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);

	// Sets the clear color
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	// Setup the AV capture
	if(!m_Camera.InitialiseSession(ePVRTHWCamera_Front))
		return false;

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES3TextureStreaming::ReleaseView()
{
	// Clean up AV capture
	m_Camera.DestroySession();

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

	// Frees the OpenGL handles for the program and the 2 shaders

	// Delete program and shader objects
	glDeleteProgram(m_LitProgram.uiId);
	glDeleteProgram(m_AmbientShaderProgram.uiId);
	glDeleteProgram(m_TVShaderProgram.uiId);
	glDeleteProgram(m_TVGreyscaleShaderProgram.uiId);
	glDeleteProgram(m_TVNoiseShaderProgram.uiId);

	for(int idx = 0; idx < eNumEffects; ++idx)
	{
		glDeleteShader(m_uiVertexShaders[idx]);
		glDeleteShader(m_uiFragmentShaders[idx]);
	}

	// Delete buffer objects
	glDeleteBuffers(m_Scene.nNumMesh, m_puiVbo);
	glDeleteBuffers(m_Scene.nNumMesh, m_puiIndexVbo);

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

	// Deletes the textures
	glDeleteTextures(m_Scene.nNumMaterial, &m_puiTextureIDs[0]);
	glDeleteTextures(1, &m_uiNoiseTex);

	// Frees the texture lookup array
	delete[] m_puiTextureIDs;
	m_puiTextureIDs = 0;

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
bool OGLES3TextureStreaming::RenderScene()
{
	// Clears the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Time based animation and locks the app to 60 FPS.
	// Uses the shell function PVRShellGetTime() to get the time in milliseconds.
	unsigned long ulTime = PVRShellGetTime();
	unsigned long ulDeltaTime = ulTime - m_ulTimePrev;
	m_ulTimePrev = ulTime;
	m_fFrame      += (float)ulDeltaTime * (60.0f/1000.0f);
	m_fBandScroll += (float)ulDeltaTime * (60.0f/1000.0f) * c_fBandScrollSpeed;
	if(m_fFrame > m_Scene.nNumFrame - 1)
		m_fFrame = 0.0f;

	if(m_fBandScroll > 1.0f)
		m_fBandScroll = -c_fBandWidth;

	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	m_Scene.SetFrame(m_fFrame);

	// Setup the main camera
	PVRTVec3	vFrom, vTo(0.0f), vUp(0.0f, 1.0f, 0.0f);
	float fFOV;

	// Camera nodes are after the mesh and light nodes in the array
	int i32CamID = m_Scene.pNode[m_Scene.nNumMeshNode + m_Scene.nNumLight + c_ui32Camera].nIdx;

	// Get the camera position, target and field of view (fov)
	if(m_Scene.pCamera[i32CamID].nIdxTarget != -1) // Does the camera have a target?
		fFOV = m_Scene.GetCameraPos( vFrom, vTo, c_ui32Camera); // vTo is taken from the target node
	else
		fFOV = m_Scene.GetCamera( vFrom, vTo, vUp, c_ui32Camera); // vTo is calculated from the rotation

    float fTargetAspect = 960.0f/640.0f;
    float fAspect       = (float)PVRShellGet(prefWidth) / (float)PVRShellGet(prefHeight);
    fFOV               *= fTargetAspect / fAspect;

	PVRTMat4 mView           = PVRTMat4::LookAtRH(vFrom, vTo, vUp);
	PVRTMat4 mProjection     = PVRTMat4::PerspectiveFovRH(fFOV, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), c_fCameraNear,
														  c_fCameraFar, PVRTMat4::OGL, bRotate);
	PVRTMat4 mViewProjection = mProjection * mView;

	DrawPODScene(mViewProjection);

	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("Texture Streaming", c_pszDescription, ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	++m_i32Frame;
	return true;
}

/*!***************************************************************************
 @Function		DrawPODScene
 @Input         mViewProjection
 @Input         bDrawCamera
 @Description   Draws the scene described by the loaded POD file.
 *****************************************************************************/
void OGLES3TextureStreaming::DrawPODScene(const PVRTMat4 &mViewProjection)
{
	// Clear the colour and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Get the position of the first light from the scene.
	PVRTVec4 vLightPosition = m_Scene.GetLightPosition(0);
	int iTVCount            = 0;

#if defined(__ANDROID__)
	// Check if the MVP has changed
	if (m_Camera.HasImageChanged() && m_Camera.HasProjectionMatrixChanged())
	{
		m_TexCoordsProjection = PVRTMat4(m_Camera.GetProjectionMatrix());
	}
#endif

	for(unsigned int i = 0; i < m_Scene.nNumMeshNode; ++i)
	{
		SPODNode& Node = m_Scene.pNode[i];

		bool bIsTVScreen   = Node.nIdxMaterial == m_uiTVScreen;
		bool bIsRecordGlow = Node.nIdxMaterial == m_uiRecordGlow;

		// Get the node model matrix
		PVRTMat4 mWorld = m_Scene.GetWorldMatrix(Node);
		PVRTMat4 mModelView, mMVP;
		mMVP = mViewProjection * mWorld;

		GLint iMVPLoc = -1;
#if defined(__ANDROID__)
		GLint iTexProjLoc = -1;
#endif
		if(bIsTVScreen) // If we're drawing the TV screen change to the correct TV shader
		{
			_ASSERT(iTVCount < c_numTVScreens);
			if(c_screenEffects[iTVCount] == eTVNoise)
			{
				glUseProgram(m_TVNoiseShaderProgram.uiId);
				iMVPLoc = m_TVNoiseShaderProgram.uiMVP;
#if defined(__ANDROID__)
				iTexProjLoc = m_TVNoiseShaderProgram.uiVideoTexProjM;
#endif

				// Do the screen scrolling
				float fBandY1 = m_fBandScroll;
				float fBandY2 = fBandY1 + c_fBandWidth;
				glUniform2f(m_TVNoiseShaderProgram.uiScreenBand, fBandY1, fBandY2);

				// Do the noise
				PVRTVec2 vNoiseCoords;
				vNoiseCoords.x = (m_iNoiseCoordIdx % 4) * 0.25f;
				vNoiseCoords.y = (m_iNoiseCoordIdx / 4) * 0.25f;

				// Set the texmod value
				glUniform2f(m_TVNoiseShaderProgram.uiNoiseLoc, vNoiseCoords.x, vNoiseCoords.y);

				// Increment and reset
				m_iNoiseCoordIdx++;
				if(m_iNoiseCoordIdx >= 16)
					m_iNoiseCoordIdx = 0;
			}
			else if(c_screenEffects[iTVCount] == eTVGreyscale)
			{
				glUseProgram(m_TVGreyscaleShaderProgram.uiId);
				iMVPLoc = m_TVGreyscaleShaderProgram.uiMVP;
#if defined(__ANDROID__)
				iTexProjLoc = m_TVGreyscaleShaderProgram.uiVideoTexProjM;
#endif
			}
			else if(c_screenEffects[iTVCount] == eTVColour)
			{
				glUseProgram(m_TVShaderProgram.uiId);
				iMVPLoc = m_TVShaderProgram.uiMVP;
#if defined(__ANDROID__)
				iTexProjLoc = m_TVShaderProgram.uiVideoTexProjM;
#endif
			}
			else
			{
				_ASSERT(false); // Invalid enum
			}
			iTVCount++;
		}
		else if(bIsRecordGlow)
		{
			// Should the glow be active?
			unsigned long ulNow = PVRShellGetTime();
			if(ulNow - m_ulGlowTime > 1000)
			{
				m_bGlowState = !m_bGlowState;
				m_ulGlowTime = ulNow;
			}

			if(!m_bGlowState)
				continue;

			glEnable(GL_BLEND);
			glUseProgram(m_AmbientShaderProgram.uiId);
			iMVPLoc = m_AmbientShaderProgram.uiMVP;
		}
		else
		{
			glUseProgram(m_LitProgram.uiId);
			iMVPLoc = m_LitProgram.uiMVP;
		}

		glUniformMatrix4fv(iMVPLoc, 1, GL_FALSE, mMVP.f);

		// Pass the light position in model space to the shader. Don't do this for the TV screen.
		if(!bIsTVScreen && !bIsRecordGlow)
		{
			PVRTVec4 vLightPos;
			vLightPos = mWorld.inverse() * vLightPosition;

			glUniform3fv(m_LitProgram.uiLightPosition, 1, &vLightPos.x);
		}

		// Bind the correct texture
		if(Node.nIdxMaterial != -1)
		{
            if(Node.nIdxMaterial == m_uiTVScreen && m_i32Frame != 0)
			{
#if defined(__ANDROID__)
				GLuint yuvTexture = m_Camera.GetYUVTexture();

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_EXTERNAL_OES, yuvTexture);

				// Set the sampler projection
				glUniformMatrix4fv(iTexProjLoc, 1, GL_FALSE, m_TexCoordsProjection.f);
#elif defined(__APPLE__)
				GLuint lumaTexure    = m_Camera.GetLuminanceTexture();
				GLuint chromaTexture = m_Camera.GetChrominanceTexture();
				GLenum lumaTarget    = m_Camera.GetLuminanceTextureTarget();
				GLenum chromaTarget  = m_Camera.GetChrominanceTextureTarget();

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(lumaTarget, lumaTexure);

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(chromaTarget, chromaTexture);
#endif

				if(c_screenEffects[iTVCount] == eTVNoise)
				{
					// Bind the noise texture
					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_2D, m_uiNoiseTex);
				}
			}
			else
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, m_puiTextureIDs[Node.nIdxMaterial]);
			}
		}
		else
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		/*
		 Now that the model-view matrix is set and the materials ready,
		 call another function to actually draw the mesh.
		 */
		DrawMesh(Node.nIdx, (bIsTVScreen || bIsRecordGlow) ? false : true);

		if(bIsRecordGlow)
		{
			glDisable(GL_BLEND);
		}
	}
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			i32MeshIndex		Mesh index of the mesh to draw
 @Input         bNormals            Whether to enable normals
 @Description	Draws a SPODMesh after the model view matrix has been set and
                the material prepared.
 ******************************************************************************/
void OGLES3TextureStreaming::DrawMesh(int i32MeshIndex, bool bNormals)
{
	SPODMesh& Mesh = m_Scene.pMesh[i32MeshIndex];

	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[i32MeshIndex]);

	// bind the index buffer, won't hurt if the handle is 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[i32MeshIndex]);

	// Set the vertex attribute offsets
	glEnableVertexAttribArray(eVertexArray);
	glVertexAttribPointer(eVertexArray, 3, GL_FLOAT, GL_FALSE, Mesh.sVertex.nStride, Mesh.sVertex.pData);

	if(bNormals)
	{
		glEnableVertexAttribArray(eNormalArray);
		glVertexAttribPointer(eNormalArray, 3, GL_FLOAT, GL_FALSE, Mesh.sNormals.nStride, Mesh.sNormals.pData);
	}

	if(Mesh.nNumUVW) // Do we have texture co-ordinates?
	{
		glEnableVertexAttribArray(eTexCoordArray);
		glVertexAttribPointer(eTexCoordArray, 2, GL_FLOAT, GL_FALSE, Mesh.psUVW[0].nStride, Mesh.psUVW[0].pData);
	}

	// Draw the Indexed Triangle list
	glDrawElements(GL_TRIANGLES, Mesh.nNumFaces*3, GL_UNSIGNED_SHORT, 0);

	// Disable the uv attribute array
	glDisableVertexAttribArray(eVertexArray);
	glDisableVertexAttribArray(eNormalArray);
	glDisableVertexAttribArray(eTexCoordArray);

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
	return new OGLES3TextureStreaming();
}

/******************************************************************************
 End of file (OGLES3TextureStreaming.cpp)
******************************************************************************/
