/******************************************************************************

 @File         OGLES3Coverflow.cpp

 @Title        Coverflow

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Demonstrates how to do coverflow

******************************************************************************/

#include "PVRShell.h"
#include "OGLES3Tools.h"
#include <stddef.h>

// Index to bind the attributes to vertex shaders
#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define COLOR_ARRAY		2
#define TEXCOORD_ARRAY	3

// Source and binary shaders
const char c_szFragShaderSrcFile[]	= "FragShader.fsh";
const char c_szFragShaderBinFile[]	= "FragShader.fsc";
const char c_szVertShaderSrcFile[]	= "VertShader.vsh";
const char c_szVertShaderBinFile[]	= "VertShader.vsc";

const float g_fCameraNear = 2.0f;
const float g_fCameraFar  = 5000.0f;
const float g_FOV = 0.78539819f;

enum EMeshOrder
{
	eLeft0,
	eLeft1,
	eLeft2,
	eLeft3,
	eLeft4,
	eFront,
	eRight0,
	eRight1,
	eRight2,
	eRight3,
	eRight4,
	eCoverMeshsNo
};

struct SCover
{
	const char*	pTextureName;
	GLuint		ui32TexID;
};

SCover g_Covers[] = {
	{ "Album1.pvr",	0 },
	{ "Album2.pvr",	0 },
	{ "Album3.pvr",	0 },
	{ "Album4.pvr",	0 },
	{ "Album5.pvr",	0 },
	{ "Album6.pvr",	0 },
	{ "Album7.pvr",	0 },
	{ "Album8.pvr",	0 },
	{ "Album9.pvr",	0 },
	{ "Album10.pvr",	0 },
	{ "Album11.pvr",	0 },
	{ "Album12.pvr",	0 },
	{ "Album13.pvr",	0 },
	{ "Album14.pvr",	0 },
	{ "Album15.pvr",	0 },
	{ "Album16.pvr",	0 }
};

const int g_i32CoverNo = sizeof(g_Covers) / sizeof(SCover);

struct SVertex
{
	PVRTVec3	p;
	PVRTVec3	n;
	PVRTVec4	c;
	PVRTVec2	t;
};

