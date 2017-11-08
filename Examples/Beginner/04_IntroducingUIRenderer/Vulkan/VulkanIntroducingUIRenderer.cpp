/*!******************************************************************************************************************
\File         VkIntroUIRenderer.cpp
\Title        Introducing uiRenderer
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief  Shows how to use the UIRenderer class to draw ASCII/UTF-8 or wide-charUnicode-compliant text in 3D.
*********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "PVRVk/ApiObjectsVk.h"

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

const uint32_t IntroTime = 4000;
const uint32_t IntroFadeTime = 1000;
const uint32_t TitleTime = 4000;
const uint32_t TitleFadeTime = 1000;
const uint32_t TextFadeStart = 300;
const uint32_t TextFadeEnd = 500;

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
	pvr::ui::Text _text[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	uint8_t _isDirty[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	uint32_t _lastUpdateText;
	uint32_t _lastUpdateColor;
	uint32_t _numElement;

	enum { DirtyTextMask, DirtyColorMask };

public:
	MultiBufferTextManager() : _numElement(0) {}
	MultiBufferTextManager& addText(pvr::ui::Text text)
	{
		_text[_numElement++] = text;
		return *this;
	}

	pvr::ui::Text getText(uint32_t swapchain) { return _text[swapchain]; }

	void setText(uint32_t swapchain, const char* str)
	{
		_lastUpdateText = swapchain;
		_text[swapchain]->getTextElement()->setText(str);
		_text[swapchain]->commitUpdates();
		for (uint32_t i = 0; i < ARRAY_SIZE(_isDirty); ++i)
		{ _isDirty[i] |= (1 << DirtyTextMask); }

		_isDirty[swapchain] &= ~(1 << DirtyTextMask) ;
	}


	void setText(uint32_t swapchain, const wchar_t* str)
	{
		_lastUpdateText = swapchain;
		_text[swapchain]->getTextElement()->setText(str);
		_text[swapchain]->commitUpdates();
		for (uint32_t i = 0; i < _numElement; ++i)
		{ _isDirty[i] |= (1 << DirtyTextMask); }

		_isDirty[swapchain] &= ~(1 << DirtyTextMask) ;
	}


	void setColor(uint32_t swapchain, uint32_t color)
	{
		for (uint32_t i = 0; i < _numElement; ++i)
		{
			_text[i]->setColor(color);
			_isDirty[i] |= (1 << DirtyColorMask);
		}

		_text[swapchain]->commitUpdates();
		_isDirty[swapchain] &= ~(1 << DirtyColorMask) ;
	}

	bool updateText(uint32_t swapchain)
	{
		if (_isDirty[swapchain] & 0x02)
		{
			_text[swapchain]->commitUpdates();
			_isDirty[swapchain] &= ~(1 << DirtyColorMask);
		}

		if (_isDirty[swapchain] & 0x01)
		{
			if (_text[_lastUpdateText]->getTextElement()->getString().length())
			{ _text[swapchain]->getTextElement()->setText(_text[_lastUpdateText]->getTextElement()->getString()); }
			else
			{ _text[swapchain]->getTextElement()->setText(_text[_lastUpdateText]->getTextElement()->getWString()); }
			_text[swapchain]->commitUpdates();
			_isDirty[swapchain] &= ~(1 << DirtyTextMask) ;
			return true;
		}
		return false;
	}

	void renderText(uint32_t swapchain)
	{
		_text[swapchain]->render();
	}


};

/*!******************************************************************************************************************
Class implementing the pvr::Shell functions.
*********************************************************************************************************************/
class VulkanIntroducingUIRenderer : public pvr::Shell
{
	struct DeviceResources
	{
		pvrvk::Instance instance;
		pvrvk::Surface surface;
		pvrvk::Device device;
		pvrvk::Swapchain swapchain;
		pvrvk::Queue queue;

		pvrvk::CommandPool commandPool;
		pvrvk::DescriptorPool descriptorPool;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;

