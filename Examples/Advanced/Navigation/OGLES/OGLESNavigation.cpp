/******************************************************************************

 @File         OGLESNavigation.cpp

 @Title        Navigation

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Demonstrates a method of rendering a navigation application using
               OpenGL ES 1.1

******************************************************************************/

#include <stddef.h>

#include "PVRShell.h"
#include "OGLESTools.h"

/******************************************************************************
 Class forward declarations
******************************************************************************/
class CPVRTWorkingSet;

/******************************************************************************
 Defines
******************************************************************************/

// All indices will be made up of 16 bit unsigned integers
typedef PVRTuint16 index_t;

#define MAPFILEIO_SECURITYCHECKPOINT  0xFACEBEED
#define MAPFILEIO_VERSION             1

#ifndef DEG2RAD
#define DEG2RAD(x) (0.0174532925199432f * (x))
#endif

/****************************************************************************
** Enums
****************************************************************************/

enum RenderMethod
{
	RENDER_FLATCOLOURED,
	RENDER_ANTIALIASEDLINES,
	RENDER_TEXT,
	RENDER_DISABLED
};

/****************************************************************************
** Structures
****************************************************************************/

/*!***********************************************************************
 *	@Struct PVRTBoundingBox2D
 *	@Brief  Structure describing a 2D bounding box. Supports all kind of
 *          set operations and provides higher level functionality.
 ************************************************************************/
struct PVRTBoundingBox2D
{
	// Min and max coordinates
	PVRTVec2 minCoords;
	PVRTVec2 maxCoords;
};

/*!***********************************************************************
 *	@Struct PVRTBoundingCircle
 *	@Brief  Structure describing a 2D bounding circle.
 ************************************************************************/
struct PVRTBoundingCircle
{
	PVRTVec2 center;
	float radius;
};

/*!***********************************************************************
 *	@Struct PVRTViewFrustum
 *	@Brief  Structure containing all view frustum corners.
 ************************************************************************/
struct PVRTViewFrustum
{
	// Encoding:
	// n == near, f == far, t == top, b == bottom, l == left, r == right
	PVRTVec3 ntl, ntr, nbl, nbr;
	PVRTVec3 ftl, ftr, fbl, fbr;
};

/*!***********************************************************************
 *	@Struct PVRTVertex
 *	@Brief  Stores two-dimensional position and texture coordinates.
 ************************************************************************/
struct PVRTVertex
{
	PVRTVec2 position;
	PVRTVec2 texcoord;
};

/*!***********************************************************************
 *	@Struct PVRTPivotQuadVertex
 *	@Brief  The PivotQuadVertex structure describes a single vertex within
 *          a screen-space aligned series of quads. It contains the origin
 *          position, the word index determining the position of the letter
 *          within the word, the quad index which determines the position of
 *          the vertex within the quad and texture coordinates.
 ************************************************************************/
struct PVRTPivotQuadVertex
{
	PVRTVec2   origin;
	PVRTint8   word_index;
	PVRTint8   height_index;
	PVRTuint8  u;
	PVRTuint8  v;
};


/*!***********************************************************************
 *	@Struct PVRTVertexDataBucket
 *	@Brief  Contains plain data.
 ************************************************************************/
struct PVRTVertexDataBucket
{
	PVRTBoundingBox2D     boundingbox;
	PVRTuint32            size;
	char                 *pData;
};

/*!***********************************************************************
 *	@Struct PVRTIndexDataBucket
 *	@Brief  A bucket indexset is a regular indexset which is defined within
 *          a coordinate bucket. The bounding box describes the extents of
 *          the contained primitives, which are defined	by the indexset.
 ************************************************************************/
struct PVRTIndexDataBucket
{
	unsigned int         bucketindex;
	PVRTBoundingBox2D    boundingbox;
	PVRTuint32           numIndices;
	index_t             *pIndices;
};

/*!***********************************************************************
 *	@Struct PVRTMapBucket
 *	@Brief  A map bucket is a regular map layer which has been split up into
 *          smaller buckets, which contain the vertex and index data.
 *          The bounding box describes the extents of the whole map layer.
 ************************************************************************/
struct PVRTMapBucket
{
	PVRTBoundingBox2D          boundingbox;
	PVRTuint32                 numVertexDataBuckets;
	PVRTVertexDataBucket      *pVertexDataBuckets;
	PVRTuint32                 numIndexDataBuckets;
	PVRTIndexDataBucket       *pIndexDataBuckets;
};

/*!**************************************************************************
 *	@Struct RenderCache
 *	@Brief  A rendercache object is a reference counted vertex buffer object.
 ****************************************************************************/
struct RenderCache
{
	GLuint       vbo;
	PVRTuint32   size;
	unsigned int references;

	RenderCache()
	{ vbo = 0; size = 0; references = 0; }
};


/*!**************************************************************************
 *	@Struct RenderLayer
 *	@Brief  The renderlayer is used to keep an index of active indexsets during
 *          runtime. Furthermore it contains attributes like colour and the
 *          actual method used to render the data.
 ****************************************************************************/
struct RenderLayer
{
	PVRTMapBucket		 mapbucket;
	RenderCache         *prendercache;
	CPVRTWorkingSet     *pworkingset;
	PVRTVec4             colour;
	PVRTVec2             scale;
	RenderMethod         renderpath;
	const char          *pszName;
};


/*!**************************************************************************
 *	@Struct LayerDescription
 *	@Brief  This structure describes a renderlayer and is just used to
 *          conveniently store each available layer.
 ****************************************************************************/
struct LayerDescription
{
	const char      *pszFilename;
	RenderMethod	renderpath;
	float			colour[4];
	float			scale[2];
};


/****************************************************************************
** Consts
****************************************************************************/

