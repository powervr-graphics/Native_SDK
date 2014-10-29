/******************************************************************************

 @File         OGLESTexturing.cpp

 @Title        Texturing

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows how to use textures in OpenGL ES 1.x

******************************************************************************/
#if defined(__APPLE__) && defined (TARGET_OS_IPHONE)
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#else
#include <GLES/gl.h>
#endif

#include "PVRShell.h"

/******************************************************************************
 Defines
******************************************************************************/
// Size of the texture we create
const int g_i32TexSize = 128;

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLESTexturing : public PVRShell
{
	// Texture handle
	GLuint	m_ui32Texture;

	// Vertex Buffer Object (VBO) handle
	GLuint	m_ui32Vbo;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();
};


/*!****************************************************************************
 @Function		InitApplication
 @Return		bool		true if no error occurred
 @Description	Code in InitApplication() will be called by PVRShell once per
				run, before the rendering context is created.
				Used to initialize variables that are not dependent on it
				(e.g. external modules, loading meshes, etc.)
				If the rendering context is lost, InitApplication() will
				not be called again.
******************************************************************************/
bool OGLESTexturing::InitApplication()
{
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
bool OGLESTexturing::QuitApplication()
{
    return true;
}

/*!****************************************************************************
 @Function		InitView
 @Return		bool		true if no error occurred
 @Description	Code in InitView() will be called by PVRShell upon
				initialization or after a change in the rendering context.
				Used to initialize variables that are dependent on the rendering
				context (e.g. textures, vertex buffers, etc.)
******************************************************************************/
bool OGLESTexturing::InitView()
{
	// Sets the clear colour
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	// Enables texturing
	glEnable(GL_TEXTURE_2D);

	/*
		Creates the texture
	*/

	// Allocates one texture handle
	glGenTextures(1, &m_ui32Texture);

	// Binds this texture handle so we can load the data into it
	glBindTexture(GL_TEXTURE_2D, m_ui32Texture);

	// Creates the data as a 32bits integer array (8bits per component)
	GLuint* pTexData = new GLuint[g_i32TexSize*g_i32TexSize];

	for(int i = 0; i < g_i32TexSize; ++i)
	{
		for(int j = 0; j < g_i32TexSize; ++j)
		{
			// Fills the data with a fancy pattern
			GLuint col = (255<<24) + ((255-j*2)<<16) + ((255-i)<<8) + (255-i*2);

			if( ((i*j)/8) % 2 )
				col = 0xffff00ff;

			pTexData[j*g_i32TexSize+i] = col;
		}
	}

	/*
		glTexImage2D loads the texture data into the texture object.
		void glTexImage2D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
						   GLint border, GLenum format, GLenum type, const GLvoid *pixels );
		target must be GL_TEXTURE_2D.
		level specify the mipmap level we want to upload.
		internalformat and format must be the same. Here we use GL_RGBA for 4 component colours (r,g,b,a).
		  We could use GL_RGB, GL_ALPHA, GL_LUMINANCE, GL_LUMINANCE_ALPHA to use different colour component combinations.
		width, height specify the size of the texture. Both of the dimensions must be power of 2.
		border must be 0.
		type specify the format of the data. We use GL_UNSIGNED_BYTE to describe a colour component as an unsigned byte.
		  So a pixel is described by a 32bits integer.
		  We could also use GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_4_4_4_4, and GL_UNSIGNED_SHORT_5_5_5_1
		  to specify the size of all 3 (or 4) colour components. If we used any of these 3 constants,
		  a pixel would then be described by a 16bits integer.
	*/

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_i32TexSize, g_i32TexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, pTexData);

	/*
		glTexParameter is used to set the texture parameters
		void glTexParameter(GLenum target, GLenum pname, GLfloat param);
		target must be GL_TEXTURE_2D.
		pname is the parameter name we want to modify.
		  If pname is GL_TEXTURE_MIN_FILTER, param is used to set the way the texture is rendered when made smaller.
		  We can tell OpenGL to interpolate between the pixels in a mipmap level but also between different mipmap levels.
		  We are not using mipmap interpolation here because we didn't defined the mipmap levels of our texture.

		  If pname is GL_TEXTURE_MAG_FILTER, param is used to set the way the texture is rendered when made bigger.
		  Here we can only tell OpenGL to interpolate between the pixels of the first mipmap level.

		  if pname is GL_TEXTURE_WRAP_S or GL_TEXTURE_WRAP_T, then param sets the way a texture tiles in both directions.
		  The default if GL_REPEAT to wrap the texture (repeat it). We could also set it to GL_CLAMP or GL_CLAMP_TO_EDGE
		  to clamp the texture.

		  On OpenGL ES 1.1 and 2.0, if pname is GL_GENERATE_MIPMAP, param tells OpenGL to create mipmap levels automatically.
	*/

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	// Deletes the texture data, it's now in OpenGL memory
	delete[] pTexData;

	// Create VBO for the triangle from our data

	// Interleaved vertex data
	float afVertices[] = {-0.4f,-0.4f,0.0f, // Position
						 0.0f,0.0f,			 // UV
						 0.4f,-0.4f,0.0f,
						 1.0f,0.0f,
						 0.0f,0.4f,0.0f,
						 0.5f,1.0f};

	glGenBuffers(1, &m_ui32Vbo);

	unsigned int uiSize = 3 * (sizeof(float) * 5); // 3 vertices * stride (5 floats per vertex)

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);

	// Set the buffer's data
	glBufferData(GL_ARRAY_BUFFER, uiSize, afVertices, GL_STATIC_DRAW);

	// Unbind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Enable culling
	glEnable(GL_CULL_FACE);
	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occurred
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLESTexturing::ReleaseView()
{
	// Frees the texture
	glDeleteTextures(1, &m_ui32Texture);

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
bool OGLESTexturing::RenderScene()
{
	// Clears the color buffer
	glClear(GL_COLOR_BUFFER_BIT);

	/*
		Draw a triangle.
		Please refer to HelloTriangle or IntroducingPVRShell for a detailed explanation.
	*/

	// bind the VBO for the triangle
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT,sizeof(float) * 5, 0);

	// Pass the texture coordinates data

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2,GL_FLOAT,sizeof(float) * 5, (unsigned char*) (sizeof(float) * 3));

	// Draws a non-indexed triangle array
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// unbind the vertex buffer as we don't need it bound anymore
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
	return new OGLESTexturing();
}

/******************************************************************************
 End of file (OGLESTexturing.cpp)
******************************************************************************/

