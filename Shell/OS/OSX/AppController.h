/*!****************************************************************************

 @file         OSX/AppController.h
 @ingroup      OS_OSX
 @copyright    Copyright (c) Imagination Technologies Limited.
 @brief        Main controller for OSX apps. 
               Manages the high-level tasks of the application such as 
               bringing the view to the foreground, creating a render loop, 
               and terminating the application properly. 

******************************************************************************/

#ifndef _APPCONTROLLER_H_
#define _APPCONTROLLER_H_

#include "PVRShell.h"
#import <AppKit/NSApplication.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSScreen.h>
#import <AppKit/NSView.h>
#import <Foundation/NSTimer.h>

/*!
 @addtogroup   OS_OSX
 @{
*/

//CLASS INTERFACES:

/*!****************************************************************************
 @interface    AppController
 @brief        Main controller class for OSX apps.  
******************************************************************************/

@interface AppController : NSObject <NSApplicationDelegate>
{
	NSTimer*				mainLoopTimer;	// timer for the main loop
	PVRShellInit*           pvrshellInit;
}

- (void) terminateApp;

@end


/*!****************************************************************************
 @interface    AppWindow
 @brief        Main window class for OSX apps.  
******************************************************************************/

@interface AppWindow : NSWindow < NSWindowDelegate>
{
    PVRShellInit* pvrshell;
    unsigned int mouseButtonState;
}

@property (assign) PVRShellInit * pvrshell;
@property (assign) unsigned int mouseButtonState;
@end

/*! @} */

#endif //_APPCONTROLLER_H_

