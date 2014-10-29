/******************************************************************************

 @File         OGLES3TransformFeedback.cpp

 @Title        Transform Feedback

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows how to use transform feedback to TODO.

******************************************************************************/

#include "PVRShell.h"
#include "OGLES3Tools.h"

/******************************************************************************
 Defines
******************************************************************************/

#define NUM_PARTICLES 1000

// Index to bind the attributes to vertex shaders
#define POSITION_ARRAY		0
#define VELOCITY_ARRAY		1
#define ATTRIBUTES_ARRAY	2

/******************************************************************************
 Content file names
******************************************************************************/

// Source and binary shaders
const char c_szFragShaderSrcFile[] = "FragShader.fsh";
const char c_szVertShaderSrcFile[] = "VertShader.vsh";
const char c_szFeedbackFragShaderSrcFile[] = "FeedbackFragShader.fsh";
const char c_szFeedbackVertShaderSrcFile[] = "FeedbackVertShader.vsh";

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES3TransformFeedback : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D  m_Print3D;

	// Projection matrix and view rotation angle
	PVRTMat4      m_mProjection;
	float         m_fViewAngle;
	
	// Number of particles
	unsigned int  m_uiNumParticles;

	// OpenGL handles for VBOs and the transform feedback object
	GLuint        m_uiTransformFeedbackObject;
	GLuint        m_auiTransformFeedbackBuffer[2];	
	GLuint        m_uiFeedbackQuery;
	
	// Shader ids and attribute locations
	struct FeedbackShader
	{
		GLuint uiId;
		GLuint uiVertShader;
		GLuint uiFragShader;				
		GLint  iEmitDirectionLoc;
		GLint  iForceLoc;
		GLint  iTimeDeltaLoc;
	}
	m_FeedbackShader;

	// Shader ids and attribute locations
	struct Shader
	{
		GLuint uiId;
		GLuint uiVertShader;
		GLuint uiFragShader;	
		GLint  iViewProjMatrixLoc;
	}
	m_Shader;

	// Particle position and attributes required for simulation
	struct Particle
	{
		PVRTVec3 position;
		PVRTVec3 velocity;
		PVRTVec3 attributes; // time to live (ttl), initial velocity, initial ttl 
	};

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	bool LoadShaders(CPVRTString* pErrorStr);	
	void LoadTransformFeedbackBuffers();
};


