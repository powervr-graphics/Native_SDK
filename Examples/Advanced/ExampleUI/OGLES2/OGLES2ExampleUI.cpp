/******************************************************************************

 @File         OGLES2ExampleUI.cpp

 @Title        ExampleUI

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Demonstrates how to efficiently render UI and sprites using OpenGL
               ES 2.0.

******************************************************************************/

#include "PVRShell.h"
#include "OGLES2Tools.h"

/******************************************************************************
** Defines
******************************************************************************/

// Index the attributes that are bound to vertex shaders
#define VERTEX_ARRAY		0
#define TEXCOORD_ARRAY		1
#define TRANSFORM_ARRAY		2
#define RGBA_ARRAY			3

#define MAX_SPRITES			256
#define INDICES_PER_TRI		3
#define INDICES_PER_QUAD	(INDICES_PER_TRI*2)

#define ATLAS_WIDTH			1024
#define ATLAS_HEIGHT		1024

#define NULL_QUAD_PIX		4.0f
#define NULL_QUAD_SAMPLE	2.0f

#define VIRTUAL_WIDTH		640.0f
#define VIRTUAL_HEIGHT		480.0f

#define CON_W				512
#define CON_H				400
#define WIN_W				400
#define WIN_H				300

#define GEOM_CIRCLE_STEPS	8
#define ATLAS_PIXEL_BORDER	1

#define UI_DISPLAY_TIME		5				// Display each page for 5 seconds

//#define DISPLAY_SPRITE_ALPHA				// Display alpha as RED for each sprite

/******************************************************************************
** Macros
******************************************************************************/
#define ELEMENTS_IN_ARRAY(x)		(sizeof(x) / sizeof(x[0]))

#ifdef _DEBUG
	// Use a macro to redefine glDrawElements and glDrawArrays so we can count how many draw calls there were this frame.
	#define glDrawElements(w,x,y,z)		{ m_iDrawCallsPerFrame++; glDrawElements(w,x,y,z); }
	#define glDrawArrays(x,y,z)			{ m_iDrawCallsPerFrame++; glDrawArrays(x,y,z); }
#endif

/****************************************************************************
** Data Enums
****************************************************************************/
// Shaders
enum eShaderNames
{
	eSPRITE_SHADER,
	eTEXCOL_SHADER,
	eCOL_SHADER,

	eSHADER_SIZE
};

// Sprites that will be added to a generated texture atlas
enum eSprites
{
	eSPRITE_CLOCKFACE,
	eSPRITE_HAND,
	eSPRITE_BATTERY,
	eSPRITE_WEB,
	eSPRITE_NEWMAIL,
	eSPRITE_NETWORK,
	eSPRITE_CALENDAR,
	eSPRITE_WEATHER_SUNCLOUD_BIG,
	eSPRITE_WEATHER_SUNCLOUD,
	eSPRITE_WEATHER_RAIN,
	eSPRITE_WEATHER_STORM,
	eSPRITE_CONTAINER_CORNER,
	eSPRITE_CONTAINER_VERT,
	eSPRITE_CONTAINER_HORI,
	eSPRITE_TEXT1,
	eSPRITE_TEXT2,
	eSPRITE_TEXT_WEATHER,
	eSPRITE_TEXT_FRI,
	eSPRITE_TEXT_SAT,
	eSPRITE_TEXT_SUN,
	eSPRITE_TEXT_MON,
	eSPRITE_CLOCKFACE_SMALL,
	eSPRITE_HAND_SMALL,
	eSPRITE_WINDOW_BOTTOM,
	eSPRITE_WINDOW_BOTTOMCORNER,
	eSPRITE_WINDOW_SIDE,
	eSPRITE_WINDOW_TOP,
	eSPRITE_WINDOW_TOPLEFT,
	eSPRITE_WINDOW_TOPRIGHT,

	eSPRITE_SIZE,
	eSPRITE_NONE = 0xFFFF
};

// Ancillary textures that won't be added to texture atlas (generally due to size)
enum eAncillary
{
	eANCILLARY_BACKGROUND,
	eANCILLARY_TOPBAR,

	eANCILLARY_SIZE
};

// Groups of quads that can be rendered together
enum eSpriteGroup
{
	eGROUP_BASE,
	eGROUP_CONTAINER,
	eGROUP_WINDOW,
	eGROUP_PAGE1,
	eGROUP_PAGE2,
	eGROUP_WINCONTENT,

	eGROUP_SIZE,
	eGROUP_NONE	= 0xFFFF
};

// Transformation types
enum eTrans
{
	eTRANS_NONE,
	eTRANS_HAND_ROTATION,

	eTRANS_SIZE
};

// Displayed pages
enum eDisplayPage
{
	eDISPPAGE_CLOCKS,
	eDISPPAGE_WEATHER,
	eDISPPAGE_WINDOW,

	eDISPPAGE_MAX,
	eDISPPAGE_DEFAULT = eDISPPAGE_CLOCKS
};

// Clipping shapes. As we're using the stencil buffer to clip, we are not limited to simple rectangles.
enum eClipShape
{
	eCLIPSHAPE_WINDOW,

	eCLIPSHAPE_SIZE,
	eCLIPSHAPE_NONE = 0xFFFF
};

/****************************************************************************
** Internal Enums
****************************************************************************/
// Pass type for rendering quads
enum eQuadPass
{
	eQUADPASS_OPAQUE,
	eQUADPASS_TRANSLUCENT,

	eQUADPASS_SIZE
};


// Display option. Toggled with keyboard.
enum eDisplayOption
{
	eDISPOPT_UI,
	eDISPOPT_TEXATLAS,

	eDISPOPT_SIZE,
	eDISPOPT_DEFAULT = eDISPOPT_UI
};

// Display state
enum eDisplayState
{
	eDISPSTATE_ELEMENT,
	eDISPSTATE_TRANSITION,

	eDISPSTATE_DEFAULT = eDISPSTATE_ELEMENT
};

// Render states
enum eRenderStates
{
	eRS_ALPHA_DISABLED		=(1<<0),
	eRS_ALPHA_ENABLED		=(1<<1),
	eRS_FILTER_BILINEAR		=(1<<12),
	eRS_FILTER_NEAREST		=(1<<13),

	eRS_ALPHA_MASK	= eRS_ALPHA_DISABLED | eRS_ALPHA_ENABLED,
	eRS_FILTER_MASK = eRS_FILTER_BILINEAR | eRS_FILTER_NEAREST
};

/****************************************************************************
** Structures
****************************************************************************/
// Group shader programs and their uniform locations together
struct SpriteShader
{
	GLuint uiId;
	GLuint uiMVPMatrixLoc;
	GLuint uiTransMatrixLoc;
};

struct ColShader
{
	GLuint uiId;
	GLuint uiMVPMatrixLoc;
	GLuint uiRGBALoc;
};

struct TexColShader : public ColShader
{
};

struct SpriteBorder
{
	GLuint uiBorderL;
	GLuint uiBorderR;
	GLuint uiBorderT;
	GLuint uiBorderB;
};

struct SpriteDesc
{
	GLuint			uiId;
	GLuint			uiWidth;
	GLuint			uiHeight;
	GLuint			uiSrcX;
	GLuint			uiSrcY;
	SpriteBorder	Border;
	bool			bHasAlpha;
};

struct PVRTVertex
{
	PVRTVec3		vPos;			// The vertex position
	PVRTVec2		vUV;			// Texture coordinate
	GLubyte			u8RGBA[4];		// Unsigned Byte RGBA
	GLfloat			fTransIndex;	// Transform index. GLSL:ES only accepts attributes in vector or float format, so cast between integer and float.
};

struct PVRTRectf
{
	GLfloat fX, fY, fW, fH;

	PVRTRectf() {}
	PVRTRectf(GLfloat x, GLfloat y, GLfloat w, GLfloat h) : fX(x), fY(y), fW(w), fH(h) {}
};

struct PVRTRecti
{
	GLint iX, iY, iW, iH;

	PVRTRecti() {}
	PVRTRecti(GLint x, GLint y, GLint w, GLint h) : iX(x), iY(y), iW(w), iH(h) {}
};

struct LayoutDesc
{
	eSprites		SpriteType;
	eSpriteGroup	Group;
	GLint			i32X;
	GLint			i32Y;
	GLuint			uiW;
	GLuint			uiH;
	GLubyte			u8RGBA[4];
	GLfloat			fOriginX;
	GLfloat			fOriginY;
	GLubyte			u8XFlip;
	GLubyte			u8YFlip;
	GLuint			uiTransform;			// The type of transform
	GLuint			uiTransformIndex;		// The index in to the transform array
	bool			bDeviceNorm;
};

struct GroupTree
{
	eSpriteGroup	Group;
	eSpriteGroup	Parent;
	GLint			i32X;
	GLint			i32Y;
	GLuint			uiW;
	GLuint			uiH;
};

struct PageDesc
{
	eSpriteGroup		GroupContainer;		// Specifies the page's container type
	eClipShape			ClipShape;			// The shape to clip this page with
};

struct PassData
{
	unsigned int			uiMask;
	CPVRTArray<LayoutDesc*>	apLayout;

	PassData() : uiMask(0) {}
};

/******************************************************************************
** Content file names
******************************************************************************/
const char* const c_apszAncillaryTex[eANCILLARY_SIZE] = 
{
	"background.pvr",		// eANCILLARY_BACKGROUND
	"topbar.pvr",			// eANCILLARY_TOPBAR
};

const char* const c_apszSprites[eSPRITE_SIZE] = 
{
	"clock-face.pvr",			// eSPRITE_CLOCKFACE
	"hand.pvr",					// eSPRITE_HAND
	"battery.pvr",				// eSPRITE_BATTERY
	"internet-web-browser.pvr",	// eSPRITE_WEB
	"mail-message-new.pvr",		// eSPRITE_NEWMAIL
	"network-wireless.pvr",		// eSPRITE_NETWORK
	"office-calendar.pvr",		// eSPRITE_CALENDAR

	"weather-sun-cloud-big.pvr",// eSPRITE_WEATHER_SUNCLOUD_BIG
	"weather-sun-cloud.pvr",	// eSPRITE_WEATHER_SUNCLOUD
	"weather-rain.pvr",			// eSPRITE_WEATHER_RAIN
	"weather-storm.pvr",		// eSPRITE_WEATHER_STORM

	"container-corner.pvr",		// eSPRITE_CONTAINER_CORNER
	"container-vertical.pvr",	// eSPRITE_CONTAINER_VERT
	"container-horizontal.pvr",	// eSPRITE_CONTAINER_HORI

	"text1.pvr",				// eSPRITE_TEXT1
	"text2.pvr",				// eSPRITE_TEXT2
	"text-weather.pvr",			// eSPRITE_TEXT_WEATHER
	"text-fri.pvr",				// eSPRITE_FRI
	"text-sat.pvr",				// eSPRITE_SAT
	"text-sun.pvr",				// eSPRITE_SUN
	"text-mon.pvr",				// eSPRITE_MON

	"clock-face-small.pvr",		// eSPRITE_CLOCKFACE_SMALL
	"hand-small.pvr",			// eSPRITE_HAND_SMALL

	"window-bottom.pvr",		// eSPRITE_WINDOW_BOTTOM
	"window-bottomcorner.pvr",	// eSPRITE_WINDOW_BOTTOMCORNER
	"window-side.pvr",			// eSPRITE_WINDOW_SIDE
	"window-top.pvr",			// eSPRITE_WINDOW_TOP
	"window-topleft.pvr",		// eSPRITE_WINDOW_TOPLEFT
	"window-topright.pvr",		// eSPRITE_WINDOW_TOPRIGHT
};
PVRTSIZEASSERT(c_apszSprites, eSPRITE_SIZE*sizeof(char*));

const char* const c_apszFragShaderSrcFile[eSHADER_SIZE] =
{
	"SpriteShaderF.fsh",		// eSPRITE_SHADER
	"TexColShaderF.fsh",		// eTEXCOL_SHADER
	"ColShaderF.fsh",			// eCOL_SHADER
};

const char* const c_apszFragShaderBinFile[eSHADER_SIZE] =
{
	"SpriteShaderF.fsc",		// eSPRITE_SHADER
	"TexColShaderF.fsc",		// eTEXCOL_SHADER
	"ColShaderF.fsc",			// eCOL_SHADER
};

const char* const c_apszVertShaderSrcFile[eSHADER_SIZE] =
{
	"SpriteShaderV.vsh",		// eSPRITE_SHADER
	"TexColShaderV.vsh",		// eTEXCOL_SHADER
	"ColShaderV.vsh",			// eCOL_SHADER
};

const char* const c_apszVertShaderBinFile[eSHADER_SIZE] =
{
	"SpriteShaderV.vsc",		// eSPRITE_SHADER
	"TexColShaderV.vsc",		// eTEXCOL_SHADER
	"ColShaderV.vsc",			// eCOL_SHADER
};

/******************************************************************************
** Auxillary functions prototypes and function pointer table
******************************************************************************/
class OGLES2ExampleUI;
int BuildCircleGeometry(const LayoutDesc& Desc, PVRTVertex* pVertArray, GLushort* pIndexArray, GLushort u16IndexStart, OGLES2ExampleUI* pApp);
int BuildQuadGeometry(const LayoutDesc& Desc, PVRTVertex* pVertArray, GLushort* pIndexArray, GLushort u16IndexStart, OGLES2ExampleUI* pApp);

