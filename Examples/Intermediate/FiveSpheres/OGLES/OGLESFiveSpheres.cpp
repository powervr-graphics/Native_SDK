/******************************************************************************

 @File         OGLESFiveSpheres.cpp

 @Title        OGLESFiveSpheres

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows different primitive types applied to a model. This is more a
               test than a demonstration. Programmers new to OpenGL ES are
               invited to start from a simpler and more featured demo like e.g.
               OGLESVase. The blending modes have been removed to keep the code
               simple and relevant.

******************************************************************************/
#include <math.h>

#include "PVRShell.h"
#include "OGLESTools.h"

/******************************************************************************
 Content file names
******************************************************************************/

// PVR texture files
const char c_szGrassTexFile[] = "Grass.pvr";

/******************************************************************************
 Enums
******************************************************************************/
// Enum to specify the sphere we are drawing
enum ESphereType 
{	
	ePoints, 
	eTriangles, 
	eLineStrip, 
	eFan, 
	eStrip, 
	eTypeCount
};

/******************************************************************************
 Structure definitions
******************************************************************************/

struct SVertex
{
	PVRTVec3	fPos;
	PVRTVec3	fNormal;
	float		fU, fV;
};

/******************************************************************************
 Consts
******************************************************************************/
const int g_i32VertexStride = sizeof(SVertex);
const unsigned char* g_i32PositionOffset = 0;
const unsigned char* g_i32NormalOffset   = (unsigned char*) sizeof(PVRTVec3);
const unsigned char* g_i32UVOffset       = (unsigned char*) sizeof(PVRTVec3);

const unsigned int   g_ui32RimNo = 9;	// Number of rims on the sphere. Including the top and bottom rims made of one vertex. Must be odd.
const unsigned short g_uiRimSize = 16;	// Number of vertices in each rim (except the top and bottom rims). Must be even.
const float g_ui32SphereScale = 1.2f;	// Sphere scale factor.

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLESFiveSpheres : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D 	m_Print3D;

	// Projection and Model View matrices
	PVRTMat4		m_mProjection, m_mView;

	// OpenGL handles for textures and VBOs
	GLuint	m_ui32Texture;

	GLuint*	m_puiVbo;
	GLuint*	m_puiIndexVbo;

	// Rotation variables
	float m_fAngleX, m_fAngleY;

	// Vertices, normals and texture coordinates of the sphere
	SVertex			*m_pVertex;
	unsigned int	m_ui32VertexNo;

	// Vertices, normals and texture coordinates of the sphere
	SVertex			*m_pVertexFan;
	unsigned int	m_i32FanVertexNo;

	// Indices to draw the triangle list
	unsigned short*	m_puiTriListIndices;
	unsigned int	m_ui32TriListIndicesNo;

	// Variables to hold the triangle strips
	unsigned short*	m_puiStrips;
	unsigned int*	m_pui32StripLength;
	unsigned int	m_ui32StripNo;
	unsigned int	m_ui32TotalStripIndicesNo;


