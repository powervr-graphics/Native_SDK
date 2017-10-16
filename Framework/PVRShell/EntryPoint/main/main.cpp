/*!
\brief Generic entry point, normally used for Linux based systems.
\file PVRShell\EntryPoint/main/main.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/OS/ShellOS.h"
#include "PVRShell/StateMachine.h"
#include "PVRShell/CommandLine.h"

int main(int argc, char** argv)
{
	pvr::platform::CommandLineParser commandLine;
	commandLine.set((argc - 1), &argv[1]);

	pvr::platform::StateMachine stateMachine(NULL, commandLine, NULL);

	stateMachine.init();

	// Main loop of the application.
	return (stateMachine.execute() == pvr::Result::Success) ? 0 : 1;
}