typedef int (*GeomBuildFunc)(const LayoutDesc&, PVRTVertex*, GLushort*, GLushort, OGLES2ExampleUI*);
GeomBuildFunc GeomBuilderFunctions[eSPRITE_SIZE] = 
{
	BuildCircleGeometry,		// eSPRITE_CLOCKFACE,
	BuildQuadGeometry,			// eSPRITE_HAND,
	BuildQuadGeometry,			// eSPRITE_BATTERY,
	BuildQuadGeometry,			// eSPRITE_WEB,
	BuildQuadGeometry,			// eSPRITE_NEWMAIL,
	BuildQuadGeometry,			// eSPRITE_NETWORK,
	BuildQuadGeometry,			// eSPRITE_CALENDAR,
	BuildQuadGeometry,			// eSPRITE_WEATHER_SUNCLOUD_BIG,
	BuildQuadGeometry,			// eSPRITE_WEATHER_SUNCLOUD,
	BuildQuadGeometry,			// eSPRITE_WEATHER_RAIN,
	BuildQuadGeometry,			// eSPRITE_WEATHER_STORM,
	BuildQuadGeometry,			// eSPRITE_CONTAINER_CORNER,
	BuildQuadGeometry,			// eSPRITE_CONTAINER_VERT,
	BuildQuadGeometry,			// eSPRITE_CONTAINER_HORI,
	BuildQuadGeometry,			// eSPRITE_TEXT1,
	BuildQuadGeometry,			// eSPRITE_TEXT2,
	BuildQuadGeometry,			// eSPRITE_TEXT_WEATHER,
	BuildQuadGeometry,			// eSPRITE_TEXT_FRI,
	BuildQuadGeometry,			// eSPRITE_TEXT_SAT,
	BuildQuadGeometry,			// eSPRITE_TEXT_SUN,
	BuildQuadGeometry,			// eSPRITE_TEXT_MON,
	BuildCircleGeometry,		// eSPRITE_CLOCKFACE_SMALL,
	BuildQuadGeometry,			// eSPRITE_HAND_SMALL,
	BuildQuadGeometry,			// eSPRITE_WINDOW_BOTTOM,
	BuildQuadGeometry,			// eSPRITE_WINDOW_BOTTOMCORNER,
	BuildQuadGeometry,			// eSPRITE_WINDOW_SIDE,
	BuildQuadGeometry,			// eSPRITE_WINDOW_TOP,
	BuildQuadGeometry,			// eSPRITE_WINDOW_TOPLEFT,
	BuildQuadGeometry,			// eSPRITE_WINDOW_TOPRIGHT,
};

PVRTSIZEASSERT(GeomBuilderFunctions, eSPRITE_SIZE*sizeof(GeomBuildFunc));

/******************************************************************************
** Constants
******************************************************************************/
/*
   Sprite borders allow us to specify an internal border for each sprite that can be used to 
   nullify bleeding while using linear filtering in conjunction with a texture atlas.
*/
const SpriteBorder c_SpriteBorders[eSPRITE_SIZE] =
{
//	 L,R,T,B
	{0,0,0,0},			// eSPRITE_CLOCKFACE,
	{0,0,0,0},			// eSPRITE_HAND,
	{0,0,0,0},			// eSPRITE_BATTERY,
	{0,0,0,0},			// eSPRITE_WEB,
	{0,0,0,0},			// eSPRITE_NEWMAIL,
	{0,0,0,0},			// eSPRITE_NETWORK,
	{0,0,0,0},			// eSPRITE_CALENDAR,
	{0,0,0,0},			// eSPRITE_WEATHER_SUNCLOUD_BIG,
	{0,0,0,0},			// eSPRITE_WEATHER_SUNCLOUD,
	{0,0,0,0},			// eSPRITE_WEATHER_RAIN,
	{0,0,0,0},			// eSPRITE_WEATHER_STORM,
	{0,0,0,0},			// eSPRITE_CONTAINER_CORNER,
	{0,0,0,0},			// eSPRITE_CONTAINER_VERT,
	{0,0,0,0},			// eSPRITE_CONTAINER_HORI,
	{0,0,0,0},			// eSPRITE_TEXT1,
	{0,0,0,0},			// eSPRITE_TEXT2,
	{0,0,0,0},			// eSPRITE_TEXT_WEATHER,
	{0,0,0,0},			// eSPRITE_TEXT_FRI,
	{0,0,0,0},			// eSPRITE_TEXT_SAT,
	{0,0,0,0},			// eSPRITE_TEXT_SUN,
	{0,0,0,0},			// eSPRITE_TEXT_MON,
	{0,0,0,0},			// eSPRITE_CLOCKFACE_SMALL,
	{0,0,0,0},			// eSPRITE_HAND_SMALL,
	{2,2,4,0},			// eSPRITE_WINDOW_BOTTOM,
	{8,0,8,0},			// eSPRITE_WINDOW_BOTTOMCORNER,
	{4,0,2,2},			// eSPRITE_WINDOW_SIDE,
	{3,3,0,32},			// eSPRITE_WINDOW_TOP,
	{0,32,0,32},		// eSPRITE_WINDOW_TOPLEFT,
	{32,0,0,32},		// eSPRITE_WINDOW_TOPRIGHT,
};
PVRTSIZEASSERT(c_SpriteBorders, eSPRITE_SIZE*sizeof(SpriteBorder));

const char* const c_apszDisplayOpts[eDISPOPT_SIZE] =
{
	"Displaying Interface",			// eDISPOPT_UI
	"Displaying Texture Atlas",		// eDISPOPT_TEXATLAS
};

#ifdef DISPLAY_SPRITE_ALPHA
const char* const c_pszSpriteShaderDefines[] = 
{
	"DISPLAY_SPRITE_ALPHA",
};
#else
const char** const c_pszSpriteShaderDefines = NULL;
#endif

const char* const *c_appszShaderDefines[eSHADER_SIZE] =
{
	c_pszSpriteShaderDefines,			// eSPRITE_SHADER
	NULL,								// eTEXCOL_SHADER
	NULL,								// eTEXCOL_SHADER
};

static const unsigned int DIM_DEFAULT	= 0xABCD;
static const unsigned int DIM_CENTRE	= 0xABCE;
static const GLfloat c_fByteToFloat		= 1.0f / 255.0f;

static const char* const c_pszLoremIpsum = "Stencil Clipping\n\nLorem ipsum dolor sit amet, consectetuer adipiscing elit.\nDonec molestie. Sed aliquam sem ut arcu.\nPhasellus sollicitudin. Vestibulum condimentum facilisis nulla.\nIn hac habitasse platea dictumst. Nulla nonummy. Cras quis libero.\nCras venenatis. Aliquam posuere lobortis pede. Nullam fringilla urna id leo.\nPraesent aliquet pretium erat. Praesent non odio. Pellentesque a magna a\nmauris vulputate lacinia. Aenean viverra. Class aptent taciti sociosqu ad litora\ntorquent per conubia nostra, per inceptos hymenaeos. Aliquam\nlacus. Mauris magna eros, semper a, tempor et, rutrum et, tortor.";

const unsigned char MARKER[4] = {0xAC,0x1D,0xCA,0xFE};

#define MARKER_ID				{ MARKER[0],MARKER[1],MARKER[2],MARKER[3] }
#define MARKER_LINEAR_FILTER	{ eSPRITE_NONE, eGROUP_NONE, 0,0,0,0, MARKER_ID, 0.0f,0.0f,0,0, eRS_FILTER_BILINEAR, 0,0, },
#define MARKER_NEAREST_FILTER	{ eSPRITE_NONE, eGROUP_NONE, 0,0,0,0, MARKER_ID, 0.0f,0.0f,0,0, eRS_FILTER_NEAREST, 0,0, },


