/******************************************************************************

 @File         OGLES3IntroducingPVRShell.cpp

 @Title        Introduces PVRShell

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows how to use the PowerVR framework for initialization. This
               framework allows platform abstraction so applications using it
               will work on any PowerVR enabled device.

******************************************************************************/
#include <stdio.h>
#include <string.h>
#if defined(__APPLE__) && defined (TARGET_OS_IPHONE)
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#else
#include <GLES3/gl3.h>
#endif

#include "PVRShell.h"

	/*
	*	Lesson 2: The PowerVR Shell
	*	===========================

	The PowerVR shell handles all OS specific initialisation code, and is
	extremely convenient for writing portable applications. It also has
	several built in command line features, which allow you to specify
	attributes like the backbuffer size, vsync and antialiasing modes.

	The code is constructed around a "PVRShell" superclass. You must define
	your app using a class which inherits from this, which should implement
	the following five methods, (which at execution time are essentially
	called in the order in which they are listed):


	InitApplication:	This is called before any API initialisation has
	taken place, and can be used to set up any application data which does
	not require API calls, for example object positions, or arrays containing
	vertex data, before they are uploaded.

	InitView:	This is called after the API has initialised, and can be
	used to do any remaining initialisation which requires API functionality.
	In this app, it is used to upload the vertex data.

	RenderScene:	This is called repeatedly to draw the geometry. Returning
	false from this function instructs the app to enter the quit sequence:

	ReleaseView:	This function is called before the API is released, and
	is used to release any API resources. In this app, it releases the
	vertex buffer.

	QuitApplication:	This is called last of all, after the API has been
	released, and can be used to free any leftover user allocated memory.


	The shell framework starts the application by calling a "NewDemo" function,
	which must return an instance of the PVRShell class you defined. We will
	now use the shell to create a "Hello triangle" app, similar to the previous
	one.

	 #include <EGL/egl.h>
	 #include <GLES3/gl3.h>
	 #include <GLES3/gl3ext.h>
	 #include <GLES3/gl3extimg.h>

	*/

/******************************************************************************
 Defines
******************************************************************************/

// Index to bind the attributes to vertex shaders
#define VERTEX_ARRAY	0

/*!****************************************************************************
 To use the shell, you have to inherit a class from PVRShell
 and implement the five virtual functions which describe how your application
 initializes, runs and releases the ressources.
******************************************************************************/
class OGLES3IntroducingPVRShell : public PVRShell
{
	// The vertex and fragment shader OpenGL handles
	GLuint m_uiVertexShader, m_uiFragShader;

	// The program object containing the 2 shader objects
	GLuint m_uiProgramObject;

