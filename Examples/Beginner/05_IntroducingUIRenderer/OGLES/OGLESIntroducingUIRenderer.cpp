/*!******************************************************************************************************************
\File         OGLESIntroducingUIRenderer.cpp
\Title        Introducing uiRenderer
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief  Shows how to use the UIRenderer class to draw ASCII/UTF-8 or wide-charUnicode-compliant text in 3D.
*********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVREngineUtils/PVREngineUtils.h"

// PVR font files
const char CentralTextFontFile[]    = "arial_36.pvr";
const char CentralTitleFontFile[]   = "starjout_60.pvr";
const char CentralTextFile[]        = "Text.txt";

namespace FontSize {
enum Enum
{
	n_36,
	n_46,
	n_56,
	Count
};
}

const char* SubTitleFontFiles[FontSize::Count] =
{
	"title_36.pvr",
	"title_46.pvr",
	"title_56.pvr",
};

const pvr::uint32 IntroTime = 4000;
const pvr::uint32 IntroFadeTime = 1000;
const pvr::uint32 TitleTime = 4000;
const pvr::uint32 TitleFadeTime = 1000;
const pvr::uint32 TextFadeStart = 300;
const pvr::uint32 TextFadeEnd = 500;

namespace Language {
enum Enum
{
	English,
	German,
	Norwegian,
	Bulgarian,
	Count
};
}

const wchar_t* Titles[Language::Count] =
{
	L"IntroducingUIRenderer",
	L"Einf\u00FChrungUIRenderer",
	L"Innf\u00F8ringUIRenderer",
	L"\u0432\u044A\u0432\u0435\u0436\u0434\u0430\u043D\u0435UIRenderer",
};

/*!******************************************************************************************************************
Class implementing the pvr::Shell functions.
*********************************************************************************************************************/
class OGLESIntroducingUIRenderer : public pvr::Shell
{
	// UIRenderer class used to display text
	pvr::ui::UIRenderer uiRenderer;
	pvr::ui::MatrixGroup centralTextGroup;
	pvr::ui::Text centralTitleLine1;
	pvr::ui::Text centralTitleLine2;
	pvr::ui::Text titleText1;
	pvr::ui::Text titleText2;
	std::vector<pvr::ui::Text> centralTextLines;
	pvr::ui::Image background;
	pvr::utils::AssetStore assetStore;

	glm::mat4 mvp;

	pvr::float32 textOffset;
	std::vector<char>	text;
	std::vector<const char*> textLines;
	Language::Enum _titleLang;
	pvr::int32 textStartY, textEndY;
	pvr::api::Fbo onScreenFbo;
	pvr::api::SecondaryCommandBuffer commandBufferWithIntro;
	pvr::api::SecondaryCommandBuffer commandBufferWithText;
	pvr::api::SecondaryCommandBuffer commandBufferSubtitle;
	pvr::api::CommandBuffer primaryCommandBuffer;

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void generateBackgroundTexture(pvr::uint32 screenWidth, pvr::uint32 screenHeight);

	void updateCentralTitle(pvr::uint64 currentTime);
	void updateSubTitle(pvr::uint64 currentTime);
	void updateCentralText(pvr::uint64 currentTime);
	void recordCommandBuffers();
};

/*!******************************************************************************************************************
\brief	Record the rendering commands
*********************************************************************************************************************/
void OGLESIntroducingUIRenderer::recordCommandBuffers()
{
	commandBufferWithIntro->beginRecording(onScreenFbo->getRenderPass());

	uiRenderer.beginRendering(commandBufferWithIntro);
	background->render();

	//This is the difference
	centralTitleLine1->render();
	centralTitleLine2->render();

	uiRenderer.getSdkLogo()->render();
	// Tells uiRenderer to do all the pending text rendering now
	uiRenderer.endRendering();

	commandBufferWithIntro->endRecording();

	commandBufferWithText->beginRecording(onScreenFbo->getRenderPass());
	uiRenderer.beginRendering(commandBufferWithText);
	background->render();
	centralTextGroup->render();
	uiRenderer.getSdkLogo()->render();
	//// Tells uiRenderer to do all the pending text rendering now
	uiRenderer.endRendering();

	commandBufferWithText->endRecording();
}

