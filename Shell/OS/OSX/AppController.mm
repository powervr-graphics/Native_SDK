/******************************************************************************

 @File         AppController.mm

 @Title        

 @Version            

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     

 @Description  

******************************************************************************/
#import "AppController.h"
#import <AppKit/NSAlert.h>
#import <AppKit/NSEvent.h>

#include "PVRShellAPI.h"
#include "PVRShellOS.h"
#include "PVRShellImpl.h"

//CONSTANTS:

const int kFPS = 60.0;
#define kAccelerometerFrequency		30.0 //Hz
#define kFilteringFactor			0.1

// MACROS
#define DEGREES_TO_RADIANS(__ANGLE__) ((__ANGLE__) / 180.0 * M_PI)

// CLASS IMPLEMENTATION
@implementation AppController

// C to Objective-C interface functions. These get called by the C++ part of the PVRShell
void ObjC_OSInit(PVRShellInit* pInit)
{
	// Setup read and write paths
	NSString* readPath = [NSString stringWithFormat:@"%@%@", [[NSBundle mainBundle] resourcePath], @"/"];
	pInit->SetReadPath([readPath UTF8String]);

    NSString* writePath = [NSString stringWithFormat:@"%@%@", [[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent], @"/"];
	pInit->SetWritePath([writePath UTF8String]);
	
	// Setup the app name
    NSString* name = [[NSProcessInfo processInfo] processName];
    pInit->SetAppName([name UTF8String]);
}

bool ObjC_OSInitOS(PVRShellInit* pOS)
{
    PVRShell *pShell = pOS->m_pShell;
    
    // Now create our window
    NSRect frame;
    NSUInteger style;
    
    if(pShell->PVRShellGet(prefFullScreen))
    {
        // Set our frame to fill the screen
        frame = [[NSScreen mainScreen] frame];
        
        // Setup our fullscreen style
        style = NSBorderlessWindowMask;
    }
    else
    {
        // Get our dimensions and position as requested from the shell
        frame = NSMakeRect(pShell->PVRShellGet(prefPositionX), pShell->PVRShellGet(prefPositionY), pShell->PVRShellGet(prefWidth), pShell->PVRShellGet(prefHeight));
        
        // Setup our style
        style = NSMiniaturizableWindowMask | NSTitledWindowMask | NSClosableWindowMask;
    }
    
    AppWindow *window = [[AppWindow alloc] initWithContentRect:frame styleMask:style backing:NSBackingStoreBuffered defer:NO screen:[NSScreen mainScreen]];
    
    if(!window)
    {
        pShell->PVRShellSet(prefExitMessage, "Failed to create window.");
        return false;
    }
    
    // Give the window a copy of the PVRShell so it can update it with keyboard/mouse events
    [window setPvrshell:pOS];
    
    // Don't release our window when closed
    [window setReleasedWhenClosed:NO];
    
    // When the window is closed, terminate
    AppController *pAppController = (AppController*) pOS->m_pAppController;
    [[NSNotificationCenter defaultCenter] addObserver:pAppController selector:@selector(terminateApp) name:NSWindowWillCloseNotification object:window];
    
    // Set the window title
    [window setTitle:[NSString stringWithUTF8String:(const char*) pShell->PVRShellGet(prefAppName)]];
    
    // Now create the view that we'll render to
    NSView* view = [[NSView alloc] initWithFrame:frame];
    
    if(!view)
    {
        [window release];
        return false;
    }
    
    // Add our view to our window
    [window setContentView:view];    
    
    // Save a pointer to our window and view
    pOS->m_pWindow = (void*) window;
    pOS->m_pView = (void*) view;
    
    if(pShell->PVRShellGet(prefFullScreen))
    {
        // Set our window's level above the main menu window
        [window setLevel:NSMainMenuWindowLevel+1];
    }

    [window makeKeyAndOrderFront:nil];
    return true;
}

bool ObjC_OSReleaseOS(PVRShellInit* pOS)
{
    // dealloc our window
    NSWindow* window = (NSWindow*) pOS->m_pWindow;
    [window release];
    pOS->m_pWindow = NULL;
	return true;
}

bool ObjC_OSGet(PVRShellInit* pInit, const prefNameIntEnum prefName, int *pn)
{
    switch(prefName)
    {
        case prefButtonState:
        {
            AppWindow* window = (AppWindow*) pInit->m_pWindow;
            if(window)
            {
                *pn = [window mouseButtonState];
                return true;
            }
            return false;
        }
        default:
            return false;
    }
}

bool ObjC_ExitMessage(PVRShellInit* pInit, const char * const pExitMessage)
{
	if(pExitMessage)
	{
        NSWindow* window = (NSWindow*) pInit->m_pWindow;
        
        if(window)
        {
            if([window level] > NSMainMenuWindowLevel) // If our window's level is above the main menu, minimise the window so we can see the alert
                [window miniaturize:nil];
        }   
        
        NSAlert * alert = [NSAlert alertWithMessageText:@"Exit message" defaultButton:@"OK" alternateButton:nil otherButton:nil informativeTextWithFormat:@"%@",[NSString stringWithUTF8String:pExitMessage]];

        [alert runModal];
	}
	
	return true;
}

- (void) mainLoop
{
	if(!pvrshellInit->Run())
	{		
        [self terminateApp];
	}
}

- (void) applicationDidFinishLaunching:(NSNotification *)notification
{
	pvrshellInit = new PVRShellInit();

	if(!pvrshellInit)
	{
		NSLog(@"Failed to allocate m_pPVRShellInit.\n");
        [self terminateApp];
	}
	
	// Give PVRShell init a pointer to this class
	pvrshellInit->m_pAppController = (void*) self;
	
	if(!pvrshellInit->Init())
	{
		NSLog(@"Failed to initialise m_pPVRShellInit\n");
		[self terminateApp];
	}
	
	// Parse the command-line
    NSMutableString *cl = [[NSMutableString alloc] init];
    NSArray *args = [[NSProcessInfo processInfo] arguments];
    
    for(NSUInteger i = 1;i < [args count]; ++i)
    {
        [cl appendString:[args objectAtIndex:i]];
        [cl appendString:@" "];
    }
    
	pvrshellInit->CommandLine([cl UTF8String]);
    [cl release];
         
	mainLoopTimer = [NSTimer scheduledTimerWithTimeInterval:(1.0 / kFPS) target:self selector:@selector(mainLoop) userInfo:nil repeats:YES];	
}

- (void) applicationWillTerminate:(NSNotification *)notification
{
    [mainLoopTimer invalidate];
    mainLoopTimer = nil;
    
    delete pvrshellInit;
    pvrshellInit = 0;
}

- (void) terminateApp
{
    [NSApp terminate:nil];
}

@end

// CLASS IMPLEMENTATION
@implementation AppWindow

@synthesize pvrshell;
@synthesize mouseButtonState;

- (void)keyDown:(NSEvent*) event
{
    if(pvrshell)
    {
        // Handle the keyboard input
        switch([event keyCode])
        {
            case 0x12: pvrshell->KeyPressed(PVRShellKeyNameACTION1);    break;    // kVK_ANSI_1
            case 0x13: pvrshell->KeyPressed(PVRShellKeyNameACTION2);    break;    // kVK_ANSI_2
                
            case 0x31: pvrshell->KeyPressed(PVRShellKeyNameSELECT);     break;    // kVK_Space
                
            case 0x69: pvrshell->KeyPressed(PVRShellKeyNameScreenshot); break;    // kVK_F13
                
            case 0x7B: pvrshell->KeyPressed(pvrshell->m_eKeyMapLEFT);break;   // kVK_LeftArrow
            case 0x7C: pvrshell->KeyPressed(pvrshell->m_eKeyMapRIGHT);break;  // kVK_RightArrow
            case 0x7D: pvrshell->KeyPressed(pvrshell->m_eKeyMapDOWN);break;   // kVK_DownArrow
            case 0x7E: pvrshell->KeyPressed(pvrshell->m_eKeyMapUP); break;    // kVK_UpArrow
                
            case 0x35: pvrshell->KeyPressed(PVRShellKeyNameQUIT);       break;    // kVK_Escape
        }
    }
}

- (void)mouseDown:(NSEvent*)event
{
    if(pvrshell)
    {
        switch([event type])
        {
            case NSLeftMouseDown:
            {
                mouseButtonState |= ePVRShellButtonLeft;
                
                NSPoint location = [event locationInWindow];
                
                float mouseLocation[2];
                mouseLocation[0] = location.x / [self frame].size.width;
                mouseLocation[1] = 1.0f - (location.y / [self frame].size.height);
                pvrshell->TouchBegan(mouseLocation);
            }
            break;
            case NSRightMouseDown: 
                mouseButtonState |= ePVRShellButtonRight;
            break;
            default: {}
        }
    }
}

- (void)mouseUp:(NSEvent*)event
{
    switch([event type])
    {
        case NSLeftMouseUp:
        {
            mouseButtonState &= ~ePVRShellButtonLeft;
            
            NSPoint location = [event locationInWindow];
            
            float mouseLocation[2];
            mouseLocation[0] = location.x / [self frame].size.width;
            mouseLocation[1] = 1.0f - (location.y / [self frame].size.height);
            pvrshell->TouchEnded(mouseLocation);
        }   
        break;
        case NSRightMouseUp: 
            mouseButtonState &= ~ePVRShellButtonRight; 
            break;
        default: {}
    }
}

- (void)mouseDragged:(NSEvent*)event
{    
    if([event type] == NSLeftMouseDragged)
    {
        NSPoint location = [event locationInWindow];
        
        float mouseLocation[2];
        mouseLocation[0] = location.x / [self frame].size.width;
        mouseLocation[1] = 1.0f - (location.y / [self frame].size.height);
        pvrshell->TouchMoved(mouseLocation);
    }
}

-(BOOL)canBecomeKeyWindow
{
    return YES;
}

@end

