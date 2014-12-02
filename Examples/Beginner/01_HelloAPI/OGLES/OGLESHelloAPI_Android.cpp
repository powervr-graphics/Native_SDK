/*******************************************************************************************************************************************

 @File         OGLESHelloAPI_Android.cpp

 @Title        OpenGL ES 1.x HelloAPI Tutorial

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     

 @Description  Basic Tutorial that shows step-by-step how to initialize OpenGL ES 1.x, use it for drawing a triangle and terminate it.
               Entry Point: android_main

*******************************************************************************************************************************************/
/*******************************************************************************************************************************************
 Include Files
*******************************************************************************************************************************************/
#include <stdio.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/log.h>
#include <android/window.h>
#include <android_native_app_glue.h>

/*******************************************************************************************************************************************
 Defines
*******************************************************************************************************************************************/
// Name of the application
#define APPLICATION_NAME "HelloAPI"

// Index to bind the attributes to vertex shaders
#define VERTEX_ARRAY	0

/*******************************************************************************************************************************************
 Structs
*******************************************************************************************************************************************/

// Data structure containing variables used in the application
struct SHelloAPIData
{
	// EGL variables
	EGLDisplay          eglDisplay;
	EGLConfig			eglConfig;
	EGLContext          eglContext;
	EGLSurface          eglSurface;
	
	// Handles for the vertex buffer object
	GLuint              vertexBuffer;

	// Should the app still be animating?
	bool                isAnimating;
	
	// Is everything required initialised?
	bool				isInitialised;

	// Has an error occurred?
	bool				errorOccurred;
};

/*******************************************************************************************************************************************
 Helper Functions
*******************************************************************************************************************************************/

/*!*****************************************************************************************************************************************
 @Function		TestEGLError
 @Input			functionLastCalled          Function which triggered the error
 @Return		True if no EGL error was detected
 @Description	Tests for an EGL error and prints it.
*******************************************************************************************************************************************/
bool TestEGLError(const char* functionLastCalled)
{
	/*	eglGetError returns the last error that occurred using EGL, not necessarily the status of the last called function. The user has to
		check after every single EGL call or at least once every frame. Usually this would be for debugging only, but for this example
		it is enabled always.
	*/
	EGLint lastError = eglGetError();
	if (lastError != EGL_SUCCESS)
	{
		__android_log_print(ANDROID_LOG_ERROR, APPLICATION_NAME, "%s failed (%d).\n", functionLastCalled, lastError);
		return false;
	}

	return true;
}

/*!*****************************************************************************************************************************************
 @Function		TestGLError
 @Input			functionLastCalled          Function which triggered the error
 @Return		True if no GL error was detected
 @Description	Tests for an GL error and prints it in a message box.
*******************************************************************************************************************************************/
bool TestGLError(const char* functionLastCalled)
{
	/*	glGetError returns the last error that occurred using OpenGL ES, not necessarily the status of the last called function. The user 
		has to check after every single OpenGL ES call or at least once every frame. Usually this would be for debugging only, but for this
		example it is enabled always
	*/
	GLenum lastError = glGetError();
	if (lastError != GL_NO_ERROR)
	{
		__android_log_print(ANDROID_LOG_ERROR, APPLICATION_NAME, "%s failed (%d).\n", functionLastCalled, lastError);
		return false;
	}

	return true;
}

/*******************************************************************************************************************************************
 Application Functions
*******************************************************************************************************************************************/

