/*!
\brief Contains an implementation of pvr::platform::ShellOS for Linux X11 systems.
\file PVRShell/OS/Wayland/ShellOS.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRShell/OS/ShellOS.h"
#include "PVRCore/IO/FilePath.h"
#include "PVRCore/Log.h"

#include <wayland-client.h>
#include <wayland-server.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstdarg>
#include <memory>
#include <cstdio>
#include <cstring>
#include <linux/input.h>

namespace pvr {
namespace platform {
/****************************************************************************
        Defines
        *****************************************************************************/
struct InternalOS
{
    wl_display*         display;
    wl_registry*        registry;
    wl_compositor*      compositor;
    wl_shell*           shell;
    wl_seat*            seat;
    wl_pointer*         pointer;
    wl_keyboard*        keyboard;
    wl_callback *       callback;
    wl_surface*         wayland_surface;
    wl_shell_surface *  shellSurface;
    int16_t               pointerXY[2];
    Shell*              pvrShell;
    InternalOS() : display(0), registry(nullptr), compositor(nullptr), shell(nullptr),
        seat(nullptr), pointer(nullptr), keyboard(nullptr), wayland_surface(nullptr), shellSurface(nullptr),
        pvrShell(nullptr)
    {}
};



static Keys Wayland_To_Keycode[255] =
{
    Keys::Unknown,// 0
    Keys::Escape,
    Keys::Key1,
    Keys::Key2,
    Keys::Key3,
    Keys::Key4,
    Keys::Key5,
    Keys::Key6,
    Keys::Key7,
    Keys::Key8,

    Keys::Key9,//10
    Keys::Key0,
    Keys::Minus,
    Keys::Equals,
    Keys::Backspace,
    Keys::Tab,
    Keys::Q,
    Keys::W,
    Keys::E,
    Keys::R,

    Keys::T,// 20
    Keys::Y,
    Keys::U,
    Keys::I,
    Keys::O,
    Keys::P,
    Keys::SquareBracketLeft,
    Keys::SquareBracketRight,
    Keys::Return,
    Keys::Control,

    Keys::A, // 30
    Keys::S,
    Keys::D,
    Keys::F,
    Keys::G,
    Keys::H,
    Keys::J,
    Keys::K,
    Keys::L,
    Keys::Semicolon,

    Keys::Quote,// 40
    Keys::Backquote,
    Keys::Shift,
    Keys::Backslash,
    Keys::Z,
    Keys::X,
    Keys::C,
    Keys::V,
    Keys::B,
    Keys::N,

    Keys::M, // 50
    Keys::Comma,
    Keys::Period,
    Keys::Slash,
    Keys::Shift,
    Keys::NumMul,
    Keys::Alt,
    Keys::Space,
    Keys::CapsLock,
    Keys::F1,

    Keys::F2, // 60
    Keys::F3,
    Keys::F4,
    Keys::F5,
    Keys::F6,
    Keys::F7,
    Keys::F8,
    Keys::F9,
    Keys::F10,
    Keys::NumLock,

    Keys::ScrollLock,// 70
    Keys::Num7,
    Keys::Num8,
    Keys::Num9,
    Keys::NumSub,
    Keys::Num4,
    Keys::Num5,
    Keys::Num6,
    Keys::NumAdd,
    Keys::Num1,

    Keys::Num2,// 80
    Keys::Num3,
    Keys::Num0,
    Keys::NumPeriod,
    Keys::Unknown,
    Keys::Unknown,
    Keys::Backslash,
    Keys::F11,
    Keys::F12,
    Keys::Unknown,

    Keys::Unknown,// 90
    Keys::Unknown,
    Keys::Unknown,
    Keys::Unknown,
    Keys::Unknown,
    Keys::Unknown,
    Keys::Return,
    Keys::Control,
    Keys::NumDiv,
    Keys::PrintScreen,

    Keys::Alt,// 100
    Keys::Unknown,
    Keys::Home,
    Keys::Up,
    Keys::PageUp,
    Keys::Left,
    Keys::Right,
    Keys::End,
    Keys::Down,
    Keys::PageDown,

    Keys::Insert,// 110
    Keys::Delete,
    Keys::Unknown,
    Keys::Unknown,
    Keys::Unknown,
    Keys::Unknown,
    Keys::Unknown,
    Keys::Unknown,
    Keys::Unknown,
    Keys::Pause,

    Keys::Unknown,// 120
    Keys::Unknown,
    Keys::Unknown,
    Keys::Unknown,
    Keys::Unknown,
    Keys::SystemKey1,
    Keys::SystemKey1,
    Keys::SystemKey2,
    Keys::Unknown,
    Keys::Unknown,

    Keys::Unknown,// 130
    Keys::Unknown,
    Keys::Unknown,
    Keys::Unknown,
    Keys::Unknown,
    Keys::Unknown,
};

static Keys getKeyFromWaylandCode(uint32_t keycode)
{
    if (keycode >= ARRAY_SIZE(Wayland_To_Keycode)) { return Keys::Unknown; }
    return Wayland_To_Keycode[keycode];
}

