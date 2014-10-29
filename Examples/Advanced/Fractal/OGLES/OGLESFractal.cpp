/******************************************************************************

 @File         OGLESFractal.cpp

 @Title        Fractal

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows how to render to a pBuffer surface and bind that to a
               texture.

******************************************************************************/

/****************************************************************************
 ** INCLUDES                                                               **
 ****************************************************************************/

#include <math.h>
#include "PVRShell.h"
#include "OGLESTools.h"
#include <string.h>

/******************************************************************************
 Defines
******************************************************************************/
#if !defined(EGL_VERSION_1_0) // Do we have access to EGL?
#if !defined(EGL_NOT_PRESENT)
#define EGL_NOT_PRESENT 1
#endif
#endif

/******************************************************************************
 Consts
******************************************************************************/
const char* m_pFBODescription = "Using FBOs";

#if !defined(EGL_NOT_PRESENT)
const char *m_pPBufferDescription = "Using PBuffers";
#endif

/****************************************************************************
 ** STRUCTURES                                                             **
 ****************************************************************************/
enum EVBO
{
	eFeedback,
	eStalk
};

/****************************************************************************
** Class: OGLESFractal
****************************************************************************/
class OGLESFractal : public PVRShell
{
		// Render contexts, etc
	GLint	   m_CurrentFBO;

#if !defined(EGL_NOT_PRESENT)
	EGLDisplay m_CurrentDisplay;
	EGLContext m_CurrentContext;
	EGLSurface m_CurrentSurface;

	// We require 2 PBuffer surfaces.
	EGLSurface m_PBufferSurface[2];
#endif

	// We require 2 FBOs
	GLuint m_uFBO[2];

	unsigned int m_ui32CurrentBuffer;
	unsigned int m_ui32PreviousBuffer;

	enum
	{
		eNone,
#if !defined(EGL_NOT_PRESENT)
		ePBuffer,
#endif
		eFBO
	} m_eR2TType;

	CPVRTglesExt m_Extensions;

	// Texture IDs
	GLuint	m_ui32Texture[2];

	// Print3D
	CPVRTPrint3D 		m_Print3D;

	int m_i32TexSize;

	float m_fAngle;
	float m_fAngle2;

	unsigned int m_ui32Framenum;
	unsigned long m_ulTime;

	// Vertex Buffer Object (VBO) handles
	GLuint	m_ui32Vbo[2];

	const char * m_pDescription;

public:
	OGLESFractal() : 	m_ui32CurrentBuffer(1),
						m_ui32PreviousBuffer(0),
						m_pDescription(0)
	{
	}

