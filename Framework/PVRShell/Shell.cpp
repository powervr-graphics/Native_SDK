/*!*********************************************************************************************************************
\file         PVRShell\Shell.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementation for the PowerVR Shell (pvr::Shell).
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRShell/Shell.h"
#include "PVRShell/ShellData.h"
#include "PVRCore/FilePath.h"
#include "PVRCore/Log.h"
#include "PVRShell/OS/ShellOS.h"
#include "PVRCore/FileStream.h"
#include "PVRShell/TGAWriter.h"
#include "PVRCore/StringFunctions.h"
#include <cstdlib>
#if defined(_WIN32)
#include "PVRCore/Windows/WindowsResourceStream.h"
#elif defined(__ANDROID__)
#include <android_native_app_glue.h>
#include "PVRCore/Android/AndroidAssetStream.h"
#endif

#define EPSILON_PIXEL_SQUARE 100
namespace pvr {
namespace platform {
Shell::Shell() : m_dragging(false),m_data(0)
{
}

Shell::~Shell()
{
}


void Shell::implSystemEvent(SystemEvent systemEvent)
{
	switch (systemEvent)
	{
	case SystemEvent::SystemEvent_Quit:
		Log(Log.Information, "SystemEvent::Quit");
		exitShell();
		break;
	default:
		break;
	}
}

void Shell::implPointingDeviceUp(uint8 buttonIdx)
{
	if (!m_pointerState.isPressed(buttonIdx))
	{
		return;
	}
	m_pointerState.setButton(buttonIdx, false);
	if (buttonIdx == 0) // NO buttons pressed - start drag
	{
		m_pointerState.endDragging();
	}

	eventButtonUp(buttonIdx); //send the ButtonUp event

	bool drag = (m_dragging && buttonIdx == 0); //Detecting drag for first button only pointer
	if (drag) // Drag button was release - Detect Drag!
	{
		m_dragging = false;
		eventDragFinished(m_pointerState.position());

		int16 dx = m_pointerState.position().x - m_pointerState.dragStartPosition().x;
		int16 dy = m_pointerState.position().y - m_pointerState.dragStartPosition().y;
		drag = (dx * dx + dy * dy > EPSILON_PIXEL_SQUARE);

		//////// MAPPING SWIPES - TOUCHES TO MAIN INPUT /////////
		// Swiping motion -> Left/Right/Up/Down
		// Touching : Center: Action1, Left part = Action2, Right part = Action3
		float dist = float(dx * dx + dy * dy);
		if (dist > 10 * EPSILON_PIXEL_SQUARE) //SWIPE -- needs a slightly bigger gesture than drag, but otherwise it's the same...
		{
			SimplifiedInput act = (dy * dy > dx * dx) ? (dy < 0 ? SimplifiedInput::Up : SimplifiedInput::Down) :
			                      (dx > 0 ? SimplifiedInput::Right : SimplifiedInput::Left);
			eventMappedInput(act);
		}
	}
	if (!drag) //Not a drag, then a click...
	{
		eventClick(buttonIdx, m_pointerState.position());

		if (buttonIdx == 0) // First button, so map to other actions as well...
		{
			SimplifiedInput act =
			  getPointerNormalisedPosition().x < .25 ? SimplifiedInput::Action2 : // Left
			  getPointerNormalisedPosition().x > .75 ? SimplifiedInput::Action3 : // Right
			  SimplifiedInput::Action1; //Center
			eventMappedInput(act);
		}
		else //For mouses, map mouse actions to other actions as well
		{
			SimplifiedInput action = MapPointingDeviceButtonToSimpleInput(buttonIdx);
			if (action != SimplifiedInput::NONE)
			{
				eventMappedInput(action);
			}
		}
	}
}

void Shell::implPointingDeviceDown(uint8 buttonIdx)
{
	if (!m_pointerState.isPressed(buttonIdx))
	{
		m_pointerState.setButton(buttonIdx, true);
		if (buttonIdx == 0) // NO buttons pressed - start drag
		{
			m_pointerState.startDragging();
		}
		eventButtonDown(buttonIdx);
	}
}

void Shell::updatePointerPosition(PointerLocation location)
{
	m_pointerState.setPointerLocation(location);
	if (!m_dragging && m_pointerState.isDragging())
	{
		int16 dx = m_pointerState.position().x - m_pointerState.dragStartPosition().x;
		int16 dy = m_pointerState.position().y - m_pointerState.dragStartPosition().y;
		m_dragging = (dx * dx + dy * dy > EPSILON_PIXEL_SQUARE);
		if (m_dragging)
		{
			eventDragStart(0, m_pointerState.dragStartPosition());
		}

	}
}

void Shell::implKeyDown(Keys key)
{
	if (!m_keystate[(uint32)key]) //Swallow event on repeat.
	{
		m_keystate[(uint32)key] = true;
		eventKeyDown(key);
	}
	eventKeyStroke(key);
}

void Shell::implKeyUp(Keys key)
{
	if (m_keystate[(uint32)key])
	{
		m_keystate[(uint32)key] = false;
		eventKeyUp(key);
		SimplifiedInput action = MapKeyToMainInput(key);
		if (action != SimplifiedInput::NONE)
		{
			eventMappedInput(action);
		}
	}
}

Result Shell::shellInitApplication()
{
	assertion(m_data != NULL);

	m_data->timeAtInitApplication = getTime();
	m_data->lastFrameTime = m_data->timeAtInitApplication;
	m_data->currentFrameTime = m_data->timeAtInitApplication;
	return initApplication();
}

Result Shell::shellQuitApplication()
{
	return quitApplication();
}

Result Shell::shellInitView()
{
	m_data->graphicsContextStore = createGraphicsContext();
	if (m_data->graphicsContextStore.isValid())
	{
		m_data->graphicsContext = m_data->graphicsContextStore;
		m_data->graphicsContext->init(*this);
	}
	pvr::Result res = initView();
	m_data->currentFrameTime = getTime() - 17; //Avoid first frame huge times
	m_data->lastFrameTime = getTime() - 32;
	return res;
}

Result Shell::shellReleaseView()
{
	if (m_data->graphicsContext.isValid())
	{
		m_data->graphicsContext->waitIdle();
	}
	pvr::Result retval = releaseView();
	return retval;
}

Result Shell::shellRenderFrame()
{
	getOS().updatePointingDeviceLocation();
	processShellEvents();
	m_data->lastFrameTime = m_data->currentFrameTime;
	m_data->currentFrameTime = getTime();
	Result result;
	if (!m_data->weAreDone)
	{
		result = renderFrame();
	}
	//m_data->weAreDone can very well be changed DURING renderFrame.
	if (m_data->weAreDone)
	{
		result = Result::ExitRenderFrame;
	}
	return result;
}

void Shell::processShellEvents()
{
	while (!eventQueue.empty())
	{
		ShellEvent event = eventQueue.front();
		eventQueue.pop();
		switch (event.type)
		{
		case ShellEvent::SystemEvent:
			implSystemEvent(event.systemEvent);
			break;
		case ShellEvent::PointingDeviceDown:
			implPointingDeviceDown(event.buttonIdx);
			break;
		case ShellEvent::PointingDeviceUp:
			implPointingDeviceUp(event.buttonIdx);
			break;
		case ShellEvent::KeyDown:
			implKeyDown(event.key);
			break;
		case ShellEvent::KeyUp:
			implKeyUp(event.key);
			break;
		case ShellEvent::PointingDeviceMove:
			break;
		}

	}
}

uint64 Shell::getFrameTime()
{
	return m_data->currentFrameTime - m_data->lastFrameTime;
}

uint64 Shell::getTime()
{
	ShellData& data = *m_data;

	if (data.forceFrameTime)
	{
		return data.frameNo * data.fakeFrameTime;
	}

	return data.timer.getCurrentTimeMilliSecs();
}

uint64 Shell::getTimeAtInitApplication() const
{
	return m_data->timeAtInitApplication;
}

Result Shell::init(struct ShellData* data)
{
	if (!m_data)
	{
		m_data = data;
		return Result::Success;
	}

	return Result::AlreadyInitialized;
}

const platform::CommandLineParser::ParsedCommandLine& Shell::getCommandLine() const
{
	return m_data->commandLine->getParsedCommandLine();
}

void Shell::setFullscreen(const bool fullscreen)
{
	if (ShellOS::getCapabilities().resizable != types::Capability::Unsupported)
	{
		m_data->attributes.fullscreen = fullscreen;
	}
}

bool Shell::isFullScreen() const
{
	return m_data->attributes.fullscreen;
}

Result Shell::setDimensions(uint32 w, uint32 h)
{
	if (ShellOS::getCapabilities().resizable != types::Capability::Unsupported)
	{
		m_data->attributes.width = w;
		m_data->attributes.height = h;
		return Result::Success;
	}

	return Result::UnsupportedRequest;
}

uint32 Shell::getWidth() const
{
	return m_data->attributes.width;
}

uint32 Shell::getHeight() const
{
	return m_data->attributes.height;
}

Result Shell::setPosition(uint32 x, uint32 y)
{
	if (ShellOS::getCapabilities().resizable != types::Capability::Unsupported)
	{
		m_data->attributes.x = x;
		m_data->attributes.y = y;
		return Result::Success;
	}

	return Result::UnsupportedRequest;
}

uint32 Shell::getPositionX() const
{
	return m_data->attributes.x;
}

uint32 Shell::getPositionY() const
{
	return m_data->attributes.y;
}

int32 Shell::getQuitAfterFrame() const
{
	return m_data->dieAfterFrame;
}

float32 Shell::getQuitAfterTime() const
{
	return m_data->dieAfterTime;
}

VsyncMode Shell::getVsyncMode() const
{
	return m_data->attributes.vsyncMode;
}

uint32 Shell::getSwapChainLength() const
{
	return getPlatformContext().getSwapChainLength();
}

uint32 Shell::getSwapChainIndex() const
{
	return getPlatformContext().getSwapChainIndex();
}

uint32 Shell::getAASamples() const
{
	return m_data->attributes.aaSamples;
}

uint32 Shell::getColorBitsPerPixel() const
{
	return m_data->attributes.redBits + m_data->attributes.blueBits + m_data->attributes.greenBits + m_data->attributes.alphaBits;
}

uint32 Shell::getDepthBitsPerPixel() const
{
	return m_data->attributes.depthBPP;
}

uint32 Shell::getStencilBitsPerPixel() const
{
	return m_data->attributes.stencilBPP;
}

void Shell::setQuitAfterFrame(uint32 value)
{
	m_data->dieAfterFrame = value;
}

void Shell::setQuitAfterTime(float32 value)
{
	m_data->dieAfterTime = value;
}

void Shell::setVsyncMode(VsyncMode value)
{
	m_data->attributes.vsyncMode = value;
}

void Shell::setPreferredSwapChainLength(uint32 swapChainLength)
{
	m_data->attributes.swapLength = swapChainLength;
}

void Shell::forceReinitView()
{
	m_data->forceReleaseInitCycle = true;
}

void Shell::setAASamples(uint32 value)
{
	// Should this be passed to the api context instead just incase the API supports dynamic changing
	// of the aa settings e.g. openVG
	m_data->attributes.aaSamples = value;
}

void Shell::setColorBitsPerPixel(uint32 r, uint32 g, uint32 b, uint32 a)
{
	m_data->attributes.redBits = r;
	m_data->attributes.greenBits = g;
	m_data->attributes.blueBits = b;
	m_data->attributes.alphaBits = a;
}

void Shell::setBackBufferColorspace(types::ColorSpace colorSpace)
{
	m_data->attributes.frameBufferSrgb = (colorSpace == types::ColorSpace::sRGB);
}

types::ColorSpace Shell::getBackBufferColorspace()
{
	return m_data->attributes.frameBufferSrgb ? types::ColorSpace::sRGB : types::ColorSpace::lRGB;
}

void Shell::setDepthBitsPerPixel(uint32 value)
{
	m_data->attributes.depthBPP = value;
}

void Shell::setStencilBitsPerPixel(uint32 value)
{
	m_data->attributes.stencilBPP = value;
}

void Shell::setCaptureFrames(uint32 start, uint32 stop)
{
	m_data->captureFrameStart = start;
	m_data->captureFrameStop = stop;
}

void Shell::setCaptureFrameScale(uint32 value)
{
	if (value >= 1)
	{
		m_data->captureFrameScale = value;
	}
}

uint32 Shell::getCaptureFrameStart() const
{
	return m_data->captureFrameStart;
}

uint32 Shell::getCaptureFrameStop() const
{
	return m_data->captureFrameStop;
}

void Shell::setContextPriority(uint32 value)
{
	m_data->attributes.contextPriority = value;
}

uint32  Shell::getContextPriority() const
{
	return m_data->attributes.contextPriority;
}

void Shell::setDesiredConfig(uint32 value)
{
	m_data->attributes.configID = value;
}

uint32  Shell::getDesiredConfig() const
{
	return m_data->attributes.configID;
}

void Shell::setFakeFrameTime(uint32 value)
{
	m_data->fakeFrameTime = value;
}

uint32  Shell::getFakeFrameTime() const
{
	return m_data->fakeFrameTime;
}

Stream::ptr_type Shell::getAssetStream(const string& filename, bool logFileNotFound)
{
	// The shell will first attempt to open a file in your readpath with the same name.
	// This allows you to override any built-in assets
	Stream::ptr_type stream;
	// Try absolute path first:
	stream.reset(new FileStream(filename, "rb"));

	if (stream->open())
	{
		return stream;
	}

	stream.reset(0);

	// Then relative to the search paths:
	const std::vector<string>& paths = getOS().getReadPaths();
	for (size_t i = 0; i < paths.size(); ++i)
	{
		string filepath(paths[i]);
		filepath += filename;
		stream.reset(new FileStream(filepath, "rb"));

		if (stream->open())
		{
			return stream;
		}

		stream.reset(0);
	}

	// Now we attempt to load assets using the OS defined method
#if defined(_WIN32) // On windows, the filename also matches the resource id in our examples, which is fortunate
	stream.reset(new WindowsResourceStream(filename.c_str()));
#elif defined(__ANDROID__) // On android, external files are packaged in the .apk as assets
	struct android_app* app = static_cast<android_app*>(m_data->os->getApplication());

	if (app && app->activity && app->activity->assetManager)
	{
		stream.reset(new AndroidAssetStream(app->activity->assetManager, filename.c_str()));
	}
	else
	{
		Log(Log.Debug, "Could not request android asset stream %s -- Application, Activity or Assetmanager was null",
		    filename.c_str());
	}
#endif //On the rest of the files, the filesystem is either sandboxed (iOS) or we use it directly (Linux)
	if (stream.get())
	{
		if (stream->open())
		{
			return stream;
		}
		stream.reset();
	}

	if (logFileNotFound)
	{
		Log(Log.Error, "pvr::Shell::getAssetStream: File Not Found; Could not retrieve a stream for filename [%s]", filename.c_str());
	}
	return Stream::ptr_type((Stream::ptr_type::element_type*)0);
}

void Shell::setExitMessage(const tchar* const format, ...)
{
	va_list argumentList;

	va_start(argumentList, format);
	m_data->exitMessage = strings::vaFormatString(format, argumentList);
	va_end(argumentList);
}

void Shell::setApplicationName(const tchar* const format, ...)
{
	va_list argumentList;

	va_start(argumentList, format);
	getOS().setApplicationName(strings::vaFormatString(format, argumentList).c_str());
	va_end(argumentList);
}

void Shell::setTitle(const tchar* const format, ...)
{
	va_list argumentList;

	va_start(argumentList, format);
	m_data->attributes.windowTitle = strings::vaFormatString(format, argumentList);
	va_end(argumentList);
}

const string& Shell::getExitMessage() const
{
	return m_data->exitMessage;
}

const string& Shell::getApplicationName() const
{
	return getOS().getApplicationName();
}

const string& Shell::getDefaultReadPath() const
{
	return getOS().getDefaultReadPath();
}

const std::vector<string>& Shell::getReadPaths() const
{
	return getOS().getReadPaths();
}

const string& Shell::getWritePath() const
{
	return getOS().getWritePath();
}

ShellOS& Shell::getOS() const
{
	return *m_data->os;
}

IPlatformContext& Shell::getPlatformContext()
{
	return *m_data->platformContext;
}
const IPlatformContext& Shell::getPlatformContext() const
{
	return *m_data->platformContext;
}

GraphicsContext& Shell::getGraphicsContext()
{
	return m_data->graphicsContext;
}

const GraphicsContext& Shell::getGraphicsContext() const
{
	return m_data->graphicsContext;
}

void Shell::setPresentBackBuffer(const bool value)
{
	m_data->presentBackBuffer = value;
}

bool Shell::isPresentingBackBuffer()
{
	return m_data->presentBackBuffer;
}

void Shell::showOutputInfo()
{
	string attributesInfo, tmp;
	attributesInfo.reserve(2048);
	tmp.reserve(1024);

	tmp = string("\nApplication name:\t") + getApplicationName() + "\n\n";
	attributesInfo.append(tmp);

	tmp = string("SDK version:\t") + string(getSDKVersion()) + "\n\n";
	attributesInfo.append(tmp);

	tmp = string("Read path:\t") + getDefaultReadPath() + "\n\n";
	attributesInfo.append(tmp);

	tmp = string("Write path:\t") + getWritePath() + "\n\n";
	attributesInfo.append(tmp);

	attributesInfo.append("Command-line:");

	const platform::CommandLineParser::ParsedCommandLine::Options& options = getCommandLine().getOptionsList();

	for (uint32 i = 0; i < options.size(); ++i)
	{
		if (options[i].val)
		{
			tmp = strings::createFormatted(" %hs=%hs", options[i].arg, options[i].val);
		}
		else
		{
			tmp = strings::createFormatted(" %hs", options[i].arg);
		}

		attributesInfo.append(tmp);
	}

	attributesInfo.append("\n");

	int32 frame = getQuitAfterFrame();
	if (frame != -1)
	{
		tmp = strings::createFormatted("Quit after frame:\t%i\n", frame);
		attributesInfo.append(tmp);
	}

	float32 time = getQuitAfterTime();
	if (time != -1)
	{
		tmp = strings::createFormatted("Quit after time:\t%f\n", time);
		attributesInfo.append(tmp);
	}

	if (!(m_data->platformContext.get()))
	{
		attributesInfo.append(m_data->platformContext->getInfo());
	}
	else
	{
		attributesInfo.append("The API is currently un-initialized.");
	}

#if defined(__ANDROID__)
	// Android's logging output truncates long strings. Therefore, output our info in blocks
	int32 size = static_cast<int32>(attributesInfo.size());
	const tchar* const ptr = attributesInfo.c_str();

	for (uint32 offset = 0; size > 0; offset += 1024, size -= 1024)
	{
		string chunk(&ptr[offset], std::min(size, 1024));
		Log(Log.Information, chunk.c_str());
	}

#else
	Log(Log.Information, attributesInfo.c_str());
#endif
}

void Shell::setForceFrameTime(const bool value)
{
	m_data->forceFrameTime = value;
	if (value)
	{
		m_data->timeAtInitApplication = 0;
		m_data->lastFrameTime = 0;
		m_data->currentFrameTime = 0;
	}
}

bool Shell::isForcingFrameTime()
{
	return m_data->forceFrameTime;
}

void Shell::setShowFPS(const bool showFPS)
{
	m_data->showFPS = showFPS;
}

bool Shell::isShowingFPS() const
{
	// Should this be passed to the api context instead just incase the API supports dynamic changing of the aa settings e.g. openVG
	return m_data->showFPS;
}

float Shell::getFPS() const
{
	return m_data->FPS;
}

void Shell::takeScreenshot() const
{
	byte* pBuffer = (byte*) calloc(1, m_data->attributes.width * m_data->attributes.height * 4);
	if (m_data->graphicsContext->screenCaptureRegion(0, 0, m_data->attributes.width, m_data->attributes.height,
	    pBuffer, IGraphicsContext::ImageFormatBGRA))
	{
		string filename;

		// Determine our screenshot filename
		string suffix;
		string prefix;
		suffix = strings::createFormatted("_f%u.tga", m_data->frameNo);

		prefix = getWritePath() + getApplicationName();
		filename = prefix + suffix;

		bool openResult;

		{
			FileStream file(filename.c_str(), "r");
			openResult = file.open();
			file.close();
		}

		if (openResult)   // Does the file already exist?
		{
			//Should we really allow 100000 screenshots? Hmmm...
			for (uint32 i = 1; i < 100000; ++i)
			{
				suffix = strings::createFormatted("_f%u_%u.tga", m_data->frameNo, i);
				filename = prefix + suffix;
				FileStream file(filename.c_str(), "r");

				if (!file.open())
				{
					break;
				}

				file.close();
			}
		}
		Log(pvr::Logger::Information, "Writing TGA screenshot, filename %s.", filename.c_str());
		writeTGA(filename.c_str(), m_data->attributes.width, m_data->attributes.height, (unsigned char*)pBuffer,
		         4, m_data->captureFrameScale);
	}
	free(pBuffer);
}

bool Shell::isScreenRotated()const
{
	return m_data->attributes.isScreenRotated();
}

void Shell::exitShell()
{
	m_data->weAreDone = true;
}

DisplayAttributes& Shell::getDisplayAttributes()
{
	return m_data->attributes;
}
OSDisplay Shell::getDisplay()
{
	return m_data->os->getDisplay();
}

OSWindow Shell::getWindow()
{
	return m_data->os->getWindow();
}

Api Shell::getApiTypeRequired()
{
	return m_data->contextType;
}
Api Shell::getApiType()
{
	return getGraphicsContext()->getApiType();
}

void Shell::setApiTypeRequired(Api value)
{
	m_data->minContextType = value;
	m_data->contextType = value;
}

void Shell::setMinApiType(Api value)
{
	m_data->minContextType = value;
}
Api Shell::getMinApiTypeRequired()
{
	return m_data->minContextType;
}

Api Shell::getMaxApiLevel()
{
	return m_data->platformContext->getMaxApiVersion();
}

bool Shell::isApiSupported(Api api)
{
	return m_data->platformContext->isApiSupported(api);
}


void Shell::setDeviceQueueTypesRequired(DeviceQueueType value)
{
	m_data->deviceQueueType = value;
}
DeviceQueueType Shell::getDeviceQueueTypesRequired()
{
	return m_data->deviceQueueType;
}
}
}
#undef EPSILON_PIXEL_SQUARE
//!\endcond