void ShellOS::updatePointingDeviceLocation()
{
    _shell->updatePointerPosition(PointerLocation(static_cast<int16_t>(_OSImplementation->pointerXY[0]),
                                  static_cast<int16_t>(_OSImplementation->pointerXY[1])));
}

// Setup the capabilities
const ShellOS::Capabilities ShellOS::_capabilities = { Capability::Immutable, Capability::Immutable };

ShellOS::ShellOS(void* hInstance, OSDATA osdata) : _instance(hInstance)
{
    _OSImplementation = new InternalOS;
}

ShellOS::~ShellOS()
{
    delete _OSImplementation;
}

bool ShellOS::init(DisplayAttributes& data)
{
    (void)data;
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


static void
keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard,
                       uint32_t format, int fd, uint32_t size)
{
}

static void
keyboard_handle_enter(void *data, struct wl_keyboard *keyboard,
                      uint32_t serial, struct wl_surface *surface,
                      struct wl_array *keys)
{
}

static void
keyboard_handle_leave(void *data, struct wl_keyboard *keyboard,
                      uint32_t serial, struct wl_surface *surface)
{
}

static void
keyboard_handle_key(void *data, struct wl_keyboard *keyboard,
                    uint32_t serial, uint32_t time, uint32_t key,
                    uint32_t state)
{
    InternalOS* internlOs = (InternalOS*)data;
    if(state)
    {
        internlOs->pvrShell->onKeyDown(getKeyFromWaylandCode(key));
    }
    else
    {
        internlOs->pvrShell->onKeyUp(getKeyFromWaylandCode(key));
    }
}

static void
keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard,
                          uint32_t serial, uint32_t mods_depressed,
                          uint32_t mods_latched, uint32_t mods_locked,
                          uint32_t group)
{
}

static const struct wl_keyboard_listener keyboard_listener = {
    keyboard_handle_keymap,
    keyboard_handle_enter,
    keyboard_handle_leave,
    keyboard_handle_key,
    keyboard_handle_modifiers,
};



static void pointer_handle_enter(void *data, struct wl_pointer *pointer,
                                 uint32_t serial, struct wl_surface *surface,
                                 wl_fixed_t sx, wl_fixed_t sy)
{
}

static void pointer_handle_leave(void *data, struct wl_pointer *pointer,
                                 uint32_t serial, struct wl_surface *surface)
{
}

