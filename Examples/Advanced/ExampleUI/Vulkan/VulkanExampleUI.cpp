/*!*********************************************************************************************************************
\File         VulkanExampleUI.cpp
\Title        ExampleUI
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief      Demonstrates how to efficiently render UI and sprites using UIRenderer
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVREngineUtils/PVREngineUtils.h"
using namespace pvr;
enum {
AtlasWidth    = 1024,
AtlasHeight   = 1024,
NullQuadPix   = 4,
VirtualWidth    = 640,
VirtualHeight   = 480,
AtlasPixelBorder  = 1,
UiDisplayTime   = 5,// Display each page for 5 seconds
UiDisplayTimeInMs = UiDisplayTime * 1000,
BaseDimX = 800,
BaseDimY = 600,
NumClocks = 22
};
static const pvr::float32 LowerContainerHeight = .3f;

using namespace pvr::types;
// Shaders
namespace ShaderNames {
enum Enum
{
	ColorTexture,
	ColorShader,
	Count
};
}
// Sprites that will be added to a generated texture atlas
namespace Sprites {
enum Enum
{
	Clockface,
	Hand,
	Battery,
	Web,
	Newmail,
	Network,
	Calendar,
	WeatherSunCloudBig,
	WeatherSunCloud,
	WeatherRain,
	WeatherStorm,
	ContainerCorner,
	ContainerVertical,
	ContainerHorizontal,
	ContainerFiller,
	VerticalBar,
	Text1,
	Text2,
	TextLorem,
	TextWeather,
	TextFriday,
	TextSaturday,
	TextSunday,
	TextMonday,
	ClockfaceSmall,
	HandSmall,
	WindowBottom,
	WindowBottomCorner,
	WindowSide,
	WindowTop,
	WindowTopLeft,
	WindowTopRight,
	Count,
	None = 0xFFFF
};
}
// Ancillary textures that won't be added to texture atlas (generally due to size)
namespace Ancillary {
enum Enum
{
	Topbar = Sprites::Count,
	Background = Sprites::Count + 1,
	Count = 2
};
}

const StringHash SpritesFileNames[Sprites::Count + Ancillary::Count] =
{
	"clock-face.pvr",				// Clockface
	"hand.pvr",						// Hand
	"battery.pvr",					// Battery
	"internet-web-browser.pvr",		// Web
	"mail-message-new.pvr",			// Newmail
	"network-wireless.pvr",			// Network
	"office-calendar.pvr",			// Calendar

	"weather-sun-cloud-big.pvr",	// Weather_SUNCLOUD_BIG
	"weather-sun-cloud.pvr",		// Weather_SUNCLOUD
	"weather-rain.pvr",				// Weather_RAIN
	"weather-storm.pvr",			// Weather_STORM

	"container-corner.pvr",			// Container_CORNER
	"container-vertical.pvr",		// Container_VERT
	"container-horizontal.pvr",		// Container_HORI
	"container-filler.pvr",			// container_FILLER
	"vertical-bar.pvr",
	"text1.pvr",					// Text1
	"text2.pvr",					// Text2
	"loremipsum.pvr",
	"text-weather.pvr",				// Text_WEATHER
	"text-fri.pvr",					// Fri
	"text-sat.pvr",					// Sat
	"text-sun.pvr",					// Sun
	"text-mon.pvr",					// Mon

	"clock-face-small.pvr",			// ClockfaceSmall
	"hand-small.pvr",				// Hand_SMALL

	"window-bottom.pvr",			// Window_BOTTOM
	"window-bottomcorner.pvr",		// Window_BOTTOMCORNER
	"window-side.pvr",				// Window_SIDE
	"window-top.pvr",				// Window_TOP
	"window-topleft.pvr",			// Window_TOPLEFT
	"window-topright.pvr",			// Window_TOPRIGHT

	"topbar.pvr",					// Topbar
	"background.pvr",				// Background
};

// Displayed pages
namespace DisplayPage {
enum Enum
{
	Clocks,
	Weather,
	Window,

	Count,
	Default = Clocks
};
}

// Display option. Toggled with keyboard.
namespace DisplayOption {
enum Enum
{
	UI,
	TexAtlas,
	Count,
	Default = UI
};
}

// Display state
namespace DisplayState {
enum Enum
{
	Element,
	Transition,
	Default = Element
};
}


const char* const FragShaderFileName[ShaderNames::Count] =
{
	"TexColShader_vk.fsh.spv",    // ColorTexture
	"ColShader_vk.fsh.spv",     // ColorShader
};

const char* const VertShaderFileName[ShaderNames::Count] =
{
	"TexColShader_vk.vsh.spv",    // ColorTexture
	"ColShader_vk.vsh.spv",     // ColorShader
};

// Group shader programs and their uniform locations together

struct DrawPass
{
	pvr::api::DescriptorSet descSet;
	api::GraphicsPipeline  pipe;
};

struct SpriteDesc
{
	pvr::api::TextureView tex;
	pvr::uint32     uiWidth;
	pvr::uint32     uiHeight;
	pvr::uint32     uiSrcX;
	pvr::uint32     uiSrcY;
	bool          bHasAlpha;
	void release() {tex.reset();}
};

struct Vertex
{
	glm::vec4 vVert;
	glm::vec2 vUV;
};

struct SpriteClock
{
	pvr::ui::PixelGroup group;// root group
	pvr::ui::PixelGroup hand;// hand group contains hand sprite
	pvr::ui::Image clock;// clock sprite
	glm::vec2 scale;
};

struct SpriteContainer
{
	pvr::ui::PixelGroup group;
	pvr::Rectangle<pvr::float32> size;
};

struct PageClock
{
	pvr::ui::MatrixGroup group[(uint8)FrameworkCaps::MaxSwapChains];// root group
	void update(uint32 swapChain, pvr::float32 frameTime, const glm::mat4& trans);
	std::vector<SpriteClock> clock;
	SpriteContainer container;
	glm::mat4 projMtx;
};

struct PageWeather
{
	pvr::ui::MatrixGroup group[(uint8)FrameworkCaps::MaxSwapChains];
	void update(uint32 swapchain, const glm::mat4& transMtx);
	glm::mat4 projMtx;
	SpriteContainer containerTop, containerBottom;
};

struct PageWindow
{
	pvr::ui::MatrixGroup group[(uint8)FrameworkCaps::MaxSwapChains];
	utils::StructuredMemoryView clippingUboBuffer;
	api::DescriptorSet clippingUboDesc[4];
	void update(glm::mat4& proj, uint32 swapChain, pvr::float32 width, pvr::float32 height, const glm::mat4& trans);
	pvr::Rectanglei clipArea;
};

/*!*********************************************************************************************************************
\brief  Update the clock page
\param  screenWidth Screen width
\param  screenHeight Screen height
\param  frameTime Current frame
\param  trans Transformation matrix
***********************************************************************************************************************/
void PageClock::update(uint32 swapChain,pvr::float32 frameTime, const glm::mat4& trans)
{
	// to do render the container
	static pvr::float32 handRotate = 0.0f;
	handRotate -= frameTime * 0.001f;
	const pvr::float32 clockHandScale(.22f);
	pvr::uint32 i = 0;
	// right groups
	glm::vec2 clockOrigin(container.size.x + container.size.width, container.size.y + container.size.height);
	const glm::uvec2 smallClockDim(clock[0].group->getDimensions() * clock[0].scale);
	glm::uvec2 clockOffset(0, 0);
	pvr::uint32 clockIndex = 1;
	for (; i < clock.size() / 2; i += 2)
	{
		// the first two small clock (left & right) at the top closer.
		if (i < 2)
		{
			clock[i].hand->setRotation(handRotate + clockIndex)->setScale(glm::vec2(clockHandScale));
			clock[i].group->setAnchor(pvr::ui::Anchor::TopRight, clockOrigin);
			clock[i].group->setPixelOffset(-(int)smallClockDim.x * 2, 0);
			++clockIndex;

			clock[i + 1].hand->setRotation(handRotate + clockIndex)->setScale(glm::vec2(clockHandScale));
			clock[i + 1].group->setAnchor(pvr::ui::Anchor::TopLeft, glm::vec2(container.size.x, clockOrigin.y));
			clock[i + 1].group->setPixelOffset(smallClockDim.x * 2, 0);
			++clockIndex;
			continue;
		}

		clock[i].hand->setRotation(handRotate + clockIndex)->setScale(glm::vec2(clockHandScale));
		clock[i].group->setAnchor(pvr::ui::Anchor::TopRight, clockOrigin);
		clock[i].group->setPixelOffset(0, clockOffset.y);
		++clockIndex;

		clock[i + 1].hand->setRotation(handRotate + clockIndex)->setScale(glm::vec2(clockHandScale));
		clock[i + 1].group->setAnchor(pvr::ui::Anchor::TopRight, clockOrigin);
		clock[i + 1].group->setPixelOffset(-(int)smallClockDim.x, clockOffset.y);

		clockOffset.y -= smallClockDim.y;
		++clockIndex;
	}

	// left group
	clockOrigin = glm::vec2(container.size.x, container.size.y + container.size.height);
	clockOffset.y = 0;
	for (; i < clock.size() - 1; i += 2)
	{
		clock[i].hand->setRotation(handRotate + clockIndex)->setScale(glm::vec2(clockHandScale));
		clock[i].group->setAnchor(pvr::ui::Anchor::TopLeft, clockOrigin);
		clock[i].group->setPixelOffset(0, clockOffset.y);
		++clockIndex;

		clock[i + 1].hand->setRotation(handRotate + clockIndex)->setScale(glm::vec2(clockHandScale));
		clock[i + 1].group->setAnchor(pvr::ui::Anchor::TopLeft, clockOrigin);
		clock[i + 1].group->setPixelOffset(smallClockDim.x, clockOffset.y);
		clockOffset.y -= smallClockDim.y;
		++clockIndex;
	}
	//render the center clock
	clock[i].hand->setRotation(handRotate);
	clock[i].group->setAnchor(pvr::ui::Anchor::Center, glm::vec2(0))->setPixelOffset(0, 30);
	group[swapChain]->setScaleRotateTranslate(trans);// transform the entire group
	group[swapChain]->commitUpdates();
}

