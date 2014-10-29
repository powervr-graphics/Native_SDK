/******************************************************************************

 @File         OGLES2MultiThreading_iOS.cpp

 @Title        OpenGL ES 2.0 multi-threaded loading Tutorial

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     iOS

 @Description  A tutorial which demonstrates loading resources on a seperate
               thread. The loading thread is artifially prolonged in order
               to demonstrate the technique.
******************************************************************************/
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <OpenGLES/EAGL.h>


#include "PVRShell.h"
#include "OGLES2Tools.h"

/******************************************************************************
 Constants
******************************************************************************/
enum ELoadingProgress
{
    eProgress_Init,
    eProgress_Shaders,
    eProgress_Program,
    eProgress_Textures,
    eProgress_VertexData,
    
    eProgress_SIZE,
};

static const char* c_pszLoadingProgress[eProgress_SIZE] =
{
    "Intialising...",           // eProgress_Init
    "Compiling Shaders...",     // eProgress_Shaders
    "Creating Program...",      // eProgress_Program
    "Generating Textures...",   // eProgress_Textures
    "Uploading Vertex Data...", // eProgress_VertexData
};

static const int c_iNumCircles = 8;
static const int c_iNumCirclePoints = 32;
static const PVRTVECTOR3 c_vCircleCols[c_iNumCircles] =
{
    { 1.0f, 1.0f, 0.66f },
    { 1.0f, 0.66f, 1.0f },
    { 0.66f, 1.0f, 1.0f },
    { 0.66f, 0.66f, 1.0f },
    { 1.0f, 0.66f, 0.66f },
    { 0.66f, 1.0f, 0.66f },
    { 1.0f, 0.86f, 0.66f },
    { 0.66f, 0.86f, 1.0f },
};

// Fragment and vertex shaders code
const char* c_pszFragShader = "\
    uniform lowp vec3       myCol;\
    void main (void)\
    {\
        gl_FragColor = vec4(myCol ,1.0);\
    }";

const char* c_pszVertShader = "\
    attribute highp vec4	myVertex;\
    uniform mediump mat4	myPMVMatrix;\
    void main(void)\
    {\
        gl_Position = myPMVMatrix * myVertex;\
    }";
    
const char* c_pszCubeFragShader = "\
    varying lowp vec3 col;\
    varying lowp float NdotL;\
    varying lowp vec2 texCoord;\
    uniform sampler2D sTexture;\
    void main (void)\
    {\
        lowp vec3 vCol = mix(texture2D(sTexture, texCoord).rgb, abs(col), 0.3);\
        gl_FragColor = vec4(vCol * NdotL, 1.0);\
    }";

const char* c_pszCubeVertShader = "\
    attribute highp vec4	myVertex;\
    attribute mediump vec3  myNormal;\
    attribute mediump vec2  myUV;\
    uniform mediump mat4	myPMVMatrix;\
    uniform mediump vec3    vLightDir;\
    varying lowp vec3 col;\
    varying lowp vec2 texCoord;\
    varying lowp float NdotL;\
    void main(void)\
    {\
        gl_Position      = myPMVMatrix * myVertex;\
        \
        mediump vec3 n = normalize(myNormal);\
        NdotL          = max(dot(n, normalize(vLightDir)), 0.0);\
        col            = myNormal;\
        texCoord       = myUV;\
    }"; 

/******************************************************************************
 Defines
******************************************************************************/
// Index to bind the attributes to vertex shaders
#define VERTEX_ARRAY	0
#define NORMAL_ARRAY    1
#define UV_ARRAY        2

#define LOAD_DELAY      1000 * 1000

#define TEX_SIZE        128

/******************************************************************************
 Global variables
******************************************************************************/
volatile static ELoadingProgress      g_eProgress = eProgress_Init;

 /*!****************************************************************************
 @Struct 	SAPIHandles
******************************************************************************/
struct SAPIHandles
{
	GLuint 			uiFragShader;
	GLuint 			uiVertShader;		    
	GLuint 			uiProgramObject;
	GLuint 			uiVbo;
	GLuint 			uiIndexVbo;
	GLuint 			uiTexture;