	// PVRShell functions
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	/****************************************************************************
	** Function Definitions
	****************************************************************************/
	bool DrawScreen();
	bool RenderFractal();
	bool CreateFBOsorPBuffers();
#if !defined(EGL_NOT_PRESENT)
	EGLConfig SelectEGLConfig();
#endif
	bool StartRenderToTexture();
	bool EndRenderToTexture();
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
bool OGLESFractal::InitApplication()
{
	// Request PBuffer support
	PVRShellSet(prefPBufferContext, true);

	return true;
}

/*!****************************************************************************
 @Function		QuitApplication
 @Return		bool		true if no error occurred
 @Description	Code in QuitApplication() will be called by PVRShell once per
				run, just before exiting the program.
				If the rendering context is lost, QuitApplication() will
				not be called.
******************************************************************************/
bool OGLESFractal::QuitApplication()
{
    return true;
}

/*!****************************************************************************
 @Function		CreateFBOsorPBuffers
 @Return		bool		true if no error occurred
 @Description	Attempts to create our FBOs if supported or PBuffers if they
				are not.
******************************************************************************/
bool OGLESFractal::CreateFBOsorPBuffers()
{
#if !defined(EGL_NOT_PRESENT)
	EGLConfig eglConfig = 0;
	EGLint list[9];
#endif

	// Find the largest square power of two texture that fits into the viewport
	m_i32TexSize = 1;
	int iSize = PVRT_MIN(PVRShellGet(prefWidth), PVRShellGet(prefHeight));
	while (m_i32TexSize * 2 < iSize) m_i32TexSize *= 2;

	// Check for FBO extension
	if(CPVRTglesExt::IsGLExtensionSupported("GL_OES_framebuffer_object"))
	{
		// FBOs are present so we're going to use them
		m_eR2TType = eFBO;

		// Load the extensions as they are required
		m_Extensions.LoadExtensions();

		// Get the currently bound frame buffer object. On most platforms this just gives 0.
		glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &m_CurrentFBO);

		// Set the description used by Print3D later on
		m_pDescription = m_pFBODescription;
	}
#if !defined(EGL_NOT_PRESENT)
	else
	{
		// FBOs aren't present so we're going to use PBuffers
		m_eR2TType = ePBuffer;

		// Set up a configuration and attribute list used for creating a PBuffer surface.
		eglConfig = SelectEGLConfig();

		// First we specify the width of the surface...
		list[0] = EGL_WIDTH;
		list[1] = m_i32TexSize;
		// ...then the height of the surface...
		list[2] = EGL_HEIGHT;
		list[3] = m_i32TexSize;
		/* ... then we specifiy the target for the texture
			that will be created when the pbuffer is created...*/
		list[4] = EGL_TEXTURE_TARGET;
		list[5] = EGL_TEXTURE_2D;
		/*..then the format of the texture that will be created
			when the pBuffer is bound to a texture...*/
		list[6] = EGL_TEXTURE_FORMAT;
		list[7] = EGL_TEXTURE_RGB;
		// The final thing is EGL_NONE which signifies the end.
		list[8] = EGL_NONE;

		/*
			Get the current display, context and surface so we can switch between the
			PBuffer surface and the main render surface.
		*/

		m_CurrentDisplay = eglGetCurrentDisplay();
		m_CurrentContext = eglGetCurrentContext();
		m_CurrentSurface = eglGetCurrentSurface(EGL_DRAW);

		// Set the description used by Print3D later on
		m_pDescription = m_pPBufferDescription;
	}
#else
	else
	{
		PVRShellSet(prefExitMessage, "ERROR: Required extension \"GL_OES_framebuffer_object\" not present.\n");
		return false;
	}
#endif

	for(int i = 0; i < 2; ++i)
	{
		// Create texture for rendering to
		glGenTextures(1, &m_ui32Texture[i]);
		glBindTexture(GL_TEXTURE_2D, m_ui32Texture[i]);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_i32TexSize, m_i32TexSize, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 0);

		// Create the surface or object that will allow us to render to the aforementioned texture
		switch(m_eR2TType)
		{
			case eFBO: // Create FBO
			{
				m_Extensions.glGenFramebuffersOES(1, &m_uFBO[i]);
				m_Extensions.glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_uFBO[i]);

				// Attach the texture to the FBO
				m_Extensions.glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, m_ui32Texture[i], 0);

				// Check that our FBO creation was successful
				GLuint uStatus = m_Extensions.glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);

				if(uStatus != GL_FRAMEBUFFER_COMPLETE_OES)
				{
					PVRShellSet(prefExitMessage, "ERROR: Failed to initialise FBO\n");
					return false;
				}

				// Unbind the FBO now we are done with it
				m_Extensions.glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_CurrentFBO);
			}
			break;
#if !defined(EGL_NOT_PRESENT)
			case ePBuffer: // Create a PBuffer surface
			{
				m_PBufferSurface[i] = eglCreatePbufferSurface(m_CurrentDisplay, eglConfig, list);

				// If we don't have both of the surfaces return false.
				if(m_PBufferSurface[i] == EGL_NO_SURFACE)
				{
					PVRShellSet(prefExitMessage, "ERROR: Failed to create pbuffer.\n");
					return false;
				}

				// Switch the render target to the pBuffer
				if(!eglMakeCurrent(m_CurrentDisplay, m_PBufferSurface[i], m_PBufferSurface[i], m_CurrentContext))
				{
					PVRShellSet(prefExitMessage, "ERROR: Unable to make the pbuffer context current.\n");
					return false;
				}

				// Bind the texture to this surface
				eglBindTexImage(m_CurrentDisplay, m_PBufferSurface[i], EGL_BACK_BUFFER);
			}
			break;
