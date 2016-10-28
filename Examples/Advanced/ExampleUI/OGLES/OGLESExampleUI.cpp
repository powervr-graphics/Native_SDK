/*!*********************************************************************************************************************
\File         OGLESExampleUI.cpp
\Title        ExampleUI
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Demonstrates how to efficiently render UI and sprites using UIRenderer
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVRUIRenderer/PVRUIRenderer.h"

const pvr::uint32 AtlasWidth		= 1024;
const pvr::uint32 AtlasHeight		= 1024;
const pvr::uint32 NullQuadPix		= 4;
const pvr::uint32 VirtualWidth		= 640;
const pvr::uint32 VirtualHeight		= 480;
const pvr::uint32 AtlasPixelBorder  = 1;
const pvr::uint32 UiDisplayTime		= 5;// Display each page for 5 seconds
const pvr::uint32 UiDisplayTimeInMs = UiDisplayTime * 1000;
const glm::vec2 BaseDim(800, 600);
const pvr::float32 LowerContainerHight = .3f;
#define ELEMENTS_IN_ARRAY(x)		(sizeof(x) / sizeof(x[0]))
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
	Background = Sprites::Count,
	Topbar = Sprites::Count + 1,
	Count = 2
};
}

const char* const SpritesFileNames[Sprites::Count + Ancillary::Count] =
{
	"clock-face.pvr",			// Clockface
	"hand.pvr",					// Hand
	"battery.pvr",				// Battery
	"internet-web-browser.pvr",	// Web
	"mail-message-new.pvr",		// Newmail
	"network-wireless.pvr",		// Network
	"office-calendar.pvr",		// Calendar

	"weather-sun-cloud-big.pvr",// Weather_SUNCLOUD_BIG
	"weather-sun-cloud.pvr",	// Weather_SUNCLOUD
	"weather-rain.pvr",			// Weather_RAIN
	"weather-storm.pvr",		// Weather_STORM

	"container-corner.pvr",		// Container_CORNER
	"container-vertical.pvr",	// Container_VERT
	"container-horizontal.pvr",	// Container_HORI
	"container-filler.pvr",     // container_FILLER
	"vertical-bar.pvr",
	"text1.pvr",				// Text1
	"text2.pvr",				// Text2
	"loremipsum.pvr",
	"text-weather.pvr",			// Text_WEATHER
	"text-fri.pvr",				// Fri
	"text-sat.pvr",				// Sat
	"text-sun.pvr",				// Sun
	"text-mon.pvr",				// Mon

	"clock-face-small.pvr",		// ClockfaceSmall
	"hand-small.pvr",			// Hand_SMALL

	"window-bottom.pvr",		// Window_BOTTOM
	"window-bottomcorner.pvr",	// Window_BOTTOMCORNER
	"window-side.pvr",			// Window_SIDE
	"window-top.pvr",			// Window_TOP
	"window-topleft.pvr",		// Window_TOPLEFT
	"window-topright.pvr",		// Window_TOPRIGHT

	"background.pvr",		// Background
	"topbar.pvr",			// Topbar
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
	"TexColShaderF.fsh",		// ColorTexture
	"ColShaderF.fsh",			// ColorShader
};

const char* const VertShaderFileName[ShaderNames::Count] =
{
	"TexColShaderV.vsh",		// ColorTexture
	"ColShaderV.vsh",			// ColorShader
};

// Group shader programs and their uniform locations together
struct Pipeline
{
	pvr::api::GraphicsPipeline pipe;
	pvr::uint32 mvpLoc;
	pvr::uint32 transMtxLoc;
	pvr::uint32 rgbaLoc;
};

struct DrawPass
{
	pvr::api::DescriptorSet descSet;
	Pipeline		 pipe;
};

struct SpriteDesc
{
	pvr::api::TextureView tex;
	pvr::uint32			uiWidth;
	pvr::uint32			uiHeight;
	pvr::uint32			uiSrcX;
	pvr::uint32			uiSrcY;
	bool			    bHasAlpha;
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
	pvr::ui::MatrixGroup group;// root group
	void update(pvr::float32 frameTime, const glm::mat4& trans);
	std::vector<SpriteClock> clock;
	SpriteContainer container;
	glm::mat4 projMtx;
	static pvr::uint32 numClocks;
};
pvr::uint32 PageClock::numClocks = 22;

struct PageWeather
{
	pvr::ui::MatrixGroup group;
	void update(const glm::mat4& transMtx);
	glm::mat4 projMtx;
	SpriteContainer containerTop, containerBottom;
};

struct PageWindow
{
	pvr::ui::MatrixGroup group;
	glm::mat4 mvp;
	glm::mat4 proj;
	void update(pvr::float32 width, pvr::float32 height, const glm::mat4& trans);
	pvr::Rectanglei clipArea;
};

/*!*********************************************************************************************************************
\brief	Update the clock page
\param	screenWidth Screen width
\param	screenHeight Screen height
\param	frameTime Current frame
\param	trans Transformation matrix
***********************************************************************************************************************/
void PageClock::update(pvr::float32 frameTime, const glm::mat4& trans)
{
	// to do render the container
	static pvr::float32 handRotate = 0.0f;
	handRotate -= frameTime * 0.001f;
	pvr::float32 clockHandScale(.22f);
	pvr::uint32 i = 0;
	// right groups
	glm::vec2 clockOrigin(container.size.width, container.size.height);
	glm::uvec2 smallClockDim(clock[0].group->getDimensions() * clock[0].scale);
	glm::uvec2 clockOffset(0, 0);
	pvr::uint32 clockIndex = 1;
	for (; i < clock.size() / 2; i += 2)
	{
		if (i < 2)
		{
			clock[i].hand->setRotation(handRotate + clockIndex)->setScale(glm::vec2(clockHandScale));
			clock[i].group->setAnchor(pvr::ui::Anchor::TopRight, clockOrigin);
			clock[i].group->setPixelOffset(-(int)smallClockDim.x * 2, 0);
			++clockIndex;

			clock[i + 1].hand->setRotation(handRotate + clockIndex)->setScale(glm::vec2(clockHandScale));
			clock[i + 1].group->setAnchor(pvr::ui::Anchor::TopLeft, glm::vec2(container.size.x, container.size.height));
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
	clockOrigin = glm::vec2(container.size.x, container.size.height);
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
	group->setScaleRotateTranslate(trans);// transform the entire group
	group->commitUpdates();
}

/*!*********************************************************************************************************************
\brief	Update the window page
\param	screenWidth Screen width
\param	screenHeight Screen height
\param	trans Transformation matrix
***********************************************************************************************************************/
void PageWindow::update(pvr::float32 width, pvr::float32 height, const glm::mat4& trans)
{
	glm::vec2 offset(width * .5f, height * .5f);// center it on the screen
	// offset it so the clip center aligned to the center of the screen
	offset.x -= clipArea.getDimension().x * .5f;
	offset.y -= clipArea.getDimension().y * .25f;

	glm::mat4 worldTrans = trans * glm::translate(glm::vec3(offset, 0.0f));
	mvp = proj * worldTrans;
	group->setScaleRotateTranslate(worldTrans);
	group->commitUpdates();
}

/*!*********************************************************************************************************************
\brief	Update the weather page
\param	screenWidth Screen width
\param	screenHeight Screen height
\param	transMtx Transformation matrix
***********************************************************************************************************************/
void PageWeather::update(const glm::mat4& transMtx)
{
	group->setScaleRotateTranslate(transMtx);
	group->commitUpdates();
}

class OGLESExampleUI;

/*!*********************************************************************************************************************
** Constants
***********************************************************************************************************************/
const char* const DisplayOpts[DisplayOption::Count] =
{
	"Displaying Interface",			// Ui
	"Displaying Texture Atlas",		// Texatlas
};

#ifdef DISPLAY_SPRITE_ALPHA
const char* const SpriteShaderDefines[] =
{
	"DISPLAY_SPRITE_ALPHA",
};
#else
const char** const SpriteShaderDefines = NULL;
#endif

const char* const* ShaderDefines[ShaderNames::Count] =
{
	NULL,								// eTEXCOL_SHADER
	NULL,								// eTEXCOL_SHADER
};

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
	pvr::int32		x;
	pvr::int32		y;
	pvr::int32		w;
	pvr::int32		h;
	pvr::int32		size;
	bool			isFilled;

	Area*			right;
	Area*			left;

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

class OGLESExampleUI : public pvr::Shell
{
private:
	struct DeviceResource
	{
		Pipeline			pipePreClip;
		Pipeline			pipePostClip;

		pvr::api::TextureView	textureAtlas;

		// Shader handles
		pvr::api::Shader		vertexShader[ShaderNames::Count];
		pvr::api::Shader		fragmentShader[ShaderNames::Count];

		// Programs
		Pipeline		pipeSprite;
		Pipeline		pipeColor;
		pvr::api::GraphicsPipeline pipeClipping;
		pvr::api::Sampler	samplerNearest;
		pvr::api::Sampler samplerBilinear;

		pvr::api::Fbo	fboAtlas;
		pvr::api::Fbo	fboOnScreen;
		pvr::api::CommandBuffer cmdBuffer;
		pvr::api::SecondaryCommandBuffer cmdBufferTitleDesc;
		pvr::api::SecondaryCommandBuffer cmdBufferTexAtlas;
		pvr::api::SecondaryCommandBuffer cmdBufferBaseUI;
		pvr::api::SecondaryCommandBuffer cmdBufferClockPage;
		pvr::api::SecondaryCommandBuffer cmdBufferWeatherpage;
		pvr::api::SecondaryCommandBuffer cmdBufferWindow;
		pvr::api::SecondaryCommandBuffer cmdBufferRenderUI;

		pvr::api::Buffer					quadVbo;

		SpriteDesc							spritesDesc[Sprites::Count + Ancillary::Count];
		pvr::ui::Text						textLorem;
		DrawPass							drawPassAtlas;
		pvr::ui::Image						spriteAtlas;
		pvr::ui::Image						sprites[Sprites::Count + Ancillary::Count];
		pvr::ui::PixelGroup					groupBaseUI;

		PageClock							pageClock;
		PageWeather							pageWeather;
		PageWindow							pageWindow;
		SpriteContainer						containerTop;

		pvr::api::AssetStore				assetManager;
		pvr::ui::UIRenderer					uiRenderer;
	};
	std::auto_ptr<DeviceResource> deviceResource;
	bool		isAtlasGenerated;

	// Transforms
	pvr::float32 clockHandRotate;
	pvr::float32 wndRotate;
	glm::mat4 transform;
	glm::mat4 projMtx;

	// Display options
	pvr::int32			displayOption;
	DisplayState::Enum	state;
	pvr::float32		transitionPerc;
	DisplayPage::Enum	currentPage;
	DisplayPage::Enum	lastPage;
	pvr::int32			cycleDir;
	pvr::uint64 currTime;
	// Data
	pvr::int32			drawCallPerFrame;

	// Time
	pvr::float32	wndRotPerc;
	pvr::uint64		prevTransTime;
	pvr::uint64  	prevTime;
	bool			swipe;
	pvr::GraphicsContext  context;
	glm::vec2 screenScale;
	void updateTitleAndDesc(DisplayOption::Enum displayOption)
	{
		switch (displayOption)
		{
		case DisplayOption::UI:
			deviceResource->uiRenderer.getDefaultDescription()->setText("Displaying Interface");
			deviceResource->uiRenderer.getDefaultDescription()->commitUpdates();
			break;
		case DisplayOption::TexAtlas:
			deviceResource->uiRenderer.getDefaultDescription()->setText("Displaying Texture Atlas");
			deviceResource->uiRenderer.getDefaultDescription()->commitUpdates();
			break;
		}
		deviceResource->cmdBufferTitleDesc->beginRecording(deviceResource->fboOnScreen, 0);
		deviceResource->uiRenderer.beginRendering(deviceResource->cmdBufferTitleDesc);
		deviceResource->uiRenderer.getDefaultTitle()->render();
		deviceResource->uiRenderer.getDefaultDescription()->render();
		deviceResource->uiRenderer.getSdkLogo()->render();
		deviceResource->uiRenderer.endRendering();
		deviceResource->cmdBufferTitleDesc->endRecording();
	}
private:
	void drawScreenAlignedQuad(const Pipeline& Shader, const pvr::Rectangle<pvr::float32>& DstRect,
	                           pvr::api::CommandBufferBase cmdBuffer, const pvr::Rectangle<pvr::float32>& SrcRect =
	                             pvr::Rectangle<pvr::float32>(0, 0, 1, 1), const pvr::uint32 uiRGBA = 0xFFFFFFFF);

	void renderUI();
	void renderPage(DisplayPage::Enum Page, const glm::mat4& mTransform);
	void renderAtlas();

	bool loadSprites();
	bool createPipelines();
	bool generateAtlas();
	void createBaseUI();
	bool createFbo();
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
	void createSpriteContainer(pvr::Rectangle<pvr::float32>const& rect, pvr::uint32 numSubContainer, pvr::float32 lowerContainerHeight, SpriteContainer& outContainer);
	void createPageClock();
	void createClockSprite(SpriteClock& outClock, Sprites::Enum sprite);
	void recordSecondaryCommandBuffers();
public:
	OGLESExampleUI();
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();
};

/*!*********************************************************************************************************************
\brief		Constructor
***********************************************************************************************************************/
OGLESExampleUI::OGLESExampleUI() :
	isAtlasGenerated(false), clockHandRotate(0.0f),
	wndRotate(0.0f),  displayOption(DisplayOption::Default), state(DisplayState::Default),
	transitionPerc(0.0f), currentPage(DisplayPage::Default), lastPage(DisplayPage::Default),
	cycleDir(1), drawCallPerFrame(0), wndRotPerc(0.0f), prevTransTime(0), prevTime(0) {}

/*!********************************************************************************************************************
\brief Create Window page
***********************************************************************************************************************/
void OGLESExampleUI::createPageWindow()
{
	// create the window page
	deviceResource->pageWindow.group = deviceResource->uiRenderer.createMatrixGroup();
	deviceResource->textLorem = deviceResource->uiRenderer.createText(TextLoremIpsum);
	deviceResource->textLorem->setAnchor(pvr::ui::Anchor::BottomLeft, glm::vec2(-1.0f, -1.0f));
	deviceResource->pageWindow.proj = deviceResource->uiRenderer.getScreenRotation() * projMtx;
	deviceResource->pageWindow.group->setViewProjection(deviceResource->pageWindow.proj);
	deviceResource->pageWindow.clipArea = pvr::Rectanglei(0, -50, 390, 250);
	deviceResource->pageWindow.clipArea.x = pvr::int32(deviceResource->pageWindow.clipArea.x * screenScale.x);
	deviceResource->pageWindow.clipArea.y = pvr::int32(deviceResource->pageWindow.clipArea.y * screenScale.y);
	deviceResource->pageWindow.clipArea.width = pvr::int32(deviceResource->pageWindow.clipArea.width * screenScale.x);
	deviceResource->pageWindow.clipArea.height = pvr::int32(deviceResource->pageWindow.clipArea.height * screenScale.y);
	deviceResource->textLorem->setScale(glm::vec2(.5f));
	deviceResource->textLorem->setColor(0.0f, 0.0f, 0.0f, 1.0f);
	deviceResource->sprites[Sprites::WindowSide]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.f, -1.f);
	deviceResource->pageWindow.group->add(deviceResource->textLorem);
}

/*!*********************************************************************************************************************
\brief Create sprite container
\param[in] rect Container rectangle
\param[in] numSubContainer Number of lower sub containers
\param[in] lowerContainerHeight lower container height
\param[out] outContainer Returned Sprite container
***********************************************************************************************************************/
void OGLESExampleUI::createSpriteContainer(pvr::Rectangle<pvr::float32>const& rect,
    pvr::uint32 numSubContainer, pvr::float32 lowerContainerHeight, SpriteContainer& outContainer)
{
	outContainer.size = rect;
	outContainer.group = deviceResource->uiRenderer.createPixelGroup();

	// calculate the border of the container
	const pvr::float32 borderX = deviceResource->sprites[Sprites::ContainerHorizontal]->getWidth() /
	                             deviceResource->uiRenderer.getRenderingDimX() * 2.f;
	const pvr::float32 borderY = deviceResource->sprites[Sprites::ContainerCorner]->getHeight() /
	                             deviceResource->uiRenderer.getRenderingDimY() * 2.f;

	pvr::Rectangle<pvr::float32> rectVerticleLeft(rect.x, rect.y + borderY, rect.x + borderX, rect.height - borderY);
	pvr::Rectangle<pvr::float32> rectVerticleRight(rect.width - borderX, rect.y + borderY, rect.width, rect.height - borderY);
	pvr::Rectangle<pvr::float32> rectTopHorizontal(rect.x + borderX, rect.height - borderY, rect.width - borderX, rect.height);
	pvr::Rectangle<pvr::float32> rectBottomHorizontal(rect.x  + borderX , rect.y, rect.width - borderX, rect.y + borderY);

	// align the sprites to lower left so they will be aligned with their group
	deviceResource->sprites[Sprites::ContainerCorner]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.0f, -1.0f);
	deviceResource->sprites[Sprites::ContainerVertical]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.0f, -1.0f);
	deviceResource->sprites[Sprites::ContainerHorizontal]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.f, -1.f);

	// add the filler
	{
		pvr::ui::PixelGroup filler = deviceResource->uiRenderer.createPixelGroup();
		filler->add(deviceResource->sprites[Sprites::ContainerFiller]);
		deviceResource->sprites[Sprites::ContainerFiller]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.f, -1.f);
		filler->setAnchor(pvr::ui::Anchor::TopLeft, rect.x + borderX, rect.height - borderY);
		filler->setScale(glm::vec2(.5f * (rect.getDimension().x - borderX * 2 /*minus the left and right borders*/) *
		                           deviceResource->uiRenderer.getRenderingDimX() / deviceResource->sprites[Sprites::ContainerFiller]->getWidth(),
		                           .501f * (rect.getDimension().y - borderY * 2/*minus Top and Bottom borders*/) *
		                           deviceResource->uiRenderer.getRenderingDimY() / deviceResource->sprites[Sprites::ContainerFiller]->getHeight()));
		outContainer.group->add(filler);
		outContainer.group->setSize(glm::vec2(deviceResource->uiRenderer.getRenderingDimX(), deviceResource->uiRenderer.getRenderingDimY()));
	}

	// Top Left Corner
	{
		pvr::ui::PixelGroup newGroup = deviceResource->uiRenderer.createPixelGroup();
		// place the center at the
		newGroup->add(deviceResource->sprites[Sprites::ContainerCorner]);
		newGroup->setAnchor(pvr::ui::Anchor::BottomRight, rectTopHorizontal.x, rectTopHorizontal.y);
		outContainer.group->add(newGroup);
	}

	//Top Right Corner
	{
		pvr::ui::PixelGroup newGroup = deviceResource->uiRenderer.createPixelGroup();
		newGroup->add(deviceResource->sprites[Sprites::ContainerCorner]);
		// flip the x coordinate by negative scale
		newGroup->setAnchor(pvr::ui::Anchor::BottomRight, rectTopHorizontal.width,
		                    rectTopHorizontal.y)->setScale(glm::vec2(-1.f, 1.0f));
		outContainer.group->add(newGroup);
	}

	//bottom left Corner
	{
		pvr::ui::PixelGroup newGroup = deviceResource->uiRenderer.createPixelGroup();
		newGroup->add(deviceResource->sprites[Sprites::ContainerCorner]);
		// flip the y coordinates
		newGroup->setAnchor(pvr::ui::Anchor::BottomRight, rectBottomHorizontal.x,
		                    rectBottomHorizontal.height)->setScale(glm::vec2(1.f, -1.0f));
		outContainer.group->add(newGroup);
	}

	//Bottom right Corner
	{
		pvr::ui::PixelGroup newGroup = deviceResource->uiRenderer.createPixelGroup();
		newGroup->add(deviceResource->sprites[Sprites::ContainerCorner]);
		// flip the x and y coordinates
		newGroup->setAnchor(pvr::ui::Anchor::BottomRight, rectBottomHorizontal.width,
		                    rectBottomHorizontal.height)->setScale(glm::vec2(-1.f, -1.0f));
		outContainer.group->add(newGroup);
	}

	// Horizontal Up
	{
		//calculate the width of the sprite
		pvr::float32 width = (rectTopHorizontal.getDimension().x * .5f * deviceResource->uiRenderer.getRenderingDimX() /
		                      deviceResource->sprites[Sprites::ContainerVertical]->getWidth());
		pvr::ui::PixelGroup horizontal = deviceResource->uiRenderer.createPixelGroup();
		horizontal->add(deviceResource->sprites[Sprites::ContainerVertical]);
		horizontal->setAnchor(pvr::ui::Anchor::BottomLeft, rectTopHorizontal.x, rectTopHorizontal.y);
		horizontal->setScale(glm::vec2(width, 1.f));
		outContainer.group->add(horizontal);
	}

	// Horizontal Down
	{
		//calculate the width of the sprite
		pvr::float32 width = (rectBottomHorizontal.getDimension().x * .5f * deviceResource->uiRenderer.getRenderingDimX() /
		                      deviceResource->sprites[Sprites::ContainerVertical]->getWidth());
		pvr::ui::PixelGroup horizontal = deviceResource->uiRenderer.createPixelGroup();
		horizontal->add(deviceResource->sprites[Sprites::ContainerVertical]);
		horizontal->setAnchor(pvr::ui::Anchor::TopLeft, rectBottomHorizontal.x, rectBottomHorizontal.y);
		horizontal->setScale(glm::vec2(width, -1.f));
		outContainer.group->add(horizontal);
	}

	// Vertical Left
	{
		//calculate the height of the sprite
		pvr::float32 height = (rectVerticleLeft.getDimension().y  * .501f * deviceResource->uiRenderer.getRenderingDimY() /
		                       deviceResource->sprites[Sprites::ContainerHorizontal]->getHeight());
		pvr::ui::PixelGroup verticle = deviceResource->uiRenderer.createPixelGroup();
		verticle->add(deviceResource->sprites[Sprites::ContainerHorizontal]);
		verticle->setScale(glm::vec2(1, height))->setAnchor(pvr::ui::Anchor::TopLeft, rectVerticleLeft.x,
		    rectVerticleLeft.height)->setPixelOffset(0, 0);
		outContainer.group->add(verticle);
	}

	// Vertical Right
	{
		//calculate the height of the sprite
		pvr::float32 height = (rectVerticleRight.getDimension().y * .501f * deviceResource->uiRenderer.getRenderingDimY() /
		                       deviceResource->sprites[Sprites::ContainerHorizontal]->getHeight());
		pvr::ui::PixelGroup vertical = deviceResource->uiRenderer.createPixelGroup();
		vertical->add(deviceResource->sprites[Sprites::ContainerHorizontal]);
		vertical->setScale(glm::vec2(-1, height))->setAnchor(pvr::ui::Anchor::TopLeft,
		    rectVerticleRight.width, rectVerticleRight.height);
		outContainer.group->add(vertical);
	}

	pvr::float32 width = 1.f / deviceResource->uiRenderer.getRenderingDimX() * deviceResource->sprites[Sprites::ContainerHorizontal]->getWidth();
	pvr::float32 height = (outContainer.size.height - outContainer.size.y) * .5f;

	// calculate the each container size
	pvr::float32 containerWidth = (rect.width - rect.x) / numSubContainer;
	pvr::float32 borderWidth = 1.f / deviceResource->uiRenderer.getRenderingDimX() * deviceResource->sprites[Sprites::VerticalBar]->getWidth();
	pvr::Rectangle<pvr::float32> subRect(rect.x , rect.y, rect.x + containerWidth, rect.y + lowerContainerHeight);
	height = .5f * (subRect.height - subRect.y) * deviceResource->uiRenderer.getRenderingDimY() /
	         deviceResource->sprites[Sprites::VerticalBar]->getHeight();
	// create the lower containers

	// Horizontal Split
	{
		// half it here because the scaling happen at the center
		width = (rect.getDimension().x * .5f * deviceResource->uiRenderer.getRenderingDimX() /
		         deviceResource->sprites[Sprites::VerticalBar]->getHeight());
		width -= .25;// reduce the width by quarter of a pixel so they fit well between the container
		pvr::ui::PixelGroup horizontal = deviceResource->uiRenderer.createPixelGroup();
		horizontal->add(deviceResource->sprites[Sprites::VerticalBar]);
		horizontal->setScale(glm::vec2(1.f, width))->setAnchor(pvr::ui::Anchor::BottomLeft,
		    rect.x + (2 / deviceResource->uiRenderer.getRenderingDimX())/*offset it by 2 pixel*/, subRect.height);
		horizontal->setRotation(glm::pi<pvr::float32>() * -.5f);// rotate y 90 degree
		outContainer.group->add(horizontal);
	}

	for (pvr::uint32 i = 0; i < numSubContainer - 1; ++i)
	{
		pvr::ui::PixelGroup groupVertical = deviceResource->uiRenderer.createPixelGroup();
		deviceResource->sprites[Sprites::VerticalBar]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.f, -1.f);
		groupVertical->add(deviceResource->sprites[Sprites::VerticalBar]);
		groupVertical->setAnchor(pvr::ui::Anchor::BottomLeft, subRect.width, subRect.y)->setScale(glm::vec2(1, height));
		outContainer.group->add(groupVertical);
		subRect.x = subRect.x + containerWidth - borderWidth;
		subRect.width = subRect.width + containerWidth;
	}
	deviceResource->containerTop = outContainer;
}

