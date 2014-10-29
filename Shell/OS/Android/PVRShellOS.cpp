/******************************************************************************
 @File          Android/PVRShellOS.cpp

 @Title         Android/PVRShellOS

 @Brief         The OS part of PVRShell


 @Copyright    Copyright (C)  Imagination Technologies Limited.

 @Platform      Non-windowed support for any Linux

 @Description   Makes programming for 3D APIs easier by wrapping window creation
 				and other functions for use by a demo.

*****************************************************************************/

#include "PVRShell.h"
#include "PVRShellAPI.h"
#include "PVRShellOS.h"
#include "PVRShellImpl.h"

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android/window.h>

// No Doxygen for CPP files, due to documentation duplication
/// @cond NO_DOXYGEN

/*!***************************************************************************
	Defines
*****************************************************************************/

/*!***************************************************************************
	Declarations
*****************************************************************************/
AAssetManager* g_AssetManager = 0;

struct SHandle
{
	size_t size;
	char* pData;
};

static void* LoadFileFunc(const char* pFilename, char** pData, size_t &size)
{
	size = 0;
	AAsset* pAsset = AAssetManager_open(g_AssetManager, pFilename, AASSET_MODE_BUFFER);

	if(pAsset)
	{
		off_t length = AAsset_getLength(pAsset);

		if(length)
		{
			SHandle *pHandle = new SHandle();

			if(pHandle)
			{
				pHandle->size = length;
				pHandle->pData = new char[length];

				if(pHandle->pData)
				{
					if(length == AAsset_read(pAsset, pHandle->pData, length))
					{
						AAsset_close(pAsset);

						size = length;
						*pData = pHandle->pData;
						return pHandle;
					}

					delete[] pHandle->pData;
				}

				delete pHandle;
			}
		}

		AAsset_close(pAsset);
	}

	return 0;
}

static bool ReleaseFileFunc(void* handle)
{
	if(handle)
	{
		SHandle *pHandle = (SHandle*) handle;
		delete[] pHandle->pData;
		delete pHandle;
		return true;
	}

	return false;
}

/*!***************************************************************************
	Class: PVRShellInit
*****************************************************************************/

/*!***********************************************************************
@Function		PVRShellOutputDebug
@Input			format			printf style format followed by arguments it requires
@Description	Writes the resultant string to the debug output (e.g. using
				printf(), OutputDebugString(), ...). Check the SDK release notes for
				details on how the string is output.
*************************************************************************/
void PVRShell::PVRShellOutputDebug(char const * const format, ...) const
{
	if(!format)
		return;

	va_list arg;
	char	buf[1024];

	va_start(arg, format);
	vsnprintf(buf, 1024, format, arg);
	va_end(arg);

	// Passes the data to a platform dependant function
	m_pShellInit->OsDisplayDebugString(buf);
}

/*!***********************************************************************
 @Function		OsInit
 @description	Initialisation for OS-specific code.
*************************************************************************/
void PVRShellInit::OsInit()
{
	m_pShell->m_pShellData->bFullScreen= true;	// linux overrides default to use fullscreen

	// Get PID (Process ID)
	char* pszAppName=0, pszSrcLink[64];

	snprintf(pszSrcLink, 64, "/proc/%d/cmdline", getpid());

	FILE* pFile = fopen(pszSrcLink, "rb");

	if(pFile)
	{
		// Get the file size
		size_t size = 128;

		pszAppName = (char*) malloc(size);

		if(pszAppName)
		{
			size_t bytesRead = 0;
			char *ptr = pszAppName;

			while((bytesRead = fread(ptr, 1, 128, pFile)) == 128)
			{
				char *resized = (char*) realloc(pszAppName, size + 128);

				if(!resized)
				{
					free(pszAppName);
					pszAppName = 0;
					break;
				}

				size += 128;
				pszAppName = resized;
				ptr = pszAppName + size;
			}
		}

		fclose(pFile);
	}

	if(!pszAppName)
	{
		m_pShell->PVRShellOutputDebug("Warning: Unable to set app name.\n");
	}
	else
	{
		SetAppName(pszAppName);
		free(pszAppName);
	}

	// Setup the read/write path

	// Construct the binary path for GetReadPath() and GetWritePath()
	char* internalDataPath = (char*) m_pAndroidState->activity->internalDataPath;

	if(!internalDataPath) // Due to a bug in Gingerbread this will always be Null
	{
		m_pShell->PVRShellOutputDebug("Warning: The internal data path returned from Android is null. Attempting to generate from the app name..\n");

		pszAppName = m_pShell->m_pShellData->pszAppName;

		if(pszAppName)
		{
			size_t size = strlen("/data/data/") + strlen(pszAppName) + 2;

			internalDataPath = (char*) malloc(size);

			if(internalDataPath)
				snprintf(internalDataPath, size, "/data/data/%s/", pszAppName);
		}

		if(!internalDataPath)
			internalDataPath = (char*) "/sdcard/";

		SetWritePath(internalDataPath);

		if(pszAppName)
			free(internalDataPath);
	}
	else
	{
		SetWritePath(internalDataPath);
	}

	SetReadPath(""); // Empty, external files should be read from the .apk file

	gettimeofday(&m_StartTime,NULL);
}

