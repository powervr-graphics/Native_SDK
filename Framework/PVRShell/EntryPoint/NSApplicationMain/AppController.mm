/*!*********************************************************************************************************************
\file         PVRShell\EntryPoint\NSApplicationMain\AppController.mm
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Implementation for the AppKit version of AppController
***********************************************************************************************************************/
#import "AppController.h"
#import <AppKit/NSAlert.h>
#import <Foundation/NSString.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSArray.h>
//CONSTANTS:
const int kFPS = 60.0;

// CLASS IMPLEMENTATION
@implementation AppController

- (void) mainLoop
{
	if(stateMachine->executeOnce() != pvr::Result::Success)
	{		
        [self terminateApp];
	}
}

- (void) applicationDidFinishLaunching:(NSNotification *)notification
{
	// Parse the command-line
    NSMutableString *cl = [[NSMutableString alloc] init];
    NSArray *args = [[NSProcessInfo processInfo] arguments];
    
    for(NSUInteger i = 1;i < [args count]; ++i)
    {
        [cl appendString:[args objectAtIndex:i]];
        [cl appendString:@" "];
    }
	
	commandLine.set([cl UTF8String]);
	//[cl release];
	
	stateMachine = new pvr::system::StateMachine((__bridge pvr::system::OSApplication)self, commandLine, NULL);

	if(!stateMachine)
	{
		NSLog(@"Failed to allocate stateMachine.\n");
        [self terminateApp];
	}
	
	if(stateMachine->init() != pvr::Result::Success)
	{
		NSLog(@"Failed to initialise stateMachine.\n");
		[self terminateApp];
	}
	
	mainLoopTimer = [NSTimer scheduledTimerWithTimeInterval:(1.0 / kFPS) target:self selector:@selector(mainLoop) userInfo:nil repeats:YES];	
}

- (void) applicationWillTerminate:(NSNotification *)notification
{
    [mainLoopTimer invalidate];
    mainLoopTimer = nil;
    
    if( stateMachine->getCurrentState() == pvr::system::StateMachine::StateRenderScene)
    {
        stateMachine->executeOnce(pvr::system::StateMachine::StateReleaseView);
        //[self terminateApp];
    }
    stateMachine->execute();
    delete stateMachine;
    stateMachine = NULL;
}

- (void) terminateApp
{
    [NSApp terminate:nil];
}

@end