/*!*********************************************************************************************************************
\brief  Update the window page
\param  screenWidth Screen width
\param  screenHeight Screen height
\param  trans Transformation matrix
***********************************************************************************************************************/
void PageWindow::update(glm::mat4& proj, uint32 swapChain, pvr::float32 width, pvr::float32 height, const glm::mat4& trans)
{
	glm::vec2 offset(width * .5f, height * .5f);// center it on the screen
	// offset the clip area center to aligned with the center of the screen
	offset -= glm::vec2(clipArea.extent()) * .5f;

	glm::mat4 worldTrans = glm::translate(glm::vec3(offset, 0.0f)) * trans ;
	group[swapChain]->setScaleRotateTranslate(worldTrans);
	group[swapChain]->commitUpdates();

	//update the clipping ubo
	glm::mat4 scale = glm::scale(glm::vec3(glm::vec2(clipArea.extent()) / glm::vec2(width, height), 1.f));
	clippingUboBuffer.map(swapChain);
	clippingUboBuffer.setValue(0, proj * worldTrans * scale);
	clippingUboBuffer.unmap(swapChain);
}

/*!*********************************************************************************************************************
\brief  Update the weather page
\param  screenWidth Screen width
\param  screenHeight Screen height
\param  transMtx Transformation matrix
***********************************************************************************************************************/
void PageWeather::update(uint32 swapchain,const glm::mat4& transMtx)
{
	group[swapchain]->setScaleRotateTranslate(transMtx);
	group[swapchain]->commitUpdates();
}

class VulkanExampleUI;

/*!*********************************************************************************************************************
** Constants
***********************************************************************************************************************/
const char* const DisplayOpts[DisplayOption::Count] =
{
	"Displaying Interface",     // Ui
	"Displaying Texture Atlas",   // Texatlas
};

#ifdef DISPLAY_SPRITE_ALPHA
const char* const SpriteShaderDefines[] =
{
	"DISPLAY_SPRITE_ALPHA",
};
#else
const char** const SpriteShaderDefines = NULL;
#endif

static const pvr::uint32 DimDefault = 0xABCD;
static const pvr::uint32 DimCentre = 0xABCE;
static const pvr::float32 ByteToFloat = 1.0f / 255.0f;

static const char* const TextLoremIpsum =
  "Stencil Clipping\n\nLorem ipsum dolor sit amet, consectetuer adipiscing elit.\nDonec molestie. "
  "Sed aliquam sem ut arcu.\nPhasellus sollicitudin. Vestibulum condimentum facilisis nulla.\nIn "
  "hac habitasse platea dictumst. Nulla nonummy. Cras quis libero.\nCras venenatis. Aliquam posuere "
  "lobortis pede. Nullam fringilla urna id leo.\nPraesent aliquet pretium erat. Praesent non odio. "
  "Pellentesque a magna a\nmauris vulputate lacinia. Aenean viverra. Class aptent taciti sociosqu "
  "ad litora\ntorquent per conubia nostra, per inceptos hymenaeos. Aliquam\nlacus. Mauris magna eros, "
  "semper a, tempor et, rutrum et, tortor.";

class Area
{
private:
	pvr::int32 x;
	pvr::int32 y;
	pvr::int32 w;
	pvr::int32 h;
	pvr::int32 size;
	bool isFilled;

	Area* right;
	Area* left;

private:
	void setSize(pvr::int32 iWidth, pvr::int32 iHeight);
public:
	Area(pvr::int32 iWidth, pvr::int32 iHeight);
	Area();

	Area* insert(pvr::int32 iWidth, pvr::int32 iHeight);
	bool deleteArea();

	pvr::int32 getX()const;
	pvr::int32 getY()const;
};

class SpriteCompare
{
public:
	bool operator()(const SpriteDesc& pSpriteDescA, const SpriteDesc& pSpriteDescB)
	{
		pvr::uint32 uiASize = pSpriteDescA.uiWidth * pSpriteDescA.uiHeight;
		pvr::uint32 uiBSize = pSpriteDescB.uiWidth * pSpriteDescB.uiHeight;
		return (uiASize > uiBSize);
	}
};

class VulkanExampleUI : public pvr::Shell
{
private:
	enum {MaxSwapChains = 8};
	struct DeviceResource
	{
		api::GraphicsPipeline pipePreClip;
		api::GraphicsPipeline pipePostClip;

		pvr::api::TextureView textureAtlas;

		// Shader handles
		pvr::api::Shader vertexShader[ShaderNames::Count];
		pvr::api::Shader fragmentShader[ShaderNames::Count];

		// Programs
		api::GraphicsPipeline pipeSprite;

		api::DescriptorSetLayout texLayout;
		api::DescriptorSetLayout uboLayoutVert;
		api::DescriptorSetLayout uboLayoutFrag;

		pvr::api::GraphicsPipeline pipeClipping;
		pvr::api::Sampler samplerNearest;
		pvr::api::Sampler samplerBilinear;


		PageClock pageClock;
		PageWeather pageWeather;
		PageWindow pageWindow;
		SpriteContainer containerTop;
		api::Buffer quadVbo;

		pvr::api::Fbo fboAtlas[MaxSwapChains];
		Multi<api::Fbo> fboOnScreen;
		pvr::api::CommandBuffer cmdBuffer[MaxSwapChains];
		pvr::api::SecondaryCommandBuffer cmdBufferTitleDesc[MaxSwapChains];
		pvr::api::SecondaryCommandBuffer cmdBufferTexAtlas[MaxSwapChains];
		pvr::api::SecondaryCommandBuffer cmdBufferBaseUI[MaxSwapChains];
		pvr::api::SecondaryCommandBuffer cmdBufferClockPage[MaxSwapChains];
		pvr::api::SecondaryCommandBuffer cmdBufferWeatherpage[MaxSwapChains];
		pvr::api::SecondaryCommandBuffer cmdBufferWindow[MaxSwapChains];
		pvr::api::SecondaryCommandBuffer cmdBufferRenderUI[MaxSwapChains];

		SpriteDesc spritesDesc[Sprites::Count + Ancillary::Count];

		pvr::ui::Text textLorem;

		DrawPass drawPassAtlas;

		pvr::ui::Image spriteAtlas;
		pvr::ui::Image sprites[Sprites::Count + Ancillary::Count];

		pvr::ui::PixelGroup groupBaseUI;
	};
	std::auto_ptr<DeviceResource> deviceResource;
	pvr::ui::UIRenderer uiRenderer;
	bool isAtlasGenerated;

	// Transforms
	pvr::float32 clockHandRotate;
	pvr::float32 wndRotate;
	glm::mat4 transform;
	glm::mat4 projMtx;

	// Display options
	pvr::int32 displayOption;
	DisplayState::Enum state;
	pvr::float32 transitionPerc;
	DisplayPage::Enum currentPage;
	DisplayPage::Enum lastPage;
	pvr::int32 cycleDir;
	pvr::uint64 currTime;
	// Data
	pvr::int32 drawCallPerFrame;

	// Time
	pvr::float32 wndRotPerc;
	pvr::uint64 prevTransTime;
	pvr::uint64 prevTime;
	bool swipe;
	pvr::utils::AssetStore assetManager;
	pvr::GraphicsContext context;
	glm::vec2 screenScale;
	Rectanglef texAtlasRegions[Sprites::Count];

	void createFullScreenQuad()
	{
		uint32 width = getWidth();
		uint32 height = getHeight();
		Vertex vVerts[4] =
		{
			{ glm::vec4(0, height, 0, 1), glm::vec2(0, 1) }, // top left
			{ glm::vec4(0, 0, 0, 1), glm::vec2(0, 0)}, // bottom left
			{ glm::vec4(width, height, 0, 1), glm::vec2(1.f, 1.0f)},// top right
			{ glm::vec4(width, 0, 0, 1), glm::vec2(1, 0.f) }// bottom right
		};
		deviceResource->quadVbo = context->createBuffer(sizeof(vVerts), BufferBindingUse::VertexBuffer, true);
		void* mapData = deviceResource->quadVbo->map(types::MapBufferFlags::Write, 0, sizeof(vVerts));
		memcpy(mapData, vVerts,  sizeof(vVerts));
		deviceResource->quadVbo->unmap();
	}

