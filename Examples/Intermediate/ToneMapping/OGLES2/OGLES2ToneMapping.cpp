/******************************************************************************
 
 @File         OGLES2ToneMapping.cpp
 
 @Title        Tone Mapping
 
 @Version
 
 @Copyright    Copyright (c) Imagination Technologies Limited.
 
 @Platform     Independant
 
 @Description  Demonstrates various tone mapping operators with a high
			   dynamic range floating point texture.
 
 ******************************************************************************/
#include "PVRShell.h"
#include "OGLES2Tools.h"

/******************************************************************************
 Constants
 ******************************************************************************/
const char* c_pszTexture = "hdrscene.pvr";
const char* c_pszPFXFile = "effects.pfx";

const int c_iMinStop     = -5;
const int c_iMaxStop     = +5;
const int c_iMeteredExp  = 6;

// Operator enumeration
enum EOperators
{
	eOp_Linear,
	eOp_Reinhard,
	eOp_HejlBurgessDawson,
	eOp_Uncharted2,
	
	eOp_MAX,
	eOp_First = 0,
	eOp_Last  = eOp_MAX-1
};

// Custom PFX semantics
enum ECustomSemantic
{
	eUsEXPOSURE = ePVRTPFX_NumSemantics
};

const char* c_pszOpNames[] =
{
	"Linear",				// eOp_Linear
	"Reinhard",				// eOp_Reinhard
	"HejlBurgessDawson",	// eOp_HejlBurgessDawson
	"Uncharted2",			// eOp_Uncharted2
};

const SPVRTPFXUniformSemantic c_sCustomSemantics[] =
{
	{ "EXPOSURE", eUsEXPOSURE },
};

/*!****************************************************************************
 Class implementing the PVRShell functions.
 ******************************************************************************/
class OGLES2ToneMapping : public PVRShell, public PVRTPFXEffectDelegate
{
	// Print3D object
	CPVRTPrint3D			m_Print3D;
	
	// Texture handle
	GLuint					m_uiTexture;
	
	// VBO handle
	GLuint					m_ui32Vbo;
	
	// Stride for vertex data
	unsigned int			m_ui32VertexStride;
	
	// Effects
	CPVRTPFXParser*			m_pPFXParser;
	CPVRTPFXEffect*			m_pPFXEffect[eOp_MAX];
	
	// Example members
	int						m_iCurrentOp;
	int						m_iStops;
	
public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();
	
protected:
	virtual EPVRTError PVRTPFXOnLoadTexture(const CPVRTStringHash& TextureName, GLuint& uiHandle, unsigned int& uiFlags);
	
private:
	bool LoadEffects(CPVRTString& ErrorStr);
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
bool OGLES2ToneMapping::InitApplication()
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
	
	m_pPFXParser = NULL;
	memset(m_pPFXEffect, 0, sizeof(CPVRTPFXEffect*) * eOp_MAX);
	
	m_uiTexture        = 0;
	m_ui32Vbo          = 0;
	m_ui32VertexStride = 0;
	m_iStops           = 0;
	
