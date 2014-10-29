/******************************************************************************

 @File         OGLESIntroducingPrint3D.cpp

 @Title        Introducing Print3D

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independent

 @Description  Shows how to use the Print3D class to draw Unicode-compliant text
               in 3D.

******************************************************************************/
#include "PVRShell.h"
#include "OGLESTools.h"

/******************************************************************************
 Content file names
******************************************************************************/
// PVR font files
const char c_szTextFile[]		= "arial_36.pvr";
const char c_szIntroFile[]		= "starjout_60.pvr";
const char c_szResTextFile[]	= "Text.txt";

/******************************************************************************
 Defines
******************************************************************************/
#define INTRO_TIME	4000
#define INTRO_FADE_TIME 1000
#define TITLE_TIME 4000
#define TITLE_FADE_TIME 500

#define TEXT_START_Y -650.0f
#define TEXT_END_Y 1300.0f
#define TEXT_FADE_START 300.0f
#define TEXT_FADE_END 500.0f

/******************************************************************************
 Constants
******************************************************************************/
const float c_fTargetFPS = 1.0f/60.0f;

enum eTitleLanguage
{
	eLang_English,
	eLang_German,
	eLang_Norwegian,
	eLang_Bulgarian,

	eLang_Size
};

enum eFontSize
{
	eFontSize_36,
	eFontSize_46,
	eFontSize_56,
	
	eFontSize_Size
};

const wchar_t* c_pwzTitles[eLang_Size] = 
{
	L"IntroducingPrint3D",
	L"Einf\u00FChrungPrint3D",
	L"Innf\u00F8ringPrint3D",
	L"\u0432\u044A\u0432\u0435\u0436\u0434\u0430\u043D\u0435Print3D",
};

const char* c_szTitleFont[eFontSize_Size] = 
{
	"title_36.pvr",
	"title_46.pvr",
	"title_56.pvr",
};

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLESIntroducingPrint3D : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D			m_Print3D;
	CPVRTPrint3D			m_CentralText;
	CPVRTPrint3D			m_IntroText;
	CPVRTPrint3D			m_TitleText;

	unsigned long			m_ulTimePrev;
	unsigned long			m_ulStartTime;

	unsigned long			m_ulPrevFrameT;
	float					m_fTextOffset;

	GLuint					m_uiStarTex;
	CPVRTBackground			m_BG;
	CPVRTArray<PVRTuint8*>	m_TextLines;

	eTitleLanguage			m_eTitleLang;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	void GenerateBackgroundTexture(unsigned int uiScreenWidth, unsigned int uiScreenHeight);

	void RenderTitle(float fFadeAmount);
	void RenderText();
};

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
bool OGLESIntroducingPrint3D::InitApplication()
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

	
	/* 
		Get and set the load/release functions for loading external files.
		In the majority of cases the PVRShell will return NULL function pointers implying that
		nothing special is required to load external files.
	*/
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	/*
		Because the C++ standard states that only ASCII characters are valid in compiled code,
		we are instead using an external resource file which contains all of the text to be
		rendered. This allows complete control over the encoding of the resource file which
		in this case is encoded as UTF-8.		
	*/
	CPVRTResourceFile TextResource(c_szResTextFile);
	if(!TextResource.IsOpen())
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load text resource file!");
		return false;
	}

	/*
		The following code simply pulls out each line in the resource file and adds it
		to an array so we can render each line separately.
	*/
	const PVRTuint8* pStart = (PVRTuint8*)TextResource.DataPtr();
	const PVRTuint8* pC = pStart;	
	while(*pC && ((unsigned int)(pC - pStart) < TextResource.Size()))
	{
		// Read how many characters on this line.
		const PVRTuint8* pLStart = pC;
		while(*pC && *pC != '\n') pC++;

		unsigned int uiDataLen = (unsigned int) (pC - pLStart + 1);
		PVRTuint8* pLineData = new PVRTuint8[uiDataLen + 1];
		memset(pLineData, 0, uiDataLen + 1);
		pC = pLStart;

		unsigned int uiIndex;
		for(uiIndex = 0; uiIndex < uiDataLen; ++uiIndex)
			pLineData[uiIndex] = pC[uiIndex];
	
		pC += uiDataLen;
		m_TextLines.Append(pLineData);
	}

	m_fTextOffset  = TEXT_START_Y;
	m_ulPrevFrameT = 0;
	m_eTitleLang   = eLang_English; 

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
bool OGLESIntroducingPrint3D::QuitApplication()
{
	for(unsigned int uiIndex = 0; uiIndex < m_TextLines.GetSize(); ++uiIndex)
	{
		delete [] m_TextLines[uiIndex];
	}

	return true;
}

