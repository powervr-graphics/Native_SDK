/******************************************************************************

 @File         OGLES2BinaryShader.cpp

 @Title        OGLES2BinaryShader

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows how to use extension GL_OES_get_program_binary (if
               supported) to save a compiled shader out as a binary file and then
               load it back in on future runs. It displays a red triangle if
               forced to compile shaders and displays a green triangle if using binary shaders.

******************************************************************************/
#include <stdio.h>
#include <string.h>

#if defined(__APPLE__) && defined (TARGET_OS_IPHONE)
	#import <OpenGLES/ES2/gl.h>
	#import <OpenGLES/ES2/glext.h>
#else
	#include <GLES2/gl2.h>
	#include <GLES2/gl2ext.h>
#endif

#if defined(EGL_NOT_PRESENT)
	#define PVRGetProcAddress(x) NULL
#else

	#if defined(__APPLE__) && defined (TARGET_OS_IPHONE)
		#define PVRGetProcAddress(x) ::x
	#else
		#include <EGL/egl.h>
		#define PVRGetProcAddress(x) eglGetProcAddress(#x)
	#endif

#endif

#include "PVRShell.h"

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
class OGLES2BinaryShader : public PVRShell
{
	// The vertex and fragment shader OpenGL handles
	GLuint m_uiVertexShader, m_uiFragShader;

	// The program object containing the 2 shader objects
	GLuint m_uiProgramObject;

	// VBO handle
	GLuint m_ui32Vbo;

	// Binary shader support check;
	bool m_bBinaryShaderSupported;

	//GLExtension checking.
	bool IsGLExtensionSupported(const char* extension);

	//Binary Handling functions to load and save a program.
	bool saveBinaryProgram(const char* Filename, GLuint &ProgramObjectID);
	bool loadBinaryProgram(const char* Filename, GLuint &ProgramObjectID);

public:

#if !defined (TARGET_OS_IPHONE)
	// Declares the binary program functions
	PFNGLGETPROGRAMBINARYOESPROC				glGetProgramBinaryOES;
	PFNGLPROGRAMBINARYOESPROC					glProgramBinaryOES;
#endif

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
bool OGLES2BinaryShader::InitApplication()
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
bool OGLES2BinaryShader::QuitApplication()
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
bool OGLES2BinaryShader::InitView()
{
	// Initialise a colour to draw our triangle
	// (For this training course, binary loaded shaders use a different colour
	// To show which is being used. Red means it had to compile the shaders,
	// green shows that it retrieved the binary from memory.
	float afColour[]={0.0,0.0,0.0};

	// Filename and path strings.
	char* pWritePath = (char*)PVRShellGet(prefWritePath);
	char* shaderPath = new char[strlen(pWritePath) + 13];
	sprintf(shaderPath, "%sShaderBinary", pWritePath);

	//Checks if the program binary handling extension is supported.
	m_bBinaryShaderSupported=IsGLExtensionSupported("GL_OES_get_program_binary");

#if !defined (TARGET_OS_IPHONE)
	glGetProgramBinaryOES=0;
	glProgramBinaryOES=0;

	// Retrieves the functions needed to use the extension.
	if (m_bBinaryShaderSupported)
	{
		glGetProgramBinaryOES = (PFNGLGETPROGRAMBINARYOESPROC) PVRGetProcAddress(glGetProgramBinaryOES);
		glProgramBinaryOES = (PFNGLPROGRAMBINARYOESPROC) PVRGetProcAddress(glProgramBinaryOES);
	}
#endif

	// If binary shaders are not supported or there isn't a valid binary shader stored, recompile the shaders.
	if (!m_bBinaryShaderSupported || !loadBinaryProgram(shaderPath,m_uiProgramObject))
	{
		{
			// Fragment shader code
			const char* pszFragShader = "\
				uniform lowp vec3 myColour;\
				void main (void)\
				{\
					gl_FragColor = vec4(myColour, 1.0);\
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
				delete [] shaderPath;
				return false;
			}
		}

		{
			// Vertex shader code.
			const char* pszVertShader = "\
			attribute highp vec4	myVertex;\
			uniform mediump mat4	myPMVMatrix;\
			void main(void)\
			{\
				gl_Position = myPMVMatrix * myVertex;\
			}";
			// Loads the vertex shader in the same way
			m_uiVertexShader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(m_uiVertexShader, 1, (const char**)&pszVertShader, NULL);
			glCompileShader(m_uiVertexShader);

			// Checks if the shader has compiled.
			GLint bShaderCompiled;
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
				delete [] shaderPath;
				return false;
			}
		}

