/******************************************************************************

 @File         OGLES3Shaders.cpp

 @Title        Shaders

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     

 @Description  Shaders library for OpenGL ES 3.0

******************************************************************************/

/****************************************************************************
 ** INCLUDES                                                               **
 ****************************************************************************/
#include <math.h>
#include <string.h>
#include "PVRShell.h"
#include "OGLES3Tools.h"

/****************************************************************************
 ** DEFINES                                                                **
 ****************************************************************************/
#define CAMERA_DISTANCE	50
#define ALTITUDE		15
#define TO_ALTITUDE		0

#define CAM_NEAR	(4.0f)
#define CAM_FAR		(5000.0f)

#define CUBEMAP_FLAG 0x1000

#define SHADER_PATH "" 	// SHADER_PATH is always relative to the location where the exe is located
char		shader_path[100];
const char*	g_ShaderList[] = { "envmap", "directional_lighting", "anisotropic_lighting", "fasttnl", "lattice", "point_lighting", "phong_lighting", "reflections", "simple", "spot_lighting", "toon", "vertex_sine", "wood"};
const char*	g_SurfacesList[] = {"Torus", "Moebius", "KleinBottle", "BoySurface", "DiniSurface"};
const char*	g_TextureList[] = {"base", "reflection", "anisotropicmap", "cubemap"};
#define		g_numShaders	13
#define		g_numSurfaces	5
#define		g_numTextures	4

typedef void (*PFUNCTION)(float u, float v, float* x, float* y, float* z);

/******************************************************************************
 Content file names
******************************************************************************/

enum ETextures
{
	eTexAnisotropic,
	eTexBase,
	eTexReflection,
	eTexCubeMap
};

// Textures
const char * const g_aszTextureNames[g_numTextures] = {
	"AnisoMap.pvr",
	"Basetex.pvr",
	"Reflection.pvr",
	"Cubemap.pvr",
};

/****************************************************************************
 Class: ParametricSurface
 Description: This class creates the geometrical meshes to which we will apply
              our shaders library.
****************************************************************************/
class ParametricSurface
{
public:
	// Vertex buffer objects for vertices, UV's and normals.
	GLuint iVertexVBO, iUvVBO, iNormalVBO;
	float *pVertex, *pUV, *pNormal;
	unsigned short* pIndex;
	float fMinU, fMaxU, fMinV, fMaxV;
	int nSampleU, nSampleV;

	ParametricSurface(int dSampleU, int dSampleV);
	~ParametricSurface();
	int GetNumFaces();
    void ComputeVertexAndNormals(PFUNCTION function, float dMinU, float dMaxU, float dMinV, float dMaxV);
};

/****************************************************************************
 Surface functions
 Description: Mathematical surfaces to be used by ParametricSurface
****************************************************************************/
void func_Plan(float u,float v, float* x,float* y,float* z);
void func_Moebius(float u,float v, float* x,float* y,float* z);
void func_Torus(float u,float v, float* x,float* y,float* z);
void func_KleinBottle(float u,float v, float* x,float* y,float* z);
void func_BoySurface(float u,float v, float* x,float* y,float* z);
void func_DiniSurface(float u,float v, float* x,float* y,float* z);

struct Surface
{
	PFUNCTION	function;
	float fMinU, fMaxU, fMinV, fMaxV;
};

/* Mesh display list composed by the name of the geometry function and the function limits*/
Surface SurfaceList[] = {
	{func_Torus,		0,2*PVRT_PI,		0,2*PVRT_PI},
	{func_Moebius,		-PVRT_PI/6,PVRT_PI/6,	0,2*PVRT_PI},
	{func_KleinBottle,	0,2*PVRT_PI,		0,2*PVRT_PI},
	{func_BoySurface,	0.001f,PVRT_PI,	0.001f,PVRT_PI},
	{func_DiniSurface,	0,4*PVRT_PI,		.01f,1.7f}
};