/*!***************************************************************************
@Function		GenerateBackgroundTexture
@Input			uiScreenWidth
@Input			uiScreenHeight
@Description	Generates a simple background texture procedurally.
*****************************************************************************/
void OGLESIntroducingPrint3D::GenerateBackgroundTexture(unsigned int uiScreenWidth, unsigned int uiScreenHeight)
{
	// Generate star texture
	unsigned int uiStarW = PVRTGetPOTHigher(uiScreenWidth, 1);
	unsigned int uiStarH = PVRTGetPOTHigher(uiScreenHeight, 1);

	glGenTextures(1, &m_uiStarTex);
	glBindTexture(GL_TEXTURE_2D, m_uiStarTex);
	PVRTuint8* pTexData = new PVRTuint8[uiStarW*uiStarH];
	memset(pTexData, 0, uiStarW*uiStarH);
	for (unsigned int uiY = 0; uiY < uiStarH; uiY++)
	{
		for (unsigned int uiX = 0; uiX < uiStarW; uiX++)
		{
			unsigned int uiIdx = (uiY*uiStarW+uiX);
			if(rand() % 200 == 1)
			{
				pTexData[uiIdx] = rand() % 255;
			}
		}
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, uiStarW, uiStarH, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pTexData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	delete [] pTexData;

	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);
	m_BG.Init(NULL, bRotate, NULL);
}

