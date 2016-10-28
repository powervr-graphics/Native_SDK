/*!*********************************************************************************************************************
\file         PVRShell\OS\AppKit\ShellOS.mm
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief			Implementation of the OS specific part of the PVRShell class for the AppKit OSX implementation
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRShell/OS/ShellOS.h"
#include "PVRCore/FilePath.h"
#include <mach/mach_time.h>
#include <Foundation/Foundation.h>
#include <AppKit/NSWindow.h>
#include <AppKit/NSScreen.h>
#include <AppKit/NSView.h>
#include <AppKit/NSAlert.h>

@interface AppWindow : NSWindow < NSWindowDelegate> // The implementation appears at the bottom of this file
{
pvr::Shell* eventQueue;
}
@property (assign) pvr::Shell * eventQueue;
@end
using namespace ::pvr::platform;
namespace pvr{namespace platform{
struct InternalOS
{
	AppWindow* window;
	NSView* view;

	InternalOS() : window(nil), view(nil)
	{
	}
};
}
}

pvr::int16 g_cursorX, g_cursorY;

// Setup the capabilities
const pvr::platform::ShellOS::Capabilities pvr::platform::ShellOS::m_capabilities = { pvr::types::Capability::Immutable, pvr::types::Capability::Immutable };

ShellOS::ShellOS(/*NSObject<NSApplicationDelegate>*/void* hInstance, OSDATA osdata) : m_instance(hInstance)
{
	m_OSImplementation = new InternalOS;
}

ShellOS::~ShellOS()
{
	delete m_OSImplementation;
}

pvr::Result ShellOS::init(DisplayAttributes &data)
{
	if(!m_OSImplementation)
		return pvr::Result::OutOfMemory;
	// Setup read and write paths
	NSString* readPath = [NSString stringWithFormat:@"%@%@", [[NSBundle mainBundle] resourcePath], @"/"];
	//m_Strings[eReadPath] = [readPath UTF8String];
    m_ReadPaths.push_back([readPath UTF8String]);
    NSString* writePath = [NSString stringWithFormat:@"%@%@", [[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent], @"/"];
    m_WritePath = [writePath UTF8String];
	// Setup the app name
    NSString* name = [[NSProcessInfo processInfo] processName];
    
    m_AppName =[name UTF8String];
	return pvr::Result::Success;
}


void ShellOS::updatePointingDeviceLocation()
{
    m_shell->updatePointerPosition(PointerLocation(g_cursorX, g_cursorY));
}

pvr::Result ShellOS::initializeWindow(pvr::platform::DisplayAttributes &data)
{
	 // Now create our window
    NSRect frame;
    NSUInteger style;
    
    if(data.fullscreen)
    {
        // Set our frame to fill the screen
        frame = [[NSScreen mainScreen] frame];
        
        // Setup our fullscreen style
        style = NSBorderlessWindowMask;
        
        data.x = frame.origin.x;
        data.y = frame.origin.y;
        data.width = frame.size.width;
        data.height = frame.size.height;
    }
    else
    {
        // Get our dimensions and position as requested from the shell
        frame = NSMakeRect(data.x, data.y, data.width, data.height);
        
        // Setup our style
        style = NSMiniaturizableWindowMask | NSTitledWindowMask | NSClosableWindowMask;
    }
    
    m_OSImplementation->window = [[AppWindow alloc] initWithContentRect:frame styleMask:style backing:NSBackingStoreBuffered defer:NO screen:[NSScreen mainScreen]];
    
    if(!m_OSImplementation->window)
    {
        NSLog(@"Failed to allocated the window.");
        return pvr::Result::UnknownError;
    }
    m_OSImplementation->window.eventQueue = m_shell.get();
	
    // Don't release our window when closed
    [m_OSImplementation->window setReleasedWhenClosed:NO];
    // When the window is closed, terminate the application
    [[NSNotificationCenter defaultCenter] addObserver:(__bridge NSObject*)m_instance selector:@selector(terminateApp) name:NSWindowWillCloseNotification object:m_OSImplementation->window];
    
    // Set the window title
    [m_OSImplementation->window setTitle:[NSString stringWithUTF8String:data.windowTitle.c_str()]];
    
    // Now create the view that we'll render to
    m_OSImplementation->view = [[NSView alloc] initWithFrame:frame];
    
    if(!m_OSImplementation->view)
    {
        return pvr::Result::UnknownError;
    }
    
    // Add our view to our window
    [m_OSImplementation->window setContentView:m_OSImplementation->view];
    
    // Save a pointer to our window and view
    if(data.fullscreen)
    {
        // Set our window's level above the main menu window
        [m_OSImplementation->window setLevel:NSMainMenuWindowLevel+1];
    }

    [m_OSImplementation->window makeKeyAndOrderFront:nil];
	return pvr::Result::Success;
}

void ShellOS::releaseWindow()
{
	if(m_OSImplementation->window)
	{
        m_OSImplementation->view = nil;
	}
}

OSApplication ShellOS::getApplication() const
{
	return NULL;
}

OSDisplay ShellOS::getDisplay() const
{
	return NULL;
}

OSWindow ShellOS::getWindow() const
{
	return (__bridge OSWindow)m_OSImplementation->view;
}

pvr::Result ShellOS::handleOSEvents()
{
	// Nothing to do
	return pvr::Result::Success;
}

bool ShellOS::isInitialized()
{
	return m_OSImplementation && m_OSImplementation->window;
}

pvr::Result ShellOS::popUpMessage(const pvr::tchar *const title, const pvr::tchar *const message, ...)const
{
    if(title && message)
    {
        va_list arg;
        
        va_start(arg, message);
        // NSString *fullMessage = [[[NSString alloc] initWithFormat:[NSString stringWithUTF8String:message] arguments:arg] autorelease];
        NSString *fullMessage = [[NSString alloc] initWithFormat:[NSString stringWithUTF8String:message] arguments:arg];
        va_end(arg);
        
        
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:@"OK"];
        [alert setMessageText:fullMessage];
        [alert runModal];
    }
    
    return Result::Success;
}