		MultiBufferTextManager titleText1;
		MultiBufferTextManager titleText2;

		pvr::ui::Image background;
		pvr::Multi<pvr::ui::MatrixGroup> centralTextGroup;
		std::vector<pvr::ui::Text> centralTextLines;
		pvr::ui::Text centralTitleLine1;
		pvr::ui::Text centralTitleLine2;

		pvr::Multi<pvrvk::ImageView> depthStencilImages;
		pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffer;

		pvrvk::Semaphore semaphoreImageAcquired[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Fence perFrameAcquireFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Semaphore semaphorePresent[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Fence perFrameCommandBufferFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

		pvr::Multi<pvrvk::SecondaryCommandBuffer> commandBufferWithIntro;
		pvr::Multi<pvrvk::SecondaryCommandBuffer> commandBufferWithText;
		pvr::Multi<pvrvk::SecondaryCommandBuffer> commandBufferSubtitle;
		pvr::Multi<pvrvk::CommandBuffer> primaryCommandBuffer;
	};

	// UIRenderer class used to display _text

	glm::mat4 _mvp;

	float _textOffset;
	float _lineSpacingNDC;
	std::vector<char> _text;
	std::vector<const char*> _textLines;
	Language::Enum _titleLang;
	int32_t _textStartY, _textEndY;

	std::unique_ptr<DeviceResources> _deviceResources;

	uint32_t _frameId;
public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void generateBackgroundTexture(uint32_t screenWidth, uint32_t screenHeight,
	                               pvrvk::CommandBuffer& uploadCmd, std::vector<pvr::utils::ImageUploadResults>& uploadResults);

	void updateCentralTitle(uint64_t currentTime);
	void updateSubTitle(uint64_t currentTime, uint32_t swapchain);
	void updateCentralText(uint64_t currentTime);
	void recordCommandBuffers();
private:
	pvr::Result loadFontFromResources(const char* filename,
	                                  pvr::ui::UIRenderer& uirenderer, pvr::ui::Font& font, pvrvk::CommandBuffer& uploadCmd,
	                                  std::vector<pvr::utils::ImageUploadResults>& imageUploads);
};

/*!******************************************************************************************************************
\brief  Record the rendering commands
*********************************************************************************************************************/
void VulkanIntroducingUIRenderer::recordCommandBuffers()
{
	for (uint32_t i = 0; i < _deviceResources->onScreenFramebuffer.size(); ++i)
	{
		// commandbuffer intro
		{
			_deviceResources->commandBufferWithIntro[i]->begin(_deviceResources->onScreenFramebuffer[i], 0);
			_deviceResources->uiRenderer.beginRendering(_deviceResources->commandBufferWithIntro[i]);
			_deviceResources->background->render();
			//This is the difference
			_deviceResources->centralTitleLine1->render();
			_deviceResources->centralTitleLine2->render();
			_deviceResources->uiRenderer.getSdkLogo()->render();
			// Tells uiRenderer to do all the pending _text rendering now
			_deviceResources->uiRenderer.endRendering();
			_deviceResources->commandBufferWithIntro[i]->end();
		}

		// commandbuffer scrolling _text
		{
			_deviceResources->commandBufferWithText[i]->begin(_deviceResources->onScreenFramebuffer[i], 0);
			_deviceResources->uiRenderer.beginRendering(_deviceResources->commandBufferWithText[i]);
			_deviceResources->background->render();
			_deviceResources->centralTextGroup[i]->render();
			_deviceResources->uiRenderer.getSdkLogo()->render();
			//// Tells uiRenderer to do all the pending _text rendering now
			_deviceResources->uiRenderer.endRendering();
			_deviceResources->commandBufferWithText[i]->end();
		}
	}
}

/*!******************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in initApplication() will be called by Shell once per run, before the rendering context is created.
    Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
    If the rendering context is lost, initApplication() will not be called again.
*********************************************************************************************************************/
pvr::Result VulkanIntroducingUIRenderer::initApplication()
{
	// Because the C++ standard states that only ASCII characters are valid in compiled code,
	// we are instead using an external resource file which contains all of the _text to be
	// rendered. This allows complete control over the encoding of the resource file which
	// in this case is encoded as UTF-8.
	pvr::Stream::ptr_type textStream = getAssetStream(CentralTextFile);

	//Create the empty API objects.
	_deviceResources.reset(new DeviceResources);

	if (!textStream.get())
	{
		this->setExitMessage("ERROR: Failed to load _text resource file!");
		return pvr::Result::UnknownError;
	}

	// The following code simply pulls out each line in the resource file and adds it
	// to an array so we can render each line separately. ReadIntoCharBuffer null-terminates the std::string
	// so it is safe to check for null character.
	textStream->readIntoCharBuffer(_text);
	size_t current = 0;
	while (current < _text.size())
	{
		const char* start = _text.data() + current;

		_textLines.push_back(start);
		while (current < _text.size() && _text[current] != '\0' && _text[current] != '\n' && _text[current] != '\r') { ++current; }

		if (current < _text.size() && (_text[current] == '\r')) { _text[current++] = '\0'; }
		//null-term the strings!!!
		if (current < _text.size() && (_text[current] == '\n' || _text[current] == '\0')) {  _text[current++] = '\0'; }
	}

	_titleLang = Language::English;
	_frameId = 0;
	return pvr::Result::Success;
}

/*!******************************************************************************************************************
\return   Return pvr::Result::Success if no error occurred
\brief    Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
      If the rendering context is lost, quitApplication() will not be called.
*********************************************************************************************************************/
pvr::Result VulkanIntroducingUIRenderer::quitApplication() {  return pvr::Result::Success; }

/*!******************************************************************************************************************
\brief  Generates a simple background texture procedurally.
\param[in]  screenWidth screen dimension's width
\param[in]  screenHeight screen dimension's height
*********************************************************************************************************************/
void VulkanIntroducingUIRenderer::generateBackgroundTexture(uint32_t screenWidth, uint32_t screenHeight,
    pvrvk::CommandBuffer& uploadCmd, std::vector<pvr::utils::ImageUploadResults>& outUploadResults)
{
	// Generate star texture
	uint32_t width = pvr::math::makePowerOfTwoHigh(screenWidth);
	uint32_t height = pvr::math::makePowerOfTwoHigh(screenHeight);

	pvr::TextureHeader::Header hd;
	hd.channelType = pvr::VariableType::UnsignedByteNorm;
	hd.pixelFormat = pvr::GeneratePixelType1<'l', 8>::ID;
	hd.colorSpace = pvr::ColorSpace::lRGB;
	hd.width = width;
	hd.height = height;
	pvr::Texture myTexture(hd);
	unsigned char* textureData = myTexture.getDataPointer();
	memset(textureData, 0, width * height);
	for (uint32_t j = 0; j < height; ++j)
	{
		for (uint32_t i = 0; i < width; ++i)
		{
			if (!(rand() % 200))
			{
				int brightness = rand() % 255;
				textureData[width * j + i] = glm::clamp(textureData[width * j + i] + brightness, 0, 255);
			}
		}
	}
	pvr::utils::ImageUploadResults result = pvr::utils::uploadImage(_deviceResources->device, myTexture, true, uploadCmd);
	if (result.getImageView().isNull())
	{
		return;
	}
	_deviceResources->background = _deviceResources->uiRenderer.createImage(result.getImageView());
	outUploadResults.push_back(result);
}

/*!******************************************************************************************************************
\brief Load font from the resource used for this example
\param[in] streamManager asset provider
\param[in] filename name of the font file
\param[in] uirenderer ui::Font creator
\param[out] font returned font
\return Return pvr::Result::Success if no error occurred.
*********************************************************************************************************************/
pvr::Result VulkanIntroducingUIRenderer::loadFontFromResources(const char* filename,
    pvr::ui::UIRenderer& uirenderer, pvr::ui::Font& font, pvrvk::CommandBuffer& uploadCmd,
    std::vector<pvr::utils::ImageUploadResults>& imageUploads)
{
	// the AssetStore is unsuitable for loading the font, because it does not keep the actual texture data that we need.
	// The assetStore immediately releases the texture data as soon as it creates the API objects and the texture header.
	// Hence we use texture load.
	pvr::Result res = pvr::Result::Success;
	pvr::Stream::ptr_type fontFile = getAssetStream(filename);
	if (!fontFile.get() || !fontFile->isReadable()) { return pvr::Result::UnknownError;   }
	pvr::Texture tmpTexture;
	pvr::utils::ImageUploadResults upload = pvr::utils::loadAndUploadImage(
	    _deviceResources->device, filename, true, uploadCmd, *this, VkImageUsageFlags::e_SAMPLED_BIT, &tmpTexture);
	if (upload.getImageView().isNull())
	{
		return pvr::Result::UnknownError;
	}
	imageUploads.push_back(upload);
	font = uirenderer.createFont(upload.getImageView(), tmpTexture);
	return res;
}

/*!******************************************************************************************************************
\return Result::Success if no error occurred
\brief  Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
    Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
*********************************************************************************************************************/
pvr::Result VulkanIntroducingUIRenderer::initView()
{
	if (!pvr::utils::createInstanceAndSurface(this->getApplicationName(), this->getWindow(), this->getDisplay(), _deviceResources->instance, _deviceResources->surface))
	{
		return pvr::Result::UnknownError;
	}

	pvr::utils::QueuePopulateInfo queuePopulateInfo =
	{
		VkQueueFlags::e_GRAPHICS_BIT, _deviceResources->surface
	};
	pvr::utils::QueueAccessInfo queueAccessInfo;
	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0),
	                           &queuePopulateInfo, 1, &queueAccessInfo);
	if (_deviceResources->device.isNull())
	{
		return pvr::Result::UnknownError;
	}
	// get the queue
	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	// Create the commandpool
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(queueAccessInfo.familyId,
	                                VkCommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT);

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(_deviceResources->surface);