/*!******************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in initApplication() will be called by Shell once per run, before the rendering context is created.
		Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
		If the rendering context is lost, initApplication() will not be called again.
*********************************************************************************************************************/
pvr::Result OGLESIntroducingUIRenderer::initApplication()
{
	// Because the C++ standard states that only ASCII characters are valid in compiled code,
	// we are instead using an external resource file which contains all of the text to be
	// rendered. This allows complete control over the encoding of the resource file which
	// in this case is encoded as UTF-8.
	pvr::Stream::ptr_type textStream = getAssetStream(CentralTextFile);
	if (!textStream.get())
	{
		this->setExitMessage("ERROR: Failed to load text resource file!");
		return pvr::Result::UnknownError;
	}

	// The following code simply pulls out each line in the resource file and adds it
	// to an array so we can render each line separately. ReadIntoCharBuffer null-terminates the string
	// so it is safe to check for null character.
	textStream->readIntoCharBuffer(text);
	size_t current = 0;
	while (current < text.size())
	{
		const char* start = text.data() + current;

		textLines.push_back(start);
		while (current < text.size() && text[current] != '\0' && text[current] != '\n' && text[current] != '\r') { ++current; }

		if (current < text.size() && (text[current] == '\r')) {	text[current++] = '\0';	}
		//null-term the strings!!!
		if (current < text.size() && (text[current] == '\n' || text[current] == '\0')) {	text[current++] = '\0';	}
	}

	_titleLang = Language::English;
	return pvr::Result::Success;
}

/*!******************************************************************************************************************
\return		Return pvr::Result::Success	if no error occurred
\brief		Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
			If the rendering context is lost, quitApplication() will not be called.
*********************************************************************************************************************/
pvr::Result OGLESIntroducingUIRenderer::quitApplication() {	return pvr::Result::Success; }

/*!******************************************************************************************************************
\brief	Generates a simple background texture procedurally.
\param[in]	screenWidth screen dimension's width
\param[in]	screenHeight screen dimension's height
*********************************************************************************************************************/
void OGLESIntroducingUIRenderer::generateBackgroundTexture(pvr::uint32 screenWidth, pvr::uint32 screenHeight)
{
	// Generate star texture
	pvr::uint32 width = pvr::math::makePowerOfTwoHigh(screenWidth);
	pvr::uint32 height = pvr::math::makePowerOfTwoHigh(screenHeight);

	pvr::TextureHeader::Header hd;
	hd.channelType = pvr::VariableType::UnsignedByteNorm;
	hd.pixelFormat = pvr::GeneratePixelType1<'l', 8>::ID;
	hd.colorSpace = pvr::types::ColorSpace::lRGB;
	hd.width = width;
	hd.height = height;
	pvr::Texture myTexture(hd);
	pvr::byte* textureData = myTexture.getDataPointer();
	memset(textureData, 0, width * height);
	for (pvr::uint32 j = 0; j < height; ++j)
	{
		for (pvr::uint32 i = 0; i < width; ++i)
		{
			if (!(rand() % 200))
			{
				int brightness = rand() % 255;
				textureData[width * j + i] = glm::clamp(textureData[width * j + i] + brightness, 0, 255);
			}
		}
	}
	background = uiRenderer.createImage(myTexture);
}


/*!******************************************************************************************************************
\brief Load font from the resource used for this example
\param[in] streamManager asset provider
\param[in] filename name of the font file
\param[in] uirenderer ui::Font creator
\param[out] font returned font
\return Return pvr::Result::Success if no error occurred.
*********************************************************************************************************************/
inline pvr::Result loadFontFromResources(pvr::Shell& streamManager, const char* filename,
    pvr::ui::UIRenderer& uirenderer, pvr::ui::Font& font)
{
	// the AssetStore is unsuitable for loading the font, because it does not keep the actual texture data that we need.
	// The assetStore immediately releases the texture data as soon as it creates the API objects and the texture header.
	// Hence we use texture load.
	pvr::Result res;
	pvr::Stream::ptr_type fontFile = streamManager.getAssetStream(filename);
	if (!fontFile.get() || !fontFile->isReadable())	{	return pvr::Result::NotFound;  	}
	pvr::Texture tmpTexture;
	if ((res = pvr::assets::textureLoad(fontFile, pvr::getTextureFormatFromFilename(filename), tmpTexture)) != pvr::Result::Success)
	{
		return res;
	}
	font = uirenderer.createFont(tmpTexture);
	return res;
}

