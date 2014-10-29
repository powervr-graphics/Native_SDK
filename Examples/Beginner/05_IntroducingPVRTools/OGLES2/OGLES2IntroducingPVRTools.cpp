/******************************************************************************

 @File         OGLES2IntroducingPVRTools.cpp

 @Title        Introducing the PVRTools

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows how to use the tools to load textures, shaders and display
               text

******************************************************************************/
#include "PVRShell.h"
#include "OGLES2Tools.h"

/******************************************************************************
 shader attributes and uniforms
******************************************************************************/
// Vertex attributes
// We define an enum for the attribute position and an array of strings that
// correspond to the attribute names in the shader. These can be used by PVRTools
enum EVertexAttrib {
	VERTEX_ARRAY, TEXCOORD_ARRAY, eNumAttribs };
const char* g_aszAttribNames[] = {
	"inVertex", "inTexCoord" };

// Shader uniforms
enum EUniform {
	eMVPMatrix, eNumUniforms };
const char* g_aszUniformNames[] = {
	"MVPMatrix" };

/******************************************************************************
 Content file names
******************************************************************************/

// Source and binary shaders
const char c_szFragShaderSrcFile[]	= "FragShader.fsh";
const char c_szFragShaderBinFile[]	= "FragShader.fsc";
const char c_szVertShaderSrcFile[]	= "VertShader.vsh";
const char c_szVertShaderBinFile[]	= "VertShader.vsc";

