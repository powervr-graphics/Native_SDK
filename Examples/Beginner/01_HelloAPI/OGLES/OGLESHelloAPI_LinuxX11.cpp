/*******************************************************************************************************************************************

 @File         OGLESHelloAPI_LinuxX11.cpp

 @Title        OpenGL ES 1.x HelloAPI Tutorial

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     

 @Description  Basic Tutorial that shows step-by-step how to initialize OpenGL ES 1.x, use it for drawing a triangle and terminate it.
               Entry Point: main

*******************************************************************************************************************************************/
/*******************************************************************************************************************************************
 Include Files
*******************************************************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "X11/Xlib.h"
#include "X11/Xutil.h"

#include <EGL/egl.h>
#include <GLES/gl.h>

/*******************************************************************************************************************************************
 Defines
*******************************************************************************************************************************************/
// Name of the application
#define APPLICATION_NAME "HelloAPI"

// Width and height of the window
#define WINDOW_WIDTH	800
#define WINDOW_HEIGHT	600

// Index to bind the attributes to vertex shaders
#define VERTEX_ARRAY	0

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
		printf("%s failed (%x).\n", functionLastCalled, lastError);
		return false;
	}

	return true;
}

/*!*****************************************************************************************************************************************
 @Function		HandleX11Errors
 @Input			nativeDisplay               Handle to the display
 @Input			error                       The error event to handle
 @Return		Result code to send to the X window system
 @Description	Processes event messages for the main window
*******************************************************************************************************************************************/
int HandleX11Errors(Display *nativeDisplay, XErrorEvent *error)
{
	// Get the X Error
	char errorStringBuffer[256];
	XGetErrorText(nativeDisplay, error->error_code, errorStringBuffer, 256);

	// Print the error
	printf("%s", errorStringBuffer);

	// Exit the application
	exit(-1);

	return 0;
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
		printf("%s failed (%x).\n", functionLastCalled, lastError);
		return false;
	}

	return true;
}

/*******************************************************************************************************************************************
 Application Functions
*******************************************************************************************************************************************/

/*!*****************************************************************************************************************************************
 @Function		CreateNativeDisplay
 @Output		nativeDisplay				Native display to create
 @Return		Whether the function succeeded or not.
 @Description	Creates a native isplay for the application to render into.
*******************************************************************************************************************************************/
bool CreateNativeDisplay(Display** nativeDisplay) 
{
	// Check for a valid display
	if (!nativeDisplay)
	{
		return false;
	}

	// Open the display
	*nativeDisplay = XOpenDisplay( 0 );
	if (!*nativeDisplay)
	{
		printf("Error: Unable to open X display\n");
		return false;
	}

	return true;
}

