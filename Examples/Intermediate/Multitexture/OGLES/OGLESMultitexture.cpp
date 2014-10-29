/******************************************************************************

 @File         OGLESMultitexture.cpp

 @Title        Demonstrates multitexturing in OGLES

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows multitexturing in OGLES using combiners.

******************************************************************************/

/*************************
**		  Includes	    **
*************************/
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "PVRShell.h"
#include "OGLESTools.h"

/******************************************************************************
 Content file names
******************************************************************************/

// PVR texture files
const char c_szCrateTexFile[]	= "Crate.pvr";
const char c_szStampTexFile[]	= "stamp.pvr";
const char c_szStampnmTexFile[]	= "stampnm.pvr";

/******************************************************************************
******************************************************************************/

// determines the speed with which the boxes rotate
const float g_fRotationSpeed  = 0.05f;

class OGLESMultitexture : public PVRShell
{
	// Print3D class
	CPVRTPrint3D 	m_Print3D;

	// Texture names
	GLuint m_ui32Crate;
	GLuint m_ui32Stamp;
	GLuint m_ui32Stampnm;

	// Angle and time variables.
	float m_fAngle;
	unsigned long m_ulPreviousTime;

	// Vertex Buffer Object (VBO) handles
	GLuint	m_ui32Vbo;
	GLuint	m_ui32IndexVbo;

public:

	OGLESMultitexture()  :	m_ui32Crate(0),
							m_ui32Stamp(0),
							m_fAngle(0)
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
	bool DrawCube(float param);
	bool DrawDot3Cube();
	void LoadVbos();
	void EnableStates();
	void DisableStates();
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
bool OGLESMultitexture::InitApplication()
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
bool OGLESMultitexture::QuitApplication()
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
bool OGLESMultitexture::InitView()
{
	SPVRTContext	sContext;

	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	//	Initialize the Print3D textures
	if(m_Print3D.SetTextures(&sContext, PVRShellGet(prefWidth), PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D.");
		return false;
	}

	/*
		Load the three textures that we'll be multitexturing with.

		The first is a texture of a wooden crate.
	*/
	if(PVRTTextureLoadFromPVR(c_szCrateTexFile, &m_ui32Crate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load crate texture.");
		return false;
	}
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//	The second is a texture of a red stamp.
	if(PVRTTextureLoadFromPVR(c_szStampTexFile, &m_ui32Stamp) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load stamp texture.");
		return false;
	}
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//	The third is normal map version of the second.
	if(PVRTTextureLoadFromPVR(c_szStampnmTexFile, &m_ui32Stampnm) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load stamp normal map texture..");
		return false;
	}
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//	Setup and set the projection and the modelview matrix
	PVRTMat4 mProjection;
	PVRTMat4 mView;
	PVRTVec3 CameraFrom, CameraTo, CameraUp;

	mProjection = PVRTMat4::PerspectiveFovRH(30.0f*(3.14f/180.0f), (float) PVRShellGet(prefWidth) / (float) PVRShellGet(prefHeight), 10.0f, 10000.0f, PVRTMat4::OGL, bRotate);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(mProjection.f);

	/*
		Create modelview matrix

		Init the Camera details
	*/
	CameraFrom = PVRTVec3(0.0f, 0.0f, -100.0f);
	CameraTo   = PVRTVec3(0.0f, 0.0f, 0.0f);
	CameraUp   = PVRTVec3(0.0f, 1.0f, 0.0f);

	//	Set the modelview matrix using the camera details
	mView = PVRTMat4::LookAtRH(CameraFrom, CameraTo, CameraUp);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(mView.f);

	// Initialise the clear colour
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	// Enable the depth test
	glEnable(GL_DEPTH_TEST);

	// Initialise the cube
	LoadVbos();

	// Initialise the time variable for the animation
	m_ulPreviousTime = PVRShellGetTime();

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
bool OGLESMultitexture::ReleaseView()
{
	// Release textures
	m_Print3D.ReleaseTextures();

	glDeleteTextures(1, &m_ui32Stamp);
	glDeleteTextures(1, &m_ui32Crate);
	glDeleteTextures(1, &m_ui32Stampnm);

	return true;
}

/*!****************************************************************************
 @Function		InitCube
 @Return		bool		true if no error occured
 @Description	Initialise m_sCube to contain the data about our cube
******************************************************************************/
void OGLESMultitexture::LoadVbos()
{
	//	Setup the cubes face values
	unsigned short ui16Faces[12 * 3] = {	0, 1, 2,
											2, 3, 0,
											4, 5, 6,
											6,  7, 4,
											8,  9, 10,
											10, 11, 8,
											12, 13, 14,
											14, 15, 12,
											16, 17, 18,
											18, 19, 16,
											20, 21, 22,
											22, 23, 20};

	/*
		Initialise the vertices and uv coordinates. We're going to use the same coordinates for both
		textures so we only require one set.
	*/
	float fVertices[24 * 5] = {	-5, -5, 5,
								1, 0,
								-5, -5, -5,
								1, 1,
								5, -5, -5,
								0, 1,
								5, -5, 5,
								0, 0,
								-5, 5, 5,
								0, 0,
								5, 5, 5,
								1, 0,
								5, 5, -5,
								1, 1,
								-5, 5, -5,
								0, 1,
								-5, -5, 5,
								0, 0,
								5, -5, 5,
								1, 0,
								5, 5, 5,
								1, 1,
								-5, 5, 5,
								0, 1,
								5, -5, 5,
								0, 0,
								5, -5, -5,
								1, 0,
								5, 5, -5,
								1, 1,
								5, 5, 5,
								0, 1,
								5, -5, -5,
								0, 0,
								-5, -5, -5,
								1, 0,
								-5, 5, -5,
								1, 1,
								5, 5, -5,
								0, 1,
								-5, -5, -5,
								0, 0,
								-5, -5, 5,
								1, 0,
								-5, 5, 5,
								1, 1,
								-5, 5, -5,
								0, 1};

	// Create vertex buffer objects from our data
	glGenBuffers(1, &m_ui32Vbo);
	glGenBuffers(1, &m_ui32IndexVbo);

	// Load vertex data into buffer object
	unsigned int uiSize = 24 * (sizeof(float) * 5);

	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);
	glBufferData(GL_ARRAY_BUFFER, uiSize, fVertices, GL_STATIC_DRAW);

	uiSize = 36 * sizeof(GLshort);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ui32IndexVbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, ui16Faces, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/*******************************************************************************
 * Function Name  : EnableStates
 * Description    : Set up the vertex and texture coordinate buffers and enable
					texturing for both textures.
 *******************************************************************************/
void OGLESMultitexture::EnableStates()
{
	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);

	// bind the index buffer, won't hurt if the handle is 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ui32IndexVbo);

	// Set vertex data
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(float) * 5, 0);

	// Set texture data

	//Set the texture data for the first texture.
	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(float) * 5, (GLvoid*) (sizeof(float) * 3));

	/*
		Set the texture for the second texture. In this case we are just reusing the first
		set of texture coordinates.
	*/
	glClientActiveTexture(GL_TEXTURE1);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(float) * 5, (GLvoid*) (sizeof(float) * 3));