	void updateTitleAndDesc(DisplayOption::Enum displayOption)
	{
		switch (displayOption)
		{
		case DisplayOption::UI:
			uiRenderer.getDefaultDescription()->setText("Displaying Interface");
			uiRenderer.getDefaultDescription()->commitUpdates();
			break;
		case DisplayOption::TexAtlas:
			uiRenderer.getDefaultDescription()->setText("Displaying Texture Atlas");
			uiRenderer.getDefaultDescription()->commitUpdates();
			break;
		}
		for (uint32 i = 0; i < getSwapChainLength(); ++i)
		{
			deviceResource->cmdBufferTitleDesc[i]->beginRecording(deviceResource->fboOnScreen[i], 0);
			uiRenderer.beginRendering(deviceResource->cmdBufferTitleDesc[i]);
			uiRenderer.getDefaultTitle()->render();
			uiRenderer.getDefaultDescription()->render();
			uiRenderer.getSdkLogo()->render();
			uiRenderer.endRendering();
			deviceResource->cmdBufferTitleDesc[i]->endRecording();
		}
	}
private:
	void drawScreenAlignedQuad(const api::GraphicsPipeline& pipe, api::DescriptorSet& ubo,
	                           pvr::api::CommandBufferBase cmdBuffer);
	void renderUI(uint32 swapChain);
	void renderPage(DisplayPage::Enum Page, const glm::mat4& mTransform, uint32 swapchain);

	bool loadSprites();
	bool createPipelines();
	void createBaseUI();
	void createPageWeather();
	void createPageWindow();
	void swipeLeft();
	void swipeRight();
	void eventMappedInput(pvr::SimplifiedInput action);

	float getVirtualWidth() {return (float)(isRotated() ? this->getHeight() : this->getWidth());}
	float getVirtualHeight() {return (float)(isRotated() ? this->getWidth() : this->getHeight());}
	float toDeviceX(float fVal) {return ((fVal / VirtualWidth) * getVirtualWidth());}
	float toDeviceY(float fVal) { return ((fVal / VirtualHeight) * getVirtualHeight()); }
	inline bool isRotated() { return this->isScreenRotated() && this->isFullScreen(); }

	bool createSamplersAndDescriptorSet();
	void createSpriteContainer(pvr::Rectangle<pvr::float32>const& rect, pvr::uint32 numSubContainer,
	                           pvr::float32 lowerContainerHeight, SpriteContainer& outContainer);
	void createPageClock();
	void createClockSprite(SpriteClock& outClock, Sprites::Enum sprite);
	void recordSecondaryCommandBuffers(uint32 swapChain);
public:
	VulkanExampleUI();
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();
};

/*!*********************************************************************************************************************
\brief    Constructor
***********************************************************************************************************************/
VulkanExampleUI::VulkanExampleUI() :
	isAtlasGenerated(false), clockHandRotate(0.0f),
	wndRotate(0.0f),  displayOption(DisplayOption::Default), state(DisplayState::Default),
	transitionPerc(0.0f), currentPage(DisplayPage::Default), lastPage(DisplayPage::Default),
	cycleDir(1), drawCallPerFrame(0), wndRotPerc(0.0f), prevTransTime(0), prevTime(0) {}

/*!********************************************************************************************************************
\brief Create Window page
***********************************************************************************************************************/
void VulkanExampleUI::createPageWindow()
{
	// create the window page
	deviceResource->textLorem = uiRenderer.createText(TextLoremIpsum);
	deviceResource->textLorem->setScale(glm::vec2(.5f));
	deviceResource->textLorem->setColor(0.0f, 0.0f, 0.0f, 1.0f);
	deviceResource->textLorem->setAnchor(pvr::ui::Anchor::BottomLeft, glm::vec2(-1.0f, -1.0f));
	deviceResource->pageWindow.clipArea = pvr::Rectanglei(0, 0, 390, 250);
	deviceResource->pageWindow.clipArea.x = pvr::int32(deviceResource->pageWindow.clipArea.x * screenScale.x);
	deviceResource->pageWindow.clipArea.y = pvr::int32(deviceResource->pageWindow.clipArea.y * screenScale.y);
	deviceResource->pageWindow.clipArea.width = pvr::int32(deviceResource->pageWindow.clipArea.width * screenScale.x);
	deviceResource->pageWindow.clipArea.height = pvr::int32(deviceResource->pageWindow.clipArea.height * screenScale.y);
	for(uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		deviceResource->pageWindow.group[i] = uiRenderer.createMatrixGroup();
		deviceResource->pageWindow.group[i]->setViewProjection(projMtx);
		deviceResource->pageWindow.group[i]->add(deviceResource->textLorem);
		deviceResource->pageWindow.group[i]->commitUpdates();
	}
}

/*!*********************************************************************************************************************
\brief Create sprite container
\param[in] rect Container rectangle
\param[in] numSubContainer Number of lower sub containers
\param[in] lowerContainerHeight lower container height
\param[out] outContainer Returned Sprite container
***********************************************************************************************************************/
void VulkanExampleUI::createSpriteContainer(pvr::Rectangle<pvr::float32>const& rect,
    pvr::uint32 numSubContainer, pvr::float32 lowerContainerHeight, SpriteContainer& outContainer)
{
	outContainer.size = rect;
	outContainer.group = uiRenderer.createPixelGroup();

	// calculate the border of the container
	const pvr::float32 borderX = deviceResource->sprites[Sprites::ContainerHorizontal]->getWidth() /
	                             uiRenderer.getRenderingDimX() * 2.f;

	const pvr::float32 borderY = deviceResource->sprites[Sprites::ContainerCorner]->getHeight() /
	                             uiRenderer.getRenderingDimY() * 2.f;

	pvr::Rectangle<pvr::float32> rectVerticleLeft(rect.x, rect.y + borderY, borderX, rect.height - borderY * 2);
	pvr::Rectangle<pvr::float32> rectVerticleRight(rect.x + rect.width, rect.y + borderY, rect.width, rect.height - borderY * 2);
	pvr::Rectangle<pvr::float32> rectTopHorizontal(rect.x + borderX, rect.y + rect.height - borderY, rect.width - borderX * 2, rect.height);
	pvr::Rectangle<pvr::float32> rectBottomHorizontal(rect.x  + borderX, rect.y, rect.width - borderX * 2, rect.y + borderY);

	// align the sprites to lower left so they will be aligned with their group
	deviceResource->sprites[Sprites::ContainerCorner]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.0f, -1.0f);
	deviceResource->sprites[Sprites::ContainerVertical]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.0f, -1.0f);
	deviceResource->sprites[Sprites::ContainerHorizontal]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.f, -1.f);

	// add the filler
	{
		pvr::ui::PixelGroup filler = uiRenderer.createPixelGroup();
		filler->add(deviceResource->sprites[Sprites::ContainerFiller]);
		deviceResource->sprites[Sprites::ContainerFiller]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.f, -1.f);
		filler->setAnchor(pvr::ui::Anchor::BottomLeft, rect.x + borderX, rect.y + borderY);

		filler->setScale(glm::vec2(.5f * (rect.width - borderX * 2 /*minus the left and right borders*/) *
		                           uiRenderer.getRenderingDimX() / deviceResource->sprites[Sprites::ContainerFiller]->getWidth(),
		                           .501f * (rect.height - borderY * 2/*minus Top and Bottom borders*/) *
		                           uiRenderer.getRenderingDimY() / deviceResource->sprites[Sprites::ContainerFiller]->getHeight()));

		outContainer.group->add(filler);
		outContainer.group->setSize(glm::vec2(uiRenderer.getRenderingDimX(), uiRenderer.getRenderingDimY()));
	}

	// Top Left Corner
	{
		pvr::ui::PixelGroup newGroup = uiRenderer.createPixelGroup();
		// place the center at the
		newGroup->add(deviceResource->sprites[Sprites::ContainerCorner]);
		newGroup->setAnchor(pvr::ui::Anchor::BottomRight, rectTopHorizontal.x, rectTopHorizontal.y);
		outContainer.group->add(newGroup);
	}

	//Top Right Corner
	{
		pvr::ui::PixelGroup newGroup = uiRenderer.createPixelGroup();
		newGroup->add(deviceResource->sprites[Sprites::ContainerCorner]);
		// flip the x coordinate by negative scale
		newGroup->setAnchor(pvr::ui::Anchor::BottomRight, rectTopHorizontal.x + rectTopHorizontal.width,
		                    rectTopHorizontal.y)->setScale(glm::vec2(-1.f, 1.0f));
		outContainer.group->add(newGroup);
	}

	//bottom left Corner
	{
		pvr::ui::PixelGroup newGroup = uiRenderer.createPixelGroup();
		newGroup->add(deviceResource->sprites[Sprites::ContainerCorner]);
		// flip the y coordinates
		newGroup->setAnchor(pvr::ui::Anchor::BottomRight, rectBottomHorizontal.x,
		                    rectBottomHorizontal.height)->setScale(glm::vec2(1.f, -1.0f));
		outContainer.group->add(newGroup);
	}

	//Bottom right Corner
	{
		pvr::ui::PixelGroup newGroup = uiRenderer.createPixelGroup();
		newGroup->add(deviceResource->sprites[Sprites::ContainerCorner]);
		// flip the x and y coordinates
		newGroup->setAnchor(pvr::ui::Anchor::BottomRight, rectBottomHorizontal.x + rectBottomHorizontal.width,
		                    rectBottomHorizontal.height)->setScale(glm::vec2(-1.f, -1.0f));
		outContainer.group->add(newGroup);
	}

	// Horizontal Up
	{
		//calculate the width of the sprite
		float32 width = (rectTopHorizontal.width * .5f * uiRenderer.getRenderingDimX() /
		                 deviceResource->sprites[Sprites::ContainerVertical]->getWidth());
		pvr::ui::PixelGroup horizontal = uiRenderer.createPixelGroup();
		horizontal->add(deviceResource->sprites[Sprites::ContainerVertical]);
		horizontal->setAnchor(pvr::ui::Anchor::BottomLeft, rectTopHorizontal.x, rectTopHorizontal.y);
		horizontal->setScale(glm::vec2(width, 1.f));
		outContainer.group->add(horizontal);
	}

	// Horizontal Down
	{
		//calculate the width of the sprite
		float32 width = (rectBottomHorizontal.width * .5f * uiRenderer.getRenderingDimX() /
		                 deviceResource->sprites[Sprites::ContainerVertical]->getWidth());
		pvr::ui::PixelGroup horizontal = uiRenderer.createPixelGroup();
		horizontal->add(deviceResource->sprites[Sprites::ContainerVertical]);
		horizontal->setAnchor(pvr::ui::Anchor::TopLeft, rectBottomHorizontal.x, rectBottomHorizontal.y);
		horizontal->setScale(glm::vec2(width, -1.f));
		outContainer.group->add(horizontal);
	}

	// Vertical Left
	{
		//calculate the height of the sprite
		float32  height = (rectVerticleLeft.height * .501f * uiRenderer.getRenderingDimY() /
		                   deviceResource->sprites[Sprites::ContainerHorizontal]->getHeight());
		pvr::ui::PixelGroup verticle = uiRenderer.createPixelGroup();
		verticle->add(deviceResource->sprites[Sprites::ContainerHorizontal]);
		verticle->setScale(glm::vec2(1, height))->setAnchor(pvr::ui::Anchor::BottomLeft, rectVerticleLeft.x,
		    rectVerticleLeft.y)->setPixelOffset(0, 0);
		outContainer.group->add(verticle);
	}

	// Vertical Right
	{
		//calculate the height of the sprite
		float32 height = (rectVerticleRight.height * .501f * uiRenderer.getRenderingDimY() /
		                  deviceResource->sprites[Sprites::ContainerHorizontal]->getHeight());
		pvr::ui::PixelGroup vertical = uiRenderer.createPixelGroup();
		vertical->add(deviceResource->sprites[Sprites::ContainerHorizontal]);
		vertical->setScale(glm::vec2(-1, height))->setAnchor(pvr::ui::Anchor::BottomLeft,
		    rectVerticleRight.x, rectVerticleRight.y);
		outContainer.group->add(vertical);
	}

	float32 width = 1.f / uiRenderer.getRenderingDimX() * deviceResource->sprites[Sprites::ContainerHorizontal]->getWidth();
	float32  height = (outContainer.size.height - outContainer.size.y) * .5f;

	// calculate the each container size
	pvr::float32 containerWidth = rect.width / numSubContainer;
	pvr::float32 borderWidth = 1.f / uiRenderer.getRenderingDimX() * deviceResource->sprites[Sprites::VerticalBar]->getWidth();
	pvr::Rectangle<pvr::float32> subRect(rect.x, rect.y, rect.x + containerWidth, rect.y + lowerContainerHeight);
	height = .5f * (subRect.height - subRect.y) * uiRenderer.getRenderingDimY() /
	         deviceResource->sprites[Sprites::VerticalBar]->getHeight();
	// create the lower containers

	// Horizontal Split
	{
		// half it here because the scaling happen at the center
		width = (rect.width * .5f * uiRenderer.getRenderingDimX() /
		         deviceResource->sprites[Sprites::VerticalBar]->getHeight());
		width -= .25;// reduce the width by quarter of a pixel so they fit well between the container
		pvr::ui::PixelGroup horizontal = uiRenderer.createPixelGroup();
		horizontal->add(deviceResource->sprites[Sprites::VerticalBar]);
		horizontal->setScale(glm::vec2(1.f, width))->setAnchor(pvr::ui::Anchor::BottomLeft,
		    rect.x + (2 / uiRenderer.getRenderingDimX())/*offset it by 2 pixel*/, subRect.height);
		horizontal->setRotation(glm::pi<pvr::float32>() * -.5f);// rotate y 90 degree
		outContainer.group->add(horizontal);
	}

	for (pvr::uint32 i = 0; i < numSubContainer - 1; ++i)
	{
		pvr::ui::PixelGroup groupVertical = uiRenderer.createPixelGroup();
		deviceResource->sprites[Sprites::VerticalBar]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.f, -1.f);
		groupVertical->add(deviceResource->sprites[Sprites::VerticalBar]);
		groupVertical->setAnchor(pvr::ui::Anchor::BottomLeft, subRect.width, subRect.y)
		->setScale(glm::vec2(1, height));
		outContainer.group->add(groupVertical);
		subRect.x = subRect.x + containerWidth - borderWidth;
		subRect.width = subRect.width + containerWidth;
	}
	deviceResource->containerTop = outContainer;
}

