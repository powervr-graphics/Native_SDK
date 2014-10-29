/******************************************************************************

 @File         OGLES2Fractal.cpp

 @Title        Fractal

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows how to use a frame buffer object to render to a texture.

******************************************************************************/
#include <math.h>

#include "PVRShell.h"
#include "OGLES2Tools.h"

/******************************************************************************
 shader attributes
******************************************************************************/
// vertex attributes
enum EVertexAttrib {
	VERTEX_ARRAY, TEXCOORD_ARRAY, eNumAttribs };
const char* g_aszAttribNames[] = {
	"inVertex", "inTexCoord" };

// shader uniforms
enum EUniform {
	eMVPMatrix, eNumUniforms };
const char* g_aszUniformNames[] = {
	"MVPMatrix" };

/******************************************************************************
 Content file names
******************************************************************************/

// Source and binary shaders
const char c_szFragShaderSrcFile[]	= "FragShader.fsh";
const char c_szFragShaderBinFile[]	= "FragShader.fsc";
const char c_szVertShaderSrcFile[]	= "VertShader.vsh";
const char c_szVertShaderBinFile[]	= "VertShader.vsc";

/*****************************************************************************
 ** Class: OGLES2Fractal
******************************************************************************/
class OGLES2Fractal : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// OpenGL handles for shaders, textures, FBOs and VBOs
	GLuint	m_uiVertShader;
	GLuint	m_uiFragShader;
	GLuint	m_auiTexture[2];
	GLuint  m_uiTrunkTex;
	GLuint  m_uiVbo;
    GLuint	m_auiFbo[2];
	int		m_i32CurrentFbo;
	int		m_i32OriginalFbo;

	// Group shader programs and their uniform locations together
	struct
	{
		GLuint uiId;
		GLuint auiLoc[eNumUniforms];
	}
	m_ShaderProgram;

	int m_i32TexSize;

	float m_fAngle;
	float m_fAngle2;

	unsigned int m_ui32Framenum;
	unsigned long m_ulTime;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	void LoadTextures();
	bool LoadShaders(CPVRTString* pErrorStr);
	void LoadVbos();

	bool DrawScreen();
	bool RenderFractal();
};