/*!*****************************************************************************************************************************************
 @Function		CreateNativeWindow
 @Input			nativeDisplay				Native display used by the application
 @Output		nativeWindow			    Native window type to create
 @Return		Whether the function succeeded or not.
 @Description	Creates a native window for the application to render into.
*******************************************************************************************************************************************/
bool CreateNativeWindow(Display* nativeDisplay, Window* nativeWindow) 
{
	// Get the default screen for the display
	int defaultScreen = XDefaultScreen(nativeDisplay);

	// Get the default depth of the display
	int defaultDepth = DefaultDepth(nativeDisplay, defaultScreen);

	// Select a visual info
	XVisualInfo* visualInfo = new XVisualInfo;
	XMatchVisualInfo( nativeDisplay, defaultScreen, defaultDepth, TrueColor, visualInfo);
	if (!visualInfo)
	{
		printf("Error: Unable to acquire visual\n");
		return false;
	}

	// Get the root window for the display and default screen
	Window rootWindow = RootWindow(nativeDisplay, defaultScreen);
	
	// Create a colour map from the display, root window and visual info
	Colormap colourMap = XCreateColormap(nativeDisplay, rootWindow, visualInfo->visual, AllocNone);

	// Now setup the final window by specifying some attributes
	XSetWindowAttributes windowAttributes;

	// Set the colour map that was just created
	windowAttributes.colormap = colourMap;
	
	// Set events that will be handled by the app, add to these for other events.
	windowAttributes.event_mask = StructureNotifyMask | ExposureMask | ButtonPressMask;

	// Create the window
	*nativeWindow =XCreateWindow(nativeDisplay,               // The display used to create the window
	                             rootWindow,                   // The parent (root) window - the desktop
						  		 0,                            // The horizontal (x) origin of the window
								 0,                            // The vertical (y) origin of the window
								 WINDOW_WIDTH,                 // The width of the window
								 WINDOW_HEIGHT,                // The height of the window
								 0,                            // Border size - set it to zero
	                             visualInfo->depth,            // Depth from the visual info
								 InputOutput,                  // Window type - this specifies InputOutput.
								 visualInfo->visual,           // Visual to use
								 CWEventMask | CWColormap,     // Mask specifying these have been defined in the window attributes
								 &windowAttributes);           // Pointer to the window attribute structure

	// Make the window viewable by mapping it to the display
	XMapWindow(nativeDisplay, *nativeWindow);

	// Set the window title
	XStoreName(nativeDisplay, *nativeWindow, APPLICATION_NAME);
	
	// Setup the window manager protocols to handle window deletion events
	Atom windowManagerDelete = XInternAtom(nativeDisplay, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(nativeDisplay, *nativeWindow, &windowManagerDelete , 1);

	// Delete the visual info
	delete visualInfo;

	return true;
}

/*!*****************************************************************************************************************************************
 @Function		CreateEGLDisplay
 @Input			nativeDisplay               The native display used by the application
 @Output		eglDisplay				    EGLDisplay created from nativeDisplay
 @Return		Whether the function succeeded or not.
 @Description	Creates an EGLDisplay from a native native display, and initialises it.
*******************************************************************************************************************************************/
bool CreateEGLDisplay( Display* nativeDisplay, EGLDisplay &eglDisplay ) 
{
	/*	Get an EGL display.
		EGL uses the concept of a "display" which in most environments corresponds to a single physical screen. After creating a native
		display for a given windowing system, EGL can use this handle to get a corresponding EGLDisplay handle to it for use in rendering.
		Should this fail, EGL is usually able to provide access to a default display.
	*/
	eglDisplay = eglGetDisplay((EGLNativeDisplayType)nativeDisplay);
	// If a display couldn't be obtained, return an error.
	if (eglDisplay == EGL_NO_DISPLAY)
	{
		printf("Failed to get an EGLDisplay");
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
		printf("Failed to initialise the EGLDisplay");
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
		printf("Failed to choose a suitable config.");
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
bool CreateEGLSurface( Window nativeWindow, EGLDisplay eglDisplay, EGLConfig eglConfig, EGLSurface& eglSurface) 
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
 @Input			nativeDisplay				The native display used by the application
 @Return		Whether the function succeeds or not.
 @Description	Renders the scene to the framebuffer. Usually called within a loop.
*******************************************************************************************************************************************/
bool RenderScene( EGLDisplay eglDisplay, EGLSurface eglSurface, Display* nativeDisplay ) 
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

	// Check for messages from the windowing system.
	int numberOfMessages = XPending(nativeDisplay);
	for( int i = 0; i < numberOfMessages; i++ )
	{
		XEvent event;
		XNextEvent(nativeDisplay, &event);

		switch( event.type )
		{
			// Exit on window close
		case ClientMessage:
			// Exit on mouse click
		case ButtonPress:
		case DestroyNotify:
			return false;
		default:
			break;
		}
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

/*!*****************************************************************************************************************************************
 @Function		ReleaseWindowAndDisplay
 @Input			nativeDisplay               The native display to release
 @Input			nativeWindow                The native window to destroy
 @Description	Releases all resources allocated by the windowing system
*******************************************************************************************************************************************/
void ReleaseNativeResources(Display* nativeDisplay, Window nativeWindow) 
{
	// Destroy the window
	if (nativeWindow)
	{
		XDestroyWindow(nativeDisplay, nativeWindow);
	}

	// Release the display.
	if (nativeDisplay)
	{
		XCloseDisplay(nativeDisplay);
	}
}

/*!*****************************************************************************************************************************************
 @Function		main
 @Input			argc                        Number of arguments passed to the application, ignored.
 @Input			argv           Command line strings passed to the application, ignored.
 @Return		Result code to send to the Operating System
 @Description	Main function of the program, executes other functions.
*******************************************************************************************************************************************/
int main(int /*argc*/, char **/*argv*/)
{
	// X11 variables
	Display* nativeDisplay = NULL;
	Window nativeWindow = 0;

	// EGL variables
	EGLDisplay			eglDisplay = NULL;
	EGLConfig			eglConfig = NULL;
	EGLSurface			eglSurface = NULL;
	EGLContext			eglContext = NULL;

	// A vertex buffer object to store our model data.
	GLuint vertexBuffer = 0;

	// Get access to a native display
	if (!CreateNativeDisplay(&nativeDisplay))
	{
		goto cleanup;
	}
	
	// Setup the windowing system, create a window
	if (!CreateNativeWindow(nativeDisplay, &nativeWindow))
	{
		goto cleanup;
	}
	
	// Create and Initialise an EGLDisplay from the native display
	if (!CreateEGLDisplay(nativeDisplay, eglDisplay))
	{
		goto cleanup;
	}

	// Choose an EGLConfig for the application, used when setting up the rendering surface and EGLContext
	if (!ChooseEGLConfig(eglDisplay, eglConfig))
	{
		goto cleanup;
	}

	// Create an EGLSurface for rendering from the native window
	if (!CreateEGLSurface(nativeWindow, eglDisplay, eglConfig, eglSurface))
	{
		goto cleanup;
	}

	// Setup the EGL Context from the other EGL constructs created so far, so that the application is ready to submit OpenGL ES commands
	if (!SetupEGLContext(eglDisplay, eglConfig, eglSurface, eglContext))
	{
		goto cleanup;
	}

	// Initialise the vertex data in the application
	if (!InitialiseBuffer(vertexBuffer))
	{
		goto cleanup;
	}	
	
	// Renders a triangle for 800 frames using the state setup in the previous function
	for (int i = 0; i < 800; ++i)
	{
		if (!RenderScene(eglDisplay, eglSurface, nativeDisplay))
		{
			break;
		}
	}

	// Release any resources we created in the Initialise functions
	DeInitialiseBuffer(vertexBuffer);

cleanup:
	// Release the EGL State
	ReleaseEGLState(eglDisplay);

	// Release the windowing system resources
	ReleaseNativeResources(nativeDisplay, nativeWindow);

	// Destroy the eglWindow
	return 0;
}

/*******************************************************************************************************************************************
 End of file (OGLESHelloAPI_LinuxX11.cpp)
*******************************************************************************************************************************************/
