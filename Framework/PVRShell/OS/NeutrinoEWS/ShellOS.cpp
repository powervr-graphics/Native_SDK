/*!
\brief Contains the implementation for the pvr::platform::ShellOS class on Example Windowing System platforms on
Neutrino.
\file PVRShell\OS/NeutrinoEWS/ShellOS.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRShell/OS/ShellOS.h"
#include "PVRCore/IO/FilePath.h"
#include "PVRCore/Log.h"
#include "EWS/ews.h"
#include <sys/time.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <csignal>
#include <termios.h>
#include <unistd.h>
#include <cstdarg>


using namespace pvr::types;
namespace pvr {
namespace platform {
struct InternalOS
{
	bool isInitialized;

	EWS_DISPLAY display;
	EWS_WINDOW  window;

	InternalOS() : display(EWS_NO_DISPLAY), window(EWS_NO_WINDOW)
	{

	}
};

// Setup the capabilities.
const ShellOS::Capabilities ShellOS::_capabilities = { Capability::Immutable, Capability::Immutable };

ShellOS::ShellOS(OSApplication hInstance, OSDATA osdata) : _instance(hInstance)
{
	_OSImplementation = new InternalOS;
}

ShellOS::~ShellOS()
{
	delete _OSImplementation;
}
void ShellOS::updatePointingDeviceLocation()
{
}

Result ShellOS::init(DisplayAttributes& data)
{
	if (!_OSImplementation)
	{
		return Result::OutOfMemory;
	}

	// Construct our read and write path.
	pid_t ourPid = getpid();
	char* exePath, srcLink[64];
	int len = 256;
	int res;
	FILE* fp;

	sprintf(srcLink, "/proc/%d/exefile", ourPid);
	exePath = 0;

	do
	{
		len *= 2;
		delete[] exePath;
		exePath = new char[len];
		fp = fopen(srcLink, "r");
		fgets(exePath, len, fp);
		res = strlen(exePath);
		fclose(fp);

		if (res < 0)
		{
			Log(Log.Warning, "Readlink %s failed. The application name, read path and write path have not been set.\n",
			    exePath);
			break;
		}
	}
	while (res >= len);

	if (res >= 0)
	{
		exePath[res] = '\0'; // Null-terminate readlink's result.
		FilePath filepath(exePath);
		setApplicationName(filepath.getFilenameNoExtension());

		_WritePath = filepath.getDirectory() + FilePath::getDirectorySeparator();
		_ReadPaths.clear();
		_ReadPaths.push_back(filepath.getDirectory() + FilePath::getDirectorySeparator());
		_ReadPaths.push_back(std::string(".") + FilePath::getDirectorySeparator());
		_ReadPaths.push_back(filepath.getDirectory() + FilePath::getDirectorySeparator() + "Assets" + FilePath::getDirectorySeparator());
	}

	delete[] exePath;

	return Result::Success;
}

Result ShellOS::initializeWindow(DisplayAttributes& data)
{
	EWS_COORD windowPosition;
	EWS_SIZE windowSize;
	EWS_PIXELFORMAT ePixelFormat;


	_OSImplementation->isInitialized = true;
	data.fullscreen = true;
	data.x = data.y = 0;
	//data.width = data.height = 0; //TODO: Hmmm, there doesn't appear to be a way of getting the monitor resolution.
	//We may have to rethink the returning of the width and height so you can only get the context width and height.
	data.width = 1280;
	data.height = 1024;

	_OSImplementation->display = EWSOpenDisplay(EWS_DEFAULT_DISPLAY, 0);

	if (_OSImplementation->display == EWS_NO_DISPLAY)
	{
		Log(Log.Error, "EWSOpenDisplay failed (%s:%i)", __FILE__, __LINE__);
		return Result::UnknownError;
	}

	switch (data.redBits + data.greenBits + data.blueBits + data.alphaBits)
	{
	case 32:
		ePixelFormat = EWS_PIXEL_FORMAT_ARGB_8888;
		break;
	default:
		ePixelFormat = EWS_PIXEL_FORMAT_RGB_565;
		data.redBits = 5;
		data.greenBits = 6;
		data.blueBits = 5;
		data.alphaBits = 0;
		break;
	}

	data.forceColorBPP = true; // The API surface when used with EWS is required to have the same color depth as the EWS surface.

	windowPosition.iX = data.x;
	windowPosition.iY = data.y;
	windowSize.uiHeight = data.width;
	windowSize.uiWidth = data.height;

	_OSImplementation->window = EWSCreateWindow(_OSImplementation->display, windowPosition, windowSize, ePixelFormat, EWS_ROTATE_0);

	if (_OSImplementation->window == EWS_NO_WINDOW)
	{
		Log(Log.Error, "EWSCreateWindow failed (%s:%i)", __FILE__, __LINE__);
		EWSCloseDisplay(_OSImplementation->display);
		return Result::UnknownError;
	}

	return Result::Success;
}

void ShellOS::releaseWindow()
{
	EWSDestroyWindow(_OSImplementation->window);
	_OSImplementation->window = EWS_NO_WINDOW;

	EWSCloseDisplay(_OSImplementation->display);
	_OSImplementation->display = EWS_NO_DISPLAY;
}

OSApplication ShellOS::getApplication() const
{
	return _instance;
}

OSDisplay ShellOS::getDisplay() const
{
	return reinterpret_cast<void*>(_OSImplementation->display);
}

OSWindow ShellOS::getWindow() const
{
	return reinterpret_cast<void*>(_OSImplementation->window);
}


static Keys mapEwsKeyToPvrKey(int key)
{
	switch (key)
	{
	case EWS_KEY_ESC: return Keys::Escape;
	case EWS_KEY_SPACE: return Keys::Space;
	case 2: return Keys::Key1;
	case 3: return Keys::Key2;
	case EWS_KEY_UP: return Keys::Up;
	case EWS_KEY_DOWN: return Keys::Down;
	case EWS_KEY_LEFT: return Keys::Left;
	case EWS_KEY_RIGHT: return Keys::Right;
	default: return Keys::Space;
	}
}

Result ShellOS::handleOSEvents()
{
	EWS_EVENT sEvent;
	while (EWSNextEventIfAvailable(&sEvent))
	{
		if (sEvent.sWindow == _OSImplementation->window)
		{
			if (sEvent.eType == EWS_EVENT_KEYPRESS)
			{
				_shell->onKeyDown(mapEwsKeyToPvrKey(sEvent.uiKeyCode));
				_shell->onKeyUp(mapEwsKeyToPvrKey(sEvent.uiKeyCode));
			}
		}
	}

	return Result::Success;
}

bool ShellOS::isInitialized()
{
	return _OSImplementation && _OSImplementation->window;
}

Result ShellOS::popUpMessage(const char8* const title, const char8* const message, ...) const
{
	if (!message)
	{
		return Result::NoData;
	}

	va_list arg;

	va_start(arg, message);
	Log.vaOutput(Log.Information, message, arg);
	va_end(arg);

	return Result::Success;
}
}
}
//!\endcond