#endif
			default: {}
		}

		// Clear the colour buffer for this FBO
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

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
bool OGLESFractal::InitView()
{
	// Create FBOs or PBuffers
	if(!CreateFBOsorPBuffers())
		return false;

	// Initialize Print3D
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D.\n");
		return false;
	}

	/*
		Change the starting point for a random number by using the current
		time as a seed. Once that is done set the frame number to something
		random.
	*/

	srand((unsigned int) PVRShellGetTime());
	m_ui32Framenum = rand() % 5000;

	// Get the initial time
	m_ulTime = PVRShellGetTime();

	// Create VBOs for the app

	// Set up the quad that we will texture to
	float m_Feedback[] = { -0.65f, 1.3f, 0.65f, // Position
							0.0f, 1.0f,					 // UV
							-0.65f, 0.0f, 0.65f,
							0.0f, 0.0f,
							0.65f, 0.0f, 0.65f,
							1.0f, 0.0f,
							0.65f, 1.3f, 0.0f,
							1.0f, 1.0f };

	// Set up the vertices for the stalk which is the basis for the pattern
	VERTTYPE m_Vertices[] = { -0.08f, -0.4f, 0.5f, // Position
							-0.1f, -1.0f, 0.5f,
							0.1f, -1.0f, 0.5f,
							0.1f,	-1.0f, 0.5f,
							0.08f, -0.4f, 0.5f,
							-0.08f, -0.4f, 0.5f,
							0.0f, -0.3f, 0.5f,
							0.08f, -0.4f, 0.5f,
							-0.08f, -0.4f, 0.5f };

	glGenBuffers(2, &m_ui32Vbo[0]);

	unsigned int uiSize = 4 * (sizeof(float) * 5); // 4 vertices * stride (5 verttypes per vertex)

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo[eFeedback]);

	// Set the buffer's data
	glBufferData(GL_ARRAY_BUFFER, uiSize, m_Feedback, GL_STATIC_DRAW);

	uiSize = 9 * (sizeof(float) * 3); // 9 vertices * stride (3 verttypes per vertex)

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo[eStalk]);

	// Set the buffer's data
	glBufferData(GL_ARRAY_BUFFER, uiSize, m_Vertices, GL_STATIC_DRAW);

	// Unbind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Enable culling
	glEnable(GL_CULL_FACE);
	return true;
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns        : Nothing
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool OGLESFractal::ReleaseView()
{
	// Release the Print3D textures
	m_Print3D.ReleaseTextures();

	// Delete the textures we created
	glDeleteTextures(2,&m_ui32Texture[0]);

	switch(m_eR2TType)
	{
		case eFBO:
			// Delete frame buffer objects
			m_Extensions.glDeleteFramebuffersOES(2, &m_uFBO[0]);
		break;
#if !defined(EGL_NOT_PRESENT)
		case ePBuffer:
			// Destroy the surfaces we created
			eglDestroySurface(m_CurrentDisplay,	m_PBufferSurface[0]);
			eglDestroySurface(m_CurrentDisplay,	m_PBufferSurface[1]);
		break;
#endif
		default: { }
	}

	return true;
}

/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool OGLESFractal::RenderScene()
{
	// Vary the branch angles on the fractal sinusoidally
	m_fAngle = (float)(sin(0.25*PVRT_PIf*(float)(m_ui32Framenum)/256.0f ))* 70.0f;

	// largeish prime number in the angular frequency here, so the motion's not obviously periodic
	m_fAngle2 = (float)(sin((79.0f/256.0f)*2.0*PVRT_PIf*(float)(m_ui32Framenum)/256.0f  ))*100.0f + 30.0f;

	// Increase the frame count
	unsigned long ulTime = PVRShellGetTime();
	if(ulTime - m_ulTime > 10)
	{
		m_ulTime = ulTime;
		m_ui32Framenum += 2;

		if(m_ui32Framenum > 20000)
			m_ui32Framenum = 0;
	}

	// Disable the depth test as we don't need it
	glDisable(GL_DEPTH_TEST);

	// Draw the Fractal
	if(!DrawScreen())
		return false;

	m_Print3D.DisplayDefaultTitle("Fractal", m_pDescription, ePVRTPrint3DSDKLogo);

	// Flush all Print3D commands
	m_Print3D.Flush();

	return true;
}


