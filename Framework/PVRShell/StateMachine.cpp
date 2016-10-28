/*!*********************************************************************************************************************
\file         PVRShell\StateMachine.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief     	  Implementation of the StateMachine class powering the pvr::Shell.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRShell/StateMachine.h"
#include "PVRShell/Shell.h"
#include "PVRCore/IPlatformContext.h"
#include "PVRCore/FileStream.h"
#include "PVRCore/Log.h"
#include "PVRCore/Time_.h"
#include <cstdlib>
#include <cmath>

#if defined(_WIN32)
#define strcasecmp(a,b) _stricmp(a,b)
#endif

using std::string;

namespace pvr {
namespace platform {

StateMachine::StateMachine(OSApplication instance, platform::CommandLineParser& commandLine, OSDATA osdata) : ShellOS(instance, osdata), m_currentState(StateNotInitialized),
	m_pause(false)
{
	m_shellData.os = this;
	m_shellData.commandLine = &commandLine;
}

Result StateMachine::init()
{
	Result result = ShellOS::init(m_shellData.attributes);

	if (result == Result::Success)
	{
		// Check for the existence of PVRShellCL.txt and load from it if it exists
		std::string filepath;

		for (size_t i = 0; i < ShellOS::getReadPaths().size(); ++i)
		{
			filepath = ShellOS::getReadPaths()[i] + PVRSHELL_COMMANDLINE_TXT_FILE;
			FileStream file(filepath.c_str(), "r");

			if (file.open() == true)
			{
				m_shellData.commandLine->prefix(&file);
				std::string info = "Command-line options have been loaded from file " + filepath;
				Log(Log.Information, info.c_str());
				break;
			}
		}

		// Build our windows title

		m_shellData.attributes.windowTitle = getApplicationName() + " - Build " +
		                                     string(Shell::getSDKVersion());

		// setup our state
		m_currentState = StateInitApplication;
	}


	return result;
}

void StateMachine::applyCommandLine()
{
	const char* arg, *val;
	const platform::CommandLineParser::ParsedCommandLine& options = m_shellData.commandLine->getParsedCommandLine();

#define WARNING_UNSUPPORTED_OPTION(x) { Log(Log.Warning, "PVRShell recognised command-line option '" x "' is unsupported in this application and has been ignored."); }

	for (unsigned int i = 0; i < options.getOptionsList().size(); ++i)
	{
		arg = options.getOptionsList()[i].arg;
		val = options.getOptionsList()[i].val;

		if (!arg) {continue;	}

		if (val)
		{
			if (strcasecmp(arg, "-width") == 0)
			{
				m_shell->setDimensions(atoi(val), m_shell->getHeight());
			}
			else if (strcasecmp(arg, "-height") == 0)
			{
				m_shell->setDimensions(m_shell->getWidth(), atoi(val));
			}
			else if (strcasecmp(arg, "-aasamples") == 0)
			{
				m_shell->setAASamples(atoi(val));
			}
			else if (strcasecmp(arg, "-fullscreen") == 0)
			{
				m_shell->setFullscreen(atoi(val) != 0);
			}
			else if (strcasecmp(arg, "-quitafterframe") == 0 || strcasecmp(arg, "-qaf") == 0)
			{
				m_shell->setQuitAfterFrame(atoi(val));
			}
			else if (strcasecmp(arg, "-quitaftertime") == 0 || strcasecmp(arg, "-qat") == 0)
			{
				m_shell->setQuitAfterTime((float32)atof(val));
			}
			else if (strcasecmp(arg, "-posx") == 0)
			{
				if (m_shell->setPosition(atoi(val), m_shell->getPositionY()) == Result::UnsupportedRequest)
				{
					WARNING_UNSUPPORTED_OPTION("posx")
				}
			}
			else if (strcasecmp(arg, "-posy") == 0)
			{
				if (m_shell->setPosition(m_shell->getPositionX(), atoi(val)) == Result::UnsupportedRequest)
				{
					WARNING_UNSUPPORTED_OPTION("posy")
				}
			}
			else if (strcasecmp(arg, "-swaplength") == 0 || strcasecmp(arg, "-preferredswaplength") == 0)
			{
				m_shell->setPreferredSwapChainLength(atoi(val));
			}
			else if (strcasecmp(arg, "-vsync") == 0)
			{
				if (!strcasecmp(val, "on"))
				{
					m_shell->setVsyncMode(VsyncMode::On);
					Log("On");
				}
				else if (!strcasecmp(val, "off"))
				{
					m_shell->setVsyncMode(VsyncMode::Off);
					Log("Off");
				}
				else if (!strcasecmp(val, "relaxed"))
				{
					m_shell->setVsyncMode(VsyncMode::Relaxed);
					Log("Relaxed");
				}
				else if (!strcasecmp(val, "mailbox"))
				{
					m_shell->setVsyncMode(VsyncMode::Mailbox);
					Log("Mailbox");
				}
				else if (!strcasecmp(val, "half"))
				{
					m_shell->setVsyncMode(VsyncMode::Half);
					Log("Half");
				}
				else
				{
					Log("Trying number");
					char* converted;
					int value = strtol(val, &converted, 0);
					if (!*converted)
					{
						switch (value)
						{
						case 0: m_shell->setVsyncMode(VsyncMode::Off); break;
						case 1: m_shell->setVsyncMode(VsyncMode::On); break;
						case 2: m_shell->setVsyncMode(VsyncMode::Half); break;
						case -1: m_shell->setVsyncMode(VsyncMode::Relaxed); break;
						case -2: m_shell->setVsyncMode(VsyncMode::Mailbox); break;
						default: break;
						}
					}
				}
				Log("%d", m_shell->getVsyncMode());
			}
			else if (strcasecmp(arg, "-loglevel") == 0)
			{
				if (!strcasecmp(val, "critical"))
				{
					Log.setVerbosity(Log.Critical);
				}
				else if (!strcasecmp(val, "error"))
				{
					Log.setVerbosity(Log.Error);
				}
				else if (!strcasecmp(val, "warning"))
				{
					Log.setVerbosity(Log.Warning);
				}
				else if (!strcasecmp(val, "information"))
				{
					Log.setVerbosity(Log.Information);
				}
				else if (!strcasecmp(val, "info"))
				{
					Log.setVerbosity(Log.Information);
				}
				else if (!strcasecmp(val, "verbose"))
				{
					Log.setVerbosity(Log.Verbose);
				}
				else if (!strcasecmp(val, "debug"))
				{
					Log.setVerbosity(Log.Debug);
				}
				else
				{
					Log(Log.Warning, "Unrecognized threshold '%s' for '-loglevel' command line parameter. Accepted values: [critical, error, warning, information(default for release build), debug(default for debug build), verbose");
				}
			}
			else if (strcasecmp(arg, "-colorbpp") == 0 || strcasecmp(arg, "-colourbpp") == 0
			         || strcasecmp(arg, "-cbpp") == 0)
			{
				switch (atoi(val))
				{
				case 16:
					m_shell->setColorBitsPerPixel(5, 6, 5, 0);
					break;
				case 24:
					m_shell->setColorBitsPerPixel(8, 8, 8, 0);
					break;
				case 32:
					m_shell->setColorBitsPerPixel(8, 8, 8, 8);
					break;
				default:
					Log(Log.Warning,
					    "PVRShell recognised command-line option '%hs' set to unsupported value %hs. Supported values are (16, 24 and 32).",
					    arg, val);
					break;
				}
			}
			else if (strcasecmp(arg, "-depthbpp") == 0 || strcasecmp(arg, "-dbpp") == 0)
			{
				m_shell->setDepthBitsPerPixel(atoi(val));
			}
			else if (strcasecmp(arg, "-stencilbpp") == 0 || strcasecmp(arg, "-dbpp") == 0)
			{
				m_shell->setStencilBitsPerPixel(atoi(val));
			}
			/*else if(strcasecmp(arg, "-rotatekeys") == 0)
			{
			m_shell->PVRShellset(PVRShell::prefRotateKeys, atoi(val));
			}*/
			else if (strcasecmp(arg, "-c") == 0)
			{
				const char* pDash = strchr(val, '-');
				uint32 start(atoi(val)), stop(pDash ? atoi(pDash + 1) : start);
				m_shell->setCaptureFrames(start, stop);
			}
			else if (strcasecmp(arg, "-screenshotscale") == 0)
			{
				m_shell->setCaptureFrameScale(atoi(val));
			}
			else if (strcasecmp(arg, "-priority") == 0)
			{
				m_shell->setContextPriority(atoi(val));
			}
			else if (strcasecmp(arg, "-config") == 0)
			{
				m_shell->setDesiredConfig(atoi(val));
			}
			/*	else if(strcasecmp(arg, "-display") == 0)
			{
			// TODO:
			m_shell->PVRShellset(PVRShell::prefNativeDisplay, atoi(val));
			}*/
			else if (strcasecmp(arg, "-forceframetime") == 0 || strcasecmp(arg, "-fft") == 0)
			{
				m_shell->setForceFrameTime(true);
				int value = atoi(val);
				m_shell->setFakeFrameTime(std::max(1, value));
			}
		}
		else
		{
			if (strcasecmp(arg, "-version") == 0)
			{
				Log(Log.Information, "Version: '%hs'", m_shell->getSDKVersion());
			}
			else if (strcasecmp(arg, "-fps") == 0)
			{
				m_shell->setShowFPS(true);
			}
			else if (strcasecmp(arg, "-info") == 0)
			{
				m_shellData.outputInfo = true;
			}
			else if (strcasecmp(arg, "-forceframetime") == 0 || strcasecmp(arg, "-fft") == 0)
			{
				m_shell->setForceFrameTime(true);
			}
		}
	}