/*!******************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
		Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
*********************************************************************************************************************/
pvr::Result OGLESIntroducingUIRenderer::initView()
{
	assetStore.init(*this);
	onScreenFbo = getGraphicsContext()->createOnScreenFbo(0);
	uiRenderer.init(onScreenFbo->getRenderPass(), 0);

	commandBufferWithIntro = getGraphicsContext()->createSecondaryCommandBufferOnDefaultPool();
	commandBufferWithText = getGraphicsContext()->createSecondaryCommandBufferOnDefaultPool();
	commandBufferSubtitle = getGraphicsContext()->createSecondaryCommandBufferOnDefaultPool();
	primaryCommandBuffer = getGraphicsContext()->createCommandBufferOnDefaultPool();

	// Create RenderPass

	// The fonts are loaded here using a PVRTool's ResourceFile wrapper. However,
	// it is possible to load the textures in any way that provides access to a pointer
	// to memory, and the size of the file.
	pvr::ui::Font subTitleFont, centralTitleFont, centralTextFont;
	{
		pvr::Result res;
		if ((res = loadFontFromResources(*this, CentralTitleFontFile, uiRenderer, centralTitleFont)) != pvr::Result::Success)
		{
			this->setExitMessage("ERROR: Failed to create font from file %s", CentralTitleFontFile);
			return res;
		}
		if ((res = loadFontFromResources(*this, CentralTextFontFile, uiRenderer, centralTextFont)) != pvr::Result::Success)
		{
			this->setExitMessage("ERROR: Failed to create font from file %s", CentralTextFontFile);
			return res;
		}

		// Determine which size title font to use.
		pvr::uint32 screenShortDimension = std::min(getWidth(), getHeight());
		const char* titleFontFileName = NULL;
		if (screenShortDimension >= 720)
		{
			titleFontFileName = SubTitleFontFiles[FontSize::n_56];
		}
		else if (screenShortDimension >= 640)
		{
			titleFontFileName = SubTitleFontFiles[FontSize::n_46];
		}
		else
		{
			titleFontFileName = SubTitleFontFiles[FontSize::n_36];
		}

		if ((res = loadFontFromResources(*this, titleFontFileName, uiRenderer, subTitleFont)) != pvr::Result::Success)
		{
			this->setExitMessage("ERROR: Failed to create font from file %s", CentralTitleFontFile);
			return res;
		}
	}

	centralTextGroup = uiRenderer.createMatrixGroup();
	titleText1 = uiRenderer.createText(subTitleFont);
	titleText2 = uiRenderer.createText(subTitleFont);
	titleText1->setAnchor(pvr::ui::Anchor::TopLeft, -.98f, .98f);
	titleText2->setAnchor(pvr::ui::Anchor::TopLeft, -.98f, .98f);

	for (std::vector<const char*>::const_iterator it = textLines.begin(); it != textLines.end(); ++it)
	{
		centralTextLines.push_back(uiRenderer.createText(*it, centralTextFont));
		centralTextGroup->add(centralTextLines.back());
	}

	centralTitleLine1 = uiRenderer.createText("introducing", centralTitleFont);
	centralTitleLine2 = uiRenderer.createText("uirenderer", centralTitleFont);

	centralTitleLine1->setAnchor(pvr::ui::Anchor::BottomCenter, glm::vec2(.0f, .0f));
	centralTitleLine2->setAnchor(pvr::ui::Anchor::TopCenter, glm::vec2(.0f, .0f));

	// Generate background texture
	generateBackgroundTexture(getWidth(), getHeight());
	textStartY = (pvr::int32)(-uiRenderer.getRenderingDimY() - centralTextGroup->getDimensions().y);
	pvr::float32 linesSize = centralTextLines.size() * centralTextLines[0]->getDimensions().y;
	textEndY = (pvr::int32)(uiRenderer.getRenderingDimY() + linesSize * .5f);
	textOffset = (pvr::float32)textStartY;
	recordCommandBuffers();
	return pvr::Result::Success;
}