/*******************************************************************************
 * Function Name  : DrawScreen
 * Description    : Draws the Fractal
 *******************************************************************************/
bool OGLESFractal::DrawScreen()
{
	/*
		We're going to do the following steps to create the effect

		Frame 0

			1.	We make surface1 the current rendering context.
			2.	We draw two quads with m_ui32Texture applied.
			3.	We release surface2 from any textures it is bound to.
			4.	We draw a non-textured polygon.
			5.	We bind m_ui32Texture to surface1.
			6.	We make the back buffer current.
			7.	We draw 6 quads with m_ui32Texture applied.

		Frame 1

			8.	We make surface2 the current rendering context.
			9.	We draw two quads with m_ui32Texture (still bound to surface1) applied.
			10.	We release surface1 from any textures it is bound to.
			11.	We draw a non-textured polygon.
			12.	We bind m_ui32Texture to surface2.
			13.	We make the back buffer current.
			14.	We draw 6 quads with m_ui32Texture (bound to surface2) applied.

		Frame 2

			15.	We make surface1 the current rendering context.
			16.	We draw two quads with m_ui32Texture (still bound to surface2) applied.
			17.	We release surface2 from any textures it is bound to.
			18.	We draw a non-textured polygon.
			19.	We bind m_ui32Texture to surface1.
			20.	We make the back buffer current.
			21.	We draw 6 quads with m_ui32Texture (bound to surface1) applied.

			22.	We repeat steps 8 through to 22 for consecutive frames.
	*/

	/*
		Draw the fractal onto m_ui32Texture
	*/

	if(!RenderFractal())
		return false;

	//	Render 6 rotated copies of the fractal to the screen:

	// Set the Viewport to the whole screen.
	glViewport(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	// Enable blending.
	glEnable (GL_BLEND);
	glBlendFunc(GL_DST_COLOR, GL_ONE);

	// Enable the vertex and the texture coordinate state.
	glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// Enable 2D texturing.
	glEnable(GL_TEXTURE_2D);


	// Set the colour of the overal effect.
	glColor4f(1.0f, 1.0f, 0.0f, 1.0f);

	// Bind the texture that is currently bound to a PBuffer surface.
	glBindTexture(GL_TEXTURE_2D,m_ui32Texture[m_ui32CurrentBuffer]);

	// Clear the background to a light blue.
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	// Scale the scene to fit the screen comfortably.
	if(PVRShellGet(prefWidth) > PVRShellGet(prefHeight))
		glScalef(0.8f * (float)PVRShellGet(prefHeight)/(float)PVRShellGet(prefWidth),0.8f,0.8f);
	else
		glScalef(0.8f,0.8f * (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight),0.8f);

	// bind the VBO for the feedback quad
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo[eFeedback]);

	// Set the vertex and texture coordinate buffers we're going to use.
	glVertexPointer(3, GL_FLOAT, sizeof(float) * 5 , 0);
	glTexCoordPointer(2, GL_FLOAT, sizeof(float) * 5, (unsigned char*) (sizeof(float) * 3));

	/*
		The pBuffer texture only contains one branch of the effect so what we do is render
		six quads rotated round a point so we end up displaying 6 branches.
	*/
	for(int i = 0; i < 6; ++i)
	{
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glRotatef(60.0, 0.0, 0.0, 1.0 );
	}

	glPopMatrix();

	// Disable the vertex and texture coordinate client states and texturing in 2D.
	glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_2D);

	// unbind the vertex buffer as we don't need it bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Swap the buffers over.
	m_ui32PreviousBuffer = m_ui32CurrentBuffer;
	++m_ui32CurrentBuffer;

	if(m_ui32CurrentBuffer > 1)
		m_ui32CurrentBuffer = 0;

	return true;

}