	// Handles which will be needed to draw the loading screen.
	GLuint 			uiLoadFragShader;
	GLuint 			uiLoadVertShader;
	GLuint 			uiLoadProgram;
	GLuint 			uiLoadVbo;

	// EAGL handles
	EAGLContext*	  eaglMainContext;
	EAGLContext*	  eaglSecContext;
	
	// Syncing handles
	pthread_mutex_t	  mutex;
	GLsync			  sync;
};

 /*!****************************************************************************
 @Class 	OGLES2IntroducingPVRShell
******************************************************************************/
class OGLES2MultiThreading : public PVRShell
{
private:
  	// Handles which will be loaded in a seperate resource loading thread.
	SAPIHandles		handles;
	CPVRTPrint3D 	print3D;
    CPVRTPrint3D 	loadingText;
    int             iFrame;
    bool            bLoading;
    
public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

public:
	static bool LoadShaders(GLuint& uiVertShader, GLuint& uiFragShader, const char* pszVertShader, const char* pszFragShader);
	static bool CreateProgram(GLuint& uiProgramObject, const GLuint uiVertShader, const GLuint uiFragShader);
	static bool CreateLoadingGeometry(GLuint& uiLoadVbo, const int iNumPoints);
	static bool CreateSceneGeometry(GLuint& uiVertexVbo, GLuint& uiIndexVbo);
    
private:
    void RenderCubeScene(int iFrame);
    void RenderLoadingScene(int iFrame);
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
bool OGLES2MultiThreading::InitApplication()
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
bool OGLES2MultiThreading::QuitApplication()
{
	return true;
}

/*!****************************************************************************
 @Function		LoadShaders
 @Output		uiVertex
 @Output		uiFragment
 @Return        true if successful.
 @Description	Loads a basic vertex and fragment shader and returns the 
                handles.
******************************************************************************/
bool OGLES2MultiThreading::LoadShaders(GLuint& uiVertShader, GLuint& uiFragShader, const char* pszVertShader, const char* pszFragShader)
{
    // Create the fragment shader object
	uiFragShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Load the source code into it
	glShaderSource(uiFragShader, 1, (const char**)&pszFragShader, NULL);

	// Compile the source code
	glCompileShader(uiFragShader);

	// Check if compilation succeeded
	GLint bShaderCompiled;
    glGetShaderiv(uiFragShader, GL_COMPILE_STATUS, &bShaderCompiled);

	if (!bShaderCompiled)
	{
		// An error happened, first retrieve the length of the log message
		int i32InfoLogLength, i32CharsWritten;
		glGetShaderiv(uiFragShader, GL_INFO_LOG_LENGTH, &i32InfoLogLength);

		// Allocate enough space for the message and retrieve it
		char* pszInfoLog = new char[i32InfoLogLength];
        glGetShaderInfoLog(uiFragShader, i32InfoLogLength, &i32CharsWritten, pszInfoLog);

		// Displays the error
		NSLog(@"Failed to compile fragment shader: %s\n", pszInfoLog);
		delete [] pszInfoLog;
		return false;
	}

	// Loads the vertex shader in the same way
	uiVertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(uiVertShader, 1, (const char**)&pszVertShader, NULL);
	glCompileShader(uiVertShader);
    glGetShaderiv(uiVertShader, GL_COMPILE_STATUS, &bShaderCompiled);

	if (!bShaderCompiled)
	{
		int i32InfoLogLength, i32CharsWritten;
		glGetShaderiv(uiVertShader, GL_INFO_LOG_LENGTH, &i32InfoLogLength);
		char* pszInfoLog = new char[i32InfoLogLength];
        glGetShaderInfoLog(uiVertShader, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
		NSLog(@"Failed to compile vertex shader: %s\n", pszInfoLog);
		delete [] pszInfoLog;
		return false;
	} 
    
    return true;
}

/*!****************************************************************************
 @Function		createProgram
 @Output		uiProgramObject
 @Input         uiVertShader
 @Input         uiFragShader
 @Return        true if successful.
 @Description	Creates a new program from given vertex and fragment shader
                handles.
******************************************************************************/
bool OGLES2MultiThreading::CreateProgram(GLuint& uiProgramObject, const GLuint uiVertShader, const GLuint uiFragShader)
{
    // Create the shader program
    uiProgramObject = glCreateProgram();

	// Attach the fragment and vertex shaders to it
    glAttachShader(uiProgramObject, uiFragShader);
    glAttachShader(uiProgramObject, uiVertShader);

	// Bind the custom vertex attribute "myVertex" to location VERTEX_ARRAY
    glBindAttribLocation(uiProgramObject, VERTEX_ARRAY, "myVertex");
    glBindAttribLocation(uiProgramObject, NORMAL_ARRAY, "myNormal");
    glBindAttribLocation(uiProgramObject, UV_ARRAY,     "myUV");

	// Link the program
    glLinkProgram(uiProgramObject);

	// Check if linking succeeded in the same way we checked for compilation success
    GLint bLinked;
    glGetProgramiv(uiProgramObject, GL_LINK_STATUS, &bLinked);

	if (!bLinked)
	{
		int ui32InfoLogLength, ui32CharsWritten;
		glGetProgramiv(uiProgramObject, GL_INFO_LOG_LENGTH, &ui32InfoLogLength);
		char* pszInfoLog = new char[ui32InfoLogLength];
		glGetProgramInfoLog(uiProgramObject, ui32InfoLogLength, &ui32CharsWritten, pszInfoLog);
		NSLog(@"Failed to link program: %s\n", pszInfoLog);
		delete [] pszInfoLog;
		return false;
	}
    
    return true;
}

/*!****************************************************************************
 @Function		createLoadingGeometry
 @Output		uiLoadVbo
 @Input         iNumPoints
 @Description	Generates a circle to use as geometry for loading.
******************************************************************************/
bool OGLES2MultiThreading::CreateLoadingGeometry(GLuint& uiLoadVbo, const int iNumPoints)
{
    const int iNumCirclePoints = iNumPoints;
    const int iNumCircleVerts  = (iNumCirclePoints + 1 + 1) * 3;
    const float PVRT_2PI       = PVRT_PI*2.0f;
    const float fCircleRad     = 20.0f;
    GLfloat fCircleVerts[iNumCircleVerts];
    
    // Centre
    fCircleVerts[0] = 0.0f;
    fCircleVerts[1] = 0.0f;
    fCircleVerts[2] = 0.0f;
    int idx = 3;
    
    for(float fRad = 0.0f; fRad < PVRT_2PI; fRad += (PVRT_2PI/iNumCirclePoints))
    {
        fCircleVerts[idx++] = cos(fRad) * fCircleRad;
        fCircleVerts[idx++] = sin(fRad) * fCircleRad;
        fCircleVerts[idx++] = 0.0f;
    }
    
    fCircleVerts[idx++] = fCircleVerts[3];
    fCircleVerts[idx++] = fCircleVerts[4];
    fCircleVerts[idx++] = fCircleVerts[5];
    
    // Generate the vertex buffer object (VBO)
    glGenBuffers(1, &uiLoadVbo);

    // Bind the VBO so we can fill it with data
    glBindBuffer(GL_ARRAY_BUFFER, uiLoadVbo);

    // Set the buffer's data
    unsigned int uiSize = iNumCircleVerts * sizeof(GLfloat); // Calc afVertices size (3 vertices * stride (3 GLfloats per vertex))
    glBufferData(GL_ARRAY_BUFFER, uiSize, fCircleVerts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    return true;
}

/*!****************************************************************************
 @Function		createSceneGeometry
 @Output		uiVertexVbo
 @Output		uiIndexVbo
 @Description	Generates an indexed cube.
******************************************************************************/
bool OGLES2MultiThreading::CreateSceneGeometry(GLuint& uiVertexVbo, GLuint& uiIndexVbo)
{
    // Interleaved vertex data
                                // Position             // Normal           // UV
    GLfloat afVertices[] = {    1.0f, 1.0f, -1.0f,      0.0f, 1.0f,  0.0f,  0.0f, 0.0f, // 0     // Top face
                                1.0f, 1.0f,  1.0f,      0.0f, 1.0f,  0.0f,  0.0f, 1.0f, // 1    
                               -1.0f, 1.0f,  1.0f,      0.0f, 1.0f,  0.0f,  1.0f, 1.0f, // 2    
                               -1.0f, 1.0f, -1.0f,      0.0f, 1.0f,  0.0f,  1.0f, 0.0f, // 3    
                               
                                1.0f,-1.0f, -1.0f,      0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // 4     // Bottom face
                                1.0f,-1.0f,  1.0f,      0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // 5
                               -1.0f,-1.0f,  1.0f,      0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // 6
                               -1.0f,-1.0f, -1.0f,      0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // 7
                               
                               -1.0f, 1.0f, -1.0f,     -1.0f, 0.0f,  0.0f,  0.0f, 0.0f, // 8     // Left face
                               -1.0f,-1.0f, -1.0f,     -1.0f, 0.0f,  0.0f,  0.0f, 1.0f, // 9
                               -1.0f,-1.0f,  1.0f,     -1.0f, 0.0f,  0.0f,  1.0f, 1.0f, // 10
                               -1.0f, 1.0f,  1.0f,     -1.0f, 0.0f,  0.0f,  1.0f, 0.0f, // 11
                               
                                1.0f, 1.0f, -1.0f,      1.0f, 0.0f,  0.0f,  0.0f, 0.0f, // 12    // Right face
                                1.0f,-1.0f, -1.0f,      1.0f, 0.0f,  0.0f,  0.0f, 1.0f, // 13
                                1.0f,-1.0f,  1.0f,      1.0f, 0.0f,  0.0f,  1.0f, 1.0f, // 14
                                1.0f, 1.0f,  1.0f,      1.0f, 0.0f,  0.0f,  1.0f, 0.0f, // 15
                                
                                -1.0f, 1.0f, -1.0f,     0.0f, 0.0f, -1.0f,  0.0f, 0.0f, // 16    // Back face
                                -1.0f,-1.0f, -1.0f,     0.0f, 0.0f, -1.0f,  0.0f, 1.0f, // 17 
                                 1.0f,-1.0f, -1.0f,     0.0f, 0.0f, -1.0f,  1.0f, 1.0f, // 18
                                 1.0f, 1.0f, -1.0f,     0.0f, 0.0f, -1.0f,  1.0f, 0.0f, // 19
                                
                                -1.0f, 1.0f,  1.0f,     0.0f, 0.0f,  1.0f,  0.0f, 0.0f, // 20    // Front face
                                -1.0f,-1.0f,  1.0f,     0.0f, 0.0f,  1.0f,  0.0f, 1.0f, // 21 
                                 1.0f,-1.0f,  1.0f,     0.0f, 0.0f,  1.0f,  1.0f, 1.0f, // 22
                                 1.0f, 1.0f,  1.0f,     0.0f, 0.0f,  1.0f,  1.0f, 0.0f, // 23
                               };
                               
    GLushort auIndices[] = {
                                0, 1, 2,        // Top face
                                2, 3, 0,
                                
                                8, 9, 10,       // Left face
                                10, 11, 8,
                                
                                12, 13, 14,     // Right face
                                14, 15, 12,
                                
                                20, 21, 22,     // Front face
                                22, 23, 20,
                                
                                16, 18, 17,     // Back face
                                16, 19, 18,
                                
                                6, 5, 4,        // Bottom face
                                6, 4, 7,
                           };

    // Generate the vertex buffer object (VBO)
    glGenBuffers(1, &uiVertexVbo);
    glGenBuffers(1, &uiIndexVbo);

    // Bind the VBO so we can fill it with data
    glBindBuffer(GL_ARRAY_BUFFER, uiVertexVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uiIndexVbo);

    // Set the buffer's data
    unsigned int uiSize = 24 * (sizeof(GLfloat) * 8);                        // Calc afVertices size (24 vertices * stride (6 GLfloats per vertex))
    glBufferData(GL_ARRAY_BUFFER, uiSize, afVertices, GL_STATIC_DRAW);
    
    // Set the index buffer's data
    uiSize = 36 * sizeof(GLushort);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, auIndices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    return true;
}

/*!****************************************************************************
 @Function		LoadResources
 @Output		handles
 @Description	Loads resources associated with this example.
******************************************************************************/
bool LoadResources(SAPIHandles& handles)
{
    // Load shaders
    pthread_mutex_lock(&handles.mutex);
        g_eProgress = eProgress_Shaders;
    pthread_mutex_unlock(&handles.mutex);
    
    if(!OGLES2MultiThreading::LoadShaders(handles.uiVertShader, handles.uiFragShader, c_pszCubeVertShader, c_pszCubeFragShader))
    {
        return false;
    }
    
    // Create program
    usleep(LOAD_DELAY);
    pthread_mutex_lock(&handles.mutex);
        g_eProgress = eProgress_Program;
    pthread_mutex_unlock(&handles.mutex);
    
    if(!OGLES2MultiThreading::CreateProgram(handles.uiProgramObject, handles.uiVertShader, handles.uiFragShader))
    {
        return false;
    }
    
    // Load vertex data
    usleep(LOAD_DELAY);
    pthread_mutex_lock(&handles.mutex);
        g_eProgress = eProgress_VertexData;
    pthread_mutex_unlock(&handles.mutex);
    
    if(!OGLES2MultiThreading::CreateSceneGeometry(handles.uiVbo, handles.uiIndexVbo))
    {
        return false;
    }
    
    // Generate procedural texture
    usleep(LOAD_DELAY);
    pthread_mutex_lock(&handles.mutex);
        g_eProgress = eProgress_Textures;
    pthread_mutex_unlock(&handles.mutex);
    
    /*
		Create the texture
	*/

	glGenTextures(1, &handles.uiTexture);
	glBindTexture(GL_TEXTURE_2D, handles.uiTexture);

	// Creates the data as a 32bits integer array (8bits per component)
	GLuint* pTexData = new GLuint[TEX_SIZE*TEX_SIZE];
	for (int i=0; i<TEX_SIZE; i++)
	for (int j=0; j<TEX_SIZE; j++)
	{
		// Fills the data with a fancy pattern
		GLuint col = (255<<24) + ((255-j*2)<<16) + ((255-i)<<8) + (255-i*2);
		if ( ((i*j)/8) % 2 ) col = (GLuint) (255<<24) + (255<<16) + (0<<8) + (255);
		pTexData[j*TEX_SIZE+i] = col;
	}

    // Upload and set parameters
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEX_SIZE, TEX_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, pTexData);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	// Deletes the texture data, it's now in OpenGL memory
	delete [] pTexData;
    
    usleep(LOAD_DELAY);
    
    return true;
}

/*!****************************************************************************
 @Function		ThreadFunc
 @Input			arg		pointer to arg data
 @Description	Worker thread entry point.
******************************************************************************/
void* ThreadFunc(void * arg)
{
    SAPIHandles* pHandles = (SAPIHandles*)arg;
    
	// Create the secondary eagl context.
	EAGLSharegroup* eaglShareGroup = [pHandles->eaglMainContext sharegroup];
    pHandles->eaglSecContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2 sharegroup:eaglShareGroup];
	
	if(pHandles->eaglSecContext == nil)
    {
        NSLog(@"Failed to create a secondary context.\n");
        return (void*)1;
    }
	
	if(![EAGLContext setCurrentContext:pHandles->eaglSecContext])
	{
		NSLog(@"Failed to make the secondary context current.\n");
		return (void*)1;
	}
	
    // Load the resources.
    bool r = LoadResources(*pHandles);
    
    /*
        Now we will insert fence sync to make sure that all prior commands are executed before
        trying to render. This will guarentee that textures, shaders and VBO data is available
        to use when we come to render our first frame.
    */
    
    // Insert a fence sync in to the command stream.
    pthread_mutex_lock(&pHandles->mutex);
        pHandles->sync = glFenceSyncAPPLE(GL_SYNC_GPU_COMMANDS_COMPLETE_APPLE, 0);
	pthread_mutex_unlock(&pHandles->mutex);
    
	if(!glIsSyncAPPLE(pHandles->sync))
    {
        NSLog(@"glFenceSyncAPPLE failed to create a valid fence sync object.\n");
    }
    
    // GL_SYNC_FLUSH_COMMANDS_BIT_APPLE causes the GL context to flush.
    GLenum status = glClientWaitSyncAPPLE(pHandles->sync, GL_SYNC_FLUSH_COMMANDS_BIT_APPLE, 0);
    if(status == GL_WAIT_FAILED_APPLE)
    {
        NSLog(@"glClientWaitSyncAPPLE failed.\n");
    }
    
    return (void*)(r ? 0 : 1);
}

/*!****************************************************************************
 @Function		InitView
 @Return		bool		true if no error occured
 @Description	Code in InitView() will be called by PVRShell upon
				initialization or after a change in the rendering context.
				Used to initialize variables that are dependant on the rendering
				context (e.g. textures, vertex buffers, etc.)
******************************************************************************/
bool OGLES2MultiThreading::InitView()
{
	// Check that APPLE_sync extension is available
	if(!CPVRTgles2Ext::IsGLExtensionSupported("GL_APPLE_sync"))
	{
		NSLog(@"Error: GL_APPLE_sync extension is required to run this example.");
		return false;
	}

    // EGL and GL variables
    memset(&handles, 0, sizeof(handles));

    // Retrieve EGL handles
    handles.eaglMainContext = [EAGLContext currentContext];

    // Create a mutex
    pthread_mutex_init(&handles.mutex, NULL);

	/*
		At this point everything is initialized and we're ready to use
		OpenGL ES to draw something on the screen.
	*/

    bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);
    int  iW      = PVRShellGet(prefWidth);
    int  iH      = PVRShellGet(prefHeight);
    