static pvr::Keys mapNSKeyToPvrKey(unsigned short keyCode)
{
    switch(keyCode){
    
        case 0x31: // kVK_Space
            {
                return pvr::Keys::Space;
            }
            break;
    
        case 0x7B: // kVK_LeftArrow
            {
                return pvr::Keys::Left;
            }
            break;
        case 0x7C: // kVK_RightArrow
            {
                return pvr::Keys::Right;
            }
            break;
        case 0x7D: // kVK_DownArrow
            {
                return pvr::Keys::Down;
            }
            break;
        case 0x7E: // kVK_UpArrow
            {
                return pvr::Keys::Up;
            }
            break;
            
        case 0x35: // kVK_Escape
            {
                return pvr::Keys::Escape;
            }
            break;
        case 0x24:// kVK_Return
            {
                return pvr::Keys::Return;
            }
            break;
               
        case 0x53:// kVK_ANSI_Keypad1
            {
                return pvr::Keys::Key1;
            }
            break;
    }
    return pvr::Keys(0);
}

// CLASS IMPLEMENTATION
@implementation AppWindow

@synthesize eventQueue;

- (void)keyDown:(NSEvent*) event
{
    eventQueue->onKeyDown(mapNSKeyToPvrKey([event keyCode]));
}

- (void)keyUp:(NSEvent*) event
{
    eventQueue->onKeyUp(mapNSKeyToPvrKey([event keyCode]));
}

- (void)mouseDown:(NSEvent*)event
{
    switch([event type])
    {
        case NSLeftMouseDown:
        {
            NSPoint location = [event locationInWindow];
            g_cursorX = location.x;
            g_cursorY = location.y;
            eventQueue->onPointingDeviceDown(0);
        }
        break;
        case NSRightMouseDown:
        {
            NSPoint location = [event locationInWindow];
            g_cursorX = location.x;
            g_cursorY = location.y;
            eventQueue->onPointingDeviceDown(1);
        }
        break;
        default: break;
    }
}

- (void)mouseUp:(NSEvent*)event
{
    switch([event type])
    {
        case NSLeftMouseUp:
        {
            NSPoint location = [event locationInWindow];
            g_cursorX = location.x;
            g_cursorY = location.y;
            eventQueue->onPointingDeviceUp(0);
        }
        break;
        case NSRightMouseUp:
        {
            NSPoint location = [event locationInWindow];
            g_cursorX = location.x;
            g_cursorY = location.y;
            eventQueue->onPointingDeviceUp(1);
        }
        break;
        default: break;
    }
}

- (void)mouseDragged:(NSEvent*)event
{
}

-(BOOL)canBecomeKeyWindow
{
    return YES;
}

@end
//!\endcond