// Serves as a look-ahead value to prevent popping of non-cached geometry
const float     g_fBoundingCircleShift = 0.00128f;
// Global scale factor for the size of the billboard signs and letters
const float     g_fPivotQuadScale      = 0.000256f;
// Orientation of the plane the roads etc. are being drawn onto
const PVRTVec4  g_MapPlane        (0.0f,    0.0f,    1.0f,    0.0f);
// Global colour presets
const PVRTVec4  g_BackgroundColour(0.8509f, 0.8392f, 0.6784f, 0.0f);
const PVRTVec4  g_FloorColour     (0.9411f, 0.7921f, 0.6078f, 1.0f);

// Individual layers of the map
const LayerDescription layerdescriptions[] =
{
	{ "LandUseA_meshes.nav",   RENDER_FLATCOLOURED,     {0.752f,  0.9411f, 0.6f,    1.0f}, {1.0f, 1.0f} },
	{ "LandUseB_meshes.nav",   RENDER_FLATCOLOURED,     {0.0f,    1.0f,    0.0f,    1.0f}, {1.0f, 1.0f} },
	{ "Landmark_meshes.nav",   RENDER_FLATCOLOURED,     {0.627f,  0.627f,  0.627f,  1.0f}, {1.0f, 1.0f} },
	{ "WaterSeg_meshes.nav",   RENDER_FLATCOLOURED,     {0.7215f, 0.8f,    0.8509f, 1.0f}, {1.0f, 1.0f} },
	{ "WaterPoly_meshes.nav",  RENDER_FLATCOLOURED,     {0.7215f, 0.8f,    0.8509f, 1.0f}, {1.0f, 1.0f} },

	// Anti-aliased lines
	{ "RailRds_meshes.nav",    RENDER_ANTIALIASEDLINES, {1.0f,    1.0f,    0.5f,    1.0f}, {1.0f, 1.0f} },
	{ "Streets_meshes.nav",    RENDER_ANTIALIASEDLINES, {0.9790f, 0.9672f, 0.9437f, 1.0f}, {1.0f, 1.0f} },
	{ "SecHwys_meshes.nav",    RENDER_ANTIALIASEDLINES, {0.8509f, 0.6196f, 0.4156f, 1.0f}, {1.0f, 1.0f} },
	{ "MajHwys_meshes.nav",    RENDER_ANTIALIASEDLINES, {0.8509f, 0.4745f, 0.2549f, 1.0f}, {1.0f, 1.0f} },

	{ "Streets_text.nav",      RENDER_TEXT,             {0.0f,    0.0f,    0.0f,    1.0f}, {0.065625f, 0.07f} },
};


/******************************************************************************
 Content file names
******************************************************************************/

// Textures
const char c_aszTextureNameRoad[]        = "Road.pvr";
const char c_aszTextureNameAlphabet[]    = "Alphabet.pvr";

/*!**************************************************************************
 Function declarations
****************************************************************************/

bool ReadPVRTMapBucket(const char *pszFilename, PVRTMapBucket &layer);

/*!****************************************************************************
 Class declarations
******************************************************************************/

/*!***********************************************************************
 *	@Class  OGLESNavigation
 *	@Brief  Navigation demo main class.
 ************************************************************************/
class OGLESNavigation : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// Camera attributes
	CPVRTModelPOD      m_CameraPod;
	float              m_fFOV;
	float              m_fAspectRatio;
	float              m_fNearClipPlane;
	float              m_fFarClipPlane;

	PVRTVec3           m_vCameraOffset;
	PVRTVec3           m_vCameraFrom;
	PVRTVec3           m_vCameraTo;
	PVRTVec3           m_vCameraUp;
	PVRTVec3           m_vCameraDirection;
	PVRTVec3           m_vCameraRight;
	PVRTViewFrustum    m_CameraFrustum;

	PVRTMat4           m_mViewMatrix;
	PVRTMat4           m_mProjectionMatrix;
	PVRTMat4           m_mViewProjectionMatrix;
	PVRTBoundingBox2D  m_BoundingBox;

	// Map layers
	RenderLayer       *m_paLayers;
	unsigned int       m_uiLayerCount;

	// Texture objects
	GLuint m_uiTextureIdRoad;
	GLuint m_uiTextureIdAlphabet;

	// Resource tracking objects
	GLuint m_uiLastBoundVboId;
	GLuint m_uiLastBoundTextureId;

	// Time variables
	unsigned long m_ulPreviousTime, m_ulLastUpdate;
	float m_fCameraAnimation;
	float m_fDebugTimeMultiplier;
	bool m_bPause;
	bool m_bRotate;

public:

	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	bool LoadTextures(CPVRTString* pErrorStr);

	void UpdateTimer();
	void UpdateObjectset();
	void HandleInput();

	void Render(const RenderLayer *pRenderLayer);
	void RenderTriangles(const RenderLayer *pRenderLayer);
	void RenderGround();

	void CacheBucketIndexSet(RenderLayer *pRenderLayer, const unsigned int bucketindex);
	void RemoveBucketIndexSet(RenderLayer *pRenderLayer, const unsigned int bucketindex);
	void BindVBO(GLuint vbo);
	void BindTexture(GLenum unit, GLuint texture);

	void CalculateCameraMatrices();
	void CalculateClipPlanes(float &near, float &far);
	void CalculateViewFrustumCorners(PVRTViewFrustum &corners) const;
	bool CircleIntersectsBoundingBox(const PVRTBoundingBox2D &bbox, const PVRTBoundingCircle &circle);
	float CalculateLinePlaneIntersection(const PVRTVec4 &plane, const PVRTVec3 &a, const PVRTVec3 &b) const;
	void CalculateCameraBoundingCircle(const PVRTVec4 &plane, const float shift, PVRTBoundingCircle &bcircle);

	void ReadData(const char **pSrc, void *pDst, PVRTuint32 size) const;
	bool CheckMarker(const char **pData, unsigned int token) const;
	bool LoadPVRTMapBucket(const char *pszFilename, PVRTMapBucket &layer) const;
};