    // Intialise Print3D  
    if(print3D.SetTextures(NULL, iW, iH, bRotate) != PVR_SUCCESS)
    {
        PVRShellSet(prefExitMessage, "ERROR: Failed to initialise Print3D.\n");
        return false;
    }
    
    if(loadingText.SetTextures(NULL, iW, iH, bRotate) != PVR_SUCCESS)
    {
        PVRShellSet(prefExitMessage, "ERROR: Failed to initialise Print3D.\n");
        return false;
    }
    
    // Load some shaders which will enable us to draw a splash screen.
    if(!LoadShaders(handles.uiLoadVertShader, handles.uiLoadFragShader, c_pszVertShader, c_pszFragShader))
    {
        PVRShellSet(prefExitMessage, "ERROR: Failed to load shaders.\n");
        return false;
    }
    
    if(!CreateProgram(handles.uiLoadProgram, handles.uiLoadVertShader, handles.uiLoadFragShader))
    {
        PVRShellSet(prefExitMessage, "ERROR: Failed to create a program.\n");
        return false;
    }
    
    if(!CreateLoadingGeometry(handles.uiLoadVbo, c_iNumCirclePoints))
    {
        PVRShellSet(prefExitMessage, "ERROR: Failed to create geometry.\n");
        return false;
    }
    