	// Enable 2D texturing for the first texture.
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);

	// Enable 2D texturing for the second texture.
	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);

}

/*******************************************************************************
 * Function Name  : DisableStates
 * Description    : Disable the vertex and texture coordinate buffers and Disable
					texturing for both textures.
 *******************************************************************************/
void OGLESMultitexture::DisableStates()
{
	// Disable states
	// Disable the vertex buffer.
	glDisableClientState(GL_VERTEX_ARRAY);

	// Disable the buffer for the first set of texture coordinates
	glClientActiveTexture(GL_TEXTURE0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	// Disable the buffer for the second set of texture coordinates
	glClientActiveTexture(GL_TEXTURE1);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	// Disable 2D texturing for the first texture
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);

	// Disable 2D texturing for the second texture
	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);

	// unbind the vertex buffers as we don't need them bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
bool OGLESMultitexture::RenderScene()
{
	// Clear the background to our clear colour and clear the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Enable the vertex, texture coordinate, etc. buffers and enable texturing
	EnableStates();

	/*
		update the angle of rotation of the boxes with respect to the time
		that has passed since the last frame.
	*/
	unsigned long ulTime = PVRShellGetTime();
	float fDeltaTime = (float) ulTime - m_ulPreviousTime;

	m_ulPreviousTime = ulTime;
	m_fAngle += fDeltaTime * g_fRotationSpeed;

	while(m_fAngle > 360.0f)
		m_fAngle -= 360.0f;


	// Push the modelview matrix onto the stack so we dont make any permanent changes to it
	glPushMatrix();
		// Translate and rotate the cube
		glTranslatef( 20.0f,15.0f,0.0f );
		glRotatef( m_fAngle, 0.0, 1.0, 0.0 );

		/*
			Draw the cube and when applying the textures multiply the colours in the first
			by the colours in the second (Arg0 * Arg1).
		*/
		DrawCube(GL_MODULATE);
		// Write modulate to the screen underneath the cube we just drew
		m_Print3D.Print3D(9, 35, 0.7f, PVRTRGBA(255,255,255,255), "GL_MODULATE");
	glPopMatrix();

	glPushMatrix();
		glTranslatef( 0.0f,15.0f,0.0f );
		glRotatef( m_fAngle, 0.0f, 1.0f, 0.0f );

		/*
			Draw the cube and add the colours of the two textures together to get
			the final colour (Arg0 + Arg1).
		*/
		DrawCube(GL_ADD);
		m_Print3D.Print3D(43, 35, 0.7f, PVRTRGBA(255,255,255,255), "GL_ADD");
	glPopMatrix();

	glPushMatrix();
		glTranslatef( -20.0f,15.0f,0.0f );
		glRotatef( m_fAngle, 0.0f, 1.0f, 0.0f );

		/*
			Draw the cube and add the colours of the two textures together and minus 0.5 to get
			the final colour (Arg0 + Arg1 - 0.5).
		*/
		DrawCube(GL_ADD_SIGNED);
		m_Print3D.Print3D(65, 35, 0.7f, PVRTRGBA(255,255,255,255), "GL_ADD_SIGNED");
	glPopMatrix();

	glPushMatrix();
		glTranslatef( 20.0f,-5.0f,0.0f );
		glRotatef( m_fAngle, 0.0f, 1.0f, 0.0f );

		/*
			Draw the cube and just use the first texture for texturing (Arg0).
		*/
		DrawCube(GL_REPLACE);
		m_Print3D.Print3D(10, 72, 0.7f, PVRTRGBA(255,255,255,255), "GL_REPLACE");
	glPopMatrix();

	glPushMatrix();
		glTranslatef( 0.0f,-5.0f,0.0f );
		glRotatef( m_fAngle, 0.0f, 1.0f, 0.0f );

		/*
			Draw the cube and subtract the colours from the second texture from the first (Arg0 - Arg1).
		*/
		DrawCube(GL_SUBTRACT);
		m_Print3D.Print3D(38, 72, 0.7f, PVRTRGBA(255,255,255,255), "GL_SUBTRACT");
	glPopMatrix();

	glPushMatrix();
		glTranslatef( -20.0f,-5.0f,0.0f );
		glRotatef( m_fAngle, 0.0f, 1.0f, 0.0f );

		/*
		*/
		DrawDot3Cube(); //GL_DOT3_RGB
		m_Print3D.Print3D(67, 72, 0.7f, PVRTRGBA(255,255,255,255), "GL_DOT3_RGB");
	glPopMatrix();

	// Disable the vertex, texture coordinate, etc. buffers and Disable texturing
	DisableStates();

	// Display info text
	m_Print3D.DisplayDefaultTitle("Multitexture", "", ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();
	return true;
}


/*******************************************************************************
 * Function Name  : DrawCube
 * Inputs		  : param
 * Returns		  : true if no error occured
 * Description    : Draw the cube and when applying the textures combine using
					the value of param.
 *******************************************************************************/
bool OGLESMultitexture::DrawCube(float param)
{
	// Set up the First Texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_ui32Stamp);

	// Set up the Second Texture
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_ui32Crate);

	// Set the texture environment mode for this texture to combine
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

	// Set the method we're going to combine the two textures by.
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, param);

	// Use the previous combine texture as source 0
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);

	// Use the current texture as source 1
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);

	/*
		Set what we will operate on, in this case we are going to use
		just the texture colours.
	*/
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

	// Indexed Triangle list
	glDrawElements(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_SHORT, 0);
	return true;
}

