/*!*********************************************************************************************************************
\file         PVRPlatformGlue\EGL\EglPlatformContext.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the implementation of the PlatformContext class for EGL. Provides the implementation of the important
createNativePlatformContext function that the PVRShell uses to create the graphics context used for the main
application window.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRPlatformGlue/EGL/NativeLibraryEgl.h"
#include "PVRPlatformGlue/EGL/ExtensionLoaderEgl.h"
#include "PVRPlatformGlue/EGL/EglPlatformHandles.h" //CAREFUL OF THE ORDER.
#include "PVRPlatformGlue/PlatformContext.h"
#include "PVRCore/StringFunctions.h"

bool isOpenGLES31NotSupported_Workaround = false;

#ifndef EGL_CONTEXT_LOST_IMG
/*! Extended error code EGL_CONTEXT_LOST_IMG generated when power management event has occurred. */
#define EGL_CONTEXT_LOST_IMG        0x300E
#endif

#ifndef EGL_CONTEXT_PRIORITY_LEVEL_IMG
/*! An extensions added to the list of attributes for the context to give it a priority hint */
#define EGL_CONTEXT_PRIORITY_LEVEL_IMG    0x3100
/*! Request the context is created with high priority */
#define EGL_CONTEXT_PRIORITY_HIGH_IMG   0x3101
/*! Request the context is created with medium priority */
#define EGL_CONTEXT_PRIORITY_MEDIUM_IMG   0x3102
/*! Request the context is created with low priority */
#define EGL_CONTEXT_PRIORITY_LOW_IMG    0x3103
#endif