/*!*********************************************************************************************************************
\brief	Code in initApplication() will be called by PVRShell once per run, before the rendering context is created.
		Used to initialize variables that are not Dependant on it (e.g. external modules, loading meshes, etc.)
		If the rendering context is lost, InitApplication() will not be called again.
\return Return pvr::Result::Success if no error occurred
***********************************************************************************************************************/
pvr::Result OGLESExampleUI::initApplication()
{
	deviceResource.reset(new DeviceResource());
	deviceResource->assetManager.init(*this);
	setStencilBitsPerPixel(8);
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief	create the weather page
***********************************************************************************************************************/
void OGLESExampleUI::createPageWeather()
{
	// background
	pvr::ui::PixelGroup backGround = deviceResource->uiRenderer.createPixelGroup();
	backGround->add(deviceResource->sprites[Ancillary::Background]);

	// create the weather page
	SpriteContainer container;
	createSpriteContainer(deviceResource->pageClock.container.size, 4, LowerContainerHight, container);

	deviceResource->pageWeather.containerTop = container;
	deviceResource->pageWeather.group = deviceResource->uiRenderer.createMatrixGroup();
	deviceResource->pageWeather.group->add(container.group);

	pvr::ui::PixelGroup group = deviceResource->uiRenderer.createPixelGroup();

	// align the sprite with its parent group
	deviceResource->sprites[Sprites::TextWeather]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.0f, -1.f);
	group->setScale(screenScale);
	group->add(deviceResource->sprites[Sprites::TextWeather]);
	const glm::vec2& containerHalfSize = deviceResource->pageWeather.containerTop.size.getDimension() * .5f;
	group->setAnchor(pvr::ui::Anchor::CenterLeft, deviceResource->pageWeather.containerTop.size.x,
	                 deviceResource->pageWeather.containerTop.size.getCenter().y)->setPixelOffset(10, 40);
	deviceResource->pageWeather.group->add(group);

	// add the Weather
	group = deviceResource->uiRenderer.createPixelGroup();
	group->add(deviceResource->sprites[Sprites::WeatherSunCloudBig]);
	// align the sprite with its parent group
	deviceResource->sprites[Sprites::WeatherSunCloudBig]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.0f, -1.f);
	group->setAnchor(pvr::ui::Anchor::Center, deviceResource->pageWeather.containerTop.size.x + containerHalfSize.x,
	                 deviceResource->pageWeather.containerTop.size.y + containerHalfSize.y)->setPixelOffset(0, 40);
	group->setScale(screenScale);
	deviceResource->pageWeather.group->add(group);

	// create the bottom 4 groups
	Sprites::Enum sprites[] =
	{
		Sprites::WeatherSunCloud, Sprites::TextFriday,
		Sprites::WeatherSunCloud, Sprites::TextSaturday,
		Sprites::WeatherRain, Sprites::TextSunday,
		Sprites::WeatherStorm, Sprites::TextMonday
	};

	pvr::float32 width = (deviceResource->pageWeather.containerTop.size.width - deviceResource->pageWeather.containerTop.size.x) / 4.f;
	pvr::float32 tempOffsetX = deviceResource->pageWeather.containerTop.size.x + (width * .5f);;
	// pvr::float32 offsetYWeather =
	for (pvr::uint32 i = 0; i < 8; i += 2)
	{
		Sprites::Enum weather = sprites[i];
		Sprites::Enum text = sprites[i + 1];

		group = deviceResource->uiRenderer.createPixelGroup();
		// align the sprite with its parent group
		deviceResource->sprites[weather]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.0f, -1.f);
		group->add(deviceResource->sprites[weather]);
		group->setAnchor(pvr::ui::Anchor::BottomCenter, tempOffsetX, deviceResource->pageWeather.containerTop.size.y);
		group->setScale(screenScale);
		deviceResource->pageWeather.group->add(group);

		//add the text
		group = deviceResource->uiRenderer.createPixelGroup();
		// align the text with its parent group
		deviceResource->sprites[text]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.0f, -1.f);
		group->add(deviceResource->sprites[text]);
		group->setAnchor(pvr::ui::Anchor::TopCenter, tempOffsetX, deviceResource->pageWeather.containerTop.size.y + LowerContainerHight)->setPixelOffset(0, -5);
		group->setScale(screenScale);

		deviceResource->pageWeather.group->add(group);
		tempOffsetX = tempOffsetX + width;
	}
}