/*!****************************************************************************
 @Function		LoadTextures
 @Description	Loads the textures required for this training course
******************************************************************************/
void OGLES2Fractal::LoadTextures()
{
    /*
		Initialise the textures
	*/

	// Allocates one texture handle for the trunk texture
	glGenTextures(1, &m_uiTrunkTex);

	// Binds this texture handle so we can load the data into it
	glBindTexture(GL_TEXTURE_2D, m_uiTrunkTex);

	// Creates the data as a 32bits integer array (8bits per component)
	GLuint* pTexData = new GLuint[32*32];

	// Create texture pattern
	for (int i=0; i<32; i++)
	{
		for (int j=0; j<32; j++)
		{
			pTexData[i*32+j] = 0xFF000000 + ((255 - (j-16)*(j-15)) << 8) + ((j * 8) & 0xFF);
		}
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, pTexData);

    /* Destroy the array as it is no longer needed*/
    delete [] pTexData;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Allocate two textures
	glGenTextures(2, m_auiTexture);
	for (int i = 0; i < 2; ++i)
	{
		// Binds this texture handle so we can load the data into it
		glBindTexture(GL_TEXTURE_2D, m_auiTexture[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_i32TexSize, m_i32TexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
}

/*!****************************************************************************
 @Function		LoadShaders
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads and compiles the shaders and links the shader programs
				required for this training course
******************************************************************************/
bool OGLES2Fractal::LoadShaders(CPVRTString* pErrorStr)
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
 @Description	Loads the vertex data required for this training course into a
				vertex buffer object
******************************************************************************/
void OGLES2Fractal::LoadVbos()
{
	float afVertexData[] = {
		// trunk
		-0.1f,	-1.0f, 0.5f,	 0.0f, 1.0f,
		 0.1f,	-1.0f, 0.5f,	 1.0f, 1.0f,
		-0.08f,	-0.4f, 0.5f,	 0.0f, 0.2f,
		 0.08f,	-0.4f, 0.5f,	 1.0f, 0.2f,
		 0.0f,	-0.3f, 0.5f,	 0.5f, 0.0f,

		 // feedback quad
		 -0.65f, 0.0f, 0.65f,	 0.0f, 0.0f,
		  0.65f, 0.0f, 0.65f,	 1.0f, 0.0f,
		 -0.65f, 1.3f, 0.65f,	 0.0f, 1.0f,
		  0.65f, 1.3f, 0.65f,	 1.0f, 1.0f,
	};

	glGenBuffers(1, &m_uiVbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_uiVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(afVertexData), afVertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
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
bool OGLES2Fractal::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));
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
bool OGLES2Fractal::QuitApplication()
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
bool OGLES2Fractal::InitView()
{
	// Find the largest square power of two texture that fits into the viewport
	m_i32TexSize = 1;
	int iSize = PVRT_MIN(PVRShellGet(prefWidth), PVRShellGet(prefHeight));
	while (m_i32TexSize * 2 < iSize) m_i32TexSize *= 2;

	srand((unsigned int) PVRShellGetTime());
	m_ui32Framenum = rand() % 5000;

	// Get the initial time
	m_ulTime = PVRShellGetTime();

	/*
		Initialize VBO data and load textures
	*/
	LoadVbos();
	LoadTextures();

	/*
		Load and compile the shaders & link programs
	*/
	CPVRTString ErrorStr;
	if (!LoadShaders(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Set the sampler2D uniforms to corresponding texture units
	glUniform1i(glGetUniformLocation(m_ShaderProgram.uiId, "sTexture"), 0);

	/*
		Initialize Print3D
	*/

	// Is the screen rotated?
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	/*
		Create two handles for a frame buffer object.
	*/
	glGenFramebuffers(2, m_auiFbo);
	m_i32CurrentFbo = 1;

	/*
		Get the currently bound frame buffer object. On most platforms this just gives 0.
	 */
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_i32OriginalFbo);

	/*
		Attach the renderable objects (e.g. textures) to the frame buffer object now as
		they will stay attached to the frame buffer object even when it is not bound.
	*/

	// We have two FBOs so we're doing the same for each
	for(int i = 0; i < 2; ++i)
	{
		/*
			Firstly, to do anything with a frame buffer object we need to bind it. In the case
			below we are binding our frame buffer object to the frame buffer.
		*/
		glBindFramebuffer(GL_FRAMEBUFFER, m_auiFbo[i]);

		/*
			To render to a texture we need to attach it texture to the frame buffer object.
			GL_COLOR_ATTACHMENT0 tells it to attach the texture to the colour buffer, the 0 on the
			end refers to the colour buffer we want to attach it to as a frame buffer object can
			have more than one colour buffer.
		*/
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_auiTexture[i], 0);

		// Clear the color buffer for this FBO
		glClear(GL_COLOR_BUFFER_BIT);

        // Check that our FBO creation was successful
        GLuint uStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if(uStatus != GL_FRAMEBUFFER_COMPLETE)
        {
            PVRShellSet(prefExitMessage, "ERROR: Failed to initialise FBO");
            return false;
        }
	}

	/*
		Unbind the frame buffer object so rendering returns back to the backbuffer.
	*/
	glBindFramebuffer(GL_FRAMEBUFFER, m_i32OriginalFbo);

	// Use a nice bright blue as clear colour
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	// Enable culling
	glEnable(GL_CULL_FACE);

	// Disable depth test as we don't need it
	glDisable(GL_DEPTH_TEST);
	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occurred
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES2Fractal::ReleaseView()
{
	// Delete textures
	glDeleteTextures(2, m_auiTexture);
	glDeleteTextures(1, &m_uiTrunkTex);

	// Delete program and shader objects
	glDeleteProgram(m_ShaderProgram.uiId);

	glDeleteShader(m_uiVertShader);
	glDeleteShader(m_uiFragShader);

	// Delete buffer objects
	glDeleteBuffers(1, &m_uiVbo);
	glDeleteFramebuffers(2, m_auiFbo);

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

	return true;
}

/*!****************************************************************************
 @Function		RenderScene
 @Return		bool		true if no error occurred
 @Description	Main rendering loop function of the program. The shell will
				call this function every frame.
				eglSwapBuffers() will be performed by PVRShell automatically.
				PVRShell will also manage important OS events.
				Will also manage relevant OS events. The user has access to
				these events through an abstraction layer provided by PVRShell.
******************************************************************************/
bool OGLES2Fractal::RenderScene()
{
	/* vary the branch angles on the fractal sinusoidally */
	m_fAngle = (float)(sin(0.25*PVRT_PIf*(float)(m_ui32Framenum)/256.0f))* 70.0f;

	/* largeish prime number in the angular frequency here, so the motion's not obviously periodic */
	m_fAngle2 = (float)(sin((79.0f/256.0f)*2.0*PVRT_PIf*(float)(m_ui32Framenum)/256.0f))*100.0f + 30.0f;

	/* Convert the angles to radians. */
	m_fAngle  *= 0.017453f;
	m_fAngle2 *= 0.017453f;

	/* Increase the frame count */
	unsigned long ulTime = PVRShellGetTime();
	if(ulTime - m_ulTime > 10)
	{
		m_ulTime = ulTime;
		m_ui32Framenum += 2;

		if(m_ui32Framenum > 20000)
			m_ui32Framenum = 0;
	}

	// Draw the Fractal
	if(!DrawScreen())
		return false;

	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("Fractal", "Using FBOs", ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}

bool OGLES2Fractal::DrawScreen()
{
	/*
		We're going to do the following steps to create the effect. Texture 1 refers to the texture
		attached to the first FBO. Texture 2 refers to the texture attached to the second FBO.

		Frame 0

			1.	We bind the second frame buffer object so we can do things to it.
			2.	We draw two quads with Texture 1 applied.
			3.	We draw the trunk.
			4.	We make the back buffer current.
			5.	We draw 6 quads with Texture 2 applied.

		Frame 1

			6.	We bind the first frame buffer object so we can do things to it.
			7.	We draw two quads with Texture 2 applied. Texture 2 still contains
				the image from the last frame.
			8.	We draw the trunk.
			9.	We make the back buffer current.
			10.	We draw 6 quads with Texture 1 applied.

		Frame 2

			11.	We bind the second frame buffer object so we can do things to it.
			12.	We draw two quads with Texture 1 applied. Texture 1 still contains
				the image from the last frame.
			13.	We draw the trunk.
			14.	We make the back buffer current.
			15.	We draw 6 quads with Texture 2 applied.

			16.	We repeat steps 6 through to 16 for consecutive frames.
	*/

	// Use the program created with the fragment and vertex shaders.
	glUseProgram(m_ShaderProgram.uiId);
	glBindBuffer(GL_ARRAY_BUFFER, m_uiVbo);

	glEnableVertexAttribArray(VERTEX_ARRAY);
	glEnableVertexAttribArray(TEXCOORD_ARRAY);
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	/*
		Draw the fractal onto the current m_ui32Texture
	*/
	if(!RenderFractal())
		return false;

	PVRTMat4 fMatrix;
	fMatrix = PVRTMat4::Identity();

	/*
		Bind the projection model view matrix (PMVMatrix) to
		the associated uniform variable in the shader
	*/
	glUniformMatrix4fv(m_ShaderProgram.auiLoc[eMVPMatrix], 1, GL_FALSE, fMatrix.ptr());

	// Clear the colour buffer
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	/*
		Set the viewport ot fill the screen.
	*/
	glViewport(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	/*	Set up the matrix we are going to use to rotate and scale the 6 quads.	*/
	PVRTMat4 fRotZ;
	fMatrix = PVRTMat4::Scale(0.8f *(float)PVRShellGet(prefHeight) / (float)PVRShellGet(prefWidth), 0.8f, 0.8f);
	fRotZ = PVRTMat4::RotationZ(1.047f);

//	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_COLOR, GL_ONE);

	/* Bind the texture that we have rendered too.*/
	glBindTexture(GL_TEXTURE_2D, m_auiTexture[m_i32CurrentFbo]);

	/* Draws 6 rotated quads */
	for(int i = 0; i < 6; ++i)
	{
		// Set the transformationh matrix
		glUniformMatrix4fv(m_ShaderProgram.auiLoc[eMVPMatrix], 1, GL_FALSE, fMatrix.ptr());

		// Draw the quad
		glDrawArrays(GL_TRIANGLE_STRIP, 5, 4);

		// Rotate the object by another 60 degrees.
		fMatrix = fMatrix * fRotZ;
	}

	// Swap the FBOs
	m_i32CurrentFbo = 1 - m_i32CurrentFbo;

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(VERTEX_ARRAY);
	glDisableVertexAttribArray(TEXCOORD_ARRAY);

	return true;
}

/*******************************************************************************
 * Function Name  : RenderFractal
 * Description    : Draws the Fractal
 *******************************************************************************/
bool OGLES2Fractal::RenderFractal()
{
    /*
         To do anything with a frame buffer object we need to bind it. In the case
		 below we are binding our frame buffer object to the frame buffer.
    */
	glBindFramebuffer(GL_FRAMEBUFFER, m_auiFbo[m_i32CurrentFbo]);

    /*
         If everything went ok then we can render to the texture.
    */
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
	{
		PVRTMat4 fMatrix, fTrans, fRot;

        // Setup the Viewport to the dimensions of the texture
		glViewport(0, 0, m_i32TexSize, m_i32TexSize);

        // Clear the screen by this colour
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Bind the texture used for rendering to for the previous frame
		glBindTexture(GL_TEXTURE_2D, m_auiTexture[1 - m_i32CurrentFbo]);

		// Enable additive blend
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		/*
			Initialise the translation array that we are going to use to translate both of the
			the quads that we are going to render to.
		*/
		fTrans = PVRTMat4::Translation(0.0f, -0.4f, 0.0f);

		/*
			Set up the rotation matrix that we are going to use to rotate the two quads
			that have the previous texture created for the previous frame bound
			to them.
		*/
		fRot = PVRTMat4::RotationZ(m_fAngle + m_fAngle2);
		fMatrix = fTrans * fRot;
		/*
			Set the transformation matrix in the shader.
		*/
		glUniformMatrix4fv(m_ShaderProgram.auiLoc[eMVPMatrix], 1, GL_FALSE, fMatrix.ptr());
		glDrawArrays(GL_TRIANGLE_STRIP, 5, 4);

		/*
			Rotate the second quad the other way.
		*/
		fRot = PVRTMat4::RotationZ(m_fAngle - m_fAngle2);
		fMatrix = fTrans * fRot;
		glUniformMatrix4fv(m_ShaderProgram.auiLoc[eMVPMatrix], 1, GL_FALSE, fMatrix.ptr());
		glDrawArrays(GL_TRIANGLE_STRIP, 5, 4);

		/*
			Now draw the trunk.
	    */
		// Bind the trunk texture
		glBindTexture(GL_TEXTURE_2D, m_uiTrunkTex);

		fMatrix = PVRTMat4::Identity();
		glUniformMatrix4fv(m_ShaderProgram.auiLoc[eMVPMatrix], 1, GL_FALSE, fMatrix.ptr());

		// Draw the trunk
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 5);
	}

	/*
		Unbind the frame buffer object so rendering returns back to the backbuffer.
	*/
	glBindFramebuffer(GL_FRAMEBUFFER, m_i32OriginalFbo);
	return true;
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
	return new OGLES2Fractal();
}

/******************************************************************************
 End of file (OGLES2Fractal.cpp)
******************************************************************************/