/*!*****************************************************************************************************************************************
 @Function		CreateEGLDisplay
 @Output		eglDisplay				    EGLDisplay created by the function
 @Return		Whether the function succeeded or not.
 @Description	Creates an EGLDisplay and initialises it.
*******************************************************************************************************************************************/
bool CreateEGLDisplay(EGLDisplay &eglDisplay) 
{
	/*	Get an EGL display.
		EGL uses the concept of a "display" which in most environments corresponds to a single physical screen. After creating a native
		display for a given windowing system, EGL can use this handle to get a corresponding EGLDisplay handle to it for use in rendering.
	*/
	eglDisplay = eglGetDisplay((EGLNativeDisplayType)0);
	if (eglDisplay == EGL_NO_DISPLAY)
	{
		__android_log_print(ANDROID_LOG_ERROR, APPLICATION_NAME, "Failed to get an EGLDisplay");
		return false;
	}

	/*	Initialize EGL.
		EGL has to be initialized with the display obtained in the previous step. All EGL functions other than eglGetDisplay 
		and eglGetError need an initialised EGLDisplay. 
		If an application is not interested in the EGL version number it can just pass NULL for the second and third parameters, but they 
		are queried here for illustration purposes.
	*/
	EGLint eglMajorVersion, eglMinorVersion;
	if (!eglInitialize(eglDisplay, &eglMajorVersion, &eglMinorVersion))
	{
		__android_log_print(ANDROID_LOG_ERROR, APPLICATION_NAME, "Failed to initialise the EGLDisplay");
		return false;
	}

	return true;
}

/*!*****************************************************************************************************************************************
 @Function		ChooseEGLConfig
 @Input			eglDisplay                  The EGLDisplay used by the application
 @Output		eglConfig                   The EGLConfig chosen by the function
 @Return		Whether the function succeeded or not.
 @Description	Chooses an appropriate EGLConfig and return it.
*******************************************************************************************************************************************/
bool ChooseEGLConfig( EGLDisplay eglDisplay, EGLConfig& eglConfig ) 
{
	/*	Specify the required configuration attributes.
		An EGL "configuration" describes the capabilities an application requires and the type of surfaces that can be used for drawing.
		Each implementation exposes a number of different configurations, and an application needs to describe to EGL what capabilities it
		requires so that an appropriate one can be chosen. The first step in doing this is to create an attribute list, which is an array
		of key/value pairs which describe particular capabilities requested. In this application nothing special is required so we can query
		the minimum of needing it to render to a window, and being OpenGL ES 1.x capable.
	*/
	const EGLint configurationAttributes[] =
	{
		EGL_SURFACE_TYPE,		EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE,	EGL_OPENGL_ES_BIT,
		EGL_NONE
	};

	/*	Find a suitable EGLConfig
		eglChooseConfig is provided by EGL to provide an easy way to select an appropriate configuration. It takes in the capabilities
		specified in the attribute list, and returns a list of available configurations that match or exceed the capabilities requested.
		Details of all the possible attributes and how they are selected for by this function are available in the EGL reference pages here:
		http://www.khronos.org/registry/egl/sdk/docs/man/xhtml/eglChooseConfig.html
		It is also possible to simply get the entire list of configurations and use a custom algorithm to choose a suitable one, as many
		advanced applications choose to do. For this application however, taking the first EGLConfig that the function returns suits
		its needs perfectly, so we limit it to returning a single EGLConfig.
	*/
	EGLint configsReturned;
	if (!eglChooseConfig(eglDisplay, configurationAttributes, &eglConfig, 1, &configsReturned) || (configsReturned != 1))
	{
		__android_log_print(ANDROID_LOG_ERROR, APPLICATION_NAME, "Failed to choose a suitable config.\n");
		return false;
	}

	return true;
}

/*!*****************************************************************************************************************************************
 @Function		CreateEGLSurface
 @Input			nativeWindow                A native window that's been created
 @Input			eglDisplay                  The EGLDisplay used by the application
 @Input			eglConfig                   An EGLConfig chosen by the application
 @Output		eglSurface					The EGLSurface created from the native window.
 @Return		Whether the function succeeds or not.
 @Description	Creates an EGLSurface from a native window
*******************************************************************************************************************************************/
bool CreateEGLSurface( ANativeWindow* nativeWindow, EGLDisplay eglDisplay, EGLConfig eglConfig, EGLSurface& eglSurface) 
{
	/*	Create an EGLSurface for rendering.
		Using a native window created earlier and a suitable eglConfig, a surface is created that can be used to render OpenGL ES calls to.
		There are three main surface types in EGL, which can all be used in the same way once created but work slightly differently:
		 - Window Surfaces  - These are created from a native window and are drawn to the screen.
		 - Pixmap Surfaces  - These are created from a native windowing system as well, but are offscreen and are not displayed to the user.
		 - PBuffer Surfaces - These are created directly within EGL, and like Pixmap Surfaces are offscreen and thus not displayed.
		The offscreen surfaces are useful for non-rendering contexts and in certain other scenarios, but for most applications the main
		surface used will be a window surface as performed below.
	*/
	eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, (EGLNativeWindowType)nativeWindow, NULL);
	if (!TestEGLError("eglCreateWindowSurface"))
	{
		return false;
	}

	return true;
}