/*!****************************************************************************
 @Function		InitView
 @Return		bool		true if no error occurred
 @Description	Code in InitView() will be called by PVRShell upon
				initialization or after a change in the rendering context.
				Used to initialize variables that are dependent on the rendering
				context (e.g. textures, vertex buffers, etc.)
******************************************************************************/
bool OGLESIntroducingPrint3D::InitView()
{
	/*
		Initialize the textures used by Print3D.
		To properly display text, Print3D needs to know the viewport dimensions
		and whether the text should be rotated. We get the dimensions using the
		shell function PVRShellGet(prefWidth/prefHeight). We can also get the
		rotate parameter by checking prefIsRotated and prefFullScreen.
	*/
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	// Retrieve screen metrics from PVRShell
	unsigned int uiWidth  = PVRShellGet(prefWidth);
	unsigned int uiHeight = PVRShellGet(prefHeight);

	/*
		The fonts are loaded here using a PVRTool's ResourceFile wrapper. However, 
		it is possible to load the textures in any way that provides access to a pointer 
		to memory, and the size of the file.
	*/
	CPVRTResourceFile CustomFont(c_szTextFile);
	if(!CustomFont.IsOpen())
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load font file!");
		return false;
	}

	CPVRTResourceFile IntroFont(c_szIntroFile);
	if(!IntroFont.IsOpen())
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load font file!");
		return false;
	}
	
	// Determine which size title font to use.
	unsigned int uiMinScreenLen = PVRT_MIN(uiWidth, uiHeight);
	const char* pTitleFontFile = NULL;
	if(uiMinScreenLen >= 720)
		pTitleFontFile = c_szTitleFont[eFontSize_56];
	else if(uiMinScreenLen >= 640)
		pTitleFontFile = c_szTitleFont[eFontSize_46];
	else
		pTitleFontFile = c_szTitleFont[eFontSize_36];

	CPVRTResourceFile TitleFont(pTitleFontFile);
	if(!TitleFont.IsOpen())
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load font file!");
		return false;
	}

	/*
		The first version of Print3D.SetTextures() presented here sets up m_Print3D 
		with default, built-in textures. The overloaded functions provided allow user-defined 
		textures to be loaded instead. Please refer to PVRTexTool's user manual for 
		instructions on generating fonts files.
	*/
	m_Print3D.SetTextures(NULL, uiWidth, uiHeight, bRotate);
	m_CentralText.SetTextures(NULL, CustomFont.DataPtr(), uiWidth, uiHeight, bRotate);
	m_IntroText.SetTextures(NULL, IntroFont.DataPtr(), uiWidth, uiHeight, bRotate);
	m_TitleText.SetTextures(NULL, TitleFont.DataPtr(), uiWidth, uiHeight, bRotate);

	// Sets the clear color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate background texture
	GenerateBackgroundTexture(uiWidth, uiHeight);

	m_ulStartTime = PVRShellGetTime();

	// Enable culling
	glEnable(GL_CULL_FACE);
	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occurred
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLESIntroducingPrint3D::ReleaseView()
{
	// Release Print3D Textures
	m_Print3D.ReleaseTextures();
	m_CentralText.ReleaseTextures();
	m_IntroText.ReleaseTextures();
	m_TitleText.ReleaseTextures();
	m_BG.Destroy();

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
bool OGLESIntroducingPrint3D::RenderScene()
{
	// Clears the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	unsigned long ulCurrentTime = PVRShellGetTime() - m_ulStartTime;

	// Draw star background
	m_BG.Draw(m_uiStarTex);
	 
	// Render the 'Introducing Print3D' title for the first n seconds.
	if(ulCurrentTime < INTRO_TIME)
	{
		float fFadeAmount = 1.0f;
		
		// Fade in
		if(ulCurrentTime < INTRO_FADE_TIME)	
		{
			fFadeAmount = ulCurrentTime / (float)INTRO_FADE_TIME;
		}
		// Fade out
		else if(ulCurrentTime > INTRO_TIME - INTRO_FADE_TIME)
		{
			fFadeAmount = 1.0f - ((ulCurrentTime - (INTRO_TIME - INTRO_FADE_TIME)) / (float)INTRO_FADE_TIME);
		}

		RenderTitle(fFadeAmount);		
	}
	// Render the 3D text.
	else
	{
		RenderText();
	}

	/*
		Here we are passing in a wide-character string to Print3D function. This allows
		Unicode to be compiled in to string-constants, which this code snippet
		demonstrates.
		Because we are not setting a projection or a model-view matrix the default projection 
		matrix is used.
	*/
	unsigned int uiTitleLang = (unsigned int) ((ulCurrentTime / 1000) / (TITLE_TIME / 1000)) % eLang_Size;
	unsigned int uiNextLang  = (uiTitleLang + 1) % eLang_Size;
	unsigned int ulModTime   = (unsigned int) ulCurrentTime % TITLE_TIME;
	float fTitlePerc = 1.0f;
	float fNextPerc  = 0.0f;
	if(ulModTime > TITLE_TIME - TITLE_FADE_TIME)
	{
		fTitlePerc = 1.0f - ((ulModTime - (INTRO_TIME - INTRO_FADE_TIME)) / (float)INTRO_FADE_TIME);
		fNextPerc = 1.0f - fTitlePerc;
	}
	unsigned int uiTitleCol = (((unsigned int)(fTitlePerc * 255)) << 24) | 0xFFFFFF;
	unsigned int uiNextCol  = (((unsigned int)(fNextPerc  * 255)) << 24) | 0xFFFFFF;
	m_TitleText.Print3D(0, 0, 1, uiTitleCol, c_pwzTitles[uiTitleLang]);
	m_TitleText.Print3D(0, 0, 1, uiNextCol, c_pwzTitles[uiNextLang]);
	m_TitleText.Flush();
		
	/*
		DisplayDefaultTitle() writes a title and description text on the top left of the screen.
		It can also display the PVR logo (ePVRTPrint3DLogoPVR), the IMG logo (ePVRTPrint3DLogoIMG) or both (ePVRTPrint3DLogoPVR | ePVRTPrint3DLogoIMG)
		which is what we are using the function for here.
		Set this last parameter to NULL not to display the logos.
		Passing NULL for the first two parameters will not display any text.
	*/
	m_Print3D.DisplayDefaultTitle(NULL, NULL, ePVRTPrint3DSDKLogo);

	// Tells Print3D to do all the pending text rendering now
	m_Print3D.Flush();
	
	return true;
}

/*!***************************************************************************
@Function		RenderTitle
@Input			fFadeAmount
@Description	Draws the title text.
*****************************************************************************/
void OGLESIntroducingPrint3D::RenderTitle(float fFadeAmount)
{
	unsigned int uiCol = (((unsigned int)(fFadeAmount * 255)) << 24) | 0x00FFFF;

	float fW = PVRShellGet(prefWidth) * 0.5f;
	float fH = PVRShellGet(prefHeight) * 0.5f;
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	/*
		Print3D can optionally be provided with user-defined projection and modelview matrices
		which allow custom layout of text. Here we are just providing a projection matrix
		so that text can be placed in viewport coordinates, rather than the default, more 
		abstract coordinate system of 0.0-100.0.
	*/
	PVRTMat4 mProjection = PVRTMat4::Ortho(-fW, fH, fW, -fH, -1.0f, 1.0f, PVRTMat4::OGL, bRotate);
	m_IntroText.SetProjection(mProjection);

	/*
		Using the MeasureText() method provided by Print3D, we can determine the bounding-box
		size of a string of text. This can be useful for justify text centrally, as we are
		doing here.
	*/
	float fLine1W = 0.0f;
	float fLine2W = 0.0f;
	m_IntroText.MeasureText(&fLine1W, NULL, 1.0f, "introducing");
	m_IntroText.MeasureText(&fLine2W, NULL, 1.0f, "print3d");

	/*
		Display some text.
		Print3D() function allows to draw text anywhere on the screen using any colour.
		Param 1: Position of the text along X
		Param 2: Position of the text along Y
		Param 3: Scale of the text
		Param 4: Colour of the text (0xAABBGGRR format)
		Param 5: Formatted string (uses the same syntax as printf)
		...
	*/
	m_IntroText.Print3D(-fLine1W*0.5f, 50.0f, 1.0f, uiCol, "introducing");
	m_IntroText.Print3D(-fLine2W*0.5f, 0.0f,   1.0f, uiCol, "print3d");

	// Tells Print3D to do all the pending text rendering now
	m_IntroText.Flush();
}

/*!***************************************************************************
@Function		RenderText
@Description	Draws the 3D text and scrolls in to the screen.
*****************************************************************************/
void OGLESIntroducingPrint3D::RenderText()
{
	float fAspect = (float)PVRShellGet(prefWidth) / (float)PVRShellGet(prefHeight);
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	// Calculate the frame delta.
	unsigned long ulNow = PVRShellGetTime();
	if(m_ulPrevFrameT == 0)	m_ulPrevFrameT = ulNow;
	float fDT = (ulNow - m_ulPrevFrameT) * 0.001f;
	m_ulPrevFrameT = ulNow;
		
	// Calculate the FPS scale.
	float fFPSScale = fDT / c_fTargetFPS;
		
	// Move the text. Progressively speed up.
	float fSpeedInc = 0.0f;
	if(m_fTextOffset > 0.0f)
		fSpeedInc = m_fTextOffset / TEXT_END_Y;
	m_fTextOffset += (0.75f + (1.0f * fSpeedInc)) * fFPSScale;
	if(m_fTextOffset > TEXT_END_Y)
		m_fTextOffset = TEXT_START_Y;

	PVRTMat4 mProjection = PVRTMat4::PerspectiveFovRH(0.7f, fAspect, 1.0f, 2000.0f, PVRTMat4::OGL, bRotate);
	PVRTMat4 mCamera     = PVRTMat4::LookAtRH(PVRTVec3(0.0f, -900.0f, 700.0f), PVRTVec3(0.0f,-200.0f,0.0f), PVRTVec3(0.0f,1.0f,0.0f));
	PVRTMat4 mTrans		 = PVRTMat4::Translation(PVRTVec3(0.0f, m_fTextOffset, 0.0f));
	PVRTMat4 mModelView  = mCamera * mTrans;
	float fStrWidth = 0.0f;

	/*
		Print3D can optionally be provided with user-defined projection and model-view matrices
		which allow custom layout of text. Here we are proving both a projection and model-view
		matrix. The projection matrix specified here uses perspective projection which will
		provide the 3D effect. The model-view matrix positions the the text in world space
		providing the 'camera' position and the scrolling of the text.
	*/
	m_CentralText.SetProjection(mProjection);
	m_CentralText.SetModelView(mModelView);

	/*
		The previous method (RenderTitle()) explains the following functions in more detail
		however put simply, we are looping the entire array of loaded text which is encoded
		in UTF-8. Print3D batches this internally and the call to Flush() will render the
		text to the frame buffer. We are also fading out the text over a certain distance.
	*/
	float fPos, fFade;
	unsigned int uiCol;
	for(unsigned int uiIndex = 0; uiIndex < m_TextLines.GetSize(); ++uiIndex)
	{
		fPos = (m_fTextOffset - (uiIndex * 36.0f));
		fFade = 1.0f;
		if(fPos > TEXT_FADE_START)
		{
			fFade = PVRTClamp(1.0f - ((fPos - TEXT_FADE_START) / (TEXT_FADE_END - TEXT_FADE_START)), 0.0f, 1.0f);
		}

		uiCol = (((unsigned int)(fFade * 255)) << 24) | 0x00FFFF;

		m_CentralText.MeasureText(&fStrWidth, NULL, 1.0f, (const char*)m_TextLines[uiIndex]);
		m_CentralText.Print3D(-(fStrWidth*0.5f), -(uiIndex * 36.0f), 1.0f, uiCol, (const char*)m_TextLines[uiIndex]);
	}

	m_CentralText.Flush();
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
	return new OGLESIntroducingPrint3D();
}

/******************************************************************************
 End of file (OGLESIntroducingPrint3D.cpp)
******************************************************************************/