/*!*********************************************************************************************************************
\brief	Create clock sprite
\param	outClock Returned clock
\param	sprite Clock Sprite to create
***********************************************************************************************************************/
void OGLESExampleUI::createClockSprite(SpriteClock& outClock, Sprites::Enum sprite)
{
	// create a group of clock and hand so they can be transformed
	outClock.group = deviceResource->uiRenderer.createPixelGroup();
	outClock.clock = deviceResource->sprites[sprite];
	outClock.hand = deviceResource->uiRenderer.createPixelGroup();

	// clock half size in ndc
	glm::vec2 halfDim = outClock.clock->getDimensions() / deviceResource->uiRenderer.getRenderingDim();

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
\brief	Create clock page
***********************************************************************************************************************/
void OGLESExampleUI::createPageClock()
{
	SpriteContainer container;
	const pvr::uint32 numClocksInColumn = 5;
	pvr::float32 containerHeight = deviceResource->sprites[Sprites::ClockfaceSmall]->getDimensions().y * numClocksInColumn / BaseDim.y;
	containerHeight += LowerContainerHight * .5f;// add the lower container height as well
	pvr::float32 containerWidth = deviceResource->sprites[Sprites::ClockfaceSmall]->getDimensions().x * 4;
	containerWidth += deviceResource->sprites[Sprites::Clockface]->getDimensions().x;
	containerWidth /= BaseDim.x;

	pvr::Rectangle<pvr::float32> containerRect(-containerWidth, -containerHeight,
	    containerWidth, containerHeight);
	createSpriteContainer(pvr::Rectangle<pvr::float32>(containerRect), 2, LowerContainerHight, container);
	deviceResource->pageClock.container = container;

	deviceResource->pageClock.group = deviceResource->uiRenderer.createMatrixGroup();
	pvr::ui::MatrixGroup groupBorder  = deviceResource->uiRenderer.createMatrixGroup();
	groupBorder->add(deviceResource->sprites[Sprites::ContainerVertical]);
	groupBorder->setScaleRotateTranslate(glm::translate(glm::vec3(0.0f, -0.45f, 0.0f))
	                                     * glm::scale(glm::vec3(.65f, .055f, .2f)));
	deviceResource->pageClock.group->add(deviceResource->containerTop.group);

	for (pvr::uint32 i = 0; i < PageClock::numClocks; ++i)
	{
		SpriteClock clock;
		createClockSprite(clock, Sprites::ClockfaceSmall);
		clock.group->setScale(screenScale);
		clock.scale = screenScale;
		// add the clock group in to page group
		deviceResource->pageClock.group->add(clock.group);
		deviceResource->pageClock.clock.push_back(clock);// add the clock
	}

	// add the center clock
	// group the hands
	SpriteClock clockCenter;
	createClockSprite(clockCenter, Sprites::Clockface);
	clockCenter.group->setScale(screenScale);

	deviceResource->pageClock.group->add(clockCenter.group);
	deviceResource->pageClock.clock.push_back(clockCenter);

	deviceResource->pageClock.group->add(deviceResource->sprites[Sprites::Text1]);
	deviceResource->sprites[Sprites::Text1]->setAnchor(pvr::ui::Anchor::BottomLeft,
	    glm::vec2(deviceResource->pageClock.container.size.x,
	              deviceResource->pageClock.container.size.y))->setPixelOffset(0, 10);

	deviceResource->sprites[Sprites::Text1]->setScale(screenScale);
	deviceResource->pageClock.group->add(deviceResource->sprites[Sprites::Text2]);

	deviceResource->sprites[Sprites::Text2]->setAnchor(pvr::ui::Anchor::BottomRight,
	    glm::vec2(deviceResource->pageClock.container.size.width - 0.05f,
	              deviceResource->pageClock.container.size.y))->setPixelOffset(0, 10);
	deviceResource->sprites[Sprites::Text2]->setScale(screenScale);
}

/*!*********************************************************************************************************************
\brief	Create base UI
***********************************************************************************************************************/
void OGLESExampleUI::createBaseUI()
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
	deviceResource->groupBaseUI = deviceResource->uiRenderer.createPixelGroup();

	pvr::ui::PixelGroup horizontalTopBarGrop = deviceResource->uiRenderer.createPixelGroup();
	deviceResource->sprites[Ancillary::Topbar]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.f, -1.f);
	horizontalTopBarGrop->add(deviceResource->sprites[Ancillary::Topbar]);
	horizontalTopBarGrop->setAnchor(pvr::ui::Anchor::TopLeft, -1.f, 1.f);
	horizontalTopBarGrop->setScale(glm::vec2(deviceResource->uiRenderer.getRenderingDimX() * .5f, 1.0f));

	deviceResource->groupBaseUI
	->add(deviceResource->sprites[Ancillary::Background])
	->add(horizontalTopBarGrop)
	->add(deviceResource->sprites[Sprites::Battery])
	->add(deviceResource->sprites[Sprites::Web])
	->add(deviceResource->sprites[Sprites::Newmail])
	->add(deviceResource->sprites[Sprites::Network]);

	glm::vec2 scale = glm::vec2(deviceResource->sprites[Ancillary::Background]->getWidth(),
	                            deviceResource->sprites[Ancillary::Background]->getHeight());
	scale = 2.5f / scale;
	scale *= glm::vec2(getWidth(), getHeight());
	deviceResource->sprites[Ancillary::Background]->setAnchor(pvr::ui::Anchor::TopLeft, -1.f, 1.f)->setScale(scale);

	deviceResource->groupBaseUI->setSize(glm::vec2(deviceResource->uiRenderer.getRenderingDimX(),
	                                     deviceResource->uiRenderer.getRenderingDimY()));
	deviceResource->groupBaseUI->setAnchor(pvr::ui::Anchor::TopRight, glm::vec2(1.0f, 1.0f));
	deviceResource->groupBaseUI->commitUpdates();// update once here
}