/*!*****************************************************************************************************************************************
 @Function		SetupEGLContext
 @Input			eglDisplay                  The EGLDisplay used by the application
 @Input			eglConfig                   An EGLConfig chosen by the application
 @Input			eglSurface					The EGLSurface created from the native window.
 @Output		eglContext                  The EGLContext created by this function
 @Input			nativeWindow                A native window, used to display error messages
 @Return		Whether the function succeeds or not.
 @Description	Sets up the EGLContext, creating it and then installing it to the current thread.
*******************************************************************************************************************************************/
bool SetupEGLContext( EGLDisplay eglDisplay, EGLConfig eglConfig, EGLSurface eglSurface, EGLContext& eglContext ) 
{
	/*	Create a context.
		EGL has to create what is known as a context for OpenGL ES. The concept of a context is OpenGL ES's way of encapsulating any
		resources and state. What appear to be "global" functions in OpenGL actually only operate on the current context. A context
		is required for any operations in OpenGL ES.
		Similar to an EGLConfig, a context takes in a list of attributes specifying some of its capabilities. However in most cases this
		is limited to just requiring the version of the OpenGL ES context required - In this case, OpenGL ES 1.x is required which is the
		default, so there's no need to specify anything.
	*/

	// Create the context with the context attributes supplied
	eglContext = eglCreateContext(eglDisplay, eglConfig, NULL, NULL);
	if (!TestEGLError("eglCreateContext"))
	{
		return false;
	}

	/*	Bind the context to the current thread.
		Due to the way OpenGL uses global functions, contexts need to be made current so that any function call can operate on the correct
		context. Specifically, make current will bind the context to the thread it's called from, and unbind it from any others. To use
		multiple contexts at the same time, users should use multiple threads and synchronise between them.
	*/
	eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
	if (!TestEGLError("eglMakeCurrent"))
	{
		return false;
	}

	return true;
}

/*!*****************************************************************************************************************************************
 @Function		InitialiseBuffer
 @Output		vertexBuffer                Handle to a vertex buffer object
 @Return		Whether the function succeeds or not.
 @Description	Initialises shaders, buffers and other state required to begin rendering with OpenGL ES
*******************************************************************************************************************************************/
bool InitialiseBuffer(GLuint &vertexBuffer) 
{
	/*	Concept: Vertices
		When rendering a polygon or model to screen, OpenGL ES has to be told where to draw the object, and more fundamentally what shape 
		it is. The data used to do this is referred to as vertices, points in 3D space which are usually collected into groups of three 
		to render as triangles. Fundamentally, any advanced 3D shape in OpenGL ES is constructed from a series of these vertices - each 
		vertex representing one corner of a polygon.
	*/

	/*	Concept: Buffer Objects
		To operate on any data, OpenGL first needs to be able to access it. The GPU maintains a separate pool of memory it uses independent
		of the CPU. Whilst on many embedded systems these are in the same physical memory, the distinction exists so that they can use and
		allocate memory without having to worry about synchronising with any other processors in the device.
		To this end, data needs to be uploaded into buffers, which are essentially a reserved bit of memory for the GPU to use. By creating
		a buffer and giving it some data we can tell the GPU how to render a triangle.
	*/

	// Vertex data containing the positions of each point of the triangle
	GLfloat vertexData[] = {-0.4f,-0.4f, 0.0f,  // Bottom Left
	                         0.4f,-0.4f, 0.0f,  // Bottom Right
	                         0.0f, 0.4f, 0.0f}; // Top Middle

	// Generate a buffer object
	glGenBuffers(1, &vertexBuffer);

	// Bind buffer as an vertex buffer so we can fill it with data
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	
	/*	Set the buffer's size, data and usage
		Note the last argument - GL_STATIC_DRAW. This tells the driver that we intend to read from the buffer on the GPU, and don't intend
		to modify the data until we're done with it.		
	*/
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	if (!TestGLError("glBufferData"))
	{
		return false;
	}

	return true;
}