// PVR texture files
const char c_szTextureFile[]		= "Image.pvr";

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES2IntroducingPVRTools : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// Texture handle
	GLuint	m_uiTexture;

	// VBO handle
	GLuint m_ui32Vbo;

	//
	unsigned int m_ui32VertexStride;

	// The vertex and fragment shader OpenGL handles
	GLuint m_uiVertexShader, m_uiFragShader;

	// Group shader programs and their uniform locations together
	struct
	{
		GLuint uiId;
		GLuint auiLoc[eNumUniforms];
	}
	m_ShaderProgram;

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
bool OGLES2IntroducingPVRTools::InitApplication()
{
	/*
		CPVRTResourceFile is a resource file helper class. Resource files can
		be placed on disk next to the executable or in a platform dependent
		read path. We need to tell the class where that read path is.
		Additionally, it is possible to wrap files into cpp modules and
		link them directly into the executable. In this case no path will be
		used. Files on disk will override "memory files".
	*/

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
bool OGLES2IntroducingPVRTools::QuitApplication()
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
bool OGLES2IntroducingPVRTools::InitView()
{
	/*
		Initialize the textures used by Print3D.
		To properly display text, Print3D needs to know the viewport dimensions
		and whether the text should be rotated. We get the dimensions using the
		shell function PVRShellGet(prefWidth/prefHeight). We can also get the
		rotate parameter by checking prefIsRotated and prefFullScreen.
	*/
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	if(m_Print3D.SetTextures(0, PVRShellGet(prefWidth), PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	// Sets the clear color
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	/*
		Loads the texture using the tool function PVRTTextureLoadFromPVR.
		The first parameter is the name of the file and the
		second parameter returns the resulting texture handle.
		The third parameter is a CPVRTString for error message output.
		This function can also be used to conveniently set the filter modes. If
		those parameters are not given, OpenGL ES defaults are used.
		Setting a mipmap filter on a mipmap-less texture will result in an error.
	*/

	if(PVRTTextureLoadFromPVR(c_szTextureFile, &m_uiTexture) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot load the texture\n");
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/*
		Compiles the shaders.
		First we use CPVRTResourceFile to load a file into memory. After construction with a
		file name, we just have to check whether the file is open or an error occured.
		We load both source and binary shaders, then try the binary shader first.
		The data of a CPVRTResourceFile will always be terminated with a 0 byte so it can
		safely be used as a C string.
	*/
	CPVRTResourceFile VertexShaderSrcFile(c_szVertShaderSrcFile);
	CPVRTResourceFile VertexShaderBinFile(c_szVertShaderBinFile);

	CPVRTString ErrorStr;
	/*
		PVRTShaderLoadBinaryFromMemory takes a pointer to the binary shader and the shader size as
		its first arguments. Then follows the shader type and binary format.
		On success, the handle to the new shader object is returned in the fifth parameter, while
		an error string is returned on failure.
	*/
	if (!VertexShaderBinFile.IsOpen() ||
		(PVRTShaderLoadBinaryFromMemory(VertexShaderBinFile.DataPtr(), VertexShaderBinFile.Size(),
			GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiVertexShader, &ErrorStr) != PVR_SUCCESS))
	{
		/*
			Fallback to source shader
			PVRTShaderLoadSourceFromMemory() takes the shader source code as its 1st argument.
			The shader type as 2nd argument (for now either GL_VERTEX_SHADER or GL_FRAGMENT_SHADER.
			It returns the shader object in its 3rd argument.
			If an error occurs during compilation, the resulting log is returned in the 4th parameter.
			We could also use PVRTLoadAndCompileShaderFromFile() to load and
			compile a shader from an external text file
		*/

		CPVRTString vertexShaderSrc((const char*) VertexShaderSrcFile.DataPtr(), VertexShaderSrcFile.Size());

		if (!VertexShaderSrcFile.IsOpen() ||
			(PVRTShaderLoadSourceFromMemory(vertexShaderSrc.c_str(), GL_VERTEX_SHADER, &m_uiVertexShader, &ErrorStr) != PVR_SUCCESS))
		{
			PVRShellSet(prefExitMessage, ErrorStr.c_str());
			return false;
		}
	}

	/*
		PVRTShaderLoadFromFile can be used to try compiling/loading shaders from files. In this variant,
		two files are tried before failing (usually binary and source files). The type of shader is determined
		from the file extension (.fsh and .vsh for source, .fsc and .vsc for SGX binary shaders)
	*/
	if (PVRTShaderLoadFromFile(c_szFragShaderBinFile, c_szFragShaderSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_uiFragShader, &ErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	/*
		PVRTCreateProgram creates a new program object, attaches the shaders, binds attributes (given as an array
		of strings and the size thereof), and makes the program current - or it returns an error string on failure.
	*/
	if (PVRTCreateProgram(&m_ShaderProgram.uiId, m_uiVertexShader, m_uiFragShader, g_aszAttribNames, eNumAttribs, &ErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Store the location of uniforms for later use
	for (int i = 0; i < eNumUniforms; ++i)
	{
		m_ShaderProgram.auiLoc[i] = glGetUniformLocation(m_ShaderProgram.uiId, g_aszUniformNames[i]);
	}

	// Create VBO for the triangle from our data

	// Interleaved vertex data
	GLfloat afVertices[] = {-0.4f,-0.4f,0.0f, // Pos
							 0.0f,0.0f ,				 // UVs
							 0.4f,-0.4f,0.0f,
							 1.0f,0.0f ,
							 0.0f,0.4f ,0.0f,
							 0.5f,1.0f};

	glGenBuffers(1, &m_ui32Vbo);

	m_ui32VertexStride = 5 * sizeof(GLfloat); // 3 floats for the pos, 2 for the UVs

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);

	// Set the buffer's data
	glBufferData(GL_ARRAY_BUFFER, 3 * m_ui32VertexStride, afVertices, GL_STATIC_DRAW);

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
bool OGLES2IntroducingPVRTools::ReleaseView()
{
	// Frees the texture
	glDeleteTextures(1,&m_uiTexture);

	// Release Vertex buffer object.
	glDeleteBuffers(1, &m_ui32Vbo);

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

	// Frees the OpenGL handles for the program and the 2 shaders
	glDeleteProgram(m_ShaderProgram.uiId);
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
bool OGLES2IntroducingPVRTools::RenderScene()
{
	// Clears the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Binds the loaded texture
	glBindTexture(GL_TEXTURE_2D, m_uiTexture);

	// Use the loaded shader program
	glUseProgram(m_ShaderProgram.uiId);

	/*
		Creates the Model View Projection (MVP) matrix using the PVRTMat4 class from the tools.
		The tools contain a complete set of functions to operate on 4x4 matrices.
	*/
	PVRTMat4 mMVP = PVRTMat4::Identity();

	if(PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen)) // If the screen is rotated
		mMVP = PVRTMat4::RotationZ(-1.57f);

	/*
		Pass this matrix to the shader.
		The .m field of a PVRTMat4 contains the array of float used to
		communicate with OpenGL ES.
	*/
	glUniformMatrix4fv(m_ShaderProgram.auiLoc[eMVPMatrix], 1, GL_FALSE, mMVP.ptr());

	/*
		Draw a triangle.
		Please refer to the training course IntroducingPVRShell for a detailed explanation.
	*/

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);

	// Pass the vertex data
	glEnableVertexAttribArray(VERTEX_ARRAY);
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, m_ui32VertexStride, 0);

	// Pass the texture coordinates data
	glEnableVertexAttribArray(TEXCOORD_ARRAY);
	glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, m_ui32VertexStride, (void*) (sizeof(GLfloat) * 3) /* Uvs start after the position */);

	// Draws a non-indexed triangle array
	glDrawArrays(GL_TRIANGLES, 0, 3);

	/*
		Display some text.
		Print3D() function allows to draw text anywhere on the screen using any color.
		Param 1: Position of the text along X (from 0 to 100 scale independent)
		Param 2: Position of the text along Y (from 0 to 100 scale independent)
		Param 3: Scale of the text
		Param 4: Colour of the text (0xAABBGGRR format)
		Param 5: Formatted string (uses the same syntax as printf)
	*/
	m_Print3D.Print3D(8.0f, 30.0f, 1.0f, 0xFFAA4040, "example");

	/*
		DisplayDefaultTitle() writes a title and description text on the top left of the screen.
		It can also display the PVR logo (ePVRTPrint3DLogoPVR), the IMG logo (ePVRTPrint3DLogoIMG) or both (ePVRTPrint3DLogoPVR | ePVRTPrint3DLogoIMG).
		Set this last parameter to NULL not to display the logos.
	*/
	m_Print3D.DisplayDefaultTitle("IntroducingPVRTools", "Description", ePVRTPrint3DSDKLogo);

	// Tells Print3D to do all the pending text rendering now
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
	return new OGLES2IntroducingPVRTools();
}

/******************************************************************************
 End of file (OGLES2IntroducingPVRTools.cpp)
******************************************************************************/