		// Create the shader program
		m_uiProgramObject = glCreateProgram();

		// Attach the fragment and vertex shaders to the shader program.
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
			delete [] shaderPath;
			return false;
		}

		// As there is no stored binary, save the current binary out for use later.
		// Note that this is done after both binding attributes and linking -
		// none of these can be performed after
		if (m_bBinaryShaderSupported) saveBinaryProgram(shaderPath,m_uiProgramObject);

		//Set red channel of the colour to maximum - red shows that the shaders had to be compiled.
		afColour[0]=1.0f;
	}
	else
	{
		//Set green channel of the colour to maximum - green shows that the shaders were loaded from binary files.
		afColour[1]=1.0f;
	}

	delete[] shaderPath;

	// Uses the program.
	glUseProgram(m_uiProgramObject);

	// Bind the colour to the fragment shader.
	GLint i32ColourLocation = glGetUniformLocation(m_uiProgramObject, "myColour");

	// Then passes the colour to that variable
	glUniform3fv(i32ColourLocation, 1, afColour);

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
bool OGLES2BinaryShader::ReleaseView()
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
 @Return		bool		true if no error occurred
 @Description	Main rendering loop function of the program. The shell will
				call this function every frame.
				eglSwapBuffers() will be performed by PVRShell automatically.
				PVRShell will also manage important OS events.
				Will also manage relevent OS events. The user has access to
				these events through an abstraction layer provided by PVRShell.
******************************************************************************/
bool OGLES2BinaryShader::RenderScene()
{
	// Matrix used for projection model view
	float afIdentity[] = {
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};

	// Clears the color buffer.
	glClear(GL_COLOR_BUFFER_BIT);

	//	Bind the projection model view matrix (PMVMatrix) to
	//	the associated uniform variable in the shader
	GLint i32PMVLocation = glGetUniformLocation(m_uiProgramObject, "myPMVMatrix");

	// Then pass the matrix to the shader
	glUniformMatrix4fv(i32PMVLocation, 1, GL_FALSE, afIdentity);

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);

	// Enable the custom vertex attribute at index VERTEX_ARRAY.
	// We previously binded that index to the variable in our shader "vec4 MyVertex;"
	glEnableVertexAttribArray(VERTEX_ARRAY);

	// Points to the data for this vertex attribute
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Draws a non-indexed triangle array from the pointers previously given.
	// This function allows the use of other primitive types : triangle strips, lines, ...
	// For indexed geometry, use the function glDrawElements() with an index list.
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// Unbind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

/*!***********************************************************************
@Function			IsGLExtensionSupported
@Input				extension extension to query for
@Returns			True if the extension is supported
@Description		Queries for support of an extension
*************************************************************************/
bool OGLES2BinaryShader::IsGLExtensionSupported(const char *extension)
{
	// The recommended technique for querying OpenGL extensions;
	// from http://opengl.org/resources/features/OGLextensions/
	const GLubyte *extensions = NULL;
	const GLubyte *start;
	GLubyte *where, *terminator;

	// Extension names should not have spaces.
	where = (GLubyte *) strchr(extension, ' ');
	if (where || *extension == '\0')
		return 0;

	extensions = glGetString(GL_EXTENSIONS);

	// It takes a bit of care to be fool-proof about parsing the
	// OpenGL extensions string. Don't be fooled by sub-strings, etc.
	start = extensions;
	for (;;) {
		where = (GLubyte *) strstr((const char *) start, extension);
		if (!where)
			break;
		terminator = where + strlen(extension);
		if (where == start || *(where - 1) == ' ')
			if (*terminator == ' ' || *terminator == '\0')
				return true;
		start = terminator;
	}

	return false;
}

