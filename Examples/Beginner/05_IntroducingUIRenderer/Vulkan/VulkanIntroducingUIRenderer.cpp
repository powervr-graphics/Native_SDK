/*!******************************************************************************************************************
\File         VkIntroUIRenderer.cpp
\Title        Introducing uiRenderer
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief  Shows how to use the UIRenderer class to draw ASCII/UTF-8 or wide-charUnicode-compliant text in 3D.
*********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVREngineUtils/PVREngineUtils.h"

using namespace pvr;

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

class MultiBufferTextManager
{
	ui::Text _text[(uint32)FrameworkCaps::MaxSwapChains];
	uint8 _isDirty[(pvr::uint32)FrameworkCaps::MaxSwapChains];
	uint32 _lastUpdateText;
	uint32 _lastUpdateColor;
	uint32 _numElement;

	enum{ DirtyTextMask, DirtyColorMask };

public:
	MultiBufferTextManager() : _numElement(0){}
	MultiBufferTextManager& addText(ui::Text text)
	{
		_text[_numElement++] = text;
		return *this;
	}

	ui::Text getText(uint32 swapchain){ return _text[swapchain]; }

	void setText(uint32 swapchain, const char* str)
	{
		_lastUpdateText = swapchain;
		_text[swapchain]->getTextElement()->setText(str);
		_text[swapchain]->commitUpdates();
		for(uint32 i = 0; i < ARRAY_SIZE(_isDirty); ++i)
			_isDirty[i] |= (1 << DirtyTextMask);

		_isDirty[swapchain] &= ~(1 << DirtyTextMask) ;
	}


	void setText(uint32 swapchain, const wchar* str)
	{
		_lastUpdateText = swapchain;
		_text[swapchain]->getTextElement()->setText(str);
		_text[swapchain]->commitUpdates();
		for(uint32 i = 0; i < _numElement; ++i)
			_isDirty[i] |= (1 << DirtyTextMask);

		_isDirty[swapchain] &= ~(1 << DirtyTextMask) ;
	}


	void setColor(uint32 swapchain, uint32 color )
	{
		for(uint32 i = 0; i < _numElement; ++i)
		{
			_text[i]->setColor(color);
			_isDirty[i] |= (1 << DirtyColorMask);
		}

		_text[swapchain]->commitUpdates();
		_isDirty[swapchain] &= ~(1 << DirtyColorMask) ;
	}

	bool updateText(uint32 swapchain)
	{
		if(_isDirty[swapchain] & 0x02)
		{
			_text[swapchain]->commitUpdates();
			_isDirty[swapchain] &= ~(1 << DirtyColorMask);
		}

		if(_isDirty[swapchain] & 0x01)
		{
			if(_text[_lastUpdateText]->getTextElement()->getString().length())
				_text[swapchain]->getTextElement()->setText(_text[_lastUpdateText]->getTextElement()->getString());
			else
				_text[swapchain]->getTextElement()->setText(_text[_lastUpdateText]->getTextElement()->getWString());
			_text[swapchain]->commitUpdates();
			_isDirty[swapchain] &= ~(1 << DirtyTextMask) ;
			return true;
		}
		return false;
	}

	void renderText(uint32 swapchain)
	{
		_text[swapchain]->render();
	}


};

/*!******************************************************************************************************************
Class implementing the pvr::Shell functions.
*********************************************************************************************************************/
class VulkanIntroducingUIRenderer : public pvr::Shell
{
	// UIRenderer class used to display text

	glm::mat4 mvp;

	pvr::float32 textOffset;
	pvr::float32 lineSpacingNDC;
	std::vector<char>	text;
	std::vector<const char*> textLines;
	Language::Enum _titleLang;
	pvr::int32 textStartY, textEndY;

	struct ApiObject
	{
		pvr::ui::Image background;
		pvr::ui::MatrixGroup centralTextGroup[uint32(FrameworkCaps::MaxSwapChains)];
		std::vector<pvr::ui::Text> centralTextLines;
		pvr::ui::Text centralTitleLine1;
		pvr::ui::Text centralTitleLine2;
		MultiBufferTextManager titleText1;
		MultiBufferTextManager titleText2;

