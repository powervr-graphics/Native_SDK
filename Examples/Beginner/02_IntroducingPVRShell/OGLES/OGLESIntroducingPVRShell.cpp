/*!*********************************************************************************************************************
\File         OGLESIntroducingPVRShell.cpp
\Title        Introduces PVRShell
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief        Shows how to use the PowerVR framework for initialization. This framework allows platform abstraction so applications
              using it will work on any PowerVR enabled device.
***********************************************************************************************************************/
#include <stdio.h>
#include <string.h>

#if defined(__APPLE__) && defined (TARGET_OS_IPHONE)
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#endif

#include "PVRShell/PVRShellNoPVRApi.h"

	/*
	*	The PowerVR Shell
	*	===========================

	The PowerVR shell handles all OS specific initialisation code, and is
	extremely convenient for writing portable applications. It also has
	several built in command line features, which allow you to specify
	attributes like the backbuffer size, vsync and antialiasing modes.

	The code is constructed around a "PVRShell" superclass. You must define
	your app using a class which inherits from this, which should implement
	the following five methods, (which at execution time are essentially
	called in the order in which they are listed):

	initApplication:	This is called before any API initialisation has
	taken place, and can be used to set up any application data which does
	not require API calls, for example object positions, or arrays containing
	vertex data, before they are uploaded.

	initView:	This is called after the API has initialized, and can be
	used to do any remaining initialisation which requires API functionality.
	In this app, it is used to upload the vertex data.

	renderFrame:	This is called repeatedly to draw the geometry. Returning
	false from this function instructs the app to enter the quit sequence:

	releaseView:	This function is called before the API is released, and
	is used to release any API resources. In this app, it releases the
	vertex buffer.

	quitApplication:	This is called last of all, after the API has been
	released, and can be used to free any leftover user allocated memory.

	The shell framework starts the application by calling a "NewDemo" function,
	which must return an instance of the PVRShell class you defined. We will
	now use the shell to create a "Hello triangle" app, similar to the previous
	one.

	*/

// Index to bind the attributes to vertex shaders
const pvr::uint32 VertexArray = 0;

/*!*********************************************************************************************************************
 To use the shell, you have to inherit a class from PVRShell
 and implement the five virtual functions which describe how your application initializes, runs and releases the resources.
***********************************************************************************************************************/
class OGLESIntroducingPVRShell : public pvr::Shell
{
	// The vertex and fragment shader OpenGL handles
	pvr::uint32 vertexShader, fragShader;

	// The program object containing the 2 shader objects
	pvr::uint32 programObject;