/******************************************************************************
** UI Layout
******************************************************************************/
LayoutDesc c_UILayout[] =
{	
	// Base Sprites
	//	Sprite Type					Group			X		Y		Width			Height			RGBA (in bytes)			Origin X/Y		X/Y Flip	Trans. type		i	Normalise
	{	eSPRITE_BATTERY,			eGROUP_BASE,	600,	2,		DIM_DEFAULT,	DIM_DEFAULT, 	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,	0,	true,	},
	{	eSPRITE_WEB,				eGROUP_BASE,	560,	2,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,	0,	true,	},
	{	eSPRITE_NEWMAIL,			eGROUP_BASE,	520,	2,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,	0,	true,	},
	{	eSPRITE_NETWORK,			eGROUP_BASE,	480,	2,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,	0,	true,	},

	// Container Sprites
	//	Sprite Type					Group				X		Y		Width			Height			RGBA (in bytes)			Origin X/Y		X/Y Flip	Trans. type		i	Normalise
	{	eSPRITE_CONTAINER_CORNER,	eGROUP_CONTAINER,	0,		0,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_CONTAINER_CORNER,	eGROUP_CONTAINER,	CON_W,	0,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		1.0f, 0.0f,		1, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_CONTAINER_CORNER,	eGROUP_CONTAINER,	0,		CON_H,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 1.0f,		0, 1,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_CONTAINER_CORNER,	eGROUP_CONTAINER,	CON_W,	CON_H,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		1.0f, 1.0f,		1, 1,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_CONTAINER_VERT,		eGROUP_CONTAINER,	32,		0,		CON_W-64,		DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_CONTAINER_VERT,		eGROUP_CONTAINER,	32,		CON_H,	CON_W-64,		DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 1.0f,		0, 1,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_CONTAINER_HORI,		eGROUP_CONTAINER,	0,		32,		DIM_DEFAULT,	CON_H-64,		{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_CONTAINER_HORI,		eGROUP_CONTAINER,	CON_W,	32,		DIM_DEFAULT,	CON_H-64,		{255, 255, 255, 255},		1.0f, 0.0f,		1, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_NONE,				eGROUP_CONTAINER,	32,		32,		CON_W-64,		CON_H-64,		{0,	 0,   0,   187},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,	0,	false,	},

	// Window Sprites
	//	Sprite Type						Group			X		Y		Width			Height			RGBA (in bytes)			Origin X/Y		X/Y Flip	Trans. type		i	Normalise
	{	eSPRITE_WINDOW_TOP,				eGROUP_WINDOW,	16,		0,		WIN_W-32,		32,				{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_WINDOW_TOPLEFT,			eGROUP_WINDOW,	0,		0,		32,				32,				{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_WINDOW_TOPRIGHT,		eGROUP_WINDOW,	WIN_W,	0,		32,				32,				{255, 255, 255, 255},		1.0f, 0.0f,		0, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_WINDOW_BOTTOMCORNER,	eGROUP_WINDOW,	0,		WIN_H,	8,				8,				{255, 255, 255, 255},		0.0f, 1.0f,		1, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_WINDOW_BOTTOMCORNER,	eGROUP_WINDOW,	WIN_W,	WIN_H,	8,				8,				{255, 255, 255, 255},		1.0f, 1.0f,		0, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_WINDOW_BOTTOM,			eGROUP_WINDOW,	8,		WIN_H,	WIN_W-16,		4,				{255, 255, 255, 255},		0.0f, 1.0f,		0, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_WINDOW_SIDE,			eGROUP_WINDOW,	0,		32,		4,				WIN_H-40,		{255, 255, 255, 255},		0.0f, 0.0f,		1, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_WINDOW_SIDE,			eGROUP_WINDOW,	WIN_W,	32,		4,				WIN_H-40,		{255, 255, 255, 255},		1.0f, 0.0f,		0, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_NONE,					eGROUP_WINDOW,	4,		4,		WIN_W-8,		WIN_H-8,		{244, 244, 244, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,	0,	false,	},

	// Page 1 Sprites - Parent is eGROUP_CONTAINER, so coordinated are local to this.
	//	Sprite Type					Group			X			Y		Width			Height			RGBA (in bytes)			Origin X/Y		X/Y Flip	Trans. type				i	Normalise
	{	eSPRITE_CLOCKFACE,			eGROUP_PAGE1,	256,		160,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.5f,		0, 0,		eTRANS_NONE,			0,	false,	},

	MARKER_LINEAR_FILTER

	{	eSPRITE_HAND,				eGROUP_PAGE1,	256,		160,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.875f,	0, 0,		eTRANS_HAND_ROTATION,	0,	false,	},

	MARKER_NEAREST_FILTER

	{	eSPRITE_CLOCKFACE_SMALL,	eGROUP_PAGE1,	0,			0,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_CLOCKFACE_SMALL,	eGROUP_PAGE1,	64,			0,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_CLOCKFACE_SMALL,	eGROUP_PAGE1,	128,		0,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_CLOCKFACE_SMALL,	eGROUP_PAGE1,	320,		0,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_CLOCKFACE_SMALL,	eGROUP_PAGE1,	384,		0,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_CLOCKFACE_SMALL,	eGROUP_PAGE1,	448,		0,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_CLOCKFACE_SMALL,	eGROUP_PAGE1,	0,			64,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_CLOCKFACE_SMALL,	eGROUP_PAGE1,	64,			64,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_CLOCKFACE_SMALL,	eGROUP_PAGE1,	384,		64,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_CLOCKFACE_SMALL,	eGROUP_PAGE1,	448,		64,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_CLOCKFACE_SMALL,	eGROUP_PAGE1,	0,			128,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_CLOCKFACE_SMALL,	eGROUP_PAGE1,	64,			128,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_CLOCKFACE_SMALL,	eGROUP_PAGE1,	384,		128,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_CLOCKFACE_SMALL,	eGROUP_PAGE1,	448,		128,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_CLOCKFACE_SMALL,	eGROUP_PAGE1,	0,			192,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_CLOCKFACE_SMALL,	eGROUP_PAGE1,	64,			192,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_CLOCKFACE_SMALL,	eGROUP_PAGE1,	384,		192,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_CLOCKFACE_SMALL,	eGROUP_PAGE1,	448,		192,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_CLOCKFACE_SMALL,	eGROUP_PAGE1,	0,			256,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_CLOCKFACE_SMALL,	eGROUP_PAGE1,	64,			256,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_CLOCKFACE_SMALL,	eGROUP_PAGE1,	384,		256,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_CLOCKFACE_SMALL,	eGROUP_PAGE1,	448,		256,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_HAND_SMALL,			eGROUP_PAGE1,	32,			32,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.875f,	0, 0,		eTRANS_HAND_ROTATION,	0,	false,	},
	{	eSPRITE_HAND_SMALL,			eGROUP_PAGE1,	96,			32,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.875f,	0, 0,		eTRANS_HAND_ROTATION,	0,	false,	},
	{	eSPRITE_HAND_SMALL,			eGROUP_PAGE1,	160,		32,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.875f,	0, 0,		eTRANS_HAND_ROTATION,	0,	false,	},
	{	eSPRITE_HAND_SMALL,			eGROUP_PAGE1,	352,		32,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.875f,	0, 0,		eTRANS_HAND_ROTATION,	0,	false,	},
	{	eSPRITE_HAND_SMALL,			eGROUP_PAGE1,	416,		32,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.875f,	0, 0,		eTRANS_HAND_ROTATION,	0,	false,	},
	{	eSPRITE_HAND_SMALL,			eGROUP_PAGE1,	480,		32,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.875f,	0, 0,		eTRANS_HAND_ROTATION,	0,	false,	},
	{	eSPRITE_HAND_SMALL,			eGROUP_PAGE1,	32,			96,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.875f,	0, 0,		eTRANS_HAND_ROTATION,	0,	false,	},
	{	eSPRITE_HAND_SMALL,			eGROUP_PAGE1,	96,			96,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.875f,	0, 0,		eTRANS_HAND_ROTATION,	0,	false,	},
	{	eSPRITE_HAND_SMALL,			eGROUP_PAGE1,	416,		96,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.875f,	0, 0,		eTRANS_HAND_ROTATION,	0,	false,	},
	{	eSPRITE_HAND_SMALL,			eGROUP_PAGE1,	480,		96,		DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.875f,	0, 0,		eTRANS_HAND_ROTATION,	0,	false,	},
	{	eSPRITE_HAND_SMALL,			eGROUP_PAGE1,	32,			160,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.875f,	0, 0,		eTRANS_HAND_ROTATION,	0,	false,	},
	{	eSPRITE_HAND_SMALL,			eGROUP_PAGE1,	96,			160,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.875f,	0, 0,		eTRANS_HAND_ROTATION,	0,	false,	},
	{	eSPRITE_HAND_SMALL,			eGROUP_PAGE1,	416,		160,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.875f,	0, 0,		eTRANS_HAND_ROTATION,	0,	false,	},
	{	eSPRITE_HAND_SMALL,			eGROUP_PAGE1,	480,		160,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.875f,	0, 0,		eTRANS_HAND_ROTATION,	0,	false,	},
	{	eSPRITE_HAND_SMALL,			eGROUP_PAGE1,	32,			224,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.875f,	0, 0,		eTRANS_HAND_ROTATION,	0,	false,	},
	{	eSPRITE_HAND_SMALL,			eGROUP_PAGE1,	96,			224,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.875f,	0, 0,		eTRANS_HAND_ROTATION,	0,	false,	},
	{	eSPRITE_HAND_SMALL,			eGROUP_PAGE1,	416,		224,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.875f,	0, 0,		eTRANS_HAND_ROTATION,	0,	false,	},
	{	eSPRITE_HAND_SMALL,			eGROUP_PAGE1,	480,		224,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.875f,	0, 0,		eTRANS_HAND_ROTATION,	0,	false,	},
	{	eSPRITE_HAND_SMALL,			eGROUP_PAGE1,	32,			288,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.875f,	0, 0,		eTRANS_HAND_ROTATION,	0,	false,	},
	{	eSPRITE_HAND_SMALL,			eGROUP_PAGE1,	96,			288,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.875f,	0, 0,		eTRANS_HAND_ROTATION,	0,	false,	},
	{	eSPRITE_HAND_SMALL,			eGROUP_PAGE1,	416,		288,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.875f,	0, 0,		eTRANS_HAND_ROTATION,	0,	false,	},
	{	eSPRITE_HAND_SMALL,			eGROUP_PAGE1,	480,		288,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.875f,	0, 0,		eTRANS_HAND_ROTATION,	0,	false,	},
	{	eSPRITE_TEXT1,				eGROUP_PAGE1,	20,			322,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_TEXT2,				eGROUP_PAGE1,	CON_W-32,	322,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		1.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_NONE,				eGROUP_PAGE1,	256,		320,	512,			1,				{191, 191, 191, 255},		0.5f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},
	{	eSPRITE_NONE,				eGROUP_PAGE1,	256,		320,	1,				80,				{191, 191, 191, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,			0,	false,	},

	// Page 2 Sprites - Parent is eGROUP_CONTAINER, so coordinates are local to this.
	//	Sprite Type						Group			X		Y		Width			Height			RGBA (in bytes)			Origin X/Y		X/Y Flip	Trans. type		i	Normalise
	{	eSPRITE_WEATHER_SUNCLOUD_BIG,	eGROUP_PAGE2,	256,	160,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.5f,		0, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_TEXT_WEATHER,			eGROUP_PAGE2,	2,		160,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.0f, 0.5f,		0, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_WEATHER_SUNCLOUD,		eGROUP_PAGE2,	64,		364,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.5f,		0, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_WEATHER_SUNCLOUD,		eGROUP_PAGE2,	192,	364,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.5f,		0, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_WEATHER_RAIN,			eGROUP_PAGE2,	320,	364,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.5f,		0, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_WEATHER_STORM,			eGROUP_PAGE2,	448,	364,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.5f,		0, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_TEXT_FRI,				eGROUP_PAGE2,	64,		324,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.5f,		0, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_TEXT_SAT,				eGROUP_PAGE2,	192,	322,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.5f,		0, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_TEXT_SUN,				eGROUP_PAGE2,	320,	322,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.5f,		0, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_TEXT_MON,				eGROUP_PAGE2,	448,	322,	DIM_DEFAULT,	DIM_DEFAULT,	{255, 255, 255, 255},		0.5f, 0.5f,		0, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_NONE,					eGROUP_PAGE2,	256,	312,	512,			1,				{191, 191, 191, 255},		0.5f, 0.0f,		0, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_NONE,					eGROUP_PAGE2,	128,	312,	1,				88,				{191, 191, 191, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_NONE,					eGROUP_PAGE2,	256,	312,	1,				88,				{191, 191, 191, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,	0,	false,	},
	{	eSPRITE_NONE,					eGROUP_PAGE2,	384,	312,	1,				88,				{191, 191, 191, 255},		0.0f, 0.0f,		0, 0,		eTRANS_NONE,	0,	false,	}
};

const GroupTree c_GroupTree[eGROUP_SIZE] = 
{
	// Group				Parent				X			Y			Width			Height
	{ eGROUP_BASE,			eGROUP_NONE,		0,			0,			DIM_DEFAULT,	DIM_DEFAULT,	},
	{ eGROUP_CONTAINER,		eGROUP_NONE,		DIM_CENTRE,	DIM_CENTRE, CON_W,			CON_H,			},
	{ eGROUP_WINDOW,		eGROUP_NONE,		DIM_CENTRE,	DIM_CENTRE,	WIN_W,			WIN_H,			},
	{ eGROUP_PAGE1,			eGROUP_CONTAINER,	0,			0,			DIM_DEFAULT,	DIM_DEFAULT,	},
	{ eGROUP_PAGE2,			eGROUP_CONTAINER,	0,			0,			DIM_DEFAULT,	DIM_DEFAULT,	},
	{ eGROUP_WINCONTENT,	eGROUP_WINDOW,		0,			0,			DIM_DEFAULT,	DIM_DEFAULT,	},
};

const bool c_bIsTransformUnique[eTRANS_SIZE] =
{
	false,						// eTRANS_NONE
	true,						// eTRANS_HAND_ROTATION
};

const PageDesc c_PageDescription[eDISPPAGE_MAX] =
{
	// Container			Clip Shape			
	{ eGROUP_CONTAINER,		eCLIPSHAPE_NONE },		// eDISPPAGE_CLOCKS
	{ eGROUP_CONTAINER,		eCLIPSHAPE_NONE },		// eDISPPAGE_WEATHER
	{ eGROUP_WINDOW,		eCLIPSHAPE_WINDOW },	// eDISPPAGE_WINDOW
};

const PVRTRectf c_ClipShapes[eCLIPSHAPE_SIZE] =
{
	PVRTRectf(2.0f, 22.0f, WIN_W - 4.0f, WIN_H - 24.0f),			// eCLIPSHAPE_WINDOW
};

static const unsigned int c_uiNumSprites = sizeof(c_UILayout) / sizeof(LayoutDesc);

/******************************************************************************
** Auxillary functions
******************************************************************************/
inline int MakePowerOfTwo(int iVal)
{
	int iTmp = 1;
	do
	{
		iTmp<<=1;
	} while(iTmp < iVal);
	return iTmp;
}

inline float QuadraticEaseOut(float fStart, float fEnd, float fT)
{
	float fTInv = 1.0f - fT;
	return ((fStart - fEnd)*fTInv*fTInv) + fEnd;
}

inline float QuadraticEaseIn(float fStart, float fEnd, float fT)
{
	return ((fEnd - fStart)*fT*fT) + fStart;
}

inline eSpriteGroup GroupFromDisplayPage(eDisplayPage Page)
{
	switch(Page)
	{
	case eDISPPAGE_CLOCKS:
		return eGROUP_PAGE1;
	case eDISPPAGE_WEATHER:
		return eGROUP_PAGE2;
	case eDISPPAGE_WINDOW:
		return eGROUP_WINCONTENT;
	default:
		break;
	}

	return GroupFromDisplayPage(eDISPPAGE_DEFAULT);
}

inline void HandRotateFunc(const LayoutDesc& Layout, PVRTMat4& mOut, float fRotate)
{
	PVRTMat4 mxRot, mxTrans;
	PVRTMatrixTranslation(mxTrans, (float)Layout.i32X, (float)Layout.i32Y, 0.0f);
	PVRTMatrixRotationZ(mxRot, fRotate);

	mOut = mxTrans * mxRot;
}

inline bool CheckState(unsigned int uiMask, eRenderStates eTest, eRenderStates eTestMask, unsigned int& uiClientState)
{
	if(uiMask & eTest)		// State requested
	{
		bool bNeedsUpdate = !(eTest & uiClientState);		// Check if this is already set in the mask
		if(bNeedsUpdate)
		{
			uiClientState &= ~eTestMask;
			uiClientState |= eTest;
		}
		return bNeedsUpdate;
	}

	return false;
}

/******************************************************************************
** Classes
******************************************************************************/
class Area
{
	private:
		int		m_iX; 
		int		m_iY;
		int		m_iW;
		int		m_iH;
		int		m_iSize;
		bool	m_bFilled;

		Area*	m_pRight;
		Area*	m_pLeft;

	private:
		void SetSize(int iWidth, int iHeight);
	public:
		Area(int iWidth, int iHeight);
		Area();

		Area* Insert(int iWidth, int iHeight);
		bool DeleteArea();

		int GetX();
		int GetY();
};

class CPassSort
{
public:
	bool operator()(const PassData& a, const PassData& b)
	{
		unsigned int uiStateA = a.uiMask & eRS_ALPHA_MASK;
		unsigned int uiStateB = b.uiMask & eRS_ALPHA_MASK;
		return (uiStateA > uiStateB);
	}
};

class CSpriteCompare
{
public:
	bool operator()(SpriteDesc* pSpriteDescA, SpriteDesc* pSpriteDescB)
	{
		GLuint uiASize = pSpriteDescA->uiWidth * pSpriteDescA->uiHeight;
		GLuint uiBSize = pSpriteDescB->uiWidth * pSpriteDescB->uiHeight;
		return (uiASize > uiBSize);
	}
};

class OGLES2ExampleUI : public PVRShell
{
	private:
		// Classes
		CPVRTPrint3D		m_Print3D;
        CPVRTPrint3D        m_PrintUI;
		SpriteDesc			m_aSprites[eSPRITE_SIZE];
		PVRTRectf			m_SpriteBox[eSPRITE_SIZE];			

		// Texture info and handles
		GLuint				m_uiAncillaryTex[eANCILLARY_SIZE];
		GLuint				m_uiTextureAtlas;
		GLuint				m_uiTextureBase;
		GLuint				m_uiBaseW;
		GLuint				m_uiBaseH;
		GLuint				m_uiAtlasW;
		GLuint				m_uiAtlasH;
		GLuint				m_uiFBOAtlas;
		GLuint				m_uiFBOBase;
		PVRTVec2			m_vNullTexCoords;
		bool				m_bAtlasGenerated;

		// Shader handles
		GLuint				m_auiVertShaderIds[eSHADER_SIZE];
		GLuint				m_auiFragShaderIds[eSHADER_SIZE];

		// Transforms
		PVRTMat4			m_mUIProj;
		PVRTMat4*			m_mTransforms;
		GLuint				m_uiNumTransforms;
		PVRTMat4			m_mDerivedGroup[eGROUP_SIZE];
		GLfloat				m_fHandRotate;
		GLfloat				m_fWinRotate;
		GLuint				m_uiTransformsInGroup[eGROUP_SIZE];

		GLint				m_i32OriginalFBO;

		// Programs
		SpriteShader		m_SpriteShader;
		TexColShader		m_TexColShader;
		ColShader			m_ColShader;

		CPVRTArray<PassData>	m_PassesPerGroup[eGROUP_SIZE];
		CPVRTArray<GLuint>		m_uiIndexOffsets[eGROUP_SIZE];		// The offset in to the index buffer for groups of quads.
		CPVRTArray<GLuint>		m_uiIndexCount[eGROUP_SIZE];		// The number of indices to render per pass, per group.
		unsigned int			m_uiCurrentRS;

		// VBO handles
		GLuint				m_ui32QuadVBO;
		GLuint				m_ui32QuadIndexVBO;

		// Data offset
		GLint				m_i32VOffset;
		GLint				m_i32UVOffset;
		GLint				m_i32RGBAOffset;
		GLint				m_i32TransIdxOffset;

		// Display options
		GLuint				m_uiDisplayOpt;
		GLuint				m_uiDisplayPage;			// The page to display. Could be several pages OR'd together in instances such as transitions.
		bool				m_bBaseDirty;
		eDisplayState		m_eState;
		float				m_fTransitionPerc;
		eDisplayPage		m_CurrentPage;
		eDisplayPage		m_LastPage;
		int					m_i32CycleDirection;

		// Data
		int					m_iDrawCallsPerFrame;

		// Time
		float				m_fWinRotPerc;
		unsigned long		m_ulPreviousTransTime;
		unsigned long		m_ulPreviousTime;

		// Builder functions
		friend int BuildCircleGeometry(const LayoutDesc&, PVRTVertex*, GLushort*, GLushort, OGLES2ExampleUI*);
		friend int BuildQuadGeometry(const LayoutDesc&, PVRTVertex*, GLushort*, GLushort, OGLES2ExampleUI*);

	private:
		void InitUI();
		void ScreenAlignedQuad(const ColShader& Shader, const PVRTRectf& DstRect, const PVRTRectf& SrcRect = PVRTRectf(0,0,1,1), const GLuint uiRGBA = 0xFFFFFFFF);

		void RenderUIGroup(eSpriteGroup Group, const PVRTMat4& mProj);
		void RenderBaseUI();
		void RenderUI();
		void RenderPage(eDisplayPage Page, const PVRTMat4& mTransform);
		void RenderAtlas();

		bool LoadSprites(CPVRTString* const pErrorStr);
		bool LoadShaders(CPVRTString* const pErrorStr);
		bool GenerateAtlas();
		bool FreeSprites();

		void LockBuffers();
		void ReleaseBuffers();

		int CountRequiredVertices();
		int CountRequiredIndices();

		void ApplyClipping(eClipShape Clip, const PVRTMat4& mMVP);
		void StopClipping();

		void UpdateRenderState(unsigned int uiMask);
    
        inline float GetVirtualWidth()
        {
            return (float)(IsRotated() ? PVRShellGet(prefHeight) : PVRShellGet(prefWidth));
        }
    
        inline float GetVirtualHeight()
        {
            return (float)(IsRotated() ? PVRShellGet(prefWidth) : PVRShellGet(prefHeight));
        }

		inline float ToDeviceX(float fVal)
		{
			return ((fVal / VIRTUAL_WIDTH) * GetVirtualWidth());
		}

		inline float ToDeviceY(float fVal)
		{
			return ((fVal / VIRTUAL_HEIGHT) * GetVirtualHeight());
		}
    
        inline bool IsRotated()
        {
            return PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);
        }

	public:
		OGLES2ExampleUI();
		virtual bool InitApplication();
		virtual bool InitView();
		virtual bool ReleaseView();
		virtual bool QuitApplication();
		virtual bool RenderScene();
};

/*!***********************************************************************
@Function			OGLES2ExampleUI
@Description		Constructor
*************************************************************************/
OGLES2ExampleUI::OGLES2ExampleUI() : m_uiTextureAtlas(0), m_uiTextureBase(0), m_uiBaseW(0), m_uiBaseH(0), m_uiAtlasW(0), m_uiAtlasH(0), m_uiFBOAtlas(0), m_uiFBOBase(0), m_bAtlasGenerated(false),
									 m_mTransforms(NULL), m_uiNumTransforms(0), m_fHandRotate(0.0f), m_fWinRotate(0.0f), m_i32OriginalFBO(0), m_uiCurrentRS(0), m_ui32QuadVBO(0), m_ui32QuadIndexVBO(0), 
									 m_uiDisplayOpt(eDISPOPT_DEFAULT), m_uiDisplayPage(eDISPPAGE_DEFAULT), m_bBaseDirty(true), m_eState(eDISPSTATE_DEFAULT), m_fTransitionPerc(0.0f),  
									 m_CurrentPage(eDISPPAGE_DEFAULT), m_LastPage(eDISPPAGE_DEFAULT), m_i32CycleDirection(1), m_iDrawCallsPerFrame(0), m_fWinRotPerc(0.0f), m_ulPreviousTransTime(0), m_ulPreviousTime(0)
{
	// Calculate some offsets
	PVRTVertex dummy;
	m_i32VOffset		= (unsigned int)((char*)(&dummy.vPos)        - (char*)(&dummy.vPos));
	m_i32UVOffset		= (unsigned int)((char*)(&dummy.vUV)         - (char*)(&dummy.vPos));
	m_i32RGBAOffset		= (unsigned int)((char*)(&dummy.u8RGBA)      - (char*)(&dummy.vPos));
	m_i32TransIdxOffset = (unsigned int)((char*)(&dummy.fTransIndex) - (char*)(&dummy.vPos));

	// Calculate unique transforms
	m_uiNumTransforms = 1;		// We know there's a minimum of 1 transform (identity transform)
	for(GLuint i = 0; i < c_uiNumSprites; ++i)
	{
		if(c_bIsTransformUnique[c_UILayout[i].uiTransform])
			c_UILayout[i].uiTransformIndex = m_uiNumTransforms++;
	}

	// Now we the size of the transform matrix
	m_mTransforms = new PVRTMat4[m_uiNumTransforms];
	PVRTMatrixIdentity(m_mTransforms[eTRANS_NONE]);			// Set eTRANS_NONE to identity
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
bool OGLES2ExampleUI::InitApplication()
{
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

    PVRShellSet(prefStencilBufferContext, true);
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
bool OGLES2ExampleUI::InitView()
{
	bool bRotate = IsRotated();
	GLfloat fOrigW = (float)PVRShellGet(prefWidth);
	GLfloat fOrigH = (float)PVRShellGet(prefHeight);
	m_mUIProj  = PVRTMat4::Ortho(0.0f, 0.0f, fOrigW, fOrigH, -1, 1, PVRTMat4::OGL, false);
    if(bRotate)
    {
        m_mUIProj = m_mUIProj * PVRTMat4::RotationZ(PVRT_PI_OVER_TWO);
        m_mUIProj.f[13] *= -1.0f;
    }
    
	m_ulPreviousTransTime = PVRShellGetTime();
    
    GLfloat fW = GetVirtualWidth();
    GLfloat fH = GetVirtualHeight();

	// Calculate derived matrices for each group, taking in to account the tree hierarchy
	// Transforms need to be calculated here as InitView is the first point in the program in which the device coordinates are known.
	PVRTMat4 mLocal, mIdentity;
	PVRTMatrixIdentity(mIdentity);
	float fX, fY;
	for(GLuint i = 0; i < eGROUP_SIZE; ++i)
	{
		// Figure out X,Y locations
		if(c_GroupTree[i].i32X == (int) DIM_CENTRE && c_GroupTree[i].uiW != DIM_DEFAULT)	
			fX = (fW-c_GroupTree[i].uiW)*0.5f;
		else																		
			fX = (float)c_GroupTree[i].i32X;

		if(c_GroupTree[i].i32Y == (int) DIM_CENTRE && c_GroupTree[i].uiH != DIM_DEFAULT)	
			fY = (fH-c_GroupTree[i].uiH)*0.5f;
		else																		
			fY = (float)c_GroupTree[i].i32Y;

		PVRTMatrixTranslation(mLocal, fX, fY, 0.0f);
		m_mDerivedGroup[i] = (c_GroupTree[i].Parent == eGROUP_NONE ? mIdentity : m_mDerivedGroup[c_GroupTree[i].Parent]) * mLocal;
	}

	CPVRTString ErrorStr;
	
	// Load the sprites
	if(!LoadSprites(&ErrorStr))
	{	
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Load the shaders
	if(!LoadShaders(&ErrorStr))
	{	
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Initialize Print3D
	if(m_Print3D.SetTextures(0, PVRShellGet(prefWidth), PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}
    
    if(m_PrintUI.SetTextures(0, PVRShellGet(prefWidth), PVRShellGet(prefHeight), false) != PVR_SUCCESS)
    {
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	// Set some GL states.
	// UI doesn't require depth test or write. Use painters algorithm instead for faster render.
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_FALSE);
	glClearStencil(0);
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Get the currently bound frame buffer object. On most platforms this just gives 0.
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_i32OriginalFBO);

	// Generate the atlas texture.
	if(!m_bAtlasGenerated)
		GenerateAtlas();

	// Generate FBO for the base screen texture
	m_uiBaseW = MakePowerOfTwo(GetVirtualWidth());
	m_uiBaseH = MakePowerOfTwo(GetVirtualHeight());
	glGenTextures(1, &m_uiTextureBase);
	glBindTexture(GL_TEXTURE_2D, m_uiTextureBase);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_uiBaseW, m_uiBaseH, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	// Disable linear filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Make sure our client-side render state is correct
	m_uiCurrentRS = eRS_FILTER_NEAREST | eRS_ALPHA_DISABLED;

	// Create texture atlas FBO and bind the previously created texture to it.
	glGenFramebuffers(1, &m_uiFBOBase);
	glBindFramebuffer(GL_FRAMEBUFFER, m_uiFBOBase);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uiTextureBase, 0);	

	// Check to make sure the FBO is OK.
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		PVRShellSet(prefExitMessage,"ERROR: Frame buffer did not set up correctly\n");
		return false;
	}
	
	// Clear the framebuffer
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Reset to main
	glBindFramebuffer(GL_FRAMEBUFFER, m_i32OriginalFBO);

	// Build the UI
	InitUI();

	return true;
}

/*!****************************************************************************
 @Function		LoadSprites
 @Output		pErrorStr	A string describing the error on failure
 @Return		bool		true if no error occured
 @Description	Loads an array of individual sprites that will be used to create
				a texture atlas.
******************************************************************************/
bool OGLES2ExampleUI::LoadSprites(CPVRTString* const pErrorStr)
{
	GLuint i = 0;
	PVRTextureHeaderV3 header;

	// Load sprites and add to sprite array so that we can generate a texture atlas from them.
	for(; i < eSPRITE_SIZE; ++i)
	{
		if(PVRTTextureLoadFromPVR(c_apszSprites[i], &m_aSprites[i].uiId, &header) != PVR_SUCCESS)
		{
			*pErrorStr = CPVRTString("ERROR: Could not open texture file ") + c_apszSprites[i];
			return false;
		}

		// Copy some useful data out of the texture header.
		m_aSprites[i].uiWidth  = header.u32Width;
		m_aSprites[i].uiHeight = header.u32Height;

		const char* pixelString = ((char*)&header.u64PixelFormat);

		if (header.u64PixelFormat == ePVRTPF_PVRTCI_2bpp_RGBA || header.u64PixelFormat == ePVRTPF_PVRTCI_4bpp_RGBA 
					|| pixelString[0] == 'a' || pixelString[1] == 'a' || pixelString[2] == 'a' || pixelString[3] == 'a')
			m_aSprites[i].bHasAlpha = true;
		else 
			m_aSprites[i].bHasAlpha = false;

		m_aSprites[i].Border    = c_SpriteBorders[i];

		// Disable filtering (not needed for 1 to 1 pixel rendering).
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	// Load ancillary textures
	for(i = 0; i < eANCILLARY_SIZE; ++i)
	{
		if(PVRTTextureLoadFromPVR(c_apszAncillaryTex[i], &m_uiAncillaryTex[i]) != PVR_SUCCESS)
		{
			*pErrorStr = CPVRTString("ERROR: Could not open texture file ") + c_apszAncillaryTex[i];
			return false;
		}

		// Disable filtering (not needed for 1 to 1 pixel rendering).
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	return true;
}

/*!****************************************************************************
 @Function		LoadShaders
 @Output		pErrorStr	A string describing the error on failure
 @Return		bool		true if no error occured
 @Description	Loads shaders.
******************************************************************************/
bool OGLES2ExampleUI::LoadShaders(CPVRTString* const pErrorStr)
{
	/*
		Load and compile the shaders from files.
		Binary shaders are tried first, source shaders
		are used as fallback.
	*/

	for(GLuint i = 0; i < eSHADER_SIZE; ++i)
	{
		if(PVRTShaderLoadFromFile(c_apszVertShaderBinFile[i], c_apszVertShaderSrcFile[i], GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, 
								  &m_auiVertShaderIds[i], pErrorStr, 0, c_appszShaderDefines[i], (c_appszShaderDefines[i] ? ELEMENTS_IN_ARRAY(c_appszShaderDefines[i]) : 0)) != PVR_SUCCESS)
		{
			return false;
		}

		if(PVRTShaderLoadFromFile(c_apszFragShaderBinFile[i], c_apszFragShaderSrcFile[i], GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, 
								  &m_auiFragShaderIds[i], pErrorStr, 0, c_appszShaderDefines[i], (c_appszShaderDefines[i] ? ELEMENTS_IN_ARRAY(c_appszShaderDefines[i]) : 0)) != PVR_SUCCESS)
		{
			return false;
		}
	}

	// --- SpriteShader
	// Link
	const char* aszSpriteShaderAttribs[] = { "inVertex", "inUVs", "inTransIdx", "inRGBA" };
	if (PVRTCreateProgram(&m_SpriteShader.uiId, m_auiVertShaderIds[eSPRITE_SHADER], m_auiFragShaderIds[eSPRITE_SHADER], aszSpriteShaderAttribs, 4, pErrorStr))
	{
		return false;
	}

	// Set some uniforms and get locations
	glUniform1i(glGetUniformLocation(m_SpriteShader.uiId, "Texture"), 0);
	m_SpriteShader.uiMVPMatrixLoc   = glGetUniformLocation(m_SpriteShader.uiId, "MVPMatrix");
	m_SpriteShader.uiTransMatrixLoc = glGetUniformLocation(m_SpriteShader.uiId, "MTransforms");

	// --- TexColShader
	// Link
	const char* aszTexColAttribs[] = { "inVertex", "inUVs"};
	if (PVRTCreateProgram(&m_TexColShader.uiId, m_auiVertShaderIds[eTEXCOL_SHADER], m_auiFragShaderIds[eTEXCOL_SHADER], aszTexColAttribs, 2, pErrorStr))
	{
		return false;
	}

	// Set some uniforms and get locations
	glUniform1i(glGetUniformLocation(m_TexColShader.uiId, "Texture"), 0);
	m_TexColShader.uiMVPMatrixLoc   = glGetUniformLocation(m_TexColShader.uiId, "MVPMatrix");
	m_TexColShader.uiRGBALoc		= glGetUniformLocation(m_TexColShader.uiId, "vRGBA");

	// --- ColShader
	// Link
	const char* aszColAttribs[] = { "inVertex"};
	if (PVRTCreateProgram(&m_ColShader.uiId, m_auiVertShaderIds[eCOL_SHADER], m_auiFragShaderIds[eCOL_SHADER], aszColAttribs, 1, pErrorStr))
	{
		return false;
	}

	// Set some uniforms and get locations
	m_ColShader.uiMVPMatrixLoc	= glGetUniformLocation(m_ColShader.uiId, "MVPMatrix");
	m_ColShader.uiRGBALoc		= glGetUniformLocation(m_ColShader.uiId, "vRGBA");


	return true;
}

/*!****************************************************************************
 @Function		GenerateAtlas
 @Return		bool		true if no error occured
 @Description	Sorts and packs sprites in to the texture atlas.
******************************************************************************/
bool OGLES2ExampleUI::GenerateAtlas()
{
	const unsigned int uiTotalBorder = ATLAS_PIXEL_BORDER * 2;

	// Figure out the necessary width and height for the texture atlas.
	// TODO. Hardcoded at the moment but remove and calculate. Brute force seems a little slow.
	m_uiAtlasW = ATLAS_WIDTH;
	m_uiAtlasH = ATLAS_HEIGHT;

	glGenTextures(1, &m_uiTextureAtlas);
	glBindTexture(GL_TEXTURE_2D, m_uiTextureAtlas);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_uiAtlasW, m_uiAtlasH, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	// Enable linear filtering such that rotated objects are automatically antialiased.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Create texture atlas FBO and bind the previously created texture to it.
	glGenFramebuffers(1, &m_uiFBOAtlas);
	glBindFramebuffer(GL_FRAMEBUFFER, m_uiFBOAtlas);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uiTextureAtlas, 0);	

	// Check to make sure the FBO is OK.
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		PVRShellSet(prefExitMessage,"ERROR: Frame buffer did not set up correctly\n");
		return false;
	}
	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glViewport(0, 0, m_uiAtlasW, m_uiAtlasH);
	
	PVRTMat4 mMVP = PVRTMat4::Ortho(0.0f, (PVRTfloat32)m_uiAtlasH, (PVRTfloat32)m_uiAtlasW, 0.0f, -1.0f, 1.0f, PVRTMat4::OGL, false);
	glUseProgram(m_TexColShader.uiId);
	glUniformMatrix4fv(m_TexColShader.uiMVPMatrixLoc,  1, GL_FALSE, mMVP.ptr());

	glActiveTexture(GL_TEXTURE0);

	// Sort sprites such that largest is first
	// Create an array of pointers to sprites so we can sort the pointers instead of the sprites themselves.
	CPVRTArray<SpriteDesc*> aSortedSprites;
	for(GLuint i = 0; i < eSPRITE_SIZE; ++i)
		aSortedSprites.Append(&m_aSprites[i]);

	CSpriteCompare pred;
	aSortedSprites.Sort(pred);

	// Set up the Area
	Area* pHead = new Area(m_uiAtlasW, m_uiAtlasH);
	Area* pRtrn = NULL;

	glDisable(GL_CULL_FACE);

	// Render some quads within the texture.
	GLfloat fX;
	GLfloat fY;

	for(GLuint i = 0; i < eSPRITE_SIZE; ++i)
	{
		pRtrn = pHead->Insert((int)aSortedSprites[i]->uiWidth + uiTotalBorder, (int)aSortedSprites[i]->uiHeight + uiTotalBorder);
		if(!pRtrn)
		{
			PVRShellSet(prefExitMessage,"ERROR: Not enough room in texture atlas!\n");
			pHead->DeleteArea();
			delete pHead;
			return false;
		}

		fX = (GLfloat)pRtrn->GetX() + ATLAS_PIXEL_BORDER;
		fY = (GLfloat)pRtrn->GetY() + ATLAS_PIXEL_BORDER;

		// Render sprite on to atlas.
		glBindTexture(GL_TEXTURE_2D, aSortedSprites[i]->uiId);
		PVRTRectf RectSrc(fX, fY, (GLfloat)aSortedSprites[i]->uiWidth, (GLfloat)aSortedSprites[i]->uiHeight);
		ScreenAlignedQuad(m_TexColShader, RectSrc);
	
		aSortedSprites[i]->uiSrcX = (GLuint)fX;
		aSortedSprites[i]->uiSrcY = (GLuint)fY;
	}

	// Now render a 4x4 white quad which can be utilised for rendering non-textured quads without switching the bound texture, or changing shader program.
	// We use 4x4 such that linear filtering will not produce an incorrect colour.
	glUseProgram(m_ColShader.uiId);
	glUniformMatrix4fv(m_ColShader.uiMVPMatrixLoc, 1, GL_FALSE, mMVP.ptr());
	{
		pRtrn = pHead->Insert(4, 4);
		if(!pRtrn)
			{
				PVRShellSet(prefExitMessage,"ERROR: Not enough room in texture atlas!\n");
				pHead->DeleteArea();
				delete pHead;
				return false;
			}
		fX = (GLfloat)pRtrn->GetX();
		fY = (GLfloat)pRtrn->GetY();

		// Shift in one pixel so that the sample will be taken from the centre of the 4x4 quad (see above regarding linear filtering)
		m_vNullTexCoords.x = fX + 1.0f;		
		m_vNullTexCoords.y = fY + 1.0f;
		
		ScreenAlignedQuad(m_ColShader, PVRTRectf(fX, fY, NULL_QUAD_PIX, NULL_QUAD_PIX));
	}
	// Done. Rebind original FBO.
	glBindFramebuffer(GL_FRAMEBUFFER, m_i32OriginalFBO);

	FreeSprites();
	pHead->DeleteArea();
	delete pHead;

	// Reset viewport to original
	glViewport(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));
	glEnable(GL_CULL_FACE);

	glUseProgram(0);

	m_bAtlasGenerated = true;
	return m_bAtlasGenerated;
}

/*!****************************************************************************
 @Function		InitUI
 @Return		void
 @Description	Initialises the interfaces and places the various elements.
******************************************************************************/
void OGLES2ExampleUI::InitUI()
{
	int i32VertsNeeded		= CountRequiredVertices();
	int i32IndicesNeeded	= CountRequiredIndices();
	PVRTVertex* Polys		= new PVRTVertex[i32VertsNeeded];
	GLushort* u16Indices	= new GLushort[i32IndicesNeeded];

	// --- Count passes per group
	unsigned int uiCurrentMask = eRS_FILTER_NEAREST;
	for(unsigned int i = 0; i < c_uiNumSprites; ++i)
	{
		const unsigned int uiGroup = c_UILayout[i].Group;
		const unsigned int uiSpriteType = c_UILayout[i].SpriteType;
		
		// Check for marker
		if(uiGroup == eGROUP_NONE)
		{
			if(uiSpriteType == eSPRITE_NONE && *((unsigned int*)c_UILayout[i].u8RGBA) == *((unsigned int*)MARKER))
			{
				// It's a marker. Update render state
				eRenderStates eNewState = (eRenderStates)c_UILayout[i].uiTransform;

				// Turn off associated states
				if(eNewState & eRS_FILTER_MASK)				uiCurrentMask &= ~eRS_FILTER_MASK;				// eRS_FILTER_MASK

				uiCurrentMask |= eNewState;				
			}
			
			continue;
		}
			
		// What's the render state mask for this sprite?
		bool bAlpha = (c_UILayout[i].u8RGBA[3] != 255) || (uiSpriteType != eSPRITE_NONE && m_aSprites[uiSpriteType].bHasAlpha);
		unsigned int uiMask = uiCurrentMask | (bAlpha ? eRS_ALPHA_ENABLED : eRS_ALPHA_DISABLED);

		// Does this mask already exist?
		int passidx = -1;
		for(unsigned int pass=0; pass < m_PassesPerGroup[uiGroup].GetSize();++pass)
		{
			if(m_PassesPerGroup[uiGroup][pass].uiMask == uiMask)
			{
				passidx = pass;
				break;
			}
		}

		if(passidx == -1)		// Mask not found. Add another pass.
		{
			passidx = m_PassesPerGroup[uiGroup].Append();
			m_PassesPerGroup[uiGroup][passidx].uiMask = uiMask;
		}

		// Add this sprite to the group
		CPVRTArray<LayoutDesc*>& Desc = m_PassesPerGroup[uiGroup][passidx].apLayout;
		Desc.Append(&c_UILayout[i]);
	}

	// --- Loop the UI database to generate the geometry and indices
	GLuint uiTotalTris	= 0;
	GLuint uiVertexIdx	= 0;
	GLuint uiIndexIdx	= 0;

	for(eSpriteGroup group = eGROUP_BASE; group < eGROUP_SIZE; group = (eSpriteGroup)(group + 1))
	{
		const unsigned int uiNumPasses = m_PassesPerGroup[group].GetSize();
		m_uiIndexOffsets[group].SetCapacity(uiNumPasses);
		m_uiIndexCount[group].SetCapacity(uiNumPasses);

		// Sort this group so that opaque passes are rendered first. This can be toggled with a constant.
		CPassSort pred;
		m_PassesPerGroup[group].Sort(pred);

		for(unsigned int uiPass = 0; uiPass < uiNumPasses; ++uiPass)
		{
			// Mark the index of this polygon.
			m_uiIndexOffsets[group][uiPass] = uiIndexIdx;
			GLuint uiNumIndices = 0;
			const PassData& PassData = m_PassesPerGroup[group][uiPass];

			for(unsigned int iLayout = 0; iLayout < PassData.apLayout.GetSize(); ++iLayout)
			{
				LayoutDesc* pLayout = PassData.apLayout[iLayout];

				// Get the function pointer
				int iTrisBuilt      = 0;
				GeomBuildFunc pFunc = NULL;
				if(pLayout->SpriteType == eSPRITE_NONE)
					pFunc = BuildQuadGeometry;
				else
					pFunc = GeomBuilderFunctions[pLayout->SpriteType];
				
				// Build geometry
				iTrisBuilt = pFunc(*pLayout, Polys + uiVertexIdx, u16Indices + uiIndexIdx, (GLushort)uiVertexIdx, this);
				uiNumIndices += iTrisBuilt * 3;
				uiVertexIdx  += iTrisBuilt + 2;
				uiIndexIdx   += iTrisBuilt * 3;
				uiTotalTris	 += iTrisBuilt;
			}

			// Count the number of indices for this group and pass
			m_uiIndexCount[group][uiPass] = uiNumIndices;
		}
	}
	
	// Upload the quads to a VBO
	glGenBuffers(1, &m_ui32QuadVBO);
	glGenBuffers(1, &m_ui32QuadIndexVBO);

	glBindBuffer(GL_ARRAY_BUFFER, m_ui32QuadVBO);
	const unsigned int uiVertexDataSize = i32VertsNeeded * sizeof(PVRTVertex);
	glBufferData(GL_ARRAY_BUFFER, uiVertexDataSize, Polys, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ui32QuadIndexVBO);
	const unsigned int uiIndexDataSize = i32IndicesNeeded * sizeof(GLushort);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiIndexDataSize, u16Indices, GL_STATIC_DRAW);

	// Unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	delete [] Polys;
	delete [] u16Indices;

#ifdef _DEBUG
	PVRShellOutputDebug("Built %u triangles.", uiTotalTris);
#endif
}

/*!****************************************************************************
 @Function		ScreenAlignedQuad
 @Return		void
 @Description	Renders a 2D quad with the given parameters. DstRect is the
				rectangle to be rendered in world coordinates. SrcRect is
				the rectangle to be cropped from the texture in pixel coordinates.
				NOTE: This is not an optimised function and should not be called
				repeatedly to draw quads to the screen at render time.
******************************************************************************/
void OGLES2ExampleUI::ScreenAlignedQuad(const ColShader& Shader, const PVRTRectf& DstRect, const PVRTRectf& SrcRect, const GLuint uiRGBA)
{
	PVRTVec4 vVerts[4] = 
	{
		PVRTVec4(DstRect.fX,				DstRect.fY,				 0, 1),
		PVRTVec4(DstRect.fX,				DstRect.fY + DstRect.fH, 0, 1),
		PVRTVec4(DstRect.fX + DstRect.fW,	DstRect.fY,				 0, 1),
		PVRTVec4(DstRect.fX + DstRect.fW,	DstRect.fY + DstRect.fH, 0, 1),
	};

	PVRTVec2 vUVs[4] =
	{
		PVRTVec2(SrcRect.fX, 1.0f - SrcRect.fY),
		PVRTVec2(SrcRect.fX, 1.0f - SrcRect.fH),
		PVRTVec2(SrcRect.fW, 1.0f - SrcRect.fY),
		PVRTVec2(SrcRect.fW, 1.0f - SrcRect.fH),
	};

	// Upload colour data for all verts
	PVRTVec4 vRGBA(((uiRGBA>>24)&0xFF)*c_fByteToFloat, ((uiRGBA>>16)&0xFF)*c_fByteToFloat, ((uiRGBA>>8)&0xFF)*c_fByteToFloat, (uiRGBA&0xFF)*c_fByteToFloat);
	glUniform4fv(Shader.uiRGBALoc, 1, (const float*)&vRGBA);

	// Enable the vertex attribute arrays
	glEnableVertexAttribArray(VERTEX_ARRAY);
	glEnableVertexAttribArray(TEXCOORD_ARRAY);

	glVertexAttribPointer(VERTEX_ARRAY, 4, GL_FLOAT, GL_FALSE, 0, &vVerts);
	glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, &vUVs);

	// Draw the quad
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// Safely disable the vertex attribute arrays
	glDisableVertexAttribArray(VERTEX_ARRAY);
	glDisableVertexAttribArray(TEXCOORD_ARRAY);	
}

/*!****************************************************************************
 @Function		FreeSprites
 @Return		bool		true if no error occured
 @Description	Frees the associated GL data.
******************************************************************************/
bool OGLES2ExampleUI::FreeSprites()
{
	for(GLuint i = 0; i < eSPRITE_SIZE; ++i)
		glDeleteTextures(1, &m_aSprites[i].uiId);

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES2ExampleUI::ReleaseView()
{
	// Frees the OpenGL handles for the programs
	glDeleteProgram(m_SpriteShader.uiId);
	glDeleteProgram(m_TexColShader.uiId);
	glDeleteProgram(m_ColShader.uiId);

	// Frees OpenGL handles for shaders.
	for(GLuint i = 0; i < eSHADER_SIZE; ++i)
	{
		glDeleteShader(m_auiVertShaderIds[i]);
		glDeleteShader(m_auiFragShaderIds[i]);
	}

	// Free textures
	glDeleteTextures(1, &m_uiTextureAtlas);
	glDeleteTextures(1, &m_uiTextureBase);
	glDeleteTextures(eANCILLARY_SIZE, m_uiAncillaryTex);

	// Free FBO
	glDeleteFramebuffers(1, &m_uiFBOAtlas);
	glDeleteFramebuffers(1, &m_uiFBOBase);

	// Free VBO
	glDeleteBuffers(1, &m_ui32QuadVBO);
	glDeleteBuffers(1, &m_ui32QuadIndexVBO);
			
	// Release Print3D Textures
	m_Print3D.ReleaseTextures();
    m_PrintUI.ReleaseTextures();
	
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
bool OGLES2ExampleUI::QuitApplication()
{
	delete [] m_mTransforms;

	return true;
}

/*!****************************************************************************
 @Function		RenderUI
 @Return		void
 @Description	Renders the background interface when requested.
				Draws the background image and the title bar/base bar.
******************************************************************************/
void OGLES2ExampleUI::RenderBaseUI()
{
	if(!m_bBaseDirty)
		return;

	float fDeviceWidth   = GetVirtualWidth();
    float fDeviceHeight  = GetVirtualHeight();
	// Bind the base UI framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_uiFBOBase);

	// Set viewport to the size of the FBO texture
	glViewport(0, 0, m_uiBaseW, m_uiBaseH);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(m_TexColShader.uiId);

	// OGL expects the first pixel to be in the bottom left corner of the texture, so we need to translate the interface down to this corner when genertaing the FBO texture.
	PVRTMat4 mMVP  = PVRTMat4::Ortho(0.0f, 0.0f, (GLfloat)m_uiBaseW, (GLfloat)m_uiBaseH, -1.0f, 1.0f, PVRTMat4::OGL, false);
	glUniformMatrix4fv(m_TexColShader.uiMVPMatrixLoc, 1, GL_FALSE, mMVP.ptr());

	PVRTRectf RectDst, RectSrc;

	// Render the background texture
	glBindTexture(GL_TEXTURE_2D, m_uiAncillaryTex[eANCILLARY_BACKGROUND]);
	RectDst = PVRTRectf(0.0f, 0.0f, fDeviceWidth, fDeviceHeight);
	RectSrc = PVRTRectf(0.0f, 0.0f, 640.0f / 1024.0f, 480.0f / 1024.0f);		// Background texture is 1024x1024, but pixel data is only contained in the top 640x480 section.
	ScreenAlignedQuad(m_TexColShader, RectDst, RectSrc);

	// Render the top bar
	glBindTexture(GL_TEXTURE_2D, m_uiAncillaryTex[eANCILLARY_TOPBAR]);
	ScreenAlignedQuad(m_TexColShader, PVRTRectf(0.0f, 0.0f, fDeviceWidth, 36.0f), PVRTRectf(0.0f, 0.0f, 1.0f, 36.0f / 64.0f));		// Bar height is 36, but the texture is 64 (POT)

	// Render the entire group
	LockBuffers();
	RenderUIGroup(eGROUP_BASE, mMVP);
	ReleaseBuffers();

	// Done.
	glBindFramebuffer(GL_FRAMEBUFFER, m_i32OriginalFBO);
	glViewport(0, 0,  PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	m_bBaseDirty = false;
}

/*!****************************************************************************
 @Function		LockBuffers
 @Return		void
 @Description	Enables the various attribute array and binds the vertex pointers
				ready for rendering. We also use the SpriteShader program
				and upload data.
******************************************************************************/
void OGLES2ExampleUI::LockBuffers()
{
	glUseProgram(m_SpriteShader.uiId);

	// Upload transform array
	GLuint uiTransArraySize = (m_uiNumTransforms > 30 ? 30 : m_uiNumTransforms);
	glUniformMatrix4fv(m_SpriteShader.uiTransMatrixLoc, uiTransArraySize, GL_FALSE, m_mTransforms[0].ptr());		// Transform array > 16 will need to be batched.

	glBindTexture(GL_TEXTURE_2D, m_uiTextureAtlas);

	// Enable the vertex attribute arrays
	glEnableVertexAttribArray(VERTEX_ARRAY);
	glEnableVertexAttribArray(TEXCOORD_ARRAY);
	glEnableVertexAttribArray(TRANSFORM_ARRAY);
	glEnableVertexAttribArray(RGBA_ARRAY);

	// Bind VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32QuadVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ui32QuadIndexVBO);

	// Set attribute pointers to offset in to VBO (precalculated).
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, sizeof(PVRTVertex),   (const void*)m_i32VOffset);
	glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, sizeof(PVRTVertex), (const void*)m_i32UVOffset);
	glVertexAttribPointer(TRANSFORM_ARRAY, 1, GL_FLOAT, GL_FALSE, sizeof(PVRTVertex), (const void*)m_i32TransIdxOffset);
	glVertexAttribPointer(RGBA_ARRAY, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(PVRTVertex), (const void*)m_i32RGBAOffset);
}

/*!****************************************************************************
 @Function		RenderUI
 @Return		void
 @Description	Releases the vertex pointers and disables attribute arrays.
******************************************************************************/
void OGLES2ExampleUI::ReleaseBuffers()
{
	// Unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Safely disable the vertex attribute arrays
	glDisableVertexAttribArray(VERTEX_ARRAY);
	glDisableVertexAttribArray(TEXCOORD_ARRAY);
	glDisableVertexAttribArray(TRANSFORM_ARRAY);
	glDisableVertexAttribArray(RGBA_ARRAY);

	glUseProgram(0);
}

/*!****************************************************************************
 @Function		UpdateRenderState
 @Return		void
 @Input			unsigned int uiMask			The render state mask to apply.
 @Description	Updates the GL render state and tracks to check for redundant state
				changes.
******************************************************************************/
void OGLES2ExampleUI::UpdateRenderState(unsigned int uiMask)
{
	// Check to see if a change is required
	if(uiMask == m_uiCurrentRS)
		return;			// Nothing to do!


	if(CheckState(uiMask, eRS_ALPHA_ENABLED, eRS_ALPHA_MASK, m_uiCurrentRS))
		glEnable(GL_BLEND);
	else if(CheckState(uiMask, eRS_ALPHA_DISABLED, eRS_ALPHA_MASK, m_uiCurrentRS))
		glDisable(GL_BLEND);

	if(CheckState(uiMask, eRS_FILTER_BILINEAR, eRS_FILTER_MASK, m_uiCurrentRS))
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else if(CheckState(uiMask, eRS_FILTER_NEAREST, eRS_FILTER_MASK, m_uiCurrentRS))
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
}

/*!****************************************************************************
 @Function		RenderUI
 @Return		void
 @Description	Renders a specific UI group.
******************************************************************************/
void OGLES2ExampleUI::RenderUIGroup(eSpriteGroup Group, const PVRTMat4& mProj)
{
	// Upload MVP matrix.
	glUniformMatrix4fv(m_SpriteShader.uiMVPMatrixLoc, 1, GL_FALSE, mProj.f);

#ifdef _DEBUG
	// Check to make sure we have a bound VBO buffer (i.e we have called LockBuffers())
	GLint iReturn;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &iReturn);
	_ASSERT(iReturn);
#endif

	unsigned int uiIndicesToDraw = 0;

	// Loop the passes in this group
	for(unsigned int iPass = 0; iPass < m_PassesPerGroup[Group].GetSize(); ++iPass)
	{
		// Check render state
		UpdateRenderState(m_PassesPerGroup[Group][iPass].uiMask);

		// Overide the render state in a few instances
		if(Group == eGROUP_WINDOW && m_fWinRotPerc > 0.0f && m_fWinRotPerc < 1.0f)
		{
			UpdateRenderState(eRS_FILTER_BILINEAR);					// Turn on linear filtering for the Window.
		}

		// Draw the quads
		uiIndicesToDraw = m_uiIndexCount[Group][iPass];			// The number of quads to draw for this opaque pass.
		if(uiIndicesToDraw > 0)
			glDrawElements(GL_TRIANGLES, uiIndicesToDraw, GL_UNSIGNED_SHORT, (char*)(m_uiIndexOffsets[Group][iPass] * sizeof(GLushort)));
	}
}


/*!****************************************************************************
 @Function		RenderPage
 @Return		void
 @Input			eDisplayPage	The page to render
 @Input			PVRTMat4&		Transform marix
 @Description	Renders a page which includes an optional container, elements
				and optional clipping.
******************************************************************************/
void OGLES2ExampleUI::RenderPage(eDisplayPage Page, const PVRTMat4& mTransform)
{
	PVRTMat4 mMVP;

	// Should we draw the container?
	eSpriteGroup Container = c_PageDescription[Page].GroupContainer;
	if(Container != eGROUP_NONE)
	{
		// Calculate MVP with UI projection matrix, and container view translation
		mMVP = m_mUIProj * m_mDerivedGroup[Container] * mTransform;

		// Render the container
		RenderUIGroup(Container, mMVP);
	}
	else
	{
		// Base MVP on page matrix
		mMVP = m_mUIProj * m_mDerivedGroup[GroupFromDisplayPage(Page)] * mTransform;
	}

	eClipShape Clip = c_PageDescription[Page].ClipShape;
	if(Clip != eCLIPSHAPE_NONE)
	{
		ReleaseBuffers();			// Need to release the currently bound buffer before we draw the clip.
		ApplyClipping(Clip, mMVP);
		LockBuffers();				// Make sure buffers are locked again.
	}

	// Render the page
	RenderUIGroup(GroupFromDisplayPage(Page), mMVP);

	// Render text if this is the 'Window'. Normally this would be included in a layout but for the purposes of this demo the values below are hardcoded and set to give the best visual appearance.
	if(Page == eDISPPAGE_WINDOW)
	{
		ReleaseBuffers();

		PVRTMat4 mxMV = m_mDerivedGroup[Container] * mTransform * PVRTMat4::Scale(1.0f, -1.0f, 1.0f);
		m_PrintUI.SetProjection(m_mUIProj);
		m_PrintUI.SetModelView(mxMV);
		m_PrintUI.Print3D(4.0f, 20.0f, 0.5f, 0xFF000000, c_pszLoremIpsum);
		m_PrintUI.Flush();

		LockBuffers();
	}

	if(Clip != eCLIPSHAPE_NONE)
		StopClipping();
}


/*!****************************************************************************
 @Function		RenderAtlas
 @Return		void
 @Description	Renders the default interface.
******************************************************************************/
void OGLES2ExampleUI::RenderUI()
{
	PVRTMat4 mMVP;

	// Render Base UI to FBO
	RenderBaseUI();

	// Render Base UI to screen    
	GLfloat fScreenW = GetVirtualWidth();
	GLfloat fScreenH = GetVirtualHeight();
	glUseProgram(m_TexColShader.uiId);
	glUniformMatrix4fv(m_TexColShader.uiMVPMatrixLoc,  1, GL_FALSE, m_mUIProj.ptr());
	glBindTexture(GL_TEXTURE_2D, m_uiTextureBase);
	ScreenAlignedQuad(m_TexColShader, PVRTRectf(0.0f, 0.0f, fScreenW, fScreenH), PVRTRectf(0.0f, 0.0f, fScreenW/m_uiBaseW, fScreenH/m_uiBaseH));

	// --- Draw the container/dynamic elements
	// Render the single page if there's not currently an active transition
	if(m_eState == eDISPSTATE_ELEMENT)
	{
		LockBuffers();

		// A transformation matrix
		PVRTMat4 mTrans = PVRTMat4::Identity();
		if(m_CurrentPage == eDISPPAGE_WINDOW)
		{
			PVRTMat4 vRot, vCentre, vInv;
			PVRTMatrixRotationZ(vRot, m_fWinRotate);
			PVRTMatrixTranslation(vCentre, -(WIN_W*0.5f), -(WIN_H*0.5f), 0.0f);
			vInv = vCentre.inverse();

			mTrans = vInv * vRot * vCentre;
		}

		// Just render the single, current page
		RenderPage(m_CurrentPage, mTrans);

		ReleaseBuffers();
	}
	else if(m_eState == eDISPSTATE_TRANSITION)
	{
		LockBuffers();

		float fX;
		PVRTMat4 mxContainerTrans;

		// --- Render outward group
		fX = QuadraticEaseIn(0.0f, -fScreenW*m_i32CycleDirection, m_fTransitionPerc);
		PVRTMatrixTranslation(mxContainerTrans, (float)((int)fX), 0.0f, 0.0f);
		
		// Render page
		RenderPage(m_LastPage, mxContainerTrans);

		// --- Render inward group
		fX = QuadraticEaseIn(fScreenW*m_i32CycleDirection, 0.0f, m_fTransitionPerc);
		PVRTMatrixTranslation(mxContainerTrans, (float)((int)fX), 0.0f, 0.0f);

		// Render page
		RenderPage(m_CurrentPage, mxContainerTrans);

		ReleaseBuffers();
	}
	else
	{
		_ASSERT(!"Unhandled state!");
	}

	ReleaseBuffers();
}

/*!****************************************************************************
 @Function		RenderAtlas
 @Return		void
 @Description	Renders the generated texture atlas.
******************************************************************************/
void OGLES2ExampleUI::RenderAtlas()
{
	bool bRotate = IsRotated();
	PVRTMat4 mMVP = PVRTMat4::Ortho(0.0f, (float)PVRShellGet(prefHeight), (float)PVRShellGet(prefWidth), 0.0f, -1, 1, PVRTMat4::OGL, bRotate);

	glDisable(GL_CULL_FACE);

	glUseProgram(m_TexColShader.uiId);
	glUniformMatrix4fv(m_TexColShader.uiMVPMatrixLoc,  1, GL_FALSE, mMVP.ptr());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uiTextureAtlas);

	float fScale[2];
    fScale[0] = GetVirtualWidth()  / m_uiAtlasW;
    fScale[1] = GetVirtualHeight() / m_uiAtlasH;
    
    float fMinScale = fScale[0] < fScale[1] ? fScale[0] : fScale[1];        // Get minimum scale

	// Enable some states
	UpdateRenderState(eRS_ALPHA_ENABLED | eRS_FILTER_BILINEAR);

	ScreenAlignedQuad(m_TexColShader, PVRTRectf(0.0f, 0.0f, m_uiAtlasW*fMinScale, m_uiAtlasH*fMinScale));

	// Disable states
	UpdateRenderState(eRS_ALPHA_DISABLED | eRS_FILTER_NEAREST);

	glEnable(GL_CULL_FACE);

	glUseProgram(0);
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
bool OGLES2ExampleUI::RenderScene()
{
	// Process inputs first
	if(PVRShellIsKeyPressed(PVRShellKeyNameUP))
	{
		m_uiDisplayOpt--;
		if((GLint)m_uiDisplayOpt < 0)	m_uiDisplayOpt = eDISPOPT_SIZE-1;
	}
	else if(PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
	{
		m_uiDisplayOpt++;
		if(m_uiDisplayOpt >= eDISPOPT_SIZE)	m_uiDisplayOpt = 0;
	}

	// --- Update UI components
	unsigned long ulCurrentTime = PVRShellGetTime();
	float fDT					= (ulCurrentTime - m_ulPreviousTime) * 0.001f;
	m_ulPreviousTime			= ulCurrentTime;
	// Update the clock hand
	{
		m_fHandRotate -= (PVRT_TWO_PI / 60) * 0.016f;
	}

	// Update Window rotation
	{
		m_fWinRotPerc += (1.0f / UI_DISPLAY_TIME) * fDT;
		m_fWinRotate   = QuadraticEaseOut(0.0f, PVRT_TWO_PI, m_fWinRotPerc);
	}

	// Check to see if we should transition to a new page (if we're not already)
	if(ulCurrentTime - m_ulPreviousTransTime > UI_DISPLAY_TIME*1000 && m_eState != eDISPSTATE_TRANSITION)
	{
		// Switch to next page
		m_eState				= eDISPSTATE_TRANSITION;
		m_fTransitionPerc		= 0.0f;
		m_LastPage				= m_CurrentPage;

		// Cycle pages
		GLint i32NextPage = m_CurrentPage + m_i32CycleDirection;
		if(i32NextPage >= eDISPPAGE_MAX || i32NextPage < 0)
		{
			m_i32CycleDirection *= -1;							// Reverse direction
			i32NextPage = m_CurrentPage + m_i32CycleDirection;	// Recalculate
		}

		m_CurrentPage = (eDisplayPage)i32NextPage;
	}

	// Calculate next transition amount
	if(m_eState == eDISPSTATE_TRANSITION)
	{
		m_fTransitionPerc += 0.016f;		// 60 FPS
		if(m_fTransitionPerc > 1.0f)
		{
			m_eState = eDISPSTATE_ELEMENT;
			m_fTransitionPerc	= 1.0f;
			m_fWinRotate		= 0.0f;			// Reset Window rotation
			m_fWinRotPerc		= 0.0f;			// Reset Window rotation percentage
			m_ulPreviousTransTime	= ulCurrentTime;	// Reset time
		}
	}

	// Update transform array
	unsigned int uiIdx = 1;
	for(unsigned int i = 0; i < c_uiNumSprites; ++i)
	{
		if(c_UILayout[i].uiTransform == eTRANS_NONE)
			continue;
		
		switch(c_UILayout[i].uiTransform)
		{
			case eTRANS_HAND_ROTATION:			
				HandRotateFunc(c_UILayout[i], m_mTransforms[c_UILayout[i].uiTransformIndex], m_fHandRotate*uiIdx);
				break;
		}

		++uiIdx;
	}
	

	// Clear buffers and render the scene
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);			// Don't bother clearing the depth bit as we aren't using depth test.

	m_iDrawCallsPerFrame = 0;

	switch(m_uiDisplayOpt)
	{
		case eDISPOPT_UI:			
			RenderUI();	
			break;
		case eDISPOPT_TEXATLAS:		
			RenderAtlas();	
			break;
	}

    m_Print3D.DisplayDefaultTitle("Example UI", c_apszDisplayOpts[m_uiDisplayOpt], ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();
    
	return true;
}

/*!****************************************************************************
 @Function		CountRequiredVertices
 @Return		int		The number of vertices required for the scene.
 @Description	Loops through each quad for the scene and calculates the
				number of required vertices.
******************************************************************************/
int OGLES2ExampleUI::CountRequiredVertices()
{
	int i32Verts = 0;
	GLushort idx = 0;
	for(GLuint i = 0; i < c_uiNumSprites; ++i)
	{
		GeomBuildFunc pFunc = NULL;
		if(c_UILayout[i].SpriteType == eSPRITE_NONE)
			pFunc = BuildQuadGeometry;
		else
			pFunc = GeomBuilderFunctions[c_UILayout[i].SpriteType];

		i32Verts += pFunc(c_UILayout[i], NULL, NULL, idx, this) + 2;
	}

	return i32Verts;
}

/*!****************************************************************************
 @Function		CountRequiredIndices
 @Return		int		The number of indices required for the scene.
 @Description	Loops through each quad for the scene and calculates the
				number of required indices.
******************************************************************************/
int OGLES2ExampleUI::CountRequiredIndices()
{
	int i32Indices = 0;
	GLushort idx = 0;
	for(GLuint i = 0; i < c_uiNumSprites; ++i)
	{
		GeomBuildFunc pFunc = NULL;
		if(c_UILayout[i].SpriteType == eSPRITE_NONE)
			pFunc = BuildQuadGeometry;
		else
			pFunc = GeomBuilderFunctions[c_UILayout[i].SpriteType];

		i32Indices += pFunc(c_UILayout[i], NULL, NULL, idx, this) * 3;
	}

	return i32Indices;
}

/*!****************************************************************************
 @Function		ApplyClipping
 @Input			eClipShape		An enum for the clip shape
 @Description	Enables stencil clipping and draws the clip shape to the
				stencil buffer. Sets up stencil testing.
******************************************************************************/
void OGLES2ExampleUI::ApplyClipping(eClipShape Clip, const PVRTMat4& mMVP)
{
	// Begin stencil test
	glEnable(GL_STENCIL_TEST);

	// Set stencil function to always pass, and write 0x1 in to the stencil buffer.
	glStencilFunc(GL_ALWAYS, 0x1, 0xFFFFFFFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	glUseProgram(m_ColShader.uiId);
	glUniformMatrix4fv(m_ColShader.uiMVPMatrixLoc, 1, GL_FALSE, mMVP.f);

	ScreenAlignedQuad(m_ColShader, c_ClipShapes[Clip]);

	// Set stencil function to only pass if there's 0x1 already in the stencil buffer.
	glStencilFunc(GL_EQUAL, 0x1, 0xFFFFFFFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

#if defined(__PALMPDK__)
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE); // The alpha part is false as we don't want to blend with the video layer on the Palm Pre
#else
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
#endif
}

/*!****************************************************************************
 @Function		StopClipping
 @Description	Disables stencil clipping.
******************************************************************************/
void OGLES2ExampleUI::StopClipping()
{
	glDisable(GL_STENCIL_TEST);
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
	return new OGLES2ExampleUI();
}

/*!****************************************************************************
 @Function		BuildCircleGeometry
 @Return		int			The number of triangles required to build.
 @Input			LayoutDesc& Reference to the layout table for this sprite.
 @Input			PVRTVertex*	Pointer to vertex array.
 @Input			GLushort*	Pointer to index array.
 @Description	
******************************************************************************/
int BuildCircleGeometry(const LayoutDesc& Desc, PVRTVertex* pVertArray, GLushort* pIndexArray, GLushort u16IndexStart, OGLES2ExampleUI* pApp)
{
	const int iNumTris = 12;

	if(pVertArray == NULL || pIndexArray == NULL)
		return iNumTris;

	SpriteDesc* aSprites = pApp->m_aSprites;
	eSprites Sprite		 = Desc.SpriteType;

	PVRTVec3 vTrans;
	memset(&vTrans, 0, sizeof(vTrans));

	// OK to translate the verts if this sprite has no transformation matrix associated.
	if(Desc.uiTransform == eTRANS_NONE)
	{
		if(Desc.bDeviceNorm)
			vTrans = PVRTVec3(pApp->ToDeviceX((PVRTfloat32)Desc.i32X), pApp->ToDeviceY((PVRTfloat32)Desc.i32Y), 0.0f);		// Normalise to device coordinates
		else
			vTrans = PVRTVec3((PVRTfloat32)Desc.i32X, (PVRTfloat32)Desc.i32Y, 0.0f);
	}

	// The origin of the polygon
	PVRTVec2 vOrigin(Desc.fOriginX, Desc.fOriginY);

	// Choose either the original sprite dimensions, or the provided dimensions if set
	GLfloat fSprW, fSprH;
	if(Desc.uiW == DIM_DEFAULT && Sprite != eSPRITE_NONE)	
		fSprW = (GLfloat)aSprites[Sprite].uiWidth;
	else													
		fSprW = (GLfloat)Desc.uiW;

	if(Desc.uiH == DIM_DEFAULT && Sprite != eSPRITE_NONE)	
		fSprH = (GLfloat)aSprites[Sprite].uiHeight;
	else													
		fSprH = (GLfloat)Desc.uiH;

	// Convert to radius
	fSprW *= 0.5f;
	fSprH *= 0.5f;

	GLfloat fAtlasW = (GLfloat)pApp->m_uiAtlasW;
	GLfloat fAtlasH = (GLfloat)pApp->m_uiAtlasH;

	// Calculate texture coordinates based on atlas position and sprite dimensions
	PVRTVec2 vTC((aSprites[Sprite].uiSrcX + (aSprites[Sprite].uiWidth / 2)) / fAtlasW,
				 (aSprites[Sprite].uiSrcY + (aSprites[Sprite].uiHeight / 2)) / fAtlasH);
	
	// Build geometry
	const float fStepDT	= PVRT_TWO_PI / (iNumTris+2);
	const int iNumSteps = (int)(PVRT_TWO_PI / fStepDT);
	float fX, fY;
	float fStep = 0.0f;
	for(int iIdx = 0; iIdx < iNumSteps; fStep += fStepDT, ++iIdx)
	{
		fX = PVRTCOS(fStep);
		fY = PVRTSIN(fStep);
		pVertArray[iIdx].vPos.x = (((vOrigin.x-0.5f)*-2.0f) + fX) * fSprW;
		pVertArray[iIdx].vPos.y = (((vOrigin.y-0.5f)*-2.0f) + fY) * fSprH;
		pVertArray[iIdx].vPos.z = 0.0f;
		pVertArray[iIdx].vPos += vTrans;
		pVertArray[iIdx].vUV.x = vTC.x + ((fX*aSprites[Sprite].uiWidth*0.5f)  / fAtlasW);
		pVertArray[iIdx].vUV.y = vTC.y + ((fY*aSprites[Sprite].uiHeight*0.5f) / fAtlasH);
		pVertArray[iIdx].fTransIndex = (GLfloat)Desc.uiTransformIndex;
		memcpy(pVertArray[iIdx].u8RGBA, Desc.u8RGBA, 4);
	}

	// Index
	GLushort u16Idx = iNumTris+1;
	for(int i = 0; i < iNumTris; ++i, --u16Idx)
	{
		pIndexArray[i*INDICES_PER_TRI+0] = u16IndexStart+0;
		pIndexArray[i*INDICES_PER_TRI+1] = u16IndexStart+u16Idx;
		pIndexArray[i*INDICES_PER_TRI+2] = u16IndexStart+u16Idx-1;
	}

	return iNumTris;
}

/*!****************************************************************************
 @Function		BuildQuadGeometry
 @Return		int			The number of triangles required to build.
 @Input			LayoutDesc& Reference to the layout table for this sprite.
 @Input			PVRTVertex*	Pointer to vertex array.
 @Input			GLushort*	Pointer to index array.
 @Description	
******************************************************************************/
int BuildQuadGeometry(const LayoutDesc& Desc, PVRTVertex* pVertArray, GLushort* pIndexArray, GLushort u16IndexStart, OGLES2ExampleUI* pApp)
{
	const int iNumTris = 2;

	if(pVertArray == NULL || pIndexArray == NULL)
		return iNumTris;

	SpriteDesc* aSprites = pApp->m_aSprites;
	eSprites Sprite		 = Desc.SpriteType;

	PVRTVec3 vTrans;
	memset(&vTrans, 0, sizeof(vTrans));

	// OK to translate the verts if this sprite has no transformation matrix associated.
	if(Desc.uiTransform == eTRANS_NONE)
	{
		if(Desc.bDeviceNorm)
			vTrans = PVRTVec3(pApp->ToDeviceX((PVRTfloat32)Desc.i32X), pApp->ToDeviceY((PVRTfloat32)Desc.i32Y), 0.0f);		// Normalise to device coordinates
		else
			vTrans = PVRTVec3((PVRTfloat32)Desc.i32X, (PVRTfloat32)Desc.i32Y, 0.0f);
	}

	// The origin within the texture atlas.
	PVRTVec2 vOrigin(Desc.fOriginX, Desc.fOriginY);

	// Choose either the original sprite dimensions, or the provided dimensions if set
	GLfloat fSprW, fSprH;
	if(Desc.uiW == DIM_DEFAULT && Sprite != eSPRITE_NONE)	
		fSprW = (GLfloat)aSprites[Sprite].uiWidth;
	else													
		fSprW = (GLfloat)Desc.uiW;

	if(Desc.uiH == DIM_DEFAULT && Sprite != eSPRITE_NONE)	
		fSprH = (GLfloat)aSprites[Sprite].uiHeight;
	else													
		fSprH = (GLfloat)Desc.uiH;

	GLfloat fAtlasW = (GLfloat)pApp->m_uiAtlasW;
	GLfloat fAtlasH = (GLfloat)pApp->m_uiAtlasH;

	// Calculate texture coordinates based on atlas position, sprite dimensions and sprite border
	PVRTVec2 vTTL(0,0), vTBR(1,1);
	if(Sprite != eSPRITE_NONE)
	{
		const SpriteBorder& Border = aSprites[Sprite].Border;
		// TL
		vTTL = PVRTVec2((aSprites[Sprite].uiSrcX + Border.uiBorderL)/ fAtlasW, (aSprites[Sprite].uiSrcY + Border.uiBorderT) / fAtlasH);

		// BR
		vTBR = PVRTVec2(vTTL.x + ((GLfloat)(aSprites[Sprite].uiWidth  - Border.uiBorderR - Border.uiBorderL)/ fAtlasW), 
						vTTL.y + ((GLfloat)(aSprites[Sprite].uiHeight - Border.uiBorderB - Border.uiBorderT)/ fAtlasH));
	}
	else		// eSPRITE_NONE is a special case. We generated a white 4x4 quad on the atlas to allow us to render non-textured quads without another glDraw call or switching shaders programs.
	{
		vTTL = PVRTVec2(pApp->m_vNullTexCoords.x / fAtlasW, pApp->m_vNullTexCoords.y / fAtlasH);
		vTBR = PVRTVec2(vTTL.x + (NULL_QUAD_SAMPLE / fAtlasW), vTTL.y + (NULL_QUAD_SAMPLE / fAtlasH));
	}

	// Determine whether to flip the texture coordinates
	float fTmp;
	if(Desc.u8XFlip)
	{
		fTmp = vTTL.x;
		vTTL.x = vTBR.x;
		vTBR.x = fTmp;
	}
	if(Desc.u8YFlip)
	{
		fTmp = vTTL.y;
		vTTL.y = vTBR.y;
		vTBR.y = fTmp;
	}
	
	// --- Build the geometry
	// Top Left
	pVertArray[0].vPos.x = -(vOrigin.x * fSprW);
	pVertArray[0].vPos.y = -(vOrigin.y * fSprH);
	pVertArray[0].vPos.z = 0.0f;
	pVertArray[0].vPos += vTrans;
	pVertArray[0].vUV.x = vTTL.x;
	pVertArray[0].vUV.y = vTTL.y;
	pVertArray[0].fTransIndex = (GLfloat)Desc.uiTransformIndex;
	memcpy(pVertArray[0].u8RGBA, Desc.u8RGBA, 4);

	// Bottom Left
	pVertArray[1].vPos.x = -(vOrigin.x * fSprW);
	pVertArray[1].vPos.y = (1.0f - vOrigin.y) * fSprH;
	pVertArray[1].vPos.z = 0.0f;
	pVertArray[1].vPos += vTrans;
	pVertArray[1].vUV.x = vTTL.x;
	pVertArray[1].vUV.y = vTBR.y;
	pVertArray[1].fTransIndex = (GLfloat)Desc.uiTransformIndex;
	memcpy(pVertArray[1].u8RGBA, Desc.u8RGBA, 4);

	// Bottom Right
	pVertArray[2].vPos.x = (1.0f - vOrigin.x) * fSprW;
	pVertArray[2].vPos.y = (1.0f - vOrigin.y) * fSprH;;
	pVertArray[2].vPos.z = 0.0f;
	pVertArray[2].vPos += vTrans;
	pVertArray[2].vUV.x = vTBR.x;
	pVertArray[2].vUV.y = vTBR.y;
	pVertArray[2].fTransIndex = (GLfloat)Desc.uiTransformIndex;
	memcpy(pVertArray[2].u8RGBA, Desc.u8RGBA, 4);

	// Top Right
	pVertArray[3].vPos.x = (1.0f - vOrigin.x) * fSprW;
	pVertArray[3].vPos.y = -(vOrigin.y * fSprH);
	pVertArray[3].vPos.z = 0.0f;
	pVertArray[3].vPos += vTrans;
	pVertArray[3].vUV.x = vTBR.x;
	pVertArray[3].vUV.y = vTTL.y;
	pVertArray[3].fTransIndex = (GLfloat)Desc.uiTransformIndex;
	memcpy(pVertArray[3].u8RGBA, Desc.u8RGBA, 4);

	// Index the quad
	pIndexArray[0] = u16IndexStart+0;
	pIndexArray[1] = u16IndexStart+1;
	pIndexArray[2] = u16IndexStart+3;
	pIndexArray[3] = u16IndexStart+1;
	pIndexArray[4] = u16IndexStart+2;
	pIndexArray[5] = u16IndexStart+3;

	return iNumTris;
}

/*!****************************************************************************
 @Function		Area
 @Return
 @Input			int iWidth
 @Input			int iHeight
 @Description	Constructor
******************************************************************************/
Area::Area(int iWidth, int iHeight) : m_iX(0), m_iY(0), m_bFilled(false), m_pRight(NULL), m_pLeft(NULL)
{
	SetSize(iWidth, iHeight);
}

/*!****************************************************************************
 @Function		Area
 @Return		
 @Description	Constructor
******************************************************************************/
Area::Area() : m_iX(0), m_iY(0), m_bFilled(false), m_pRight(NULL), m_pLeft(NULL)
{
	SetSize(0, 0);
}

/*!****************************************************************************
 @Function		Area
 @Return		Area*
 @Input			int iWidth
 @Input			int iHeight
 @Description	Calculates an area where there's sufficient space or returns
				NULL if no space could be found.
******************************************************************************/
Area* Area::Insert(int iWidth, int iHeight)
{
	// If this area has branches below it (i.e. is not a leaf) then traverse those.
	// Check the left branch first.
	if(m_pLeft)
	{
		Area *tempPtr = NULL;
		tempPtr = m_pLeft->Insert(iWidth, iHeight);
		if (tempPtr != NULL) return tempPtr;
	}
	// Now check right
	if(m_pRight) 
		return m_pRight->Insert(iWidth, iHeight);

	if (m_bFilled)			// Already filled!
		return NULL;

	// Too small
	if (m_iSize < iWidth*iHeight || m_iW < iWidth || m_iH < iHeight) 
		return NULL;

	// Just right!
	if (m_iSize == iWidth*iHeight && m_iW == iWidth && m_iH == iHeight)
	{
		m_bFilled = true;
		return this;
	}
	// Too big. Split up.
	if (m_iSize > iWidth*iHeight && m_iW >= iWidth && m_iH >= iHeight)
	{
		// Initialises the children, and sets the left child's coordinates as these don't change.
		m_pLeft  = new Area;
		m_pRight = new Area;
		m_pLeft->m_iX = m_iX;
		m_pLeft->m_iY = m_iY;	
		
		// --- Splits the current area depending on the size and position of the placed texture.
		// Splits vertically if larger free distance across the texture.
		if ((m_iW - iWidth) > (m_iH - iHeight))
		{	
			m_pLeft->m_iW = iWidth;
			m_pLeft->m_iH = m_iH;

			m_pRight->m_iX = m_iX + iWidth;
			m_pRight->m_iY = m_iY;
			m_pRight->m_iW = m_iW - iWidth;
			m_pRight->m_iH = m_iH;
		}
		// Splits horizontally if larger or equal free distance downwards.
		else
		{	
			m_pLeft->m_iW = m_iW;
			m_pLeft->m_iH = iHeight;

			m_pRight->m_iX = m_iX;
			m_pRight->m_iY = m_iY + iHeight;
			m_pRight->m_iW = m_iW;
			m_pRight->m_iH = m_iH - iHeight;
		}

		//Initialises the child members' size attributes.
		m_pLeft->m_iSize  = m_pLeft->m_iH  * m_pLeft->m_iW;
		m_pRight->m_iSize = m_pRight->m_iH * m_pRight->m_iW;

		//Inserts the texture into the left child member.
		return m_pLeft->Insert(iWidth, iHeight);
	}

	//Catch all error return.
	return NULL;
}

/*!****************************************************************************
 @Function		Area
 @Return		bool
 @Description	Delete's the given area.
******************************************************************************/
bool Area::DeleteArea()
{
	if (m_pLeft != NULL)
	{
		if (m_pLeft->m_pLeft != NULL)
		{
			if (!m_pLeft->DeleteArea())  return false;
			if (!m_pRight->DeleteArea()) return false;
		}
	}
	if (m_pRight != NULL)
	{
		if (m_pRight->m_pLeft != NULL)
		{
			if (!m_pLeft->DeleteArea())  return false;
			if (!m_pRight->DeleteArea()) return false;
		}
	}
	delete m_pRight;
	m_pRight=NULL;
	delete m_pLeft;
	m_pLeft=NULL;

	return true;
}

/*!****************************************************************************
 @Function		SetSize
 @Return		void
 @Input			int iWidth
 @Input			int iHeight
 @Description	Sets the size properties of the area.
******************************************************************************/
void Area::SetSize(int iWidth, int iHeight)
{
	m_iW = iWidth;
	m_iH = iHeight;
	m_iSize = iWidth * iHeight;
}

/*!****************************************************************************
 @Function		GetX
 @Return		int
 @Description	Returns the X position of the area.
******************************************************************************/
int Area::GetX() 
{
	return m_iX; 
}

/*!****************************************************************************
 @Function		GetY
 @Return		int
 @Description	Returns the Y position of the area.
******************************************************************************/
int Area::GetY() 
{ 
	return m_iY; 
}

/******************************************************************************
 End of file (OGLES2ExampleUI.cpp)
******************************************************************************/