		pvr::ui::UIRenderer uiRenderer;
		pvr::Multi<pvr::api::Fbo> onScreenFbo;
		std::vector<pvr::api::SecondaryCommandBuffer> commandBufferWithIntro;
		std::vector<pvr::api::SecondaryCommandBuffer> commandBufferWithText;
		std::vector<pvr::api::SecondaryCommandBuffer> commandBufferSubtitle;
		std::vector<pvr::api::CommandBuffer> primaryCommandBuffer;
		pvr::utils::AssetStore assetStore;
	};
	pvr::RefCountedResource<ApiObject> apiObj;

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
void VulkanIntroducingUIRenderer::recordCommandBuffers()
{
	apiObj->commandBufferWithIntro.resize(apiObj->onScreenFbo.size());
	apiObj->commandBufferWithText.resize(apiObj->onScreenFbo.size());
	apiObj->commandBufferSubtitle.resize(apiObj->onScreenFbo.size());
	apiObj->primaryCommandBuffer.resize(apiObj->onScreenFbo.size());
	for (pvr::uint32 i = 0; i < apiObj->onScreenFbo.size(); ++i)
	{
		apiObj->commandBufferWithIntro[i] = getGraphicsContext()->createSecondaryCommandBufferOnDefaultPool();
		apiObj->commandBufferWithText[i] = getGraphicsContext()->createSecondaryCommandBufferOnDefaultPool();
		apiObj->commandBufferSubtitle[i] = getGraphicsContext()->createSecondaryCommandBufferOnDefaultPool();
		apiObj->primaryCommandBuffer[i] = getGraphicsContext()->createCommandBufferOnDefaultPool();

		// commandbuffer intro
		{
			apiObj->commandBufferWithIntro[i]->beginRecording(apiObj->onScreenFbo[i], 0);
			apiObj->uiRenderer.beginRendering(apiObj->commandBufferWithIntro[i]);
			apiObj->background->render();
			//This is the difference
			apiObj->centralTitleLine1->render();
			apiObj->centralTitleLine2->render();
			apiObj->uiRenderer.getSdkLogo()->render();
			// Tells uiRenderer to do all the pending text rendering now
			apiObj->uiRenderer.endRendering();
			apiObj->commandBufferWithIntro[i]->endRecording();
		}

		// commandbuffer scrolling text
		{
			apiObj->commandBufferWithText[i]->beginRecording(apiObj->onScreenFbo[i], 0);
			apiObj->uiRenderer.beginRendering(apiObj->commandBufferWithText[i]);
			apiObj->background->render();
			apiObj->centralTextGroup[i]->render();
			apiObj->uiRenderer.getSdkLogo()->render();
			//// Tells uiRenderer to do all the pending text rendering now
			apiObj->uiRenderer.endRendering();
			apiObj->commandBufferWithText[i]->endRecording();
		}
	}
}

/*!******************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in initApplication() will be called by Shell once per run, before the rendering context is created.
		Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
		If the rendering context is lost, initApplication() will not be called again.
*********************************************************************************************************************/
pvr::Result VulkanIntroducingUIRenderer::initApplication()
{
	// Because the C++ standard states that only ASCII characters are valid in compiled code,
	// we are instead using an external resource file which contains all of the text to be
	// rendered. This allows complete control over the encoding of the resource file which
	// in this case is encoded as UTF-8.
	pvr::Stream::ptr_type textStream = getAssetStream(CentralTextFile);
	apiObj.construct();
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
pvr::Result VulkanIntroducingUIRenderer::quitApplication() {	return pvr::Result::Success; }

/*!******************************************************************************************************************
\brief	Generates a simple background texture procedurally.
\param[in]	screenWidth screen dimension's width
\param[in]	screenHeight screen dimension's height
*********************************************************************************************************************/
void VulkanIntroducingUIRenderer::generateBackgroundTexture(pvr::uint32 screenWidth, pvr::uint32 screenHeight)
{
	// Generate star texture
	pvr::uint32 width = pvr::math::makePowerOfTwoHigh(screenWidth);
	pvr::uint32 height = pvr::math::makePowerOfTwoHigh(screenHeight);

	pvr::TextureHeader::Header hd;
	hd.channelType = pvr::VariableType::UnsignedByteNorm;
	hd.pixelFormat = pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID;
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
				textureData[(width * j + i) * 4] = glm::clamp(textureData[width * j + i] + brightness, 0, 255);
				textureData[(width * j + i) * 4 + 1] = glm::clamp(textureData[(width * j + i) * 4 + 1] + brightness, 0, 255);
				textureData[(width * j + i) * 4 + 2] = glm::clamp(textureData[(width * j + i) * 4 + 2] + brightness, 0, 255);
				textureData[(width * j + i) * 4 + 3] = glm::clamp(textureData[(width * j + i) * 4 + 3] + brightness, 0, 255);
			}
		}
	}
	apiObj->background = apiObj->uiRenderer.createImage(myTexture);
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
pvr::Result VulkanIntroducingUIRenderer::initView()
{
	apiObj->assetStore.init(*this);
	apiObj->onScreenFbo = getGraphicsContext()->createOnScreenFboSet();
	apiObj->uiRenderer.init(apiObj->onScreenFbo[0]->getRenderPass(), 0, 128);

	// The fonts are loaded here using a PVRTool's ResourceFile wrapper. However,
	// it is possible to load the textures in any way that provides access to a pointer
	// to memory, and the size of the file.
	pvr::ui::Font subTitleFont, centralTitleFont, centralTextFont;
	{
		pvr::Result res;
		if ((res = loadFontFromResources(*this, CentralTitleFontFile, apiObj->uiRenderer, centralTitleFont)) != pvr::Result::Success)
		{
			this->setExitMessage("ERROR: Failed to create font from file %s", CentralTitleFontFile);
			return res;
		}
		if ((res = loadFontFromResources(*this, CentralTextFontFile, apiObj->uiRenderer, centralTextFont)) != pvr::Result::Success)
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

		if ((res = loadFontFromResources(*this, titleFontFileName, apiObj->uiRenderer, subTitleFont)) != pvr::Result::Success)
		{
			this->setExitMessage("ERROR: Failed to create font from file %s", CentralTitleFontFile);
			return res;
		}
	}

	apiObj->uiRenderer.getSdkLogo()->commitUpdates();
	const uint32  swapChainLength = getSwapChainLength();
	for(uint32 i = 0; i < swapChainLength; ++i)
	{
		ui::Text text1 = apiObj->uiRenderer.createText(subTitleFont);
		ui::Text text2 = apiObj->uiRenderer.createText(subTitleFont);
		text1->setAnchor(pvr::ui::Anchor::TopLeft, -.98f, .98f);
		text2->setAnchor(pvr::ui::Anchor::TopLeft, -.98f, .98f);

		apiObj->titleText1.addText(text1);
		apiObj->titleText2.addText(text2);
		apiObj->centralTextGroup[i] = apiObj->uiRenderer.createMatrixGroup();
	}

	apiObj->centralTextLines.push_back(apiObj->uiRenderer.createText(textLines[0], centralTextFont));
	for(uint32 i = 0; i < swapChainLength; ++i)
	{
		apiObj->centralTextGroup[i]->add(apiObj->centralTextLines.back());
	}
	lineSpacingNDC = 1.6f * apiObj->centralTextLines[0]->getFont()->getFontLineSpacing() / (pvr::float32)apiObj->uiRenderer.getRenderingDimY();
	for (pvr::uint32 i = 1; i < textLines.size(); ++i)
	{
		ui::Text text = apiObj->uiRenderer.createText(textLines[i], centralTextFont);
		text->setAnchor(pvr::ui::Anchor::Center, glm::vec2(0.f, -(i * lineSpacingNDC)));
		apiObj->centralTextLines.push_back(text);
		for(uint32 i = 0; i < swapChainLength; ++i)
		{
			apiObj->centralTextGroup[i]->add(text);
		}
	}
	apiObj->centralTextLines[0]->setAlphaRenderingMode(true);
	apiObj->centralTitleLine1 = apiObj->uiRenderer.createText("introducing", centralTitleFont);
	apiObj->centralTitleLine2 =  apiObj->uiRenderer.createText("uirenderer", centralTitleFont);

	apiObj->centralTitleLine1->setAnchor(pvr::ui::Anchor::BottomCenter, glm::vec2(.0f, .0f));
	apiObj->centralTitleLine2->setAnchor(pvr::ui::Anchor::TopCenter, glm::vec2(.0f, .0f));

	// Generate background texture
	generateBackgroundTexture(getWidth(), getHeight());
	apiObj->background->commitUpdates();
	textStartY = static_cast<pvr::int32>(-apiObj->uiRenderer.getRenderingDimY() -
	                                     apiObj->centralTextGroup[0]->getDimensions().y);

	textEndY = static_cast<pvr::int32>(apiObj->uiRenderer.getRenderingDimY() +
	                                   apiObj->centralTextGroup[0]->getDimensions().y +
	                                   lineSpacingNDC * (pvr::float32)apiObj->uiRenderer.getRenderingDimY());

	textOffset = static_cast<pvr::float32>(textStartY);
	recordCommandBuffers();

	return pvr::Result::Success;
}

