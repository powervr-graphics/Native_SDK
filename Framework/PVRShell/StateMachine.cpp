/*!
\brief Implementation of the StateMachine class powering the pvr::Shell.
\file PVRShell/StateMachine.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRShell/StateMachine.h"
#include "PVRShell/Shell.h"
#include "PVRCore/Interfaces/IPlatformContext.h"
#include "PVRCore/IO/FileStream.h"
#include "PVRCore/Log.h"
#include "PVRCore/Base/Time_.h"
#include <cstdlib>
#include <cmath>

#if defined(_WIN32)
#define strcasecmp(a,b) _stricmp(a,b)
#endif

using std::string;

namespace pvr {
namespace platform {

StateMachine::StateMachine(OSApplication instance, platform::CommandLineParser& commandLine, OSDATA osdata) : ShellOS(instance, osdata), _currentState(StateNotInitialized),
	_pause(false)
{
	_shellData.os = this;
	_shellData.commandLine = &commandLine;
	Log.initializeMessenger();
}

Result StateMachine::init()
{
	Result result = ShellOS::init(_shellData.attributes);

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
				_shellData.commandLine->prefix(&file);
				std::string info = "Command-line options have been loaded from file " + filepath;
				Log(Log.Information, info.c_str());
				break;
			}
		}

		// Build our windows title

		_shellData.attributes.windowTitle = getApplicationName() + " - Build " +
		                                    string(Shell::getSDKVersion());

		// setup our state
		_currentState = StateInitApplication;
	}


	return result;
}

void StateMachine::readApiFromCommandLine()
{
	const char* arg, *val;
	const platform::CommandLineParser::ParsedCommandLine& options = _shellData.commandLine->getParsedCommandLine();

	for (unsigned int i = 0; i < options.getOptionsList().size(); ++i)
	{
		arg = options.getOptionsList()[i].arg;
		val = options.getOptionsList()[i].val;

		if (!arg) { continue; }

		if (val && strcasecmp(arg, "-apitype") == 0)
		{
			if (!strcasecmp(val, "vulkan") || !strcasecmp(val, "vk"))
			{
				Log(Log.Information, "Base API type setting set to Vulkan. This will only take effect if PVRApi is linked dynamically.");
				_shellData.baseContextType = BaseApi::Vulkan;
			}
			else if (!strcasecmp(val, "ogles") || !strcasecmp(val, "opengles") || !strcasecmp(val, "gles") || !strcasecmp(val, "es") || !strcasecmp(val, "gles"))
			{
				Log(Log.Information, "Base API type setting set to OpenGL ES. This will only take effect if PVRApi is linked dynamically.");
				_shellData.baseContextType = BaseApi::OpenGLES;
			}
			else
			{
				Log(Log.Error, "Base API type setting '%s' set NOT RECOGNIZED. Default to unspecified.", val);
			}
		}
		else if (strcasecmp(arg, "-apiversion") == 0)
		{
			if (!strcasecmp(val, "vulkan"))
			{
				Log(Log.Debug, "Base API type setting set to Vulkan. This will only take effect if PVRApi is linked dynamically.");
				_shellData.baseContextType = BaseApi::Vulkan;
				_shellData.minContextType = Api::Vulkan;
				_shellData.contextType = Api::Vulkan;
			}
			else if (!strcasecmp(val, "ogles31") || !strcasecmp(val, "gles31") || !strcasecmp(val, "gl31") || !strcasecmp(val, "es31"))
			{
				Log(Log.Debug, "Base API type setting set to OpenGL ES. This will only take effect if PVRApi is linked dynamically.");
				Log(Log.Information, "Api version forced to OpenGL ES 3.1");
				_shellData.baseContextType = BaseApi::OpenGLES;
				_shellData.minContextType = Api::OpenGLES31;
				_shellData.contextType = Api::OpenGLES31;
			}
			else if (!strcasecmp(val, "ogles3") || !strcasecmp(val, "gles3") || !strcasecmp(val, "gl3") || !strcasecmp(val, "es3"))
			{
				Log(Log.Debug, "Base API type setting set to OpenGL ES. This will only take effect if PVRApi is linked dynamically.");
				Log(Log.Information, "Api version forced to OpenGL ES 3.0");
				_shellData.baseContextType = BaseApi::OpenGLES;
				_shellData.minContextType = Api::OpenGLES3;
				_shellData.contextType = Api::OpenGLES3;
			}
			else if (!strcasecmp(val, "ogles2") || !strcasecmp(val, "gles2") || !strcasecmp(val, "gl2") || !strcasecmp(val, "es2"))
			{
				Log(Log.Debug, "Base API type setting set to OpenGL ES. This will only take effect if PVRApi is linked dynamically.");
				Log(Log.Information, "Api version forced to OpenGL ES 2.0");
				_shellData.baseContextType = BaseApi::OpenGLES;
				_shellData.minContextType = Api::OpenGLES2;
				_shellData.contextType = Api::OpenGLES2;

			}
			else
			{
				Log(Log.Error, "Unrecognized command line value '%s' for command line argument '-apiversion'", val);
			}
		}
		else if (strcasecmp(arg, "-minapiversion") == 0 || strcasecmp(arg, "-minapi") == 0)
		{
			if (!strcasecmp(val, "vulkan"))
			{
				Log(Log.Debug, "Base API type setting set to Vulkan. This will only take effect if PVRApi is linked dynamically.");
				_shellData.baseContextType = BaseApi::Vulkan;
				_shellData.minContextType = Api::Vulkan;
			}
			else if (!strcasecmp(val, "ogles31") || !strcasecmp(val, "gles31") || !strcasecmp(val, "gl31") || !strcasecmp(val, "es31"))
			{
				Log(Log.Debug, "Base API type setting set to OpenGL ES. This will only take effect if PVRApi is linked dynamically.");
				Log(Log.Information, "Minimum api version set to OpenGL ES 3.1");
				_shellData.baseContextType = BaseApi::OpenGLES;
				_shellData.minContextType = Api::OpenGLES31;
			}
			else if (!strcasecmp(val, "ogles3") || !strcasecmp(val, "gles3") || !strcasecmp(val, "gl3") || !strcasecmp(val, "es3"))
			{
				Log(Log.Debug, "Base API type setting set to OpenGL ES. This will only take effect if PVRApi is linked dynamically.");
				Log(Log.Information, "Minimum api version set to OpenGL ES 3.0");
				_shellData.baseContextType = BaseApi::OpenGLES;
				_shellData.minContextType = Api::OpenGLES3;
			}
			else if (!strcasecmp(val, "ogles2") || !strcasecmp(val, "gles2") || !strcasecmp(val, "gl2") || !strcasecmp(val, "es2"))
			{
				Log(Log.Debug, "Base API type setting set to OpenGL ES. This will only take effect if PVRApi is linked dynamically.");
				Log(Log.Information, "Minimum api version set to OpenGL ES 2.0");
				_shellData.baseContextType = BaseApi::OpenGLES;
				_shellData.minContextType = Api::OpenGLES2;
			}
			else
			{
				Log(Log.Error, "Unrecognized command line value '%s' for command line argument '-minapiversion'", val);
			}
		}
	}
}

void StateMachine::applyCommandLine()
{
	const char* arg, *val;
	const platform::CommandLineParser::ParsedCommandLine& options = _shellData.commandLine->getParsedCommandLine();

#define WARNING_UNSUPPORTED_OPTION(x) { Log(Log.Warning, "PVRShell recognised command-line option '" x "' is unsupported in this application and has been ignored."); }

	for (unsigned int i = 0; i < options.getOptionsList().size(); ++i)
	{
		arg = options.getOptionsList()[i].arg;
		val = options.getOptionsList()[i].val;

		if (!arg) {continue;  }

		if (val)
		{
			if (strcasecmp(arg, "-width") == 0)
			{
				_shell->setDimensions(atoi(val), _shell->getHeight());
			}
			else if (strcasecmp(arg, "-height") == 0)
			{
				_shell->setDimensions(_shell->getWidth(), atoi(val));
			}
			else if (strcasecmp(arg, "-aasamples") == 0)
			{
				_shell->setAASamples(atoi(val));
			}
			else if (strcasecmp(arg, "-fullscreen") == 0)
			{
				_shell->setFullscreen(atoi(val) != 0);
			}
			else if (strcasecmp(arg, "-quitafterframe") == 0 || strcasecmp(arg, "-qaf") == 0)
			{
				_shell->setQuitAfterFrame(atoi(val));
			}
			else if (strcasecmp(arg, "-quitaftertime") == 0 || strcasecmp(arg, "-qat") == 0)
			{
				_shell->setQuitAfterTime((float32)atof(val));
			}
			else if (strcasecmp(arg, "-posx") == 0)
			{
				if (_shell->setPosition(atoi(val), _shell->getPositionY()) == Result::UnsupportedRequest)
				{
					WARNING_UNSUPPORTED_OPTION("posx")
				}
			}
			else if (strcasecmp(arg, "-posy") == 0)
			{
				if (_shell->setPosition(_shell->getPositionX(), atoi(val)) == Result::UnsupportedRequest)
				{
					WARNING_UNSUPPORTED_OPTION("posy")
				}
			}
			else if (strcasecmp(arg, "-swaplength") == 0 || strcasecmp(arg, "-preferredswaplength") == 0)
			{
				_shell->setPreferredSwapChainLength(atoi(val));
			}
			else if (strcasecmp(arg, "-vsync") == 0)
			{
				if (!strcasecmp(val, "on"))
				{
					_shell->setVsyncMode(VsyncMode::On);
					Log("On");
				}
				else if (!strcasecmp(val, "off"))
				{
					_shell->setVsyncMode(VsyncMode::Off);
					Log("Off");
				}
				else if (!strcasecmp(val, "relaxed"))
				{
					_shell->setVsyncMode(VsyncMode::Relaxed);
					Log("Relaxed");
				}
				else if (!strcasecmp(val, "mailbox"))
				{
					_shell->setVsyncMode(VsyncMode::Mailbox);
					Log("Mailbox");
				}
				else if (!strcasecmp(val, "half"))
				{
					_shell->setVsyncMode(VsyncMode::Half);
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
						case 0: _shell->setVsyncMode(VsyncMode::Off); break;
						case 1: _shell->setVsyncMode(VsyncMode::On); break;
						case 2: _shell->setVsyncMode(VsyncMode::Half); break;
						case -1: _shell->setVsyncMode(VsyncMode::Relaxed); break;
						case -2: _shell->setVsyncMode(VsyncMode::Mailbox); break;
						default: break;
						}
					}
				}
				Log("%d", _shell->getVsyncMode());
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
					_shell->setColorBitsPerPixel(5, 6, 5, 0);
					break;
				case 24:
					_shell->setColorBitsPerPixel(8, 8, 8, 0);
					break;
				case 32:
					_shell->setColorBitsPerPixel(8, 8, 8, 8);
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
				_shell->setDepthBitsPerPixel(atoi(val));
			}
			else if (strcasecmp(arg, "-stencilbpp") == 0 || strcasecmp(arg, "-dbpp") == 0)
			{
				_shell->setStencilBitsPerPixel(atoi(val));
			}
			/*else if(strcasecmp(arg, "-rotatekeys") == 0)
			{
			_shell->PVRShellset(PVRShell::prefRotateKeys, atoi(val));
			}*/
			else if (strcasecmp(arg, "-c") == 0)
			{
				const char* pDash = strchr(val, '-');
				uint32 start(atoi(val)), stop(pDash ? atoi(pDash + 1) : start);
				_shell->setCaptureFrames(start, stop);
			}
			else if (strcasecmp(arg, "-screenshotscale") == 0)
			{
				_shell->setCaptureFrameScale(atoi(val));
			}
			else if (strcasecmp(arg, "-priority") == 0)
			{
				_shell->setContextPriority(atoi(val));
			}
			else if (strcasecmp(arg, "-config") == 0)
			{
				_shell->setDesiredConfig(atoi(val));
			}
			/*  else if(strcasecmp(arg, "-display") == 0)
			{
			// TODO:
			_shell->PVRShellset(PVRShell::prefNativeDisplay, atoi(val));
			}*/
			else if (strcasecmp(arg, "-forceframetime") == 0 || strcasecmp(arg, "-fft") == 0)
			{
				_shell->setForceFrameTime(true);
				int value = atoi(val);
				_shell->setFakeFrameTime(std::max(1, value));
			}
		}
		else
		{
			if (strcasecmp(arg, "-version") == 0)
			{
				Log(Log.Information, "Version: '%hs'", _shell->getSDKVersion());
			}
			else if (strcasecmp(arg, "-fps") == 0)
			{
				_shell->setShowFPS(true);
			}
			else if (strcasecmp(arg, "-info") == 0)
			{
				_shellData.outputInfo = true;
			}
			else if (strcasecmp(arg, "-forceframetime") == 0 || strcasecmp(arg, "-fft") == 0)
			{
				_shell->setForceFrameTime(true);
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
			while (_currentState != StateExit) { executeOnce(); }// Loop to tidy up
		}
		if (_currentState == StateExit) { return result; }
	}
}