/****************************************************************************
 Class: OGLES3Shaders
 To use the PowerVR framework, you have to inherit a class from PVRShell
 and implement the five virtual functions which describe how your application
 initializes, runs and releases the resources.
****************************************************************************/
class OGLES3Shaders : public PVRShell
{
    // Print 3D Class Object
	CPVRTPrint3D 	m_Print3D;

	ParametricSurface*	m_Surface;

	PVRTMat4		m_mProjection, m_mModelView, m_mView;

	int				m_nCurrentShader, m_nCurrentSurface;
	float			m_fViewAngle;
	GLuint			m_puiTextureHandle[g_numTextures];
	unsigned int	m_uiTextureFlags[g_numTextures];

	// The effect file handlers
	CPVRTPFXParser*				m_ppEffectParser[g_numShaders];
	CPVRTPFXEffect*				m_ppEffect[g_numShaders];

public:

	OGLES3Shaders();

	/* PVRShell functions */
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	void ComputeSurface(int nSurface);
	void ComputeViewMatrix();
	void DrawModel();
	void FreeMemory();
};
/*!****************************************************************************
 @Function		OGLES3Shaders
 @Return		none
 @Description	Application class constructor to set some variables.
******************************************************************************/
OGLES3Shaders::OGLES3Shaders() : 	m_fViewAngle(0),
									m_nCurrentShader(0),
									m_nCurrentSurface(0),
									m_Surface(0)
{
	for(int i = 0; i < g_numShaders; i++)
	{
		m_ppEffectParser[i] = 0;
		m_ppEffect[i] = 0;
	}
}
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
bool OGLES3Shaders::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	/*
		Analyse command line
		the format is: -s=? -m=? where s in the shader and m is the mesh
		e.g. OGLES3Shaders -s=3 -m=4
	*/
	int nClNum	= PVRShellGet(prefCommandLineOptNum);
	const SCmdLineOpt* pOpt	= (const SCmdLineOpt*)PVRShellGet(prefCommandLineOpts);

	for(int i = 0; i < nClNum; ++i)
	{
		if(pOpt[i].pArg[0] == '-' && pOpt[i].pVal)
		{
			switch (pOpt[i].pArg[1])
			{
			case 's':case 'S':
				if (pOpt[i].pArg[2] == '\0')
				{
					m_nCurrentShader = atoi(pOpt[i].pVal) % g_numShaders;
				}
				continue;

			case 'm':case 'M':
				if (pOpt[i].pArg[2] == '\0')
				{
					m_nCurrentSurface = atoi(pOpt[i].pVal) % g_numSurfaces;
				}
				continue;
			}
		}
	}

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
bool OGLES3Shaders::QuitApplication()
{
    return true;
}