/*!***********************************************************************
 @Function		OsInitOS
 @description	Saves instance handle and creates main window
				In this function, we save the instance handle in a global variable and
				create and display the main program window.
*************************************************************************/
bool PVRShellInit::OsInitOS()
{
	return true;
}

/*!***********************************************************************
 @Function		OsReleaseOS
 @description	Destroys main window
*************************************************************************/
void PVRShellInit::OsReleaseOS()
{
}

/*!***********************************************************************
 @Function		OsExit
 @description	Destroys main window
*************************************************************************/
void PVRShellInit::OsExit()
{
	// Show the exit message to the user
	const char* pExitMessage = static_cast<const char*>(m_pShell->PVRShellGet(prefExitMessage));

	if(pExitMessage)
	{
		if(m_pAndroidState)
		{
			ANativeActivity* activity = m_pAndroidState->activity;

			JNIEnv *env;
			activity->vm->AttachCurrentThread(&env, NULL); 

			jclass clazz = env->GetObjectClass(activity->clazz);
			jmethodID methodID = env->GetMethodID(clazz, "displayExitMessage", "(Ljava/lang/String;)V");
			jstring exitMsg = env->NewStringUTF(pExitMessage);
			env->CallVoidMethod(activity->clazz, methodID, exitMsg);

			activity->vm->DetachCurrentThread();
		}

		m_pShell->PVRShellOutputDebug(pExitMessage);
	}
}

/*!***********************************************************************
 @Function		OsDoInitAPI
 @Return		true on success
 @description	Perform API initialisation and bring up window / fullscreen
*************************************************************************/
bool PVRShellInit::OsDoInitAPI()
{
	if(!ApiInitAPI())
	{
		return false;
	}

	// No problem occured
	return true;
}

/*!***********************************************************************
 @Function		OsDoReleaseAPI
 @description	Clean up after we're done
*************************************************************************/
void PVRShellInit::OsDoReleaseAPI()
{
	ApiReleaseAPI();
}

/*!***********************************************************************
 @Function		OsRenderComplete
 @Returns		false when the app should quit
 @description	Main message loop / render loop
*************************************************************************/
void PVRShellInit::OsRenderComplete()
{
}

/*!***********************************************************************
 @Function		OsPixmapCopy
 @Return		true if the copy succeeded
 @description	When using pixmaps, copy the render to the display
*************************************************************************/
bool PVRShellInit::OsPixmapCopy()
{
	return false;
}

/*!***********************************************************************
 @Function		OsGetNativeDisplayType
 @Return		The 'NativeDisplayType' for EGL
 @description	Called from InitAPI() to get the NativeDisplayType
*************************************************************************/
void *PVRShellInit::OsGetNativeDisplayType()
{
	return 0;
}

/*!***********************************************************************
 @Function		OsGetNativePixmapType
 @Return		The 'NativePixmapType' for EGL
 @description	Called from InitAPI() to get the NativePixmapType
*************************************************************************/
void *PVRShellInit::OsGetNativePixmapType()
{
	// Pixmap support: return the pixmap
	return 0;
}

/*!***********************************************************************
 @Function		OsGetNativeWindowType
 @Return		The 'NativeWindowType' for EGL
 @description	Called from InitAPI() to get the NativeWindowType
*************************************************************************/
void *PVRShellInit::OsGetNativeWindowType()
{
	return m_pAndroidState ? m_pAndroidState->window : 0;
}

/*!***********************************************************************
 @Function		OsGet
 @Input			prefName	Name of value to get
 @Modified		pn A pointer set to the value asked for
 @Returns		true on success
 @Description	Retrieves OS-specific data
*************************************************************************/
bool PVRShellInit::OsGet(const prefNameIntEnum prefName, int *pn)
{
	return false;
}

/*!***********************************************************************
 @Function		OsGet
 @Input			prefName	Name of value to get
 @Modified		pp A pointer set to the value asked for
 @Returns		true on success
 @Description	Retrieves OS-specific data
*************************************************************************/
bool PVRShellInit::OsGet(const prefNamePtrEnum prefName, void **pp)
{
	switch(prefName)
	{
		case prefLoadFileFunc:
		{
			*pp = (void*) &LoadFileFunc;
			return true;
		}
		case prefReleaseFileFunc:
		{
			*pp = (void*) &ReleaseFileFunc;
			return true;
		}
		case prefAndroidNativeActivity:
		{
			if(m_pAndroidState && m_pAndroidState->activity)
			{
				*pp = (void*) m_pAndroidState->activity;
				return true;
			}
			return false;
		}
		default:
			return false;
	}
	return false;
}