namespace pvr {
using std::string;
namespace platform {

// File local function to return the global context store.
inline static std::map<int32, platform::PlatformContext*>& getContextStore()
{
	static std::map<int32, platform::PlatformContext*> contextStore;
	return contextStore;
}

// This function would actually return the Current Context of this thread, but due to EGL and other API's considerations, it cannot always be guaranteed that a context can remain
// bound - for example, if it is actually the SAME context that we tried to bind to more than one threads, it will automatically stop being bound to the previous thread but we
// have no way of detecting that.
size_t& idOfLastBoundContextPerThread()
{
	static PVR_THREAD_LOCAL size_t currentThreadId = (size_t) - 1;
	return currentThreadId;
}

PlatformContext* PlatformContext::getLastBoundContext() { return getContextStore()[(pvr::int32)idOfLastBoundContextPerThread()]; }

char const* eglErrorToStr(EGLint errorCode)
{
	switch (errorCode)
	{
	case EGL_SUCCESS: return "EGL_SUCCESS";
	case EGL_NOT_INITIALIZED: return "EGL_NOT_INITIALIZED";
	case EGL_BAD_ACCESS: return "EGL_BAD_ACCESS";
	case EGL_BAD_ALLOC: return "EGL_BAD_ALLOC";
	case EGL_BAD_ATTRIBUTE: return "EGL_BAD_ATTRIBUTE";
	case EGL_BAD_CONTEXT: return "EGL_BAD_CONTEXT";
	case EGL_BAD_CONFIG: return "EGL_BAD_CONFIG";
	case EGL_BAD_CURRENT_SURFACE: return "EGL_BAD_CURRENT_SURFACE";
	case EGL_BAD_DISPLAY: return "EGL_BAD_DISPLAY";
	case EGL_BAD_SURFACE: return "EGL_BAD_SURFACE";
	case EGL_BAD_MATCH: return "EGL_BAD_MATCH";
	case EGL_BAD_PARAMETER: return "EGL_BAD_PARAMETER";
	case EGL_BAD_NATIVE_PIXMAP: return "EGL_BAD_NATIVE_PIXMAP";
	case EGL_BAD_NATIVE_WINDOW: return "EGL_BAD_NATIVE_WINDOW";
	case EGL_CONTEXT_LOST: return "EGL_CONTEXT_LOST";
	default: return "EGL_SUCCESS";
	}
}

void logEGLConfiguration(const DisplayAttributes& attributes)
{
	Log(Log.Information, "=== Final EGL Configuration ===");
	Log(Log.Information, "\tRedBits: %d", attributes.redBits);
	Log(Log.Information, "\tGreenBits: %d", attributes.greenBits);
	Log(Log.Information, "\tBlueBits: %d", attributes.blueBits);
	Log(Log.Information, "\tAlphaBits: %d", attributes.alphaBits);
	Log(Log.Information, "\tDepthBits: %d", attributes.depthBPP);
	Log(Log.Information, "\tStencilBits: %d", attributes.stencilBPP);
	Log(Log.Information, "\taaSamples: %d", attributes.aaSamples);
	Log(Log.Information, "\tFullScreen: %s", (attributes.fullscreen ? "true" : "false"));
	Log(Log.Information, "===============================");
}

glm::uint32 PlatformContext::getSwapChainLength() const { return 1; }


void PlatformContext::release()
{
	if (m_initialized)
	{
		// Check the current context/surface/display. If they are equal to the handles in this class, remove them from the current context
		if (m_platformContextHandles->display == egl::GetCurrentDisplay() &&
		    m_platformContextHandles->display != EGL_NO_DISPLAY &&
		    m_platformContextHandles->drawSurface == egl::GetCurrentSurface(EGL_DRAW) &&
		    m_platformContextHandles->readSurface == egl::GetCurrentSurface(EGL_READ) &&
		    m_platformContextHandles->context == egl::GetCurrentContext())
		{
			egl::MakeCurrent(egl::GetCurrentDisplay(), EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		}

		// These are all refcounted, so these can be safely deleted.
		if (m_platformContextHandles->display)
		{
			if (m_platformContextHandles->context)
			{
				egl::DestroyContext(m_platformContextHandles->display, m_platformContextHandles->context);
			}

			if (m_platformContextHandles->drawSurface)
			{
				egl::DestroySurface(m_platformContextHandles->display, m_platformContextHandles->drawSurface);
			}

			if (m_platformContextHandles->readSurface && m_platformContextHandles->readSurface != m_platformContextHandles->drawSurface)
			{
				egl::DestroySurface(m_platformContextHandles->display, m_platformContextHandles->readSurface);
			}

			egl::Terminate(m_platformContextHandles->display);
		}

		m_initialized = false;
	}
	m_ContextImplementationID = size_t(-1);
	m_maxApiVersion = Api::Unspecified;
	m_preInitialized = false;
}

static inline EGLContext getContextForConfig(EGLDisplay display, EGLConfig config, Api graphicsapi)
{
	static bool firstRun = true;
	EGLint contextAttributes[10];
	int i = 0;

	int requestedMajorVersion = -1;
	int requestedMinorVersion = -1;

	switch (graphicsapi)
	{
	case Api::OpenGLES2:
		requestedMajorVersion = 2;
		requestedMinorVersion = 0;
		break;
	case Api::OpenGLES3:
		requestedMajorVersion = 3;
		requestedMinorVersion = 0;
		break;
	case Api::OpenGLES31:
		requestedMajorVersion = 3;
		requestedMinorVersion = 1;
		break;
	default:
		return EGL_NO_CONTEXT;
	}

#ifdef DEBUG
	int debug_flag = 0;
#endif

#if defined(EGL_CONTEXT_MAJOR_VERSION_KHR)
	if (egl::isEglExtensionSupported(display, "EGL_KHR_create_context"))
	{
		if (firstRun)
		{
			firstRun = false;
			Log(Log.Information, "EGL context creation: EGL_KHR_create_context supported");
		}
		contextAttributes[i++] = EGL_CONTEXT_MAJOR_VERSION_KHR;
		contextAttributes[i++] = requestedMajorVersion;
		contextAttributes[i++] = EGL_CONTEXT_MINOR_VERSION_KHR;
		contextAttributes[i++] = requestedMinorVersion;
#ifdef DEBUG
		debug_flag = i;
		contextAttributes[i++] = EGL_CONTEXT_FLAGS_KHR;
		contextAttributes[i++] = EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;

#endif
	}
	else
#endif
	{
		if (firstRun)
		{
			firstRun = false;
			Log(Log.Information, "EGL context creation: EGL_KHR_create_context NOT supported. Minor versions and debug context are unavailable.");
		}
		contextAttributes[i++] = EGL_CONTEXT_CLIENT_VERSION;
		contextAttributes[i++] = requestedMajorVersion;
	}

	contextAttributes[i] = EGL_NONE;

	//Create the context
	EGLContext ctx = egl::CreateContext(display, config, NULL, contextAttributes);
	if (ctx == EGL_NO_CONTEXT)
	{
#ifdef DEBUG
		//RETRY! Do we support debug bit?
		egl::GetError(); //clear error
		contextAttributes[debug_flag] = EGL_NONE;
		ctx = egl::CreateContext(display, config, NULL, contextAttributes);
#endif
	}
	return ctx;
}

static inline Result isGlesVersionSupported(EGLDisplay display, DisplayAttributes& attributes, Api graphicsapi, bool& isSupported)
{
#if defined(TARGET_OS_MAC)
	/* Max Api supported on OSX is OGLES3*/
	if (graphicsapi > pvr::Api::OpenGLES3) { return Result::UnsupportedRequest; }
#endif

	isSupported = false;
	std::vector<EGLConfig> configs;
	EGLint configAttributes[32];
	unsigned int i = 0;


	if (attributes.configID > 0)
	{
		configAttributes[i++] = EGL_CONFIG_ID;
		configAttributes[i++] = attributes.configID;
	}
	else
	{
		configAttributes[i++] = EGL_SURFACE_TYPE;
		configAttributes[i++] = EGL_WINDOW_BIT;

		switch (graphicsapi)
		{
		case Api::OpenGLES2: //GLES2
			Log(Log.Debug, "EglPlatformContext.cpp: isGlesVersionSupported: Setting EGL_OPENGL_ES2_BIT");
			configAttributes[i++] = EGL_RENDERABLE_TYPE;
			configAttributes[i++] = EGL_OPENGL_ES2_BIT;
			break;
		case Api::OpenGLES3: //GLES2
		case Api::OpenGLES31: //GLES2
			Log(Log.Debug, "EglPlatformContext.cpp: isGlesVersionSupported: Setting EGL_OPENGL_ES3_BIT_KHR");
			configAttributes[i++] = EGL_RENDERABLE_TYPE;
			configAttributes[i++] = EGL_OPENGL_ES3_BIT_KHR;
			break;
		default:
			return Result::UnknownError;
			break;
		}
	}

	configAttributes[i] = EGL_NONE;

	EGLint  numConfigs;
	EGLint  configsSize;


	// Find out how many configs there are in total that match our criteria
	if (!egl::ChooseConfig(display, configAttributes, NULL, 0, &configsSize))
	{
		Log("EglPlatformContext.cpp: getMaxEglVersion: eglChooseConfig error");
		return Result::UnknownError;
	}
	Log(Log.Debug, "EglPlatformContext.cpp: isGlesVersionSupported: number of configurations found for ES version [%s] was [%d]", apiName(graphicsapi), configsSize);
	if (configsSize >= 0)
	{
		configs.resize(configsSize);

		if (egl::ChooseConfig(display, configAttributes, configs.data(), configsSize, &numConfigs) != EGL_TRUE
		    || numConfigs != configsSize)
		{
			Log("EglPlatformContext.cpp: getMaxEglVersion - eglChooseConfig unexpected error %x getting list of configurations, but %d possible configs were already detected.",
			    egl::GetError(), configsSize);
			return Result::UnknownError;
		}

		Log(Log.Information, "Trying to create context for all configs.");
		for (size_t ii = 0; ii != configs.size(); ++ii)
		{
			EGLContext ctx;
			if ((ctx = getContextForConfig(display, configs[ii], graphicsapi)) != EGL_NO_CONTEXT)
			{
				Log(Log.Information, "SUCCESS creating context! Reporting success. (Used config #%d) .", ii);
				isSupported = true;
				egl::DestroyContext(display, ctx);
				return Result::Success;
			}
		}
		Log(Log.Information, "Failed to create context for any configs. Tried %d configs.", configs.size());

	}
	return Result::Success;

}

enum { retry_RemoveDebugBit, retry_DisableAA, retry_ReduceStencilBpp, retry_NoStencil, retry_StencilBpp, retry_ColorBpp, retry_reduceAlphaBpp, retry_NoAlpha, retry_DepthBpp, retry_DONE };
const char* retries_string[] = { "RemoveDebugBit", "DisableAA", "ReduceStencilBpp", "NoStencil", "ColorBpp", "ReduceAlphaBpp", "NoAlpha", "DepthBpp" };

void fix_attributes(DisplayAttributes& origAttr, DisplayAttributes& attr, uint32* retries, bool& debugBit)
{
	if (retries[retry_ColorBpp] == 1) //0:inactive 1:active, currently tested 2: active, unsure  3:active fixed
	{
		attr.redBits = 1;
		attr.greenBits = 1;
		attr.blueBits = 1;
	}
	else if (retries[retry_ColorBpp] == 0)
	{
		attr.redBits = origAttr.redBits;
		attr.greenBits = origAttr.greenBits;
		attr.blueBits = origAttr.blueBits;
	}
	if (!(retries[retry_reduceAlphaBpp] == 3) && !(retries[retry_NoAlpha] == 3)) // fixed. leave it alone..
	{
		if (retries[retry_reduceAlphaBpp] == 0 && retries[retry_NoAlpha] == 0) // reset.
		{
			attr.alphaBits = origAttr.alphaBits;
		} // mutually exclusiv

		if (retries[retry_reduceAlphaBpp] == 1) // test one
		{
			attr.alphaBits = 1;
		} // mutually exclusive
		if (retries[retry_NoAlpha] == 1) // test two
		{
			attr.alphaBits = 0;
		} // mutually exclusive
	}

	if (retries[retry_DepthBpp] == 1)
	{
		attr.depthBPP = 1;
	}
	else if (retries[retry_DepthBpp] == 0)
	{
		attr.depthBPP = origAttr.depthBPP;
	}
	if (!(retries[retry_ReduceStencilBpp] == 3) && !(retries[retry_NoStencil] == 3)) // fixed. leave it alone..
	{
		if (retries[retry_ReduceStencilBpp] == 0 && retries[retry_NoStencil] == 0) // reset.
		{
			attr.stencilBPP = origAttr.stencilBPP;
		} // mutually exclusiv

		if (retries[retry_ReduceStencilBpp] == 1) // test one
		{
			attr.stencilBPP = 1;
		} // mutually exclusive
		if (retries[retry_NoStencil] == 1) // test two
		{
			attr.stencilBPP = 0;
		} // mutually exclusive
	}

	if (retries[retry_DisableAA] == 1)
	{
		if (attr.aaSamples > 0)
		{
			// Reduce the anti-aliasing
			attr.aaSamples = attr.aaSamples >> 1;
		}
	}
	else if (retries[retry_DisableAA] == 0)
	{
		attr.aaSamples = origAttr.aaSamples;
	}
#ifdef DEBUG
	bool ORIG_DEBUG_BIT = true;
#else
	bool ORIG_DEBUG_BIT = false;
#endif
	if (retries[retry_RemoveDebugBit] == 1)
	{
		debugBit = false;
	}
	else if (retries[retry_RemoveDebugBit] == 0)
	{
		debugBit = ORIG_DEBUG_BIT;
	}
}


static inline Result initializeContext(const bool wantWindow, DisplayAttributes& original_attributes, const NativePlatformHandles& handles, EGLConfig& config, Api graphicsapi)
{
	// Choose a config based on user supplied attributes
	std::vector<EGLConfig> configs;
	EGLint configAttributes[32];
	bool debugBit = 0;


	int requestedMajorVersion = -1;
	int requestedMinorVersion = -1;


	switch (graphicsapi)
	{
	case Api::OpenGLES2:
		requestedMajorVersion = 2;
		requestedMinorVersion = 0;
		break;
	case Api::OpenGLES3:
		requestedMajorVersion = 3;
		requestedMinorVersion = 0;
		break;
	case Api::OpenGLES31:
		requestedMajorVersion = 3;
		requestedMinorVersion = 1;
		break;
	default:
		break;
	}

	bool create_context_supported = egl::isEglExtensionSupported(handles->display, "EGL_KHR_create_context");
	if (create_context_supported) { Log(Log.Information, "EGL context creation: EGL_KHR_create_context supported..."); }
	else
	{
		Log(requestedMinorVersion ? Log.Warning : Log.Information, "EGL context creation: EGL_KHR_create_context not supported. Minor version will be discarded, and debug disabled.");
		requestedMinorVersion = 0;
	}

	Log(Log.Information, "Trying to get OpenGL ES version : %d.%d", requestedMajorVersion, requestedMinorVersion);

	bool context_priority_supported = egl::isEglExtensionSupported(handles->display, "EGL_IMG_context_priority");
	if (context_priority_supported)
	{
		switch (original_attributes.contextPriority)
		{
		case 0: Log(Log.Information, "EGL context creation: EGL_IMG_context_priority supported! Setting context LOW priority..."); break;
		case 1: Log(Log.Information, "EGL context creation: EGL_IMG_context_priority supported! Setting context MEDIUM priority..."); break;
		default: Log(Log.Information, "EGL context creation: EGL_IMG_context_priority supported! Setting context HIGH priority (default)..."); break;
		}
	}
	else
	{
		Log(Log.Information, "EGL context creation: EGL_IMG_context_priority not supported. Ignoring context Priority attribute.");
	}

	uint32 retries[retry_DONE] = {};
	// O = not tried, 1 = now trying, 2 = not sure, 3 = keep disabled.
	DisplayAttributes attributes = original_attributes;
#ifdef DEBUG
	debugBit = true;
#endif

	if (!debugBit) { retries[retry_RemoveDebugBit] = 4; }
	if (attributes.aaSamples == 0) { retries[retry_DisableAA] = 3; }
	if (attributes.alphaBits == 0) { retries[retry_reduceAlphaBpp] = 3; }
	if (attributes.alphaBits == 0) { retries[retry_NoAlpha] = 3; }
	if (attributes.stencilBPP == 0) { retries[retry_StencilBpp] = 3; }
	if (attributes.stencilBPP == 0) { retries[retry_NoStencil] = 3; }
	if (attributes.depthBPP == 0) { retries[retry_DepthBpp] = 3; }
	if (attributes.forceColorBPP) { retries[retry_ColorBpp] = 3; }

	for (;;)
	{
		unsigned int i = 0;
		Log(Log.Debug, "Attempting to create context with:\n");
		Log(Log.Debug, "\tDebugbit: %s", debugBit ? "true" : "false");
		Log(Log.Debug, "\tRedBits: %d", attributes.redBits);
		Log(Log.Debug, "\tGreenBits: %d", attributes.greenBits);
		Log(Log.Debug, "\tBlueBits: %d", attributes.blueBits);
		Log(Log.Debug, "\tAlphaBits: %d", attributes.alphaBits);
		Log(Log.Debug, "\tDepthBits: %d", attributes.depthBPP);
		Log(Log.Debug, "\tStencilBits: %d", attributes.stencilBPP);

		if (attributes.configID > 0)
		{
			configAttributes[i++] = EGL_CONFIG_ID;
			configAttributes[i++] = attributes.configID;
		}
		else
		{
			configAttributes[i++] = EGL_RED_SIZE;
			configAttributes[i++] = attributes.redBits;

			configAttributes[i++] = EGL_GREEN_SIZE;
			configAttributes[i++] = attributes.greenBits;

			configAttributes[i++] = EGL_BLUE_SIZE;
			configAttributes[i++] = attributes.blueBits;

			configAttributes[i++] = EGL_ALPHA_SIZE;
			configAttributes[i++] = attributes.alphaBits;

			// For OGLES clamp between 0 and 24
			attributes.depthBPP = std::min(attributes.depthBPP, 24u);

			configAttributes[i++] = EGL_DEPTH_SIZE;
			configAttributes[i++] = attributes.depthBPP;

			configAttributes[i++] = EGL_STENCIL_SIZE;
			configAttributes[i++] = attributes.stencilBPP;

			if (wantWindow)
			{
				configAttributes[i++] = EGL_SURFACE_TYPE;
				configAttributes[i++] = EGL_WINDOW_BIT;
			}

			switch (graphicsapi)
			{
			case Api::OpenGLES2: //GLES2
				Log(Log.Debug, "EGL context creation: Setting EGL_OPENGL_ES2_BIT");
				configAttributes[i++] = EGL_RENDERABLE_TYPE;
				configAttributes[i++] = EGL_OPENGL_ES2_BIT;
				break;
			case Api::OpenGLES3: //GLES2
			case Api::OpenGLES31: //GLES2
				Log(Log.Debug, "EGL context creation: EGL_OPENGL_ES3_BIT");
				configAttributes[i++] = EGL_RENDERABLE_TYPE;
				configAttributes[i++] = EGL_OPENGL_ES3_BIT_KHR;
				break;
			default:
				return Result::UnsupportedRequest;
				break;
			}

			// Append number of number of samples depending on AA samples value set
			if (attributes.aaSamples > 0)
			{
				Log(Log.Debug, "EGL context creation: EGL_SAMPLE_BUFFERS 1");
				Log(Log.Debug, "EGL context creation: EGL_SAMPLES %d", attributes.aaSamples);
				configAttributes[i++] = EGL_SAMPLE_BUFFERS;
				configAttributes[i++] = 1;
				configAttributes[i++] = EGL_SAMPLES;
				configAttributes[i++] = attributes.aaSamples;
			}
		}

		configAttributes[i] = EGL_NONE;

		EGLint  numConfigs;
		EGLint  configsSize;

		EGLint eglerror = egl::GetError();
		assertion(eglerror == EGL_SUCCESS, "initializeContext: egl error logged before choosing egl config");
		eglerror = egl::ChooseConfig(handles->display, configAttributes, NULL, 0, &configsSize);
		assertion(eglerror == EGL_TRUE, "initializeContext: EGL config returned a value that was not EGL_TRUE");
		eglerror = egl::GetError();
		assertion(eglerror == EGL_SUCCESS, "initializeContext: EGL choose config raised EGL error");

		if (attributes.forceColorBPP)
		{
			if (configsSize == 0) { return Result::UnknownError; }
		}
		else
		{
			if (configsSize > 1) { configsSize = 1; }
		}
		numConfigs = configsSize;
		if (configsSize)
		{

			configs.resize(configsSize);

			if (egl::ChooseConfig(handles->display, configAttributes, configs.data(), configsSize, &numConfigs) != EGL_TRUE)
			{
				Log("EGL context creation: initializeContext Error choosing egl config. %x.    Expected number of configs: %d    Actual: %d.", egl::GetError(), numConfigs, configsSize);
				return Result::UnsupportedRequest;
			}
		}
		Log(Log.Information, "EGL context creation: Number of EGL Configs found: %d", configsSize);

		if (numConfigs > 0)
		{
			EGLint configIdx = 0;

			if (attributes.forceColorBPP)
			{
				Log(Log.Information, "EGL context creation: Trying to find a for forced BPP compatible context support...");
				EGLint value;

				for (; configIdx < configsSize; ++configIdx)
				{
					if ((egl::GetConfigAttrib(handles->display, configs[configIdx], EGL_RED_SIZE, &value)
					     && value == static_cast<EGLint>(original_attributes.redBits))
					    && (egl::GetConfigAttrib(handles->display, configs[configIdx], EGL_GREEN_SIZE, &value)
					        && value == static_cast<EGLint>(original_attributes.greenBits))
					    && (egl::GetConfigAttrib(handles->display, configs[configIdx], EGL_BLUE_SIZE, &value)
					        && value == static_cast<EGLint>(original_attributes.blueBits))
					    && (egl::GetConfigAttrib(handles->display, configs[configIdx], EGL_ALPHA_SIZE, &value)
					        && value == static_cast<EGLint>(original_attributes.alphaBits))
					   )
					{
						break;
					}
				}
			}

			config = configs[configIdx];

			EGLint contextAttributes[32];
			i = 0;


#if defined(EGL_CONTEXT_MAJOR_VERSION_KHR)
			if (create_context_supported)
			{
				contextAttributes[i++] = EGL_CONTEXT_MAJOR_VERSION_KHR;
				contextAttributes[i++] = requestedMajorVersion;
				contextAttributes[i++] = EGL_CONTEXT_MINOR_VERSION_KHR;
				contextAttributes[i++] = requestedMinorVersion;
#ifdef DEBUG
				if (debugBit)
				{
					contextAttributes[i++] = EGL_CONTEXT_FLAGS_KHR;
					contextAttributes[i++] = EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;
				}
#endif
			}
			else
#endif
			{
				contextAttributes[i++] = EGL_CONTEXT_CLIENT_VERSION;
				contextAttributes[i++] = requestedMajorVersion;
			}

			if (context_priority_supported)
			{
				contextAttributes[i++] = EGL_CONTEXT_PRIORITY_LEVEL_IMG;

				switch (attributes.contextPriority)
				{
				case 0: contextAttributes[i++] = EGL_CONTEXT_PRIORITY_LOW_IMG; break;
				case 1: contextAttributes[i++] = EGL_CONTEXT_PRIORITY_MEDIUM_IMG; break;
				default: contextAttributes[i++] = EGL_CONTEXT_PRIORITY_HIGH_IMG; break;
				}
			}
			contextAttributes[i] = EGL_NONE;

			Log(Log.Information, "Creating EGL context...");
			//Create the context
			handles->context = egl::CreateContext(handles->display, config, NULL, contextAttributes);

			//// SUCCESS -- FUNCTION SUCCESSFUL EXIT POINT
			if (handles->context != EGL_NO_CONTEXT)
			{
				Log(Log.Debug, "EGL context created. Will now check if any attributes were being debugged, and try to roll back unnecessary changes.");
				bool is_final = true;
				for (uint32 retrybit = 0; retrybit < retry_DONE && is_final; ++retrybit)
				{
					if (retries[retrybit] == 1) // If was "now trying"...
					{
						Log(Log.Debug, "Current testing bit was %s. Will mark this as 'definitely not supported'(3), clear all 'tentative'(2) bits if present. If no tentative bits were found, will succeed!", retries_string[retrybit]);
						retries[retrybit] = 3; // Fix that we definitely need that.
						// Now, we try and enable all the rest of them...
						for (uint32 retrybit_2 = 0; retrybit_2 < retry_DONE; ++retrybit_2)
						{
							if (retries[retrybit_2] == 2) // Bit is: Tried disabling it, not sure yet
							{
								is_final = false;
								retries[retrybit_2] = 0; // Reset it!
							}
						}

					}
				}

				if (!is_final)
				{
					Log(Log.Debug, "Found EGL attribute retry bits to attempt reset. Will now test without the disabled attributes.");
					fix_attributes(original_attributes, attributes, retries, debugBit);
					continue;
				}



				Log(Log.Debug, "EGL context successfully created! Updating Config Attributes to reflect actual context parameters...");

				// Update the attributes to the config's
				egl::GetConfigAttrib(handles->display, config, EGL_RED_SIZE, (EGLint*)&attributes.redBits);
				egl::GetConfigAttrib(handles->display, config, EGL_GREEN_SIZE, (EGLint*)&attributes.greenBits);
				egl::GetConfigAttrib(handles->display, config, EGL_BLUE_SIZE, (EGLint*)&attributes.blueBits);
				egl::GetConfigAttrib(handles->display, config, EGL_ALPHA_SIZE, (EGLint*)&attributes.alphaBits);
				egl::GetConfigAttrib(handles->display, config, EGL_DEPTH_SIZE, (EGLint*)&attributes.depthBPP);
				egl::GetConfigAttrib(handles->display, config, EGL_STENCIL_SIZE, (EGLint*)&attributes.stencilBPP);
				Log(Log.Information, "EGL Initialized Successfully");
				platform::logEGLConfiguration(attributes);
				return Result::Success;
			}

			// clear the EGL error
			eglerror = egl::GetError();
			if (eglerror != EGL_SUCCESS)
			{
				Log(Log.Debug, "Context not created yet. Clearing EGL errors.");
			}
		} //eglChooseConfif failed.

		//// FAILURE ////
		if (attributes.configID > 0)
		{
			Log(Log.Error, "Failed to create egl::Context with config ID %i", attributes.configID);
			return Result::UnknownError;
		}

		Log(Log.Debug, "Context creation failed - Will change EGL attributes and retry.");

		// FAILURE
		bool must_retry = false;
		for (uint32 retry_bit = 0; retry_bit < retry_DONE; ++retry_bit)
		{
			if (retries[retry_bit] == 1) //Was the current one being checked out?
			{
				Log(Log.Information, "Setting bit %s as 'unsure'(2), since the context creation still failed.", retries_string[retry_bit]);
				retries[retry_bit] = 2; //Mark it as "unsure"
				break;
			}
		}
		for (uint32 retry_bit = 0; retry_bit < retry_DONE && !must_retry; ++retry_bit)
		{
			if (retries[retry_bit] == 0) //Found one we didn't test?
			{
				Log(Log.Information, "Setting bit %s as 'currently testing'(1).", retries_string[retry_bit]);
				retries[retry_bit] = 1; //Test it...
				must_retry = true;
			}
		}

		if (must_retry)
		{
			fix_attributes(original_attributes, attributes, retries, debugBit);
		}
		else
		{
			// Nothing else we can remove, fail
			Log(Log.Critical, "Failed to create egl::Context. Unknown reason of failure. Last error logged is: %s", platform::eglErrorToStr(egl::GetError()));

			return Result::UnknownError;
		}
	}

}


static inline Result preInitialize(OSManager& mgr, NativePlatformHandles& handles)
{
	if (!handles.get())
	{
		handles.reset(new NativePlatformHandles::element_type);
	}

	// Associate the display with EGL.
	handles->display = egl::GetDisplay(reinterpret_cast<EGLNativeDisplayType>(mgr.getDisplay()));

	if (handles->display == EGL_NO_DISPLAY)
	{
		handles->display = egl::GetDisplay(static_cast<EGLNativeDisplayType>(EGL_DEFAULT_DISPLAY));
	}


	if (handles->display == EGL_NO_DISPLAY)
	{
		return Result::UnknownError;
	}

	// Initialize the display
	if (egl::Initialize(handles->display, NULL, NULL) != EGL_TRUE)
	{
		return Result::UnknownError;
	}

	//  // Bind the correct API
	int result = EGL_FALSE;

	result = egl::BindAPI(EGL_OPENGL_ES_API);

	if (result != EGL_TRUE)
	{
		return Result::UnsupportedRequest;
	}
	return Result::Success;
}

/*This function assumes that the osManager's getDisplay() and getWindow() types are one and the same with NativePlatformHandles::NativeDisplay and NativePlatformHandles::NativeWindow.*/
Result PlatformContext::init()
{
	if (m_initialized)
	{
		return Result::AlreadyInitialized;
	}

	Result result;
	if (!m_preInitialized)
	{
		result = preInitialize(m_OSManager, m_platformContextHandles);
		if (result != Result::Success)
		{
			return result;
		}
		m_preInitialized = true;

		if (m_OSManager.getApiTypeRequired() == Api::Unspecified)
		{
			populateMaxApiVersion();
		}
		else
		{
			m_maxApiVersion = m_OSManager.getApiTypeRequired();
		}
	}

	if (m_OSManager.getApiTypeRequired() == Api::Unspecified)
	{
		if (m_OSManager.getMinApiTypeRequired() == Api::Unspecified)
		{
			apiType = getMaxApiVersion();
			m_OSManager.setApiTypeRequired(apiType);
			Log(Log.Information, "Unspecified target API -- Setting to max API level : %s", apiName(apiType));
		}
		else
		{
			apiType = std::max(m_OSManager.getMinApiTypeRequired(), getMaxApiVersion());
			Log(Log.Information, "Requested minimum API level : %s. Will actually create %s since it is supported.",
			    apiName(m_OSManager.getMinApiTypeRequired()), apiName(getMaxApiVersion()));
			m_OSManager.setApiTypeRequired(apiType);
		}
	}
	else
	{
		Log(Log.Information, "Forcing specific API level: %s", apiName(apiType = m_OSManager.getApiTypeRequired()));
	}

	if (apiType > getMaxApiVersion())
	{
		Log(Log.Error, "================================================================================\n"
		    "API level requested [%s] was not supported. Max supported API level on this device is [%s]\n"
		    "**** APPLICATION WILL EXIT ****\n"
		    "================================================================================",
		    apiName(apiType), apiName(getMaxApiVersion()));
		return Result::UnsupportedRequest;
	}

	EGLConfig config;
	result = initializeContext(true, m_OSManager.getDisplayAttributes(), m_platformContextHandles, config, apiType);

	if (result != Result::Success)
	{
		return result;
	}



	EGLint eglattribs[] = { EGL_NONE, EGL_NONE, EGL_NONE };

	if (m_OSManager.getDisplayAttributes().frameBufferSrgb)
	{
		bool isSrgbSupported = egl::isEglExtensionSupported(m_platformContextHandles->display, "EGL_KHR_gl_colorspace");
		if (isSrgbSupported)
		{
			eglattribs[0] = EGL_COLORSPACE;
			eglattribs[1] = EGL_COLORSPACE_sRGB;
		}
		else
		{
			Log(Log.Warning, "sRGB window backbuffer requested, but EGL_KHR_gl_colorspace is not supported. Creating linear RGB backbuffer.");
			m_OSManager.getDisplayAttributes().frameBufferSrgb = false;
		}
	}

	m_platformContextHandles->drawSurface = m_platformContextHandles->readSurface = egl::CreateWindowSurface(
	    m_platformContextHandles->display, config, reinterpret_cast<EGLNativeWindowType>(m_OSManager.getWindow()), eglattribs);
	if (m_platformContextHandles->drawSurface == EGL_NO_SURFACE)
	{
		Log(Log.Error, "Context creation failed\n");
		return Result::InvalidArgument;
	}


	// Update the attributes to the surface's
	egl::QuerySurface(m_platformContextHandles->display, m_platformContextHandles->drawSurface, EGL_WIDTH,
	                  (EGLint*)&m_OSManager.getDisplayAttributes().width);
	egl::QuerySurface(m_platformContextHandles->display, m_platformContextHandles->drawSurface, EGL_HEIGHT,
	                  (EGLint*)&m_OSManager.getDisplayAttributes().height);

	m_swapInterval = 1;
	switch (m_OSManager.getDisplayAttributes().vsyncMode)
	{
	case VsyncMode::Half: m_swapInterval = 2; break;
	case VsyncMode::Mailbox: case VsyncMode::Off: m_swapInterval = 0; break;
	case VsyncMode::Relaxed: m_swapInterval = -1; break;
	default: break;
	}
	m_initialized = true;
	return Result::Success;
}

Api PlatformContext::getMaxApiVersion()
{

	if (!m_preInitialized)
	{
		if (preInitialize(m_OSManager, m_platformContextHandles) != Result::Success)
		{
			Log(Log.Critical, "Could not query max API version. Error while initialising OpenGL ES");
			return Api::Unspecified;
		}
		m_preInitialized = true;
		populateMaxApiVersion();
	}

	return m_maxApiVersion;
}

void PlatformContext::populateMaxApiVersion()
{
	m_maxApiVersion = Api::Unspecified;
	Api graphicsapi = Api::OpenGLESMaxVersion;
	bool supported;
	Result result;
	while (graphicsapi > Api::Unspecified)
	{
		const char* esversion = (graphicsapi == Api::OpenGLES31 ? "3.1" : graphicsapi == Api::OpenGLES3 ? "3.0" : graphicsapi == Api::OpenGLES2 ?
		                         "2.0" : "UNKNOWN_VERSION");
		result = isGlesVersionSupported(m_platformContextHandles->display, m_OSManager.getDisplayAttributes(), graphicsapi, supported);

		if (result == Result::Success)
		{
			if (supported && graphicsapi == Api::OpenGLES31)
			{
				///////////////////////////  WORKAROUND FOR SOME DEBUG DRIVERS ///////////////////////////
				if (isOpenGLES31NotSupported_Workaround)
				{
					supported = false;
					Log(Log.Information, "Activating workaround - OpenGL ES 3.1 support was reported, but is not present.");
				}
				/////////////////////////// /WORKAROUND FOR SOME DEBUG DRIVERS //////////////////////////
			}

			if (supported)
			{
				m_maxApiVersion = graphicsapi;
				Log(Log.Information, "Maximum API level detected: OpenGL ES %s", esversion);
				return;
			}
			else
			{
				Log(Log.Information, "OpenGL ES %s NOT supported. Trying lower version...", esversion);
			}
		}
		else
		{
			Log("Error detected while testing OpenGL ES version %s for compatibility. Trying lower version", esversion);
		}
		graphicsapi = (Api)((int)graphicsapi - 1);
	}
	Log(Log.Critical, "=== FATAL: COULD NOT FIND COMPATIBILITY WITH ANY OPENGL ES VERSION ===");

}

bool PlatformContext::isApiSupported(Api apiLevel)
{

	if (!m_preInitialized)
	{
		if (preInitialize(m_OSManager, m_platformContextHandles) != Result::Success)
		{
			return false;
		}
		m_preInitialized = true;
		populateMaxApiVersion();
	}
	return apiLevel >= m_maxApiVersion;
}

bool PlatformContext::makeCurrent()
{
	bool result = (egl::MakeCurrent(m_platformContextHandles->display, m_platformContextHandles->drawSurface,
	                                m_platformContextHandles->drawSurface, m_platformContextHandles->context) == EGL_TRUE);
#if !defined(__ANDROID__)&&!defined(TARGET_OS_IPHONE)
	if (m_swapInterval != -2)
	{
		// Set our swap interval which affects the current draw surface
		egl::SwapInterval(m_platformContextHandles->display, m_swapInterval);
		m_swapInterval = -2;
	}
#endif
	return result;
}

bool PlatformContext::presentBackbuffer()
{
	return (egl::SwapBuffers(m_platformContextHandles->display, m_platformContextHandles->drawSurface) == EGL_TRUE);
}

string PlatformContext::getInfo()
{
	string out, tmp;
	EGLint i32Values[5];

	out.reserve(2048);
	tmp.reserve(1024);


	out.append("\nEGL:\n");

	tmp = strings::createFormatted("\tVendor:   %hs\n", (const char*)egl::QueryString(m_platformContextHandles->display,
	                               EGL_VENDOR));
	out.append(tmp);

	tmp = strings::createFormatted("\tVersion:  %hs\n", (const char*)egl::QueryString(m_platformContextHandles->display,
	                               EGL_VERSION));
	out.append(tmp);

	tmp = strings::createFormatted("\tExtensions:  %hs\n",
	                               (const char*)egl::QueryString(m_platformContextHandles->display,
	                                   EGL_EXTENSIONS));
	out.append(tmp);

	if (egl::QueryContext(m_platformContextHandles->display, m_platformContextHandles->context, EGL_CONTEXT_PRIORITY_LEVEL_IMG, &i32Values[0]))
	{
		switch (i32Values[0])
		{
		case EGL_CONTEXT_PRIORITY_HIGH_IMG:
			out.append("\tContext priority: High\n");
			break;
		case EGL_CONTEXT_PRIORITY_MEDIUM_IMG:
			out.append("\tContext priority: Medium\n");
			break;
		case EGL_CONTEXT_PRIORITY_LOW_IMG:
			out.append("\tContext priority: Low\n");
			break;
		default:
			out.append("\tContext priority: Unrecognised.\n");
		}
	}
	else
	{
		egl::GetError(); // Clear error
		out.append("\tContext priority: Unsupported\n");
	}

#if defined(EGL_VERSION_1_2)
	tmp = strings::createFormatted("\tClient APIs:  %hs\n",
	                               (const char*)egl::QueryString(m_platformContextHandles->display,
	                                   EGL_CLIENT_APIS));
	out.append(tmp);
#endif

	egl::QuerySurface(m_platformContextHandles->display, m_platformContextHandles->drawSurface, EGL_WIDTH, &i32Values[0]);
	tmp = strings::createFormatted("\nSurface Width:  %i\n", i32Values[0]);
	out.append(tmp);

	egl::QuerySurface(m_platformContextHandles->display, m_platformContextHandles->drawSurface, EGL_HEIGHT, &i32Values[0]);
	tmp = strings::createFormatted("Surface Height: %i\n\n", i32Values[0]);
	out.append(tmp);

	// EGLSurface details

	// Get current config
	EGLConfig config;
	egl::QueryContext(m_platformContextHandles->display, m_platformContextHandles->context, EGL_CONFIG_ID, &i32Values[0]);
	const EGLint attributes[] = { EGL_CONFIG_ID, i32Values[0], EGL_NONE };
	egl::ChooseConfig(m_platformContextHandles->display, attributes, &config, 1, &i32Values[1]);

	out.append("EGL Surface:\n");

	tmp = strings::createFormatted("\tConfig ID:\t%i\n", i32Values[0]);
	out.append(tmp);

	// Color buffer
	egl::GetConfigAttrib(m_platformContextHandles->display, config, EGL_BUFFER_SIZE, &i32Values[0]);
	egl::GetConfigAttrib(m_platformContextHandles->display, config, EGL_RED_SIZE, &i32Values[1]);
	egl::GetConfigAttrib(m_platformContextHandles->display, config, EGL_GREEN_SIZE, &i32Values[2]);
	egl::GetConfigAttrib(m_platformContextHandles->display, config, EGL_BLUE_SIZE, &i32Values[3]);
	egl::GetConfigAttrib(m_platformContextHandles->display, config, EGL_ALPHA_SIZE, &i32Values[4]);
	tmp = strings::createFormatted("\tColor Buffer:  %i bits (R%i G%i B%i A%i)\n", i32Values[0], i32Values[1],
	                               i32Values[2], i32Values[3], i32Values[4]);
	out.append(tmp);

	// Depth buffer
	egl::GetConfigAttrib(m_platformContextHandles->display, config, EGL_DEPTH_SIZE, &i32Values[0]);
	tmp = strings::createFormatted("\tDepth Buffer:   %i bits\n", i32Values[0]);
	out.append(tmp);

	// Stencil Buffer
	egl::GetConfigAttrib(m_platformContextHandles->display, config, EGL_STENCIL_SIZE, &i32Values[0]);
	tmp = strings::createFormatted("\tStencil Buffer: %i bits\n", i32Values[0]);
	out.append(tmp);

	// EGL surface bits support
	egl::GetConfigAttrib(m_platformContextHandles->display, config, EGL_SURFACE_TYPE, &i32Values[0]);
	tmp = strings::createFormatted("\tSurface type:   %hs%hs%hs\n", i32Values[0] & EGL_WINDOW_BIT ? "WINDOW " : "",
	                               i32Values[1] & EGL_PBUFFER_BIT ? "PBUFFER " : "",
	                               i32Values[2] & EGL_PIXMAP_BIT ? "PIXMAP " : "");
	out.append(tmp);

	// EGL renderable type
#if defined(EGL_VERSION_1_2)
	egl::GetConfigAttrib(m_platformContextHandles->display, config, EGL_RENDERABLE_TYPE, &i32Values[0]);
	tmp = strings::createFormatted("\tRenderable type: %hs%hs%hs%hs\n",
	                               i32Values[0] & EGL_OPENVG_BIT ? "OPENVG " : "",
	                               i32Values[0] & EGL_OPENGL_ES_BIT ? "OPENGL_ES " : "",
#if defined(EGL_OPENGL_BIT)
	                               i32Values[0] & EGL_OPENGL_BIT ? "OPENGL " : "",
#endif
	                               i32Values[0] & EGL_OPENGL_ES2_BIT ? "OPENGL_ES2 " : "");
	out.append(tmp);
#endif

	egl::GetConfigAttrib(m_platformContextHandles->display, config, EGL_SAMPLE_BUFFERS, &i32Values[0]);
	egl::GetConfigAttrib(m_platformContextHandles->display, config, EGL_SAMPLES, &i32Values[1]);
	tmp = strings::createFormatted("\tSample buffer No.: %i\n", i32Values[0]);
	out.append(tmp);
	tmp = strings::createFormatted("\tSamples per pixel: %i", i32Values[1]);
	out.append(tmp);

	return out;
}


}
}



namespace pvr {
//Creates an instance of a graphics context
std::auto_ptr<IPlatformContext> createNativePlatformContext(OSManager& mgr)
{
	if (!egl::initEgl()) { return std::auto_ptr<IPlatformContext>(); }
	eglext::initEglExt();
	return std::auto_ptr<IPlatformContext>(new platform::PlatformContext(mgr));
}
}
//!\endcond