/*!****************************************************************************
 @Function		InitView
 @Return		bool		true if no error occurred
 @Description	Code in InitView() will be called by PVRShell upon
				initialization or after a change in the rendering context.
				Used to initialize variables that are dependant on the rendering
				context (e.g. textures, vertex buffers, etc.)
******************************************************************************/
bool OGLES3Shaders::InitView()
{
	// Is the screen rotated
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	/* Initialize Print3D textures */
	if (m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellOutputDebug("ERROR: Cannot initialise Print3D\n");
		return false;
	}

	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	m_mProjection = PVRTMat4::PerspectiveFovRH(PVRT_PI/6, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), CAM_NEAR, CAM_FAR, PVRTMat4::OGL, bRotate);

	m_mView = PVRTMat4::Identity();

	/*
		Loads the textures.
	*/

	// Textures

	// Get pointer to the texture name
	CPVRTString ErrorStr;

	char *pTexture = 0;
	PVRTextureHeaderV3 Header;

	for(int i = 0; i < g_numTextures; ++i)
	{
		if(strcmp(g_TextureList[i], "base") == 0)
			pTexture = (char*) g_aszTextureNames[eTexBase];
		else if(strcmp(g_TextureList[i], "reflection") == 0)
			pTexture = (char*) g_aszTextureNames[eTexReflection];
		else if(strcmp(g_TextureList[i], "cubemap") == 0)
			pTexture = (char*) g_aszTextureNames[eTexCubeMap];
		else if(strcmp(g_TextureList[i], "anisotropicmap") == 0)
			pTexture = (char*) g_aszTextureNames[eTexAnisotropic];

		if(PVRTTextureLoadFromPVR(pTexture, &m_puiTextureHandle[i], &Header) != PVR_SUCCESS)
		{
			ErrorStr = CPVRTString("ERROR: Could not open texture file ") + pTexture;
			PVRShellSet(prefExitMessage, ErrorStr.c_str());
			return false;
		}

		// Get texture flags form the header
		m_uiTextureFlags[i] = (Header.u32NumFaces==6?PVRTEX_CUBEMAP:0) | (Header.u32MIPMapCount>1?PVRTEX_MIPMAP:0) | (Header.u32Depth>1?PVRTEX_VOLUME:0);
	}

	/*
		Load the effect file
	*/
	for(int j = 0; j < g_numShaders; j++)
	{
		CPVRTString		fileName;
		unsigned int	nUnknownUniformCount;
		CPVRTString error;

		/*
			Parse the file
		*/
		m_ppEffectParser[j] = new CPVRTPFXParser();
		fileName = CPVRTString(g_ShaderList[j]) + ".pfx";

		if(m_ppEffectParser[j]->ParseFromFile(fileName.c_str(), &error) != PVR_SUCCESS)
		{
			error = CPVRTString("Parse failed for ") + fileName + ":\n\n" + error;
			PVRShellSet(prefExitMessage, error.c_str());
			FreeMemory();
			return false;
		}

		/*
			Load the effect from the file
		*/
		error = "";
		m_ppEffect[j] = new CPVRTPFXEffect();
		if(m_ppEffect[j]->Load(*(m_ppEffectParser[j]), "myEffect", fileName.c_str(), NULL, nUnknownUniformCount, &error)  != PVR_SUCCESS)
		{
			PVRShellSet(prefExitMessage, error.c_str());
			FreeMemory();
			return false;
		}

		if(nUnknownUniformCount)
		{
			error = PVRTStringFromFormattedStr("PFX File: %s\n%s Unknown uniform semantic count: %d\n", fileName.c_str(), error.c_str(), nUnknownUniformCount);
			PVRShellSet(prefExitMessage, error.c_str());

			FreeMemory();
			return false;
		}
		if(!error.empty())
		{
			PVRShellOutputDebug(error.c_str());
		}

		/*
			Link the textrues to the effect.
		*/
		const CPVRTArray<SPVRTPFXTexture>& sTex = m_ppEffect[j]->GetTextureArray();

		// Loop over textures used in the CPVRTPFXEffect
		for(unsigned int i = 0; i < sTex.GetSize(); ++i)
		{
			int iTexIdx = m_ppEffectParser[j]->FindTextureByName(sTex[i].Name);
			const CPVRTStringHash& FileName = m_ppEffectParser[j]->GetTexture(iTexIdx)->FileName;

			int k;
			// Loop over available textures
			for( k = 0; k < g_numTextures; k++)
			{
				CPVRTString texName;
				texName =  CPVRTString(g_TextureList[k]) + ".pvr";
				if(FileName == texName)
				{
					// Set the current texture
					if((m_uiTextureFlags[k] & CUBEMAP_FLAG) != 0)
						glBindTexture(GL_TEXTURE_CUBE_MAP, m_puiTextureHandle[k]);
					else
						glBindTexture(GL_TEXTURE_2D, m_puiTextureHandle[k]);

					// Link the texture to the CPVRTPFXEffect and apply filtering
					m_ppEffect[j]->SetTexture(i, m_puiTextureHandle[k], m_uiTextureFlags[k]);

					break;
				}
			}
			if(k == g_numTextures)
			{
				// Texture not found
				PVRShellOutputDebug("Warning: effect file requested unrecognised texture: \"%s\"\n", FileName.c_str());
				m_ppEffect[j]->SetTexture(i, 0);
			}
		}
	}

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	// Create the surface
	m_Surface = new ParametricSurface(50,50);
	ComputeSurface(m_nCurrentSurface);

	return true;
}
/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occurred
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES3Shaders::ReleaseView()
{
	for(int j = 0; j < g_numShaders; j++)
	{
		// Release textures
		const CPVRTArray<SPVRTPFXTexture>& sTex = m_ppEffect[j]->GetTextureArray();
		for(unsigned int i = 0; i < sTex.GetSize(); ++i)
			glDeleteTextures(1, &sTex[i].ui);

		// Release the effect[s] then the parser
		delete m_ppEffect[j];
		delete m_ppEffectParser[j];
	}

	m_Print3D.ReleaseTextures();

	if(m_Surface)
	{
		delete m_Surface;
		m_Surface = 0;
	}

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
bool OGLES3Shaders::RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Keyboard input (cursor to change shaders and meshes)
	if (PVRShellIsKeyPressed(PVRShellKeyNameLEFT))
	{
		m_nCurrentShader--;
		if(m_nCurrentShader<0) m_nCurrentShader=(g_numShaders-1);
	}
	if (PVRShellIsKeyPressed(PVRShellKeyNameRIGHT))
	{
		m_nCurrentShader++;
		if(m_nCurrentShader>(g_numShaders-1)) m_nCurrentShader=0;
	}
	if (PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
	{
		m_nCurrentSurface--;
		if(m_nCurrentSurface<0) m_nCurrentSurface=(g_numSurfaces-1);
		ComputeSurface(m_nCurrentSurface);
	}
	if (PVRShellIsKeyPressed(PVRShellKeyNameUP))
	{
		m_nCurrentSurface++;
		if(m_nCurrentSurface>(g_numSurfaces-1)) m_nCurrentSurface=0;
		ComputeSurface(m_nCurrentSurface);
	}

	// Draw the mesh
	ComputeViewMatrix();
	DrawModel();

	// Display screen info
	m_Print3D.DisplayDefaultTitle("Shaders", NULL, ePVRTPrint3DSDKLogo);
	m_Print3D.Print3D(0.3f, 7.5f, 0.75f, 0xFFFFFFFF, "Shader: %s\nMesh: %s", g_ShaderList[m_nCurrentShader], g_SurfacesList[m_nCurrentSurface]);
	m_Print3D.Flush();

	return true;
}

/*******************************************************************************
 * Function Name : DrawModel
 * Description   : Draws the model
 *******************************************************************************/
void OGLES3Shaders::DrawModel()
{
	// Use the loaded effect
	m_ppEffect[m_nCurrentShader]->Activate();

	/*
		Set attributes and uniforms
	*/
	const CPVRTArray<SPVRTPFXUniform>& Uniforms = m_ppEffect[m_nCurrentShader]->GetUniformArray();

	for(unsigned int j = 0; j < Uniforms.GetSize(); ++j)
	{

		switch(Uniforms[j].nSemantic)
		{
		case ePVRTPFX_UsPOSITION:
			{
				glBindBuffer(GL_ARRAY_BUFFER, m_Surface->iVertexVBO);
				glVertexAttribPointer(Uniforms[j].nLocation, 3, GL_FLOAT, GL_FALSE, 0, (const void*) NULL);
				glEnableVertexAttribArray(Uniforms[j].nLocation);
			}
			break;
		case ePVRTPFX_UsNORMAL:
			{
				glBindBuffer(GL_ARRAY_BUFFER, m_Surface->iNormalVBO);
				glVertexAttribPointer(Uniforms[j].nLocation, 3, GL_FLOAT, GL_FALSE, 0, (const void*) NULL);
				glEnableVertexAttribArray(Uniforms[j].nLocation);
			}
			break;
		case ePVRTPFX_UsUV:
			{
				glBindBuffer(GL_ARRAY_BUFFER, m_Surface->iUvVBO);
				glVertexAttribPointer(Uniforms[j].nLocation, 2, GL_FLOAT, GL_FALSE, 0, (const void*) NULL);
				glEnableVertexAttribArray(Uniforms[j].nLocation);
			}
			break;
		case ePVRTPFX_UsWORLDVIEWPROJECTION:
			{
				PVRTMat4 mMVP;

				/* Passes the model-view-projection matrix (MVP) to the shader to transform the vertices */
				mMVP = m_mProjection * m_mModelView;
				glUniformMatrix4fv(Uniforms[j].nLocation, 1, GL_FALSE, mMVP.f);
			}
			break;
		case ePVRTPFX_UsWORLDVIEW:
			{
				glUniformMatrix4fv(Uniforms[j].nLocation, 1, GL_FALSE, m_mModelView.f);
			}
			break;
		case ePVRTPFX_UsWORLDVIEWIT:
			{
				PVRTMat4 mModelViewI, mModelViewIT;

				/* Passes the inverse transpose of the model-view matrix to the shader to transform the normals */
				mModelViewI = m_mModelView.inverse();
				mModelViewIT= mModelViewI.transpose();
				PVRTMat3 ModelViewIT = PVRTMat3(mModelViewIT);

				glUniformMatrix3fv(Uniforms[j].nLocation, 1, GL_FALSE, ModelViewIT.f);
			}
			break;
		case ePVRTPFX_UsVIEWIT:
			{
				PVRTMat4 mViewI, mViewIT;

				/* Passes the inverse transpose of the model-view matrix to the shader to transform the normals */
				mViewI = m_mView.inverse();
				mViewIT= mViewI.transpose();

				PVRTMat3 ViewIT = PVRTMat3(mViewIT);

				glUniformMatrix3fv(Uniforms[j].nLocation, 1, GL_FALSE, ViewIT.f);
			}
			break;
		case ePVRTPFX_UsTEXTURE:
			{
				// Set the sampler variable to the texture unit
				glUniform1i(Uniforms[j].nLocation, Uniforms[j].nIdx);
			}
			break;
		case ePVRTPFX_UsANIMATION:
			{
				// Float in the range 0..1: contains this objects distance through its animation.
				float fAnimation = 0.5f * m_fViewAngle / PVRT_PI;
				glUniform1f(Uniforms[j].nLocation, fAnimation);
			}
			break;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);	// Unbind the last buffer used.

	glDrawElements(GL_TRIANGLES, m_Surface->GetNumFaces()*3, GL_UNSIGNED_SHORT, m_Surface->pIndex);

	/*
		Disable attributes
	*/
	for(unsigned int j = 0; j < Uniforms.GetSize(); ++j)
	{
		switch(Uniforms[j].nSemantic)
		{
		case ePVRTPFX_UsPOSITION:
			{
				glDisableVertexAttribArray(Uniforms[j].nLocation);
			}
			break;
		case ePVRTPFX_UsNORMAL:
			{
				glDisableVertexAttribArray(Uniforms[j].nLocation);
			}
			break;
		case ePVRTPFX_UsUV:
			{
				glDisableVertexAttribArray(Uniforms[j].nLocation);
			}
			break;
		}
	}

	return;
}

/*!****************************************************************************
 @Function		ComputeSurface
 @Return		none
 @Description	Compute the a surface
******************************************************************************/
void OGLES3Shaders::ComputeSurface(int nSurface)
{
	m_Surface->ComputeVertexAndNormals( SurfaceList[nSurface].function,
										SurfaceList[nSurface].fMinU,
										SurfaceList[nSurface].fMaxU,
										SurfaceList[nSurface].fMinV,
                                        SurfaceList[nSurface].fMaxV);
}

/*******************************************************************************
 * Function Name  : ComputeViewMatrix
 * Description    : Calculate the view matrix turning around the balloon
 *******************************************************************************/
void OGLES3Shaders::ComputeViewMatrix()
{
	float factor = 1.0f * .03f;

	PVRTVec3 vFrom, vTo, vUp;
	vTo.x = 0;
	vTo.y = TO_ALTITUDE;
	vTo.z = 0;

	vUp.x = 0;
	vUp.y = 1;
	vUp.z = 0;

	/* Calculate the angle of the camera around the balloon */
	vFrom.x = CAMERA_DISTANCE * (float)cos(m_fViewAngle);
	vFrom.y = ALTITUDE;
	vFrom.z = CAMERA_DISTANCE * (float)sin(m_fViewAngle);
	m_fViewAngle += factor;

	while(m_fViewAngle > 2*PVRT_PI)
		m_fViewAngle = m_fViewAngle - 2*PVRT_PI;

	/* Compute and set the matrix */
	m_mModelView = PVRTMat4::LookAtRH(vFrom, vTo, vUp);
}

/*!****************************************************************************
 @Function		FreeMemory
 @Description	Free allocated memory.
******************************************************************************/
void OGLES3Shaders::FreeMemory()
{
	for(int j = 0; j < g_numShaders; j++)
	{
		// Release textures
		if(m_ppEffect[j])
		{
			const CPVRTArray<SPVRTPFXTexture>& sTex = m_ppEffect[j]->GetTextureArray();
			for(unsigned int i = 0; i < sTex.GetSize(); ++i)
				glDeleteTextures(1, &(sTex[i].ui));
		}

		// Release the effect[s] then the parser
		if(m_ppEffect[j])
			delete m_ppEffect[j];
		if(m_ppEffectParser[j])
			delete m_ppEffectParser[j];
	}

	m_Print3D.ReleaseTextures();
}

/*******************************************************************************
 * Function Name : NewDemo
 * Returns       : true if no problem
 * Description   : Register our demo class
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new OGLES3Shaders();
}

/*****************************************************************************
 ParametricSurface
*****************************************************************************/
ParametricSurface::~ParametricSurface()
{
	glDeleteBuffers(1, &iVertexVBO);
	glDeleteBuffers(1, &iUvVBO);
	glDeleteBuffers(1, &iNormalVBO);
	delete[] pIndex;
}

ParametricSurface::ParametricSurface(int dSampleU, int dSampleV) : iVertexVBO(0), iUvVBO(0), iNormalVBO(0), pVertex(0),
																	pUV(0),pNormal(0), pIndex(0), fMinU(0), fMaxU(0), fMinV(0), 
																	fMaxV(0), nSampleU(dSampleU), nSampleV(dSampleV)
{
	pIndex = new unsigned short[GetNumFaces()*3];

	// Generate three vertex buffer objects.
	glGenBuffers(1, &iVertexVBO);
	glGenBuffers(1, &iUvVBO);
	glGenBuffers(1, &iNormalVBO);

	for (int i=0; i<nSampleU-1; i++)
	{
		for (int j=0; j<nSampleV-1; j++)
		{
			pIndex[ (j*(nSampleU-1)+i)*6 + 0 ] = (unsigned short) (j * nSampleU + i);
			pIndex[ (j*(nSampleU-1)+i)*6 + 1 ] = (unsigned short) (j * nSampleU + (i+1));
			pIndex[ (j*(nSampleU-1)+i)*6 + 2 ] = (unsigned short) ((j+1) * nSampleU + (i+1));
			pIndex[ (j*(nSampleU-1)+i)*6 + 3 ] = (unsigned short) (j * nSampleU + i);
			pIndex[ (j*(nSampleU-1)+i)*6 + 4 ] = (unsigned short) ((j+1) * nSampleU + (i+1));
			pIndex[ (j*(nSampleU-1)+i)*6 + 5 ] = (unsigned short) ((j+1) * nSampleU + i);
		}
	}
}
int ParametricSurface::GetNumFaces()
{
	return (nSampleU-1)*(nSampleV-1)*2;
}

void ParametricSurface::ComputeVertexAndNormals(PFUNCTION function, float dMinU, float dMaxU, float dMinV, float dMaxV)
{
	int nVertex = nSampleU * nSampleV;
	pVertex = new float[nVertex*3];
	pNormal = new float[nVertex*3];
	pUV = new float[nVertex*2];

	fMinU = dMinU;
	fMaxU = dMaxU;
	fMinV = dMinV;
	fMaxV = dMaxV;

	for (int i=0; i<nSampleU; i++)
	{
		for (int j=0; j<nSampleV; j++)
		{
			float u = fMinU + i * (fMaxU-fMinU) / (float)(nSampleU-1);
			float v = fMinV + j * (fMaxV-fMinV) / (float)(nSampleV-1);
			float x,y,z;
            function(u,v, &x,&y,&z);
			pVertex[(j*nSampleU+i)*3 + 0] = x;
			pVertex[(j*nSampleU+i)*3 + 1] = y;
			pVertex[(j*nSampleU+i)*3 + 2] = z;
		}
	}

	for (int i=0; i<nSampleU; i++)
	{
		for (int j=0; j<nSampleV; j++)
		{
			pUV[ (j*nSampleU+i)*2 + 0 ] = (float)i / (float)(nSampleU-1);
			pUV[ (j*nSampleU+i)*2 + 1 ] = (float)j / (float)(nSampleV-1);
		}
	}

	for (int i=0; i<nSampleU-1; i++)
	{
		for (int j=0; j<nSampleV-1; j++)
		{
			PVRTVec3 ptA = PVRTVec3(pVertex[(j*nSampleU+i)*3+0],pVertex[(j*nSampleU+i)*3+1],pVertex[(j*nSampleU+i)*3+2]);
			PVRTVec3 ptB = PVRTVec3(pVertex[(j*nSampleU+i+1)*3+0],pVertex[(j*nSampleU+i+1)*3+1],pVertex[(j*nSampleU+i+1)*3+2]);
			PVRTVec3 ptC = PVRTVec3(pVertex[((j+1)*nSampleU+i)*3+0],pVertex[((j+1)*nSampleU+i)*3+1],pVertex[((j+1)*nSampleU+i)*3+2]);
			PVRTVec3 AB = PVRTVec3(ptB.x-ptA.x, ptB.y-ptA.y, ptB.z-ptA.z);
			PVRTVec3 AC = PVRTVec3(ptC.x-ptA.x, ptC.y-ptA.y, ptC.z-ptA.z);
			PVRTVec3 normal;

			normal = AB.cross(AC);
			normal.normalize();

			pNormal[(j*nSampleU+i)*3 + 0] = -normal.x;
			pNormal[(j*nSampleU+i)*3 + 1] = -normal.y;
			pNormal[(j*nSampleU+i)*3 + 2] = -normal.z;
		}
	}

	for (int i=0; i<nSampleU-1; i++)
	{
		pNormal[((nSampleV-1)*nSampleU+i)*3+0] = pNormal[(i)*3+0];
		pNormal[((nSampleV-1)*nSampleU+i)*3+1] = pNormal[(i)*3+1];
		pNormal[((nSampleV-1)*nSampleU+i)*3+2] = pNormal[(i)*3+2];
	}

	for (int j=0; j<nSampleV-1; j++)
	{
		pNormal[(j*nSampleU+nSampleU-1)*3+0] = pNormal[(j*nSampleU)*3+0];
		pNormal[(j*nSampleU+nSampleU-1)*3+1] = pNormal[(j*nSampleU)*3+1];
		pNormal[(j*nSampleU+nSampleU-1)*3+2] = pNormal[(j*nSampleU)*3+2];
	}

	pNormal[((nSampleV-1)*nSampleU + (nSampleU-1))*3+0]= pNormal[((nSampleV-2)*nSampleU + (nSampleU-2))*3+0];
	pNormal[((nSampleV-1)*nSampleU + (nSampleU-1))*3+1]= pNormal[((nSampleV-2)*nSampleU + (nSampleU-2))*3+1];
	pNormal[((nSampleV-1)*nSampleU + (nSampleU-1))*3+2]= pNormal[((nSampleV-2)*nSampleU + (nSampleU-2))*3+2];

	// Insert generated data into vertex buffer objects.
    glBindBuffer(GL_ARRAY_BUFFER, iVertexVBO);
	glBufferData(GL_ARRAY_BUFFER, nVertex * 3 * sizeof (float), pVertex, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, iUvVBO);
	glBufferData(GL_ARRAY_BUFFER, nVertex * 2 * sizeof (float), pUV, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, iNormalVBO);
	glBufferData(GL_ARRAY_BUFFER, nVertex * 3 * sizeof (float), pNormal, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind the last buffer used.

	delete[] pVertex;
	delete[] pNormal;
	delete[] pUV;
}



void func_Plan(float u,float v, float* x,float* y,float* z)
{
	*x = u;
	*y = 0;
	*z = v;
}

void func_Moebius(float u,float v, float* x,float* y,float* z)
{
	float R = 9;
	*x = R * ((float) cos(v) + u * (float) cos(v / 0.5f) * (float) cos(v));
	*y = R * ((float) sin(v) + u * (float) cos(v / 0.5f) * (float) sin(v));
	*z = R * u * (float) sin(v / 0.5f);
}

void func_Torus(float u,float v, float* x,float* y,float* z)
{
	float R=2, r=4;
	*x = R * (float) cos(v) * (r + (float) cos(u));
	*y = R * (float) sin(v) * (r + (float) cos(u));
	*z = R * (float) sin(u);
}

void func_KleinBottle(float u,float v, float* x,float* y,float* z)
{
	float botx = (6-2)  * (float) cos(u) * (1 + (float) sin(u));
	float boty = (16-4) * (float) sin(u);
	float rad  = (4-1)  * (1 - (float) cos(u)/2);

	if (u > 1.7 * PVRT_PI)
	{
		*x = botx + rad * (float) cos(u) * (float) cos(v);
		*y = boty + rad * (float) sin(u) * (float) cos(v);
	}
	else if (u > PVRT_PI)
	{
		*x = botx + rad * (float) cos(v+PVRT_PI);
		*y = boty;
	}
	else
	{
		*x = botx + rad * (float) cos(u) * (float) cos(v);
		*y = boty + rad * (float) sin(u) * (float) cos(v);
	}

	*z = rad * (float) -sin(v);
	*y -= 2;
}
void func_BoySurface(float u,float v, float* x,float* y,float* z)
{
	float a = (float) cos(u*0.5f) * (float) sin(v);
	float b = (float) sin(u*0.5f) * (float) sin(v);
	float c = (float) cos(v);
	*x = ( (2*a*a-b*b-c*c) + 2*b*c*(b*b-c*c) + c*a*(a*a-c*c) + a*b*(b*b-a*a) ) / 2;
	*y = ( (b*b-c*c) + c*a*(c*c-a*a) + a*b*(b*b-a*a) ) * (float) sqrt(3.0f) / 2;
	*z = (a+b+c) * ( (a+b+c)*(a+b+c)*(a+b+c) + 4*(b-a)*(c-b)*(a-c) )/8;
	*x*=10;
	*y*=10;
	*z*=10;
}
void func_DiniSurface(float u,float v, float* x,float* y,float* z)
{
	*x = (float)  cos(u) * (float) sin(v);
	*y = (float) -cos(v) - (float) log((float) tan(v/2)) - .2f*u;
	*z = (float) -sin(u) * (float) sin(v);
	*x*=5;
	*y*=4;
	*z*=5;
	*y-=3;
}

/*****************************************************************************
 End of file (OGLES3Shaders.cpp)
*****************************************************************************/