	// VBO handle
	GLuint m_ui32Vbo;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();
};


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
bool OGLES3IntroducingPVRShell::InitApplication()
{
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
bool OGLES3IntroducingPVRShell::QuitApplication()
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
bool OGLES3IntroducingPVRShell::InitView()
{
	// Fragment and vertex shaders code
	const char* pszFragShader = "\
		#version 300 es\n\
		layout (location = 0) out lowp vec4 oColour;\
		void main (void)\
		{\
			oColour = vec4(1.0, 1.0, 0.66  ,1.0);\
		}";
	const char* pszVertShader = "\
		#version 300 es\n\
		#define VERTEX_ARRAY 0\n\
		layout (location = VERTEX_ARRAY) in highp vec4	myVertex;\
		uniform mediump mat4	myPMVMatrix;\
		void main(void)\
		{\
			gl_Position = myPMVMatrix * myVertex;\
		}";

	// Create the fragment shader object
	m_uiFragShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Load the source code into it
	glShaderSource(m_uiFragShader, 1, (const char**)&pszFragShader, NULL);

	// Compile the source code
	glCompileShader(m_uiFragShader);

	// Check if compilation succeeded
	GLint bShaderCompiled;
    glGetShaderiv(m_uiFragShader, GL_COMPILE_STATUS, &bShaderCompiled);
	if (!bShaderCompiled)
	{
		// An error happened, first retrieve the length of the log message
		int i32InfoLogLength, i32CharsWritten;
		glGetShaderiv(m_uiFragShader, GL_INFO_LOG_LENGTH, &i32InfoLogLength);

		// Allocate enough space for the message and retrieve it
		char* pszInfoLog = new char[i32InfoLogLength];
        glGetShaderInfoLog(m_uiFragShader, i32InfoLogLength, &i32CharsWritten, pszInfoLog);

		/*
			Displays the message in a dialog box when the application quits
			using the shell PVRShellSet function with first parameter prefExitMessage.
		*/
		char* pszMsg = new char[i32InfoLogLength+256];
		strcpy(pszMsg, "Failed to compile fragment shader: ");
		strcat(pszMsg, pszInfoLog);
		PVRShellSet(prefExitMessage, pszMsg);

		delete [] pszMsg;
		delete [] pszInfoLog;
		return false;
	}

	// Loads the vertex shader in the same way
	m_uiVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(m_uiVertexShader, 1, (const char**)&pszVertShader, NULL);
	glCompileShader(m_uiVertexShader);
    glGetShaderiv(m_uiVertexShader, GL_COMPILE_STATUS, &bShaderCompiled);
	if (!bShaderCompiled)
	{
		int i32InfoLogLength, i32CharsWritten;
		glGetShaderiv(m_uiVertexShader, GL_INFO_LOG_LENGTH, &i32InfoLogLength);
		char* pszInfoLog = new char[i32InfoLogLength];
        glGetShaderInfoLog(m_uiVertexShader, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
		char* pszMsg = new char[i32InfoLogLength+256];
		strcpy(pszMsg, "Failed to compile vertex shader: ");
		strcat(pszMsg, pszInfoLog);
		PVRShellSet(prefExitMessage, pszMsg);

		delete [] pszMsg;
		delete [] pszInfoLog;
		return false;
	}

	// Create the shader program
    m_uiProgramObject = glCreateProgram();

	// Attach the fragment and vertex shaders to it
    glAttachShader(m_uiProgramObject, m_uiFragShader);
    glAttachShader(m_uiProgramObject, m_uiVertexShader);

	// Bind the custom vertex attribute "myVertex" to location VERTEX_ARRAY
    glBindAttribLocation(m_uiProgramObject, VERTEX_ARRAY, "myVertex");

	// Link the program
    glLinkProgram(m_uiProgramObject);

	// Check if linking succeeded in the same way we checked for compilation success
    GLint bLinked;
    glGetProgramiv(m_uiProgramObject, GL_LINK_STATUS, &bLinked);
	if (!bLinked)
	{
		int i32InfoLogLength, i32CharsWritten;
		glGetProgramiv(m_uiProgramObject, GL_INFO_LOG_LENGTH, &i32InfoLogLength);
		char* pszInfoLog = new char[i32InfoLogLength];
		glGetProgramInfoLog(m_uiProgramObject, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
		char* pszMsg = new char[i32InfoLogLength+256];
		strcpy(pszMsg, "Failed to link program: ");
		strcat(pszMsg, pszInfoLog);
		PVRShellSet(prefExitMessage, pszMsg);

		delete [] pszMsg;
		delete [] pszInfoLog;
		return false;
	}

	// Actually use the created program
	glUseProgram(m_uiProgramObject);

	// Sets the clear color
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	// Create VBO for the triangle from our data

	// Vertex data
	GLfloat afVertices[] = {-0.4f,-0.4f,0.0f,  0.4f,-0.4f,0.0f,   0.0f,0.4f,0.0f};

	// Gen VBO
	glGenBuffers(1, &m_ui32Vbo);

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);

	// Set the buffer's data
	glBufferData(GL_ARRAY_BUFFER, 3 * (3 * sizeof(GLfloat)) /* 3 Vertices of 3 floats in size */, afVertices, GL_STATIC_DRAW);

	// Unbind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Enable culling
	glEnable(GL_CULL_FACE);
	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES3IntroducingPVRShell::ReleaseView()
{
	// Release Vertex buffer object.
	glDeleteBuffers(1, &m_ui32Vbo);

	// Frees the OpenGL handles for the program and the 2 shaders
	glDeleteProgram(m_uiProgramObject);
	glDeleteShader(m_uiVertexShader);
	glDeleteShader(m_uiFragShader);
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
bool OGLES3IntroducingPVRShell::RenderScene()
{
	// Matrix used for projection model view
	float afIdentity[] = {
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};

	/*
		Clears the color buffer.
		glClear() can also be used to clear the depth or stencil buffer
		(GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT)
	*/
	glClear(GL_COLOR_BUFFER_BIT);

	/*
		Bind the projection model view matrix (PMVMatrix) to
		the associated uniform variable in the shader
	*/

	// First gets the location of that variable in the shader using its name
	int i32Location = glGetUniformLocation(m_uiProgramObject, "myPMVMatrix");

	// Then passes the matrix to that variable
	glUniformMatrix4fv(i32Location, 1, GL_FALSE, afIdentity);

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);

	/*
	 Enable the custom vertex attribute at index VERTEX_ARRAY.
	 We previously binded that index to the variable in our shader "vec4 MyVertex;"
	*/
	glEnableVertexAttribArray(VERTEX_ARRAY);

	// Points to the data for this vertex attribute
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/*
	 Draws a non-indexed triangle array from the pointers previously given.
	 This function allows the use of other primitive types : triangle strips, lines, ...
	 For indexed geometry, use the function glDrawElements() with an index list.
	*/
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// Unbind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

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
	return new OGLES3IntroducingPVRShell();
}

/******************************************************************************
 End of file (OGLES3IntroducingPVRShell.cpp)
******************************************************************************/