/*!*********************************************************************************************************************
\brief  Code in initApplication() will be called by PVRShell once per run, before the rendering context is created.
    Used to initialize variables that are not Dependant on it (e.g. external modules, loading meshes, etc.)
    If the rendering context is lost, InitApplication() will not be called again.
\return Return pvr::Result::Success if no error occurred
***********************************************************************************************************************/
pvr::Result VulkanExampleUI::initApplication()
{
	assetManager.init(*this);
	setStencilBitsPerPixel(8);
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief  create the weather page
***********************************************************************************************************************/
void VulkanExampleUI::createPageWeather()
{
	// background
	pvr::ui::PixelGroup backGround = uiRenderer.createPixelGroup();
	backGround->add(deviceResource->sprites[Ancillary::Background]);

	// create the weather page
	std::vector<ui::Sprite> groupsList;


	SpriteContainer container;
	createSpriteContainer(deviceResource->pageClock.container.size, 4, LowerContainerHeight, container);
	deviceResource->pageWeather.containerTop = container;
	groupsList.push_back(container.group);
	pvr::ui::PixelGroup group = uiRenderer.createPixelGroup();

	// align the sprite with its parent group
	deviceResource->sprites[Sprites::TextWeather]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.0f, -1.f);
	group->setScale(screenScale);
	group->add(deviceResource->sprites[Sprites::TextWeather]);
	const glm::vec2& containerHalfSize = deviceResource->pageWeather.containerTop.size.extent() * .5f;
	group->setAnchor(pvr::ui::Anchor::CenterLeft, deviceResource->pageWeather.containerTop.size.x,
					 deviceResource->pageWeather.containerTop.size.center().y)->setPixelOffset(10, 40);
	groupsList.push_back(group);

	// add the Weather
	group = uiRenderer.createPixelGroup();
	group->add(deviceResource->sprites[Sprites::WeatherSunCloudBig]);
	// align the sprite with its parent group
	deviceResource->sprites[Sprites::WeatherSunCloudBig]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.0f, -1.f);
	group->setAnchor(pvr::ui::Anchor::Center, deviceResource->pageWeather.containerTop.size.x + containerHalfSize.x,
					 deviceResource->pageWeather.containerTop.size.y + containerHalfSize.y)->setPixelOffset(0, 40);
	group->setScale(screenScale);
	groupsList.push_back(group);

	// create the bottom 4 groups
	const Sprites::Enum sprites[] =
	{
		Sprites::WeatherSunCloud, Sprites::TextFriday,
		Sprites::WeatherSunCloud, Sprites::TextSaturday,
		Sprites::WeatherRain, Sprites::TextSunday,
		Sprites::WeatherStorm, Sprites::TextMonday
	};

	const pvr::float32 width = deviceResource->pageWeather.containerTop.size.width / 4.f;
	pvr::float32 tempOffsetX = deviceResource->pageWeather.containerTop.size.x + (width * .5f);;

	for (pvr::uint32 i = 0; i < 8; i += 2)
	{
		Sprites::Enum weather = sprites[i];
		Sprites::Enum text = sprites[i + 1];

		group = uiRenderer.createPixelGroup();
		// align the sprite with its parent group
		this->deviceResource->sprites[weather]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.0f, -1.f);
		group->add(this->deviceResource->sprites[weather]);
		group->setAnchor(pvr::ui::Anchor::BottomCenter, tempOffsetX, deviceResource->pageWeather.containerTop.size.y);
		group->setScale(screenScale);
		groupsList.push_back(group);

		//add the text
		group = uiRenderer.createPixelGroup();
		// align the text with its parent group
		this->deviceResource->sprites[text]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.0f, -1.f);
		group->add(this->deviceResource->sprites[text]);
		group->setAnchor(pvr::ui::Anchor::TopCenter, tempOffsetX, deviceResource->pageWeather.containerTop.size.y
						 + LowerContainerHeight)->setPixelOffset(0, -5);

		group->setScale(screenScale);
		groupsList.push_back(group);
		tempOffsetX = tempOffsetX + width;
	}

	for(uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		deviceResource->pageWeather.group[i] = uiRenderer.createMatrixGroup();
		deviceResource->pageWeather.group[i]->add(groupsList.data(), groupsList.size());
		deviceResource->pageWeather.group[i]->setViewProjection(projMtx);
		deviceResource->pageWeather.group[i]->commitUpdates();
	}




}