/*******************************************************************************
 * Function Name  : RenderFractal
 * Description    : Draws the Fractal
 *******************************************************************************/
bool OGLESFractal::RenderFractal()
{
	if(!StartRenderToTexture())
		return false;

	// Setup the Viewport to the dimensions of the pBuffer
	glViewport(0, 0, m_i32TexSize, m_i32TexSize);

	// Clear the screen by this colour
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Render two rotated copies of the previous PBuffer onto current frame:

	// Bind the texture created on the previous frame:
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,m_ui32Texture[m_ui32PreviousBuffer]);

	// Enable additive blend:
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	// Enable the vertex and texture coordinate client states
	glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// bind the VBO for the feedback quad
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo[eFeedback]);

	// Set up the vertex and texture coordinate buffers we are going to use.
	glVertexPointer(3, GL_FLOAT, sizeof(float) * 5 , 0);
	glTexCoordPointer(2, GL_FLOAT, sizeof(float) * 5, (unsigned char*) (sizeof(float) * 3));

	/*
	Switch to the modelview matrix and push it onto the stack so we don't make any
	permanent changes.
	*/
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	// Translate and rotate the first quad.
	glTranslatef( 0.0f,-0.4f,0.0f );
	glRotatef( m_fAngle + m_fAngle2, 0.0, 0.0, 1.0 );

	// Draw the first quad.
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glPopMatrix();

	glPushMatrix();

	// Translate and rotate the second quad.
	glTranslatef( 0.0f,-0.4f,0.0f );
	glRotatef( m_fAngle - m_fAngle2, 0.0, 0.0, 1.0 );

	// Draw the second quad
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glPopMatrix();

	/*
		Now draw the trunk.

		Firstly disable the texture coordinate state as the
		trunk doesn't have texture coordinates.
	*/

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_2D);

	// bind the VBO for the stalk
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo[eStalk]);

	/*
		Set up the vertex buffer we'll be using.
	*/
	glVertexPointer(3, GL_FLOAT, sizeof(float) * 3, 0);

	// Draw the trunk
	glDrawArrays(GL_TRIANGLES, 0, 9);

	// Disable the vertex array as we don't need it anymore.
	glDisableClientState(GL_VERTEX_ARRAY);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,m_ui32Texture[m_ui32CurrentBuffer]);

	// We've now done rendering to our texture
	return EndRenderToTexture();
}

/*!****************************************************************************
 @Function		StartRenderToTexture
 @Return		bool		true if no error occured
 @Description	Setup the render to texture
******************************************************************************/
bool OGLESFractal::StartRenderToTexture()
{
	switch(m_eR2TType)
	{
		case eFBO:
			m_Extensions.glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_uFBO[m_ui32CurrentBuffer]);
		break;
#if !defined(EGL_NOT_PRESENT)
		case ePBuffer:
			// Switch the render target to the pBuffer
			if(!eglMakeCurrent(m_CurrentDisplay, m_PBufferSurface[m_ui32CurrentBuffer], m_PBufferSurface[m_ui32CurrentBuffer], m_CurrentContext))
			{
				PVRShellSet(prefExitMessage, "ERROR: Unable to make the pbuffer context current.\n");
				return false;
			}

			/*
				We no longer need the texture bound to the surface so we release the previous surface from
				all the textures it is bound to.
			*/

			if(!eglReleaseTexImage(m_CurrentDisplay, m_PBufferSurface[m_ui32CurrentBuffer], EGL_BACK_BUFFER))
			{
				PVRShellSet(prefExitMessage, "ERROR: Failed to release m_PBufferSurface.\n");
				return false;
			}
		break;
#endif
		default: {}
	}

	return true;
}

