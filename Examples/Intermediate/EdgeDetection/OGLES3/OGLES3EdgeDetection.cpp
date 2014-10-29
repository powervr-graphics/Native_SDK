/******************************************************************************

 @File         OGLES3EdgeDetection.cpp

 @Title        EdgeDetection

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Specifically this program shows how to apply a sketched look using
               shaders, however it is a good base with some general ideas for any
               post-processing techniques.

******************************************************************************/
#include "PVRShell.h"
#include "OGLES3Tools.h"

/******************************************************************************
 Content file names
******************************************************************************/
// Source and binary shader filenames
const char c_szPreFragShaderSrc[]	= "PreFragShader.fsh";
const char c_szPreFragShaderBin[]	= "PreFragShader.fsc";
const char c_szPreVertShaderSrc[]	= "PreVertShader.vsh";
const char c_szPreVertShaderBin[]	= "PreVertShader.vsc";
const char c_szPostFragShaderSrc[]	= "PostFragShader.fsh";
const char c_szPostFragShaderBin[]	= "PostFragShader.fsc";
const char c_szPostVertShaderSrc[]	= "PostVertShader.vsh";
const char c_szPostVertShaderBin[]	= "PostVertShader.vsc";

// Scene and external textures files.
const char c_szSceneFile[] = "SketchObject.pod"; //The .pod file was exported from 3DSMax using PVRGeoPOD.
/******************************************************************************
 Defines
******************************************************************************/
//#define SHOW_MAX_FPS

/******************************************************************************
 Consts
******************************************************************************/
// Camera constants. Used for making the projection matrix
const float g_fCameraNear = 4.0f;
const float g_fCameraFar  = 500.0f;

/******************************************************************************
 Shader attributes
******************************************************************************/
enum EQuadAttrib {eQUAD_VERTEX_ARRAY, eQUAD_TEXCOORD_ARRAY};
// vertex attributes
enum EVertexAttrib {
	eVERTEX_ARRAY, eTEXCOORD_ARRAY, eNumAttribs };
const char* g_aszAttribNames[] = {
	"inVertex", "inTexCoord" };

// shader uniforms
enum EPreUniform {
	eMVPMatrix, eColorData, eNumPreUniforms };
const char* g_aszPreUniformNames[] = {
	"MVPMatrix", "inColor"};

enum EPostUniform {
	ePixelSize, eColorBufferTexture, eHatchTexture, eNumPostUniforms };
const char* g_aszPostUniformNames[] = {
	"PixelSize", "sColorBufferTexture", "sHatchTexture"};

/*	To add an alternate post shader based on the current shaders, you can simply add another
	#define path through the current post shader, then update these arrays and the shader defines.	*/
enum EPostShaders {
	eBasic, eEdgeDetection, eInverseEdges, eBlurEdges, eNumPostShaders };
const char* g_aszPostShaderNames[] = {
	"Basic", "Edge Detection", "Edge Detection: Inverse", "Edge Detection: Blur"};

/******************************************************************************
 Shader defines
******************************************************************************/
// Array containing the number of actual values stored in the shader definition array.
static const unsigned int g_auiNumPostShaderDefines[]= {0,1,2,2};

// Declares shader defines for use when loading shaders, allowing pre computed alternate paths through shaders.
// NB: Due to a call later determining whether or not to print the line width, EDGE_DETECTION should be defined here first.
static const char* c_aszPostShaderDefines[eNumPostShaders][2]= {{""},{"EDGE_DETECTION"},{"EDGE_DETECTION","INVERSE"},{"EDGE_DETECTION","BLUR"}};