/*!***********************************************************************
 *	@Class  CPVRTWorkingSet
 *	@Brief  Simple set implementation.
 ************************************************************************/
class CPVRTWorkingSet
{
private:

	unsigned int *m_puiEntries;
	unsigned int  m_uiMaxEntries;
	unsigned int  m_uiNumEntries;

public:

	CPVRTWorkingSet(const CPVRTWorkingSet &ws)
	{
		m_puiEntries = new unsigned int[ws.m_uiMaxEntries];
		memcpy(m_puiEntries, ws.m_puiEntries, sizeof(unsigned int) * ws.m_uiNumEntries);
		m_uiMaxEntries = ws.m_uiMaxEntries;
		m_uiNumEntries = ws.m_uiNumEntries;
	}

	CPVRTWorkingSet(unsigned int maxEntries)
	{
		m_puiEntries = new unsigned int[maxEntries];
		m_uiMaxEntries = maxEntries;
		m_uiNumEntries = 0;
	}

	~CPVRTWorkingSet()
	{
		delete [] m_puiEntries;
	}

	const unsigned int *GetEntries() const
	{
		return  m_puiEntries;
	}

	unsigned int GetEntry(unsigned int index) const
	{
		return m_puiEntries[index];
	}

	bool Contains(unsigned int entry) const
	{
		for (unsigned int i=0; i < m_uiNumEntries; i++)
			if (m_puiEntries[i] == entry)
				return true;
		return false;
	}

	bool Insert(unsigned int entry)
	{
		if (m_uiMaxEntries > m_uiNumEntries)
		{
			m_puiEntries[m_uiNumEntries] = entry;
			m_uiNumEntries++;
			return true;
		}
		return false;
	}

	void Clear()
	{
		m_uiNumEntries = 0;
	}

	unsigned int Size() const
	{
		return m_uiNumEntries;
	}

	unsigned int Capacity() const
	{
		return m_uiMaxEntries;
	}
};