/*!***********************************************************************
 @Function		OsSet
 @Input			prefName				Name of preference to set to value
 @Input			value					Value
 @Return		true for success
 @Description	Sets OS-specific data
*************************************************************************/
bool PVRShellInit::OsSet(const prefNameBoolEnum prefName, const bool value)
{
	return false;
}

/*!***********************************************************************
 @Function		OsSet
 @Input			prefName	Name of value to set
 @Input			i32Value 	The value to set our named value to
 @Returns		true on success
 @Description	Sets OS-specific data
*************************************************************************/
bool PVRShellInit::OsSet(const prefNameIntEnum prefName, const int i32Value)
{
	return false;
}

/*!***********************************************************************
 @Function		OsDisplayDebugString
 @Input			str		string to output
 @Description	Prints a debug string
*************************************************************************/
void PVRShellInit::OsDisplayDebugString(char const * const str)
{
	const char* pszAppName = m_pShell->m_pShellData->pszAppName;
	__android_log_print(ANDROID_LOG_INFO, pszAppName ? pszAppName : "PVRShell", "%s", str);
}

/*!***********************************************************************
 @Function		OsGetTime
 @Return		An incrementing time value measured in milliseconds
 @Description	Returns an incrementing time value measured in milliseconds
*************************************************************************/
unsigned long PVRShellInit::OsGetTime()
{
	timeval tv;
	gettimeofday(&tv,NULL);

	if(tv.tv_sec < m_StartTime.tv_sec)
		m_StartTime.tv_sec = 0;

	unsigned long sec = tv.tv_sec - m_StartTime.tv_sec;
	return (unsigned long)((sec*(unsigned long)1000) + (tv.tv_usec/1000.0));
}

/*****************************************************************************
 Class: PVRShellInitOS
*****************************************************************************/

/*****************************************************************************
 Global code
*****************************************************************************/
static int32_t handle_input(struct android_app* app, AInputEvent* event)
{
	PVRShellInit* init = (PVRShellInit*) app->userData;

	if(init)
	{
		switch(AInputEvent_getType(event))
		{
			case AINPUT_EVENT_TYPE_KEY: // Handle keyboard events
			{
				switch(AKeyEvent_getAction(event))
				{
					case AKEY_EVENT_ACTION_DOWN:
					{
						switch(AKeyEvent_getKeyCode(event))
						{
							case AKEYCODE_Q:			init->KeyPressed(PVRShellKeyNameQUIT);		break;
							case AKEYCODE_BACK:			init->KeyPressed(PVRShellKeyNameQUIT);		break;
							case AKEYCODE_DPAD_CENTER:	init->KeyPressed(PVRShellKeyNameSELECT);	break;
							case AKEYCODE_SPACE: 		init->KeyPressed(PVRShellKeyNameACTION1); 	break;
							case AKEYCODE_SHIFT_LEFT: 	init->KeyPressed(PVRShellKeyNameACTION2); 	break;
							case AKEYCODE_DPAD_UP: 		init->KeyPressed(init->m_eKeyMapUP); 		break;
							case AKEYCODE_DPAD_DOWN: 	init->KeyPressed(init->m_eKeyMapDOWN); 		break;
							case AKEYCODE_DPAD_LEFT: 	init->KeyPressed(init->m_eKeyMapLEFT);  	break;
							case AKEYCODE_DPAD_RIGHT: 	init->KeyPressed(init->m_eKeyMapRIGHT); 	break;
							case AKEYCODE_S: 			init->KeyPressed(PVRShellKeyNameScreenshot);break;
							default:
								break;
						}
					}
					return 1;

					default:
						break;
				}
				return 1;
			}
			case AINPUT_EVENT_TYPE_MOTION: // Handle touch events
			{
				switch(AMotionEvent_getAction(event))
				{
					case AMOTION_EVENT_ACTION_DOWN:
					{
						PVRShell *pShell = init->m_pShell;
						if(pShell)
						{
							float vec2TouchPosition[2] = { AMotionEvent_getX(event, 0) / pShell->PVRShellGet(prefWidth), AMotionEvent_getY(event, 0) / pShell->PVRShellGet(prefHeight) };
							init->TouchBegan(vec2TouchPosition);
						}
						break;
					}
					case AMOTION_EVENT_ACTION_MOVE:
					{
						PVRShell *pShell = init->m_pShell;

						if(pShell)
						{
							float vec2TouchPosition[2] = { AMotionEvent_getX(event, 0) / pShell->PVRShellGet(prefWidth), AMotionEvent_getY(event, 0) / pShell->PVRShellGet(prefHeight) };
							init->TouchMoved(vec2TouchPosition);
						}
						break;
					}
					case AMOTION_EVENT_ACTION_UP:
					{
						PVRShell *pShell = init->m_pShell;
						if(pShell)
						{
							float vec2TouchPosition[2] = { AMotionEvent_getX(event, 0) / pShell->PVRShellGet(prefWidth), AMotionEvent_getY(event, 0) / pShell->PVRShellGet(prefHeight) };
							init->TouchEnded(vec2TouchPosition);
						}
						break;
					}
				}
				return 1;
			}
		}
	}

    return 1;
}