Result StateMachine::executeUpTo(const State state)
{
	Result result = _currentState > state ? Result::InvalidArgument : Result::Success;
	while (result == Result::Success && _currentState < state)
	{
		if (_currentState == StateRenderScene && state > StateRenderScene)
		{
			_currentState = StateReleaseView;
		}
		result = executeOnce();
	}
	return result;
}

Result StateMachine::executeOnce(const State state)
{
	_currentState = state;
	return executeOnce();
}

typedef std::auto_ptr<IPlatformContext>(*NativeContextCreatorFn)(OSManager& mgr);

Result StateMachine::executeOnce()
{
	// don't handle the events while paused
	Result result(Result::Success);
	if (_pause) {  return result;   }

	// Handle our state
	switch (_currentState)
	{
	case StateNotInitialized:
		return Result::NotInitialized;
	case StateInitApplication:
		_shell = newDemo();

		{
			readApiFromCommandLine();

			result = Result::Success;
			// DETECT LINKING - Is PVRApi linked in as a DLL or statically?
			// createNativePlatformContext it either a Real function, provided by PVRNativeApi,
			// or a dummy function, to avoid link errors, and the real one provided by PVRApi dynamically.
			// If PVRAPI_DYNAMIC_LIBRARY is defined when building the Application, the dummy function will
			// be used, and it will return a NULL pointer. In that case, we know that PVRNativeApi is NOT
			// linked in, hence the relevant functions are provided by PVRApi.

			NativeContextCreatorFn createNative = &pvr::createNativePlatformContext;
			_shellData.platformContext = createNative(*_shell);
			if (_shellData.platformContext.get())
			{
				// LINKING STATICALLY: Nothing to do. Keep walking.
				_shellData.baseContextType = _shellData.platformContext->getBaseApi();
				Log(Log.Information, "PVRApi static linking detected. Api mode '%s'. Application is built and running in Static linking mode.",
				    _shellData.baseContextType == BaseApi::OpenGLES ? "OpenGL ES" : "Vulkan");
			}
			if (!_shellData.platformContext.get())
			{
				// LINKING DYNAMICALLY: Now, attempt to load a PVRApi library based on user preferences.
#ifdef _WIN32
#define PVR_VK_LIB_NAME "PVRVulkan.dll"
#define PVR_GLES_LIB_NAME "PVRGles.dll"
#elif defined(TARGET_OS_MAC)
#define PVR_VK_LIB_NAME "libPVRVulkan.dylib"
#define PVR_GLES_LIB_NAME "libPVRGles.dylib"
#else
#define PVR_VK_LIB_NAME "libPVRVulkan.so"
#define PVR_GLES_LIB_NAME "libPVRGles.so"
#endif
				Log(Log.Information, "PVRApi dynamic linking detected.");
				bool tryVulkan = true;
				bool tryGles = true;
				switch (_shellData.baseContextType)
				{
				case BaseApi::Unspecified: Log(Log.Information, "PVRApi base version was unspecified."
					                               "Will try loading " PVR_VK_LIB_NAME " library first, otherwise " PVR_GLES_LIB_NAME " if that fails."); break;
				case BaseApi::OpenGLES:
					Log(Log.Information, "PVRApi base API type was set to OpenGL ES. "
					    "Will load PVRGles dynamic library " PVR_GLES_LIB_NAME ".");
					tryVulkan = false;
					break;
				case BaseApi::Vulkan:
					tryGles = false;
					Log(Log.Information, "PVRApi base API type was set to Vulkan. "
					    "Will load PVRVulkan dynamic library " PVR_VK_LIB_NAME "."); break;
				}

				if (tryVulkan)
				{
					_shell->pvrapi.reset(new pvr::native::NativeLibrary(PVR_VK_LIB_NAME, Log.Debug));
					if (_shell->pvrapi->LoadFailed())
					{
						Log(Log.Information, "Vulkan version of PVRApi not detected.");
					}
					else
					{
						Log(Log.Information, PVR_VK_LIB_NAME " library successfully loaded. Application will run in Vulkan mode.");
						_shellData.baseContextType = BaseApi::Vulkan;
					}
				}
				if (tryGles && (!tryVulkan || _shell->pvrapi->LoadFailed()))
				{
					_shell->pvrapi.reset(new pvr::native::NativeLibrary(PVR_GLES_LIB_NAME, Log.Debug));
					if (_shell->pvrapi->LoadFailed())
					{
						Log(Log.Information, "PVRGles not detected.");
						result = Result::UnableToOpen;
					}
					else
					{
						Log(Log.Information, "PVRGles library successfully loaded. Application will run in OpenGL ES mode.");
						_shellData.baseContextType = BaseApi::Vulkan;
					}
				}
				if (!_shell->pvrapi->LoadFailed())
				{
					createNative = _shell->pvrapi->getFunction<NativeContextCreatorFn>("createNativePlatformContextApi");
					if (!createNative)
					{
						Log(Log.Critical, "createNativePlatformContextApi function not found in PVRApi library");
						result = Result::UnableToOpen;
					}
				}
				_shellData.platformContext = createNative(*_shell);
			}
#undef PVR_VK_LIB_NAME
#undef PVR_GLES_LIB_NAME
		}// End linking dynamically
		if (result == Result::Success)
		{
			if (_shellData.platformContext.get())
			{
				result = _shell->init(&_shellData);
			}
			else
			{
				result = Result::UnableToOpen;
			}
		}

		if (result == Result::Success)
		{
			result = _shell->shellInitApplication();

			if (result == Result::Success)
			{
				_currentState = StateInitWindow;
			}
			else
			{
				_shell.reset();

				_currentState = StatePreExit;

				string error = string("InitApplication() failed with pvr error '") + Log.getResultCodeString(result) + string("'\n");
				Log(Log.Error, error.c_str());
			}
		}
		else
		{
			_shell.reset();

			_currentState = StatePreExit; // If we have reached this point, then _shell has already been initialized.
			string error = string("State Machine initialisation failed with error '") + Log.getResultCodeString(result) + string("'\n");
			Log(Log.Error, error.c_str());
		}
		break;
	case StateInitWindow:
	{
		applyCommandLine();
		// Initialize our window. On some platforms this will be a dummy function
		result = ShellOS::initializeWindow(_shellData.attributes);
		if (result == Result::Success) {  _currentState = StateInitAPI;}
		else {  _currentState = StateQuitApplication; }
	}
	break;
	case StateInitAPI:
		if (!_shellData.platformContext.get())
		{
			_currentState = StateReleaseWindow;
			return Result::NotInitialized;
		}
		else
		{
			result = _shellData.platformContext->init();

			if (result == Result::Success)
			{
				_shellData.platformContext->makeCurrent();
				_currentState = StateInitView;
			}
			else
			{
				if (_shell->getApiTypeRequired() != Api::Unspecified)
				{
					_shell->setExitMessage("Requested Graphics context type %s was unsupported on this device.", apiName(_shell->getApiTypeRequired()));
				}
				else
				{
					_shell->setExitMessage("Unable to create context. Unknown error. Examine log for details.");
				}
				_currentState = StateReleaseAPI;  // Though we failed to init the API we call ReleaseAPI to delete _pplatformContext.
			}
		}
		break;
	case StateInitView:
		result = _shell->shellInitView();

		if (result == Result::Success)
		{
			_currentState = StateRenderScene;
			_shellData.startTime = _shellData.timer.getCurrentTimeMilliSecs();
		}
		else
		{
			_currentState = StateReleaseView;

			string error = string("InitView() failed with pvr error '") + Log.getResultCodeString(result) + string("'\n");
			Log(Log.Error, error.c_str());
		}

		if (_shellData.outputInfo)
		{
			_shell->showOutputInfo();
		}
		break;
	case StateRenderScene:
	{
		// Process any OS events
		ShellOS::handleOSEvents();

		// Call RenderScene
		result = _shell->shellRenderFrame();

		if (_shellData.weAreDone && result == Result::Success)
		{
			result = Result::ExitRenderFrame;
		}

		if (result == Result::Success)
		{
			// Read from backbuffer before swapping - take screenshot if frame capture is enabled
			if ((static_cast<int32>(_shellData.frameNo) >= _shellData.captureFrameStart
			     && static_cast<int32>(_shellData.frameNo) <= _shellData.captureFrameStop))
			{
				_shell->takeScreenshot();
			}

			// Swap buffers
			result = (_shellData.presentBackBuffer && _shellData.platformContext->presentBackbuffer()) ? Result::Success : Result::UnknownError;

			if (result != Result::Success)
			{
				_currentState = StateReleaseView;
			}
		}
		else
		{
			if (result != Result::Success && result != Result::ExitRenderFrame)
			{
				string error = string("renderFrame() failed with pvr error '") + Log.getResultCodeString(result) + string("'\n");
				Log(Log.Error, error.c_str());
			}
			_currentState = StateReleaseView;
		}

		// Calculate our FPS
		{
			static uint64 prevTime(_shellData.timer.getCurrentTimeMilliSecs()), FPSFrameCount(0);
			uint64 time(_shellData.timer.getCurrentTimeMilliSecs()), delta(time - prevTime);

			++FPSFrameCount;

			if (delta >= 1000)
			{
				_shellData.FPS = 1000.0f * FPSFrameCount / (float)delta;

				FPSFrameCount = 0;
				prevTime = time;

				if (_shellData.showFPS)
				{
					Log(Log.Information, "Frame %i, FPS %.2f", _shellData.frameNo, _shellData.FPS);
				}
			}
		}

		// Have we reached the point where we need to die?
		if ((_shellData.dieAfterFrame >= 0 && _shellData.frameNo >= static_cast<uint32>(_shellData.dieAfterFrame))
		    || (_shellData.dieAfterTime >= 0 && ((_shellData.timer.getCurrentTimeMilliSecs() - _shellData.startTime) * 0.001f) > _shellData.dieAfterTime)
		    || _shellData.forceReleaseInitCycle)
		{
			if (_shellData.forceReleaseInitCycle)
			{
				Log(Log.Information, "Reinit requested. Going through Reinitialization cycle. ReleaseView will be called next, and then InitView.");
			}
			_currentState = StateReleaseView;
			break;
		}

		// Increment our frame number
		++_shellData.frameNo;
	}
	break;
	case StateReleaseView:
		Log(Log.Debug, "ReleaseView");
		result = _shell->shellReleaseView();

		if (result != Result::Success)
		{
			string error = string("ReleaseView() failed with pvr error '") + Log.getResultCodeString(result) + string("'\n");
			Log(Log.Error, error.c_str());
		}

		_currentState = StateReleaseAPI;
		break;
	case StateReleaseAPI:
		Log(Log.Debug, "ReleaseApi");
		if (_shellData.graphicsContextStore.isValid())
		{
			_shellData.graphicsContextStore->release();
			_shellData.graphicsContextStore.reset();
		}
		_shellData.graphicsContext.reset();
		_currentState = StateReleaseWindow;
		break;
	case StateReleaseWindow:
		Log(Log.Debug, "ReleaseWindow");
		if (_shellData.platformContext.get())
		{
			_shellData.platformContext->release();
		}
		ShellOS::releaseWindow();

		if (!_shellData.weAreDone && _shellData.forceReleaseInitCycle)
		{
			_shellData.forceReleaseInitCycle = false;
			_currentState = StateInitWindow;
		}
		else
		{
			_currentState = StateQuitApplication;
		}
		break;
	case StateQuitApplication:
		Log(Log.Debug, "QuitApplication");
		result = _shell->shellQuitApplication();

		if (result != Result::Success)
		{
			string error = string("QuitApplication() failed with pvr error '") + Log.getResultCodeString(result) + string("'\n");
			Log(Log.Error, error.c_str());
		}

		_shell.reset();

		_currentState = StatePreExit;
	case StatePreExit:
		Log(Log.Debug, "StateExit");
		if (!_shellData.exitMessage.empty())
		{
			ShellOS::popUpMessage(ShellOS::getApplicationName().c_str(), _shellData.exitMessage.c_str());
		}
		_shellData.platformContext.reset();
		_currentState = StateExit;
	case StateExit:
		break;
	}
	return result;
}
}
}
//!\endcond