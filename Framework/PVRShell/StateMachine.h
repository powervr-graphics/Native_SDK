#pragma once
/*!*********************************************************************************************************************
\file         PVRShell\StateMachine.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief     	  The StateMachine controlling the PowerVR Shell.
***********************************************************************************************************************/
#include "PVRShell/ShellData.h"
#include "PVRShell/OS/ShellOS.h"
namespace pvr {
namespace platform {
class Shell;

/*!****************************************************************************************************************
\brief The StateMachine controlling the PowerVR Shell. Provides the application main loop and callbacks.
*******************************************************************************************************************/
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
	/*!****************************************************************************************************************
	\brief Constructor. Called by the application's entry point (main).
	*******************************************************************************************************************/
	StateMachine(OSApplication instance, platform::CommandLineParser& commandLine, OSDATA osdata);

	/*!****************************************************************************************************************
	\brief Called by the application's entry point (main).
	*******************************************************************************************************************/
	Result init();

	/*!****************************************************************************************************************
	\brief Called by the application's entry point (main).
	*******************************************************************************************************************/
	Result execute();

	/*!****************************************************************************************************************
	\brief Called internally by the state machine.
	*******************************************************************************************************************/
	Result executeOnce();

	/*!****************************************************************************************************************
	\brief Called internally by the state machine.
	*******************************************************************************************************************/
	Result executeOnce(const State state);

	/*!****************************************************************************************************************
	\brief Called internally by the state machine.
	*******************************************************************************************************************/
	Result executeUpTo(const State state);

	/*!****************************************************************************************************************
	\return The current state of the StateMachine.
	*******************************************************************************************************************/
	State getState() const { return m_currentState; }

	/*!****************************************************************************************************************
	\return True if the StateMachine is paused.
	*******************************************************************************************************************/
	bool isPaused() const { return m_pause; }

	/*!****************************************************************************************************************
	\brief Pauses the state machine.
	*******************************************************************************************************************/
	void pause() { m_pause = true; }

	/*!****************************************************************************************************************
	\brief Resumes (exits pause state for) the state machine.
	*******************************************************************************************************************/
	void resume() { m_pause = false; }

	State getCurrentState()const { return m_currentState; }
private:
	void applyCommandLine();

	State m_currentState;
	bool m_pause;
};
}
}