/*!******************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
*********************************************************************************************************************/
pvr::Result OGLESIntroducingUIRenderer::releaseView()
{
	// Release uiRenderer Textures
	uiRenderer.release();
	centralTextLines.clear();
	centralTitleLine1.reset();
	centralTitleLine2.reset();
	titleText1.reset();
	titleText2.reset();
	assetStore.releaseAll();
	centralTextGroup.reset();
	background.reset();
	commandBufferWithIntro.reset();
	commandBufferWithText.reset();
	commandBufferSubtitle.reset();
	onScreenFbo.reset();
	primaryCommandBuffer.reset();

	return pvr::Result::Success;
}

/*!******************************************************************************************************************
\brief	Main rendering loop function of the program. The shell will call this function every frame.
\return	Result::Success if no error occurred
*********************************************************************************************************************/
pvr::Result OGLESIntroducingUIRenderer::renderFrame()
{
	// Clears the color and depth buffer
	pvr::uint64 currentTime = this->getTime() - this->getTimeAtInitApplication();

	updateSubTitle(currentTime);

	primaryCommandBuffer->beginRecording();
	primaryCommandBuffer->beginRenderPass(onScreenFbo, pvr::Rectanglei(0, 0, getWidth(), getHeight()), false);

	// Render the 'Introducing uiRenderer' title for the first n seconds.
	if (currentTime < IntroTime)
	{
		updateCentralTitle(currentTime);
		primaryCommandBuffer->enqueueSecondaryCmds(commandBufferWithIntro);
	}
	//Render the 3D text.
	else
	{
		updateCentralText(currentTime);
		primaryCommandBuffer->enqueueSecondaryCmds(commandBufferWithText);
	}
	commandBufferSubtitle->beginRecording(onScreenFbo->getRenderPass());
	uiRenderer.beginRendering(commandBufferSubtitle);
	titleText1->render();
	titleText2->render();
	uiRenderer.endRendering();
	commandBufferSubtitle->endRecording();

	primaryCommandBuffer->enqueueSecondaryCmds(commandBufferSubtitle);
	primaryCommandBuffer->endRenderPass();
	primaryCommandBuffer->endRecording();
	primaryCommandBuffer->submit();
	return pvr::Result::Success;
}

/*!******************************************************************************************************************
\brief	Update the description sprite
\param	currentTime Current Time
*********************************************************************************************************************/
void OGLESIntroducingUIRenderer::updateSubTitle(pvr::uint64 currentTime)
{
	// Fade effect
	static int prevLang = (int) - 1;
	pvr::uint32 titleLang = (pvr::uint32)((currentTime / 1000) / (TitleTime / 1000)) % Language::Count;

	pvr::uint32 nextLang = (titleLang + 1) % Language::Count;
	pvr::uint32 modTime = (pvr::uint32)currentTime % TitleTime;
	float titlePerc = 1.0f;
	float nextPerc = 0.0f;
	if (modTime > TitleTime - TitleFadeTime)
	{
		titlePerc = 1.0f - ((modTime - (TitleTime - TitleFadeTime)) / (float)TitleFadeTime);
		nextPerc = 1.0f - titlePerc;
	}
	pvr::uint32 titleCol = (((pvr::uint32)(titlePerc * 255)) << 24) | 0xFFFFFF;
	pvr::uint32 nextCol = (((pvr::uint32)(nextPerc * 255)) << 24) | 0xFFFFFF;

	// Here we are passing in a wide-character string to uiRenderer function. This allows
	// Unicode to be compiled in to string-constants, which this code snippet demonstrates.
	// Because we are not setting a projection or a model-view matrix the default projection
	// matrix is used.
	if (titleLang != prevLang)
	{
		titleText1->setText(Titles[titleLang]);
		titleText2->setText(Titles[nextLang]);
		prevLang = titleLang;
	}
	titleText1->setColor(titleCol);
	titleText2->setColor(nextCol);
	titleText1->commitUpdates();
	titleText2->commitUpdates();
}