    // The colours are passed per channel (red,green,blue,alpha) as float values from 0.0 to 1.0
    glClearColor(0.6f, 0.8f, 1.0f, 1.0f); // clear blue

    iFrame   = 0;
    bLoading = true;
    
    /*
        Spawn a thread which will create it's own context and load resouces while we render
        a loading screen.
    */
    pthread_t   hThreadHandle;
    int r = pthread_create(&hThreadHandle, NULL, &ThreadFunc, (void *)&handles);
    
    if(r)
    {
        PVRShellSet(prefExitMessage, "ERROR: Failed to spawn a worker thread.\n");
        return false;
    }
    
	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES2MultiThreading::ReleaseView()
{
	// Frees the OpenGL handles for the program and the 2 shaders
	if(handles.uiProgramObject) glDeleteProgram(handles.uiProgramObject);
	if(handles.uiFragShader)    glDeleteShader(handles.uiFragShader);
	if(handles.uiVertShader)    glDeleteShader(handles.uiVertShader);
    
	if(handles.uiLoadProgram)    glDeleteProgram(handles.uiLoadProgram);
	if(handles.uiLoadFragShader) glDeleteShader(handles.uiLoadFragShader);
	if(handles.uiLoadVertShader) glDeleteShader(handles.uiLoadVertShader);

	// Delete the VBO as it is no longer needed
    if(handles.uiLoadVbo) glDeleteBuffers(1, &handles.uiLoadVbo);
	if(handles.uiVbo)     glDeleteBuffers(1, &handles.uiVbo);
    
    if(handles.uiTexture) glDeleteTextures(1, &handles.uiTexture);

    // Clean up Print3D
    print3D.ReleaseTextures();
    loadingText.ReleaseTextures();
    
    // Destroy the pthread mutex.
    pthread_mutex_destroy(&handles.mutex);

	return true;
}

/*!****************************************************************************
 @Function		RenderLoadingScene
 @Input			iFrame
 @Description	Renders an animated loading screen.
******************************************************************************/
void OGLES2MultiThreading::RenderLoadingScene(int iFrame)
{
    bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);
    float fHW      = PVRShellGet(prefWidth) / 2.0f;
    float fHH      = PVRShellGet(prefHeight) / 2.0f;

