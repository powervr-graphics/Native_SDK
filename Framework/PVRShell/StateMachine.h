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
	/// <summary>Possible States of the StateMachine</summary<
	enum State
	{
		StateNotInitialized, //!<First: The State Machine is not yet initialized.
		StateInitApplication, //!<Second: The State Machine is initialized and is initializing the application (calls initApplication)
		StateInitWindow, //!<Third: Creating the Window
		StateInitView, //!<Fourth: Signals the application that the window is created and should initialize itself (calls initView)
		StateRenderScene, //!<Fifth: The Main Loop - repeatedly calls renderScene until signalled to stop
		StateReleaseView, //!<Sixth: Signals the application that the application window is about to be torn down/lost (calls releaseView)
		StateReleaseWindow, //!<Seventh: Tears dow the window
		StateQuitApplication, //!<Eighth: Signals the applications that the program is exiting
		StatePreExit, //!<Ninth: Is exiting
		StateExit //!< Exits
	};

public:
	/// <summary>Constructor. Called by the application's entry point (main).</summary>
	/// <param name="instance">Platform-specific object containing pointer/s to the application instance.</param>
	/// <param name="commandLine">The command line arguments passed by the user.</param>
	/// <param name="osdata">Platform specific data passed in by the system</param>
	StateMachine(OSApplication instance, platform::CommandLineParser& commandLine, OSDATA osdata);

	/// <summary>Called by the application's entry point (main).</summary>
	/// <returns>pvr::Result::Success if successful, otherwise error code.</returns>
	Result init();

	/// <summary>Called by the application's entry point (main).</summary>
	/// <returns>pvr::Result::Success if successful, otherwise error code.</returns>
	Result execute();

	/// <summary>Called internally by the state machine.</summary>
	/// <returns>pvr::Result::Success if successful, otherwise error code.</returns>
	Result executeOnce();

	/// <summary>Called internally by the state machine. Executes a specific state code path once.</summary>
	/// <param name="state">The state to execute</param>
	/// <returns>pvr::Result::Success if successful, otherwise error code.</returns>
	Result executeOnce(const State state);

	/// <summary>Called internally by the state machine. Executes all code paths between current state
	/// and the state requested (naturally reaching that state). For example, executeUpTo(QuitApplication)
	/// when current state is RenderFrame, will execute ReleaseView, ReleaseWindow, QuitApplication</summary>
	/// <param name="state">The state to execute</param>
	/// <returns>pvr::Result::Success if successful, otherwise error code.</returns>
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

	/// <summary>Gets the current state of the state machine.</summary>
	/// <returns>The current state of the state machine</returns>
	State getCurrentState()const { return _currentState; }
private:
	void applyCommandLine();
	void readApiFromCommandLine();

	State _currentState;
	bool _pause;
};
}
}