/*!****************************************************************************
 @Function		LoadShaders
 @Output		pErrorStr		A CPVRTString describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads and compiles the shaders and links the shader programs
				required for this training course
******************************************************************************/
bool OGLES3TransformFeedback::LoadShaders(CPVRTString* pErrorStr)
{
	//
	//  Load the simple shader
	//
	if (PVRTShaderLoadFromFile(NULL, c_szVertShaderSrcFile, GL_VERTEX_SHADER, 0, &m_Shader.uiVertShader, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	if (PVRTShaderLoadFromFile(NULL, c_szFragShaderSrcFile, GL_FRAGMENT_SHADER, 0, &m_Shader.uiFragShader, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	const char* aszAttribs[] = { "inPosition", "inVelocity", "inAttributes" };
	if (PVRTCreateProgram(&m_Shader.uiId, m_Shader.uiVertShader, m_Shader.uiFragShader, aszAttribs, 3, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	// Retrieve uniform locations
	m_Shader.iViewProjMatrixLoc = glGetUniformLocation(m_Shader.uiId, "ViewProjMatrix");


	//
	// Transform feedback
	// Compile both shaders and then determine the feedback attributes to capture.
	//
	if (PVRTShaderLoadFromFile(NULL, c_szFeedbackVertShaderSrcFile, GL_VERTEX_SHADER, 0, &m_FeedbackShader.uiVertShader, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	if (PVRTShaderLoadFromFile(NULL, c_szFeedbackFragShaderSrcFile, GL_FRAGMENT_SHADER, 0, &m_FeedbackShader.uiFragShader, pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	// Create the program and register transform feedback attributes.
	m_FeedbackShader.uiId = glCreateProgram();

	glAttachShader(m_FeedbackShader.uiId, m_FeedbackShader.uiFragShader);
    glAttachShader(m_FeedbackShader.uiId, m_FeedbackShader.uiVertShader);

	const char* aszFeedbackAttribs[] = { "inPosition", "inVelocity", "inAttributes" };
	glBindAttribLocation(m_FeedbackShader.uiId, POSITION_ARRAY, aszFeedbackAttribs[0]);
	glBindAttribLocation(m_FeedbackShader.uiId, VELOCITY_ARRAY, aszFeedbackAttribs[1]);
	glBindAttribLocation(m_FeedbackShader.uiId, ATTRIBUTES_ARRAY, aszFeedbackAttribs[2]);
		
	const char* aszFeedbackCaptureAttribs[] = { "oPosition", "oVelocity", "oAttributes" };	
	glTransformFeedbackVaryings(m_FeedbackShader.uiId, 3, aszFeedbackCaptureAttribs, GL_INTERLEAVED_ATTRIBS);
	
	// Link the program object and print out the info log
	glLinkProgram(m_FeedbackShader.uiId);

    GLint Linked;
	glGetProgramiv(m_FeedbackShader.uiId, GL_LINK_STATUS, &Linked);

	if (!Linked)
	{
		int i32InfoLogLength;
		glGetProgramiv(m_FeedbackShader.uiId, GL_INFO_LOG_LENGTH, &i32InfoLogLength);
		char* pszInfoLog = new char[i32InfoLogLength];
		glGetProgramInfoLog(m_FeedbackShader.uiId, i32InfoLogLength, NULL, pszInfoLog);
		*pErrorStr = CPVRTString("Failed to link: ") + pszInfoLog + "\n";
		delete [] pszInfoLog;
		return false;
	}
	
	glUseProgram(m_FeedbackShader.uiId);
	
	// Retrieve uniform locations
	m_FeedbackShader.iEmitDirectionLoc = glGetUniformLocation(m_FeedbackShader.uiId, "EmitDirection");
	m_FeedbackShader.iForceLoc = glGetUniformLocation(m_FeedbackShader.uiId, "Force");
	m_FeedbackShader.iTimeDeltaLoc = glGetUniformLocation(m_FeedbackShader.uiId, "TimeDelta");
			
	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLES3TransformFeedback::LoadTransformFeedbackBuffers()
{
	// Create initial particle seed for the physics simulation
	Particle *pParticles = new Particle[m_uiNumParticles];
	for (unsigned int i=0; i < m_uiNumParticles; i++)
	{
		float angle = (rand() / (float)RAND_MAX) * PVRT_PI * 2.0f;		
		pParticles[i].position = PVRTVec3(0.0f);
		pParticles[i].velocity = PVRTVec3((float) sin(angle), angle * 0.5f, (float) cos(angle));
		pParticles[i].attributes.x = 1.0f + (rand() / (float)RAND_MAX);        // ttl: [1.0, 2.0]
		pParticles[i].attributes.y = 1.0f + (rand() / (float)RAND_MAX) * 4.0f; // initial velocity: [1.0, 5.0]
		pParticles[i].attributes.z = 1.0f + (rand() / (float)RAND_MAX);        // initial ttl: [1.0, 2.0]
	}

	glGenBuffers(2, m_auiTransformFeedbackBuffer);		
	glGenTransformFeedbacks(1, &m_uiTransformFeedbackObject);
	glGenQueries(1, &m_uiFeedbackQuery);

	glBindBuffer(GL_ARRAY_BUFFER, m_auiTransformFeedbackBuffer[0]);		
	glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * m_uiNumParticles, pParticles, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, m_auiTransformFeedbackBuffer[1]);		
	glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * m_uiNumParticles, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);	

	// clean up
	delete [] pParticles;
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
bool OGLES3TransformFeedback::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));
	
	m_uiNumParticles = NUM_PARTICLES;
	m_fViewAngle = 0.0f;
	
	return true;
}

/*!****************************************************************************
 @Function		QuitApplication
 @Return		bool		true if no error occured
 @Description	Code in QuitApplication() will be called by PVRShell once per
				run, just before exiting the program.
				If the rendering context is lost, QuitApplication() will
				not be called.x
******************************************************************************/
bool OGLES3TransformFeedback::QuitApplication()
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
bool OGLES3TransformFeedback::InitView()
{
	CPVRTString ErrorStr;
	
	//  Create the transform feedback buffers and objects
	//
	LoadTransformFeedbackBuffers();


    //Load and compile the shaders & link programs
	//
	if (!LoadShaders(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Is the screen rotated?
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);
	
	//	Initialize Print3D
	//
	if(m_Print3D.SetTextures(NULL,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}
					
	//	Calculate the projection and view matrices
	m_mProjection = PVRTMat4::PerspectiveFovRH(PVRT_PI/6, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), 1.0f, 100.0f, PVRTMat4::OGL, bRotate);
	
	//	Set OpenGL render states needed for this training course
	//
	// Enable backface culling and depth test
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES3TransformFeedback::ReleaseView()
{
	// Delete program and shader objects
	glDeleteProgram(m_Shader.uiId);
	glDeleteShader(m_Shader.uiVertShader);
	glDeleteShader(m_Shader.uiFragShader);

	glDeleteProgram(m_FeedbackShader.uiId);
	glDeleteShader(m_FeedbackShader.uiVertShader);
	glDeleteShader(m_FeedbackShader.uiFragShader);
	
	glDeleteBuffers(2, m_auiTransformFeedbackBuffer);

	glDeleteTransformFeedbacks(1, &m_uiTransformFeedbackObject);
	glDeleteQueries(1, &m_uiFeedbackQuery);
		
	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

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
bool OGLES3TransformFeedback::RenderScene()
{	
	float angle = (rand() / (float)RAND_MAX) * PVRT_PI * 2.0f;
	PVRTVec3 emitdirection((float)sin(angle), angle * 0.25f, (float)cos(angle));
	PVRTVec3 force(0.0f, -9.81f, 0.0f);	

	// Calculate timing values for the physics simulation
	static unsigned long prevtime = PVRShellGetTime();
	unsigned long curtime = PVRShellGetTime();
	unsigned long delta = curtime - prevtime;
	prevtime = curtime;
	float timedelta = delta * 0.001f;
	float timedeltasq = timedelta;

	// slowly rotate the camera around the y-axis
	m_fViewAngle += timedelta * 0.5f;

	// Clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	/*
	   Transform Feedback
	   
	   Disable the rasterization as we are using the vertex shader for the physics simulation.
	   Then bind the buffer for the transform feedback; there are two buffers: 
	   one to read the current state from and another one the results are written to.
	   The roles of both are switched after each frame.

	   The bound transform feedback object captures the amount of primitives written to the buffer
	   which allows to 'replay' the primitives in the buffer by calling glDrawTransformFeedback with 
	   the corresponding transform feedback object.

     */		
	glEnable(GL_RASTERIZER_DISCARD);				

	glBindBuffer(GL_ARRAY_BUFFER, m_auiTransformFeedbackBuffer[0]);
	glEnableVertexAttribArray(POSITION_ARRAY);
	glEnableVertexAttribArray(VELOCITY_ARRAY);
	glEnableVertexAttribArray(ATTRIBUTES_ARRAY);
	glVertexAttribPointer(POSITION_ARRAY, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), 0);
	glVertexAttribPointer(VELOCITY_ARRAY, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const void *)sizeof(PVRTVec3));
	glVertexAttribPointer(ATTRIBUTES_ARRAY, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const void *)(sizeof(PVRTVec3)*2));

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_uiTransformFeedbackObject);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_auiTransformFeedbackBuffer[1]);

	glUseProgram(m_FeedbackShader.uiId);		
	glUniform3fv(m_FeedbackShader.iEmitDirectionLoc, 1, &emitdirection.x);
	glUniform3fv(m_FeedbackShader.iForceLoc, 1, &force.x);
	glUniform1f(m_FeedbackShader.iTimeDeltaLoc, timedeltasq);
	
	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, m_uiFeedbackQuery);
	glBeginTransformFeedback(GL_POINTS);	
	glDrawArrays(GL_POINTS, 0, m_uiNumParticles);
	glEndTransformFeedback();		
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

	glDisableVertexAttribArray(POSITION_ARRAY);
	glDisableVertexAttribArray(VELOCITY_ARRAY);
	glDisableVertexAttribArray(ATTRIBUTES_ARRAY);

	glDisable(GL_RASTERIZER_DISCARD);

	// Swap source and target transform feedback buffers for the next frame
	GLuint tmpbuffer = m_auiTransformFeedbackBuffer[1];
	m_auiTransformFeedbackBuffer[1] = m_auiTransformFeedbackBuffer[0];
	m_auiTransformFeedbackBuffer[0] = tmpbuffer;

	GLuint available = GL_FALSE;		
	do {
		glGetQueryObjectuiv(m_uiFeedbackQuery, GL_QUERY_RESULT_AVAILABLE, &available);
	} while (available == GL_FALSE);

	GLuint uiNumFeedbackPrimitives = 0;
	glGetQueryObjectuiv(m_uiFeedbackQuery, GL_QUERY_RESULT, &uiNumFeedbackPrimitives);

	//
	// Render
	//

	if (uiNumFeedbackPrimitives > 0)
	{
		// Calculate and set the view-projection matrix 
		PVRTMat4 mModelView = PVRTMat4::LookAtRH(PVRTVec3((float)sin(m_fViewAngle) * 10.0f, 0.0f, (float)cos(m_fViewAngle) * 10.0f), PVRTVec3(0.0f), PVRTVec3(0.0f, 1.0f, 0.0f));	
		PVRTMat4 mModelViewProj = m_mProjection * mModelView;
		glUseProgram(m_Shader.uiId);
		glUniformMatrix4fv(m_Shader.iViewProjMatrixLoc, 1, GL_FALSE, mModelViewProj.f);	

		// Bind the buffer containing the updated transformations
		glBindBuffer(GL_ARRAY_BUFFER, m_auiTransformFeedbackBuffer[0]);	
		glEnableVertexAttribArray(POSITION_ARRAY);
		glEnableVertexAttribArray(VELOCITY_ARRAY);
		glEnableVertexAttribArray(ATTRIBUTES_ARRAY);
		glVertexAttribPointer(POSITION_ARRAY, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), 0);
		glVertexAttribPointer(VELOCITY_ARRAY, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const void *)sizeof(PVRTVec3));
		glVertexAttribPointer(ATTRIBUTES_ARRAY, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const void *)(sizeof(PVRTVec3)*2));

		// Render 		
		glDrawArrays(GL_POINTS, 0, uiNumFeedbackPrimitives);

		glDisableVertexAttribArray(POSITION_ARRAY);
		glDisableVertexAttribArray(VELOCITY_ARRAY);
		glDisableVertexAttribArray(ATTRIBUTES_ARRAY);
	}

	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("Transform Feedback", "", ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();		

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
	return new OGLES3TransformFeedback();
}

/******************************************************************************
 End of file (OGLES3TransformFeedback.cpp)
******************************************************************************/