static void handle_cmd(struct android_app* app, int32_t cmd)
{
    PVRShellInit* init = (PVRShellInit*) app->userData;

    switch (cmd)
    {
		case APP_CMD_START:
			init->m_bRendering = true;
			break;
		case APP_CMD_RESUME:
			init->m_bRendering = true;
			break;
		case APP_CMD_PAUSE:
			init->m_bRendering = false;
			break;
        case APP_CMD_SAVE_STATE:
			// PVRShell doesn't support saving our state
			init->m_bRendering = false;
            break;
        case APP_CMD_INIT_WINDOW:
			// Call init view
			init->m_eState = ePVRShellInitInstance;
				
			init->m_bRendering = init->Run() && init->m_eState == ePVRShellRender;
			init->m_bError = !init->m_bRendering;
            break;

		case APP_CMD_WINDOW_RESIZED:
			// Unsupported by the shell
			break;
        case APP_CMD_TERM_WINDOW: // The window is being hidden or closed.
           	// Call release view
           	if(init->m_eState <= ePVRShellReleaseView)
           	{
				init->m_eState = ePVRShellReleaseView;
				init->Run();
			}

			init->m_bRendering = false;
            break;
        case APP_CMD_STOP:
        	init->m_bRendering = false;
            break;
        case APP_CMD_DESTROY:
        	init->Deinit();
        	break;
    }
}

/*!***************************************************************************
@function		android_main
@input			state	the android app state
@description	Main function of the program
*****************************************************************************/
void android_main(struct android_app* state)
{
    // Make sure glue isn't stripped.
    app_dummy();

    // Initialise the demo, process the command line, create the OS initialiser.
    PVRShellInit init;

	{ // Handle command-line
		/*
		How to launch an app from an adb shell with command-line options, e.g. 
	
			am start -a android.intent.action.MAIN -n com.powervr.OGLESIntroducingPOD/.OGLESIntroducingPOD --es args "-info"
		*/
		ANativeActivity* activity = state->activity;
		
		JNIEnv *env;
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
            const char * args = env->GetStringUTFChars(jsArgs, 0);
           
            init.CommandLine(args);

            // Tidy up the args string
            env->ReleaseStringUTFChars(jsArgs, args);
        }
		
		activity->vm->DetachCurrentThread();
	}
	
	// Setup our android state
	state->userData = &init;
	state->onAppCmd = handle_cmd;
	state->onInputEvent = handle_input;

	init.m_pAndroidState = state;
	g_AssetManager = state->activity->assetManager;

	if(!init.Init())
	{
		__android_log_print(ANDROID_LOG_INFO, "PVRShell", "Error: Failed to initialise");
		ANativeActivity_finish(state->activity);
		return;
	}

	// Call init app
	init.m_eState = ePVRShellInitApp;
	init.m_bError = !(init.Run() && init.m_eState == ePVRShellInitInstance);

	// Handle our events until we have a valid window or destroy has been requested
	int ident;
	int events;
    struct android_poll_source* source;

	//	Initialise our window/run/shutdown
	for(;;)
	{
		while ((ident = ALooper_pollAll((init.m_eState == ePVRShellRender && init.m_bRendering) ? 0 : -1, NULL, &events, (void**)&source)) >= 0)
		{
			if(init.m_bError)
			{
				ANativeActivity_finish(state->activity);
				
				// An error has occurred during setup. Execute the run loop till everything has been tidied up.
				while(init.Run()) { }

				init.m_bError = false;
			}

			// Process this event.
			if (source != NULL)
			{
				source->process(state, source);
			}

			// Check if we are exiting.
			if (state->destroyRequested != 0)
			{
				return;
			}
		}
		
		// Render our scene
		do
		{
			if(!init.Run())
			{
				ANativeActivity_finish(state->activity);
				break;
			}
		}
		while( init.m_eState != ePVRShellRender );
	}
}

/// @endcond

/*****************************************************************************
 End of file (PVRShellOS.cpp)
*****************************************************************************/