/*!****************************************************************************
 @Function		saveBinaryProgram
 @Return		bool	True if save succeeded.
 @Description	This function takes as input the ID of a shader program object
				which should have been created prior to calling this function,
				as well as a filename to save the binary program to.
				The function will save out a file storing the binary shader
				program, and the enum value determining its format.
******************************************************************************/
bool OGLES2BinaryShader::saveBinaryProgram(const char* Filename, GLuint &ProgramObjectID)
{
#if !defined (TARGET_OS_IPHONE)
	//Quick check to make sure that the program actually exists.
	GLint linked;
	glGetProgramiv(ProgramObjectID, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Shaders not linked correctly, no binary to retrieve.
		return false;
	}

	// Get the length of the shader binary program in memory.
	// Doing this ensures that a sufficient amount of memory is allocated for storing the binary program you retrieve.
	GLsizei length=0;
	glGetProgramiv(ProgramObjectID,GL_PROGRAM_BINARY_LENGTH_OES,&length);

	// Pointer to the binary shader program in memory, needs to be allocated with the right size.
	GLvoid* ShaderBinary = (GLvoid*)malloc(length);

	// The format that the binary is retrieved in.
	GLenum binaryFormat=0;

	// Error checking variable - this should be greater than 0 after glGetProgramBinaryOES, otherwise there was an error.
	GLsizei lengthWritten=0;

	// Get the program binary from GL and save it out.
	glGetProgramBinaryOES(ProgramObjectID,length,&lengthWritten,&binaryFormat,ShaderBinary);
	if (!lengthWritten)
	{
		// Save failed. Insufficient memory allocated to write binary shader.
		return false;
	}

	// Cache the program binary for future runs
	FILE* outfile = fopen(Filename, "wb");

	if(!outfile)
	{
		PVRShellOutputDebug("Failed to open %s for writing to.\n", Filename);
		return false;
	}

	// Save the binary format.
	if(!fwrite((void*)&binaryFormat,sizeof(GLenum),1,outfile)) 
	{
		fclose(outfile);
		return false; // Couldn't write binary format to file.
	}

	// Save the actual binary program.
	if(!fwrite(ShaderBinary, length,1,outfile))
	{
		fclose(outfile);
		return false;				 // Couldn't write binary data to file.
	}

	// Close the file.
	fclose(outfile);

	// Free the memory used by Shader Binary.
	free(ShaderBinary);
	return true;
#else
	return false;
#endif
}
/*!****************************************************************************
 @Function		loadBinaryProgram
 @Return		bool	True if load succeeded.
 @Description	This function takes as input the ID of a shader program object
				which should have been created prior to calling this function,
				as well as a filename to load the binary program from.
				The function will load in a file storing the binary shader
				program, and the enum value determining its format.
				It will then load the binary into memory.

 @Note:			This function is not able to check if the shaders have changed.
				If you change the shaders then the file this saves out either
				needs to be deleted	or a new file used.
******************************************************************************/
bool OGLES2BinaryShader::loadBinaryProgram(const char* Filename, GLuint &ProgramObjectID)
{
#if !defined (TARGET_OS_IPHONE)
    // Open the file.
    FILE* infile = fopen(Filename, "rb");

	// File open failed, either doesn't exist or is empty.
	if (!infile)
		return false;

	// Find initialise the shader binary.
    fseek(infile, 0, SEEK_END);
    GLsizei length = (GLint)ftell(infile)-sizeof(GLenum);

	if (!length)
	{
		fclose(infile);
		return false;	// File appears empty.
	}

	// Allocate a buffer large enough to store the binary program.
    GLvoid* ShaderBinary = (GLvoid*)malloc(length);

	// Read in the binary format
	GLenum format=0;
    fseek(infile, 0, SEEK_SET);
    fread(&format, sizeof(GLenum), 1, infile);

	// Read in the program binary.
    fread(ShaderBinary, length, 1, infile);
    fclose(infile);

	// Create an empty shader program
	ProgramObjectID = glCreateProgram();

    // Load the binary into the program object -- no need to link!
    glProgramBinaryOES(ProgramObjectID, format, ShaderBinary, length);

	// Delete the binary program from memory.
    free(ShaderBinary);

	// Check that the program was loaded correctly, uses the same checks as when linking with a standard shader.
	GLint loaded;
    glGetProgramiv(ProgramObjectID, GL_LINK_STATUS, &loaded);
    if (!loaded)
    {
        // Something must have changed. Need to recompile shaders.
		int i32InfoLogLength, i32CharsWritten;
		glGetProgramiv(ProgramObjectID, GL_INFO_LOG_LENGTH, &i32InfoLogLength);
		char* pszInfoLog = new char[i32InfoLogLength];
		glGetProgramInfoLog(ProgramObjectID, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
		char* pszMsg = new char[i32InfoLogLength+256];
		strcpy(pszMsg, "Failed to load binary program: ");
		strcat(pszMsg, pszInfoLog);
		PVRShellSet(prefExitMessage, pszMsg);

		delete [] pszMsg;
		delete [] pszInfoLog;
		return false;
    }
	return true;
#else
	return false;
#endif
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
	return new OGLES2BinaryShader();
}

/******************************************************************************
 End of file (OGLES2BinaryShader.cpp)
******************************************************************************/

