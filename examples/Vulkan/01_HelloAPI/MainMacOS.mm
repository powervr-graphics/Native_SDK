/*!*********************************************************************************************************************
\File         MainMacOS.mm
\Title        Main MacOS
\Author       PowerVR by Imagination, Developer Technology Team.
\Copyright    Copyright(c) Imagination Technologies Limited.
\brief        Adds the entry point for running the example on a MacOS surface platform.
***********************************************************************************************************************/

#include "VulkanHelloAPI.h"
#include "../framework/PVRShell/OS/AppKit/ViewMVK.h"

#include <AppKit/NSApplication.h>
#include <AppKit/NSWindow.h>
#include <Foundation/NSTimer.h>

@class AppController;

@interface AppController : NSObject <NSApplicationDelegate>
{
@private
    NSWindow*      m_window; // Our window
    ViewMVK*       m_view;   // Our view
    NSTimer*       m_timer;  // Our view

    VulkanHelloAPI m_vulkanExample;
}
@end

@implementation AppController

- (void) mainLoop
{
    m_vulkanExample.drawFrame();
}

- (void) applicationDidFinishLaunching:(NSNotification *)notification
{
    m_vulkanExample.surfaceData.width = 1280.0f;
    m_vulkanExample.surfaceData.height = 800.0f;
    
    // Create our window
    NSRect frame = NSMakeRect(0, 0, m_vulkanExample.surfaceData.width, m_vulkanExample.surfaceData.height);
    m_window = [[NSWindow alloc] initWithContentRect:frame styleMask:NSMiniaturizableWindowMask | NSTitledWindowMask | NSClosableWindowMask backing:NSBackingStoreBuffered defer:NO];
    
    if(!m_window)
    {
        NSLog(@"Failed to allocated the window.");
        [self terminateApp];
    }
    
    [m_window setTitle:@"VulkanHelloAPI"];
    
    // Create our view
    m_view = [[ViewMVK alloc] initWithFrame:frame];
    m_vulkanExample.surfaceData.view = m_view;
    m_vulkanExample.initialize();
    m_vulkanExample.recordCommandBuffer();
    
    // Now we have a view, add it to our window
    [m_window setContentView:m_view];
    [m_window makeKeyAndOrderFront:nil];
    
    // Add an observer so when our window is closed we terminate the app
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(terminateApp) name:NSWindowWillCloseNotification object:m_window];
    
    m_timer = [NSTimer scheduledTimerWithTimeInterval:(1.0 / 60.0) target:self selector:@selector(mainLoop) userInfo:nil repeats:YES];
}

- (void) applicationWillTerminate:(NSNotification *)notification
{
    m_vulkanExample.deinitialize();
    m_vulkanExample.surfaceData.view = nullptr;
    
    // Release our view and window
    [m_view release];
    m_view = nil;
    
    [m_window release];
    m_window = nil;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}

- (void) terminateApp
{
    [NSApp terminate:nil];
}

@end

int main(int argc, char** argv)
{
    return NSApplicationMain(argc, (const char **)argv);
}