public:
	OGLESFiveSpheres() : m_puiVbo(0),
						 m_puiIndexVbo(0),
						 m_fAngleX(0),
						 m_fAngleY(0)
	{
	}

	/* PVRShell functions */
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	/****************************************************************************
	** Function Definitions
	****************************************************************************/
	bool LoadTextures(CPVRTString* const pErrorStr);
	bool CreateGeometry();
	bool CreateIndices();
	unsigned short GetVertexIndex(int i32Rim, int i32Position);
	void SetModelViewMatrix(ESphereType eSphere);
	void LoadVbos();
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
bool OGLESFiveSpheres::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	// Creates all the geometry needed
	if(!CreateGeometry())
		return false;

	if(!CreateIndices())
		return false;

	// Calculates the view matrix
	m_mView = PVRTMat4::Translation(0, 0, -8.0);

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
bool OGLESFiveSpheres::QuitApplication()
{
	// Deletes the geometry
	delete[] m_pVertexFan;
	delete[] m_puiStrips;
	delete[] m_pui32StripLength;
	delete[] m_puiTriListIndices;
	delete[] m_pVertex;

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
bool OGLESFiveSpheres::InitView()
{
	// Is the screen rotated?
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	// Initialize Print3D
	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	// Load the texture
	if(PVRTTextureLoadFromPVR(c_szGrassTexFile, &m_ui32Texture) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot load the texture\n");
		return false;
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Calculates the projection matrix
	PVRTMatrixPerspectiveFovRH(m_mProjection, 0.6f, (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight), 1.0f, 100.0f, bRotate);

	// Set point size
	glPointSize(2.0f);

	// Set front face direction
	glFrontFace(GL_CW);

	// Loads the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(m_mProjection.f);

	// Set the clear color
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	// Set material properties
	float fObjectMatAmb[]	= { 0.1f, 0.1f, 0.1f, 1.0f};
	float fObjectMatDiff[]	= { 0.5f, 0.5f, 0.5f, 1.0f};
	float fObjectMatSpec[]	= { 1.0f, 1.0f, 1.0f, 1.0f};

	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, fObjectMatDiff);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, fObjectMatAmb);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, fObjectMatSpec);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 5);

	// Set lighting properties (light position set in RenderScene())
	float fLightAmb[4]  = { 0.1f, 0.1f, 0.1f, 1.0f };
	float fLightDif[4]  = { 1.0f, 1.0f, 1.0f, 1.0f };
	float fLightSpec[4]	= { 1.0f, 1.0f, 1.0f, 1.0f };
	float fAmbient[4]	= { 1.0f, 1.0f, 1.0f, 1.0f };

	glLightfv(GL_LIGHT0, GL_AMBIENT, fLightAmb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, fLightDif);
	glLightfv(GL_LIGHT0, GL_SPECULAR, fLightSpec);
	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 5.0f);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, fAmbient);

	// Set the light direction
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(m_mView.f);

	float fLightPos[4]  = { 0.0f, 0.0f, 1.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, fLightPos);

	// Create Vertex buffers
	LoadVbos();

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLESFiveSpheres::ReleaseView()
{
	// Release the texture
	glDeleteTextures(1, &m_ui32Texture);

	// Release Print3D textures
	m_Print3D.ReleaseTextures();

	// Destroy vertex buffers
	delete[] m_puiVbo;
	m_puiVbo = 0;

	delete[] m_puiIndexVbo;
	m_puiIndexVbo = 0;
	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLESFiveSpheres::LoadVbos()
{
	unsigned int uiSize;

	if(!m_puiVbo)
		m_puiVbo = new GLuint[2]; // One vertex buffer for the fan vertices and one for the rest

	if(!m_puiIndexVbo)
		m_puiIndexVbo = new GLuint[2]; // One index buffer for strips and one for the triangle lists

	//	Load vertex data into VBOs

	glGenBuffers(2, m_puiVbo);
	glGenBuffers(2, m_puiIndexVbo);

	// Vertex data
	uiSize = m_ui32VertexNo * g_i32VertexStride;

	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[0]);
	glBufferData(GL_ARRAY_BUFFER, uiSize, m_pVertex, GL_STATIC_DRAW);

	// Index data
	uiSize = m_ui32TriListIndicesNo * sizeof(GLshort);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, m_puiTriListIndices, GL_STATIC_DRAW);


	// Triangle fan vertex data
	uiSize = m_i32FanVertexNo * g_i32VertexStride;

	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[1]);
	glBufferData(GL_ARRAY_BUFFER, uiSize, m_pVertexFan, GL_STATIC_DRAW);

	uiSize = m_ui32TotalStripIndicesNo * sizeof(GLshort);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, m_puiStrips, GL_STATIC_DRAW);

	// Unbind the buffers
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
bool OGLESFiveSpheres::RenderScene()
{
	unsigned int i, j;
	int i32Drawn = 0;
	unsigned int ui32Idx = 0;

	// Clears the buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//	Setup the OpenGL ES states needed.

	// Enables texturing
	glEnable(GL_TEXTURE_2D);

	// Setup back-face culling
	glFrontFace(GL_CW);
	glCullFace(GL_FRONT);
	glEnable(GL_CULL_FACE);

	// Use the texture we loaded
	glBindTexture(GL_TEXTURE_2D, m_ui32Texture);

	// Enable lighting - this needs to be re-enabled every frame because Print3D will disable it
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// Gives the vertex, normals and texture coordinates to OpenGL ES
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[0]);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT,   g_i32VertexStride, g_i32PositionOffset);
	glNormalPointer(GL_FLOAT,	   g_i32VertexStride, g_i32NormalOffset);
	glTexCoordPointer(2, GL_FLOAT, g_i32VertexStride, g_i32UVOffset);

	//	Draw the points.
	SetModelViewMatrix(ePoints);
	glDrawArrays(GL_POINTS, 0, (g_ui32RimNo - 2) * g_uiRimSize + 2);

	//	Draw the triangle list.
	SetModelViewMatrix(eTriangles);
	glDrawElements(GL_TRIANGLES, m_ui32TriListIndicesNo, GL_UNSIGNED_SHORT, 0);

	//	Draw the line strip.
	SetModelViewMatrix(eLineStrip);
	glDrawElements(GL_LINE_STRIP, m_ui32TriListIndicesNo, GL_UNSIGNED_SHORT, 0);

	//	Draw the triangle strip.
	SetModelViewMatrix(eStrip);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[1]);

	for(i = 0; i < m_ui32StripNo; ++i)
	{
		glDrawElements(GL_TRIANGLE_STRIP, m_pui32StripLength[i] + 2, GL_UNSIGNED_SHORT, (void*) (i32Drawn * sizeof(GLshort)));
		i32Drawn += m_pui32StripLength[i] + 2;
	}

	//	Draw the triangle eFan.

	// Unbind the indices buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	SetModelViewMatrix(eFan);

	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[1]);

	glVertexPointer(3, GL_FLOAT,   g_i32VertexStride, g_i32PositionOffset);
	glNormalPointer(GL_FLOAT,	   g_i32VertexStride, g_i32NormalOffset);
	glTexCoordPointer(2, GL_FLOAT, g_i32VertexStride, g_i32UVOffset);

	// First draw the two caps
	glDrawArrays(GL_TRIANGLE_FAN, ui32Idx, g_uiRimSize + 2);
	ui32Idx += g_uiRimSize + 2;
 
	glDrawArrays(GL_TRIANGLE_FAN, ui32Idx, g_uiRimSize + 2);
	ui32Idx += g_uiRimSize + 2;

	// Then draw all the other fans organized in an array over the sphere
	for(i = 2; i <= (g_ui32RimNo-3); i += 2)
	{
		for(j = 0; j < g_uiRimSize; j += 2)
		{
			glDrawArrays(GL_TRIANGLE_FAN, ui32Idx, 10);
			ui32Idx += 10;
		}
	}

	// Unbind the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	// Increase rotation angles
	m_fAngleX += 1.0f / 100.0f;
	m_fAngleY += 1.0f / 100.0f;

	if(m_fAngleX > PVRT_TWO_PI)
		m_fAngleX -= PVRT_TWO_PI;

	if(m_fAngleY > PVRT_TWO_PI)
		m_fAngleY -= PVRT_TWO_PI;

	// Disable normals before our call to print3D
	glDisableClientState(GL_NORMAL_ARRAY);

	// Display info text
	m_Print3D.DisplayDefaultTitle("FiveSpheres", "Primitives test", ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		CreateGeometry
 @Description	Creates all the geometry used in this demo.
******************************************************************************/
bool OGLESFiveSpheres::CreateGeometry()
{
	unsigned int i;

	/*
		Creates the sphere vertices and texture coordinates
	*/
	m_ui32VertexNo = (g_ui32RimNo - 2) * g_uiRimSize + 2;
	m_pVertex = new SVertex[m_ui32VertexNo];

	if(!m_pVertex)
	{
		PVRShellSet(prefExitMessage, "Error: Out of memory.");
		return false;
	}

	// Bottom vertex
	m_pVertex[0].fPos.x = 0;
	m_pVertex[0].fPos.y = -0.5f * g_ui32SphereScale;
	m_pVertex[0].fPos.z = 0;
	m_pVertex[0].fU = 0.5f;
	m_pVertex[0].fV = 0.5f;

	// 7 rims of 16 vertices each
	float fYAngleInc = PVRT_PI / (float) (g_ui32RimNo - 1);
	float fYAngle = fYAngleInc;
	int i32Index = 1;

	for(i = 0; i < (g_ui32RimNo - 2); ++i)
	{
		float fPosY = (float) cos(fYAngle) * -0.5f;
		fYAngle += fYAngleInc;

		for(int j = 0; j < g_uiRimSize; ++j)
		{
			float fAngle = (float)j * 2.0f * PVRT_PI / (float)g_uiRimSize;
			float fSize = (float) cos(asin(fPosY*2)) * 0.50f;
			m_pVertex[GetVertexIndex(i+1,j)].fPos.x = (float)PVRTFCOS(fAngle) * (fSize*g_ui32SphereScale);
			m_pVertex[GetVertexIndex(i+1,j)].fPos.y = fPosY * g_ui32SphereScale;
			m_pVertex[GetVertexIndex(i+1,j)].fPos.z = (float)PVRTFSIN(fAngle) * (fSize*g_ui32SphereScale);
			m_pVertex[GetVertexIndex(i+1,j)].fU = m_pVertex[GetVertexIndex(i+1,j)].fPos.x + 0.5f;
			m_pVertex[GetVertexIndex(i+1,j)].fV = m_pVertex[GetVertexIndex(i+1,j)].fPos.z + 0.5f;
			++i32Index;
		}
	}

	// Top vertex
	m_pVertex[i32Index].fPos.x = 0;
	m_pVertex[i32Index].fPos.y = 0.5f * g_ui32SphereScale;
	m_pVertex[i32Index].fPos.z = 0;
	m_pVertex[i32Index].fU = 0.5f;
	m_pVertex[i32Index].fV = 0.5f;

	/*
		Creates the sphere normals.
	*/
	for(i = 0; i < (g_ui32RimNo - 2) * g_uiRimSize + 2; ++i)
	{
		PVRTVec3 vNormal;
		vNormal.x = m_pVertex[i].fPos.x;
		vNormal.y = m_pVertex[i].fPos.y;
		vNormal.z = m_pVertex[i].fPos.z;

		vNormal.normalize();

		m_pVertex[i].fNormal.x = vNormal.x;
		m_pVertex[i].fNormal.y = vNormal.y;
		m_pVertex[i].fNormal.z = vNormal.z;
	}

	return true;
}

/*!****************************************************************************
 @Function		CreateIndices
 @Description	Creates all the geometry used in this demo.
******************************************************************************/
bool OGLESFiveSpheres::CreateIndices()
{
	unsigned int i, j;
	unsigned int *puiStrips = 0;

	//	Creates the indices for the triangle list.
	m_ui32TriListIndicesNo = (g_uiRimSize + (g_ui32RimNo-3)*g_uiRimSize*2 + g_uiRimSize) * 3;

	unsigned int* puiTriListIndices = new unsigned int[m_ui32TriListIndicesNo];

	if(!puiTriListIndices)
	{
		PVRShellSet(prefExitMessage, "Out of memory.");
		return false;
	}

	// From bottom vertex to lowest rim and from top vertex to highest rim
	int iRimTopIndex = (g_uiRimSize + (g_ui32RimNo-3)*g_uiRimSize*2) * 3;

	for(i = 0; i < g_uiRimSize; ++i)
	{
		puiTriListIndices[i*3+2] = GetVertexIndex(0,0);
		puiTriListIndices[i*3+1] = GetVertexIndex(1, (i+1)%g_uiRimSize);
		puiTriListIndices[i*3+0] = GetVertexIndex(1, i);

		puiTriListIndices[iRimTopIndex+i*3+2] = GetVertexIndex((g_ui32RimNo-1), 0);
		puiTriListIndices[iRimTopIndex+i*3+1] = GetVertexIndex((g_ui32RimNo-2), i);
		puiTriListIndices[iRimTopIndex+i*3+0] = GetVertexIndex((g_ui32RimNo-2), (i+1)%g_uiRimSize);
	}

	// From rim to rim
	for(i = 1; i < g_ui32RimNo - 2; ++i)
	{
		for(j = 0; j < g_uiRimSize; ++j)
		{
			puiTriListIndices[g_uiRimSize*3 + (i-1)*g_uiRimSize*2*3 + j*2*3 + 5] = GetVertexIndex(i,j);
			puiTriListIndices[g_uiRimSize*3 + (i-1)*g_uiRimSize*2*3 + j*2*3 + 4] = GetVertexIndex(i,(j+1)%g_uiRimSize);
			puiTriListIndices[g_uiRimSize*3 + (i-1)*g_uiRimSize*2*3 + j*2*3 + 3] = GetVertexIndex(i+1,j);
			puiTriListIndices[g_uiRimSize*3 + (i-1)*g_uiRimSize*2*3 + j*2*3 + 2] = GetVertexIndex(i+1,j);
			puiTriListIndices[g_uiRimSize*3 + (i-1)*g_uiRimSize*2*3 + j*2*3 + 1] = GetVertexIndex(i,(j+1)%g_uiRimSize);
			puiTriListIndices[g_uiRimSize*3 + (i-1)*g_uiRimSize*2*3 + j*2*3 + 0] = GetVertexIndex(i+1,(j+1)%g_uiRimSize);
		}
	}

	//	Generates the triangle strips using our tools.
	PVRTTriStrip(&puiStrips, &m_pui32StripLength, &m_ui32StripNo, puiTriListIndices, g_uiRimSize + (g_ui32RimNo-3)*g_uiRimSize*2 + g_uiRimSize);

	// Convert our indices to unsigned short
	m_puiTriListIndices = new unsigned short[m_ui32TriListIndicesNo];

	for(i = 0; i < m_ui32TriListIndicesNo; ++i)
		m_puiTriListIndices[i] = (unsigned short) puiTriListIndices[i];
	
	delete[] puiTriListIndices;

	// Calculate the total number of strips and convert them to unsigned short
	m_ui32TotalStripIndicesNo = 0;

	for(i = 0; i < m_ui32StripNo; ++i)
		m_ui32TotalStripIndicesNo += m_pui32StripLength[i] + 2;

	m_puiStrips = new unsigned short[m_ui32TotalStripIndicesNo];

	for(i = 0; i < m_ui32TotalStripIndicesNo; ++i)
		m_puiStrips[i] = (unsigned short) puiStrips[i];

	delete[] puiStrips;

	//	Creates the indices for the triangle eFan.

	// Indices to draw the triangle eFan
	m_i32FanVertexNo = 1+(g_uiRimSize+1) + 1+(g_uiRimSize+1) + ((g_ui32RimNo-3)/2)*(g_uiRimSize/2)*10;
	m_pVertexFan = new SVertex[m_i32FanVertexNo];

	if(!m_pVertexFan)
	{
		PVRShellSet(prefExitMessage, "Error: Out of memory.");
		return false;
	}

	// Does the caps at the bottom and top
	m_pVertexFan[0] = m_pVertex[GetVertexIndex(0,0)];
	m_pVertexFan[g_uiRimSize+2] = m_pVertex[GetVertexIndex((g_ui32RimNo-1),0)];

	for(i = 0; i < g_uiRimSize + 1; ++i)
	{
			m_pVertexFan[i+1] = m_pVertex[GetVertexIndex(1,i%g_uiRimSize)];
			m_pVertexFan[(g_uiRimSize+2)+i+1] = m_pVertex[GetVertexIndex((g_ui32RimNo-2),(g_uiRimSize-i)%g_uiRimSize)];
	}

	// Fans the rest of the sphere
	int i32Index = (g_uiRimSize+2) * 2;

	for(i = 2; i <= g_ui32RimNo - 3; i += 2)
	{
		for(j = 1; j < g_uiRimSize; j += 2)
		{
			m_pVertexFan[i32Index++] = m_pVertex[GetVertexIndex(i,	j)];
			m_pVertexFan[i32Index++] = m_pVertex[GetVertexIndex(i,	(j+1)%g_uiRimSize)];
			m_pVertexFan[i32Index++] = m_pVertex[GetVertexIndex(i-1,	(j+1)%g_uiRimSize)];
			m_pVertexFan[i32Index++] = m_pVertex[GetVertexIndex(i-1,	j)];
			m_pVertexFan[i32Index++] = m_pVertex[GetVertexIndex(i-1,	(j-1)%g_uiRimSize)];
			m_pVertexFan[i32Index++] = m_pVertex[GetVertexIndex(i,	(j-1)%g_uiRimSize)];
			m_pVertexFan[i32Index++] = m_pVertex[GetVertexIndex(i+1,	(j-1)%g_uiRimSize)];
			m_pVertexFan[i32Index++] = m_pVertex[GetVertexIndex(i+1,	j)];
			m_pVertexFan[i32Index++] = m_pVertex[GetVertexIndex(i+1,	(j+1)%g_uiRimSize)];
			m_pVertexFan[i32Index++] = m_pVertex[GetVertexIndex(i,	(j+1)%g_uiRimSize)];
		}
	}

	return true;
}

/*!****************************************************************************
 @Function		GetVertexIndex
 @Input			iRim		rim where the vertex is (between 0 and g_ui32RimNo-1)
 @Input			iPosition	position on the rim where the vertex is (between 0 and g_uiRimSize-1)
 @Return		int			Index of the vertex in the list
 @Description	Returns the index of a vertex in the list by its position.
******************************************************************************/
unsigned short OGLESFiveSpheres::GetVertexIndex(int i32Rim, int i32Position)
{
	if(i32Rim == 0)
		return 0;
	
	if(i32Rim == (g_ui32RimNo - 1))
		return 1 + g_uiRimSize * (g_ui32RimNo - 2);

	return (unsigned short) (1 + g_uiRimSize * (i32Rim - 1) + i32Position);
}

/*!****************************************************************************
 @Function		SetModelViewMatrix
 @Input			eSphere		Sphere to set the matrix for
 @Description	Sets the model view matrix for a specific sphere.
******************************************************************************/
void OGLESFiveSpheres::SetModelViewMatrix(ESphereType eSphere)
{
	// Each sphere specific translations
	float aTranslations[] = {
		-0.7f, 0.6f, 0.0f,
		 0.7f, 0.6f, 0.0f,
		-1.5f, -0.6f, 0.0f,
		 0.0f,  -0.6f, 0.0f,
		 1.5f, -0.6f, 0.0f
	};

	// Compose the sphere specific translation with two rotations to get the Model matrix
	PVRTMat4 mModel, mModelView, mRotationX, mRotationY, mTranslation;

	mTranslation = PVRTMat4::Translation(aTranslations[eSphere * 3 + 0], aTranslations[eSphere * 3 + 1], aTranslations[eSphere * 3 + 2]);
	
	mRotationX = PVRTMat4::RotationX(m_fAngleX);
	mRotationY = PVRTMat4::RotationY(m_fAngleY);

	mModel = mRotationX * mRotationY * mTranslation;
	
	// Multiply the Model matrix with the View matrix to get the Model-View matrix
	mModelView = m_mView * mModel;

	// Loads the Model-View matrix into OpenGL ES
	glLoadMatrixf(mModelView.f);
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
	return new OGLESFiveSpheres();
}

/*****************************************************************************
 End of file (OGLESFiveSpheres.cpp)
*****************************************************************************/