/*!*****************************************************************************************************************************************
 @Function		RenderScene
 @Input			eglDisplay                  The EGLDisplay used by the application
 @Input			eglSurface					The EGLSurface created from the native window.
 @Return		Whether the function succeeds or not.
 @Description	Renders the scene to the framebuffer. Usually called within a loop.
*******************************************************************************************************************************************/
bool RenderScene( EGLDisplay eglDisplay, EGLSurface eglSurface) 
{
	/*	Set the clear color
		At the start of a frame, generally you clear the image to tell OpenGL ES that you're done with whatever was there before and want to
		draw a new frame. In order to do that however, OpenGL ES needs to know what colour to set in the image's place. glClearColor
		sets this value as 4 floating point values between 0.0 and 1.x, as the Red, Green, Blue and Alpha channels. Each value represents
		the intensity of the particular channel, with all 0.0 being transparent black, and all 1.x being opaque white. Subsequent calls to
		glClear with the colour bit will clear the frame buffer to this value.
		The functions glClearDepth and glClearStencil allow an application to do the same with depth and stencil values respectively.
	*/
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	/*	Clears the color buffer.
		glClear is used here with the Colour Buffer to clear the colour. It can also be used to clear the depth or stencil buffer using 
		GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT, respectively.
	*/
	glClear(GL_COLOR_BUFFER_BIT);
	
	// Enable the vertex array
	glEnableClientState(GL_VERTEX_ARRAY);

	// Sets the vertex data to this attribute index, with the number of floats in each position
	glVertexPointer(3, GL_FLOAT, 3*sizeof(GLfloat), (GLvoid*)0);
	if (!TestGLError("glVertexAttribPointer"))
	{
		return false;
	}

	// Set a color to render
	glColor4f(1.0f, 1.0f, 0.66f, 1.0f);

	/*	Draw the triangle
		glDrawArrays is a draw call, and executes the shader program using the vertices and other state set by the user. Draw calls are the
		functions which tell OpenGL ES when to actually draw something to the framebuffer given the current state.
		glDrawArrays causes the vertices to be submitted sequentially from the position given by the "first" argument until it has processed
		"count" vertices. Other draw calls exist, notably glDrawElements which also accepts index data to allow the user to specify that
		some vertices are accessed multiple times, without copying the vertex multiple times.
		Others include versions of the above that allow the user to draw the same object multiple times with slightly different data, and
		a version of glDrawElements which allows a user to restrict the actual indices accessed.
	*/
	glDrawArrays(GL_TRIANGLES, 0, 3);
	if (!TestGLError("glDrawArrays"))
	{
		return false;
	}

	/*	Present the display data to the screen.
		When rendering to a Window surface, OpenGL ES is double buffered. This means that OpenGL ES renders directly to one frame buffer, 
		known as the back buffer, whilst the display reads from another - the front buffer. eglSwapBuffers signals to the windowing system
		that OpenGL ES 1.x has finished rendering a scene, and that the display should now draw to the screen from the new data. At the same
		time, the front buffer is made available for OpenGL ES 1.x to start rendering to. In effect, this call swaps the front and back 
		buffers.
	*/
	if (!eglSwapBuffers(eglDisplay, eglSurface) )
	{
		TestEGLError("eglSwapBuffers");
		return false;
	}
	
	return true;
}

/*!*****************************************************************************************************************************************
 @Function		DeInitialiseGLState
 @Input			vertexBuffer                Handle to a vertex buffer object
 @Description	Releases the resources created by "InitialiseGLState"
*******************************************************************************************************************************************/
void DeInitialiseBuffer( GLuint vertexBuffer ) 
{
	// Delete the VBO as it is no longer needed
	glDeleteBuffers(1, &vertexBuffer);
}

