/*!
\brief Contains an implementation of pvr::platform::ShellOS for Linux X11 systems.
\file PVRShell\OS/X11/ShellOS.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRShell/OS/ShellOS.h"
#include "PVRCore/IO/FilePath.h"
#include "PVRCore/Log.h"

#include "xcb/xcb.h"

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
namespace {

xcb_screen_t* screen_of_display(xcb_connection_t* c, int screen)
{
	xcb_screen_iterator_t iter;

	iter = xcb_setup_roots_iterator(xcb_get_setup(c));
	for (; iter.rem; --screen, xcb_screen_next(&iter))
		if (screen == 0)
		{ return iter.data; }

	return NULL;
}
}
namespace pvr {
namespace platform {
/****************************************************************************
	Defines
*****************************************************************************/
struct InternalOS
{
	xcb_connection_t* connection;
	xcb_screen_t* screen;
	xcb_window_t window;
	xcb_intern_atom_reply_t* atomWmDeleteWindow;

	InternalOS() : connection(0), screen(0), window(0), atomWmDeleteWindow(0) {}
};


static Keys X11_To_Keycode[255] =
{
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
static Keys getKeyFromX11Code(int keycode)
{
    Keys key = X11_To_Keycode[keycode];
	return key;
}

void ShellOS::updatePointingDeviceLocation()
{
	int x, y, dummy0, dummy1;
}

// Setup the capabilities
const ShellOS::Capabilities ShellOS::_capabilities = { Capability::Immutable, Capability::Immutable };

ShellOS::ShellOS(void* hInstance, OSDATA osdata) : _instance(hInstance)
{
	_OSImplementation = new InternalOS;
}

ShellOS::~ShellOS() {	delete _OSImplementation;   }

bool ShellOS::init(DisplayAttributes& data)
{
	if (!_OSImplementation)
	{
		return false;
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
			Log(LogLevel::Warning, "Readlink %s failed. The application name, read path and write path have not been set.\n", exePath);
			break;
		}
	}
	while (res >= len);

	if (res >= 0)
	{
		exePath[res] = '\0'; // Null-terminate readlink's result
		FilePath filepath(exePath);
		setApplicationName(filepath.getFilenameNoExtension());
		_WritePath = filepath.getDirectory() + FilePath::getDirectorySeparator();
		_ReadPaths.clear();
		_ReadPaths.push_back(filepath.getDirectory() + FilePath::getDirectorySeparator());
		_ReadPaths.push_back(std::string(".") + FilePath::getDirectorySeparator());
		_ReadPaths.push_back(filepath.getDirectory() + FilePath::getDirectorySeparator() + "Assets" + FilePath::getDirectorySeparator());
	}

	delete[] exePath;

	return true;
}
/*
static Bool waitForMapNotify(Display* d, XEvent* e, char* arg)
{
	return (e->type == MapNotify) && (e->xmap.window == (Window)arg);
}*/

bool ShellOS::initializeWindow(DisplayAttributes& data)
{
	// initialize the connection
	int numScreenDefault;
	_OSImplementation->connection = xcb_connect(NULL, &numScreenDefault);
	if (_OSImplementation->connection == NULL)
	{
		Log(LogLevel::Error, "Could not find a compatible Vulkan ICD!\n");
		return false;
	}
	const xcb_setup_t* setup = xcb_get_setup(_OSImplementation->connection);
	xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
	int scr = numScreenDefault;
	while (scr-- > 0) { xcb_screen_next(&iter); }
	_OSImplementation->screen = iter.data;

	// initialize the window
	if (!(_OSImplementation->window = xcb_generate_id(_OSImplementation->connection)))
	{
		Log(LogLevel::Error, "Unable to generate XID for the X11 window (%s:%i)", __FILE__, __LINE__);
		return false;
	}
	uint32_t valueMask = XCB_CW_BACK_PIXEL | XCB_CW_BORDER_PIXMAP | XCB_CW_EVENT_MASK;
	uint32_t valueList[32] =
	{
		_OSImplementation->screen->black_pixel,
		0,
		XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY
	};
	/*
	If there is a fullscreen request then
	set the window size to the display size.
	If there is no fullscreen request then reduce window size while keeping
	the same aspect by dividing the dims by two until it fits inside the display area.
	If the position has not changed from its default value, set it to the middle of the screen.
	*/
	if (data.fullscreen)
	{
		xcb_screen_t* defaultScreen = screen_of_display(_OSImplementation->connection, numScreenDefault);
		if (defaultScreen)
		{
			data.width = std::max<int32_t>(data.width, defaultScreen->width_in_pixels);
			data.height = std::max<int32_t>(data.height, defaultScreen->height_in_pixels);
		}
	}
	if (data.x == DisplayAttributes::PosDefault) {	data.x = 0; }

	if (data.y == DisplayAttributes::PosDefault) {	data.y = 0;	}

	xcb_create_window(_OSImplementation->connection, XCB_COPY_FROM_PARENT, _OSImplementation->window,
	                  _OSImplementation->screen->root, data.x, data.y,  data.width, data.height, 0,
	                  XCB_WINDOW_CLASS_INPUT_OUTPUT, _OSImplementation->screen->root_visual, valueMask, valueList);
	// Send notification when window is Destroyed
	xcb_intern_atom_cookie_t cookie = xcb_intern_atom(_OSImplementation->connection, 1, 12, "WM_PROTOCOLS");
	xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(_OSImplementation->connection, cookie, 0);

	xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(_OSImplementation->connection, 0, 16, "WM_DELETE_WINDOW");
	_OSImplementation->atomWmDeleteWindow = xcb_intern_atom_reply(_OSImplementation->connection, cookie2, 0);

	xcb_change_property(_OSImplementation->connection, XCB_PROP_MODE_REPLACE, _OSImplementation->window, (*reply).atom, 4, 32, 1,
	                    &(*_OSImplementation->atomWmDeleteWindow).atom);
	const char* title = "VulkanHelloAPI";
	xcb_change_property(_OSImplementation->connection, XCB_PROP_MODE_REPLACE, _OSImplementation->window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
	                    strlen(title), title);

	free(reply);
	xcb_map_window(_OSImplementation->connection, _OSImplementation->window);
	xcb_flush(_OSImplementation->connection);
	return true;
}

void ShellOS::releaseWindow()
{
	xcb_destroy_window(_OSImplementation->connection, _OSImplementation->window);
	xcb_disconnect(_OSImplementation->connection);
}

OSApplication ShellOS::getApplication() const
{
	return _instance;
}

OSDisplay ShellOS::getDisplay() const
{
	return static_cast<OSDisplay>(_OSImplementation->connection);
}

OSWindow ShellOS::getWindow() const
{
	return  reinterpret_cast<void*>(&_OSImplementation->window);
}

bool ShellOS::handleOSEvents()
{
	xcb_generic_event_t* event = xcb_poll_for_event(_OSImplementation->connection);
	if (event)
	{
		free(event);
	}
	return true;

}


bool ShellOS::isInitialized()
{
	return _OSImplementation && _OSImplementation->window;
}

bool ShellOS::popUpMessage(const char* title, const char* message, ...) const
{
	if (!title && !message)
	{
		return false;
	}

	va_list arg;

	va_start(arg, message);
	Log(LogLevel::Information, message, arg);
	va_end(arg);

	return true;
}
}
}
//!\endcond