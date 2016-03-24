/*!*********************************************************************************************************************
\file         PVRShell\OS\X11\ShellOS.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains an implementation of pvr::system::ShellOS for Linux X11 systems.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRShell/OS/ShellOS.h"
#include "PVRCore/FilePath.h"
#include "PVRCore/Log.h"

#include "X11/Xlib.h"
#include "X11/Xutil.h"
#undef Success
//#if defined(BUILDING_FOR_DESKTOP_GL)
//#include "X11/extensions/xf86vmode.h"
//#endif

#include <sys/time.h>
#include <unistd.h>
#include <cstdarg>
#include <memory>
#include <cstdio>
#include <cstring>

namespace pvr {
namespace system {
/****************************************************************************
	Defines
	*****************************************************************************/
struct InternalOS
{
	Display*     display;
	long         screen;
	XVisualInfo* visual;
	Colormap     colorMap;
	Window       window;

//#if defined(BUILDING_FOR_DESKTOP_GL)
//	XF86VidModeModeLine originalMode;  // modeline that was active at the starting point of this aplication
//	int         originalModeDotClock;
//#endif

	InternalOS() : display(0), screen(0), visual(0), window(0)
// #if defined(BUILDING_FOR_DESKTOP_GL)
		// , originalModeDotClock(0)
// #endif
	{

	}
};


static Keys::Enum X11_To_Keycode[255] ={
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Escape,

	Keys::Key1,//10
	Keys::Key2,
	Keys::Key3,
	Keys::Key4,
	Keys::Key5,
	Keys::Key6,
	Keys::Key7,
	Keys::Key8,
	Keys::Key9,
	Keys::Key0,

	Keys::Minus,//20
	Keys::Equals,
	Keys::Backspace,
	Keys::Tab,
	Keys::Q,
	Keys::W,
	Keys::E,
	Keys::R,
	Keys::T,
	Keys::Y,

	Keys::U,//30
	Keys::I,
	Keys::O,
	Keys::P,
	Keys::SquareBracketLeft,
	Keys::SquareBracketRight,
	Keys::Return,
	Keys::Control,
	Keys::A,
	Keys::S,

	Keys::D,//40
	Keys::F,
	Keys::G,
	Keys::H,
	Keys::J,
	Keys::K,
	Keys::L,
	Keys::Semicolon,
	Keys::Quote,
	Keys::Backquote,
	Keys::Shift,//50

	Keys::Backslash,
	Keys::Z,
	Keys::X,
	Keys::C,
	Keys::V,
	Keys::B,
	Keys::N,
	Keys::M,
	Keys::Comma,

	Keys::Period,//60
	Keys::Slash,
	Keys::Shift,
	Keys::NumMul,
	Keys::Alt,
	Keys::Space,
	Keys::CapsLock,
	Keys::F1,
	Keys::F2,
	Keys::F3,

	Keys::F4,//70
	Keys::F5,
	Keys::F6,
	Keys::F7,
	Keys::F8,
	Keys::F9,
	Keys::F10,
	Keys::NumLock,
	Keys::ScrollLock,
	Keys::Num7,

	Keys::Num8,//80
	Keys::Num9,
	Keys::NumSub,
	Keys::Num4,
	Keys::Num5,
	Keys::Num6,
	Keys::NumAdd,
	Keys::Num1,
	Keys::Num2,
	Keys::Num3,

	Keys::Num0,//90
	Keys::NumPeriod,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Backslash,
	Keys::F11,
	Keys::F12,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,

	Keys::Unknown,//100
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Return,
	Keys::Control,
	Keys::NumDiv,
	Keys::PrintScreen,
	Keys::Alt,
	Keys::Unknown,

	Keys::Home,//110
	Keys::Up,
	Keys::PageUp,
	Keys::Left,
	Keys::Right,
	Keys::End,
	Keys::Down,
	Keys::PageDown,
	Keys::Insert,
	Keys::Delete,

	Keys::Unknown,//120
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Pause,
	Keys::Unknown,
	Keys::Unknown,

	Keys::Unknown,//130
	Keys::Unknown,
	Keys::Unknown,
	Keys::SystemKey1,
	Keys::SystemKey1,
	Keys::SystemKey2,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
};
static Keys::Enum getKeyFromX11Code(int keycode)
{
	Keys::Enum key = X11_To_Keycode[keycode];
	return key;
}