/*******************************************************************************
 * Function Name  : DrawDot3Cube
 * Inputs		  : param
 * Returns		  : true if no error occured
 * Description    : Draw the cube with Dot3 lighting.
 *******************************************************************************/
bool OGLESMultitexture::DrawDot3Cube()
{
	/*
		The Dot3 example will be different to the other ones as it
		will use a normal map and a light vector.
	*/
	PVRTVec3 fLightVector;

	// Set up the light vector and rotate it round the cube.
	fLightVector.x = PVRTSIN(m_fAngle * (PVRT_PIf / 180.0f));
	fLightVector.y = 0.0f;
	fLightVector.z = PVRTCOS(m_fAngle * (PVRT_PIf / 180.0f));

	// Half shifting to have a value between 0.0f and 1.0f
	fLightVector.x = fLightVector.x * 0.5f + 0.5f;
	fLightVector.y = fLightVector.y * 0.5f + 0.5f;
	fLightVector.z = fLightVector.z * 0.5f + 0.5f;

	/* Set light direction as a colour
	 * (the colour ordering depend on how the normal map has been computed)
	 * red=x, green=y, blue=z */
	glColor4f(fLightVector.x, fLightVector.y, fLightVector.z, 1.0f);

	/*
		Set up the First Texture (the normal map) and combine it with the texture
		colour that we've set.
	*/
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_ui32Stampnm);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGB);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);

	// Set up the Second Texture and combine it with the result of the Dot3 combination
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_ui32Crate);

	// Set the texture environment mode for this texture to combine
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

	// Set the method we're going to combine the two textures by.
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);

	// Use the previous combine texture as source 0
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);

	// Use the current texture as source 1
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);

	/*
		Set what we will operate on, in this case we are going to use
		just the texture colours.
	*/
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

	// Draw mesh
	glDrawElements(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_SHORT, 0);

	// Reset
	glColor4f(1.0,1.0,1.0,1.0);
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
	return new OGLESMultitexture();
}

/*****************************************************************************
 End of file (OGLESMultitexture.cpp)
*****************************************************************************/

