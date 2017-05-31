/*!
\brief The StateMachine controlling the PowerVR Shell.
\file PVRShell/StateMachine.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRShell/OS/ShellOS.h"
namespace pvr {
namespace platform {
class Shell;

/// <summary>The StateMachine controlling the PowerVR Shell. Provides the application main loop and callbacks.
/// </summary>
class StateMachine : public ShellOS
{
public:
	enum State
	{
		StateNotInitialized,
		StateInitApplication,
		StateInitWindow,
		StateInitAPI,
		StateInitView,
		StateRenderScene,
		StateReleaseView,
		StateReleaseAPI,
		StateReleaseWindow,
		StateQuitApplication,
		StatePreExit,
		StateExit
	};

public:
	/// <summary>Constructor. Called by the application's entry point (main).</summary>
	StateMachine(OSApplication instance, platform::CommandLineParser& commandLine, OSDATA osdata);

	/// <summary>Called by the application's entry point (main).</summary>
	Result init();

	/// <summary>Called by the application's entry point (main).</summary>
	Result execute();

	/// <summary>Called internally by the state machine.</summary>
	Result executeOnce();

	/// <summary>Called internally by the state machine.</summary>
	Result executeOnce(const State state);

	/// <summary>Called internally by the state machine.</summary>
	Result executeUpTo(const State state);

	/// <summary>Get the current state of the StateMachine.</summary>
	/// <returns>The current state of the StateMachine.</returns>
	State getState() const { return _currentState; }

	/// <summary>Check if the StateMachine is paused.</summary>
	/// <returns>True if the StateMachine is paused.</returns>
	bool isPaused() const { return _pause; }

	/// <summary>Pauses the state machine.</summary>
	void pause() { _pause = true; }

	/// <summary>Resumes (exits pause state for) the state machine.</summary>
	void resume() { _pause = false; }

	State getCurrentState()const { return _currentState; }
private:
	void applyCommandLine();
	void readApiFromCommandLine();

	State _currentState;
	bool _pause;
};
}
}