/*!******************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
*********************************************************************************************************************/
pvr::Result VulkanIntroducingUIRenderer::releaseView()
{
	// Release uiRenderer Textures
	apiObj.reset();
	return pvr::Result::Success;
}

/*!******************************************************************************************************************
\brief	Main rendering loop function of the program. The shell will call this function every frame.
\return	Result::Success if no error occurred
*********************************************************************************************************************/
pvr::Result VulkanIntroducingUIRenderer::renderFrame()
{
	// Clears the color and depth buffer
	pvr::uint64 currentTime = this->getTime() - this->getTimeAtInitApplication();

	updateSubTitle(currentTime);

	const pvr::uint32 swapChainIndex = getSwapChainIndex();
	// record the primary commandbuffer
	apiObj->primaryCommandBuffer[swapChainIndex]->beginRecording();
	apiObj->primaryCommandBuffer[swapChainIndex]->beginRenderPass(apiObj->onScreenFbo[swapChainIndex],
	    pvr::Rectanglei(0, 0, getWidth(), getHeight()), false);
	// Render the 'Introducing uiRenderer' title for the first n seconds.
	if (currentTime < IntroTime)
	{
		updateCentralTitle(currentTime);
		apiObj->primaryCommandBuffer[swapChainIndex]->enqueueSecondaryCmds(apiObj->commandBufferWithIntro[swapChainIndex]);
	}
	//Render the 3D text.
	else
	{
		updateCentralText(currentTime);
		apiObj->primaryCommandBuffer[swapChainIndex]->enqueueSecondaryCmds(apiObj->commandBufferWithText[swapChainIndex]);
	}
	apiObj->centralTextGroup[swapChainIndex]->commitUpdates();

	apiObj->commandBufferSubtitle[swapChainIndex]->beginRecording(apiObj->onScreenFbo[swapChainIndex], 0);
	apiObj->uiRenderer.beginRendering(apiObj->commandBufferSubtitle[swapChainIndex]);
	apiObj->titleText1.renderText(swapChainIndex);
	apiObj->titleText2.renderText(swapChainIndex);
	apiObj->uiRenderer.endRendering();
	apiObj->commandBufferSubtitle[swapChainIndex]->endRecording();

	apiObj->primaryCommandBuffer[swapChainIndex]->enqueueSecondaryCmds(apiObj->commandBufferSubtitle[swapChainIndex]);
	apiObj->primaryCommandBuffer[swapChainIndex]->endRenderPass();
	apiObj->primaryCommandBuffer[swapChainIndex]->endRecording();
	apiObj->primaryCommandBuffer[swapChainIndex]->submit();
	return pvr::Result::Success;
}