/*!*********************************************************************************************************************
\brief  Create clock sprite
\param  outClock Returned clock
\param  sprite Clock Sprite to create
***********************************************************************************************************************/
void VulkanExampleUI::createClockSprite(SpriteClock& outClock, Sprites::Enum sprite)
{
	// create a group of clock and hand so they can be transformed
	outClock.group = uiRenderer.createPixelGroup();
	outClock.clock = deviceResource->sprites[sprite];
	outClock.hand = uiRenderer.createPixelGroup();

	// clock half size in ndc
	glm::vec2 halfDim = outClock.clock->getDimensions() / uiRenderer.getRenderingDim();

	outClock.hand->add(deviceResource->sprites[Sprites::Hand]);
	outClock.group->add(outClock.clock);
	outClock.group->add(outClock.hand);

	// set the size of the parent group
	outClock.group->setSize(outClock.clock->getDimensions());

	// center the clock's to the center of the parent group
	outClock.clock->setAnchor(pvr::ui::Anchor::Center, 0.0f, 0.0f);

	// center the hand group so that it can be rotated at the center of the clock
	outClock.hand->setSize(deviceResource->sprites[Sprites::Hand]->getDimensions())->setAnchor(pvr::ui::Anchor::BottomCenter, .0f, .0f);
	// center the clock hand bottom center and offset it by few pixel so they can be rotated at that point
	deviceResource->sprites[Sprites::Hand]->setAnchor(pvr::ui::Anchor::BottomCenter, glm::vec2(0.0f, -1.f))->setPixelOffset(0, -10);
}

/*!*********************************************************************************************************************
\brief  Create clock page
***********************************************************************************************************************/
void VulkanExampleUI::createPageClock()
{
	SpriteContainer container;
	const pvr::uint32 numClocksInColumn = 5;
	pvr::float32 containerHeight = deviceResource->sprites[Sprites::ClockfaceSmall]->getDimensions().y * numClocksInColumn / BaseDimY;
	containerHeight += LowerContainerHeight * .5f;// add the lower container height as well
	pvr::float32 containerWidth = deviceResource->sprites[Sprites::ClockfaceSmall]->getDimensions().x * 4;
	containerWidth += deviceResource->sprites[Sprites::Clockface]->getDimensions().x;
	containerWidth /= BaseDimX;

	pvr::Rectangle<pvr::float32> containerRect(-containerWidth, -containerHeight,
	    containerWidth * 2.f, containerHeight * 2.f);
	createSpriteContainer(pvr::Rectangle<pvr::float32>(containerRect), 2, LowerContainerHeight, container);
	deviceResource->pageClock.container = container;

	ui::Sprite groupSprites[NumClocks + 3];
	uint32 i = 0;
	for (; i < NumClocks; ++i)
	{
		SpriteClock clock;
		createClockSprite(clock, Sprites::ClockfaceSmall);
		clock.group->setScale(screenScale);
		clock.scale = screenScale;
		// add the clock group in to page group
		groupSprites[i] = clock.group;
		deviceResource->pageClock.clock.push_back(clock);// add the clock
	}

	// add the center clock
	// group the hands
	SpriteClock clockCenter;
	createClockSprite(clockCenter, Sprites::Clockface);
	clockCenter.group->setScale(screenScale);
	groupSprites[i++] = clockCenter.group;
	deviceResource->pageClock.clock.push_back(clockCenter);

	deviceResource->sprites[Sprites::Text1]->setAnchor(pvr::ui::Anchor::BottomLeft,
	    glm::vec2(deviceResource->pageClock.container.size.x,
	              deviceResource->pageClock.container.size.y))->setPixelOffset(0, 10);
	deviceResource->sprites[Sprites::Text1]->setScale(screenScale);
	groupSprites[i++] = deviceResource->sprites[Sprites::Text1];

	deviceResource->sprites[Sprites::Text2]->setAnchor(pvr::ui::Anchor::BottomRight,
	    glm::vec2(deviceResource->pageClock.container.size.width +
				  deviceResource->pageClock.container.size.x - 0.05f,
	              deviceResource->pageClock.container.size.y))->setPixelOffset(0, 10);
	deviceResource->sprites[Sprites::Text2]->setScale(screenScale);
	groupSprites[i++] =deviceResource->sprites[Sprites::Text2];

	for(uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		deviceResource->pageClock.group[i] = uiRenderer.createMatrixGroup();
		pvr::ui::MatrixGroup groupBorder  = uiRenderer.createMatrixGroup();
		groupBorder->add(deviceResource->sprites[Sprites::ContainerVertical]);
		groupBorder->setScaleRotateTranslate(glm::translate(glm::vec3(0.0f, -0.45f, 0.0f))
											 * glm::scale(glm::vec3(.65f, .055f, .2f)));
		deviceResource->pageClock.group[i]->add(deviceResource->containerTop.group);
		deviceResource->pageClock.group[i]->add(groupSprites, ARRAY_SIZE(groupSprites));
		deviceResource->pageClock.group[i]->setViewProjection(projMtx);
		deviceResource->pageClock.group[i]->commitUpdates();
	}
}

/*!*********************************************************************************************************************
\brief  Create base UI
***********************************************************************************************************************/
void VulkanExampleUI::createBaseUI()
{
	// build the render base UI
	pvr::float32 offset = 0.f;
	pvr::int32 offsetPixel = 10;
	// battery sprite
	deviceResource->sprites[Sprites::Battery]->setAnchor(pvr::ui::Anchor::TopRight, glm::vec2(1.f, 1.0f));
	offset -= deviceResource->sprites[Sprites::Battery]->getDimensions().x + offsetPixel;

	// web sprite
	deviceResource->sprites[Sprites::Web]->setAnchor(pvr::ui::Anchor::TopRight, glm::vec2(1.f, 1.0))->setPixelOffset((pvr::int32)offset, 0);
	offset -= deviceResource->sprites[Sprites::Web]->getDimensions().x + offsetPixel;

	// new mail sprite
	deviceResource->sprites[Sprites::Newmail]->setAnchor(pvr::ui::Anchor::TopRight, glm::vec2(1.f, 1.0f))->setPixelOffset((pvr::int32)offset, 0);
	offset -= deviceResource->sprites[Sprites::Newmail]->getDimensions().x + offsetPixel;;

	// network sprite
	deviceResource->sprites[Sprites::Network]->setAnchor(pvr::ui::Anchor::TopRight, glm::vec2(1.f, 1.0f))->setPixelOffset((pvr::int32)offset, 0);
	deviceResource->groupBaseUI = uiRenderer.createPixelGroup();

	pvr::ui::PixelGroup horizontalTopBarGrop = uiRenderer.createPixelGroup();
	deviceResource->sprites[Ancillary::Topbar]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.f, -1.f);
	horizontalTopBarGrop->add(deviceResource->sprites[Ancillary::Topbar]);
	horizontalTopBarGrop->setAnchor(pvr::ui::Anchor::TopLeft, -1.f, 1.f);
	horizontalTopBarGrop->setScale(glm::vec2(uiRenderer.getRenderingDimX() * .5f, 1.0f));

	deviceResource->groupBaseUI
	->add(deviceResource->sprites[Ancillary::Background])
	->add(horizontalTopBarGrop)
	->add(deviceResource->sprites[Sprites::Battery])
	->add(deviceResource->sprites[Sprites::Web])
	->add(deviceResource->sprites[Sprites::Newmail])
	->add(deviceResource->sprites[Sprites::Network]);

	glm::vec2 scale = glm::vec2(deviceResource->sprites[Ancillary::Background]->getWidth(), deviceResource->sprites[Ancillary::Background]->getHeight());
	scale = 2.5f / scale;
	scale *= glm::vec2(getWidth(), getHeight());
	deviceResource->sprites[Ancillary::Background]->setAnchor(pvr::ui::Anchor::TopLeft, -1.f, 1.f)->setScale(scale);

	deviceResource->groupBaseUI
	->setSize(glm::vec2(uiRenderer.getRenderingDimX(), uiRenderer.getRenderingDimY()))
	->setAnchor(pvr::ui::Anchor::TopRight, glm::vec2(1.0f, 1.0f));

	deviceResource->groupBaseUI->commitUpdates();// update once here
}