/*!*****************************************************************************************************************************************
 @Function		ReleaseEGLState
 @Input			eglDisplay                   The EGLDisplay used by the application
 @Description	Releases all resources allocated by EGL
*******************************************************************************************************************************************/
void ReleaseEGLState(EGLDisplay eglDisplay) 
{
	if(eglDisplay != NULL)
	{
		// To release the resources in the context, first the context has to be released from its binding with the current thread.
		eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

		// Terminate the display, and any resources associated with it (including the EGLContext)
		eglTerminate(eglDisplay);
	}
}

static void HandleAndroidCommands(struct android_app* application, int32_t commandID)
{
	SHelloAPIData* applicationData = (SHelloAPIData*)application->userData;

	switch (commandID)
	{
	case APP_CMD_INIT_WINDOW:
		// The window is being shown, get it ready.
		if (application->window != NULL)
		{
			if (!CreateEGLDisplay(applicationData->eglDisplay) )
			{
				applicationData->errorOccurred = true;
			}

			if (applicationData->errorOccurred != true)
			{
				if (!ChooseEGLConfig(applicationData->eglDisplay, applicationData->eglConfig) )
				{
					applicationData->errorOccurred = true;
				}

				if (applicationData->errorOccurred != true)
				{
					if (!CreateEGLSurface(application->window, applicationData->eglDisplay, 
						applicationData->eglConfig, applicationData->eglSurface) )
					{
						applicationData->errorOccurred = true;
					}

					if (applicationData->errorOccurred != true)
					{
						if (!SetupEGLContext(applicationData->eglDisplay, applicationData->eglConfig, 
							applicationData->eglSurface, applicationData->eglContext) )
						{
							applicationData->errorOccurred = true;
						}

						if (applicationData->errorOccurred != true)
						{
							if (!InitialiseBuffer(applicationData->vertexBuffer) )
							{
								applicationData->errorOccurred = true;
							}
							else
							{
								applicationData->isInitialised = true;
							}
						}
					}
				}
			}

		}
	case APP_CMD_RESUME:
		applicationData->isAnimating = true;
		break;

	case APP_CMD_TERM_WINDOW:
		{
			DeInitialiseBuffer(applicationData->vertexBuffer);

			ReleaseEGLState(applicationData->eglDisplay);
			
			applicationData->isInitialised = false;
		}
	case APP_CMD_PAUSE:
	case APP_CMD_SAVE_STATE:
		applicationData->isAnimating = false;
		break;
	}
}

/*!*****************************************************************************************************************************************
 @Function		android_main
 @Input			application                 Application structure passed in from the OS
 @Description	Main function of the program, executes other functions.
*******************************************************************************************************************************************/
void android_main(struct android_app* application)
{
	// Application Data
	SHelloAPIData applicationData =
	{
		NULL,
		NULL,
		NULL,
		NULL,
		0,
		false,
		false,
		false
	};

	// Make sure android glue (events functions) aren't stripped out by the compiler
	app_dummy();

	// Set the user data of the application to our application data.
	application->userData = &applicationData;

	// Set the command handler to the custom handling function
	application->onAppCmd = HandleAndroidCommands;

	// Event handling variables
	int eventIdentifier;
	int events;
	struct android_poll_source* pollSource;

	// Renders a triangle whilst the android application is active
	while(true)
	{
		// Block while there are events to process or if we're not animating
		while( (eventIdentifier=ALooper_pollAll(applicationData.isInitialised && applicationData.isAnimating ? 0 : -1, NULL, &events, (void**)&pollSource) ) >= 0 )
		{
			if (pollSource != NULL)
			{
				pollSource->process(application, pollSource);
			}

			// Check for early exit that hasn't been handled by the android commands system
			if (application->destroyRequested != 0)
			{
				// Release the GL vertex buffer
				DeInitialiseBuffer(applicationData.vertexBuffer);

				// Release EGL
				ReleaseEGLState(applicationData.eglDisplay);
				return;
			}
		}

		// Once events are processed, and assuming that animation is going to occur, continue animating
		if (applicationData.isAnimating)
		{
			if (!RenderScene(applicationData.eglDisplay, applicationData.eglSurface) )
			{
				break;
			}
		}
	}
}

/*******************************************************************************************************************************************
 End of file (OGLESHelloAPI_Android.cpp)
*******************************************************************************************************************************************/
