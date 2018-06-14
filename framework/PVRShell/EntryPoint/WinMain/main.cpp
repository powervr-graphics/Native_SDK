/*!
\brief Application entry point for Microsoft Windows systems.
\file PVRShell/EntryPoint/WinMain/main.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "PVRShell/OS/ShellOS.h"
#include "PVRShell/StateMachine.h"
#include "PVRShell/CommandLine.h"
#include "PVRShell/OS/Windows/WindowsOSData.h"
#include <windows.h>
#include <io.h>

/// <summary>The entry point for Microsoft Windows (Windowed application). See the Win32 spec for
/// a detailed explanation of the function parameters.</summary>
/// <param name="hInstance">The Win32 Application Instance</param>
/// <param name="HINSTANCE">Unused</param>
/// <param name="lpCmdLine">The command line for the application, excluding program name</param>
/// <param name="nCmdShow">Controls how the window is to be shown. See the Win32 spec.</param>
/// <returns>0 on no error, otherwise 1</returns>
int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	int retval;
	{
#if defined(_WIN32) && defined(_DEBUG)
		// Enable memory-leak reports
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_EVERY_128_DF);
#endif
		pvr::platform::WindowsOSData data;
		data.cmdShow = nCmdShow;

		pvr::platform::CommandLineParser commandLine;
		commandLine.set(lpCmdLine);

		pvr::platform::StateMachine stateMachine(hInstance, commandLine, &data);

		stateMachine.init();

		// Enter our loop
		retval = (stateMachine.execute() == pvr::Result::Success) ? 0 : 1;
	}
	return retval;
}