/*!******************************************************************************************************************
\brief	Draws the title text.
\param[in]	fadeAmount
*********************************************************************************************************************/
void OGLESIntroducingUIRenderer::updateCentralTitle(pvr::uint64 currentTime)
{

	// Using the MeasureText() method provided by uiRenderer, we can determine the bounding-box
	// size of a string of text. This can be useful for justify text centrally, as we are
	// doing here.
	pvr::float32 fadeAmount = 1.0f;

	// Fade in
	if (currentTime < IntroFadeTime)
	{
		fadeAmount = currentTime / (float)IntroFadeTime;
	}
	// Fade out
	else if (currentTime > IntroTime - IntroFadeTime)
	{
		fadeAmount = 1.0f - ((currentTime - (IntroTime - IntroFadeTime)) / (float)IntroFadeTime);
	}
	//Editing the text's alpha based on the fade amount.
	centralTitleLine1->setColor(1.f, 1.f, 0.f, fadeAmount);
	centralTitleLine2->setColor(1.f, 1.f, 0.f, fadeAmount);
	centralTitleLine1->commitUpdates();
	centralTitleLine2->commitUpdates();
}

/*!******************************************************************************************************************
\brief	Draws the 3D text and scrolls in to the screen.
*********************************************************************************************************************/
void OGLESIntroducingUIRenderer::updateCentralText(pvr::uint64 currentTime)
{
	const glm::mat4 mProjection = glm::perspective(0.7f, float(uiRenderer.getRenderingDimX()) / float(uiRenderer.getRenderingDimY()), 1.0f, 2000.0f);

	const glm::mat4 mCamera = glm::lookAt(glm::vec3(uiRenderer.getRenderingDimX() * .5f, -uiRenderer.getRenderingDimY(), 700.0f),
	                                      glm::vec3(uiRenderer.getRenderingDimX() * .5f, 0, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	mvp = mProjection * mCamera;

	float lineSpacingNDC = 1.6f * centralTextLines[0]->getFont()->getFontLineSpacing() / (float)getHeight();

	// Calculate the FPS scale.
	float fFPSScale = float(getFrameTime()) * 60 / 1000;

	// Move the text. Progressively speed up.
	float fSpeedInc = 0.0f;
	if (textOffset > 0.0f) { fSpeedInc = textOffset / textEndY; }
	textOffset += (0.75f + (1.0f * fSpeedInc)) * fFPSScale;
	if (textOffset > (pvr::float32)textEndY) { textOffset = (pvr::float32)textStartY; }

	glm::mat4 trans = glm::translate(glm::vec3(0.0f, textOffset, 0.0f));

	// uiRenderer can optionally be provided with user-defined projection and model-view matrices
	// which allow custom layout of text. Here we are proving both a projection and model-view
	// matrix. The projection matrix specified here uses perspective projection which will
	// provide the 3D effect. The model-view matrix positions the the text in world space
	// providing the 'camera' position and the scrolling of the text.

	centralTextGroup->setScaleRotateTranslate(trans);
	centralTextGroup->setViewProjection(mvp);

	// The previous method (renderTitle()) explains the following functions in more detail
	// however put simply, we are looping the entire array of loaded text which is encoded
	// in UTF-8. uiRenderer batches this internally and the call to Flush() will render the
	// text to the frame buffer. We are also fading out the text over a certain distance.
	float pos, fade;
	pvr::uint32 uiCol;
	for (pvr::uint32 uiIndex = 0; uiIndex < textLines.size(); ++uiIndex)
	{
		pos = (textOffset - (uiIndex * 36.0f));
		fade = 1.0f;
		if (pos > TextFadeStart)
		{
			fade = glm::clamp(1.0f - ((pos - TextFadeStart) / (TextFadeEnd - TextFadeStart)), 0.0f, 1.0f);
		}

		uiCol = (((pvr::uint32)(fade * 255)) << 24) | 0x00FFFF;

		centralTextLines[uiIndex]->setColor(uiCol);
		centralTextLines[uiIndex]->setAnchor(pvr::ui::Anchor::Center, glm::vec2(0.f, -(uiIndex * lineSpacingNDC)));
	}
	centralTextGroup->commitUpdates();
}

/*!******************************************************************************************************************
\brief	This function must be implemented by the user of the shell.
		The user should return its pvr::Shell object defining the behaviour of the application.
\return	Return auto ptr to the demo supplied by the user
*********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() {	return std::auto_ptr<pvr::Shell>(new OGLESIntroducingUIRenderer()); }
