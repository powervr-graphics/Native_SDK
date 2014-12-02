/******************************************************************************

 @File         OGLES3TextureArray.cpp

 @Title        Texture Arrays

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows how to use textures in OpenGL ES 3.0

******************************************************************************/

#include "PVRShell.h"
#include "OGLES3Tools.h"

/******************************************************************************
 Defines
******************************************************************************/

// Index to bind the attributes to vertex shaders
#define VERTEX_ARRAY	0
#define TEXCOORD_ARRAY	1
#define TEXINDEX_ARRAY	2

/******************************************************************************
 Structures and enums
******************************************************************************/

typedef struct
{
	PVRTVec2 position;
	PVRTVec2 texcoord;
	float    index;
} Vertex;

enum eCustomSemantics
{
	eCUSTOMSEMANTIC_TEXINDEX = ePVRTPFX_NumSemantics + 1,	
};

const SPVRTPFXUniformSemantic c_CustomSemantics[] = 
{ 
	{ "CUSTOMSEMANTIC_TEXINDEX", eCUSTOMSEMANTIC_TEXINDEX },
};
const unsigned int c_uiNumCustomSemantics = sizeof(c_CustomSemantics)/sizeof(c_CustomSemantics[0]);

/******************************************************************************
 Content file names
******************************************************************************/

const char *c_aszTextureFile = "textureArray.pvr";
const char c_szPFXSrcFile[]	 = "effect.pfx";

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES3TextureArray : public PVRShell
{	
	CPVRTPrint3D m_Print3D;
	SPVRTContext m_sContext;

	// OpenGL handles
	GLuint m_ui32Vbo;
	GLuint m_uiTexture;

	// The effect file handlers
	CPVRTPFXParser	*m_pPFXEffectParser;
	CPVRTPFXEffect **m_ppPFXEffects;	
	
public:

	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();
	
	bool LoadPFX(CPVRTString *pErrorMsg);
	bool LoadTextures(CPVRTString* const pErrorStr);
};

