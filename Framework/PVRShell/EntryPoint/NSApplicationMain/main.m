/*!****************************************************************************
\file         PVRShell/EntryPoint/NSApplicationMain/main.m
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Entry Point for the AppKit implementation of the Shell
******************************************************************************/
#import <AppKit/NSApplication.h>

/// <summary>Application entry point for the OSX Appkit. Forwards to NSApplicationMain</summary>
/// <param name="argc">Number of command line arguments</param>
/// <param name="argv">Array of command line arguments</param>
/// <returns>0 on no error, otherwise 1</returns>
int main(int argc, char *argv[])
{
    return NSApplicationMain(argc, (const char **)argv);
}

