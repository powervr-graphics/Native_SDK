/*!****************************************************************************
\file         PVRShell\EntryPoint\UIApplicationMain\main.m
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Entry point for UIKit(iOS) systems.
******************************************************************************/
#import <UIKit/UIKit.h>

int main(int argc, char *argv[])
{
	//NSAutoreleasePool* pool = [NSAutoreleasePool new];
	@autoreleasepool {
        UIApplicationMain(argc, argv, nil, @"AppController");
    }
	
	
//	[pool release];
	
	return 0;
}