static void pointer_handle_motion(void *data, struct wl_pointer *pointer,
                                  uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
{
    InternalOS* displayHandle = (InternalOS*)data;
    displayHandle->pointerXY[0] = static_cast<int16_t>(sx);
    displayHandle->pointerXY[1] = static_cast<int16_t>(sy);
}

static void pointer_handle_button(
        void *data, struct wl_pointer *wl_pointer,
        uint32_t serial, uint32_t time, uint32_t button,
        uint32_t state)
{
    InternalOS* displayHandle = (InternalOS*)data;
    if (button == BTN_LEFT && state == WL_POINTER_BUTTON_STATE_PRESSED)
    {
        wl_shell_surface_move(displayHandle->shellSurface, displayHandle->seat, serial);
    }
}

static void
pointer_handle_axis(void *data, struct wl_pointer *wl_pointer,
                    uint32_t time, uint32_t axis, wl_fixed_t value)
{
    Log(LogLevel::Debug, "Pointer handle axis\n");
}

static const struct wl_pointer_listener pointer_listener =
{
    pointer_handle_enter,
    pointer_handle_leave,
    pointer_handle_motion,
    pointer_handle_button,
    pointer_handle_axis,
};

static void seatHandleCapabilities(void *data, struct wl_seat *seat, uint32_t caps)
{
    pvr::platform::InternalOS& internalOS = *(pvr::platform::InternalOS*)data;
    if ((caps & WL_SEAT_CAPABILITY_POINTER) && !internalOS.pointer)
    {
        internalOS.pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(internalOS.pointer, &pointer_listener, data);
        Log(LogLevel::Debug, "seatHandleCapabilities add pointer listener");
    }
    if (!(caps & WL_SEAT_CAPABILITY_POINTER) && internalOS.pointer)
    {
        wl_pointer_destroy(internalOS.pointer);
        internalOS.pointer = nullptr;
        Log(LogLevel::Debug, "seatHandleCapabilities destroy pointer listener");
    }
    if (caps & WL_SEAT_CAPABILITY_KEYBOARD && (!internalOS.keyboard))
    {
        internalOS.keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(internalOS.keyboard, &keyboard_listener, data);
    }
    if(!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && internalOS.keyboard)
    {
        wl_keyboard_destroy(internalOS.keyboard);
        internalOS.keyboard = nullptr;
        Log(LogLevel::Debug, "seatHandleCapabilities destroy keyboard listener");
    }
}
static void seat_handle_name(void   *data,
                             struct wl_seat  *seat,
                             const char  *name)
{
}



static const struct wl_seat_listener seatListener =
{
    seatHandleCapabilities,
    seat_handle_name
};

static void registerGlobalCallback(
        void *data, wl_registry* registry, uint32_t name,
        const char* interface, uint32_t version)
{
    pvr::platform::InternalOS& internalOS = *(pvr::platform::InternalOS*)data;

    if (strcmp(interface, "wl_compositor") == 0)
    {
        internalOS.compositor = (wl_compositor *) wl_registry_bind(registry, name,
                                                                   &wl_compositor_interface, 1);
    }
    else if (strcmp(interface, "wl_shell") == 0)
    {
        internalOS.shell = (wl_shell *) wl_registry_bind(registry, name,
                                                         &wl_shell_interface, 1);
    }
    else if(strcmp(interface, "wl_seat") == 0)
    {
        internalOS.seat = (wl_seat *)wl_registry_bind(registry, name, &wl_seat_interface, 1);
        wl_seat_add_listener(internalOS.seat, &seatListener, data);
    }
}

static void globalObjectRemove(void *data, struct wl_registry *wl_registry, uint32_t name) {}

static const wl_registry_listener registryListener =
{
    registerGlobalCallback,
    globalObjectRemove
};
bool initWaylandConnection(pvr::platform::InternalOS& internalOS)
{
    if((internalOS.display = wl_display_connect(NULL)) == NULL)
    {
        Log(LogLevel::Error, "Failed to connect to Wayland display!");
        return false;
    }

    if((internalOS.registry = wl_display_get_registry(internalOS.display)) == NULL)
    {
        Log(LogLevel::Error,  "Faield to get Wayland registry!");
        return false;
    }

    wl_registry_add_listener(internalOS.registry, &registryListener, &internalOS);
    wl_display_dispatch(internalOS.display);
    if (!internalOS.compositor)// || !internalOS.seat)
    {
        Log(LogLevel::Error, "Could not bind Wayland protocols!");
        return false;
    }

    return true;
}

void releaseWaylandConnection(pvr::platform::InternalOS& internalOS)
{
    wl_shell_surface_destroy(internalOS.shellSurface);
    wl_surface_destroy(internalOS.wayland_surface);
    if (internalOS.keyboard)
    {
        wl_keyboard_destroy(internalOS.keyboard);
    }
    if (internalOS.pointer)
    {
        wl_pointer_destroy(internalOS.pointer);
    }
    wl_seat_destroy(internalOS.seat);
    wl_shell_destroy(internalOS.shell);
    wl_compositor_destroy(internalOS.compositor);
    wl_registry_destroy(internalOS.registry);
    wl_display_disconnect(internalOS.display);

    memset(&internalOS,0, sizeof(internalOS));
}

static void ping_cb(void *data, struct wl_shell_surface *shell_surface, uint32_t serial)
{
    wl_shell_surface_pong(shell_surface, serial);
}

static void configure_cb(void* /*data*/, struct wl_shell_surface* /*shell_surface*/,
                         uint32_t /*edges*/, int32_t /*width*/, int32_t /*height*/)
{
}

static void popupDone_cb(void* /*data*/, struct wl_shell_surface* /*shell_surface*/)
{
}

static void redraw(void *data, struct wl_callback *callback, uint32_t time)
{
    Log("Redrawing\n");
}

static const struct wl_callback_listener FrameListener =
{
    redraw
};

static void configure_cb(void *data, struct wl_callback *callback, uint32_t  time)
{
    if (callback == NULL)
        redraw(data, NULL, time);
}

static struct wl_callback_listener ConfigureCbListener =
{
    configure_cb,
};

static const struct wl_shell_surface_listener shellSurfaceListeners =
{
    ping_cb,
    configure_cb,
    popupDone_cb
};

bool ShellOS::initializeWindow(DisplayAttributes& data)
{
    _OSImplementation->pvrShell = getShell();
    if(!initWaylandConnection(*_OSImplementation)) return false;

    _OSImplementation->wayland_surface = wl_compositor_create_surface(_OSImplementation->compositor);
    if(_OSImplementation->wayland_surface == NULL)
    {
        Log("Failed to create Wayland surface");
        return false;
    }

    _OSImplementation->shellSurface = wl_shell_get_shell_surface(_OSImplementation->shell, _OSImplementation->wayland_surface);
    if(_OSImplementation->shellSurface == NULL)
    {
        Log("Failed to get Wayland shell surface");
        return false;
    }

    wl_shell_surface_add_listener(_OSImplementation->shellSurface, &shellSurfaceListeners, _OSImplementation);
    wl_shell_surface_set_toplevel(_OSImplementation->shellSurface);
    wl_shell_surface_set_title(_OSImplementation->shellSurface, data.windowTitle.c_str());
    return true;
}

void ShellOS::releaseWindow()
{
}

OSApplication ShellOS::getApplication() const
{
    return _instance;
}

OSDisplay ShellOS::getDisplay() const
{
    return static_cast<OSDisplay>(_OSImplementation->display);
}

OSWindow ShellOS::getWindow() const
{
    return  reinterpret_cast<void*>(_OSImplementation->wayland_surface);
}

bool ShellOS::handleOSEvents()
{
    wl_display_dispatch_pending(_OSImplementation->display);
    return true;
}


bool ShellOS::isInitialized()
{
    return _OSImplementation && _OSImplementation->wayland_surface;
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