/*!****************************************************************************
 @Function		LoadTextures
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course
******************************************************************************/
bool OGLES3TextureArray::LoadTextures(CPVRTString* const pErrorStr)
{
	//
	// Load texture array
	//	

	if (PVR_SUCCESS != PVRTTextureLoadFromPVR(c_aszTextureFile, &m_uiTexture))
	{
		*pErrorStr = "PVRTTextureArrayLoadFromPVRs() failed\n";
		return false;
	}
	
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return true;
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
bool OGLES3TextureArray::InitApplication()
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
bool OGLES3TextureArray::QuitApplication()
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
bool OGLES3TextureArray::InitView()
{   
	CPVRTString ErrorStr;

	/*
		Initialize VBO data
	*/
	Vertex vertices[] = 
	{ 
		// First primitive, bound to texture 0
		{ PVRTVec2(-1.0f, -1.0f), PVRTVec2(0.0f, 0.0f), 0.0f },
		{ PVRTVec2( 0.0f, -1.0f), PVRTVec2(1.0f, 0.0f), 0.0f },
		{ PVRTVec2( 0.0f,  0.0f), PVRTVec2(1.0f, 1.0f), 0.0f },

		{ PVRTVec2(-1.0f, -1.0f), PVRTVec2(0.0f, 0.0f), 0.0f },
		{ PVRTVec2( 0.0f,  0.0f), PVRTVec2(1.0f, 1.0f), 0.0f },
		{ PVRTVec2(-1.0f,  0.0f), PVRTVec2(0.0f, 1.0f), 0.0f },
		
		// First primitive, bound to texture 1
		{ PVRTVec2( 0.0f, -1.0f), PVRTVec2(0.0f, 0.0f), 1.0f },
		{ PVRTVec2( 1.0f, -1.0f), PVRTVec2(1.0f, 0.0f), 1.0f },
		{ PVRTVec2( 1.0f,  0.0f), PVRTVec2(1.0f, 1.0f), 1.0f },

		{ PVRTVec2( 0.0f, -1.0f), PVRTVec2(0.0f, 0.0f), 1.0f },
		{ PVRTVec2( 1.0f,  0.0f), PVRTVec2(1.0f, 1.0f), 1.0f },
		{ PVRTVec2( 0.0f,  0.0f), PVRTVec2(0.0f, 1.0f), 1.0f },
		
		// First primitive, bound to texture 2
		{ PVRTVec2(-1.0f,  0.0f), PVRTVec2(0.0f, 0.0f), 2.0f },
		{ PVRTVec2( 0.0f,  0.0f), PVRTVec2(1.0f, 0.0f), 2.0f },
		{ PVRTVec2( 0.0f,  1.0f), PVRTVec2(1.0f, 1.0f), 2.0f },

		{ PVRTVec2(-1.0f,  0.0f), PVRTVec2(0.0f, 0.0f), 2.0f },
		{ PVRTVec2( 0.0f,  1.0f), PVRTVec2(1.0f, 1.0f), 2.0f },
		{ PVRTVec2(-1.0f,  1.0f), PVRTVec2(0.0f, 1.0f), 2.0f },
		
		// First primitive, bound to texture 3
		{ PVRTVec2( 0.0f,  0.0f), PVRTVec2(0.0f, 0.0f), 3.0f },
		{ PVRTVec2( 1.0f,  0.0f), PVRTVec2(1.0f, 0.0f), 3.0f },
		{ PVRTVec2( 1.0f,  1.0f), PVRTVec2(1.0f, 1.0f), 3.0f },

		{ PVRTVec2( 0.0f,  0.0f), PVRTVec2(0.0f, 0.0f), 3.0f },
		{ PVRTVec2( 1.0f,  1.0f), PVRTVec2(1.0f, 1.0f), 3.0f },
		{ PVRTVec2( 0.0f,  1.0f), PVRTVec2(0.0f, 1.0f), 3.0f },
		
	};
	glGenBuffers(1, &m_ui32Vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
		
	/*
		Initialize Print3D
	*/

	// Is the screen rotated?
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	if(m_Print3D.SetTextures(NULL, PVRShellGet(prefWidth), PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}	

	/*
		Load and compile the shaders & link programs
	*/
	if (!LoadPFX(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	if (!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
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
bool OGLES3TextureArray::ReleaseView()
{
	// Release the effect[s] then the parser
	if (m_pPFXEffectParser)
		for (unsigned int i=0; i < m_pPFXEffectParser->GetNumberEffects(); i++)
			if (m_ppPFXEffects[i])
				delete m_ppPFXEffects[i];
	delete [] m_ppPFXEffects;
	delete m_pPFXEffectParser;

	// Release textures
	glDeleteTextures(1, &m_uiTexture);

	// Release Vertex buffer object.
	glDeleteBuffers(1, &m_ui32Vbo);	

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
bool OGLES3TextureArray::RenderScene()
{
	// Clears the color buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Activate the passed effect
	m_ppPFXEffects[0]->Activate();

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);

	// Bind semantics
	const CPVRTArray<SPVRTPFXUniform>& Uniforms = m_ppPFXEffects[0]->GetUniformArray();
	for(unsigned int j = 0; j < Uniforms.GetSize(); ++j)
	{
		switch(Uniforms[j].nSemantic)
		{
		case ePVRTPFX_UsPOSITION:
			{
				glVertexAttribPointer(VERTEX_ARRAY, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
				glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(PVRTVec2));

				
				glEnableVertexAttribArray(Uniforms[j].nLocation);
			}
			break;
		case ePVRTPFX_UsUV:
			{
				glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(PVRTVec2));
				glEnableVertexAttribArray(Uniforms[j].nLocation);
			}
			break;
		case eCUSTOMSEMANTIC_TEXINDEX:
			{
				glVertexAttribPointer(TEXINDEX_ARRAY, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(PVRTVec2) + sizeof(PVRTVec2)));
				glEnableVertexAttribArray(Uniforms[j].nLocation);
			}
			break;
		case ePVRTPFX_UsTEXTURE:
			{
				// Set the sampler variable to the texture unit
				glUniform1i(Uniforms[j].nLocation, Uniforms[j].nIdx);
			}		
			break;
		default:
			{
				PVRShellOutputDebug("Error: Unhandled semantic in RenderSceneWithEffect()\n");
				return false;
			}
		}
	}

	//	Now that all uniforms are set and the materials ready, draw the mesh.		
	glDrawArrays(GL_TRIANGLES, 0, 24);

	// Disable all vertex attributes
	for(unsigned int j = 0; j < Uniforms.GetSize(); ++j)
	{
		switch(Uniforms[j].nSemantic)
		{
		case ePVRTPFX_UsPOSITION:
		case ePVRTPFX_UsUV:
			glDisableVertexAttribArray(Uniforms[j].nLocation);
			break;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	m_Print3D.DisplayDefaultTitle("Texture Array", NULL, ePVRTPrint3DSDKLogo);	
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		LoadPFX
  @Return		bool			true if no error occurred
 @Description	Loads and compiles the shaders and links the shader programs
				required for this training course
******************************************************************************/
bool OGLES3TextureArray::LoadPFX(CPVRTString *pErrorMsg)
{
	CPVRTString error = "";

	// Parse the whole PFX and store all data.
	m_pPFXEffectParser = new CPVRTPFXParser();
	if(m_pPFXEffectParser->ParseFromFile(c_szPFXSrcFile, &error) != PVR_SUCCESS)
	{
		error = "Parse failed:\n\n" + error;
		PVRShellSet(prefExitMessage, error.c_str());
		return false;
	}
	
	// Setup all effects in the PFX file so we initialize the shaders and
	// store uniforms and attributes locations.
	unsigned int uiNumEffects = m_pPFXEffectParser->GetNumberEffects();
	m_ppPFXEffects = new CPVRTPFXEffect*[uiNumEffects];

	// Load one by one the effects. This will also compile the shaders.
	for (unsigned int i=0; i < uiNumEffects; i++)
	{
		m_ppPFXEffects[i] = new CPVRTPFXEffect(m_sContext);

		if(m_ppPFXEffects[i]->RegisterUniformSemantic(c_CustomSemantics, c_uiNumCustomSemantics, &error))
		{
			*pErrorMsg = CPVRTString("Failed to set custom semantics:\n") + error;
			return false;
		}

		unsigned int nUnknownUniformCount = 0;
		if(m_ppPFXEffects[i]->Load(*m_pPFXEffectParser, m_pPFXEffectParser->GetEffect(i).Name.c_str(), NULL, NULL, nUnknownUniformCount, &error)  != PVR_SUCCESS)
		{
			*pErrorMsg = CPVRTString("Failed to load effect ") + m_pPFXEffectParser->GetEffect(i).Name.String() + CPVRTString(":\n\n") + error;
			return false;
		}

		// .. upps, some uniforms are not in our table. Better to quit because something is not quite right.
		if(nUnknownUniformCount)
		{
			*pErrorMsg = CPVRTString("Unknown uniforms found in effect: ") + m_pPFXEffectParser->GetEffect(i).Name.String();
			return false;
		}		
	}
	
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
	return new OGLES3TextureArray();
}

/******************************************************************************
 End of file (OGLES3TextureArray.cpp)
******************************************************************************/