/*!*********************************************************************************************************************
\brief	Code in initView() will be called by PVRShell upon initialization or after a change in the rendering context.
        Used to initialize variables that are Dependant on the rendering context (e.g. textures, vertex buffers, etc.)
\return	Return true if no error occurred
***********************************************************************************************************************/
pvr::Result OGLESExampleUI::initView()
{
	context = getGraphicsContext();
	deviceResource->fboOnScreen = context->createOnScreenFbo(0);
	deviceResource->cmdBuffer = context->createCommandBufferOnDefaultPool();
	deviceResource->cmdBufferTitleDesc = context->createSecondaryCommandBufferOnDefaultPool();

	// Initialize uiRenderer
	if (deviceResource->uiRenderer.init(deviceResource->fboOnScreen->getRenderPass(), 0) != pvr::Result::Success)
	{
		this->setExitMessage("ERROR: Cannot initialize Print3D\n");
		return pvr::Result::NotInitialized;
	}
	screenScale = deviceResource->uiRenderer.getRenderingDim() / BaseDim;
	screenScale = glm::vec2(glm::min(screenScale.x, screenScale.y));

	if (!createFbo())
	{
		setExitMessage("Failed to create Fbo");
		return pvr::Result::NotInitialized;
	}

	prevTransTime = this->getTime();

	// Load the sprites
	if (!loadSprites()) {return pvr::Result::NotInitialized;}

	// Load the shaders
	if (!createPipelines())
	{
		this->setExitMessage("Failed to create pipelines");
		return pvr::Result::NotInitialized;
	}

	if (!createSamplersAndDescriptorSet())
	{
		pvr::Log("Failed to create Texture and samplers Descriptor sets");
		return pvr::Result::NotInitialized;
	}
	// Generate the atlas texture.
	if (!isAtlasGenerated) {	generateAtlas();   }

	if (isScreenRotated())
	{
		projMtx = glm::ortho<pvr::float32>(0.f, (pvr::float32) getHeight(), 0.f, (pvr::float32)getWidth(), 0.0f, 1.f);
	}
	else
	{
		projMtx = glm::ortho<pvr::float32>(0.f, (pvr::float32)getWidth(), 0.f, (pvr::float32)getHeight(), 0.0f, 1.f);
	}
	swipe = false;
	// set the default title
	deviceResource->uiRenderer.getDefaultTitle()->setText("Example UI");
	deviceResource->uiRenderer.getDefaultTitle()->commitUpdates();

	// create the base ui which will be the same for all the pages
	createBaseUI();
	createPageClock();
	createPageWeather();
	createPageWindow();
	deviceResource->pageClock.group->setViewProjection(projMtx);
	deviceResource->pageWeather.group->setViewProjection(projMtx);
	deviceResource->pageWindow.group->setViewProjection(projMtx);

	recordSecondaryCommandBuffers();
	updateTitleAndDesc((DisplayOption::Enum)displayOption);
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief	Loads sprites that will be used to create a texture atlas.
\return	Return true if no error occurred
***********************************************************************************************************************/
bool OGLESExampleUI::loadSprites()
{
	pvr::assets::SamplerCreateParam samplerInfo;
	samplerInfo.minificationFilter = SamplerFilter::Nearest;
	samplerInfo.magnificationFilter = SamplerFilter::Nearest;
	samplerInfo.mipMappingFilter = SamplerFilter::None;
	samplerInfo.wrapModeU = SamplerWrap::Clamp;
	samplerInfo.wrapModeV = SamplerWrap::Clamp;
	pvr::api::Sampler samplerNearest = context->createSampler(samplerInfo);

	samplerInfo.minificationFilter = samplerInfo.magnificationFilter = SamplerFilter::Linear;

	pvr::assets::TextureHeader header;
	// Load sprites and add to sprite array so that we can generate a texture atlas from them.
	for (pvr::uint32 i = 0; i < Sprites::Count + Ancillary::Count; i++)
	{
		if (!deviceResource->assetManager.getTextureWithCaching(context, SpritesFileNames[i], &deviceResource->spritesDesc[i].tex, &header))
		{
			pvr::Log("Failed to load texture %s", SpritesFileNames[i]);
			return false;
		}
		// Copy some useful data out of the texture header.
		deviceResource->spritesDesc[i].uiWidth = header.getWidth();
		deviceResource->spritesDesc[i].uiHeight = header.getHeight();

		const pvr::uint8* pixelString = header.getPixelFormat().getPixelTypeChar();

		if (header.getPixelFormat().getPixelTypeId() == (pvr::uint64)pvr::CompressedPixelFormat::PVRTCI_2bpp_RGBA ||
		    header.getPixelFormat().getPixelTypeId() == (pvr::uint64)pvr::CompressedPixelFormat::PVRTCI_4bpp_RGBA ||
		    pixelString[0] == 'a' || pixelString[1] == 'a' || pixelString[2] == 'a' || pixelString[3] == 'a')
		{
			deviceResource->spritesDesc[i].bHasAlpha = true;
		}
		else
		{
			deviceResource->spritesDesc[i].bHasAlpha = false;
		}

		deviceResource->sprites[i] = deviceResource->uiRenderer.createImage(deviceResource->spritesDesc[i].tex, header.getWidth(), header.getHeight());

		deviceResource->sprites[i]->setSampler(samplerNearest);

		if (i == Sprites::ContainerCorner || i == Sprites::ContainerVertical || i == Sprites::ContainerHorizontal || i == Sprites::ContainerFiller)
		{
			deviceResource->sprites[i]->setSampler(samplerNearest);
		}
	}
	return true;
}

/*!*********************************************************************************************************************
\brief	Create nearest and bilinear sampler, and descriptor set for texture atlas
\return	Return true if no error occurred
***********************************************************************************************************************/
bool OGLESExampleUI::createSamplersAndDescriptorSet()
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
	// all pipeline are using the same pipelineLayout
	pvr::api::DescriptorSetLayout descSetLayout = deviceResource->drawPassAtlas.pipe.pipe->getPipelineLayout()->getDescriptorSetLayout(0);

	pvr::api::DescriptorSetUpdate descSetInfo;
	pvr::api::DescriptorSet descSetTexAtlas;
	descSetInfo.setCombinedImageSampler(0, deviceResource->textureAtlas, deviceResource->samplerBilinear);
	descSetTexAtlas = context->createDescriptorSetOnDefaultPool(descSetLayout);
	descSetTexAtlas->update(descSetInfo);

	// setup the draw pass atlas
	deviceResource->drawPassAtlas.descSet = descSetTexAtlas;
	return true;
}