	m_iCurrentOp       = eOp_First;
	
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
bool OGLES2ToneMapping::QuitApplication()
{
    return true;
}

/*!****************************************************************************
 @Function		PVRTPFXOnLoadTexture
 @Input			TextureName
 @Output		uiHandle
 @Output		uiFlags
 @Return		EPVRTError		PVR_SUCCESS on success.
 @Description	Callback for texture load.
 ******************************************************************************/
EPVRTError OGLES2ToneMapping::PVRTPFXOnLoadTexture(const CPVRTStringHash& TextureName, GLuint& uiHandle, unsigned int& uiFlags)
{
	uiHandle = m_uiTexture;		// The texture is already loaded. Simply set the handle.
	return PVR_SUCCESS;
}

/*!****************************************************************************
 @Function		LoadEffects
 @Output		ErrorStr	A description of an error, if one occurs.
 @Return		bool		true if no error occured
 @Description	Loads and parses the bundled PFX and generates the various
                effect objects.
 ******************************************************************************/
bool OGLES2ToneMapping::LoadEffects(CPVRTString& ErrorStr)
{
	// Load and parse the effect file
	m_pPFXParser = new CPVRTPFXParser;
	if(m_pPFXParser->ParseFromFile(c_pszPFXFile, &ErrorStr) != PVR_SUCCESS)
	{
		return false;
	}
	
	unsigned int uiUnknownUniforms;
	for(int iOp = 0; iOp < eOp_MAX; ++iOp)
	{
		uiUnknownUniforms = 0;
		m_pPFXEffect[iOp] = new CPVRTPFXEffect;
		m_pPFXEffect[iOp]->RegisterUniformSemantic(c_sCustomSemantics, sizeof(c_sCustomSemantics) / sizeof(c_sCustomSemantics[0]), &ErrorStr);
		
		if(m_pPFXEffect[iOp]->Load(*m_pPFXParser, c_pszOpNames[iOp], c_pszPFXFile, this, uiUnknownUniforms, &ErrorStr) != PVR_SUCCESS)
		{
			return false;
		}
		
		if(uiUnknownUniforms > 0)
		{
			ErrorStr += PVRTStringFromFormattedStr("WARNING: Unknown uniforms detected in effect '%s'\n", c_pszOpNames[iOp]);
		}
	}
	
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
bool OGLES2ToneMapping::InitView()
{
	// Check to see if GL_OES_texture_float is supported
	if(!CPVRTgles2Ext::IsGLExtensionSupported("GL_OES_texture_float") && !CPVRTgles2Ext::IsGLExtensionSupported("GL_OES_texture_float_linear"))
	{
		PVRShellSet(prefExitMessage, "Error: Unable to run this example as it requires extension 'GL_OES_texture_float'");
		return false;
	}

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
	 Creates the texture and sets parameters
	 */
	if(PVRTTextureLoadFromPVR(c_pszTexture, &m_uiTexture) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load HDR texture\n");
		return false;
	}

	/*
	 Load the effects
	 */
	CPVRTString ErrorStr;
	if(!LoadEffects(ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}
	
	// Create VBO for the triangle from our data
	// Interleaved vertex data
	GLfloat afVertices[] = {
		-1.0f,  1.0f, 0.0f,	1.0f,	// Pos
		 0.0f,  0.0f,			    // UVs
		
		-1.0f, -1.0f, 0.0f,	1.0f,
		 0.0f,  1.0f,
		
		 1.0f,  1.0f, 0.0f,	1.0f,
		 1.0f,  0.0f,
	
		 1.0f, -1.0f, 0.0f,	1.0f,
		 1.0f,  1.0f,
		};
	
	glGenBuffers(1, &m_ui32Vbo);
	m_ui32VertexStride = 6 * sizeof(GLfloat); // 4 floats for the pos, 2 for the UVs
	
	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);
	
	// Set the buffer's data
	glBufferData(GL_ARRAY_BUFFER, 4 * m_ui32VertexStride, afVertices, GL_STATIC_DRAW);
	
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
bool OGLES2ToneMapping::ReleaseView()
{
	// Frees the texture
	glDeleteTextures(1, &m_uiTexture);
	
	// Release Vertex buffer object.
	glDeleteBuffers(1, &m_ui32Vbo);
	
	// Release effects
	for(int i = 0; i < eOp_MAX; ++i)
		delete m_pPFXEffect[i];
	
	delete m_pPFXParser;
	
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
bool OGLES2ToneMapping::RenderScene()
{
	// Clears the color buffer
	glClear(GL_COLOR_BUFFER_BIT);
		
	/*
	 Creates the Model View Projection (MVP) matrix using the PVRTMat4 class from the tools.
	 The tools contain a complete set of functions to operate on 4x4 matrices.
	 */
	PVRTMat4 mMVP = PVRTMat4::Identity();
	
	if(PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen)) // If the screen is rotated
		mMVP = PVRTMat4::RotationZ(-1.57f);
	
	/*
	 Process input to switch between tone mapping operators
	 */
	if(PVRShellIsKeyPressed(PVRShellKeyNameUP))
	{
		m_iCurrentOp++;
		if(m_iCurrentOp > eOp_Last) m_iCurrentOp = eOp_First;
	}
	if(PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
	{
		m_iCurrentOp--;
		if(m_iCurrentOp < eOp_First) m_iCurrentOp = eOp_Last;
	}
	if(PVRShellIsKeyPressed(PVRShellKeyNameLEFT))
	{
		if(m_iStops > c_iMinStop)
			m_iStops--;
	}
	if(PVRShellIsKeyPressed(PVRShellKeyNameRIGHT))
	{
		if(m_iStops < c_iMaxStop)
			m_iStops++;
	}
	
	/* 
	 Process the uniforms and attributes from the current effect.
	 */
	CPVRTPFXEffect* pEffect = m_pPFXEffect[m_iCurrentOp];
	pEffect->Activate();
	
	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);
	
	const CPVRTArray<SPVRTPFXUniform>& aUniforms = pEffect->GetUniformArray();
	for(unsigned int i = 0; i < aUniforms.GetSize(); ++i)
	{
		switch (aUniforms[i].nSemantic)
		{
			case ePVRTPFX_UsPOSITION:
			{
				glEnableVertexAttribArray(aUniforms[i].nLocation);
				glVertexAttribPointer(aUniforms[i].nLocation, 4, GL_FLOAT, GL_FALSE, m_ui32VertexStride, 0);
			}
				break;
			case ePVRTPFX_UsUV:
			{
				glEnableVertexAttribArray(aUniforms[i].nLocation);
				glVertexAttribPointer(aUniforms[i].nLocation, 2, GL_FLOAT, GL_FALSE, m_ui32VertexStride, (void*) (4 * sizeof(GLfloat)));
			}
				break;
			case ePVRTPFX_UsWORLDVIEWPROJECTION:
			{
				glUniformMatrix4fv(aUniforms[i].nLocation, 1, GL_FALSE, mMVP.ptr());
			}
				break;
			case ePVRTPFX_UsTEXTURE:
			{
				glUniform1i(aUniforms[i].nLocation, 0);
			}
				break;
			case eUsEXPOSURE:
			{
				float fExp = (float)pow(2.0f, c_iMeteredExp+m_iStops);
				glUniform1f(aUniforms[i].nLocation, fExp);
			}
				break;
			default:
			{
				_ASSERT(!"Unhandled uniform!");
			}
				break;
		}
	}
	
	/*
	 Draw a screen-aligned quad.
	 */
	
	// Draws a non-indexed triangle array
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	/*
	 Disable vertex attributes
	 */
	for(unsigned int i = 0; i < aUniforms.GetSize(); ++i)
	{
		switch (aUniforms[i].nSemantic)
		{
			case ePVRTPFX_UsPOSITION:
			case ePVRTPFX_UsUV:
			{
				glDisableVertexAttribArray(aUniforms[i].nLocation);
			}
				break;
		}
	}
	
	// Unbind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Render exposure text
	char cPrefix = ' ';
	if(m_iStops < 0)
		cPrefix = '-';
	else if(m_iStops > 0)
		cPrefix = '+';
		
	m_Print3D.Print3D(0, 90, 0.8f, 0xFFFFFFFF, "Exposure: %c%d stop(s)", cPrefix, abs(m_iStops));
	
	// Render title
	m_Print3D.DisplayDefaultTitle("HDR ToneMapping", c_pszOpNames[m_iCurrentOp], ePVRTPrint3DSDKLogo);
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
	return new OGLES2ToneMapping();
}

/******************************************************************************
 End of file (OGLES2ToneMapping.cpp)
 ******************************************************************************/