void ShellOS::updatePointingDeviceLocation()
{
	int x, y, dummy0, dummy1;
	uint dummy2;
	Window child_return, root_return;
	if (XQueryPointer(m_OSImplementation->display, m_OSImplementation->window, &root_return, &child_return, &x, &y, &dummy0, &dummy1, &dummy2))
	{
		m_shell->updatePointerPosition(PointerLocation((int16)x, (int16)y));
	}
}

// Setup the capabilities
const ShellOS::Capabilities ShellOS::m_capabilities = { types::Capability::Immutable, types::Capability::Immutable };

ShellOS::ShellOS(void* hInstance, OSDATA osdata) : m_instance(hInstance)
{
	m_OSImplementation = new InternalOS;
}

ShellOS::~ShellOS()
{
	delete m_OSImplementation;
}

Result::Enum ShellOS::init(DisplayAttributes& data)
{
	if (!m_OSImplementation)
	{
		return Result::OutOfMemory;
	}

	// Construct our read and write path
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
			Log(Log.Warning, "Readlink %s failed. The application name, read path and write path have not been set.\n", exePath);
			break;
		}
	}
	while (res >= len);

	if (res >= 0)
	{
		exePath[res] = '\0'; // Null-terminate readlink's result
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

static Bool waitForMapNotify(Display* d, XEvent* e, char* arg)
{
	return (e->type == MapNotify) && (e->xmap.window == (Window)arg);
}

Result::Enum ShellOS::initializeWindow(DisplayAttributes& data)
{
	Display* display = XOpenDisplay(NULL);

	if (!display)
	{
		Log(Log.Error, "Unable to open X display (%s:%i)", __FILE__, __LINE__);
		return Result::UnknownError;
	}

	long screen = XDefaultScreen(display);

	/*
		If there is a fullscreen request then
		set the window size to the display size.
		If there is no fullscreen request then reduce window size while keeping
		the same aspect by dividing the dims by two until it fits inside the display area.
		If the position has not changed from its default value, set it to the middle of the screen.
	*/

	int display_width = XDisplayWidth(display, screen);
	int display_height = XDisplayHeight(display, screen);

	if (!data.fullscreen)
	{
		// Resize if the width and height if they're out of bounds
		if (data.width > display_width)
		{
			data.width = display_width;
		}

		if (data.height > display_height)
		{
			data.height = display_height;
		}
	}

	if (data.x == DisplayAttributes::PosDefault)
	{
		data.x = 0;
	}

	if (data.y == DisplayAttributes::PosDefault)
	{
		data.y = 0;
	}
	// Create the window
	XSetWindowAttributes	WinAttibutes;
	XSizeHints				sh;
	XEvent					event;

	int depth = DefaultDepth(display, screen);
	XVisualInfo* visual = new XVisualInfo;
	XMatchVisualInfo(display, screen, depth, TrueColor, visual);

	if (!visual)
	{
		Log(Log.Error, "Unable to acquire visual (%s:%i)", __FILE__, __LINE__);
		return Result::UnknownError;
	}

	Colormap colorMap = XCreateColormap(display, RootWindow(display, screen), visual->visual, AllocNone);

	Window window;
//#if defined(BUILDING_FOR_DESKTOP_GL)
//	m_i32OriginalModeDotClock = XF86VidModeBadClock;
//
//	if (data.fullScreen)
//	{
//		XF86VidModeModeInfo** modes;       // modes of display
//		int numModes;                      // number of modes of display
//		int chosenMode;
//		int edimx, edimy;                   //established width and height of the chosen modeline
//		int i;
//
//		// Get mode lines to see if there is requested modeline
//		XF86VidModeGetAllModeLines(display, screen, &numModes, &modes);
//
//		// look for mode with requested resolution
//		chosenMode = -1;
//		i = 0;
//		while ((chosenMode == -1) && (i < numModes))
//		{
//			if ((modes[i]->hdisplay == data.width) && (modes[i]->vdisplay == data.height))
//			{
//				chosenMode = i;
//			}
//			++i;
//		}
//
//		// If there is no requested resolution among modelines then terminate
//		if (chosenMode == -1)
//		{
//			LogError("Chosen resolution for fullscreen mode does not match any modeline available (%s:%i)", __FILE__, __LINE__);
//			return false;
//		}
//
//		// save desktop-resolution before switching modes
//		XF86VidModeGetModeLine(display, screen, &m_OSImplementation->originalModeDotClock, &m_OSImplementation->originalMode);
//
//		XF86VidModeSwitchToMode(display, screen, modes[chosenMode]);
//		XF86VidModeSetViewPort(display, screen, 0, 0);
//		edimx = modes[chosenMode]->hdisplay;
//		edimy = modes[chosenMode]->vdisplay;
//		LogInfo("Fullscreen Resolution %d x %d (chosen mode = %d)", edimx, edimy, chosenMode);
//		XFree(modes);
//
//		WinAttibutes.colormap = colorMap;
//		WinAttibutes.background_pixel = 0xFFFFFFFF;
//		WinAttibutes.border_pixel = 0;
//		WinAttibutes.override_redirect = true;
//
//		// add to these for handling other events
//		WinAttibutes.event_mask = StructureNotifyMask | ExposureMask | ButtonPressMask | ButtonReleaseMask | Button1MotionMask | KeyPressMask | KeyReleaseMask;
//
//		// The diffrence is that we want to ignore influence of window manager for our fullscreen window
//		unsigned long mask = CWBackPixel | CWBorderPixel | CWEventMask | CWColormap | CWOverrideRedirect;
//
//		window = XCreateWindow(display, RootWindow(display, screen), 0, 0, edimx, edimy, 0, CopyFromParent, InputOutput, CopyFromParent, mask, &WinAttibutes);
//
//		// keeping the pointer of mouse and keyboard in window to prevent from scrolling the virtual screen
//		XWarpPointer(display, None, window, 0, 0, 0, 0, 0, 0);
//
//		// Map and then wait till mapped, grabbing should be after mapping the window
//		XMapWindow(display, window);
//		XGrabKeyboard(display, window, True, GrabModeAsync, GrabModeAsync, CurrentTime);
//		XGrabPointer(display, window, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, window, None, CurrentTime);
//		XIfEvent(display, &event, waitForMapNotify, (char*)window);
//	}
//	else
//#endif
	{
//#if !defined(BUILDING_FOR_DESKTOP_GL)
//		if (data.fullscreen)
//		{
//			data.width = display_width;
//			data.height = display_height;
//			return Result::UnknownError;
//		}
//#endif
		WinAttibutes.colormap = colorMap;
		WinAttibutes.background_pixel = 0xFFFFFFFF;
		WinAttibutes.border_pixel = 0;

		// add to these for handling other events
		WinAttibutes.event_mask = StructureNotifyMask | ExposureMask | ButtonPressMask | ButtonReleaseMask | Button1MotionMask | KeyPressMask | KeyReleaseMask;

		// The attribute mask
		unsigned long mask = CWBackPixel | CWBorderPixel | CWEventMask | CWColormap;

		window = XCreateWindow(display, // Display
		                       RootWindow(display, screen), 	// Parent
		                       data.x, 	// X position of window
		                       data.y,		// Y position of window
		                       data.width,	// Window width
		                       data.height,// Window height
		                       0,									// Border width
		                       CopyFromParent, 					// Depth (taken from parent)
		                       InputOutput, 						// Window class
		                       CopyFromParent, 					// Visual type (taken from parent)
		                       mask, 								// Attributes mask
		                       &WinAttibutes);						// Attributes

		// Set the window position
		sh.flags = USPosition;
		sh.x = data.x;
		sh.y = data.y;
		XSetStandardProperties(display, window, data.windowTitle.c_str(), getApplicationName().c_str(), 0, 0, 0, &sh);

		// Map and then wait till mapped
		XMapWindow(display, window);
		XIfEvent(display, &event, waitForMapNotify, (char*)window);

		// An attempt to hide a border for fullscreen on non OGL apis (OGLES,OGLES2)
		if (data.fullscreen)
		{
			XEvent xev;
			Atom wmState = XInternAtom(display, "_NET_WM_STATE", False);
			Atom wmStateFullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);

			memset(&xev, 0, sizeof(XEvent));
			xev.type = ClientMessage;
			xev.xclient.window = window;
			xev.xclient.message_type = wmState;
			xev.xclient.format = 32;
			xev.xclient.data.l[0] = 1;
			xev.xclient.data.l[1] = wmStateFullscreen;
			xev.xclient.data.l[2] = 0;
			XSendEvent(display, RootWindow(display, screen), False, SubstructureNotifyMask, &xev);
		}

		Atom wmDelete = XInternAtom(display, "WM_DELETE_WINDOW", True);
		XSetWMProtocols(display, window, &wmDelete, 1);
		XSetWMColormapWindows(display, window, &window, 1);
	}

	XFlush(display);

	// Save our variables
	m_OSImplementation->display = display;
	m_OSImplementation->window = window;
	m_OSImplementation->visual = visual;
	m_OSImplementation->colorMap = colorMap;
	return Result::Success;
}