/*!*********************************************************************************************************************
\brief create graphics pipeline for texture-atlas, pre-clip and post-clip pass.
\return	 Return true if no error occurred
***********************************************************************************************************************/
bool OGLESExampleUI::createPipelines()
{
	// create the descriptorsetLayout and pipelineLayout
	pvr::api::DescriptorSetLayoutCreateParam descSetLayoutInfo;
	pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;

	descSetLayoutInfo.setBinding(0, DescriptorType::CombinedImageSampler, 1, ShaderStageFlags::Fragment);
	pipeLayoutInfo.addDescSetLayout(context->createDescriptorSetLayout(descSetLayoutInfo));
	pvr::api::PipelineLayout pipeLayout = context->createPipelineLayout(pipeLayoutInfo);
	pvr::assets::ShaderFile shaderVersioning;

	// create the vertex and fragment shaders
	for (pvr::uint32 i = 0; i < ShaderNames::Count; ++i)
	{
		shaderVersioning.populateValidVersions(VertShaderFileName[i], *this);
		deviceResource->vertexShader[i] =
		  context->createShader(*shaderVersioning.getBestStreamForApi(context->getApiType()), ShaderType::VertexShader,
		                        ShaderDefines[i], (ShaderDefines[i] ? ELEMENTS_IN_ARRAY(ShaderDefines[i]) : 0));

		shaderVersioning.populateValidVersions(FragShaderFileName[i], *this);
		deviceResource->fragmentShader[i] =
		  context->createShader(*shaderVersioning.getBestStreamForApi(context->getApiType()), ShaderType::FragmentShader,
		                        ShaderDefines[i], (ShaderDefines[i] ? ELEMENTS_IN_ARRAY(ShaderDefines[i]) : 0));

		if (deviceResource->vertexShader[i].isNull() || deviceResource->fragmentShader[i].isNull())	{ return false; }
	}

	// --- texture-atlas pipeline
	{
		pvr::api::GraphicsPipelineCreateParam pipeInfo;
		pipeInfo.rasterizer.setCullFace(Face::None);
		pipeInfo.pipelineLayout = pipeLayout;
		pipeInfo.vertexShader = deviceResource->vertexShader[ShaderNames::ColorTexture];
		pipeInfo.fragmentShader = deviceResource->fragmentShader[ShaderNames::ColorTexture];
		pipeInfo.vertexInput.addVertexAttribute(0, 0, pvr::assets::VertexAttributeLayout(DataType::Float32, 4, 0)).
		addVertexAttribute(1, 0, pvr::assets::VertexAttributeLayout(DataType::Float32, 2, sizeof(glm::vec4)));

		pipeInfo.vertexInput.setInputBinding(0, sizeof(Vertex));
		pipeInfo.inputAssembler.setPrimitiveTopology(PrimitiveTopology::TriangleStrip);
		pipeInfo.colorBlend.setAttachmentState(0, pvr::types::BlendingConfig());
		pipeInfo.depthStencil.setDepthTestEnable(false).setDepthWrite(false);
		deviceResource->drawPassAtlas.pipe.pipe = context->createParentableGraphicsPipeline(pipeInfo);

		if (deviceResource->drawPassAtlas.pipe.pipe.isNull())
		{
			pvr::Log("Failed to create TexColor pipeline");
			return false;
		}
		// get uniform locations
		deviceResource->drawPassAtlas.pipe.mvpLoc = deviceResource->drawPassAtlas.pipe.pipe->getUniformLocation("MVPMatrix");
		deviceResource->drawPassAtlas.pipe.rgbaLoc = deviceResource->drawPassAtlas.pipe.pipe->getUniformLocation("vRGBA");
	}

	// --- pre-clip pipeline
	{
		pvr::api::GraphicsPipelineCreateParam pipeInfo;
		pvr::types::BlendingConfig colorAttachment;
		pipeInfo.pipelineLayout = pipeLayout;
		pipeInfo.vertexShader = deviceResource->vertexShader[ShaderNames::ColorShader];
		pipeInfo.fragmentShader = deviceResource->fragmentShader[ShaderNames::ColorShader];
		pipeInfo.vertexInput.addVertexAttribute(0, 0, pvr::assets::VertexAttributeLayout(DataType::Float32, 4, 0)).
		addVertexAttribute(1, 0, pvr::assets::VertexAttributeLayout(DataType::Float32, 2, sizeof(glm::vec4)));
		pipeInfo.vertexInput.setInputBinding(0, sizeof(Vertex));
		pipeInfo.colorBlend.setAttachmentState(0, colorAttachment);
		pipeInfo.inputAssembler.setPrimitiveTopology(PrimitiveTopology::TriangleStrip);
		pipeInfo.depthStencil.setDepthTestEnable(false).setDepthWrite(false);
		pipeInfo.rasterizer.setCullFace(Face::None);

		deviceResource->pipeColor.pipe = context->createParentableGraphicsPipeline(pipeInfo);
		// get uniform locations
		deviceResource->pipeColor.mvpLoc = deviceResource->pipeColor.pipe->getUniformLocation("MVPMatrix");
		deviceResource->pipeColor.rgbaLoc = deviceResource->pipeColor.pipe->getUniformLocation("vRGBA");
		if (deviceResource->pipeColor.pipe.isNull())
		{
			pvr::Log("Failed to create Color Pipeline");
			return false;
		}

		// Set stencil function to always pass, and write 0x1 in to the stencil buffer.
		pvr::api::StencilState stencilState;
		stencilState.opDepthPass = StencilOp::Replace;
		stencilState.compareOp = ComparisonMode::Always;
		pipeInfo.depthStencil.setStencilTest(true).setStencilFrontBack(stencilState);
		// disable writing to the color buffer, write only in the stencil buffer
		pipeInfo.colorBlend.setAttachmentState(0, colorAttachment);
		deviceResource->pipePreClip.pipe = context->createGraphicsPipeline(pipeInfo,
		                                   pvr::api::ParentableGraphicsPipeline(deviceResource->pipeColor.pipe));
		deviceResource->pipePreClip.mvpLoc = deviceResource->pipeColor.mvpLoc;
		deviceResource->pipePreClip.rgbaLoc = deviceResource->pipeColor.rgbaLoc;

	}

	// --- post clip pipeline
	{
		pvr::types::BlendingConfig colorAttachment;
		colorAttachment.blendEnable = true;
		pvr::api::GraphicsPipelineCreateParam pipeInfo;
		pipeInfo.depthStencil.setDepthTestEnable(false).setDepthWrite(false).setStencilTest(true);
		// Set stencil function to always pass, and write 0x1 in to the stencil buffer.
		pvr::api::StencilState stencilState;
		stencilState.compareOp = ComparisonMode::Equal;
		pipeInfo.depthStencil.setStencilFrontBack(stencilState);
		colorAttachment.srcBlendColor = colorAttachment.srcBlendAlpha = pvr::types::BlendFactor::SrcAlpha;
		colorAttachment.destBlendColor = colorAttachment.destBlendAlpha = pvr::types::BlendFactor::OneMinusSrcAlpha;
		pipeInfo.colorBlend.setAttachmentState(0, colorAttachment);
		deviceResource->pipePostClip.pipe = context->createGraphicsPipeline(pipeInfo,
		                                    pvr::api::ParentableGraphicsPipeline(deviceResource->uiRenderer.getPipeline()));
	}

	// set the shader sampler location
	deviceResource->cmdBuffer->beginRecording();
	deviceResource->cmdBuffer->bindPipeline(deviceResource->drawPassAtlas.pipe.pipe);
	deviceResource->cmdBuffer->setUniform<pvr::int32>(deviceResource->drawPassAtlas.pipe.pipe->getUniformLocation("Texture"), 0);
	deviceResource->cmdBuffer->endRecording();
	deviceResource->cmdBuffer->submit();
	return true;
}