    PVRTMat4 mxProjection = PVRTMat4::Ortho(-fHW, fHH, fHW, -fHH, -1.0f, 1.0f, PVRTMat4::OGL, bRotate);

    /*
        Clears the color buffer.
    */
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Actually use the created program
    glUseProgram(handles.uiLoadProgram);
    
    // First gets the location of that variable in the shader using its name
    int i32MVPLocation = glGetUniformLocation(handles.uiLoadProgram, "myPMVMatrix");
    int i32ColLocation = glGetUniformLocation(handles.uiLoadProgram, "myCol");

    for(int iCircleIdx = 0; iCircleIdx < c_iNumCircles; ++iCircleIdx)
    {
        int iProg    = iFrame+iCircleIdx*4;
        float fScale = (0.75f + cos(iProg * 0.1f) * 0.25f);
        float fY     = sin(iProg * 0.1f) * 25.0f;
        
        // Then passes the matrix to that variable
        PVRTMat4 mxMVP = mxProjection * PVRTMat4::Translation(-175.0f + iCircleIdx * 50.0f, fY, 0.0f) * PVRTMat4::Scale(fScale,fScale,1.0f);
        glUniformMatrix4fv(i32MVPLocation, 1, GL_FALSE, mxMVP.ptr());
        
        // Pass the colour
        glUniform3f(i32ColLocation, c_vCircleCols[iCircleIdx].x, c_vCircleCols[iCircleIdx].y, c_vCircleCols[iCircleIdx].z);

        // Draw the loading circle
        glBindBuffer(GL_ARRAY_BUFFER, handles.uiLoadVbo);
        glEnableVertexAttribArray(VERTEX_ARRAY);
        glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, 0, 0);
        