void ShellOS::releaseWindow()
{
	XDestroyWindow(m_OSImplementation->display, m_OSImplementation->window);
	m_OSImplementation->window = 0;
	XFreeColormap(m_OSImplementation->display, m_OSImplementation->colorMap);
	m_OSImplementation->colorMap = 0;

	delete m_OSImplementation->visual;
	m_OSImplementation->visual = 0;

	XCloseDisplay(m_OSImplementation->display);
	m_OSImplementation->display = 0;
}

OSApplication ShellOS::getApplication() const
{
	return m_instance;
}

OSDisplay ShellOS::getDisplay() const
{
	return static_cast<OSDisplay>(m_OSImplementation->display);
}

OSWindow ShellOS::getWindow() const
{
	return  reinterpret_cast<void*>(m_OSImplementation->window);
}

Result::Enum ShellOS::handleOSEvents()
{
	XEvent	event;
	char*		atoms;

	// Are there messages waiting?
	int numMessages = XPending(m_OSImplementation->display);

	for (int i = 0; i < numMessages; ++i)
	{
		XNextEvent(m_OSImplementation->display, &event);

		switch (event.type)
		{
		case ClientMessage:
			atoms = XGetAtomName(m_OSImplementation->display, event.xclient.message_type);
			if (*atoms == *"WM_PROTOCOLS")
			{
				m_shell->onSystemEvent(SystemEvent::SystemEvent_Quit);
			}
			XFree(atoms);
			break;

		case ButtonRelease:
		{
			XButtonEvent* button_event = ((XButtonEvent*)&event);
			switch (button_event->button)
			{
			case 1:
			{
				m_shell->onPointingDeviceUp(0);
			}
			break;
			default: break;
			}
			break;
		}
		case ButtonPress:
		{
			XButtonEvent* button_event = ((XButtonEvent*)&event);
			switch (button_event->button)
			{
			case 1:
			{
				m_shell->onPointingDeviceDown(0);
			}
			break;
			default: break;
			}
			break;
		}
		case MotionNotify:
		//{
		//	XMotionEvent* motion_event = ((XMotionEvent*)&event);

		//	Event e = EventTypeTouchMove;
		//	e.touch.touchID = 0;

		//	e.touch.x = motion_event->x / (float)XDisplayWidth(m_OSImplementation->display, m_OSImplementation->screen);
		//	e.touch.y = motion_event->y / (float)XDisplayHeight(m_OSImplementation->display, m_OSImplementation->screen);

		//	eventManager.submitEvent(e);
		//	break;
		//}
		case KeyPress:
		{
			XKeyEvent* key_event = ((XKeyEvent*)&event);
			Log(Log.Debug, "%d", key_event->keycode);
if (getKeyFromX11Code(key_event->keycode)==Keys::Escape) {Log(Log.Debug, "Escape");} 
else {Log(Log.Debug, "???");}
			m_shell->onKeyDown(getKeyFromX11Code(key_event->keycode));
		}
		break;
		case KeyRelease:
		{
			XKeyEvent* key_event = ((XKeyEvent*)&event);
			m_shell->onKeyUp(getKeyFromX11Code(key_event->keycode));
		}
		break;

		default:
			break;
		}
	}

	return Result::Success;
}


bool ShellOS::isInitialized()
{
	return m_OSImplementation && m_OSImplementation->window;
}

Result::Enum ShellOS::popUpMessage(const char* title, const char* message, ...) const
{
	if (!title && !message)
	{
		return Result::NoData;
	}

	va_list arg;

	va_start(arg, message);
	Log(Log.Information, message, arg);
	va_end(arg);

	return Result::Success;
}
}
}
//!\endcond
