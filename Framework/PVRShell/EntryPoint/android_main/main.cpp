/*!
\brief Entry point for Android systems (android_main).
\file PVRShell\EntryPoint/android_main/main.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/OS/ShellOS.h"
#include "PVRShell/StateMachine.h"
#include "PVRShell/CommandLine.h"
#include "PVRCore/Log.h"
#include <android_native_app_glue.h>

/// <summary>A function to handle the android OS system messages. Used for lifecycle management.</summary>
/// <param name="app">The android application.</param>
/// <param name="cmd">The command type (android predetermined).</param>
static void handle_cmd(struct android_app* app, int32_t cmd)
{
	pvr::platform::StateMachine* stateMachinePtr = static_cast<pvr::platform::StateMachine*>(app->userData);
	pvr::Result result;
	switch (cmd)
	{
	case APP_CMD_START:
		(LogLevel::Debug, "APP_CMD_START");
		result = pvr::Result::Success;

		if (stateMachinePtr->getCurrentState() == pvr::platform::StateMachine::StateNotInitialized)
		{
			(LogLevel::Debug, "Initializing State Machine");
			if ((result = stateMachinePtr->init()) != pvr::Result::Success)
			{
				(LogLevel::Error, "Error: Failed to initialize main State Machine with code %s", getResultCodeString(result));
				ANativeActivity_finish(app->activity);
				return;
			}
		}
		else
		{
			(LogLevel::Debug, "State Machine already Initialized");
		}
		if (stateMachinePtr->getCurrentState() == pvr::platform::StateMachine::StateInitApplication ||
		    stateMachinePtr->getCurrentState() > pvr::platform::StateMachine::StateQuitApplication)
		{
			(LogLevel::Debug, "Executing Init Application");
			if ((result = stateMachinePtr->executeOnce(pvr::platform::StateMachine::StateInitApplication)) != pvr::Result::Success)
			{
				ANativeActivity_finish(app->activity);
				return;
			}
		}
		else
		{
			(LogLevel::Debug, "Skipped Init Application.");
		}
		break;
	case APP_CMD_PAUSE:
		(LogLevel::Debug, "APP_CMD_PAUSE");
		stateMachinePtr->pause();
		break;
	case APP_CMD_RESUME:
		(LogLevel::Debug, "APP_CMD_RESUME");
		stateMachinePtr->resume();
		break;
	case APP_CMD_INIT_WINDOW:
		(LogLevel::Debug, "APP_CMD_INIT_WINDOW");
		stateMachinePtr->resume();

		if (stateMachinePtr->getCurrentState() != pvr::platform::StateMachine::StateInitWindow &&
		    stateMachinePtr->getCurrentState() < pvr::platform::StateMachine::StateReleaseView)
		{
			(LogLevel::Debug, "APP_CMD_INIT_WINDOW Was the wrong state: %d", stateMachinePtr->getCurrentState());
			ANativeActivity_finish(app->activity);
			return;
		}
		if (stateMachinePtr->executeOnce(pvr::platform::StateMachine::StateInitWindow) != pvr::Result::Success)
		{
			(LogLevel::Debug, "APP_CMD_INIT_WINDOW failed to reach InitWindow");
			ANativeActivity_finish(app->activity);
			return;
		}

		if (stateMachinePtr->executeUpTo(pvr::platform::StateMachine::StateRenderScene) != pvr::Result::Success)
		{
			(LogLevel::Debug, "APP_CMD_INIT_WINDOW failed to reach RenderScene");
			ANativeActivity_finish(app->activity);
			return;
		}

		break;
	case APP_CMD_TERM_WINDOW:
		(LogLevel::Debug, "APP_CMD_TERM_WINDOW");
		stateMachinePtr->resume();

		if (stateMachinePtr->getState() < pvr::platform::StateMachine::StateReleaseView)
		{
			if (stateMachinePtr->executeOnce(pvr::platform::StateMachine::StateReleaseView) != pvr::Result::Success)
			{
				ANativeActivity_finish(app->activity);
				return;
			}
			(LogLevel::Debug, "APP_CMD_TERM_WINDOW:ReleaseViewDone");
		}
		if (stateMachinePtr->executeUpTo(pvr::platform::StateMachine::StateQuitApplication) != pvr::Result::Success)
		{
			(LogLevel::Debug, "APP_CMD_TERM_WINDOW:Failed release window.");
			ANativeActivity_finish(app->activity);
			return;
		}
		(LogLevel::Debug, "APP_CMD_TERM_WINDOW:Release window done");
		break;
	case APP_CMD_STOP:
		(LogLevel::Debug, "APP_CMD_STOP");
		break;
	case APP_CMD_DESTROY:
		(LogLevel::Debug, "APP_CMD_DESTROY");
		stateMachinePtr->resume();
		if (stateMachinePtr->executeUpTo(pvr::platform::StateMachine::StateExit) != pvr::Result::Success)
		{
			ANativeActivity_finish(app->activity);
			return;
		}
		break;
	default:
		return;
	};
}

/// <summary>Main function: Entry point for the Android platform</summary>
/// <param name="state">the android app state</param>
/// <remarks>This Main function is the Entry point for a NativeActivity- style android NDK main app</remarks>
void android_main(struct android_app* state)
{
	app_dummy();

	// Make sure glue isn't stripped.
	pvr::platform::CommandLineParser commandLine;

	{
		// Handle command-line
		/*
		How to launch an app from an adb shell with command-line options, e.g.

			am start -a android.intent.action.MAIN -n com.powervr.OGLESIntroducingPOD/.OGLESIntroducingPOD --es args "-info"
		*/
		ANativeActivity* activity = state->activity;

		JNIEnv* env;
		activity->vm->AttachCurrentThread(&env, 0);

		jobject me = activity->clazz;

		jclass acl = env->GetObjectClass(me); //class pointer of NativeActivity
		jmethodID giid = env->GetMethodID(acl, "getIntent", "()Landroid/content/Intent;");

		jobject intent = env->CallObjectMethod(me, giid); //Got our intent
		jclass icl = env->GetObjectClass(intent); //class pointer of Intent
		jmethodID gseid = env->GetMethodID(icl, "getStringExtra", "(Ljava/lang/String;)Ljava/lang/String;");

		jstring jsArgs = (jstring)env->CallObjectMethod(intent, gseid, env->NewStringUTF("args"));

		if (jsArgs != NULL)
		{
			const char* args = env->GetStringUTFChars(jsArgs, 0);

			if (args != NULL)
			{
				commandLine.set(args);

				// Tidy up the args std::string
				env->ReleaseStringUTFChars(jsArgs, args);
			}
		}

		activity->vm->DetachCurrentThread();
	}

	pvr::platform::StateMachine stateMachine(state, commandLine, NULL);

	state->userData = &stateMachine;
	state->onAppCmd = &handle_cmd;

	// Handle our events until we have a valid window or destroy has been requested
	int events;
	struct android_poll_source* source;

	//	Initialize our window/run/shutdown
	while (true)
	{
		while (ALooper_pollAll((stateMachine.getState() == pvr::platform::StateMachine::StateRenderScene && !stateMachine.isPaused()) ? 0 : -1, NULL, &events, (void**)&source) >= 0)
		{
			// Process this event.
			if (source != NULL)
			{
				source->process(state, source);
			}

			// Check if we are exiting.
			if (state->destroyRequested != 0)
			{
				(LogLevel::Debug, "MAIN: Destroy requested. Exiting applications");
				return;
			}
		}

		// Render our scene
		do
		{
			if (stateMachine.executeOnce() != pvr::Result::Success)
			{
				(LogLevel::Debug, "MAIN: Requesting main finish...");
				ANativeActivity_finish(state->activity);
				break;
			}
			if (stateMachine.getState() == pvr::platform::StateMachine::StateExit)
			{
				return;
			}
		}
		while (stateMachine.getState() != pvr::platform::StateMachine::StateRenderScene);
	}
}