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

#ifndef EGL_CONTEXT_LOST_IMG
/*! Extended error code EGL_CONTEXT_LOST_IMG generated when power management event has occurred. */
#define EGL_CONTEXT_LOST_IMG				0x300E
#endif

#ifndef EGL_CONTEXT_PRIORITY_LEVEL_IMG
/*! An extensions added to the list of attributes for the context to give it a priority hint */
#define EGL_CONTEXT_PRIORITY_LEVEL_IMG		0x3100
/*! Request the context is created with high priority */
#define EGL_CONTEXT_PRIORITY_HIGH_IMG		0x3101
/*! Request the context is created with medium priority */
#define EGL_CONTEXT_PRIORITY_MEDIUM_IMG		0x3102
/*! Request the context is created with low priority */
#define EGL_CONTEXT_PRIORITY_LOW_IMG		0x3103
#endif

namespace pvr {
using std::string;
namespace system {

// File local function to return the global context store.
inline static std::map<int32, system::PlatformContext*>& getContextStore()
{
	static std::map<int32, system::PlatformContext*> contextStore;
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
	Log(Log.Debug, "EGL Configuration");
	Log(Log.Debug, "\tRedBits: %d", attributes.redBits);
	Log(Log.Debug, "\tGreenBits: %d", attributes.greenBits);
	Log(Log.Debug, "\tBlueBits: %d", attributes.blueBits);
	Log(Log.Debug, "\tAlphaBits: %d", attributes.alphaBits);
	Log(Log.Debug, "\taaSamples: %d", attributes.aaSamples);
	Log(Log.Debug, "\tFullScreen: %s", (attributes.fullscreen ? "true" : "false"));
}

glm::uint32 PlatformContext::getSwapChainLength() const { return 1; }


void PlatformContext::release()
{
	if (m_initialized)
	{
		// Check the current context/surface/display. If they are equal to the handles in this class, remove them from the current context
		if (m_platformContextHandles->display == egl::GetCurrentDisplay() &&
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

static inline EGLContext getContextForConfig(EGLDisplay display, EGLConfig config, Api::Enum graphicsapi)
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
	assertion(requestedMajorVersion && (requestedMinorVersion >= 0), "Unsupported major-minor version");

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

static inline Result::Enum isGlesVersionSupported(EGLDisplay display, Api::Enum graphicsapi, bool& isSupported)
{
#if defined(TARGET_OS_MAC)
	/* Max Api supported on OSX is OGLES3*/
	if (graphicsapi > pvr::Api::OpenGLES3) {		return Result::UnsupportedRequest; }
#endif

	isSupported = false;
	std::vector<EGLConfig> configs;
	EGLint configAttributes[5];
	unsigned int i = 0;


	configAttributes[i++] = EGL_SURFACE_TYPE;
	configAttributes[i++] = EGL_WINDOW_BIT;

	switch (graphicsapi)
	{
	case Api::OpenGLES2: //GLES2
		Log(Log.Verbose, "EglPlatformContext.cpp: isGlesVersionSupported: Setting EGL_OPENGL_ES2_BIT");
		configAttributes[i++] = EGL_RENDERABLE_TYPE;
		configAttributes[i++] = EGL_OPENGL_ES2_BIT;
		break;
	case Api::OpenGLES3: //GLES2
	case Api::OpenGLES31: //GLES2
		Log(Log.Verbose, "EglPlatformContext.cpp: isGlesVersionSupported: Setting EGL_OPENGL_ES3_BIT_KHR");
		configAttributes[i++] = EGL_RENDERABLE_TYPE;
		configAttributes[i++] = EGL_OPENGL_ES3_BIT_KHR;
		break;
	default:
		return Result::UnknownError;
		break;
	}

	configAttributes[i] = EGL_NONE;

	EGLint	numConfigs;
	EGLint	configsSize;


	// Find out how many configs there are in total that match our criteria
	if (!egl::ChooseConfig(display, configAttributes, NULL, 0, &configsSize))
	{
		Log("EglPlatformContext.cpp: getMaxEglVersion: eglChooseConfig error");
		return Result::UnknownError;
	}
	Log(Log.Information, "EglPlatformContext.cpp: isGlesVersionSupported: number of configurations found for ES version [%s] was [%d]", Api::getApiName(graphicsapi), configsSize);
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

		for (size_t i = 0; i != configs.size(); ++i)
		{

			Log(Log.Verbose, "Trying to create context for config #%d...", i);
			EGLContext ctx;
			if ((ctx = getContextForConfig(display, configs[i], graphicsapi)) != EGL_NO_CONTEXT)
			{
				Log(Log.Verbose, "SUCCESS creating context! Reporting success.");
				isSupported = true;
				egl::DestroyContext(display, ctx);
				return Result::Success;
			}
			Log(Log.Verbose, "Failed to create context for config #%d.", i);
		}

		// Choose a config based on user supplied attributes
	}
	return Result::Success;

}

static inline Result::Enum initializeContext(const bool wantWindow, DisplayAttributes& attributes, const NativePlatformHandles& handles, EGLConfig& config, Api::Enum graphicsapi)
{
	// Choose a config based on user supplied attributes
	std::vector<EGLConfig> configs;
	EGLint configAttributes[32];
	unsigned int i = 0;
	bool debugBit = 0;
#ifdef DEBUG
	debugBit = true;
#endif

	for (;;)
	{
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
			else
			{
				Log(Log.Debug, "EGL context creation: EGL_SAMPLE_BUFFERS 0");
				configAttributes[i++] = EGL_SAMPLE_BUFFERS;
				configAttributes[i++] = 0;
			}
		}

		configAttributes[i] = EGL_NONE;

		EGLint	numConfigs;
		EGLint	configsSize;

		if (attributes.forceColorBPP)
		{
			// Find out how many configs there are in total that match our criteria
			if (!egl::ChooseConfig(handles->display, configAttributes, NULL, 0, &configsSize) || configsSize == 0)
			{
				return Result::UnknownError;
			}
		}
		else
		{
			configsSize = 1;
		}

		configs.resize(configsSize);

		if (egl::ChooseConfig(handles->display, configAttributes, configs.data(), configsSize, &numConfigs) != EGL_TRUE
		        || numConfigs != configsSize)
		{
			Log("EGL context creation: initializeContext Error choosing egl config. %x",
			    egl::GetError());
			return Result::UnsupportedRequest;
		}
		Log(Log.Information, "EGL context creation: Number of EGL Configs found: %d", configsSize);
		EGLint configIdx = 0;

		if (attributes.forceColorBPP)
		{
			Log(Log.Debug, "EGL context creation: Trying to find a for forced BPP compatible context support...");
			EGLint value;

			for (; configIdx < configsSize; ++configIdx)
			{
				if ((egl::GetConfigAttrib(handles->display, configs[configIdx], EGL_RED_SIZE, &value)
				        && value == static_cast<EGLint>(attributes.redBits))
				        && (egl::GetConfigAttrib(handles->display, configs[configIdx], EGL_GREEN_SIZE, &value)
				            && value == static_cast<EGLint>(attributes.greenBits))
				        && (egl::GetConfigAttrib(handles->display, configs[configIdx], EGL_BLUE_SIZE, &value)
				            && value == static_cast<EGLint>(attributes.blueBits))
				        && (egl::GetConfigAttrib(handles->display, configs[configIdx], EGL_ALPHA_SIZE, &value)
				            && value == static_cast<EGLint>(attributes.alphaBits))
				   )
				{
					break;
				}
			}
		}

		config = configs[configIdx];

		EGLint contextAttributes[32];
		i = 0;

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
		assertion(requestedMajorVersion && (requestedMinorVersion >= 0), "Unsupported major-minor version");
		Log(Log.Information, "EGL context creation: Trying to get OpenGL ES version : %d.%d", requestedMajorVersion, requestedMinorVersion);


#if defined(EGL_CONTEXT_MAJOR_VERSION_KHR)
		if (egl::isEglExtensionSupported(handles->display, "EGL_KHR_create_context"))
		{
			Log(Log.Information, "EGL context creation: EGL_KHR_create_context supported...");
			contextAttributes[i++] = EGL_CONTEXT_MAJOR_VERSION_KHR;
			contextAttributes[i++] = requestedMajorVersion;
			contextAttributes[i++] = EGL_CONTEXT_MINOR_VERSION_KHR;
			contextAttributes[i++] = requestedMinorVersion;
#ifdef DEBUG
			if (debugBit)
			{
				Log(Log.Information, "EGL context creation: Trying to set EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR. Since no direct query is enabled, failure will need to be tested...");
				contextAttributes[i++] = EGL_CONTEXT_FLAGS_KHR;
				contextAttributes[i++] = EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;
			}
#endif
		}
		else
#endif
		{
			Log(requestedMinorVersion ? Log.Warning : Log.Information, "EGL context creation: EGL_KHR_create_context not supported. Minor version will be discarded, and debug disabled.");
			contextAttributes[i++] = EGL_CONTEXT_CLIENT_VERSION;
			contextAttributes[i++] = requestedMajorVersion;
		}

		if (egl::isEglExtensionSupported(handles->display, "EGL_IMG_context_priority"))
		{
			contextAttributes[i++] = EGL_CONTEXT_PRIORITY_LEVEL_IMG;

			switch (attributes.contextPriority)
			{
			case 0: contextAttributes[i++] = EGL_CONTEXT_PRIORITY_LOW_IMG;
				Log(Log.Information, "EGL context creation: EGL_IMG_context_priority supported! Setting context LOW priority...");
				break;
			case 1: contextAttributes[i++] = EGL_CONTEXT_PRIORITY_MEDIUM_IMG;
				Log(Log.Information, "EGL context creation: EGL_IMG_context_priority supported! Setting context MEDIUM priority...");
				break;
			default: contextAttributes[i++] = EGL_CONTEXT_PRIORITY_HIGH_IMG;
				Log(Log.Information, "EGL context creation: EGL_IMG_context_priority supported! Setting context HIGH priority (default)...");
				break;
			}
		}
		else
		{
			Log(Log.Information, "EGL context creation: EGL_IMG_context_priority not supported. Ignoring context Priority attribute.");
		}

		contextAttributes[i] = EGL_NONE;

		Log(Log.Verbose, "Creating EGL context...");
		//Create the context
		handles->context = egl::CreateContext(handles->display, config, NULL, contextAttributes);

		//// SUCCESS -- FUNCTION SUCCESSFUL EXIT POINT
		if (handles->context != EGL_NO_CONTEXT)
		{
			Log(Log.Verbose, "EGL context created. Updating Config Attributes to reflect actual context parameters...");

			// Update the attributes to the config's
			egl::GetConfigAttrib(handles->display, config, EGL_RED_SIZE, (EGLint*)&attributes.redBits);
			egl::GetConfigAttrib(handles->display, config, EGL_GREEN_SIZE, (EGLint*)&attributes.greenBits);
			egl::GetConfigAttrib(handles->display, config, EGL_BLUE_SIZE, (EGLint*)&attributes.blueBits);
			egl::GetConfigAttrib(handles->display, config, EGL_ALPHA_SIZE, (EGLint*)&attributes.alphaBits);
			egl::GetConfigAttrib(handles->display, config, EGL_DEPTH_SIZE, (EGLint*)&attributes.depthBPP);
			egl::GetConfigAttrib(handles->display, config, EGL_STENCIL_SIZE, (EGLint*)&attributes.stencilBPP);
			Log(Log.Verbose, "EGL Initialized Successfully");
			system::logEGLConfiguration(attributes);
			return Result::Success;
		}

		//// FAILURE ////
		if (attributes.configID > 0)
		{
			Log(Log.Error, "Failed to create egl::Context with config ID %i", attributes.configID);
			return Result::UnknownError;
		}

		// We've failed to create a context, a slight tweak of the config may allow us to salvage things
		//First, try without the debug bit, if it was se...
		if (debugBit)
		{
			debugBit = 0;
			Log(Log.Warning, "Failed to create egl::Context - possible that EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR was responsible. Removing debug bit and retrying...");
			continue;
		}
		// We've failed to create a context, a slight tweak of the config may allow us to salvage things
		else if (attributes.aaSamples > 0)
		{
			// Reduce the anti-aliasing
			attributes.aaSamples = attributes.aaSamples >> 1;
			Log(Log.Error, "Failed to create egl::Context. Reducing the number of anti-aliasing samples down to %i and trying again.",
			    attributes.aaSamples);
			continue;
		}
		else if (attributes.stencilBPP > 0)
		{
			attributes.stencilBPP = 0;
			Log(Log.Critical, "Failed to create egl::Context. Is it possible that a Stencil buffer is not supported?");
			return Result::UnknownError;
		}
		else
		{
			// Nothing else we can remove, fail
			Log(Log.Critical, "Failed to create egl::Context. Unknown reason of failure. Error is: %s", system::eglErrorToStr(egl::GetError()));

			return Result::UnknownError;
		}
	}

}


static inline Result::Enum preInitialize(OSManager& mgr, NativePlatformHandles& handles)
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

	//	// Bind the correct API
	int result = EGL_FALSE;

	result = egl::BindAPI(EGL_OPENGL_ES_API);

	if (result != EGL_TRUE)
	{
		return Result::UnsupportedRequest;
	}
	return Result::Success;
}

/*This function assumes that the osManager's getDisplay() and getWindow() types are one and the same with NativePlatformHandles::NativeDisplay and NativePlatformHandles::NativeWindow.*/
Result::Enum PlatformContext::init()
{
	if (m_initialized)
	{
		return Result::AlreadyInitialized;
	}

	Result::Enum result;
	if (!m_preInitialized)
	{
		result = preInitialize(m_OSManager, m_platformContextHandles);
		if (result != Result::Success)
		{
			return result;
		}
		m_preInitialized = true;
		populateMaxApiVersion();
	}

	if (m_OSManager.getApiTypeRequired() == Api::Unspecified)
	{
		if (m_OSManager.getMinApiTypeRequired() == Api::Unspecified)
		{
			apiType = getMaxApiVersion();
			m_OSManager.setApiTypeRequired(apiType);
			Log(Log.Information, "Unspecified target API -- Setting to max API level : %s", Api::getApiName(apiType));
		}
		else
		{
			apiType = std::max(m_OSManager.getMinApiTypeRequired(), getMaxApiVersion());
			Log(Log.Information, "Requested minimum API level : %s. Will actually create %s since it is supported.",
			    Api::getApiName(m_OSManager.getMinApiTypeRequired()), Api::getApiName(getMaxApiVersion()));
			m_OSManager.setApiTypeRequired(apiType);
		}
	}
	else
	{
		Log(Log.Information, "Forcing specific API level: %s", Api::getApiName(apiType = m_OSManager.getApiTypeRequired()));
	}

	if (apiType > getMaxApiVersion())
	{
		Log(Log.Error, "================================================================================\n"
		    "API level requested [%s] was not supported. Max supported API level on this device is [%s]\n"
		    "**** APPLICATION WILL EXIT ****\n"
		    "================================================================================",
		    Api::getApiName(apiType), Api::getApiName(getMaxApiVersion()));
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

	m_swapInterval = m_OSManager.getDisplayAttributes().swapInterval;
	m_initialized = true;
	return Result::Success;
}

Api::Enum PlatformContext::getMaxApiVersion()
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
	Api::Enum graphicsapi = Api::OpenGLESMaxVersion;
	bool supported;
	Result::Enum result;
	while (graphicsapi > Api::Unspecified)
	{
		const char* esversion = (graphicsapi == Api::OpenGLES31 ? "3.1" : graphicsapi == Api::OpenGLES3 ? "3.0" : graphicsapi == Api::OpenGLES2 ?
		                         "2.0" : "UNKNOWN_VERSION");
		result = isGlesVersionSupported(m_platformContextHandles->display, graphicsapi, supported);

		if (result == Result::Success)
		{
			if (supported)
			{
				m_maxApiVersion = graphicsapi;
				Log(Log.Verbose, "Maximum API level detected: OpenGL ES %s", esversion);
				return;
			}
			else
			{
				Log(Log.Verbose, "OpenGL ES %s NOT supported. Trying lower version...", esversion);
			}
		}
		else
		{
			Log("Error detected while testing OpenGL ES version %s for compatibility. Trying lower version", esversion);
		}
		graphicsapi = (Api::Enum)(graphicsapi - 1);
	}
	Log(Log.Critical, "=== FATAL: COULD NOT FIND COMPATIBILITY WITH ANY OPENGL ES VERSION ===");

}

bool PlatformContext::isApiSupported(Api::Enum apiLevel)
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
	return std::auto_ptr<IPlatformContext>(new system::PlatformContext(mgr));
}
}
//!\endcond