/*!****************************************************************************
 @Function		LoadTextures
 @Output		pErrorStr		A string describing the error on failure
 @Return		bool			true if no error occured
 @Description	Loads the textures required for this training course
******************************************************************************/
bool OGLESNavigation::LoadTextures(CPVRTString* const pErrorStr)
{
	if (PVRTTextureLoadFromPVR(c_aszTextureNameAlphabet, &m_uiTextureIdAlphabet) != PVR_SUCCESS)
	{
		*pErrorStr = CPVRTString("ERROR: Could not open texture file ") + c_aszTextureNameAlphabet;
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	if (PVRTTextureLoadFromPVR(c_aszTextureNameRoad, &m_uiTextureIdRoad) != PVR_SUCCESS)
	{
		*pErrorStr = CPVRTString("ERROR: Could not open texture file ") + c_aszTextureNameRoad;
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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
bool OGLESNavigation::InitApplication()
{
	m_bPause = false;
	m_fFOV = 45.0f;
	m_fNearClipPlane = 0.001f;
	m_fFarClipPlane = 0.01f;
	unsigned int i;

	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	if (PVR_SUCCESS != m_CameraPod.ReadFromFile("cameratrack.pod"))
	{
		PVRShellSet(prefExitMessage, "Error: Failed to parse POD cameratrack.\n");
		return false;
	}

	// Load each layer of the map
	m_uiLayerCount = sizeof(layerdescriptions) / sizeof(layerdescriptions[0]);
	m_paLayers = new RenderLayer[m_uiLayerCount];

	for (i=0; i < m_uiLayerCount; i++)
	{
		m_paLayers[i].prendercache = 0;
		m_paLayers[i].pworkingset = 0;
		m_paLayers[i].renderpath = RENDER_DISABLED;
		m_paLayers[i].colour = layerdescriptions[i].colour;
		m_paLayers[i].scale = layerdescriptions[i].scale;
		m_paLayers[i].pszName = layerdescriptions[i].pszFilename;

		if (!LoadPVRTMapBucket(layerdescriptions[i].pszFilename, m_paLayers[i].mapbucket))
			continue;

		if ((m_paLayers[i].mapbucket.numVertexDataBuckets > 0) &&
			(m_paLayers[i].mapbucket.numIndexDataBuckets > 0))
		{
			m_paLayers[i].prendercache = new RenderCache[m_paLayers[i].mapbucket.numVertexDataBuckets];
			m_paLayers[i].pworkingset = new CPVRTWorkingSet((unsigned int)m_paLayers[i].mapbucket.numIndexDataBuckets);
			m_paLayers[i].renderpath = layerdescriptions[i].renderpath;
		}
	}

	// Determine a global bounding box, encompassing all individual bounding boxes
	m_BoundingBox = m_paLayers[0].mapbucket.boundingbox;
	for (i=1; i < m_uiLayerCount; i++)
	{
		if (m_paLayers[i].renderpath == RENDER_DISABLED)
			continue;
		m_BoundingBox.minCoords.x = PVRT_MIN(m_BoundingBox.minCoords.x, m_paLayers[i].mapbucket.boundingbox.minCoords.x);
		m_BoundingBox.minCoords.y = PVRT_MIN(m_BoundingBox.minCoords.y, m_paLayers[i].mapbucket.boundingbox.minCoords.y);
		m_BoundingBox.maxCoords.x = PVRT_MAX(m_BoundingBox.maxCoords.x, m_paLayers[i].mapbucket.boundingbox.maxCoords.x);
		m_BoundingBox.maxCoords.y = PVRT_MAX(m_BoundingBox.maxCoords.y, m_paLayers[i].mapbucket.boundingbox.maxCoords.y);
	}
	m_vCameraOffset = PVRTVec3(m_BoundingBox.minCoords.x, m_BoundingBox.minCoords.y, 0.0f);

	// Set timer variables
	m_ulPreviousTime = PVRShellGetTime();
	m_ulLastUpdate = 0;
	m_fCameraAnimation = 0.0f;
	m_fDebugTimeMultiplier = 1.0f;

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
bool OGLESNavigation::QuitApplication()
{
	unsigned int j;

	for (unsigned int i=0; i < m_uiLayerCount; i++)
	{
		delete [] m_paLayers[i].prendercache;
		delete m_paLayers[i].pworkingset;

		PVRTMapBucket &mapbucket = m_paLayers[i].mapbucket;
		for (j=0; j < mapbucket.numIndexDataBuckets; j++)
			delete [] mapbucket.pIndexDataBuckets[j].pIndices;
		delete [] mapbucket.pIndexDataBuckets;
		for (j=0; j < mapbucket.numVertexDataBuckets; j++)
			delete [] mapbucket.pVertexDataBuckets[j].pData;
		delete [] mapbucket.pVertexDataBuckets;
	}
	delete [] m_paLayers;

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
bool OGLESNavigation::InitView()
{
	m_fAspectRatio = PVRShellGet(prefWidth) / (float)PVRShellGet(prefHeight);

	CPVRTString ErrorStr;

	// Load textures
	if (!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Is the screen rotated?
	m_bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);
#ifdef _WIN32
	if (PVRShellGet(prefWidth) < PVRShellGet(prefHeight))
		m_bRotate = true;
#endif

	// Initialize Print3D
	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), m_bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	// Enable culling and depth test
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	// No depth testing when rendering a plain 2D map
	glDisable(GL_DEPTH_TEST);
	// No stencil test required by default
	glDisable(GL_STENCIL_TEST);
	// Disable blending by default for now, but set proper blending mode
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Use the ground colour for clearing
	glClearColor(g_BackgroundColour.x, g_BackgroundColour.y, g_BackgroundColour.z, 1.0f);

	return true;
}


/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLESNavigation::ReleaseView()
{
	// Delete textures
	glDeleteTextures(1, &m_uiTextureIdAlphabet);
	glDeleteTextures(1, &m_uiTextureIdRoad);

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
				The user has access to these events through an abstraction
				layer provided by PVRShell.
******************************************************************************/
bool OGLESNavigation::RenderScene()
{
	// Handle user input and update the timer based variables
	HandleInput();
	UpdateTimer();

	// Update the camera interpolation and extract required matrices
	CalculateCameraMatrices();

	// Initialize id tracking variables
	m_uiLastBoundVboId = m_uiLastBoundTextureId = 0;

	// Setup the viewport for the whole window
	glViewport(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));
	// Clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Disable culling and depth testing
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(m_mProjectionMatrix.f);

	// Render ground
	RenderGround();

	// Render each layer
	for (unsigned int i=0; i < m_uiLayerCount; i++)
	{
		Render(&m_paLayers[i]);
	}

	// Displays the demo name and other information using the Print3D tool.
	// For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("Navigation", NULL, ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		UpdateTimer
 @Input			none
 @Description	Updates the values of the current time, previous time, current time
				in seconds, delta time and the FPS counter used in the program
******************************************************************************/
void OGLESNavigation::UpdateTimer()
{
	unsigned long ulCurrentTime = PVRShellGetTime();
	unsigned long ulTimeDelta = ulCurrentTime - m_ulPreviousTime;
	m_ulPreviousTime = ulCurrentTime;

	// Update the visible object four times per second
	if (ulCurrentTime - m_ulLastUpdate > 250)
	{
		m_ulLastUpdate = ulCurrentTime;
		UpdateObjectset();
	}

	// Advance camera animation when not paused
	if (!m_bPause)
		m_fCameraAnimation += ulTimeDelta * 0.00005f * m_fDebugTimeMultiplier;
}

/*!****************************************************************************
 @Function		UpdateObjectset
 @Input			none
 @Description	Updates the visible object set based on the camera position and
                the intersections of the view frustum with the global map plane.
******************************************************************************/
void OGLESNavigation::UpdateObjectset()
{
	unsigned int j;

	// Calculate the bounding circle based on the camera frustum intersection with the global map plane.
	// Shift the resulting bounding circle towards the viewing direction to cache objects ahead.
	PVRTBoundingCircle circle;
	CalculateCameraBoundingCircle(g_MapPlane, g_fBoundingCircleShift, circle);

	// Update the object set for each layer
	for (unsigned int i=0; i < m_uiLayerCount; i++)
	{
		// If we do not render it, skip it
		if (m_paLayers[i].renderpath == RENDER_DISABLED)
			continue;

		RenderLayer *pLayer = &m_paLayers[i];
		CPVRTWorkingSet workingset_old(*(pLayer->pworkingset));
		pLayer->pworkingset->Clear();

		// If the bounding circle does not intersect the bucket layer, skip it
		if (CircleIntersectsBoundingBox(pLayer->mapbucket.boundingbox, circle))
		{
			// Check against each individual bucket-indexset
			for (j=0; j < pLayer->mapbucket.numIndexDataBuckets; j++)
			{
				// Skip it if it is empty
				if (pLayer->mapbucket.pIndexDataBuckets[j].numIndices == 0)
					continue;
				if (CircleIntersectsBoundingBox(pLayer->mapbucket.pIndexDataBuckets[j].boundingbox, circle))
					pLayer->pworkingset->Insert(j);
			}
		}

		// Check all the old indices agains the new ones and if one entry is not
		// in the new one, drop the cache
		for (j=0; j < workingset_old.Size(); j++)
			if (!pLayer->pworkingset->Contains(workingset_old.GetEntry(j)))
				RemoveBucketIndexSet(pLayer, workingset_old.GetEntry(j));

		// Check all the new indices agains the old ones and if one entry is not
		// in the old one, cache it
		for (j=0; j < pLayer->pworkingset->Size(); j++)
			if (!workingset_old.Contains(pLayer->pworkingset->GetEntry(j)))
				CacheBucketIndexSet(pLayer, pLayer->pworkingset->GetEntry(j));
	}
}

/*!****************************************************************************
 @Function		HandleInput
 @Description	Handles user input.
******************************************************************************/
void OGLESNavigation::HandleInput()
{
	// Handle user input
	if (PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
		m_bPause = !m_bPause;
#ifdef _DEBUG
	if (PVRShellIsKeyPressed(PVRShellKeyNameRIGHT))
		m_fDebugTimeMultiplier *= 2.0f;
	if (PVRShellIsKeyPressed(PVRShellKeyNameLEFT))
		m_fDebugTimeMultiplier *= 0.5f;
#endif
}

/*!****************************************************************************
 @Function		Render
 @Input         pRenderLayer     The navigation map layer to render.
 @Description	Renders
******************************************************************************/
void OGLESNavigation::Render(const RenderLayer *pRenderLayer)
{
	// Return immediately if there is nothing to render for this layer or
	// the rendering has been disabled
	if (pRenderLayer->renderpath == RENDER_DISABLED)
		return;

	// Determine the renderpath
	switch (pRenderLayer->renderpath)
	{
	case RENDER_TEXT:
		{
			glEnable(GL_BLEND);
			BindTexture(GL_TEXTURE0, m_uiTextureIdAlphabet);
			RenderTriangles(pRenderLayer);
			break;
		}

	case RENDER_ANTIALIASEDLINES:
		{
			glEnable(GL_BLEND);
			BindTexture(GL_TEXTURE0, m_uiTextureIdRoad);
			RenderTriangles(pRenderLayer);
			break;
		}

	case RENDER_FLATCOLOURED:
		{
			glDisable(GL_BLEND);
			RenderTriangles(pRenderLayer);
			break;
		}

	case RENDER_DISABLED:
	default:
		{
			break;
		}
	}
}


/*!****************************************************************************
 @Function		RenderTriangles
 @Input         pRenderLayer     The navigation map layer to render.
 @Description	Draws a rectangle the size of the maps bounding boxes.
******************************************************************************/
void OGLESNavigation::RenderTriangles(const RenderLayer *pRenderLayer)
{
	if (pRenderLayer->pworkingset->Size() == 0)
		return;

	PVRTVec4 colour = pRenderLayer->colour;
	glColor4f(colour.x, colour.y, colour.z, colour.w);

	// Enable the vertex attribute arrays
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// Iterate over the active object set
	for (unsigned int i=0; i < pRenderLayer->pworkingset->Size(); i++)
	{
		// Get the index for the bucket
		const unsigned int index = pRenderLayer->pworkingset->GetEntry(i);
		// Get the bucket-indexset
		const PVRTIndexDataBucket &indexdatabucket = pRenderLayer->mapbucket.pIndexDataBuckets[index];

		// Go to the next workingset item if no primitives to draw
		if (indexdatabucket.numIndices == 0)
		{
			PVRShellOutputDebug("Warning: Tried to draw empty indexlist!");
			continue;
		}

		// Move camera according to the min coords of the current bounding box as the data is stored relative to it
		PVRTVec2 mincoords = pRenderLayer->mapbucket.pVertexDataBuckets[indexdatabucket.bucketindex].boundingbox.minCoords;
		PVRTVec3 offset = PVRTVec3(mincoords.x, mincoords.y, 0.0f);
		PVRTMat4 viewmatrix = PVRTMat4::LookAtRH(m_vCameraFrom - offset, m_vCameraTo - offset, m_vCameraUp);
		PVRTMat4 mvp = m_mProjectionMatrix * viewmatrix;

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(viewmatrix.f);

		// Get the VBO id from the rendercache
		const RenderCache &cache = pRenderLayer->prendercache[indexdatabucket.bucketindex];
		if (cache.references == 0)
		{
			PVRShellOutputDebug("Error: Tried to draw non-cached bucket!");
			continue;
		}

		BindVBO(cache.vbo);

		glVertexPointer(2, GL_FLOAT, sizeof(PVRTVertex), NULL);
		glTexCoordPointer(2, GL_FLOAT, sizeof(PVRTVertex), (const void*)offsetof(PVRTVertex, texcoord));

		// Draw the triangles
		glDrawElements(GL_TRIANGLES, (GLsizei)indexdatabucket.numIndices, GL_UNSIGNED_SHORT, indexdatabucket.pIndices);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


/*!****************************************************************************
 @Function		RenderGround
 @Description	Draws a rectangle the size of the maps bounding boxes.
******************************************************************************/
void OGLESNavigation::RenderGround()
{
	PVRTVec4 col = g_FloorColour;
	glColor4f(col.x, col.y, col.z, col.w);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(m_mViewMatrix.f);

	glEnableClientState(GL_VERTEX_ARRAY);

	// Enable vertex arributes
	PVRTVec2 minCorner = m_BoundingBox.minCoords;
	PVRTVec2 maxCorner = m_BoundingBox.maxCoords;

	const float afVertexData[] = { minCorner.x, minCorner.y, 0.0f, maxCorner.x, minCorner.y, 0.0f,
		                           minCorner.x, maxCorner.y, 0.0f, maxCorner.x, maxCorner.y, 0.0f };

	glVertexPointer(3, GL_FLOAT, 0, afVertexData);

	// Draw the quad
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// Safely disable the vertex attribute arrays
	glDisableClientState(GL_VERTEX_ARRAY);
}


/*!****************************************************************************
 @Function		CacheBucketIndexSet
 @Description	Creates a VBO of a bucket of coordinates and adds it to the
                cache queue of the map tile or increases reference count if
				already cached.
******************************************************************************/
void OGLESNavigation::CacheBucketIndexSet(RenderLayer *pRenderLayer, const unsigned int bucketindex)
{
	// Retrieve referenced rendercache entity
	const unsigned int index = pRenderLayer->mapbucket.pIndexDataBuckets[bucketindex].bucketindex;
	RenderCache &rendercache = pRenderLayer->prendercache[index];

	// If it is already cached do nothing
	if (rendercache.references > 0)
	{
		rendercache.references++;
		return;
	}

	// Not cached, so create VBO now
	PVRTVertexDataBucket &vertexdatabucket = pRenderLayer->mapbucket.pVertexDataBuckets[index];
	rendercache.size = vertexdatabucket.size;
	rendercache.references++;

	glGenBuffers(1, &(rendercache.vbo));
	glBindBuffer(GL_ARRAY_BUFFER, rendercache.vbo);
	glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)rendercache.size, vertexdatabucket.pData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/*!****************************************************************************
 @Function		RemoveBucketIndexSet
 @Description	Decreases reference count of VBO and removes VBO if not
                required any longer.
******************************************************************************/
void OGLESNavigation::RemoveBucketIndexSet(RenderLayer *pRenderLayer, const unsigned int bucketindex)
{
	const unsigned int index = pRenderLayer->mapbucket.pIndexDataBuckets[bucketindex].bucketindex;
	RenderCache &rendercache = pRenderLayer->prendercache[index];
	if (rendercache.references == 0)
	{
		PVRShellOutputDebug("Error: Tried to remove non-cached element!");
		return;
	}

	rendercache.references--;

	// Remove if reference count reaches zero
	if (rendercache.references == 0)
	{
		glDeleteBuffers(1, &(rendercache.vbo));
		rendercache.size = 0;
	}
}

/*!****************************************************************************
 @Function		BindVBO
 @Description	Checks whether the given VBO is already bound and binds it if
                necessary.
******************************************************************************/
void OGLESNavigation::BindVBO(GLuint vbo)
{
	// Only set the VBO if we bind a different one (optimize -> sort for vbos)
	if (vbo != m_uiLastBoundVboId)
	{
		// Bind the current bucket
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		m_uiLastBoundVboId = vbo;
	}
}

/*!****************************************************************************
 @Function		BindTexture
 @Description	Checks whether the given texture is already bound and binds it if
                necessary. Currently only supports one global texture,
				regardless to which texture unit it is currently bound.
******************************************************************************/
void OGLESNavigation::BindTexture(GLenum unit, GLuint id)
{
	glActiveTexture(unit);
	if (m_uiLastBoundTextureId != id)
	{
		glBindTexture(GL_TEXTURE_2D, id);
		m_uiLastBoundTextureId = id;
	}
}

/*!****************************************************************************
 @Function		CalculateCameraMatrices
 @Description	Calculates all required camera matrices.
******************************************************************************/
void OGLESNavigation::CalculateCameraMatrices()
{
	// Animate the camera
	if (m_fCameraAnimation > (m_CameraPod.nNumFrame - 2))
		m_fCameraAnimation = 0.0f;
	m_CameraPod.SetFrame(m_fCameraAnimation);

	static PVRTMATRIX rotMatrix;
	m_CameraPod.GetRotationMatrix(rotMatrix, m_CameraPod.pNode[0]);

	int baseFrame = (int)m_fCameraAnimation;
	PVRTVec3 pos0(&m_CameraPod.pNode[0].pfAnimPosition[baseFrame * 3]);
	PVRTVec3 pos1(&m_CameraPod.pNode[0].pfAnimPosition[(baseFrame + 1) * 3]);

	// Offset the camera by a reference coordinate to center it around the origin
	// This should help circumvent floating point precision problems
	pos0 -= m_vCameraOffset;
	pos1 -= m_vCameraOffset;
	float lerp = (m_fCameraAnimation - (float)(baseFrame));
	m_vCameraFrom = pos0 + (pos1 - pos0) * lerp;
	m_vCameraFrom += m_vCameraOffset;

	// Extract camera tripod orientation from rotation matrix
	m_vCameraDirection = PVRTVec3(-rotMatrix[1][0], -rotMatrix[1][1], -rotMatrix[1][2]);
	m_vCameraUp = PVRTVec3(-rotMatrix[2][0], -rotMatrix[2][1], -rotMatrix[2][2]);
	m_vCameraTo = m_vCameraFrom + m_vCameraDirection;
	m_vCameraRight = m_vCameraUp.cross(m_vCameraDirection);

	// Adjust the near and far clip plane for a tight view frustum
	CalculateViewFrustumCorners(m_CameraFrustum);
	//CalculateClipPlanes(m_fNearClipPlane, m_fFarClipPlane);

	m_mViewMatrix = PVRTMat4::LookAtRH(m_vCameraFrom, m_vCameraTo, m_vCameraUp);
	m_mProjectionMatrix = PVRTMat4::PerspectiveFovRH(DEG2RAD(m_fFOV), m_fAspectRatio, m_fNearClipPlane, m_fFarClipPlane, PVRTMat4::OGL, m_bRotate);
	m_mViewProjectionMatrix = m_mProjectionMatrix * m_mViewMatrix;
}


/*!****************************************************************************
 @Function		CalculateClipPlanes
 @Output        nearplane       The calculated near plane.
 @Output        farplane        The calculated far plane.
 @Description	Calculates optimal clipping planes based on the current camera
                setup and the viewable area.
******************************************************************************/
void OGLESNavigation::CalculateClipPlanes(float &nearplane, float &farplane)
{
	// Adjust the clip planes so that the near plane is pushed away as far as possible
	// before intersecting the ground
	PVRTVec3 lowdir = ((m_CameraFrustum.fbl - m_vCameraFrom) + (m_CameraFrustum.fbr - m_vCameraFrom)).normalized();
	PVRTVec3 updir = ((m_CameraFrustum.ftl - m_vCameraFrom) + (m_CameraFrustum.ftr - m_vCameraFrom)).normalized();

	// Calculate the intersections with the global plane
	float u = CalculateLinePlaneIntersection(g_MapPlane, m_vCameraFrom, lowdir + m_vCameraFrom);
	float v = CalculateLinePlaneIntersection(g_MapPlane, m_vCameraFrom, updir + m_vCameraFrom);

	// Project the intersection onto the view direction
	nearplane = (lowdir * u).dot(m_vCameraDirection);
	farplane = (updir * v).dot(m_vCameraDirection);
}


/*!********************************************************************************************
 @Function		CalculateViewFrustumCorners
 @Output        corners        The eight vertices defining the viewfrustum shape.
 @Description	Calculates the eight viewfrustum vertices based on the current camera setup.
***********************************************************************************************/
void OGLESNavigation::CalculateViewFrustumCorners(PVRTViewFrustum &corners) const
{
	const PVRTVec3 nearcenter = m_vCameraFrom + m_vCameraDirection * m_fNearClipPlane;
	const PVRTVec3 farcenter = m_vCameraFrom + m_vCameraDirection * m_fFarClipPlane;

	const float tang = (float)tan(DEG2RAD(m_fFOV) * 0.5f);
	const float nearheight = m_fNearClipPlane * tang;
	const float nearwidth = nearheight * m_fAspectRatio;
	const float farheight = m_fFarClipPlane  * tang;
	const float farwidth = farheight * m_fAspectRatio;

	// Near plane frustum corners
	corners.ntl = nearcenter + m_vCameraUp * nearheight - m_vCameraRight * nearwidth;
	corners.ntr = nearcenter + m_vCameraUp * nearheight + m_vCameraRight * nearwidth;
	corners.nbl = nearcenter - m_vCameraUp * nearheight - m_vCameraRight * nearwidth;
	corners.nbr = nearcenter - m_vCameraUp * nearheight + m_vCameraRight * nearwidth;

	// Far plane frustum corners
	corners.ftl = farcenter + m_vCameraUp * farheight - m_vCameraRight * farwidth;
	corners.ftr = farcenter + m_vCameraUp * farheight + m_vCameraRight * farwidth;
	corners.fbl = farcenter - m_vCameraUp * farheight - m_vCameraRight * farwidth;
	corners.fbr = farcenter - m_vCameraUp * farheight + m_vCameraRight * farwidth;
}

/*!********************************************************************************************
 @Function		CalculateCameraBoundingCircle
 @Input         plane                The plane which will be intersected with the viewfrustum.
 @Input			shift                The resulting bounding circle will be shifted along the
				                     view direction by this amount.
 @Return        PVRTBoundingCircle   The bounding circle enclosing the intersection primitive.
 @Description	Calculates the intersection points of a viewfrustum and a plane. Only the viewfrustum
                side vectors (pointing towards the viewing direction) will be taken into account.
				The resulting intersection points are then used to calculate a bounding circle,
				which will be shifted along the viewing direction.
***********************************************************************************************/
void OGLESNavigation::CalculateCameraBoundingCircle(const PVRTVec4 &plane, const float shift, PVRTBoundingCircle &circle)
{
	PVRTVec2 a(m_vCameraFrom + (m_CameraFrustum.fbl - m_vCameraFrom) * CalculateLinePlaneIntersection(plane, m_vCameraFrom, m_CameraFrustum.fbl));
	PVRTVec2 b(m_vCameraFrom + (m_CameraFrustum.fbr - m_vCameraFrom) * CalculateLinePlaneIntersection(plane, m_vCameraFrom, m_CameraFrustum.fbr));
	PVRTVec2 c(m_vCameraFrom + (m_CameraFrustum.ftl - m_vCameraFrom) * CalculateLinePlaneIntersection(plane, m_vCameraFrom, m_CameraFrustum.ftl));
	PVRTVec2 d(m_vCameraFrom + (m_CameraFrustum.ftr - m_vCameraFrom) * CalculateLinePlaneIntersection(plane, m_vCameraFrom, m_CameraFrustum.ftr));

	circle.center = (a + b + c + d) * 0.25f;
	circle.radius = (a - circle.center).length();

	// now add a little shift towards view direction
	circle.center += PVRTVec2(m_vCameraDirection) * shift;
}

/*!********************************************************************************************
 @Function		CircleIntersectsBoundingBox
 @Input			bbox     The bounding box.
 @Input         circle   The bounding circle.
 @Return        bool     Indicates whether the primitives intersect or not.
 @Description	Determines whether a bounding circle and a bounding box intersect or not.
***********************************************************************************************/
bool OGLESNavigation::CircleIntersectsBoundingBox(const PVRTBoundingBox2D &bbox, const PVRTBoundingCircle &circle)
{
	// Extremely simple, just add border the same width as radius. Gives false positives at edge cases!
	PVRTBoundingBox2D extbbox;
	extbbox.minCoords = bbox.minCoords - PVRTVec2(circle.radius);
	extbbox.maxCoords = bbox.maxCoords + PVRTVec2(circle.radius);

	if ((circle.center.x >= extbbox.minCoords.x) && (circle.center.y >= extbbox.minCoords.y) &&
		(circle.center.x <= extbbox.maxCoords.x) && (circle.center.y <= extbbox.maxCoords.y))
		return true;
	else
		return false;
}


/*!****************************************************************************
 @Function		CalculateLinePlaneIntersection
 @Input         plane    Plane definition in normal plus distance format.
 @Input         a        First point of line.
 @Input         b        Second point of line.
 @Return		Intersection distance.
 @Description	Calculates the intersection of a line and a plane.
******************************************************************************/
float OGLESNavigation::CalculateLinePlaneIntersection(const PVRTVec4 &plane, const PVRTVec3 &a, const PVRTVec3 &b) const
{
	PVRTVec3 normal(plane.x, plane.y, plane.z);
	float nom = -(normal.dot(a) + plane.w);
	float denom = normal.dot(b - a);

	if (denom == 0.0f) return 0.0f;
	else return nom / denom;
}

/*!****************************************************************************
 @Function		ReadData
 @Input         pSrc     Pointer to pointer to read data from.
 @Input         size     Amount of data in bytes to copy.
 @Output        pDst     Destination buffer.
 @Description	Copies a defined amount of data from a source pointer and sets
                it after the last read byte.
******************************************************************************/
void OGLESNavigation::ReadData(const char **pSrc, void *pDst, PVRTuint32 size) const
{
	memcpy(pDst, *pSrc, size);
	*pSrc += size;
}

/*!****************************************************************************
 @Function		CheckMarker
 @Input         pData    Pointer to data to read the data from.
 @Input         token    Value of token to check.
 @Return		True if the marker is correct, false otherwise.
 @Description	Reads a 32-bit token from the data stream and compares it
                against the provided one.
******************************************************************************/
bool OGLESNavigation::CheckMarker(const char **pData, unsigned int token) const
{
	unsigned int value = 0;
	ReadData(pData, &value, sizeof(value));
	return (value == token);
}

/*!********************************************************************************************
 @Function		readPVRTMapBucket
 @Input			pszFilename      Filename to read the map layer from.
 @Input			layer            The layer to write.
 @Return        bool             True if successful, false otherwise.
 @Description	Reads a map bucket from a file.
***********************************************************************************************/
bool OGLESNavigation::LoadPVRTMapBucket(const char *pszFilename, PVRTMapBucket &layer) const
{
	CPVRTResourceFile file(pszFilename);
	if (!file.IsOpen())
		return false;

	const char *pData = (const char *)file.DataPtr();

	if (!CheckMarker(&pData, MAPFILEIO_VERSION))	return false;
	ReadData(&pData, &layer.boundingbox, sizeof(layer.boundingbox));

	if (!CheckMarker(&pData, MAPFILEIO_SECURITYCHECKPOINT))	return false;

	// Read vertex data
	ReadData(&pData, &layer.numVertexDataBuckets, sizeof(layer.numVertexDataBuckets));
	if (layer.numVertexDataBuckets > 0)
	{
		layer.pVertexDataBuckets = new PVRTVertexDataBucket[layer.numVertexDataBuckets];
		for (unsigned int i=0; i < layer.numVertexDataBuckets; i++)
		{
			ReadData(&pData, &layer.pVertexDataBuckets[i].boundingbox, sizeof(layer.pVertexDataBuckets[i].boundingbox));
			if (!CheckMarker(&pData, MAPFILEIO_SECURITYCHECKPOINT))	return false;

			ReadData(&pData, &layer.pVertexDataBuckets[i].size, sizeof(layer.pVertexDataBuckets[i].size));
			if (layer.pVertexDataBuckets[i].size == 0) return false;
			layer.pVertexDataBuckets[i].pData = new char[layer.pVertexDataBuckets[i].size];
			ReadData(&pData, layer.pVertexDataBuckets[i].pData, layer.pVertexDataBuckets[i].size);
			if (!CheckMarker(&pData, MAPFILEIO_SECURITYCHECKPOINT))	return false;
		}
	}
	else
		layer.pVertexDataBuckets = NULL;

	if (!CheckMarker(&pData, MAPFILEIO_SECURITYCHECKPOINT))	return false;

	ReadData(&pData, &layer.numIndexDataBuckets, sizeof(layer.numIndexDataBuckets));
	if (layer.numIndexDataBuckets > 0)
	{
		layer.pIndexDataBuckets = new PVRTIndexDataBucket[layer.numIndexDataBuckets];
		for (unsigned int i=0; i < layer.numIndexDataBuckets; i++)
		{
			ReadData(&pData, &layer.pIndexDataBuckets[i].bucketindex, sizeof(layer.pIndexDataBuckets[i].bucketindex));
			if (!CheckMarker(&pData, MAPFILEIO_SECURITYCHECKPOINT))	return false;

			ReadData(&pData, &layer.pIndexDataBuckets[i].boundingbox, sizeof(layer.pIndexDataBuckets[i].boundingbox));
			if (!CheckMarker(&pData, MAPFILEIO_SECURITYCHECKPOINT))	return false;

			PVRTuint32 size;
			ReadData(&pData, &size, sizeof(PVRTuint32));
			layer.pIndexDataBuckets[i].numIndices = size / sizeof(index_t);
			if (layer.pIndexDataBuckets[i].numIndices == 0) return false;
			layer.pIndexDataBuckets[i].pIndices = new index_t[layer.pIndexDataBuckets[i].numIndices];
			ReadData(&pData, layer.pIndexDataBuckets[i].pIndices, size);
			if (!CheckMarker(&pData, MAPFILEIO_SECURITYCHECKPOINT))	return false;
		}
	}
	else
		layer.pIndexDataBuckets = NULL;

	if (!CheckMarker(&pData, MAPFILEIO_SECURITYCHECKPOINT))	return false;

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
	return new OGLESNavigation();
}

/******************************************************************************
 End of file (OGLESNavigation.cpp)
******************************************************************************/