/*****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES3EdgeDetection : public PVRShell
{
	CPVRTPrint3D	m_Print3D;		// Print3D class used to display text
	CPVRTModelPOD	m_Scene;		// 3D Model containing the full object scene.

	// Projection and Model View matrices
	PVRTMat4 m_mR2TProjection, m_mR2TView;

	// Pointer vector full of color data.
	PVRTVec3 *m_pvColorData;

	// Vertex Buffer Object (VBO) and Frame Buffer Object (FBO) handles
	GLuint *m_puiVbo, *m_puiIndexVbo, m_uiFramebufferObject, m_uiShaderID;
	GLint m_i32OriginalFramebuffer;

	// Texture IDs used by the app
	GLuint	m_uiDepthRenderbuffer, m_uiColorTexture;

	// Handles for our shaders
	GLuint	m_uiPreFragShader, m_uiPostFragShaders[eNumPostShaders],
			m_uiPreVertShader, m_uiPostVertShaders[eNumPostShaders];

	// Group shader programs and their uniform locations together.
	struct
	{
		GLuint uiId;
		GLuint auiLoc[eNumPreUniforms];
	} m_PreShader;

	struct
	{
		GLuint uiId;
		GLuint auiLoc[eNumPostUniforms];
	} m_PostShaders[eNumPostShaders];

	// Time variables
	unsigned long m_ulPreviousTimeAngle, m_ulPreviousTimeFPS, m_ulCurrentTime;

	// Variables used for the Aspect and framerate
	GLfloat	m_fAngleY, m_fFPS, m_fLineWidth;
	GLint m_iFrameCount;

	// App Variables
	int m_i32TexWidth, m_i32TexHeight;
	int m_i32WinWidth, m_i32WinHeight;

public:
	OGLES3EdgeDetection() :	m_pvColorData(0), m_puiVbo(0), m_puiIndexVbo(0), m_uiShaderID(1),
							m_ulPreviousTimeAngle(0), m_ulPreviousTimeFPS(0), m_ulCurrentTime(0),
							m_fAngleY(0), m_fFPS(0), m_fLineWidth(1.0f),m_iFrameCount(0),
							m_i32TexWidth(0),m_i32TexHeight(0)	{}

	// PVRShell functions that need to be implemented
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	// Program specific functions
	bool LoadTextures(CPVRTString* pErrorStr);
	bool LoadShaders(CPVRTString* pErrorStr);
	bool LoadVbos(CPVRTString* pErrorStr);
	bool CreateFBO(CPVRTString* pErrorStr);
	void SetupView(bool bRotate);
	void DrawMesh(unsigned int ui32MeshID);
	void DrawQuad();
};

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
bool OGLES3EdgeDetection::InitApplication()
{
#ifdef SHOW_MAX_FPS
	// Disable v-sync
	PVRShellSet(prefSwapInterval,0);
#endif

	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	/*	Loads the scene from the .pod file into a CPVRTModelPOD object.
		We could also export the scene as a header file and
		load it with ReadFromMemory().	*/
	if(m_Scene.ReadFromFile(c_szSceneFile) != PVR_SUCCESS)
	{
		CPVRTString ErrorStr = "ERROR: Couldn't load '" + CPVRTString(c_szSceneFile) + "'.";
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
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
				not be called.
******************************************************************************/
bool OGLES3EdgeDetection::QuitApplication()
{
	// Frees the memory allocated for the scene
	m_Scene.Destroy();

	// Deletes the vertex buffer object
	delete[] m_puiVbo;
	delete[] m_puiIndexVbo;

    return true;
}

/*!****************************************************************************
 @Function		InitView
 @Return		bool		true if no error occurred
 @Description	Code in InitView() will be called by PVRShell upon
				initialization or after a change in the rendering context.
				Used to initialize variables that are dependant on the rendering
				context (e.g. textures, vertex buffers, etc.)
******************************************************************************/
bool OGLES3EdgeDetection::InitView()
{
	// Store width and height of the viewport.
	m_i32WinWidth = PVRShellGet(prefWidth);
	m_i32WinHeight = PVRShellGet(prefHeight);

	// Set our texture dimensions to be the same as our windows
	m_i32TexWidth = m_i32WinWidth;
	m_i32TexHeight = m_i32WinHeight;

	/*	Get the current frame buffer object. As the program hasn't set it yet, this is the default buffer.
		On most platforms this just gives 0, but there are exceptions.	*/
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_i32OriginalFramebuffer);

	// Create string for error codes.
	CPVRTString ErrorStr;

	// Checks to see if the screen is rotated or not.
    bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	// Initialize VBO data
	if(!LoadVbos(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Initialise Print3D
	if(m_Print3D.SetTextures(0,m_i32WinWidth,m_i32WinHeight,bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	// Load external textures and create internal ones.
	if(!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Load and compile the shaders & link programs
	if (!LoadShaders(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Creates and checks FBO creation
	if (!CreateFBO(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	SetupView(bRotate);

	//Initialises the time variables.
	m_ulCurrentTime = PVRShellGetTime();
	m_ulPreviousTimeAngle = m_ulCurrentTime;
	m_ulPreviousTimeFPS = m_ulCurrentTime;

	return true;
}
/*!****************************************************************************
 @Function		SetupView()
 @Return		N/A
 @Description	Sets up the view matrices required for the training course
******************************************************************************/
void OGLES3EdgeDetection::SetupView(bool bRotate)
{
	PVRTVec3 vEyePos, vLookAt, vCamUp=PVRTVec3(0.00f, 1.0001f, 0.00f);

	// Checks if a camera is in the scene, if there is, uses it, otherwise creates one.
	if(m_Scene.nNumCamera>0)
	{
		// vLookAt is taken from the target node, or..
		if(m_Scene.pCamera[0].nIdxTarget != -1) m_Scene.GetCameraPos(vEyePos, vLookAt, 0);
		// ..it is calculated from the rotation
		else m_Scene.GetCamera(vEyePos, vLookAt, vCamUp, 0);
	}
	else
	{
		//Creates a camera if none exist.
		vEyePos = PVRTVec3(0, 0, 200);
		vLookAt = PVRTVec3(0, 0, 0);
	}

	// Set the view and projection matrix for rendering to texture.
	m_mR2TView = PVRTMat4::LookAtRH(vEyePos, vLookAt, vCamUp);
	m_mR2TProjection = PVRTMat4::PerspectiveFovRH(PVRT_PI*0.125, (float)m_i32TexWidth/(float)m_i32TexHeight, g_fCameraNear, g_fCameraFar, PVRTMat4::OGL, bRotate);

	// The textured quad this program renders to will be rendered full screen, in orthographic mode, so doesn't need camera variables to be set.
}

/*!****************************************************************************
 @Function		CreateFBO()
 @Return		bool			true if no error occured
 @Description	Creates a framebuffer object.
******************************************************************************/
bool OGLES3EdgeDetection::CreateFBO(CPVRTString* pErrorStr)
{
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Sets clear color to white
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	// Create a depth render buffer object to perform depth testing in our FBO.
	glGenRenderbuffers(1, &m_uiDepthRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_uiDepthRenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_i32TexWidth, m_i32TexHeight);

	// Create frame buffer object
	glGenFramebuffers(1, &m_uiFramebufferObject);
	glBindFramebuffer(GL_FRAMEBUFFER, m_uiFramebufferObject);

	// Attach color and depth texture buffers/textures.
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uiColorTexture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_uiDepthRenderbuffer);

	// Checks that the framebuffer was constructed successfully.
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		*pErrorStr += "ERROR: Frame buffer not set up correctly\n";
		return false;
	}

	// Rebind the default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_i32OriginalFramebuffer);
	return true;
}

/*!****************************************************************************
 @Function		LoadTextures
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course
******************************************************************************/
bool OGLES3EdgeDetection::LoadTextures(CPVRTString* pErrorStr)
{
	// Reads in and stores the diffuse color of every material used in the scene.
	m_pvColorData = new PVRTVec3[m_Scene.nNumMaterial];
	for (int i=0; i<(int)m_Scene.nNumMaterial; i++)	m_pvColorData[i] = PVRTVec3(m_Scene.pMaterial[i].pfMatDiffuse);

	/*	By setting textures up to active textures other than GL_TEXTURE0 (the default)
		we can avoid needing to bind them again later, as Print3D binds to 0, meaning we'd
		need to reset the binding each frame. This way keeps the rebindings to a minimum;
		however there are only 8 active texture units so this can only be done up to a point.*/

	// Create the color texture and bind it to texture unit 1.
	glGenTextures(1, &m_uiColorTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_uiColorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_i32TexWidth, m_i32TexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	// Sets texture parameters.
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Error checks color texture creation.
	GLint status = glGetError();
	if (status != GL_NO_ERROR)
	{
		*pErrorStr += "Error: Could not create color textures.";
		return false;
	}

	// Rebinds the initial texture unit.
	glActiveTexture(GL_TEXTURE0);
	return true;
}

/*!****************************************************************************
 @Function		LoadShaders
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads and compiles the shaders and links the shader programs
				required for this training course
******************************************************************************/
bool OGLES3EdgeDetection::LoadShaders(CPVRTString* pErrorStr)
{
	/*	Load and compile the shaders from files.
		Binary shaders are tried first, source shaders
		are used as fallback. */

	// Load vertex shaders
	if (PVRTShaderLoadFromFile(
			c_szPreVertShaderBin, c_szPreVertShaderSrc, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiPreVertShader, pErrorStr) != PVR_SUCCESS)
	{
		*pErrorStr += "Error: Could not load Pre Process Vertex Shader.";
		return false;
	}
	for (int i=0; i<eNumPostShaders; ++i)
	{
		if (PVRTShaderLoadFromFile(
				c_szPostVertShaderBin, c_szPostVertShaderSrc, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiPostVertShaders[i],
				pErrorStr, 0, c_aszPostShaderDefines[i], g_auiNumPostShaderDefines[i]) != PVR_SUCCESS)
		{
			*pErrorStr += "Error: Could not load Post Process Vertex Shader: ";
			*pErrorStr += g_aszPostShaderNames[i];
			return false;
		}
	}

	// Load fragment shaders
	if (PVRTShaderLoadFromFile(
			c_szPreFragShaderBin, c_szPreFragShaderSrc, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_uiPreFragShader, pErrorStr) != PVR_SUCCESS)
	{
		*pErrorStr += "Error: Could not load Pre Process Fragment Shader.";
		return false;
	}
	for (int i=0; i<eNumPostShaders; ++i)
	{
		if (PVRTShaderLoadFromFile(
				c_szPostFragShaderBin, c_szPostFragShaderSrc, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_uiPostFragShaders[i],
				pErrorStr, 0, c_aszPostShaderDefines[i], g_auiNumPostShaderDefines[i]) != PVR_SUCCESS)
		{
			*pErrorStr += "Error: Could not load Post Process Vertex Shader: ";
			*pErrorStr += g_aszPostShaderNames[i];
			return false;
		}
	}

	// Set up and link the shader programs
	if (PVRTCreateProgram(&m_PreShader.uiId, m_uiPreVertShader, m_uiPreFragShader, g_aszAttribNames, eNumAttribs, pErrorStr) != PVR_SUCCESS)
	{
		*pErrorStr += "Failed to Link Pre Shader";
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}
	for (int i=0; i<eNumPostShaders; i++)
	{
		if (PVRTCreateProgram(&m_PostShaders[i].uiId, m_uiPostVertShaders[i], m_uiPostFragShaders[i], g_aszAttribNames, eNumAttribs, pErrorStr) != PVR_SUCCESS)
		{
			*pErrorStr += "Failed to Link Post Shader: ";
			*pErrorStr += g_aszPostShaderNames[i];
			PVRShellSet(prefExitMessage, pErrorStr->c_str());
			return false;
		}
	}

	// Store the location of uniforms for later use
	for (int i = 0; i < eNumPreUniforms; ++i)
	{
		m_PreShader.auiLoc[i] = glGetUniformLocation(m_PreShader.uiId, g_aszPreUniformNames[i]);
	}
	for (int i = 0; i < eNumPostShaders; i++)
	{
		for (int j = 0; j <eNumPostUniforms; j++)
		{
			m_PostShaders[i].auiLoc[j] = glGetUniformLocation(m_PostShaders[i].uiId, g_aszPostUniformNames[j]);
		}
		//Set the post shaders to use the render texture.
		glUseProgram(m_PostShaders[i].uiId);
		glUniform1i(m_PostShaders[i].auiLoc[eColorBufferTexture], 1);
	}

	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
bool OGLES3EdgeDetection::LoadVbos(CPVRTString* pErrorStr)
{
	// If there are no VBOs to create, return
	if(m_Scene.nNumMesh == 0) return true;

	// Checks to make sure that POD data is interleaved:
	// Although this training course doesn't use normals or texture data for the models, this is done for code re-use.
	if(!m_Scene.pMesh[0].pInterleaved)
	{
		*pErrorStr = "ERROR: EdgeDetection requires the pod data to be interleaved. Please re-export with the interleaved option enabled.";
		return false;
	}

	// Initialises the vertex buffer objects.
	if(!m_puiVbo) m_puiVbo = new GLuint[m_Scene.nNumMesh];
	if(!m_puiIndexVbo) m_puiIndexVbo = new GLuint[m_Scene.nNumMesh];

	/*	The meshes have been exported with the "Interleave Vectors" option,
		so all data is interleaved in the buffer at pMesh->pInterleaved.
		Interleaving data improves the memory access pattern and cache efficiency,
		thus it can be read faster by the hardware.	*/

	// Generates the vertex buffer objects.
	glGenBuffers(m_Scene.nNumMesh, m_puiVbo);

	// Load vertex data from all meshes in the scene into the VBOs
	for(unsigned int i = 0; i < m_Scene.nNumMesh; ++i)
	{
		// Load vertex data
		SPODMesh& Mesh = m_Scene.pMesh[i];
		unsigned int uiSize = Mesh.nNumVertex * Mesh.sVertex.nStride;

		// Bind the associated VBO
		glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[i]);
		glBufferData(GL_ARRAY_BUFFER, uiSize, Mesh.pInterleaved, GL_STATIC_DRAW);

		// Load mesh index data into buffer object if available
		m_puiIndexVbo[i] = 0;
		if(Mesh.sFaces.pData)
		{
			glGenBuffers(1, &m_puiIndexVbo[i]);
			uiSize = PVRTModelPODCountIndices(Mesh) * sizeof(GLshort);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, Mesh.sFaces.pData, GL_STATIC_DRAW);
		}
	}

	// Unbind the VBOs
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES3EdgeDetection::ReleaseView()
{
	// Delete the color texture
	glDeleteTextures(1, &m_uiColorTexture);

	// Delete the depth render buffer
	glDeleteRenderbuffers(1, &m_uiDepthRenderbuffer);

    // delete shader program , and shaders
    glDeleteProgram(m_PreShader.uiId);
    glDeleteShader(m_uiPreVertShader);
    glDeleteShader(m_uiPreFragShader);
	for (int i=0; i<eNumPostShaders; ++i)
	{
	    glDeleteProgram(m_PostShaders[i].uiId);
	    glDeleteShader(m_uiPostVertShaders[i]);
        glDeleteShader(m_uiPostFragShaders[i]);
	}



	// Delete the stored color data.
	delete [] m_pvColorData->ptr();
	m_pvColorData=NULL;

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

	// Delete frame buffer objects
	glDeleteFramebuffers(1, &m_uiFramebufferObject);

	return true;
}

/*!****************************************************************************
 @Function		RenderScene
 @Return		bool		true if no error occured
 @Description	Main rendering loop function of the program. The shell will
				call this function every frame.
				eglSwapBuffers() will be performed by PVRShell automatically.
				PVRShell will also manage important and relevant OS events.
				The user has access to these events through an abstraction
				layer provided by PVRShell.
******************************************************************************/
bool OGLES3EdgeDetection::RenderScene()
{
	// Declares world orientation variables.
	PVRTMat4 mWorld, mMVP;

	// Updates the current time.
	m_ulCurrentTime=PVRShellGetTime();

#ifdef SHOW_MAX_FPS
	//Updates and checks framerate.
	m_iFrameCount+=1;
	if (m_ulCurrentTime-m_ulPreviousTimeFPS>=1000)
	{
		m_fFPS=(GLfloat)m_iFrameCount/(GLfloat)(m_ulCurrentTime-m_ulPreviousTimeFPS)*1000.0f;
		m_ulPreviousTimeFPS=m_ulCurrentTime;
		m_iFrameCount=0;
	}
	// Display fps data
	m_Print3D.Print3D(2.0f, 10.0f, 0.75f, 0xff0000ff, "%i fps", (int)m_fFPS);
#endif

	// Time dependant updates for the rotational velocity of the scene.
	m_fAngleY += 0.0002f*PVRT_PI*(m_ulCurrentTime-m_ulPreviousTimeAngle);
	m_ulPreviousTimeAngle=PVRShellGetTime();

	// Render to our texture (bracketed for viewing convenience)
	{
		// Use the first shader program to perform the initial render of the mask.
		glUseProgram(m_PreShader.uiId);

		// Bind render-to-texture frame buffer and set the viewPort
		glBindFramebuffer(GL_FRAMEBUFFER, m_uiFramebufferObject);

		if(m_i32TexWidth != m_i32WinWidth || m_i32TexHeight != m_i32WinHeight)
			glViewport(0, 0, m_i32TexWidth, m_i32TexHeight);

#if defined(__PALMPDK__)
		// Enable writing to the alpha channel again as usually it is disabled so
		// we don't blend with the video layer on webOS devices.
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
#endif
		// Clear the color and depth buffer of our FBO
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Rotates the scene and sets the model-view-projection matrix
		mWorld = PVRTMat4::RotationY(m_fAngleY);
		mMVP = m_mR2TProjection * m_mR2TView * mWorld;

		// Send the view matrix information to the shader.
		glUniformMatrix4fv(m_PreShader.auiLoc[eMVPMatrix], 1, GL_FALSE, mMVP.f);

		// Enable vertex attribute array
		glEnableVertexAttribArray(eVERTEX_ARRAY);

		//Enable depth testing and culling.
		glEnable(GL_DEPTH_TEST);
		glFrontFace(GL_CCW);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		// Draw our models by looping through each mesh as defined by nNumMesh.
		for (unsigned int i=0; i<m_Scene.nNumMeshNode; i++)
		{
			DrawMesh(i);
		}

		// Unbind the VBO and index buffer.
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		//Invalidate the framebuffer attachments we don't need to avoid unnecessary copying to system memory
		const GLenum attachment = GL_DEPTH_ATTACHMENT;
		glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attachment);
	}

	// Bind the original frame buffer to draw to screen and set the Viewport.
	glBindFramebuffer(GL_FRAMEBUFFER, m_i32OriginalFramebuffer);

	if(m_i32TexWidth != m_i32WinWidth || m_i32TexHeight != m_i32WinHeight)
		glViewport(0, 0, m_i32WinWidth, m_i32WinHeight);

	// Clear the color and depth buffers for the screen.
	glClear(GL_COLOR_BUFFER_BIT);

	// Uses PVRShell input handling to update the line width in the edge detection shaders.
	if (PVRShellIsKeyPressed(PVRShellKeyNameRIGHT))
	{
		m_fLineWidth++;
		if (m_fLineWidth>10) m_fLineWidth=10;
	}
	if (PVRShellIsKeyPressed(PVRShellKeyNameLEFT))
	{
		m_fLineWidth--;
		if (m_fLineWidth<1) m_fLineWidth=1;
	}
	// Uses PVRShell input to choose which post shader program to use for post processing.
	// Loops through all shaders defined in EPostShaders
	if (PVRShellIsKeyPressed(PVRShellKeyNameUP))
	{
		if (m_uiShaderID==eNumPostShaders-1) m_uiShaderID=0;
		else m_uiShaderID++;
	}
	else if (PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
	{
		if (m_uiShaderID==0) m_uiShaderID=eNumPostShaders-1;
		else m_uiShaderID--;
	}

	// Sets the shader based on the shader ID value, and sets the line width each frame (as it can change);
	glUseProgram(m_PostShaders[m_uiShaderID].uiId);
	glUniform2f(m_PostShaders[m_uiShaderID].auiLoc[ePixelSize],m_fLineWidth/(float)m_i32TexWidth,m_fLineWidth/(float)m_i32TexHeight);

	/*  Note: We do not need to pass any projection data to these shaders as they are used only to render a texture to a
		full screen quad which is parallel with the viewport. The model meshes have already been positioned in the previous
		shader and now only exist as a 2D image.*/

	// Enable texture attribute array
	glEnableVertexAttribArray(eTEXCOORD_ARRAY);
	// Draw the fullscreen quad to render the screen to.
	DrawQuad();

	// Disable the vertex and texture attribute arrays
	glDisableVertexAttribArray(eTEXCOORD_ARRAY);
	glDisableVertexAttribArray(eVERTEX_ARRAY);

	// Print the demo title, current post shader's name and the line width if applicable
	m_Print3D.DisplayDefaultTitle("Edge Detection", "", ePVRTPrint3DSDKLogo);
	m_Print3D.Print3D(5,80,1,0xff885500,g_aszPostShaderNames[m_uiShaderID]);
	if (!strcmp(c_aszPostShaderDefines[m_uiShaderID][0],"EDGE_DETECTION"))
		m_Print3D.Print3D(5,90,0.7f,0xff000055,"Line Width = %i", (int)m_fLineWidth);
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			meshID		The number of the mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set.
				Also works out and passes color and ID data to the shader in
				this case.
******************************************************************************/
void OGLES3EdgeDetection::DrawMesh(unsigned int ui32MeshID)
{
	// Calls the mesh and its material index.
	SPODMesh& Mesh = m_Scene.pMesh[ui32MeshID];
	GLuint ui32MaterialIndex = m_Scene.pNode[ui32MeshID].nIdxMaterial;

	// Works out an ID Number for the mesh - somewhere between 0 and 1.
	GLfloat ID = (GLfloat)ui32MeshID/(GLfloat)m_Scene.nNumMeshNode;

	// Binds color and ID data to the shader program.
	glUniform4fv(m_PreShader.auiLoc[eColorData], 1, PVRTVec4(m_pvColorData[ui32MaterialIndex],ID).ptr());

	// Bind the VBO for the mesh and the index buffer.
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[ui32MeshID]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[ui32MeshID]);

	// Set the vertex attribute offsets from the mesh
	glVertexAttribPointer(eVERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, Mesh.sVertex.nStride, Mesh.sVertex.pData);

	// Indexed Triangle list
	glDrawElements(GL_TRIANGLES, Mesh.nNumFaces*3, GL_UNSIGNED_SHORT, 0);
}

/*!****************************************************************************
 @Function		DrawQuad
 @Input			N/A
 @Description	Draws a full screen quad, without depth testing. Ideal for
				rendering a full screen texture.
******************************************************************************/
void OGLES3EdgeDetection::DrawQuad()
{
	//Sets vertex data for the quad.
	const float afVertexData[] = {-1,-1, 0, 1,-1, 0,-1, 1, 0, 1, 1, 0};
	glVertexAttribPointer(eVERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, 0, afVertexData);

	//Sets texture coordinate data for the quad
	const float afTexCoordData[] = { 0, 0,  1, 0,  0, 1,  1, 1 };
	glVertexAttribPointer(eTEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, afTexCoordData);

	// Draw the quad
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
	return new OGLES3EdgeDetection();
}

/******************************************************************************
 End of file (OGLES3EdgeDetection.cpp)
******************************************************************************/