	// validate the supported swapchain image usage
	VkImageUsageFlags swapchainImageUsage = VkImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, VkImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= VkImageUsageFlags::e_TRANSFER_SRC_BIT;
	}

	// Create the swapchain image
	if (!pvr::utils::createSwapchainAndDepthStencilImageView(_deviceResources->device, _deviceResources->surface,
	    getDisplayAttributes(), _deviceResources->swapchain, _deviceResources->depthStencilImages, swapchainImageUsage))
	{
		return pvr::Result::UnknownError;
	}

	pvrvk::RenderPass renderPass;
	pvr::utils::createOnscreenFramebufferAndRenderpass(_deviceResources->swapchain, &_deviceResources->depthStencilImages[0], _deviceResources->onScreenFramebuffer, renderPass);
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), renderPass, 0, _deviceResources->commandPool, _deviceResources->queue, true, true, true, 128);

	// Create the sync objects and the commandbuffer
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->commandBufferSubtitle[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->commandBufferWithIntro[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->commandBufferWithText[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->primaryCommandBuffer[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->semaphorePresent[i] = _deviceResources->device->createSemaphore();
		_deviceResources->semaphoreImageAcquired[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameCommandBufferFence[i] = _deviceResources->device->createFence(VkFenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->perFrameAcquireFence[i] = _deviceResources->device->createFence(VkFenceCreateFlags::e_SIGNALED_BIT);
	}

	std::vector<pvr::utils::ImageUploadResults> imageUploads;
	_deviceResources->primaryCommandBuffer[0]->begin();
	// Generate background texture
	generateBackgroundTexture(getWidth(), getHeight(), _deviceResources->primaryCommandBuffer[0], imageUploads);

	// The fonts are loaded here using a PVRTool's ResourceFile wrapper. However,
	// it is possible to load the textures in any way that provides access to a pointer
	// to memory, and the size of the file.
	pvr::ui::Font subTitleFont, centralTitleFont, centralTextFont;
	{
		pvr::Result res;
		if ((res = loadFontFromResources(CentralTitleFontFile, _deviceResources->uiRenderer, centralTitleFont,
		                                 _deviceResources->primaryCommandBuffer[0], imageUploads)) != pvr::Result::Success)
		{
			this->setExitMessage("ERROR: Failed to create font from file %s", CentralTitleFontFile);
			return res;
		}
		if ((res = loadFontFromResources(CentralTextFontFile, _deviceResources->uiRenderer, centralTextFont,
		                                 _deviceResources->primaryCommandBuffer[0], imageUploads)) != pvr::Result::Success)
		{
			this->setExitMessage("ERROR: Failed to create font from file %s", CentralTextFontFile);
			return res;
		}

		// Determine which size title font to use.
		uint32_t screenShortDimension = std::min(getWidth(), getHeight());
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

		if ((res = loadFontFromResources(titleFontFileName, _deviceResources->uiRenderer, subTitleFont,
		                                 _deviceResources->primaryCommandBuffer[0], imageUploads)) != pvr::Result::Success)
		{
			this->setExitMessage("ERROR: Failed to create font from file %s", CentralTitleFontFile);
			return res;
		}
	}

		_deviceResources->primaryCommandBuffer[0]->end();

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->primaryCommandBuffer[0];
	submitInfo.numCommandBuffers = 1;
	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle();
	imageUploads.clear();
	_deviceResources->primaryCommandBuffer[0]->reset(VkCommandBufferResetFlags::e_RELEASE_RESOURCES_BIT);

	_deviceResources->background->commitUpdates();

	_deviceResources->uiRenderer.getSdkLogo()->commitUpdates();
	const uint32_t  swapChainLength = _deviceResources->swapchain->getSwapchainLength();
	for (uint32_t i = 0; i < swapChainLength; ++i)
	{
		pvr::ui::Text text1 = _deviceResources->uiRenderer.createText(subTitleFont);
		pvr::ui::Text text2 = _deviceResources->uiRenderer.createText(subTitleFont);
		text1->setAnchor(pvr::ui::Anchor::TopLeft, -.98f, .98f);
		text2->setAnchor(pvr::ui::Anchor::TopLeft, -.98f, .98f);

		_deviceResources->titleText1.addText(text1);
		_deviceResources->titleText2.addText(text2);
		_deviceResources->centralTextGroup[i] = _deviceResources->uiRenderer.createMatrixGroup();
	}

	_deviceResources->centralTextLines.push_back(_deviceResources->uiRenderer.createText(_textLines[0], centralTextFont));
	for (uint32_t i = 0; i < swapChainLength; ++i)
	{
		_deviceResources->centralTextGroup[i]->add(_deviceResources->centralTextLines.back());
	}
	_lineSpacingNDC = 1.6f * _deviceResources->centralTextLines[0]->getFont()->getFontLineSpacing() / (float)_deviceResources->uiRenderer.getRenderingDimY();

	for (uint32_t i = 1; i < _textLines.size(); ++i)
	{
		pvr::ui::Text _text = _deviceResources->uiRenderer.createText(_textLines[i], centralTextFont);
		_text->setAnchor(pvr::ui::Anchor::Center, glm::vec2(0.f, -(i * _lineSpacingNDC)));
		_deviceResources->centralTextLines.push_back(_text);
		for (uint32_t i = 0; i < swapChainLength; ++i)
		{
			_deviceResources->centralTextGroup[i]->add(_text);
		}
	}

	_deviceResources->centralTextLines[0]->setAlphaRenderingMode(true);
	_deviceResources->centralTitleLine1 = _deviceResources->uiRenderer.createText("introducing", centralTitleFont);
	_deviceResources->centralTitleLine2 =  _deviceResources->uiRenderer.createText("uirenderer", centralTitleFont);

	_deviceResources->centralTitleLine1->setAnchor(pvr::ui::Anchor::BottomCenter, glm::vec2(.0f, .0f));
	_deviceResources->centralTitleLine2->setAnchor(pvr::ui::Anchor::TopCenter, glm::vec2(.0f, .0f));

	_textStartY = static_cast<int32_t>(-_deviceResources->uiRenderer.getRenderingDimY() -
	                                   _deviceResources->centralTextGroup[0]->getDimensions().y);

	_textEndY = static_cast<int32_t>(_deviceResources->uiRenderer.getRenderingDimY() +
	                                 _deviceResources->centralTextGroup[0]->getDimensions().y +
	                                 _lineSpacingNDC * (float)_deviceResources->uiRenderer.getRenderingDimY());

	_textOffset = static_cast<float>(_textStartY);
	recordCommandBuffers();
	return pvr::Result::Success;
}

/*!******************************************************************************************************************
\return Result::Success if no error occurred
\brief  Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
*********************************************************************************************************************/
pvr::Result VulkanIntroducingUIRenderer::releaseView()
{
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); i++)
	{
		_deviceResources->perFrameAcquireFence[i]->wait();
		_deviceResources->perFrameAcquireFence[i]->reset();

		_deviceResources->perFrameCommandBufferFence[i]->wait();
		_deviceResources->perFrameCommandBufferFence[i]->reset();
	}
        _deviceResources->device->waitIdle();
	_deviceResources.reset();
	return pvr::Result::Success;
}

/*!******************************************************************************************************************
\brief  Main rendering loop function of the program. The shell will call this function every frame.
\return Result::Success if no error occurred
*********************************************************************************************************************/
pvr::Result VulkanIntroducingUIRenderer::renderFrame()
{
	// Clears the color and depth buffer
	uint64_t currentTime = this->getTime() - this->getTimeAtInitApplication();

	_deviceResources->perFrameAcquireFence[_frameId]->wait();
	_deviceResources->perFrameAcquireFence[_frameId]->reset();
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->semaphoreImageAcquired[_frameId], _deviceResources->perFrameAcquireFence[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameCommandBufferFence[swapchainIndex]->wait();
	_deviceResources->perFrameCommandBufferFence[swapchainIndex]->reset();

	updateSubTitle(currentTime, swapchainIndex);
	// record the primary commandbuffer
	_deviceResources->primaryCommandBuffer[swapchainIndex]->begin();
	const pvrvk::ClearValue clearValue[2] =
	{
		pvrvk::ClearValue(0.f, 0.f, 0.f, 0.f),
		pvrvk::ClearValue(1.f, 0u)
	};
	_deviceResources->primaryCommandBuffer[swapchainIndex]->beginRenderPass(_deviceResources->onScreenFramebuffer[swapchainIndex],
	    false, clearValue, ARRAY_SIZE(clearValue));
	// Render the 'Introducing uiRenderer' title for the first n seconds.
	if (currentTime < IntroTime)
	{
		updateCentralTitle(currentTime);
		_deviceResources->primaryCommandBuffer[swapchainIndex]->executeCommands(_deviceResources->commandBufferWithIntro[swapchainIndex]);
	}
	//Render the 3D _text.
	else
	{
		updateCentralText(currentTime);
		_deviceResources->primaryCommandBuffer[swapchainIndex]->executeCommands(_deviceResources->commandBufferWithText[swapchainIndex]);
	}
	_deviceResources->centralTextGroup[swapchainIndex]->commitUpdates();
	_deviceResources->commandBufferSubtitle[swapchainIndex]->begin(_deviceResources->onScreenFramebuffer[swapchainIndex], 0);
	_deviceResources->uiRenderer.beginRendering(_deviceResources->commandBufferSubtitle[swapchainIndex]);
	_deviceResources->titleText1.renderText(swapchainIndex);
	_deviceResources->titleText2.renderText(swapchainIndex);
	_deviceResources->uiRenderer.endRendering();
	_deviceResources->commandBufferSubtitle[swapchainIndex]->end();

	_deviceResources->primaryCommandBuffer[swapchainIndex]->executeCommands(_deviceResources->commandBufferSubtitle[swapchainIndex]);
	_deviceResources->primaryCommandBuffer[swapchainIndex]->endRenderPass();
	_deviceResources->primaryCommandBuffer[swapchainIndex]->end();

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers =  &_deviceResources->primaryCommandBuffer[swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->semaphoreImageAcquired[_frameId];
	submitInfo.signalSemaphores = &_deviceResources->semaphorePresent[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.numSignalSemaphores = 1;
	VkPipelineStageFlags waitStage = VkPipelineStageFlags::e_TRANSFER_BIT;
	submitInfo.waitDestStages = &waitStage;

	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameCommandBufferFence[swapchainIndex]);

	if (this->shouldTakeScreenshot())
	{
		if (_deviceResources->swapchain->supportsUsage(VkImageUsageFlags::e_TRANSFER_SRC_BIT))
		{
			pvr::utils::takeScreenshot(_deviceResources->swapchain, swapchainIndex, _deviceResources->commandPool, _deviceResources->queue, this->getScreenshotFileName());
		}
		else
		{
			Log(LogLevel::Warning, "Could not take screenshot as the swapchain does not support TRANSFER_SRC_BIT");
		}
	}

	// PRESENT
	pvrvk::PresentInfo presentInfo;
	presentInfo.imageIndices = &swapchainIndex;
	presentInfo.numSwapchains = 1;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numWaitSemaphores = 1;
	presentInfo.waitSemaphores = &_deviceResources->semaphorePresent[_frameId];
	_deviceResources->queue->present(presentInfo);

	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();

	return pvr::Result::Success;
}

/*!******************************************************************************************************************
\brief  Update the description sprite
\param  currentTime Current Time
*********************************************************************************************************************/
void VulkanIntroducingUIRenderer::updateSubTitle(uint64_t currentTime, uint32_t swapchain)
{
	// Fade effect
	static int prevLang = (int) - 1;
	uint32_t titleLang = static_cast<uint32_t>((currentTime / 1000) / (TitleTime / 1000)) % Language::Count;

	uint32_t nextLang = (titleLang + 1) % Language::Count;
	uint32_t modTime = static_cast<uint32_t>(currentTime) % TitleTime;
	float titlePerc = 1.0f;
	float nextPerc = 0.0f;
	if (modTime > TitleTime - TitleFadeTime)
	{
		titlePerc = 1.0f - ((modTime - (TitleTime - TitleFadeTime)) / (float)TitleFadeTime);
		nextPerc = 1.0f - titlePerc;
	}
	uint32_t titleCol = ((static_cast<uint32_t>(titlePerc * 255)) << 24) | 0xFFFFFF;
	uint32_t nextCol = ((static_cast<uint32_t>(nextPerc * 255)) << 24) | 0xFFFFFF;

	// Here we are passing in a wide-character std::string to uiRenderer function. This allows
	// Unicode to be compiled in to std::string-constants, which this code snippet demonstrates.
	// Because we are not setting a projection or a model-view matrix the default projection
	// matrix is used.
	if (titleLang != prevLang)
	{
		_deviceResources->titleText1.setText(swapchain, Titles[titleLang]);
		_deviceResources->titleText2.setText(swapchain, Titles[nextLang]);
		prevLang = titleLang;
	}
	_deviceResources->titleText1.setColor(swapchain, titleCol);
	_deviceResources->titleText2.setColor(swapchain, nextCol);
	_deviceResources->titleText1.updateText(swapchain);
	_deviceResources->titleText2.updateText(swapchain);
}

/*!******************************************************************************************************************
\brief  Draws the title _text.
\param[in]  fadeAmount
*********************************************************************************************************************/
void VulkanIntroducingUIRenderer::updateCentralTitle(uint64_t currentTime)
{

	// Using the MeasureText() method provided by uiRenderer, we can determine the bounding-box
	// size of a std::string of _text. This can be useful for justify _text centrally, as we are
	// doing here.
	float fadeAmount = 1.0f;

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
	//Editing the _text's alpha based on the fade amount.
	_deviceResources->centralTitleLine1->setColor(1.f, 1.f, 0.f, fadeAmount);
	_deviceResources->centralTitleLine2->setColor(1.f, 1.f, 0.f, fadeAmount);
	_deviceResources->centralTitleLine1->commitUpdates();
	_deviceResources->centralTitleLine2->commitUpdates();
}

/*!******************************************************************************************************************
\brief  Draws the 3D _text and scrolls in to the screen.
*********************************************************************************************************************/
void VulkanIntroducingUIRenderer::updateCentralText(uint64_t currentTime)
{
	const glm::mat4 mProjection = pvr::math::perspective(pvr::Api::Vulkan, 0.7f,
	                              float(_deviceResources->uiRenderer.getRenderingDimX()) / float(_deviceResources->uiRenderer.getRenderingDimY()), 1.0f, 2000.0f);

	const glm::mat4 mCamera = glm::lookAt(glm::vec3(_deviceResources->uiRenderer.getRenderingDimX() * .5f,
	                                      -_deviceResources->uiRenderer.getRenderingDimY(), 700.0f), glm::vec3(_deviceResources->uiRenderer.getRenderingDimX() * .5f, 0, 0.0f),
	                                      glm::vec3(0.0f, 1.0f, 0.0f));
	_mvp = mProjection * mCamera;

	// Calculate the FPS scale.
	float fFPSScale = float(getFrameTime()) * 60 / 1000;

	// Move the _text. Progressively speed up.
	float fSpeedInc = 0.0f;
	if (_textOffset > 0.0f) { fSpeedInc = _textOffset / _textEndY; }
	_textOffset += (0.75f + (1.0f * fSpeedInc)) * fFPSScale;
	if (_textOffset > _textEndY)
	{
		_textOffset = static_cast<float>(_textStartY);
	}
	glm::mat4 trans = glm::translate(glm::vec3(0.0f, _textOffset, 0.0f));

	// uiRenderer can optionally be provided with user-defined projection and model-view matrices
	// which allow custom layout of _text. Here we are proving both a projection and model-view
	// matrix. The projection matrix specified here uses perspective projection which will
	// provide the 3D effect. The model-view matrix positions the the _text in world space
	// providing the 'camera' position and the scrolling of the _text.

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->centralTextGroup[i]->setScaleRotateTranslate(trans);
		_deviceResources->centralTextGroup[i]->setViewProjection(_mvp);
	}

	// The previous method (renderTitle()) explains the following functions in more detail
	// however put simply, we are looping the entire array of loaded _text which is encoded
	// in UTF-8. uiRenderer batches this internally and the call to Flush() will render the
	// _text to the frame buffer. We are also fading out the _text over a certain distance.
	float pos, fade;
	uint32_t uiCol;
	for (uint32_t uiIndex = 0; uiIndex < _textLines.size(); ++uiIndex)
	{
		pos = (_textOffset - (uiIndex * 36.0f));
		fade = 1.0f;
		if (pos > TextFadeStart)
		{
			fade = glm::clamp(1.0f - ((pos - TextFadeStart) / (TextFadeEnd - TextFadeStart)), 0.0f, 1.0f);
		}

		uiCol = ((static_cast<uint32_t>(fade * 255)) << 24) | 0x00FFFF;

		_deviceResources->centralTextLines[uiIndex]->setColor(uiCol);
	}
	_deviceResources->centralTextLines[0]->commitUpdates();
}

/*!******************************************************************************************************************
\brief  This function must be implemented by the user of the shell.
    The user should return its pvr::Shell object defining the behaviour of the application.
\return Return auto ptr to the demo supplied by the user
*********************************************************************************************************************/
std::unique_ptr<pvr::Shell> pvr::newDemo() {  return std::unique_ptr<pvr::Shell>(new VulkanIntroducingUIRenderer()); }