/*!*********************************************************************************************************************
\brief	Sorts and packs sprites in to the texture atlas.
\return	Return true if no error occurred
***********************************************************************************************************************/
bool OGLESExampleUI::generateAtlas()
{
	const pvr::uint32 uiTotalBorder = AtlasPixelBorder * 2;

	// Sort sprites such that largest is first
	// Create an array of pointers to sprites so we can sort the pointers instead of the sprites themselves.
	std::vector<SpriteDesc> sortedSprites(deviceResource->spritesDesc, deviceResource->spritesDesc + Sprites::Count);
	std::sort(sortedSprites.begin(), sortedSprites.end(), SpriteCompare());// sort the sprites

	const pvr::api::DescriptorSetLayout descSetLayout = deviceResource->drawPassAtlas.pipe.pipe->getPipelineLayout()
	    ->getDescriptorSetLayout(0);

	glm::mat4 const& mMVP = glm::ortho(0.0f, (pvr::float32)AtlasWidth, (pvr::float32)AtlasWidth,
	                                   .0f, -1.0f, 1.0f);
	deviceResource->cmdBuffer->beginRecording();

	// Set up the Area
	Area* head = new Area(AtlasWidth, AtlasHeight);
	Area* pRtrn = NULL;

	deviceResource->cmdBuffer->bindPipeline(deviceResource->drawPassAtlas.pipe.pipe);
	deviceResource->cmdBuffer->setUniform<glm::mat4>(deviceResource->drawPassAtlas.pipe.mvpLoc, mMVP);
	deviceResource->cmdBuffer->endRecording();
	deviceResource->cmdBuffer->submit();
	// Render some quads within the texture.
	pvr::float32 fX;
	pvr::float32 fY;
	// create empty descriptor set
	pvr::api::DescriptorSet descSet = context->createDescriptorSetOnDefaultPool(descSetLayout);
	for (pvr::uint32 i = 0; i < Sprites::Count; ++i)
	{
		deviceResource->cmdBuffer->beginRecording();
		deviceResource->cmdBuffer->beginRenderPass(deviceResource->fboAtlas,
		    pvr::Rectanglei(0, 0, AtlasWidth, AtlasHeight), true);
		// clear the color attachment on the first iteration
		if (i == 0)
		{
			deviceResource->cmdBuffer->clearColorAttachment(1, glm::vec4(.3f, 0.3f, .3f, 1.f), pvr::Rectanglei(0, 0, AtlasWidth, AtlasHeight));
		}

		pRtrn = head->insert((pvr::int32)sortedSprites[i].uiWidth + uiTotalBorder, (pvr::int32)sortedSprites[i].uiHeight + uiTotalBorder);
		if (!pRtrn)
		{
			this->setExitMessage("ERROR: Not enough room in texture atlas!\n");
			head->deleteArea();
			delete head;
			return false;
		}

		fX = (pvr::float32)pRtrn->getX() + AtlasPixelBorder;
		fY = (pvr::float32)pRtrn->getY() + AtlasPixelBorder;

		// Render sprite on to atlas.
		pvr::api::DescriptorSetUpdate descSetInfo;
		descSetInfo.setCombinedImageSampler(0, sortedSprites[i].tex, deviceResource->samplerNearest);
		descSet->update(descSetInfo);
		deviceResource->cmdBuffer->bindDescriptorSet(
		  deviceResource->drawPassAtlas.pipe.pipe->getPipelineLayout(), 0, descSet, 0);

		// draw
		drawScreenAlignedQuad(deviceResource->drawPassAtlas.pipe, pvr::Rectangle<pvr::float32>(fX, fY,
		                      (pvr::float32)sortedSprites[i].uiWidth, (pvr::float32)sortedSprites[i].uiHeight),
		                      deviceResource->cmdBuffer);

		sortedSprites[i].uiSrcX = (pvr::uint32)fX;
		sortedSprites[i].uiSrcY = (pvr::uint32)fY;
		// we need to submit here because we are modifying the same buffer to draw the next sprite
		deviceResource->cmdBuffer->endRenderPass();
		deviceResource->cmdBuffer->endRecording();
		deviceResource->cmdBuffer->submit();
	}
	deviceResource->cmdBuffer->beginRecording();
	// Now render a 4x4 white quad which can be utilized for rendering non-textured quads without
	// switching the bound texture, or changing shader program.
	// We use 4x4 such that linear filtering will not produce an incorrect color.
	deviceResource->cmdBuffer->beginRenderPass(deviceResource->fboAtlas,
	    pvr::Rectanglei(0, 0, AtlasWidth, AtlasHeight), true);
	deviceResource->cmdBuffer->bindPipeline(deviceResource->pipeColor.pipe);
	deviceResource->cmdBuffer->setUniform<glm::mat4>(deviceResource->pipeColor.mvpLoc, mMVP);
	{
		pRtrn = head->insert(4, 4);
		if (!pRtrn)
		{
			this->setExitMessage("ERROR: Not enough room in texture atlas!\n");
			head->deleteArea();
			delete head;
			return false;
		}
		fX = (pvr::float32)pRtrn->getX();
		fY = (pvr::float32)pRtrn->getY();
		drawScreenAlignedQuad(deviceResource->pipeColor, pvr::Rectangle<pvr::float32>(fX, fY, (pvr::float32)NullQuadPix,
		                      (pvr::float32)NullQuadPix), deviceResource->cmdBuffer);
	}
	head->deleteArea();
	delete head;

	deviceResource->cmdBuffer->endRenderPass();
	deviceResource->cmdBuffer->endRecording();
	deviceResource->cmdBuffer->submit();
	isAtlasGenerated = true;
	return isAtlasGenerated;
}