#undef WARNING_UNSUPPORTED_OPTION
}

Result StateMachine::execute()
{
	Result result;

	for (;;)
	{
		result = executeOnce();
		if (result != Result::Success)
		{
			if (result == Result::ExitRenderFrame) { result = Result::Success; }
			while (m_currentState != StateExit) { executeOnce(); }// Loop to tidy up
		}
		if (m_currentState == StateExit) { return result; }
	}
}

Result StateMachine::executeUpTo(const State state)
{
	Result result = m_currentState > state ? Result::InvalidArgument : Result::Success;
	while (result == Result::Success && m_currentState < state)
	{
		if (m_currentState == StateRenderScene && state > StateRenderScene)
		{
			m_currentState = StateReleaseView;
		}
		result = executeOnce();
	}
	return result;
}

Result StateMachine::executeOnce(const State state)
{
	m_currentState = state;
	return executeOnce();
}

Result StateMachine::executeOnce()
{
	// don't handle the events while paused
	Result result(Result::Success);
	if (m_pause) {  return result;   }

	// Handle our state
	switch (m_currentState)
	{
	case StateNotInitialized:
		return Result::NotInitialized;
	case StateInitApplication:
		m_shell = newDemo();
		m_shellData.platformContext = pvr::createNativePlatformContext(*m_shell);
		result = Result::UnableToOpen;
		if (m_shellData.platformContext.get())
		{
			result = m_shell->init(&m_shellData);
		}

		if (result == Result::Success)
		{
			result = m_shell->shellInitApplication();

			if (result == Result::Success)
			{
				m_currentState = StateInitWindow;
			}
			else
			{
				m_shell.reset();

				m_currentState = StatePreExit;

				string error = string("InitApplication() failed with pvr error '") + Log.getResultCodeString(result) + string("'\n");
				Log(Log.Error, error.c_str());
			}
		}
		else
		{
			m_shell.reset();

			m_currentState = StatePreExit; // If we have reached this point, then m_shell has already been initialized.
			string error = string("State Machine initialisation failed with error '") + Log.getResultCodeString(result) + string("'\n");
			Log(Log.Error, error.c_str());
		}
		break;
	case StateInitWindow:
	{
		applyCommandLine();
		// Initialize our window. On some platforms this will be a dummy function
		result = ShellOS::initializeWindow(m_shellData.attributes);
		if (result == Result::Success) {	m_currentState = StateInitAPI;}
		else {	m_currentState = StateQuitApplication;	}
	}
	break;
	case StateInitAPI:
		if (!m_shellData.platformContext.get())
		{
			m_currentState = StateReleaseWindow;
			return Result::NotInitialized;
		}
		else
		{
			result = m_shellData.platformContext->init();

			if (result == Result::Success)
			{
				m_shellData.platformContext->makeCurrent();
				m_currentState = StateInitView;
			}
			else
			{
				if (m_shell->getApiTypeRequired() != Api::Unspecified)
				{
					m_shell->setExitMessage("Requested Graphics context type %s was unsupported on this device.", apiName(m_shell->getApiTypeRequired()));
				}
				else
				{
					m_shell->setExitMessage("Unable to create context. Unknown error. Examine log for details.");
				}
				m_currentState = StateReleaseAPI;  // Though we failed to init the API we call ReleaseAPI to delete m_pplatformContext.
			}
		}
		break;
	case StateInitView:
		result = m_shell->shellInitView();

		if (result == Result::Success)
		{
			m_currentState = StateRenderScene;
			m_shellData.startTime = m_shellData.timer.getCurrentTimeMilliSecs();
		}
		else
		{
			m_currentState = StateReleaseView;

			string error = string("InitView() failed with pvr error '") + Log.getResultCodeString(result) + string("'\n");
			Log(Log.Error, error.c_str());
		}

		if (m_shellData.outputInfo)
		{
			m_shell->showOutputInfo();
		}
		break;
	case StateRenderScene:
	{
		// Process any OS events
		ShellOS::handleOSEvents();

		// Call RenderScene
		result = m_shell->shellRenderFrame();

		if (m_shellData.weAreDone && result == Result::Success)
		{
			result = Result::ExitRenderFrame;
		}

		if (result == Result::Success)
		{
			// Read from backbuffer before swapping - take screenshot if frame capture is enabled
			if ((static_cast<int32>(m_shellData.frameNo) >= m_shellData.captureFrameStart
			     && static_cast<int32>(m_shellData.frameNo) <= m_shellData.captureFrameStop))
			{
				m_shell->takeScreenshot();
			}

			// Swap buffers
			result = (m_shellData.presentBackBuffer && m_shellData.platformContext->presentBackbuffer()) ? Result::Success : Result::UnknownError;

			if (result != Result::Success)
			{
				m_currentState = StateReleaseView;
			}
		}
		else
		{
			if (result != Result::Success && result != Result::ExitRenderFrame)
			{
				string error = string("renderFrame() failed with pvr error '") + Log.getResultCodeString(result) + string("'\n");
				Log(Log.Error, error.c_str());
			}
			m_currentState = StateReleaseView;
		}

		// Calculate our FPS
		{
			static uint64 prevTime(m_shellData.timer.getCurrentTimeMilliSecs()), FPSFrameCount(0);
			uint64 time(m_shellData.timer.getCurrentTimeMilliSecs()), delta(time - prevTime);

			++FPSFrameCount;

			if (delta >= 1000)
			{
				m_shellData.FPS = 1000.0f * FPSFrameCount / (float)delta;

				FPSFrameCount = 0;
				prevTime = time;

				if (m_shellData.showFPS)
				{
					Log(Log.Information, "Frame %i, FPS %.2f", m_shellData.frameNo, m_shellData.FPS);
				}
			}
		}

		// Have we reached the point where we need to die?
		if ((m_shellData.dieAfterFrame >= 0 && m_shellData.frameNo >= static_cast<uint32>(m_shellData.dieAfterFrame))
		    || (m_shellData.dieAfterTime >= 0 && ((m_shellData.timer.getCurrentTimeMilliSecs() - m_shellData.startTime) * 0.001f) > m_shellData.dieAfterTime)
		    || m_shellData.forceReleaseInitCycle)
		{
			if (m_shellData.forceReleaseInitCycle)
			{
				Log(Log.Information, "Reinit requested. Going through Reinitialization cycle. ReleaseView will be called next, and then InitView.");
			}
			m_currentState = StateReleaseView;
			break;
		}

		// Increment our frame number
		++m_shellData.frameNo;
	}
	break;
	case StateReleaseView:
		Log(Log.Debug, "ReleaseView");
		result = m_shell->shellReleaseView();

		if (result != Result::Success)
		{
			string error = string("ReleaseView() failed with pvr error '") + Log.getResultCodeString(result) + string("'\n");
			Log(Log.Error, error.c_str());
		}

		m_currentState = StateReleaseAPI;
		break;
	case StateReleaseAPI:
		Log(Log.Debug, "ReleaseApi");
		if (m_shellData.graphicsContextStore.isValid())
		{
			m_shellData.graphicsContextStore->release();
			m_shellData.graphicsContextStore.reset();
		}
		m_shellData.graphicsContext.reset();
		m_currentState = StateReleaseWindow;
		break;
	case StateReleaseWindow:
		Log(Log.Debug, "ReleaseWindow");
		if (m_shellData.platformContext.get())
		{
			m_shellData.platformContext->release();
		}
		ShellOS::releaseWindow();

		if (!m_shellData.weAreDone && m_shellData.forceReleaseInitCycle)
		{
			m_shellData.forceReleaseInitCycle = false;
			m_currentState = StateInitWindow;
		}
		else
		{
			m_currentState = StateQuitApplication;
		}
		break;
	case StateQuitApplication:
		Log(Log.Debug, "QuitApplication");
		result = m_shell->shellQuitApplication();

		if (result != Result::Success)
		{
			string error = string("QuitApplication() failed with pvr error '") + Log.getResultCodeString(result) + string("'\n");
			Log(Log.Error, error.c_str());
		}

		m_shell.reset();

		m_currentState = StatePreExit;
	case StatePreExit:
		Log(Log.Debug, "StateExit");
		if (!m_shellData.exitMessage.empty())
		{
			ShellOS::popUpMessage(ShellOS::getApplicationName().c_str(), m_shellData.exitMessage.c_str());
		}
		m_shellData.platformContext.reset();
		m_currentState = StateExit;
	case StateExit:
		break;
	}
	return result;
}
}
}
//!\endcond