	// VBO handle
	pvr::uint32 vbo;

public:
    // following function must be override
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();
};

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in initApplication() will be called by pvr::Shell once per run, before the rendering context is created.
		Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
		If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result OGLESIntroducingPVRShell::initApplication(){	return pvr::Result::Success; }

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.
        If the rendering context is lost, QuitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result OGLESIntroducingPVRShell::quitApplication(){	return pvr::Result::Success; }

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occured
\brief	Code in initView() will be called by pvr::Shell upon initialization or after a change in the rendering context.
		Used to initialize variables that are dependant on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result OGLESIntroducingPVRShell::initView()
{
	// Fragment and vertex shaders code
	const char* pszFragShader = "\
		void main (void)\
		{\
			gl_FragColor = vec4(1.0, 1.0, 0.66  ,1.0);\
		}";

	const char* pszVertShader = "\
		attribute highp vec4	myVertex;\
		uniform mediump mat4	myPMVMatrix;\
		void main(void)\
		{\
			gl_Position = myPMVMatrix * myVertex;\
		}";

	// Create the fragment shader object
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Load the source code into it
	glShaderSource(fragShader, 1, (const char**)&pszFragShader, NULL);

	// Compile the source code
	glCompileShader(fragShader);

	// Check if compilation succeeded
	pvr::int32 bShaderCompiled;
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &bShaderCompiled);
	if (!bShaderCompiled)
	{
		// An error happened, first retrieve the length of the log message
		int infoLogLength, numCharsWritten;
		glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &infoLogLength);

		// Allocate enough space for the message and retrieve it
		std::vector<char> infoLog; infoLog.resize(infoLogLength);
        glGetShaderInfoLog(fragShader, infoLogLength, &numCharsWritten, infoLog.data());

        //  Displays the message in a dialog box when the application quits
        //  using the shell PVRShellSet function with first parameter prefExitMessage.
		std::string message("Failed to compile fragment shader: ");
		message += infoLog.data();
		this->setExitMessage(message.c_str());
		return pvr::Result::InvalidData;
	}

	// Loads the vertex shader in the same way
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, (const char**)&pszVertShader, NULL);
	glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &bShaderCompiled);
	if (!bShaderCompiled)
	{
		int infoLogLength, numCharsWritten;
		glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLogLength);
		
		std::vector<char> infoLog; infoLog.resize(infoLogLength);
        glGetShaderInfoLog(vertexShader, infoLogLength, &numCharsWritten, infoLog.data());

		std::string message("Failed to compile vertex shader: ");
		message += infoLog.data();
		this->setExitMessage(message.c_str());
		return pvr::Result::InvalidData;
	}

	// Create the shader program
    programObject = glCreateProgram();

	// Attach the fragment and vertex shaders to it
    glAttachShader(programObject, fragShader);
    glAttachShader(programObject, vertexShader);

	// Bind the custom vertex attribute "myVertex" to location VERTEX_ARRAY
    glBindAttribLocation(programObject, VertexArray, "myVertex");

	// Link the program
    glLinkProgram(programObject);

	// Check if linking succeeded in the same way we checked for compilation success
    pvr::int32 bLinked;
    glGetProgramiv(programObject, GL_LINK_STATUS, &bLinked);
	if (!bLinked)
	{
		int infoLogLength, numCharsWritten;
		glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::vector<char> infoLog; infoLog.resize(infoLogLength);
		glGetProgramInfoLog(programObject, infoLogLength, &numCharsWritten, infoLog.data());
		
		std::string message("Failed to link program: ");
		message += infoLog.data();
		this->setExitMessage(message.c_str());
		return pvr::Result::InvalidData;
	}

	// Actually use the created program
	glUseProgram(programObject);

	// Sets the clear color
	glClearColor(0.00, 0.70, 0.67, 1.0f);

	// Create VBO for the triangle from our data

	// Vertex data
	GLfloat afVertices[] = {-0.4f,-0.4f,0.0f,  0.4f,-0.4f,0.0f,   0.0f,0.4f,0.0f};

	// Gen VBO
	glGenBuffers(1, &vbo);

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// Set the buffer's data
	glBufferData(GL_ARRAY_BUFFER, 3 * (3 * sizeof(GLfloat)) /* 3 Vertices of 3 floats in size */, afVertices, GL_STATIC_DRAW);

	// Unbind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Enable culling
	glEnable(GL_CULL_FACE);
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in releaseView() will be called by pvr::Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result OGLESIntroducingPVRShell::releaseView()
{
	// Release Vertex buffer object.
	glDeleteBuffers(1, &vbo);

	// Frees the OpenGL handles for the program and the 2 shaders
	glDeleteProgram(programObject);
	glDeleteShader(vertexShader);
	glDeleteShader(fragShader);
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result OGLESIntroducingPVRShell::renderFrame()
{
	// Matrix used for projection model view
	float afIdentity[] = 
	{
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};

	//	Clears the color buffer. glClear() can also be used to clear the depth or stencil buffer
	//	(GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT)
	glClear(GL_COLOR_BUFFER_BIT);

	
	//	Bind the projection model view matrix (PMVMatrix) to
	//	the associated uniform variable in the shader
	// First gets the location of that variable in the shader using its name
	int i32Location = glGetUniformLocation(programObject, "myPMVMatrix");

	// Then passes the matrix to that variable
	glUniformMatrix4fv(i32Location, 1, GL_FALSE, afIdentity);

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// Enable the custom vertex attribute at index VERTEX_ARRAY.
	// We previously binded that index to the variable in our shader "vec4 MyVertex;"
	glEnableVertexAttribArray(VertexArray);

	// Points to the data for this vertex attribute
	glVertexAttribPointer(VertexArray, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Draws a non-indexed triangle array from the pointers previously given.
	// This function allows the use of other primitive types : triangle strips, lines, ...
	// For indexed geometry, use the function glDrawElements() with an index list.
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// Unbind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return auto ptr to the demo supplied by the user
\brief	This function must be implemented by the user of the shell.
        The user should return its PVRShell object defining the behaviour of the application.
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo(){	return std::auto_ptr<pvr::Shell>(new OGLESIntroducingPVRShell()); }