/*!*********************************************************************************************************************
\brief	Renders a 2D quad with the given parameters. DstRect is the rectangle to be rendered in
		world coordinates. SrcRect is the rectangle to be cropped from the texture in pixel coordinates.
		NOTE: This is not an optimized function and should not be called repeatedly to draw quads to the screen at render time.
***********************************************************************************************************************/
void OGLESExampleUI::drawScreenAlignedQuad(const Pipeline& pipe,
    const pvr::Rectangle<pvr::float32>& dstRect, pvr::api::CommandBufferBase cmdBuffer,
    const pvr::Rectangle<pvr::float32>& srcRect, const pvr::uint32 uiRGBA)
{
	Vertex vVerts[4] =
	{
		{ glm::vec4(dstRect.x, dstRect.y, 0, 1), glm::vec2(srcRect.x, 1.0f - srcRect.y) },
		{ glm::vec4(dstRect.x, dstRect.y + dstRect.height, 0, 1), glm::vec2(srcRect.x, 1.0f - srcRect.height)},
		{ glm::vec4(dstRect.x + dstRect.width, dstRect.y, 0, 1), glm::vec2(srcRect.width, 1.0f - srcRect.y)},
		{ glm::vec4(dstRect.x + dstRect.width, dstRect.y + dstRect.height, 0, 1), glm::vec2(srcRect.width, 1.0f - srcRect.height) }
	};

	if (!deviceResource->quadVbo.isValid())
	{
		deviceResource->quadVbo = context->createBuffer(sizeof(vVerts), BufferBindingUse::VertexBuffer);
	}
	deviceResource->quadVbo->update(vVerts, 0, sizeof(vVerts));
	// Upload color data for all verts
	glm::vec4 vRGBA(((uiRGBA >> 24) & 0xFF)*ByteToFloat, ((uiRGBA >> 16) & 0xFF)*ByteToFloat,
	                ((uiRGBA >> 8) & 0xFF)*ByteToFloat, (uiRGBA & 0xFF)*ByteToFloat);

	cmdBuffer->setUniform<glm::vec4>(pipe.rgbaLoc, glm::vec4(vRGBA));
	cmdBuffer->bindVertexBuffer(deviceResource->quadVbo, 0, 0);
	cmdBuffer->drawArrays(0, 4, 0, 1);
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result OGLESExampleUI::releaseView()
{
	// release all the textures and sprites
	deviceResource.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief	Code in quitApplication() will be called by PVRShell once per run, just before exiting
		the program. If the rendering context is lost, quitApplication() will not be called.
\return	Return pvr::Result::Success if no error occurred
***********************************************************************************************************************/
pvr::Result OGLESExampleUI::quitApplication() { return pvr::Result::Success; }

/*!*********************************************************************************************************************
\brief	Render the page
\param	page Page to render
\param	mTransform Transformation matrix
***********************************************************************************************************************/
void OGLESExampleUI::renderPage(DisplayPage::Enum page, const glm::mat4& mTransform)
{
	switch (page)
	{
	case DisplayPage::Clocks:
		deviceResource->pageClock.update(pvr::float32(getFrameTime()), mTransform);
		deviceResource->cmdBuffer->enqueueSecondaryCmds(deviceResource->cmdBufferClockPage);
		break;
	case DisplayPage::Weather:
		deviceResource->pageWeather.update(mTransform);
		deviceResource->cmdBuffer->enqueueSecondaryCmds(deviceResource->cmdBufferWeatherpage);
		break;
	case DisplayPage::Window:
		deviceResource->pageWindow.update(deviceResource->uiRenderer.getRenderingDimX(), deviceResource->uiRenderer.getRenderingDimY(), mTransform);
		deviceResource->cmdBuffer->enqueueSecondaryCmds(deviceResource->cmdBufferWindow);
		break;
	}
}

/*!*********************************************************************************************************************
\brief	Renders the default interface.
***********************************************************************************************************************/
void OGLESExampleUI::renderUI()
{
	deviceResource->cmdBuffer->beginRenderPass(deviceResource->fboOnScreen,
	    pvr::Rectanglei(0, 0, getWidth(), getHeight()), false, glm::vec4(0.3f, .3f, 0.3f, 1.f));
	// render the baseUI
	deviceResource->cmdBuffer->enqueueSecondaryCmds(deviceResource->cmdBufferBaseUI);

	if (state == DisplayState::Element)
	{
		// A transformation matrix
		if (currentPage == DisplayPage::Window)
		{
			glm::mat4 vRot, vCentre, vInv;
			vRot = glm::rotate(wndRotate, glm::vec3(0.0f, 0.0f, 1.0f));
			vCentre = glm::translate(glm::vec3(-deviceResource->uiRenderer.getRenderingDimX() * .5f,
			                                   -deviceResource->uiRenderer.getRenderingDimY() * .5f, 0.0f));
			vInv = glm::inverse(vCentre);
			// align the group center to the center of the rotation, rotate and translate it back.
			transform =  vInv * vRot * vCentre;
		}
		else
		{
			transform = glm::mat4(1.f);
		}
		// Just render the single, current page
		renderPage(currentPage, transform);
	}
	else if (state == DisplayState::Transition)
	{
		//--- Render outward group
		pvr::float32 fX = pvr::math::quadraticEaseIn(0.0f, -deviceResource->uiRenderer.getRenderingDimX() * cycleDir, transitionPerc);
		transform = glm::translate(glm::vec3(fX, 0.0f, 0.0f));

		//	the last page page
		renderPage(lastPage, transform);

		// --- Render inward group
		fX = pvr::math::quadraticEaseIn(deviceResource->uiRenderer.getRenderingDimX() *  cycleDir, 0.0f, transitionPerc);
		transform = glm::translate(glm::vec3(fX, 0.0f, 0.0f));

		//Render page
		renderPage(currentPage, transform);
	}
	// record draw title and description commands
	deviceResource->cmdBuffer->enqueueSecondaryCmds(deviceResource->cmdBufferTitleDesc);
	deviceResource->cmdBuffer->endRenderPass();
}

/*!*********************************************************************************************************************
\brief	Renders the generated texture atlas.
***********************************************************************************************************************/
void OGLESExampleUI::renderAtlas()
{
	deviceResource->cmdBuffer->beginRenderPass(deviceResource->fboOnScreen,
	    pvr::Rectanglei(0, 0, (pvr::uint32)deviceResource->uiRenderer.getRenderingDimX(),
	                    (pvr::uint32)deviceResource->uiRenderer.getRenderingDimY()), false,
	    glm::vec4(.3f, 0.3f, 0.3f, 1.0f));
	// record draw title and description commands
	deviceResource->cmdBuffer->enqueueSecondaryCmds(deviceResource->cmdBufferTexAtlas);
	// record draw title and description commands
	deviceResource->cmdBuffer->enqueueSecondaryCmds(deviceResource->cmdBufferTitleDesc);
	deviceResource->cmdBuffer->endRenderPass();
}

/*!*********************************************************************************************************************
\brief	Swipe left
***********************************************************************************************************************/
void OGLESExampleUI::swipeLeft()
{
	if (currentPage == 0) {  return; }
	swipe = true;
	cycleDir = -1;
}

/*!*********************************************************************************************************************
\brief	Swipe right
***********************************************************************************************************************/
void OGLESExampleUI::swipeRight()
{
	if (currentPage == DisplayPage::Count - 1) { return; }
	swipe = true;
	cycleDir = 1;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success	if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result OGLESExampleUI::renderFrame()
{
	// begin recording the command buffer
	deviceResource->cmdBuffer->beginRecording();
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
			cycleDir *= -1;							// Reverse direction
			nextPage = currentPage + cycleDir;	// Recalculate
		}
		currentPage = (DisplayPage::Enum)nextPage;
		swipe = false;
	}

	// Calculate next transition amount
	if (state == DisplayState::Transition)
	{
		transitionPerc += 0.01666f;	// 60 FPS
		if (transitionPerc > 1.f)
		{
			state = DisplayState::Element;
			transitionPerc = 1.f;
			wndRotate = 0.0f;			// Reset Window rotation
			wndRotPerc = 0.0f;		// Reset Window rotation percentage
			prevTransTime = currTime;	// Reset time
		}
	}

	drawCallPerFrame = 0;
	renderUI();

	// record commands to draw the title and description
	deviceResource->cmdBuffer->endRecording();
	deviceResource->cmdBuffer->submit();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief	create texture atlas fbo
\return	Return true on success
***********************************************************************************************************************/
bool OGLESExampleUI::createFbo()
{
	// create on-screen fbo
	deviceResource->fboOnScreen = context->createOnScreenFbo(0);

	// create texture atlas fbo
	{
		pvr::api::RenderPassCreateParam renderPassInfo;
		pvr::api::SubPass subPass;

		// create texture-atlas texture
		pvr::api::ImageStorageFormat texAtlasFmt(pvr::PixelFormat::RGBA_8888, 1, ColorSpace::lRGB,
		    pvr::VariableType::UnsignedByteNorm);
		pvr::api::TextureStore texAtlas = context->createTexture();
		texAtlas->allocate2D(texAtlasFmt, AtlasWidth, AtlasHeight);
		deviceResource->textureAtlas = context->createTextureView(texAtlas);


		// Create texture atlas FBO and bind the previously created texture to it.
		pvr::api::FboCreateParam fboInfo;
		renderPassInfo.setColorInfo(0, pvr::api::RenderPassColorInfo(texAtlasFmt));

		subPass.setColorAttachment(0, 0);
		renderPassInfo.setSubPass(0, subPass);

		fboInfo.setColor(0, deviceResource->textureAtlas).setRenderPass(context->createRenderPass(renderPassInfo));
		deviceResource->fboAtlas = context->createFbo(fboInfo);
	}

	deviceResource->spriteAtlas = deviceResource->uiRenderer.createImage(deviceResource->textureAtlas, AtlasWidth, AtlasHeight);
	// scale it by half so they fit in to the screen
	deviceResource->spriteAtlas->setAnchor(pvr::ui::Anchor::TopLeft, -1.0f, 1.0f)->setScale(glm::vec2(.5f));
	deviceResource->spriteAtlas->commitUpdates();

	return (deviceResource->fboAtlas.isNull() ? false : true);
}

/*!*********************************************************************************************************************
\brief	Record secondary command buffer for drawing texture atlas, clock page, weather page and Window page
***********************************************************************************************************************/
void OGLESExampleUI::recordSecondaryCommandBuffers()
{
	// record render texture atlas commands
	{
		deviceResource->cmdBufferTexAtlas = context->createSecondaryCommandBufferOnDefaultPool();
		deviceResource->cmdBufferTexAtlas->beginRecording(deviceResource->fboOnScreen, 0);
		deviceResource->uiRenderer.beginRendering(deviceResource->cmdBufferTexAtlas);
		deviceResource->spriteAtlas->render();
		deviceResource->uiRenderer.endRendering();
		deviceResource->cmdBufferTexAtlas->endRecording();
	}
	{
		deviceResource->cmdBufferBaseUI = context->createSecondaryCommandBufferOnDefaultPool();
		deviceResource->uiRenderer.beginRendering(deviceResource->cmdBufferBaseUI);
		deviceResource->groupBaseUI->render();//render the base GUI
		deviceResource->uiRenderer.endRendering();
	}

	// record DrawClock commands
	{
		deviceResource->cmdBufferClockPage = context->createSecondaryCommandBufferOnDefaultPool();
		deviceResource->cmdBufferClockPage->beginRecording(deviceResource->fboOnScreen, 0);
		deviceResource->uiRenderer.beginRendering(deviceResource->cmdBufferClockPage);
		deviceResource->pageClock.group->render();
		deviceResource->uiRenderer.endRendering();
		deviceResource->cmdBufferClockPage->endRecording();
	}

	// record draw weather commands
	{
		deviceResource->cmdBufferWeatherpage = context->createSecondaryCommandBufferOnDefaultPool();
		deviceResource->cmdBufferWeatherpage->beginRecording(deviceResource->fboOnScreen, 0);
		deviceResource->uiRenderer.beginRendering(deviceResource->cmdBufferWeatherpage);
		deviceResource->pageWeather.group->render();
		deviceResource->uiRenderer.endRendering();
		deviceResource->cmdBufferWeatherpage->endRecording();
	}

	// record draw Window commands
	{
		deviceResource->cmdBufferWindow = context->createSecondaryCommandBufferOnDefaultPool();
		deviceResource->cmdBufferWindow->beginRecording(deviceResource->fboOnScreen, 0);
		// bind the pre-clipping pipeline
		deviceResource->cmdBufferWindow->bindPipeline(deviceResource->pipePreClip.pipe);
		// clear the stencil buffer to 0
		deviceResource->cmdBufferWindow->clearStencilAttachment(pvr::Rectanglei(0, 0,
		    (pvr::int32)deviceResource->uiRenderer.getRenderingDimX(),
		    (pvr::int32)deviceResource->uiRenderer.getRenderingDimY()), 0);
		deviceResource->cmdBufferWindow->setUniformPtr<glm::mat4>(deviceResource->pipePreClip.mvpLoc,
		    1, (const glm::mat4*)&deviceResource->pageWindow.mvp);

		// draw a quad only in to the stencil buffer
		drawScreenAlignedQuad(deviceResource->pipePreClip, pvr::Rectangle<pvr::float32>((pvr::float32)deviceResource->pageWindow.clipArea.x,
		                      (pvr::float32)deviceResource->pageWindow.clipArea.y, (pvr::float32)deviceResource->pageWindow.clipArea.width,
		                      (pvr::float32)deviceResource->pageWindow.clipArea.height), deviceResource->cmdBufferWindow);

		// bind the post clip pipeline and render the text only where the stencil passes
		deviceResource->uiRenderer.beginRendering(deviceResource->cmdBufferWindow, deviceResource->pipePostClip.pipe);
		deviceResource->cmdBufferWindow->setStencilReference(StencilFace::FrontBack, 1);
		deviceResource->cmdBufferWindow->setStencilCompareMask(StencilFace::FrontBack, 0xFFFFFFFF);
		deviceResource->pageWindow.group->render();
		deviceResource->uiRenderer.endRendering();
		deviceResource->cmdBufferWindow->endRecording();
	}
}

/*!*********************************************************************************************************************
\brief Handle input events
\param[in] action Input event to handle
***********************************************************************************************************************/
void OGLESExampleUI::eventMappedInput(pvr::SimplifiedInput action)
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
\brief	Constructor
\param[in] width Area width
\param[in] height Area height
***********************************************************************************************************************/
Area::Area(pvr::int32 width, pvr::int32 height) : x(0), y(0), isFilled(false), right(NULL), left(NULL) { setSize(width, height); }

/*!*********************************************************************************************************************
\brief	Constructor
***********************************************************************************************************************/
Area::Area() : x(0), y(0), isFilled(false), right(NULL), left(NULL) { setSize(0, 0); }

/*!*********************************************************************************************************************
\brief	Calculates an area where there's sufficient space or returns NULL if no space could be found.
\return	Return a pointer to the area added, else NULL if it fails
\param	width Area width
\param	height Area height
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
	if (isFilled)	{ return NULL; }

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
\brief	Deletes the given area.
\return	 Return true on success
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
\brief	set the area size
\param	width Area width
\param  height Area height
***********************************************************************************************************************/
void Area::setSize(pvr::int32 width, pvr::int32 height)
{
	w = width;	h = height;	size = width * height;
}

/*!*********************************************************************************************************************
\brief	Get the X position of the area.
\return	Return the area's x position
***********************************************************************************************************************/
inline pvr::int32 Area::getX()const {return x;}

/*!*********************************************************************************************************************
\brief	get the Y position of the area.
\return Return the area's y position
***********************************************************************************************************************/
inline pvr::int32 Area::getY()const { return y; }

/*!*********************************************************************************************************************
\brief	This function must be implemented by the user of the shell.The user should return its pvr::Shell object defining
        the behavior of the application.
\return	Return The demo supplied by the user
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() {return std::auto_ptr<pvr::Shell>(new OGLESExampleUI());}