/*!****************************************************************************
 @Function		EndRenderToTexture
 @Return		bool		true if no error occured
 @Description	We have finished rendering to our texture. Switch rendering
				back to the backbuffer.
******************************************************************************/
bool OGLESFractal::EndRenderToTexture()
{
	switch(m_eR2TType)
	{
		case eFBO:
			m_Extensions.glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_CurrentFBO);
		break;
#if !defined(EGL_NOT_PRESENT)
		case ePBuffer:
			// We now switch back to the backbuffer for rendering.
			if(!eglMakeCurrent(m_CurrentDisplay, m_CurrentSurface, m_CurrentSurface, m_CurrentContext))
			{
				PVRShellOutputDebug("ERROR: Unable to make the main context current.\n");
				return false;
			}

			glBindTexture(GL_TEXTURE_2D, m_ui32Texture[m_ui32CurrentBuffer]);

			if(!eglBindTexImage(m_CurrentDisplay, m_PBufferSurface[m_ui32CurrentBuffer], EGL_BACK_BUFFER))
			{
				PVRShellOutputDebug("ERROR: Failed to bind m_PBufferSurface.\n");
				return false;
			}
		break;
#endif
		default: {}
	}

	return true;
}

#if !defined(EGL_NOT_PRESENT)
/*!****************************************************************************
 @Function		SelectEGLConfig
 @Description	Finds an EGL config with required options based on Mode Requested - for PBuffer
******************************************************************************/
EGLConfig OGLESFractal::SelectEGLConfig()
{
	EGLConfig EglConfig = 0;
	EGLint i32ConfigID;
	EGLint i32BufferSize;
	EGLint i32SampleBuffers;
	EGLint i32Samples;

	// Get the colour buffer size and the anti-aliasing parameters of the current surface so we can create
	// a PBuffer surface that matches.
	EGLDisplay eglDisplay = eglGetCurrentDisplay();
	eglQueryContext(eglDisplay, eglGetCurrentContext(), EGL_CONFIG_ID, &i32ConfigID);

	eglGetConfigAttrib(eglDisplay, (EGLConfig) (size_t) i32ConfigID, EGL_BUFFER_SIZE,&i32BufferSize);
	eglGetConfigAttrib(eglDisplay, (EGLConfig) (size_t) i32ConfigID, EGL_SAMPLE_BUFFERS,&i32SampleBuffers);
	eglGetConfigAttrib(eglDisplay, (EGLConfig) (size_t) i32ConfigID, EGL_SAMPLES,&i32Samples);

    EGLint i32ConfigNo;

	// Setup the configuration list for our surface.
    EGLint conflist[] =
	{
		EGL_CONFIG_CAVEAT, EGL_NONE,
		/*
			Tell it the minimum size we want for our colour buffer, depth size and
			anti-aliasing settings so eglChooseConfig will choose a config that is
			a good match for our window context so we only need a single context.
		*/
		EGL_BUFFER_SIZE, i32BufferSize,
		EGL_DEPTH_SIZE, 16,

		EGL_SAMPLE_BUFFERS, i32SampleBuffers,
		EGL_SAMPLES, i32Samples,

		// The PBuffer bit is the important part as it shows we want a PBuffer
		EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
		EGL_BIND_TO_TEXTURE_RGB, EGL_TRUE,
		EGL_NONE
	};

	// Find and return the config
    if(!eglChooseConfig(eglDisplay, conflist, &EglConfig, 1, &i32ConfigNo) || i32ConfigNo != 1)
	{
		PVRShellOutputDebug("Error: Failed to find a suitable config.\n");
		return 0;
    }

    return EglConfig;
}
#endif

/*!****************************************************************************
 @Function		NewDemo
 @Return		PVRShell*		The demo supplied by the user
 @Description	This function must be implemented by the user of the shell.
				The user should return its PVRShell object defining the
				behaviour of the application.
******************************************************************************/
PVRShell* NewDemo()
{
	return new OGLESFractal();
}

/*****************************************************************************
 End of file (OGLESFractal.cpp)
*****************************************************************************/

