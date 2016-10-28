/*!*********************************************************************************************************************
\file         PVRShell\OS\UIKit\ShellOS.mm
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Implementation of the ShellOS class for the UIKit
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "OS/ShellOS.h"
#include "PVRCore/FilePath.h"
#include <mach/mach_time.h>
#import <UIKit/UIKit.h>

@interface AppWindow : UIWindow // The implementation appears at the bottom of this file
{
    pvr::Shell* eventQueue;
	CGFloat screenScale;
}

@property (assign) pvr::Shell * eventQueue;
@property (assign) CGFloat screenScale;
@end
namespace pvr{
namespace platform{
struct InternalOS
{
	AppWindow* window;

	InternalOS() : window(nil)
	{
	}
};
}
}
pvr::int16 g_cursorX, g_cursorY;

namespace pvr{
namespace platform{
// Setup the capabilities
const ShellOS::Capabilities ShellOS::m_capabilities = { types::Capability::Immutable, types::Capability::Immutable };

ShellOS::ShellOS(/*NSObject<NSApplicationDelegate>*/void* hInstance, OSDATA osdata) : m_instance(hInstance)
{
	m_OSImplementation = new InternalOS;
}

ShellOS::~ShellOS()
{
	delete m_OSImplementation;
}

void ShellOS::updatePointingDeviceLocation()
{
    m_shell->updatePointerPosition(PointerLocation(g_cursorX, g_cursorY));
}

Result ShellOS::init(DisplayAttributes &data)
{
	if(!m_OSImplementation)
		return Result::OutOfMemory;

	// Setup read and write paths
	NSString* readPath = [NSString stringWithFormat:@"%@%@", [[NSBundle mainBundle] resourcePath], @"/"];
	m_ReadPaths.push_back([readPath UTF8String]);

	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString* writePath = [NSString stringWithFormat:@"%@%@", [paths objectAtIndex:0], @"/"];
	m_WritePath = [writePath UTF8String];
    
	// Setup the app name
    NSString* name = [[NSProcessInfo processInfo] processName];
    m_AppName =[name UTF8String];
    
	return Result::Success;
}

Result ShellOS::initializeWindow(DisplayAttributes &data)
{
	CGFloat scale = 1.0;
	
	// Now create our window
    data.fullscreen = true;
 
	UIScreen* screen = [UIScreen mainScreen];
	if([UIScreen instancesRespondToSelector:@selector(scale)])
	{
		scale = [screen scale];
	}
	
	// Set our frame to fill the screen
    CGRect frame = [screen applicationFrame];

    data.x = frame.origin.x;
    data.y = frame.origin.y;
    data.width = frame.size.width * scale;
    data.height = frame.size.height * scale;

    m_OSImplementation->window = [[AppWindow alloc] initWithFrame:frame];
    
    if(!m_OSImplementation->window) {  return Result::UnknownError;   }
    
    // pass the shell as the event queue
    m_OSImplementation->window.eventQueue = m_shell.get();
    
	// Give the window a copy of the eventQueue so it can pass on the keyboard/mouse events
	[m_OSImplementation->window setScreenScale:scale];
	[m_OSImplementation->window makeKeyAndVisible];
	
	return Result::Success;
}

void ShellOS::releaseWindow()
{
	if(m_OSImplementation->window)
	{
	//	[m_OSImplementation->window release];
		m_OSImplementation->window = nil;
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
	return (__bridge OSWindow)m_OSImplementation->window;
}

Result ShellOS::handleOSEvents()
{
	// Nothing to do
	return Result::Success;
}

bool ShellOS::isInitialized()
{
	return m_OSImplementation && m_OSImplementation->window;
}

Result ShellOS::popUpMessage(const tchar * const title, const tchar * const message, ...) const
{
    if(title && message)
    {
        va_list arg;

        va_start(arg, message);
 
        NSString *fullMessage = [[NSString alloc] initWithFormat:[NSString stringWithUTF8String:message] arguments:arg];
        UIAlertView *alert = [[UIAlertView alloc] initWithTitle:[NSString stringWithUTF8String:title]
                                                    message:fullMessage
                                                    delegate:nil
                                                    cancelButtonTitle:@"OK"
                                                    otherButtonTitles:nil];
        
        va_end(arg);
        
        [alert show];
      //  [alert release];
    }
    
    return Result::Success;
}
}
}


// CLASS IMPLEMENTATION
@implementation AppWindow

@synthesize eventQueue;
@synthesize screenScale;


- (void)sendEvent:(UIEvent *)event 
{
    pvr::assertion(eventQueue != NULL, "Event Queue Is NULL");
    if(event.type == UIEventTypeTouches) 
	{
        for(UITouch * t in [event allTouches]) 
		{
			switch(t.phase)
			{
				case UITouchPhaseBegan:
				{
					CGPoint location = [t locationInView:self];
                    g_cursorX =(location.x * screenScale);
                    g_cursorY = (location.y) * screenScale;
                    if(eventQueue->isScreenRotated() && eventQueue->isFullScreen())
                    {
                        std::swap(g_cursorX,g_cursorY);
                    }
                    eventQueue->onPointingDeviceDown(0);
                    
                
                }
				break;
				case UITouchPhaseMoved:
                {
                }
				break;
				case UITouchPhaseEnded:
				case UITouchPhaseCancelled:
                {
                    CGPoint location = [t locationInView:self];
                   
                    g_cursorX =(location.x * screenScale);
                    g_cursorY =(location.y * screenScale);
                    if(eventQueue->isScreenRotated() && eventQueue->isFullScreen())
                    {
                        std::swap(g_cursorX,g_cursorY);
                    }
                    eventQueue->onPointingDeviceUp(0);
                    
                }
				break;
			}
        }
    }

    [super sendEvent:event];
}

@end
//!\endcond