/*!*********************************************************************************************************************
\brief  Code in initView() will be called by PVRShell upon initialization or after a change in the rendering context.
        Used to initialize variables that are Dependant on the rendering context (e.g. textures, vertex buffers, etc.)
\return Return true if no error occurred
***********************************************************************************************************************/
pvr::Result VulkanExampleUI::initView()
{
	deviceResource.reset(new DeviceResource());
	context = getGraphicsContext();
	deviceResource->fboOnScreen = context->createOnScreenFboSet(types::LoadOp::Clear,
	                              types::StoreOp::Store, types::LoadOp::Clear, types::StoreOp::Store,
	                              types::LoadOp::Clear, types::StoreOp::Store);

	for (pvr::uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		deviceResource->cmdBuffer[i] = context->createCommandBufferOnDefaultPool();
		deviceResource->cmdBufferTitleDesc[i] = context->createSecondaryCommandBufferOnDefaultPool();
	}
	// Initialize uiRenderer
	if (uiRenderer.init(deviceResource->fboOnScreen[0]->getRenderPass(), 0, 1024) != pvr::Result::Success)
	{
		this->setExitMessage("ERROR: Cannot initialize Print3D\n");
		return pvr::Result::NotInitialized;
	}
	screenScale = glm::vec2(glm::min(uiRenderer.getRenderingDim().x / BaseDimX, uiRenderer.getRenderingDim().y / BaseDimY));
	prevTransTime = this->getTime();

	// Load the sprites
	if (!loadSprites()) { return pvr::Result::NotInitialized; }

	// Load the shaders
	if (!createPipelines())
	{
		this->setExitMessage("Failed to create pipelines");
		return pvr::Result::NotInitialized;
	}
	createFullScreenQuad();
	if (!createSamplersAndDescriptorSet())
	{
		pvr::Log("Failed to create Texture and samplers Descriptor sets");
		return pvr::Result::NotInitialized;
	}

	if (isScreenRotated())
	{
		projMtx = pvr::math::ortho(context->getApiType(), 0.f, (pvr::float32) getHeight(), 0.f,
		                           (pvr::float32)getWidth(), 0.0f);
	}
	else
	{
		projMtx = pvr::math::ortho(context->getApiType(), 0.f, (pvr::float32)getWidth(), 0.f,
		                           (pvr::float32)getHeight(), 0.0f);
	}
	swipe = false;
	// set the default title
	uiRenderer.getDefaultTitle()->setText("Example UI");
	uiRenderer.getDefaultTitle()->commitUpdates();
	// create the base ui which will be the same for all the pages
	createBaseUI();
	createPageClock();
	createPageWeather();
	createPageWindow();

	for (pvr::uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		recordSecondaryCommandBuffers(i);
	}


	updateTitleAndDesc((DisplayOption::Enum)displayOption);

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief  Loads sprites that will be used to create a texture atlas.
\return Return true if no error occurred
***********************************************************************************************************************/
bool VulkanExampleUI::loadSprites()
{
	pvr::assets::SamplerCreateParam samplerInfo;
	samplerInfo.minificationFilter = SamplerFilter::Nearest;
	samplerInfo.magnificationFilter = SamplerFilter::Nearest;
	samplerInfo.mipMappingFilter = SamplerFilter::None;
	samplerInfo.wrapModeU = SamplerWrap::Clamp;
	samplerInfo.wrapModeV = SamplerWrap::Clamp;
	pvr::api::Sampler samplerNearest = context->createSampler(samplerInfo);

	samplerInfo.minificationFilter = samplerInfo.magnificationFilter = SamplerFilter::Linear;

	api::Sampler samplerBilinear = context->createSampler(samplerInfo);

	pvr::TextureHeader header;
	// Load sprites and add to sprite array so that we can generate a texture atlas from them.
	for (pvr::uint32 i = 0; i < Sprites::Count + Ancillary::Count; i++)
	{
		if (!assetManager.getTextureWithCaching(context, SpritesFileNames[i], &deviceResource->spritesDesc[i].tex, &header))
		{
			pvr::Log("Failed to load texture %s", SpritesFileNames[i].c_str());
			return false;
		}
		// Copy some useful data out of the texture header.
		deviceResource->spritesDesc[i].uiWidth = header.getWidth();
		deviceResource->spritesDesc[i].uiHeight = header.getHeight();

		const pvr::uint8* pixelString = header.getPixelFormat().getPixelTypeChar();

		if (header.getPixelFormat().getPixelTypeId() == (uint64)pvr::CompressedPixelFormat::PVRTCI_2bpp_RGBA ||
		    header.getPixelFormat().getPixelTypeId() == (uint64)pvr::CompressedPixelFormat::PVRTCI_4bpp_RGBA ||
		    pixelString[0] == 'a' || pixelString[1] == 'a' || pixelString[2] == 'a' || pixelString[3] == 'a')
		{
			deviceResource->spritesDesc[i].bHasAlpha = true;
		}
		else
		{
			deviceResource->spritesDesc[i].bHasAlpha = false;
		}


		deviceResource->sprites[i] = uiRenderer.createImage(deviceResource->spritesDesc[i].tex, header.getWidth(), header.getHeight(), samplerNearest);
	}
	TextureHeader atlasHeader;
	if (!assetManager.generateTextureAtlas(context, SpritesFileNames, texAtlasRegions, Sprites::Count,
	                                       &deviceResource->textureAtlas, &atlasHeader))
	{
		setExitMessage("Failed to generate the texture atlas");
		return false;
	}

	deviceResource->spriteAtlas = uiRenderer.createImage(deviceResource->textureAtlas, atlasHeader.getWidth(), atlasHeader.getHeight());
	deviceResource->spriteAtlas->setScale(glm::vec2(.75));
	deviceResource->spriteAtlas->commitUpdates();

	return true;
}

/*!*********************************************************************************************************************
\brief  Create nearest and bilinear sampler, and descriptor set for texture atlas
\return Return true if no error occurred
***********************************************************************************************************************/
bool VulkanExampleUI::createSamplersAndDescriptorSet()
{
	// create the samplers.
	pvr::assets::SamplerCreateParam samplerInfo;

	// create bilinear sampler
	samplerInfo.minificationFilter = SamplerFilter::Linear;
	samplerInfo.magnificationFilter = SamplerFilter::Linear;
	deviceResource->samplerBilinear = context->createSampler(samplerInfo);

	// create point sampler
	samplerInfo.minificationFilter = SamplerFilter::Nearest;
	samplerInfo.magnificationFilter = SamplerFilter::Nearest;
	deviceResource->samplerNearest = context->createSampler(samplerInfo);

	pvr::api::DescriptorSetLayoutCreateParam descSetLayoutInfo;
	descSetLayoutInfo.setBinding(0, DescriptorType::CombinedImageSampler, 1, ShaderStageFlags::Fragment);
	pvr::api::DescriptorSetUpdate descSetInfo;
	pvr::api::DescriptorSet descSetTexAtlas;
	descSetInfo.setCombinedImageSampler(0, deviceResource->textureAtlas, deviceResource->samplerBilinear);
	descSetTexAtlas = context->createDescriptorSetOnDefaultPool(deviceResource->texLayout);
	descSetTexAtlas->update(descSetInfo);

	// setup the draw pass atlas
	deviceResource->drawPassAtlas.descSet = descSetTexAtlas;

	// set up the page window ubo
	auto& ubo = deviceResource->pageWindow.clippingUboBuffer;
	ubo.addEntryPacked("MVP", GpuDatatypes::mat4x4);
	ubo.finalize(getGraphicsContext(), 1, types::BufferBindingUse::UniformBuffer, false, false);
	for (uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		api::DescriptorSet& uboDesc = deviceResource->pageWindow.clippingUboDesc[i];
		ubo.connectWithBuffer(i, getGraphicsContext()->createBufferAndView(ubo.getAlignedElementSize(),
		                      types::BufferBindingUse::UniformBuffer, true));
		uboDesc = getGraphicsContext()->createDescriptorSetOnDefaultPool(
		            deviceResource->pipePreClip->getPipelineLayout()->getDescriptorSetLayout(0));
		uboDesc->update(api::DescriptorSetUpdate().setUbo(0, ubo.getConnectedBuffer(i)));
	}
	return true;
}

/*!*********************************************************************************************************************
\brief create graphics pipeline for texture-atlas, pre-clip and post-clip pass.
\return  Return true if no error occurred
***********************************************************************************************************************/
bool VulkanExampleUI::createPipelines()
{
	// create the descriptorsetLayout and pipelineLayout
	pvr::api::DescriptorSetLayoutCreateParam descSetLayoutInfo;

	deviceResource->texLayout = context->createDescriptorSetLayout(
	                              pvr::api::DescriptorSetLayoutCreateParam().setBinding(0,
	                                  DescriptorType::CombinedImageSampler, 1, ShaderStageFlags::Fragment));

	deviceResource->uboLayoutVert = context->createDescriptorSetLayout(
	                                  pvr::api::DescriptorSetLayoutCreateParam().setBinding(0,
	                                      DescriptorType::UniformBuffer, 1, ShaderStageFlags::Vertex));

	deviceResource->uboLayoutFrag = context->createDescriptorSetLayout(
	                                  pvr::api::DescriptorSetLayoutCreateParam().setBinding(0,
	                                      DescriptorType::UniformBuffer, 1, ShaderStageFlags::Fragment));

	pvr::assets::ShaderFile shaderVersioning;

	// create the vertex and fragment shaders
	for (pvr::uint32 i = 0; i < ShaderNames::Count; ++i)
	{
		shaderVersioning.populateValidVersions(VertShaderFileName[i], *this);
		deviceResource->vertexShader[i] =
		  context->createShader(*shaderVersioning.getBestStreamForApi(context->getApiType()),
		                        ShaderType::VertexShader, NULL, 0);

		shaderVersioning.populateValidVersions(FragShaderFileName[i], *this);
		deviceResource->fragmentShader[i] =
		  context->createShader(*shaderVersioning.getBestStreamForApi(context->getApiType()),
		                        ShaderType::FragmentShader, NULL, 0);

		if (deviceResource->vertexShader[i].isNull() || deviceResource->fragmentShader[i].isNull())
		{
			Log("Failed to create the shaders");
			return false;
		}
	}

	// --- texture-atlas pipeline
	{


		pvr::api::GraphicsPipelineCreateParam pipeInfo;
		pipeInfo.rasterizer.setCullFace(Face::None);
		pipeInfo.pipelineLayout = context->createPipelineLayout(pvr::api::PipelineLayoutCreateParam()
		                          .addDescSetLayout(deviceResource->uboLayoutVert)
		                          .addDescSetLayout(deviceResource->texLayout)
		                          .addDescSetLayout(deviceResource->uboLayoutFrag));

		pipeInfo.vertexShader = deviceResource->vertexShader[ShaderNames::ColorTexture];
		pipeInfo.fragmentShader = deviceResource->fragmentShader[ShaderNames::ColorTexture];

		pipeInfo.vertexInput
		.addVertexAttribute(0, 0, pvr::assets::VertexAttributeLayout(DataType::Float32, 4, 0))
		.addVertexAttribute(1, 0, pvr::assets::VertexAttributeLayout(DataType::Float32, 2, sizeof(glm::vec4)));

		pipeInfo.vertexInput.setInputBinding(0, sizeof(Vertex));
		pipeInfo.inputAssembler.setPrimitiveTopology(PrimitiveTopology::TriangleStrip);
		pipeInfo.colorBlend.setAttachmentState(0, types::BlendingConfig());
		pipeInfo.depthStencil.setDepthTestEnable(false).setDepthWrite(false);
		pipeInfo.renderPass = deviceResource->fboOnScreen[0]->getRenderPass();
		deviceResource->drawPassAtlas.pipe = context->createGraphicsPipeline(pipeInfo);

		if (deviceResource->drawPassAtlas.pipe.isNull())
		{
			pvr::Log("Failed to create TexColor pipeline");
			return false;
		}
	}

	// --- pre-clip pipeline
	{
		pvr::api::GraphicsPipelineCreateParam pipeInfo;
		types::BlendingConfig colorAttachment;
		pipeInfo.pipelineLayout = context->createPipelineLayout(pvr::api::PipelineLayoutCreateParam()
		                          .setDescSetLayout(0, deviceResource->uboLayoutVert));

		pipeInfo.vertexShader = deviceResource->vertexShader[ShaderNames::ColorShader];
		pipeInfo.fragmentShader = deviceResource->fragmentShader[ShaderNames::ColorShader];

		pipeInfo.vertexInput
		.addVertexAttribute(0, 0, pvr::assets::VertexAttributeLayout(DataType::Float32, 4, 0))
		.addVertexAttribute(1, 0, pvr::assets::VertexAttributeLayout(DataType::Float32, 2, sizeof(glm::vec4)));

		pipeInfo.vertexInput.setInputBinding(0, sizeof(Vertex));
		pipeInfo.colorBlend.setAttachmentState(0, colorAttachment);
		pipeInfo.inputAssembler.setPrimitiveTopology(PrimitiveTopology::TriangleStrip);
		pipeInfo.rasterizer.setCullFace(Face::Back);
		pipeInfo.renderPass = deviceResource->fboOnScreen[0]->getRenderPass();


		//- Set stencil function to always pass, and write 0x1 in to the stencil buffer.
		//- disable depth write and depth test
		api::StencilState stencilState;
		stencilState.opDepthPass = StencilOp::Replace;
		stencilState.compareOp = ComparisonMode::Always;
		stencilState.writeMask = 0xffffffff;
		stencilState.reference = 1;
		pipeInfo.depthStencil.setStencilFrontBack(stencilState).setStencilTest(true);
		pipeInfo.colorBlend.setAttachmentState(0, colorAttachment);

		pipeInfo.depthStencil
		.setStencilFrontBack(stencilState)
		.setDepthTestEnable(true)
		.setDepthWrite(false);

		deviceResource->pipePreClip = context->createGraphicsPipeline(pipeInfo);
		if (deviceResource->pipePreClip.isNull())
		{
			pvr::Log("Failed to create pre clip pipeline");
			return false;
		}
	}

	// --- post clip pipeline
	{

		// copy the create param from the parent
		pvr::api::GraphicsPipelineCreateParam pipeInfo = uiRenderer.getPipeline()->getCreateParam();
		pipeInfo.depthStencil.setDepthTestEnable(false).setDepthWrite(false).setStencilTest(true);
		// Set stencil function to always pass, and write 0x1 in to the stencil buffer.
		pvr::api::StencilState stencilState;
		stencilState.compareOp = ComparisonMode::Equal;
		stencilState.compareMask = 0xffffffff;
		stencilState.reference = 1;
		pipeInfo.depthStencil.setStencilFrontBack(stencilState);
		types::BlendingConfig colorAttachment;
		colorAttachment.blendEnable = true;
		colorAttachment.srcBlendColor = colorAttachment.srcBlendAlpha = pvr::types::BlendFactor::SrcAlpha;
		colorAttachment.destBlendColor = colorAttachment.destBlendAlpha = pvr::types::BlendFactor::OneMinusSrcAlpha;
		pipeInfo.colorBlend.setAttachmentState(0, colorAttachment);

		deviceResource->pipePostClip = context->createGraphicsPipeline(pipeInfo,
		                               pvr::api::ParentableGraphicsPipeline(uiRenderer.getPipeline()));
		if (deviceResource->pipePostClip.isNull())
		{
			pvr::Log("Failed to create post clip pipeline");
			return false;
		}
	}
	return true;
}

/*!*********************************************************************************************************************
\brief  Renders a 2D quad with the given parameters. DstRect is the rectangle to be rendered in
    world coordinates. SrcRect is the rectangle to be cropped from the texture in pixel coordinates.
    NOTE: This is not an optimized function and should not be called repeatedly to draw quads to the screen at render time.
***********************************************************************************************************************/
void VulkanExampleUI::drawScreenAlignedQuad(const api::GraphicsPipeline& pipe,
    api::DescriptorSet& ubo, pvr::api::CommandBufferBase cmdBuffer)
{
	cmdBuffer->bindDescriptorSet(pipe->getPipelineLayout(), 0, ubo);
	cmdBuffer->bindVertexBuffer(deviceResource->quadVbo, 0, 0);
	cmdBuffer->drawArrays(0, 4, 0, 1);
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result VulkanExampleUI::releaseView()
{
	// release all the textures and sprites
	uiRenderer.release();
	assetManager.releaseAll();
	deviceResource.reset();
	context.release();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief  Code in quitApplication() will be called by PVRShell once per run, just before exiting
    the program. If the rendering context is lost, quitApplication() will not be called.
\return Return pvr::Result::Success if no error occurred
***********************************************************************************************************************/
pvr::Result VulkanExampleUI::quitApplication() { return pvr::Result::Success; }

/*!*********************************************************************************************************************
\brief  Render the page
\param  page Page to render
\param  mTransform Transformation matrix
***********************************************************************************************************************/
void VulkanExampleUI::renderPage(DisplayPage::Enum page, const glm::mat4& mTransform, uint32 swapChain)
{
	switch (page)
	{
	case DisplayPage::Clocks:
		deviceResource->pageClock.update(getSwapChainIndex(), pvr::float32(getFrameTime()), mTransform);
		deviceResource->cmdBuffer[swapChain]->enqueueSecondaryCmds(deviceResource->cmdBufferClockPage[swapChain]);
		break;
	case DisplayPage::Weather:
		deviceResource->pageWeather.update(getSwapChainIndex(), mTransform);
		deviceResource->cmdBuffer[swapChain]->enqueueSecondaryCmds(deviceResource->cmdBufferWeatherpage[swapChain]);
		break;
	case DisplayPage::Window:
		deviceResource->pageWindow.update(projMtx, swapChain, uiRenderer.getRenderingDimX(),
		                                  uiRenderer.getRenderingDimY(), mTransform);
		deviceResource->cmdBuffer[swapChain]->enqueueSecondaryCmds(deviceResource->cmdBufferWindow[swapChain]);
		break;
	}
}

/*!*********************************************************************************************************************
\brief  Renders the default interface.
***********************************************************************************************************************/
void VulkanExampleUI::renderUI(uint32 swapChain)
{
	deviceResource->cmdBuffer[swapChain]->beginRenderPass(deviceResource->fboOnScreen[swapChain],
	    pvr::Rectanglei(0, 0, getWidth(), getHeight()), false, glm::vec4(.3f, .3f, 0.3f, 0.f));

	// render the baseUI
	deviceResource->cmdBuffer[swapChain]->enqueueSecondaryCmds(deviceResource->cmdBufferBaseUI[swapChain]);

	if (state == DisplayState::Element)
	{
		// A transformation matri,x
		if (currentPage == DisplayPage::Window)
		{
			glm::mat4 vRot, vCentre, vInv;
			vRot = glm::rotate(wndRotate, glm::vec3(0.0f, 0.0f, 1.0f));

			vCentre = glm::translate(glm::vec3(-uiRenderer.getRenderingDim() * .5f, 0.0f));

			vInv = glm::inverse(vCentre);
			const glm::vec2 rotateOrigin = -glm::vec2(deviceResource->pageWindow.clipArea.extent()) * glm::vec2(.5f);

			vCentre = glm::translate(glm::vec3(rotateOrigin, 0.0f));
			vInv = glm::inverse(vCentre);

			// align the group center to the center of the rotation, rotate and translate it back.
			transform =  vInv * vRot * vCentre;

		}
		else
		{
			transform = glm::mat4(1.f);
		}
		// Just render the single, current page
		renderPage(currentPage, transform, swapChain);
	}
	else if (state == DisplayState::Transition)
	{
		//--- Render outward group
		pvr::float32 fX = pvr::math::quadraticEaseIn(0.f, -uiRenderer.getRenderingDimX() * cycleDir, transitionPerc);
		transform = glm::translate(glm::vec3(fX, 0.0f, 0.0f));

		//  the last page page
		renderPage(lastPage, transform, swapChain);

		// --- Render inward group
		fX = pvr::math::quadraticEaseIn(uiRenderer.getRenderingDimX() *  cycleDir, 0.0f, transitionPerc);
		transform = glm::translate(glm::vec3(fX, 0.0f, 0.0f));

		//Render page
		renderPage(currentPage, transform, swapChain);
	}
	// record draw title and description commands
	deviceResource->cmdBuffer[swapChain]->enqueueSecondaryCmds(deviceResource->cmdBufferTitleDesc[swapChain]);
	deviceResource->cmdBuffer[swapChain]->endRenderPass();
}

/*!*********************************************************************************************************************
\brief  Swipe left
***********************************************************************************************************************/
void VulkanExampleUI::swipeLeft()
{
	if (currentPage == 0) {  return; }
	swipe = true;
	cycleDir = -1;
}

/*!*********************************************************************************************************************
\brief  Swipe right
***********************************************************************************************************************/
void VulkanExampleUI::swipeRight()
{
	if (currentPage == DisplayPage::Count - 1) { return; }
	swipe = true;
	cycleDir = 1;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result VulkanExampleUI::renderFrame()
{
	// begin recording the command buffer
	deviceResource->cmdBuffer[getSwapChainIndex()]->beginRecording();
	currTime = this->getTime();
	pvr::float32 deltaTime = (currTime - prevTime) * 0.001f;
	prevTime = currTime;

	// Update Window rotation
	wndRotPerc += (1.0f / UiDisplayTime) * deltaTime;
	wndRotate = pvr::math::quadraticEaseOut(0.0f, glm::pi<pvr::float32>() * 2.f, wndRotPerc);
	// Check to see if we should transition to a new page (if we're not already)
	if ((currTime - prevTransTime > UiDisplayTimeInMs && state != DisplayState::Transition) || swipe)
	{
		// Switch to next page
		state = DisplayState::Transition;
		transitionPerc = 0.0f;
		lastPage = currentPage;

		// Cycle pages
		pvr::int32 nextPage = currentPage + cycleDir;
		if (nextPage >= DisplayPage::Count || nextPage < 0)
		{
			cycleDir *= -1;             // Reverse direction
			nextPage = currentPage + cycleDir;  // Recalculate
		}
		currentPage = (DisplayPage::Enum)nextPage;
		swipe = false;
	}

	// Calculate next transition amount
	if (state == DisplayState::Transition)
	{
		transitionPerc += 0.01666f; // 60 FPS
		if (transitionPerc > 1.f)
		{
			state = DisplayState::Element;
			transitionPerc = 1.f;
			wndRotate = 0.0f;     // Reset Window rotation
			wndRotPerc = 0.0f;    // Reset Window rotation percentage
			prevTransTime = currTime; // Reset time
		}
	}

	drawCallPerFrame = 0;
	renderUI(getSwapChainIndex());
	// record commands to draw the title and description
	deviceResource->cmdBuffer[getSwapChainIndex()]->endRecording();
	deviceResource->cmdBuffer[getSwapChainIndex()]->submit();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief  Record secondary command buffer for drawing texture atlas, clock page, weather page and Window page
***********************************************************************************************************************/
void VulkanExampleUI::recordSecondaryCommandBuffers(uint32 swapChain)
{
	// record the base ui
	{
		deviceResource->cmdBufferBaseUI[swapChain] = context->createSecondaryCommandBufferOnDefaultPool();
		uiRenderer.beginRendering(deviceResource->cmdBufferBaseUI[swapChain]);
		deviceResource->groupBaseUI->render();//render the base GUI
		uiRenderer.endRendering();
	}

	// record DrawClock commands
	{
		deviceResource->cmdBufferClockPage[swapChain] = context->createSecondaryCommandBufferOnDefaultPool();
		uiRenderer.beginRendering(deviceResource->cmdBufferClockPage[swapChain], deviceResource->fboOnScreen[swapChain]);
		deviceResource->pageClock.group[swapChain]->render();
		uiRenderer.endRendering();
	}

	// record draw weather commands
	{
		deviceResource->cmdBufferWeatherpage[swapChain] = context->createSecondaryCommandBufferOnDefaultPool();
		deviceResource->cmdBufferWeatherpage[swapChain]->beginRecording(deviceResource->fboOnScreen[swapChain]);
		uiRenderer.beginRendering(deviceResource->cmdBufferWeatherpage[swapChain], deviceResource->fboOnScreen[swapChain]);
		deviceResource->pageWeather.group[swapChain]->render();
		uiRenderer.endRendering();
		deviceResource->cmdBufferWeatherpage[swapChain]->endRecording();
	}

	// record draw Window commands
	{
		deviceResource->cmdBufferWindow[swapChain] = context->createSecondaryCommandBufferOnDefaultPool();
		deviceResource->cmdBufferWindow[swapChain]->beginRecording(deviceResource->fboOnScreen[swapChain], 0);

		// clear the stencil buffer to 0
		deviceResource->cmdBufferWindow[swapChain]->clearStencilAttachment(pvr::Rectanglei(0, 0,
		    (pvr::int32)uiRenderer.getRenderingDimX(), (pvr::int32)uiRenderer.getRenderingDimY()), 0);

		// bind the pre-clipping pipeline
		deviceResource->cmdBufferWindow[swapChain]->bindPipeline(deviceResource->pipePreClip);

		// draw a quad only in to the stencil buffer
		drawScreenAlignedQuad(deviceResource->pipePreClip, deviceResource->pageWindow.clippingUboDesc[swapChain],
		                      deviceResource->cmdBufferWindow[swapChain]);
		// bind the post clip pipeline and render the text only where the stencil passes
		uiRenderer.beginRendering(deviceResource->cmdBufferWindow[swapChain], deviceResource->pipePostClip,
		                          deviceResource->fboOnScreen[swapChain]);
		deviceResource->pageWindow.group[swapChain]->render();
		uiRenderer.endRendering();
		deviceResource->cmdBufferWindow[swapChain]->endRecording();
	}
}

/*!*********************************************************************************************************************
\brief Handle input events
\param[in] action Input event to handle
***********************************************************************************************************************/
void VulkanExampleUI::eventMappedInput(pvr::SimplifiedInput action)
{
	switch (action)
	{
	case pvr::SimplifiedInput::Right:
		swipeLeft();
		break;
	case pvr::SimplifiedInput::Left:
		swipeRight();
		break;
	case pvr::SimplifiedInput::ActionClose:// quit the application
		this->exitShell();
		break;
	}
}

/*!*********************************************************************************************************************
\brief  Constructor
\param[in] width Area width
\param[in] height Area height
***********************************************************************************************************************/
Area::Area(pvr::int32 width, pvr::int32 height) : x(0), y(0), isFilled(false), right(NULL), left(NULL) { setSize(width, height); }

/*!*********************************************************************************************************************
\brief  Constructor
***********************************************************************************************************************/
Area::Area() : x(0), y(0), isFilled(false), right(NULL), left(NULL) { setSize(0, 0); }

/*!*********************************************************************************************************************
\brief  Calculates an area where there's sufficient space or returns NULL if no space could be found.
\return Return a pointer to the area added, else NULL if it fails
\param  width Area width
\param  height Area height
***********************************************************************************************************************/
Area* Area::insert(pvr::int32 width, pvr::int32 height)
{
	// If this area has branches below it (i.e. is not a leaf) then traverse those.
	// Check the left branch first.
	if (left)
	{
		Area* tempPtr = NULL;
		tempPtr = left->insert(width, height);
		if (tempPtr != NULL) { return tempPtr; }
	}
	// Now check right
	if (right) { return right->insert(width, height); }
	// Already filled!
	if (isFilled) { return NULL; }

	// Too small
	if (size < width * height || w < width || h < height) { return NULL; }

	// Just right!
	if (size == width * height && w == width && h == height)
	{
		isFilled = true;
		return this;
	}
	// Too big. Split up.
	if (size > width * height && w >= width && h >= height)
	{
		// Initializes the children, and sets the left child's coordinates as these don't change.
		left = new Area;
		right = new Area;
		left->x = x;
		left->y = y;

		// --- Splits the current area depending on the size and position of the placed texture.
		// Splits vertically if larger free distance across the texture.
		if ((w - width) > (h - height))
		{
			left->w = width;
			left->h = h;

			right->x = x + width;
			right->y = y;
			right->w = w - width;
			right->h = h;
		}
		// Splits horizontally if larger or equal free distance downwards.
		else
		{
			left->w = w;
			left->h = height;

			right->x = x;
			right->y = y + height;
			right->w = w;
			right->h = h - height;
		}

		//Initializes the child members' size attributes.
		left->size = left->h  * left->w;
		right->size = right->h * right->w;

		//Inserts the texture into the left child member.
		return left->insert(width, height);
	}
	//Catch all error return.
	return NULL;
}

/*!*********************************************************************************************************************
\brief  Deletes the given area.
\return  Return true on success
***********************************************************************************************************************/
bool Area::deleteArea()
{
	if (left != NULL)
	{
		if (left->left != NULL)
		{
			if (!left->deleteArea())  { return false; }
			if (!right->deleteArea()) { return false; }
		}
	}
	if (right != NULL)
	{
		if (right->left != NULL)
		{
			if (!left->deleteArea())  { return false; }
			if (!right->deleteArea()) { return false; }
		}
	}
	delete right;
	right = NULL;
	delete left;
	left = NULL;
	return true;
}

/*!*********************************************************************************************************************
\brief  set the area size
\param  width Area width
\param  height Area height
***********************************************************************************************************************/
void Area::setSize(pvr::int32 width, pvr::int32 height)
{
	w = width;  h = height; size = width * height;
}

/*!*********************************************************************************************************************
\brief  Get the X position of the area.
\return Return the area's x position
***********************************************************************************************************************/
inline pvr::int32 Area::getX()const {return x;}

/*!*********************************************************************************************************************
\brief  get the Y position of the area.
\return Return the area's y position
***********************************************************************************************************************/
inline pvr::int32 Area::getY()const { return y; }

/*!*********************************************************************************************************************
\brief  This function must be implemented by the user of the shell.The user should return its pvr::Shell object defining
        the behavior of the application.
\return Return The demo supplied by the user
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() {return std::auto_ptr<pvr::Shell>(new VulkanExampleUI());}