/*!******************************************************************************************************************
\brief	Update the description sprite
\param	currentTime Current Time
*********************************************************************************************************************/
void VulkanIntroducingUIRenderer::updateSubTitle(pvr::uint64 currentTime)
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


	const uint32 swapChain = getSwapChainIndex();
	// Here we are passing in a wide-character string to uiRenderer function. This allows
	// Unicode to be compiled in to string-constants, which this code snippet demonstrates.
	// Because we are not setting a projection or a model-view matrix the default projection
	// matrix is used.
	if (titleLang != prevLang)
	{
		apiObj->titleText1.setText(swapChain, Titles[titleLang]);
		apiObj->titleText2.setText(swapChain, Titles[nextLang]);
		prevLang = titleLang;
	}
	apiObj->titleText1.setColor(swapChain, titleCol);
	apiObj->titleText2.setColor(swapChain, nextCol);
	apiObj->titleText1.updateText(swapChain);
	apiObj->titleText2.updateText(swapChain);
}

/*!******************************************************************************************************************
\brief	Draws the title text.
\param[in]	fadeAmount
*********************************************************************************************************************/
void VulkanIntroducingUIRenderer::updateCentralTitle(pvr::uint64 currentTime)
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
	apiObj->centralTitleLine1->setColor(1.f, 1.f, 0.f, fadeAmount);
	apiObj->centralTitleLine2->setColor(1.f, 1.f, 0.f, fadeAmount);
	apiObj->centralTitleLine1->commitUpdates();
	apiObj->centralTitleLine2->commitUpdates();
}