const unsigned int NormalOffset = (unsigned int)sizeof(PVRTVec3);
const unsigned int ColorOffset = NormalOffset+(unsigned int)sizeof(PVRTVec3);
const unsigned int TexCoordOffset = ColorOffset+(unsigned int)sizeof(PVRTVec4);

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLES3Coverflow : public PVRShell
{
	CPVRTPrint3D	m_Print3D;
	
	GLuint m_uiVertShader;
	GLuint m_uiFragShader;

	GLuint	m_uiVbo;
	GLuint	m_uiBlendIndexVbo;
	GLuint	m_uiOpaqueIndexVbo;

	SVertex m_coverPoints[32];

	unsigned short m_indicesOpaque[12];
	unsigned short m_indicesBlend[96];

	float m_fBorderFraction;

	PVRTMat4		m_mProjection, m_mView;

	// Group shader programs and their uniform locations together
	struct
	{
		GLuint uiId;
		GLuint uiMVPMatrixLoc;
	}
	m_ShaderProgram;
	
	float m_fLerpDir;
	float m_fLerp;
	unsigned long m_ulTimePrev;
	float m_fCyclesPerSecond;
	int m_iCoverIndex;
	int m_iNumOpaque;
	int m_iNumBlend;
	bool m_bGoRight;
	
	bool LoadTextures(CPVRTString* pErrorStr);
	bool LoadShaders(CPVRTString* pErrorStr);
	void CreateCover();
	void DrawInPosition(int index, float queueLerp, int coverIndex);
	void DrawLeftCovers();
	void DrawRightCovers();
	void DrawMesh();

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
				Used to initialize variables that are not dependant on it
				(e.g. external modules, loading meshes, etc.)
				If the rendering context is lost, InitApplication() will
				not be called again.
******************************************************************************/
bool OGLES3Coverflow::InitApplication()
{
	m_fLerp = 0.f;
	m_fLerpDir = 0;
	m_ulTimePrev = 0;
	m_fCyclesPerSecond = 10.f;
	m_iCoverIndex = 0;
	m_bGoRight = true;

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
bool OGLES3Coverflow::QuitApplication()
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
bool OGLES3Coverflow::InitView()
{
	CPVRTString ErrorStr;

	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));
	
	if (!LoadTextures(&ErrorStr))
	{
        PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}
	
	if (!LoadShaders(&ErrorStr))
	{
        PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	PVRTVECTOR3	vFrom = {0.0f, 0.0f, 15.0f },
		vTo = { 0, 0, 0 },
		vUp = { 0, 1, 0 };
	
	PVRTMatrixPerspectiveFovRH(m_mProjection, g_FOV, f2vt((float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight)), f2vt(g_fCameraNear), f2vt(g_fCameraFar), bRotate);

	PVRTMatrixLookAtRH(m_mView, vFrom, vTo, vUp);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	//this must be called after InitApplication
	CreateCover();

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
bool OGLES3Coverflow::ReleaseView()
{
	m_Print3D.ReleaseTextures();
	return true;
}

bool OGLES3Coverflow::LoadTextures(CPVRTString* const pErrorStr)
{
	for(int i = 0; i < g_i32CoverNo ; ++i)
	{
		
		if(PVRTTextureLoadFromPVR(g_Covers[i].pTextureName, &g_Covers[i].ui32TexID) != PVR_SUCCESS)
		{
            *pErrorStr = "Failed to load '" + CPVRTString(g_Covers[i].pTextureName) + "'.";
			return false;
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
	}

	return true;
}
/*!****************************************************************************
 @Function		LoadShaders
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads and compiles the shaders and links the shader programs
				required for this training course
******************************************************************************/
bool OGLES3Coverflow::LoadShaders(CPVRTString* pErrorStr)
{
	/*
		Load and compile the shaders from files.
		Binary shaders are tried first, source shaders
		are used as fallback.
	*/
	if(PVRTShaderLoadFromFile(
			c_szVertShaderBinFile, c_szVertShaderSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiVertShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	if (PVRTShaderLoadFromFile(
			c_szFragShaderBinFile, c_szFragShaderSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_uiFragShader, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	/*
		Set up and link the shader program
	*/
	const char* aszAttribs[] = { "inVertex", "inNormal", "inColor", "inTexCoord" };

	if(PVRTCreateProgram(
			&m_ShaderProgram.uiId, m_uiVertShader, m_uiFragShader, aszAttribs, 4, pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}
	// Set the sampler2D variable to the first texture unit
	glUniform1i(glGetUniformLocation(m_ShaderProgram.uiId, "sTexture"), 0);

	// Store the location of uniforms for later use
	m_ShaderProgram.uiMVPMatrixLoc	= glGetUniformLocation(m_ShaderProgram.uiId, "MVPMatrix");

	return true;
}
//this function creates the vertex position, colour, normal, and tex coordinate values for one cover.
void OGLES3Coverflow::CreateCover()
{
	int i, row, col;
	float width = 6.0f;
	float height = 6.0f;
	float heightFromMirror = 0.0f;
	float dim = 0.5f; //initialise vertices to normalised size, can then also be used as uv coorda and scales up after
	m_fBorderFraction = 0.00f;
	float dimLess = 0.5f - (0.5f*m_fBorderFraction);//size minus the fraction of the border
	PVRTVec3 normal = PVRTVec3(0.0f, 1.0f, 0.0f); //all the normals are the same

	/* The covers are made up of 16 vertices, 9 quads, 18 triangles. The four colours of the center vertices are fully opaque while all the 
	outside vertices are fully transparent. This produces a thin fade out at the edges which avoids antialiasing.

	0--1------2--3
	|  |      |  |
	4--5------6--7
	|  |      |  |
	|  |      |  |
	|  |      |  |
	8--9-----10--11
	|  |      |  |
	12-13----14--15

	*/

	m_coverPoints[0].p	=	PVRTVec3(-dim, dim, 0);
	m_coverPoints[1].p	=	PVRTVec3(-dimLess, dim, 0); 
	m_coverPoints[2].p	=	PVRTVec3(dimLess, dim, 0);
	m_coverPoints[3].p	=	PVRTVec3(dim, dim, 0);
	m_coverPoints[4].p	=	PVRTVec3(-dim, dimLess, 0);
	m_coverPoints[5].p	=	PVRTVec3(-dimLess, dimLess, 0); 
	m_coverPoints[6].p	=	PVRTVec3(dimLess, dimLess, 0);
	m_coverPoints[7].p	=	PVRTVec3(dim, -dimLess, 0);
	m_coverPoints[8].p	=	PVRTVec3(-dim, -dimLess, 0);
	m_coverPoints[9].p	=	PVRTVec3(-dimLess, -dimLess, 0); 
	m_coverPoints[10].p	=	PVRTVec3(dimLess, -dimLess, 0);
	m_coverPoints[11].p	=	PVRTVec3(dim, -dimLess, 0);
	m_coverPoints[12].p	=	PVRTVec3(-dim, -dim, 0);
	m_coverPoints[13].p	=	PVRTVec3(-dimLess, -dim, 0); 
	m_coverPoints[14].p	=	PVRTVec3(dimLess, -dim, 0);
	m_coverPoints[15].p	=	PVRTVec3(dim, -dim, 0);


	for(i = 0; i < 16; ++i)
	{
		m_coverPoints[i].n = normal;
		m_coverPoints[i].c = PVRTVec4(1.0f, 1.0f, 1.0f, 0.0f);

		//the uvs are matched to the positions (+0.5 for range 0-1)
		m_coverPoints[i].t.x = m_coverPoints[i].p.x + 0.5f;
		m_coverPoints[i].t.y = m_coverPoints[i].p.y + 0.5f;

		//scale up to desired size
		m_coverPoints[i].p.x *= width;
		m_coverPoints[i].p.y *= height;
	}
	
	//only the center 4 vertices are fully opaque all the rest around the edge are tranparent
	m_coverPoints[5].c.w = 1.0f;
	m_coverPoints[6].c.w = 1.0f;
	m_coverPoints[9].c.w = 1.0f;
	m_coverPoints[10].c.w = 1.0f;


	//create indices for the 2 triangles for every square
	m_iNumOpaque = 0;
	m_iNumBlend = 0;

	for(row = 0; row < 3; ++row)
	{
		for(col = 0; col < 3; ++col)
		{
			int start = (row*4)+col;
			//the centre indices are kept in a separate buffer to the border ones as they are going
			//to be drawn in 2 separate passes. 
			if(row==1 && col == 1)
			{
				m_indicesOpaque[m_iNumOpaque++] = (unsigned short) start+1;
				m_indicesOpaque[m_iNumOpaque++] = (unsigned short) start;
				m_indicesOpaque[m_iNumOpaque++] = (unsigned short) start+4;
				m_indicesOpaque[m_iNumOpaque++] = (unsigned short) start+1;
				m_indicesOpaque[m_iNumOpaque++] = (unsigned short) start+4;
				m_indicesOpaque[m_iNumOpaque++] = (unsigned short) start+5;
			}
			else
			{
				m_indicesBlend[m_iNumBlend++] = (unsigned short) start+1;
				m_indicesBlend[m_iNumBlend++] = (unsigned short) start;
				m_indicesBlend[m_iNumBlend++] = (unsigned short) start+4;
				m_indicesBlend[m_iNumBlend++] = (unsigned short) start+1;
				m_indicesBlend[m_iNumBlend++] = (unsigned short) start+4;
				m_indicesBlend[m_iNumBlend++] = (unsigned short) start+5;

			}
		}
	}


	//adjusted the triangle alignment for two of the corners so that the transparency 
	//falls off in the same direction as the other two corners (comment out to see what i mean!)
	//top left
	m_indicesBlend[0] = 1;
	m_indicesBlend[1] = 0;
	m_indicesBlend[2] = 5;
	m_indicesBlend[3] = 0;
	m_indicesBlend[4] = 4;
	m_indicesBlend[5] = 5;
	
	//bottom right
	m_indicesBlend[42] = 11;
	m_indicesBlend[43] = 10;
	m_indicesBlend[44] = 15;
	m_indicesBlend[45] = 10;
	m_indicesBlend[46] = 14;
	m_indicesBlend[47] = 15;
	
	//copy the indices for the mirrored versions. Note, we change the winding order
	int index = 0;

	for(i = 0; i < 48; i += 3)
	{
		if(i<6)
		{
			index = i + 6;
			m_indicesOpaque[index+0] = m_indicesOpaque[i+0] + 16;
			m_indicesOpaque[index+2] = m_indicesOpaque[i+1] + 16;
			m_indicesOpaque[index+1] = m_indicesOpaque[i+2] + 16;
		}

		index = i + 48;
		m_indicesBlend[index+0] = m_indicesBlend[i+0] + 16;
		m_indicesBlend[index+2] = m_indicesBlend[i+1] + 16;
		m_indicesBlend[index+1] = m_indicesBlend[i+2] + 16;
	}

	m_iNumBlend *= 2;
	m_iNumOpaque *= 2;

	//create the vertex points for the mirrored cover.
	for(row = 0; row < 4; ++row)
	{
		//the colours on the flip vertices are going to fade out the further away from the mirror they are
		// using the y coordinate of the coverpoints 
		float dark = 1 - ((m_coverPoints[(row*4)].p.y/height)+ 0.5f);
		dark -= 0.5f; //made a little darker in the reflection to mimic properties of dull mirror

		for(col = 0; col < 4; ++col)
		{
			//copy the vertices with y values reversed for the mirrored equivalents
			m_coverPoints[((row*4)+col) + 16] = m_coverPoints[((row*4)+col)];
			m_coverPoints[((row*4)+col) + 16].p.y *= -1.f;
			m_coverPoints[((row*4)+col) + 16].p.y -= (height + heightFromMirror);

			m_coverPoints[((row*4)+col) + 16].c.x = dark;
			m_coverPoints[((row*4)+col) + 16].c.y = dark;
			m_coverPoints[((row*4)+col) + 16].c.z = dark;
		}
	}

	glGenBuffers(1, &m_uiVbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_uiVbo);
	glBufferData(GL_ARRAY_BUFFER, 32 * sizeof(SVertex), m_coverPoints, GL_STATIC_DRAW);

	glGenBuffers(1, &m_uiBlendIndexVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiBlendIndexVbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_iNumBlend*2, m_indicesBlend, GL_STATIC_DRAW);

	glGenBuffers(1, &m_uiOpaqueIndexVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiOpaqueIndexVbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_iNumOpaque*2, m_indicesOpaque, GL_STATIC_DRAW);
	
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
bool OGLES3Coverflow::RenderScene()
{
	// Clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use shader program
	glUseProgram(m_ShaderProgram.uiId);

	if(PVRShellIsKeyPressed(PVRShellKeyNameRIGHT)) //input permmanently set for demo purposes
		m_bGoRight = true;

	if(PVRShellIsKeyPressed(PVRShellKeyNameLEFT))
		m_bGoRight = false;

	m_fLerpDir = m_bGoRight ? 1.0f : -1.0f;

	unsigned long ulTime = PVRShellGetTime();
	unsigned long ulDeltaTime = ulTime - m_ulTimePrev;
	m_ulTimePrev = ulTime;

	m_fLerp += (ulDeltaTime*.0001f)*m_fCyclesPerSecond*m_fLerpDir;

	if(m_fLerpDir && (m_fLerp >= 1.0 || m_fLerp <= -1.0))
	{

		if(m_fLerpDir < 0)
		{
			m_iCoverIndex++;
			if(m_iCoverIndex > g_i32CoverNo)
				m_iCoverIndex = 1;
		}
		else
		{
			m_iCoverIndex--;
			if(m_iCoverIndex < 0)
				m_iCoverIndex = g_i32CoverNo-1;
		}
		m_fLerpDir = 0.f;
		m_fLerp = 0;
	}

	glEnableVertexAttribArray(VERTEX_ARRAY);
	glEnableVertexAttribArray(NORMAL_ARRAY);
	glEnableVertexAttribArray(COLOR_ARRAY);
	glEnableVertexAttribArray(TEXCOORD_ARRAY);

	//the order in which the covers are drawn is very important for the transparency here. As the covers flip from
	//one position to the next there is a point in the cycle where the center cover moves from being in front of the
	//position following it to behind it. The draw order needs to reflect this so that the blend is still drawn correctly.
	if(m_fLerp < -0.5)
	{
		DrawLeftCovers();
		DrawInPosition(eFront, m_fLerp, m_iCoverIndex);
		DrawRightCovers();
	}
	else if(m_fLerp >  0.5)
	{
		DrawRightCovers();
		DrawInPosition(eFront, m_fLerp, m_iCoverIndex);
		DrawLeftCovers();
	}
	else
	{
		DrawRightCovers();
		DrawLeftCovers();
		DrawInPosition(eFront, m_fLerp, m_iCoverIndex);
	}
	
	// unbind the vertex buffers as we don't need them bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(VERTEX_ARRAY);
	glDisableVertexAttribArray(NORMAL_ARRAY);
	glDisableVertexAttribArray(COLOR_ARRAY);
	glDisableVertexAttribArray(TEXCOORD_ARRAY);

	m_Print3D.DisplayDefaultTitle("Coverflow", "", ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}

void OGLES3Coverflow::DrawLeftCovers()
{
	for(int i = 0; i < eFront; ++i)
	{
		DrawInPosition(i, m_fLerp, m_iCoverIndex);
	}
}
void OGLES3Coverflow::DrawRightCovers()
{
	for(int i = eCoverMeshsNo-1; i > eFront; --i)
	{
		DrawInPosition(i, m_fLerp, m_iCoverIndex);
	}
}

//this function takes the index position of a cover, the linear interpolation too the next position and the index of the 
//texture it is displaying. The matrices and texture load are applied and then draw mesh is called to draw an individual cover.
void OGLES3Coverflow::DrawInPosition(int index, float queueLerp, int coverIndex)
{
	PVRTVec3 pos, posNext;
	float angle = 0.f;
	float backgroundPosition = -8.f;
	float backgroundAngle = (PVRT_PI/2.5);
	float distInQueue = 3.f;

	queueLerp += (float)index;
	coverIndex += index;

	if(coverIndex >= g_i32CoverNo)
	{
		coverIndex = coverIndex - g_i32CoverNo;
	}
	if(coverIndex < 0)
	{
		coverIndex = g_i32CoverNo;
	}

	pos = 0.f;

	pos.x = (queueLerp - eFront) * distInQueue;

	if(queueLerp > eFront - 1 && queueLerp < eFront + 1)
	{
		float lerpAbs = (float) fabs(queueLerp - eFront);
		pos.z = backgroundPosition * lerpAbs;
		angle = backgroundAngle * (queueLerp - eFront);
		pos.x += 2.0f * (queueLerp - eFront);
	}
	else
	{
		queueLerp - eFront < 0 ? angle = -backgroundAngle : angle = backgroundAngle;
		pos.z = backgroundPosition;
		if(queueLerp - eFront > 0)
			pos.x += 2.0;
		else
			pos.x -= 2.0;
	}

	PVRTMat4 mTrans, mRotation;
	PVRTMatrixTranslation(mTrans, pos.x, pos.y, pos.z);

	PVRTMatrixRotationY(mRotation, angle);
	
	PVRTMat4 mModelView, mMVP;
	mModelView = m_mView * mTrans * mRotation;
	mMVP = m_mProjection * mModelView;

	glUniformMatrix4fv(m_ShaderProgram.uiMVPMatrixLoc, 1, GL_FALSE, mMVP.f);
		
	glBindTexture(GL_TEXTURE_2D, g_Covers[coverIndex].ui32TexID);

	DrawMesh();
}
void OGLES3Coverflow::DrawMesh()
{
	int stride = sizeof(SVertex);

	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, m_uiVbo);

	// Set the vertex attribute offsest
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, stride, 0);
	glVertexAttribPointer(NORMAL_ARRAY, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)NormalOffset);
	glVertexAttribPointer(COLOR_ARRAY, 4, GL_FLOAT, GL_FALSE, stride, (GLvoid*)ColorOffset);
	glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)TexCoordOffset);
	
	//firstly draw the opaque quad in the center
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiOpaqueIndexVbo);
	glDrawElements(GL_TRIANGLES, m_iNumOpaque, GL_UNSIGNED_SHORT, 0);
	
	//enable alpha blending just for the borders
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//draw blended borders
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiBlendIndexVbo);
	glDrawElements(GL_TRIANGLES, m_iNumBlend, GL_UNSIGNED_SHORT, 0);

	glDisable(GL_BLEND);
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
	return new OGLES3Coverflow();
}

/******************************************************************************
 End of file (OGLES3Coverflow.cpp)
******************************************************************************/

