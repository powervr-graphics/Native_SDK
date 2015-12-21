/*!*********************************************************************************************************************
\file         PVRShell\OS\EWS\ShellOS.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief     	  Contains the implementation for the pvr::system::ShellOS class on Example Windowing System for Linux.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRShell/OS/ShellOS.h"
#include "PVRCore/FilePath.h"
#include "PVRCore/Log.h"
#include "EWS/ews.h"
#include <sys/time.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <csignal>
#include <termios.h>
#include <unistd.h>
#include <cstdarg>


namespace pvr {
namespace system{
struct InternalOS
{
	bool isInitialised;

	EWS_DISPLAY display;
	EWS_WINDOW  window;

	InternalOS() : display(EWS_NO_DISPLAY), window(EWS_NO_WINDOW)
	{

	}
};

// Setup the capabilities.
const ShellOS::Capabilities ShellOS::m_capabilities = { Capability::Immutable, Capability::Immutable };

ShellOS::ShellOS(OSApplication hInstance, OSDATA osdata) : m_instance(hInstance)
{
	m_OSImplementation = new InternalOS;
}

ShellOS::~ShellOS()
{
	delete m_OSImplementation;
}
void ShellOS::updatePointingDeviceLocation()
{
}

Result::Enum ShellOS::init(DisplayAttributes& data)
{
	if (!m_OSImplementation)
	{ return Result::OutOfMemory; }

	// Construct our read and write path.
	pid_t ourPid = getpid();
	char* exePath, srcLink[64];
	int len = 256;
	int res;

	sprintf(srcLink, "/proc/%d/exe", ourPid);
	exePath = 0;

	do
	{
		len *= 2;
		delete[] exePath;
		exePath = new char[len];
		res = readlink(srcLink, exePath, len);

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

		m_WritePath = filepath.getDirectory() + FilePath::getDirectorySeparator();
		m_ReadPaths.clear();
		m_ReadPaths.push_back(filepath.getDirectory() + FilePath::getDirectorySeparator());
		m_ReadPaths.push_back(std::string(".") + FilePath::getDirectorySeparator());
		m_ReadPaths.push_back(filepath.getDirectory() + FilePath::getDirectorySeparator() + "Assets" + FilePath::getDirectorySeparator());
	}

	delete[] exePath;

	return Result::Success;
}

Result::Enum ShellOS::initialiseWindow(DisplayAttributes& data)
{
	EWS_COORD windowPosition;
	EWS_SIZE windowSize;
	EWS_PIXELFORMAT ePixelFormat;


	m_OSImplementation->isInitialised = true;
	data.fullscreen = true;
	data.x = data.y = 0;
	data.width = 1280;
	data.height = 1024;

	m_OSImplementation->display = EWSOpenDisplay(EWS_DEFAULT_DISPLAY, 0);

	if (m_OSImplementation->display == EWS_NO_DISPLAY)
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

	m_OSImplementation->window = EWSCreateWindow(m_OSImplementation->display, windowPosition, windowSize, ePixelFormat, EWS_ROTATE_0);

	if (m_OSImplementation->window == EWS_NO_WINDOW)
	{
		Log(Log.Error, "EWSCreateWindow failed (%s:%i)", __FILE__, __LINE__);
		EWSCloseDisplay(m_OSImplementation->display);
		return Result::UnknownError;
	}

	return Result::Success;
}

void ShellOS::releaseWindow()
{
	EWSDestroyWindow(m_OSImplementation->window);
	m_OSImplementation->window = EWS_NO_WINDOW;

	EWSCloseDisplay(m_OSImplementation->display);
	m_OSImplementation->display = EWS_NO_DISPLAY;
}

OSApplication ShellOS::getApplication() const
{
	return m_instance;
}

OSDisplay ShellOS::getDisplay() const
{
	return reinterpret_cast<void*>(m_OSImplementation->display);
}

OSWindow ShellOS::getWindow() const
{
	return reinterpret_cast<void*>(m_OSImplementation->window);
}


static Keys::Enum mapEwsKeyToPvrKey(int key)
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

Result::Enum ShellOS::handleOSEvents()
{
	EWS_EVENT sEvent;
	while (EWSNextEventIfAvailable(&sEvent))
	{
		if (sEvent.sWindow == m_OSImplementation->window)
		{
			if (sEvent.eType == EWS_EVENT_KEYPRESS)
			{
				m_shell->onKeyDown(mapEwsKeyToPvrKey(sEvent.uiKeyCode));
				m_shell->onKeyUp(mapEwsKeyToPvrKey(sEvent.uiKeyCode));
			}
		}
	}

	return Result::Success;
}

bool ShellOS::isInitialised()
{
	return m_OSImplementation && m_OSImplementation->window;
}

Result::Enum ShellOS::popUpMessage(const tchar* const title, const tchar* const message, ...) const
{
	if (!message)
	{ return Result::NoData; }

	va_list arg;

	va_start(arg, message);
	Log.vaOutput(Log.Information, message, arg);
	va_end(arg);

	return Result::Success;
}
}
}
//!\endcond