/*!******************************************************************************************************************
\brief	Draws the 3D text and scrolls in to the screen.
*********************************************************************************************************************/
void VulkanIntroducingUIRenderer::updateCentralText(pvr::uint64 currentTime)
{
	const glm::mat4 mProjection = pvr::math::perspective(pvr::Api::Vulkan, 0.7f, float(apiObj->uiRenderer.getRenderingDimX()) / float(apiObj->uiRenderer.getRenderingDimY()), 1.0f, 2000.0f);

	const glm::mat4 mCamera = glm::lookAt(glm::vec3(apiObj->uiRenderer.getRenderingDimX() * .5f, -apiObj->uiRenderer.getRenderingDimY(), 700.0f),
	                                      glm::vec3(apiObj->uiRenderer.getRenderingDimX() * .5f, 0, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	mvp = mProjection * mCamera;

	// Calculate the FPS scale.
	float fFPSScale = float(getFrameTime()) * 60 / 1000;

	// Move the text. Progressively speed up.
	float fSpeedInc = 0.0f;
	if (textOffset > 0.0f) { fSpeedInc = textOffset / textEndY; }
	textOffset += (0.75f + (1.0f * fSpeedInc)) * fFPSScale;
	if (textOffset > textEndY)
	{
		textOffset = static_cast<pvr::float32>(textStartY);
	}
	glm::mat4 trans = glm::translate(glm::vec3(0.0f, textOffset, 0.0f));

	// uiRenderer can optionally be provided with user-defined projection and model-view matrices
	// which allow custom layout of text. Here we are proving both a projection and model-view
	// matrix. The projection matrix specified here uses perspective projection which will
	// provide the 3D effect. The model-view matrix positions the the text in world space
	// providing the 'camera' position and the scrolling of the text.

	for(uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		apiObj->centralTextGroup[i]->setScaleRotateTranslate(trans);
		apiObj->centralTextGroup[i]->setViewProjection(mvp);
	}

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

		apiObj->centralTextLines[uiIndex]->setColor(uiCol);
	}
	apiObj->centralTextLines[0]->commitUpdates();
}

/*!******************************************************************************************************************
\brief	This function must be implemented by the user of the shell.
		The user should return its pvr::Shell object defining the behaviour of the application.
\return	Return auto ptr to the demo supplied by the user
*********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() {	return std::auto_ptr<pvr::Shell>(new VulkanIntroducingUIRenderer()); }