        // Submit
        glDrawArrays(GL_TRIANGLE_FAN, 0, c_iNumCirclePoints+2);
        
        glDisableVertexAttribArray(VERTEX_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);   
    }
            
    float fW;
    loadingText.SetProjection(mxProjection);
    
    ELoadingProgress eProgress = eProgress_Init;
    pthread_mutex_lock(&handles.mutex);
        eProgress = g_eProgress;
    pthread_mutex_unlock(&handles.mutex);
    
    loadingText.MeasureText(&fW, NULL, 1.0f, c_pszLoadingProgress[eProgress]);
    loadingText.Print3D(-fW*0.5f, -50.0f, 1.0f, 0xFFFFFFFF, c_pszLoadingProgress[eProgress]);
    loadingText.Flush();
}

/*!****************************************************************************
 @Function		RenderCubeScene
 @Input			iFrame
 @Description	Renders the pre-loaded scene.
******************************************************************************/
void OGLES2MultiThreading::RenderCubeScene(int iFrame)
{
    bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);
    int  iW      = PVRShellGet(prefWidth);
    int  iH      = PVRShellGet(prefHeight);

    PVRTMat4 mxProjection = PVRTMat4::PerspectiveFovRH(0.7f, (float)iW / (float)iH, 1.0f, 1000.0f, PVRTMat4::OGL, bRotate);
    PVRTMat4 mxView       = PVRTMat4::Translation(0.0f, 0.0f, -200.0f);
    PVRTMat4 mxModel      = PVRTMat4::RotationX(-0.5f) * PVRTMat4::RotationY(iFrame * 0.016f) * PVRTMat4::Scale(30.0f, 30.0f, 30.0f);
    
    PVRTMat4 mxMVP        = mxProjection * mxView * mxModel;
    
    PVRTVec4 vLightDir    = PVRTVec4(0.0f, 0.3f, 1.0f, 0.0f) * mxModel;

    // Actually use the created program
    glUseProgram(handles.uiProgramObject);

    // Sets the clear color.
    // The colours are passed per channel (red,green,blue,alpha) as float values from 0.0 to 1.0
    glClearColor(0.6f, 0.8f, 1.0f, 1.0f); // clear blue
    
    glBindTexture(GL_TEXTURE_2D, handles.uiTexture);
    
    // Bind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, handles.uiVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handles.uiIndexVbo);

    /*
        Clears the color buffer and depth buffer
    */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /*
        Bind the projection model view matrix (PMVMatrix) to
        the associated uniform variable in the shader
    */

    // First gets the location of that variable in the shader using its name
    int i32MVPLocation  = glGetUniformLocation(handles.uiProgramObject, "myPMVMatrix");
    int i32LDLocation   = glGetUniformLocation(handles.uiProgramObject, "vLightDir");
    int i32TexLocation  = glGetUniformLocation(handles.uiProgramObject, "sTexture");

    // Then passes the matrix to that variable
    glUniformMatrix4fv( i32MVPLocation, 1, GL_FALSE, mxMVP.ptr());
    
    // Pass light direction
    glUniform3fv( i32LDLocation, 1, vLightDir.ptr());
    
    // Set texture location
    glUniform1i( i32TexLocation, 0);

    /*
        Enable the custom vertex attribute at index VERTEX_ARRAY.
        We previously binded that index to the variable in our shader "vec4 MyVertex;"
     */
    glEnableVertexAttribArray(VERTEX_ARRAY);
    glEnableVertexAttribArray(NORMAL_ARRAY);
    glEnableVertexAttribArray(UV_ARRAY);

    // Sets the vertex data to this attribute index
    glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
    glVertexAttribPointer(NORMAL_ARRAY, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));
    glVertexAttribPointer(UV_ARRAY,     2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6*sizeof(GLfloat)));

    /*
        Draws a non-indexed triangle array from the pointers previously given.
        This function allows the use of other primitive types : triangle strips, lines, ...
        For indexed geometry, use the function glDrawElements() with an index list.
    */
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
    
    // Disable states
    glDisableVertexAttribArray(VERTEX_ARRAY);
    glDisableVertexAttribArray(NORMAL_ARRAY);
    glDisableVertexAttribArray(UV_ARRAY);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    glBindTexture(GL_TEXTURE_2D, 0);
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
bool OGLES2MultiThreading::RenderScene()
{
   // Render the loading screen while we wait for resources to load.
    if(bLoading)
    {  
        // Render the animated loading scene.
		RenderLoadingScene(iFrame++);
                
        /*
            Check if the resources are still being loaded.
            This is performed by querying GL to determine if a sync object has been signalled.
        */
        pthread_mutex_lock(&handles.mutex);
        
		/*
		 The following condition will only pass once the sync object has been created in the worker thread.
		 */
		if(glIsSyncAPPLE(handles.sync))
		{
			/*
			 Perform a non-blocking poll to check if the shared gl sync object has been signalled.
			 Non-blocking is achieved by waiting for 0ms.
			 */
			GLenum status = glClientWaitSyncAPPLE(handles.sync, 0, 0);
			if(status == GL_CONDITION_SATISFIED_APPLE || status == GL_ALREADY_SIGNALED_APPLE)
			{
				// Destroy the gl sync object as soon as we aware of it's signal status.
				glDeleteSyncAPPLE(handles.sync);
				bLoading = false;
			}
		}
        
        pthread_mutex_unlock(&handles.mutex);
    }
    else
    {
    	glEnable(GL_DEPTH_TEST);

    	// Render the pre-loaded scene
        RenderCubeScene(iFrame++);

        glDisable(GL_DEPTH_TEST);    
    }
    
    print3D.DisplayDefaultTitle("MultiThreading", "", ePVRTPrint3DSDKLogo);
    print3D.Flush();

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
	return new OGLES2MultiThreading();
}

/******************************************************************************
 End of file (OGLES2MultiThreading_iOS.cpp)
******************************************************************************/
