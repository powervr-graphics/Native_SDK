/*!*********************************************************************************************************************
\file         PVRShell\EntryPoint\NSApplicationMain\AppController.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief     	  Class necessary for the entry point of the AppKit based implementation of PVRShell.
***********************************************************************************************************************/
#ifndef _APPCONTROLLER_H_
#define _APPCONTROLLER_H_

#include "PVRShell/StateMachine.h"
#include "PVRShell/CommandLine.h"
#import <AppKit/NSApplication.h>
#import <Foundation/NSTimer.h>

/*!***************************************************************************************************************
\brief iOS entry point implementation
*****************************************************************************************************************/
@interface AppController : NSObject <NSApplicationDelegate>
{
	NSTimer*		 mainLoopTimer;	//!< timer for the main loop
	pvr::platform::StateMachine* stateMachine; //!< The State Machine powering the pvr::Shell
	pvr::platform::CommandLineParser commandLine;    //!< Command line options passed on app launch
}

- (void) terminateApp;

@end

#endif //_APPCONTROLLER_H_
